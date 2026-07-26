# script: x87_fpu_shifter.py
import sys
import re

def optimize_x87_fpu_32bit(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    modified_asm_lines = []
    header_functions = {}
    current_func = None
    
    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    # Matches FPU stack loads from memory, e.g., fldl 4(%esp) or flds 8(%esp)
    fld_stack_pattern = re.compile(r'^\s*fld[ls]\s+([0-9]+)\(%esp\)')

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
                
            header_functions[current_func] = {'args_count': 0}
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                current_func = None
                modified_asm_lines.append(line)
                continue

            # Intercept and eliminate the slow memory load into the FPU
            fld_match = fld_stack_pattern.match(line)
            if fld_match:
                header_functions[current_func]['args_count'] += 1
                # 🚀 ELIMINATE THE INSTUCTION: We do not append it to the optimized assembly!
                # The value will already be sitting cleanly in st(0)/st(1)
                continue

        modified_asm_lines.append(line)

    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # Generate the Zero-Memory FPU Pipeline Header
    with open(output_h_path, 'a') as h_out:
        for f_name, data in header_functions.items():
            if data['args_count'] == 0: continue
            
            # Formulate the C arguments
            args = ", ".join([f"double a{i}" for i in range(data['args_count'])])
            h_out.write(f"static inline double zero_mem_fpu_{f_name}({args}) {{\n")
            h_out.write("    double result;\n")
            h_out.write("    __asm__ __volatile__ (\n")
            
            # The inline template automatically pre-loads values directly onto the x87 FPU stack!
            # Using standard GCC constraints: "t" maps to st(0), "u" maps to st(1)
            h_out.write(f"        \"call {f_name}\\n\\t\"\n")
            
            if data['args_count'] == 1:
                h_out.write("        : \"=t\"(result)\n") # Returns in st(0)
                h_out.write("        : \"0\"(a0)\n")      # Passed directly in st(0)
            elif data['args_count'] == 2:
                h_out.write("        : \"=t\"(result)\n") 
                h_out.write("        : \"0\"(a0), \"u\"(a1)\n") # passed in st(0) and st(1)
                
            h_out.write("        : \"cc\"\n") # Removes memory clobbers completely
            h_out.write("    );\n")
            h_out.write("    return result;\n")
            h_out.write("}\n\n")

if __name__ == '__main__':
    optimize_x87_fpu_32bit(sys.argv, sys.argv, sys.argv)
  
