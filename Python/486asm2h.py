# script: regcall_32bit_shifter.py
import sys
import re

def optimize_to_regcall_32bit(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    modified_asm_lines = []
    header_functions = {}
    current_func = None
    
    # 🚀 Clang __regcall standard GPR allocation sequence for x86 32-bit
    available_regs = ['eax', 'ecx', 'edx', 'edi', 'esi']
    stack_to_reg_map = {}

    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    mov_stack_pattern = re.compile(r'^\s*movl\s+([0-9]+)\((%esp|%ebp)\),\s+(%[a-z]+)')

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
                
            header_functions[current_func] = {'inputs': [], 'clobbers': set()}
            stack_to_reg_map = {}
            # Reset the pool per function signature
            available_regs = ['eax', 'ecx', 'edx', 'edi', 'esi'] 
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                # Track output states and clobber lists
                used_regs = set(header_functions[current_func]['inputs']).union({'eax'})
                for r in ['eax', 'edx', 'ecx', 'ebx', 'esi', 'edi']:
                    if r not in used_regs:
                        header_functions[current_func]['clobbers'].add(r)
                
                current_func = None
                modified_asm_lines.append(line)
                continue

            # Capture stack argument pulls (e.g., movl 12(%esp), %esi)
            stack_match = mov_stack_pattern.match(line)
            if stack_match:
                offset = int(stack_match.group(1))
                internal_dest_reg = stack_match.group(3).replace('%', '')
                
                if available_regs:
                    # Allocate next register from Clang's regcall pool matrix
                    assigned_carrier_reg = available_regs.pop(0)
                    stack_to_reg_map[offset] = (assigned_carrier_reg, internal_dest_reg)
                    header_functions[current_func]['inputs'].append(assigned_carrier_reg)
                    
                    # Convert structural RAM read to a lightning-fast internal register move
                    if assigned_carrier_reg != internal_dest_reg:
                        modified_asm_lines.append(f"    movl %{assigned_carrier_reg}, %{internal_dest_reg}\n")
                    continue # ❌ STRIP THE RAM DELAY ENTIRELY FROM THE SHARED LIBRARY

        modified_asm_lines.append(line)

    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # Emit the expanded 5-Register C Header macro wrappers
    with open(output_h_path, 'a') as h_out:
        for f_name, data in header_functions.items():
            if not data['inputs']: continue
            
            args = ", ".join([f"int param_{r}" for r in data['inputs']])
            h_out.write(f"static inline int zero_stack_{f_name}({args}) {{\n")
            for r in data['inputs']:
                h_out.write(f"    register int reg_{r} __asm__(\"{r}\") = param_{r};\n")
            h_out.write(f"    register int reg_eax __asm__(\"eax\");\n")
            
            h_out.write("    __asm__ __volatile__ (\n")
            h_out.write(f"        \"call {f_name}\\n\\t\"\n")
            
            clob_strs = [f'\"{c}\"' for c in sorted(list(data['clobbers']))]
            clob_strs.append('"cc"')
            
            h_out.write(f"        : \"=a\"(reg_eax)\n")
            h_out.write(f"        : {', '.join([f'\"r\"(reg_{r})' for r in data['inputs']])}\n")
            h_out.write(f"        : {', '.join(clob_strs)}\n")
            h_out.write("    );\n")
            h_out.write("    return reg_eax;\n")
            h_out.write("}\n\n")

if __name__ == '__main__':
    optimize_to_regcall_32bit(sys.argv, sys.argv, sys.argv)
