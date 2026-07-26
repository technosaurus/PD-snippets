# script: unified_asm_optimizer.py
import sys
import re

def run_unified_optimization_pass(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    sys_v_32_inputs = {'%eax', '%ecx', '%edx', '%edi', '%esi'}
    
    modified_asm_lines = []
    header_functions = {}
    
    current_func = None
    in_prologue = False
    
    # State tracking variables per function block
    all_written_regs = set()
    shuffled_registers = set()
    stack_args_found = []     # Track 32-bit parameters loaded via memory stack frames
    array_base_mappings = {}  # Tracks registers used as active array base addresses

    # Compilation Regex Matrix
    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    mov_reg_pattern = re.compile(r'^\s*movl\s+(%[a-z0-9]+),\s+(%[a-z0-9]+)')
    mov_stack_pattern = re.compile(r'^\s*movl\s+([0-9]+)\(%esp\),\s+(%[a-z0-9]+)')
    ptr_write_pattern = re.compile(r'^\s*movl\s+(%[a-z0-9]+),\s*([0-9]*)\((%[a-z0-9]+)\)')
    array_scale_pattern = re.compile(r'^\s*([a-z]+)\s+\((%[a-z]+),(%[a-z]+),([0-9]+)\),\s+(%[a-z0-9]+)')

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
            
            # Initialize consolidated metadata map for this function block
            header_functions[current_func] = {
                'inputs': set(), 'outputs': set(), 'clobbers': set(), 'stack_loads': [], 'is_memory': False
            }
            in_prologue = True
            all_written_regs = set()
            shuffled_registers = set()
            stack_args_found = []
            array_base_mappings = {}
            
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                # Resolve combined output register tracking (EAX, or EDX for multi-returns)
                for reg in all_written_regs.intersection({'%eax', '%edx', '%ecx'}):
                    header_functions[current_func]['outputs'].add(reg.replace('%', ''))
                if not header_functions[current_func]['outputs']:
                    header_functions[current_func]['outputs'].add('eax') # Default fallback
                
                # Filter down internal scratch registers for clobbers
                for reg in all_written_regs:
                    reg_clean = reg.replace('%', '')
                    if reg_clean not in header_functions[current_func]['outputs'] and reg_clean not in header_functions[current_func]['inputs']:
                        if reg_clean not in ['rsp', 'rbp', 'esp', 'ebp']:
                            header_functions[current_func]['clobbers'].add(reg_clean)
                
                current_func = None
                in_prologue = False
                modified_asm_lines.append(line)
                continue

            # 1. OPTIMIZATION A & B: Match Prologue Parameter Shuffling & Stack Argument Spilling
            mov_match = mov_reg_pattern.match(line)
            if in_prologue and mov_match:
                src, dest = mov_match.group(1), mov_match.group(2)
                if src in sys_v_32_inputs:
                    header_functions[current_func]['inputs'].add(dest.replace('%', ''))
                    shuffled_registers.add(dest)
                    continue # Complete Elimination: Skip emitting this shuffling instruction!

            stack_match = mov_stack_pattern.match(line)
            if in_prologue and stack_match:
                offset = int(stack_match.group(1))
                dest = stack_match.group(2)
                if offset >= 4:
                    header_functions[current_func]['inputs'].add(dest.replace('%', ''))
                    header_functions[current_func]['stack_loads'].append((offset, dest.replace('%', '')))
                    continue # Complete Elimination: Strip the RAM delay from the library binary!

            # Terminate prologue scanning if code execution loops begin
            if in_prologue and not (mov_match or stack_match or line.strip().startswith('.')):
                in_prologue = False

            # 2. OPTIMIZATION C: Intercept Loop Array Lookups (Convert Base-Index-Scale to Pointer Streaming)
            array_match = array_scale_pattern.match(line)
            if array_match:
                op = array_match.group(1)       # e.g., movl
                base = array_match.group(2)     # e.g., %ebx (The Array Base Pointer)
                scale = int(array_match.group(4)) # e.g., 4 (Data type width)
                dest = array_match.group(5)     # e.g., %eax
                
                header_functions[current_func]['inputs'].add(base.replace('%', ''))
                header_functions[current_func]['is_memory'] = True
                
                # Rewrite to an auto-incrementing pointer sequence
                modified_asm_lines.append(f"    {op} ({base}), {dest}\n")
                modified_asm_lines.append(f"    addl ${scale}, {base}\n")
                continue

            # 3. OPTIMIZATION D: Intercept Pointer Outputs & Map directly to multi-return fields
            write_match = ptr_write_pattern.match(line)
            if write_match:
                src_val_reg = write_match.group(1)
                target_ptr_reg = write_match.group(3)
                
                if '(%esp)' not in line and '(%ebp)' not in line:
                    header_functions[current_func]['is_memory'] = True
                    # If this writes back through a tracking register pointer, convert it to a multi-register return map
                    if target_ptr_reg.replace('%', '') in header_functions[current_func]['inputs']:
                        # Assign output parameters straight to additional general-purpose registers (EDX/ECX)
                        modified_asm_lines.append(f"    movl {src_val_reg}, %edx\n")
                        header_functions[current_func]['outputs'].add('edx')
                        continue

            # Track global register writes for clobber generation
            if ',' in line and not line.strip().startswith('.'):
                parts = line.split(',')
                last_operand = parts[-1].strip()
                if last_operand.startswith('%') and '(%esp)' not in last_operand:
                    clean_reg = last_operand.split()[0]
                    all_written_regs.add(clean_reg)

        modified_asm_lines.append(line)

    # Output the optimized, multi-pass transformed assembly code file
    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # 4. Generate the Complete Macro Wrapper File Matrix
    with open(output_h_path, 'w') as h_out:
        h_out.write("#ifndef ZERO_OVERHEAD_MASTER_H\n#define ZERO_OVERHEAD_MASTER_H\n\n")
        
        # Structure definitions for multi-return handling
        h_out.write("typedef struct {\n    unsigned long primary;\n    unsigned long secondary;\n} multi_return_t;\n\n")
        
        for f_name, data in header_functions.items():
            if not data['inputs'] and not data['outputs']: continue
            
            inputs_sorted = sorted(list(data['inputs']))
            outputs_sorted = sorted(list(data['outputs']))
            
            args = ", ".join([f"unsigned long param_{r}" for r in inputs_sorted])
            
            # Choose return profile: primitive value if single, struct if multi-return
            ret_type = "multi_return_t" if len(outputs_sorted) > 1 else "unsigned long"
            
            h_out.write(f"static inline {ret_type} zero_overhead_{f_name}({args}) {{\n")
            #h_out.write(f"\tif (__builtin_constant_p({args})) {return __builtin_{f_name}({args});}"
            if len(outputs_sorted) > 1:
                h_out.write("    multi_return_t result;\n")
                
            for r in inputs_sorted:
                h_out.write(f"    register unsigned long reg_{r} __asm__(\"{r}\") = param_{r};\n")
            for r in outputs_sorted:
                if r not in inputs_sorted:
                    h_out.write(f"    register unsigned long reg_{r} __asm__(\"{r}\");\n")
            
            h_out.write("    __asm__ __volatile__ (\n")
            
            # Re-inject dynamically computed stack args inside the macro wrapper boundary
            for offset, dest in data['stack_loads']:
                h_out.write(f"        \"pushl %%{dest}\\n\\t\"\n")
                
            h_out.write(f"        \"call {f_name}\\n\\t\" // Call straight to the stripped computation body\n")
            
            if data['stack_loads']:
                h_out.write(f"        \"addl ${len(data['stack_loads'])*4}, %%esp\\n\\t\"\n")
                
            out_strs = [f'"+r"(reg_{r})' if r in inputs_sorted else f'"=r"(reg_{r})' for r in outputs_sorted]
            in_strs = [f'"r"(reg_{r})' for r in inputs_sorted if r not in outputs_sorted]
            
            clob_strs = [f'\"{c}\"' for c in sorted(list(data['clobbers'])) if c not in outputs_sorted]
            clob_strs.append('"cc"')
            if data['is_memory']:
                clob_strs.append('"memory"')
            
            h_out.write(f"        : {', '.join(out_strs)}\n")
            h_out.write(f"        : {', '.join(in_strs)}\n")
            h_out.write(f"        : {', '.join(sorted(list(set(clob_strs))))}\n")
            h_out.write("    );\n\n")
            
            if len(outputs_sorted) > 1:
                h_out.write(f"    result.primary = reg_{outputs_sorted[0]};\n")
                h_out.write(f"    result.secondary = reg_{outputs_sorted[1]};\n")
                h_out.write("    return result;\n")
            else:
                h_out.write(f"    return reg_{outputs_sorted[0]};\n")
                
            h_out.write("}\n\n")
            h_out.write("#endif // ZERO_OVERHEAD_MASTER_H\n")

if name == 'main':run_unified_optimization_pass(sys.argv[1], sys.argv[2], sys.argv[3])

          
