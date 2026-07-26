# script: rewrite_asm_and_header.py
import sys
import re

def process_asm_and_generate_header(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    sys_v_inputs = {'%rdi', '%rsi', '%rdx', '%rcx', '%r8', '%r9'}
    
    modified_asm_lines = []
    header_functions = {}
    
    current_func = None
    in_prologue = False
    shuffled_registers = set()
    all_written_regs = set()
    has_global_write = False

    # Regex patterns for matching structural assembly elements
    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    mov_reg_pattern = re.compile(r'^\s*movq\s+(%[a-z0-9]+),\s+(%[a-z0-9]+)')
    mov_stack_pattern = re.compile(r'^\s*movq\s+([0-9]+)\(%rsp\),\s+(%[a-z0-9]+)')
    mem_write_pattern = re.compile(r'^\s*[a-z]+\s+.*,\s*\(%[a-z0-9]+\)')

    for line in lines:
        # 1. Match a new function start boundary
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            header_functions[current_func] = {
                'inputs': set(), 'outputs': set(), 'clobbers': set(), 'stack_loads': []
            }
            in_prologue = True
            shuffled_registers = set()
            all_written_regs = set()
            has_global_write = False
            modified_asm_lines.append(line)
            continue

        if current_func:
            # Detect function end
            if 'ret' in line:
                # Map outputs (standard System V return registers)
                for reg in all_written_regs.intersection({'%rax', '%rdx', '%rsi'}):
                    header_functions[current_func]['outputs'].add(reg.replace('%', ''))
                
                # Filter down internal scratch registers for clobbers
                for reg in all_written_regs:
                    reg_clean = reg.replace('%', '')
                    if reg_clean not in header_functions[current_func]['outputs'] and reg_clean not in header_functions[current_func]['inputs']:
                        if reg_clean not in ['rsp', 'rbp']:
                            header_functions[current_func]['clobbers'].add(reg_clean)
                
                if has_global_write:
                    header_functions[current_func]['clobbers'].add('memory')
                
                current_func = None
                in_prologue = False
                modified_asm_lines.append(line)
                continue

            # 2. Check for Parameter Shuffling in the prologue (e.g., movq %rdi, %r10)
            mov_match = mov_reg_pattern.match(line)
            if in_prologue and mov_match:
                src, dest = mov_match.group(1), mov_match.group(2)
                if src in sys_v_inputs:
                    header_functions[current_func]['inputs'].add(dest.replace('%', ''))
                    shuffled_registers.add(dest)
                    # Complete Elimination: We skip appending this line entirely!
                    continue

            # 3. Check for Stack Loads in the prologue (e.g., movq 16(%rsp), %r11)
            stack_match = mov_stack_pattern.match(line)
            if in_prologue and stack_match:
                offset, dest = int(stack_match.group(1)), stack_match.group(2)
                if offset >= 16:
                    header_functions[current_func]['inputs'].add(dest.replace('%', ''))
                    header_functions[current_func]['stack_loads'].append((offset, dest.replace('%', '')))
                    # Complete Elimination: Skip appending this line!
                    continue

            # End of structural prologue setup 
            if in_prologue and not (mov_match or stack_match or line.strip().startswith('.')):
                in_prologue = False

            # Track writing changes for remaining body calculations
            if ',' in line:
                parts = line.split(',')
                last_operand = parts[-1].strip()
                if last_operand.startswith('%'):
                    all_written_regs.add(last_operand.split()[0])
            
            # Check for pointer writes: Modifying non-stack memory references
            if mem_write_pattern.match(line) and '(%rsp)' not in line and '(%rbp)' not in line:
                has_global_write = True

        modified_asm_lines.append(line)

    # Output the newly re-engineered inline assembly file (.s)
    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # Output the optimized accompanying C header file (.h)
    with open(output_h_path, 'w') as h_out:
        h_out.write("#ifndef ZERO_OVERHEAD_BOUNDARIES_H\n#define ZERO_OVERHEAD_BOUNDARIES_H\n\n")
        
        for f_name, data in header_functions.items():
            if not data['inputs'] and not data['outputs']: continue # Skip metadata symbols
            
            inputs_sorted = sorted(list(data['inputs']))
            outputs_sorted = sorted(list(data['outputs']))
            
            args = ", ".join([f"long param_{r}" for r in inputs_sorted])
            out_ptrs = ", ".join([f"long *out_{r}" for r in outputs_sorted])
            sig = f"{args}, {out_ptrs}" if out_ptrs else args
            
            h_out.write(f"static inline void call_{f_name}({sig}) {{\n")
            for r in inputs_sorted:
                h_out.write(f"    register long reg_{r} __asm__(\"{r}\") = param_{r};\n")
            for r in outputs_sorted:
                if r not in inputs_sorted:
                    h_out.write(f"    register long reg_{r} __asm__(\"{r}\");\n")
            
            h_out.write("    __asm__ __volatile__ (\n")
            
            # Re-inject the stack frames inside the macro safely if needed
            for offset, dest in data['stack_loads']:
                h_out.write(f"        \"pushq %%{dest}\\n\\t\"\n")
                
            h_out.write(f"        \"call {f_name}\\n\\t\" // Call straight to the start!\n")
            
            if data['stack_loads']:
                h_out.write(f"        \"addq ${len(data['stack_loads'])*8}, %%rsp\\n\\t\"\n")
                
            out_strs = [f'"+r"(reg_{r})' if r in inputs_sorted else f'"=r"(reg_{r})' for r in outputs_sorted]
            in_strs = [f'"r"(reg_{r})' for r in inputs_sorted if r not in outputs_sorted]
            clob_strs = [f'\"{c}\"' for c in sorted(list(data['clobbers']))]
            if 'eflags' in clob_strs: clob_strs = [c if c != 'eflags' else 'cc' for c in clob_strs]
            
            h_out.write(f"        : {', '.join(out_strs)}\n")
            h_out.write(f"        : {', '.join(in_strs)}\n")
            h_out.write(f"        : {', '.join(clob_strs or ['\"cc\"'])}\n")
            h_out.write("    );\n")
            
            for r in outputs_sorted:
                h_out.write(f"    *out_{r} = reg_{r};\n")
            h_out.write("}\n\n")
            
        h_out.write("#endif\n")

if __name__ == '__main__':
    process_asm_and_generate_header(sys.argv[1], sys.argv[2], sys.argv[3])
