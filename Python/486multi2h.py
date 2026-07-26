# script: multi_return_32bit_shifter.py
import sys
import re

def optimize_multi_return_32bit(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    modified_asm_lines = []
    header_functions = {}
    current_func = None
    
    # Track the pointer registers passed as arguments
    sin_ptr_reg = None
    cos_ptr_reg = None

    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    # Matches the loading of the destination pointers from the stack
    mov_stack_pattern = re.compile(r'^\s*movl\s+([0-9]+)\(%esp\),\s+(%[a-z]+)')
    # Matches a pointer write: e.g., movl %eax, (%ecx)
    ptr_write_pattern = re.compile(r'^\s*movl\s+(%[a-z0-9]+),\s*\((%[a-z0-9]+)\)')

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
                
            header_functions[current_func] = {'inputs': ['eax'], 'outputs': ['eax', 'edx']}
            sin_ptr_reg = None
            cos_ptr_reg = None
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                current_func = None
                modified_asm_lines.append(line)
                continue

            # 1. Strip the loading of the pointer destination addresses
            stack_match = mov_stack_pattern.match(line)
            if stack_match:
                offset = int(stack_match.group(1))
                dest_reg = stack_match.group(2)
                # If these correspond to the 2nd and 3rd arguments (the pointers)
                if offset == 12: # sin pointer offset
                    sin_ptr_reg = dest_reg
                    continue
                if offset == 16: # cos pointer offset
                    cos_ptr_reg = dest_reg
                    continue

            # 2. Intercept the memory writes and re-route them straight to EAX and EDX!
            write_match = ptr_write_pattern.match(line)
            if write_match:
                src_val_reg = write_match.group(1)
                target_ptr_reg = write_match.group(2)
                
                # If writing to the sin pointer, reroute to EAX
                if target_ptr_reg == sin_ptr_reg:
                    modified_asm_lines.append(f"    movl {src_val_reg}, %eax\n")
                    continue
                # If writing to the cos pointer, reroute to EDX
                if target_ptr_reg == cos_ptr_reg:
                    modified_asm_lines.append(f"    movl {src_val_reg}, %edx\n")
                    continue

        modified_asm_lines.append(line)

    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # 3. Emit the Twin-Register Output Wrapper Header
    with open(output_h_path, 'a') as h_out:
        for f_name, data in header_functions.items():
            if f_name not in ['sincos', 'divmod']: continue
            
            h_out.write(f"static inline void call_zero_overhead_{f_name}(int input_val, int *out_primary, int *out_secondary) {{\n")
            h_out.write(f"    register int reg_eax __asm__(\"eax\") = input_val;\n")
            h_out.write(f"    register int reg_edx __asm__(\"edx\");\n")
            
            h_out.write("    __asm__ __volatile__ (\n")
            h_out.write(f"        \"call {f_name}\\n\\t\"\n")
            h_out.write(f"        : \"+a\"(reg_eax), \"=d\"(reg_edx)\n") # Dual register output contract!
            h_out.write(f"        : \n")
            h_out.write(f"        : \"ecx\", \"cc\"\n") # Memory clobber is completely gone!
            h_out.write("    );\n")
            h_out.write("    *out_primary = reg_eax;\n")
            h_out.write("    *out_secondary = reg_edx;\n")
            h_out.write("}\n\n")

if __name__ == '__main__':
    optimize_multi_return_32bit(sys.argv, sys.argv, sys.argv)
  
