# script: array_lookup_shifter.py
import sys
import re

def optimize_array_lookups(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    modified_asm_lines = []
    header_functions = {}
    current_func = None
    
    # Track array base pointer configuration
    array_base_reg = None
    index_reg = None

    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    # Matches a base-index-scale lookup instruction: e.g., movl (%ebx,%ecx,4), %eax
    array_lookup_pattern = re.compile(r'^\s*([a-z]+)\s+\((%[a-z]+),(%[a-z]+),([0-9]+)\),\s+(%[a-z0-9]+)')

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
                
            header_functions[current_func] = {'inputs': set()}
            array_base_reg = None
            index_reg = None
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                current_func = None
                modified_asm_lines.append(line)
                continue

            # Intercept array index lookups and flatten them to direct pointer lookups
            lookup_match = array_lookup_pattern.match(line)
            if lookup_match:
                op = lookup_match.group(1)       # e.g., movl
                base = lookup_match.group(2)     # e.g., %ebx (The Array Base Pointer)
                index = lookup_match.group(3)    # e.g., %ecx (The loop index counter)
                scale = int(lookup_match.group(4)) # e.g., 4 (for int / float data)
                dest = lookup_match.group(5)     # e.g., %eax
                
                header_functions[current_func]['inputs'].add(base.replace('%', ''))
                
                # 🚀 Rewrite the multi-register lookup to a zero-overhead pointer stream
                modified_asm_lines.append(f"    {op} ({base}), {dest}\n")   # Direct, single-register read
                modified_asm_lines.append(f"    addl ${scale}, {base}\n")  # Shift pointer base to next index item inline
                continue

        modified_asm_lines.append(line)

    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # Output the optimized accompanying header file mapping
    with open(output_h_path, 'a') as h_out:
        for f_name, data in header_functions.items():
            if not data['inputs']: continue
            
            inputs_sorted = sorted(list(data['inputs']))
            args = ", ".join([f"void *param_{r}" for r in inputs_sorted])
            
            h_out.write(f"static inline void run_opt_array_{f_name}({args}) {{\n")
            for r in inputs_sorted:
                h_out.write(f"    register void *reg_{r} __asm__(\"{r}\") = param_{r};\n")
                
            h_out.write("    __asm__ __volatile__ (\n")
            h_out.write(f"        \"call {f_name}\\n\\t\"\n")
            h_out.write(f"        : \"+r\"(reg_{inputs_sorted[0]})\n")
            h_out.write(f"        : \n")
            h_out.write(f"        : \"cc\", \"memory\"\n") # Memory clobber handles the pointer buffer reads safely
            h_out.write("    );\n")
            h_out.write("}\n\n")

if __name__ == '__main__':
    optimize_array_lookups(sys.argv, sys.argv, sys.argv)
  
