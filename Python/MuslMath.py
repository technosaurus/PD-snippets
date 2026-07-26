# script: musl_math_optimizer.py
import sys
import re

def optimize_musl_math(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    sys_v_fp_inputs = {'%xmm0', '%xmm1', '%xmm2', '%xmm3', '%xmm4', '%xmm5', '%xmm6', '%xmm7'}
    
    modified_asm_lines = []
    header_functions = {}
    
    current_func = None
    in_prologue = False
    all_written_fp_regs = set()
    all_read_fp_regs = set()

    # Regex patterns for matching structural assembly elements
    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    mov_fp_pattern = re.compile(r'^\s*movap[sd]\s+(%xmm[0-7]),\s+(%xmm[0-9]+)|^\s*movs[sd]\s+(%xmm[0-7]),\s+(%xmm[0-9]+)')

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            # Filter for math functions, skipping internal or metadata labels
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
                
            header_functions[current_func] = {'inputs': set(), 'outputs': set(), 'clobbers': set()}
            in_prologue = True
            all_written_fp_regs = set()
            all_read_fp_regs = set()
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                # Math functions universally return floating point values via xmm0
                if '%xmm0' in all_written_fp_regs:
                    header_functions[current_func]['outputs'].add('xmm0')
                
                # Identify scratch registers clobbered by this function
                for reg in all_written_fp_regs:
                    reg_clean = reg.replace('%', '')
                    if reg_clean not in header_functions[current_func]['outputs'] and reg_clean not in header_functions[current_func]['inputs']:
                        header_functions[current_func]['clobbers'].add(reg_clean)
                
                current_func = None
                in_prologue = False
                modified_asm_lines.append(line)
                continue

            # Check for FP parameter shuffling in prologue (e.g., movaps %xmm0, %xmm4)
            fp_match = mov_fp_pattern.match(line)
            if in_prologue and fp_match:
                # Extract non-None groups from the OR regex match
                groups = [g for g in fp_match.groups() if g is not None]
                src, dest = groups[0], groups[1]
                
                if src in sys_v_fp_inputs:
                    header_functions[current_func]['inputs'].add(dest.replace('%', ''))
                    # Complete Elimination: Skip emitting this shuffling instruction!
                    continue

            if in_prologue and not (fp_match or line.strip().startswith('.')):
                in_prologue = False

            # Track floating-point register usage throughout the function body
            fp_regs_found = re.findall(r'%xmm[0-9]+', line)
            if fp_regs_found:
                # In standard x86 assembly, the destination register is the rightmost/last operand
                all_written_fp_regs.add(fp_regs_found[-1])
                for r in fp_regs_found[:-1]:
                    all_read_fp_regs.add(r)

        modified_asm_lines.append(line)

    # Output the optimized assembly file
    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # Append the custom Zero-Overhead C Wrappers to our custom header file
    with open(output_h_path, 'a') as h_out:
        for f_name, data in header_functions.items():
            if not data['inputs'] and 'xmm0' not in data['outputs']: continue
            
            inputs_sorted = sorted(list(data['inputs']))
            args = ", ".join([f"double param_{r}" for r in inputs_sorted])
            
            h_out.write(f"static inline double zero_overhead_{f_name}({args}) {{\n")
            for r in inputs_sorted:
                h_out.write(f"    register double reg_{r} __asm__(\"{r}\") = param_{r};\n")
            h_out.write(f"    register double reg_xmm0 __asm__(\"xmm0\");\n")
            
            h_out.write("    __asm__ __volatile__ (\n")
            h_out.write(f"        \"call {f_name}\\n\\t\"\n")
            
            out_strs = ['"=t"(reg_xmm0)'] # Use the 't' standard constraint for top-of-SSE vector maps
            in_strs = [f'"{r}"(reg_{r})' for r in inputs_sorted]
            clob_strs = [f'\"{c}\"' for c in sorted(list(data['clobbers'])) if c != 'xmm0']
            clob_strs.append('"cc"') # Append CPU status flags
            
            h_out.write(f"        : \"=x\"(reg_xmm0)\n") # Map explicitly to SSE vector register constraints
            h_out.write(f"        : {', '.join([f'\"x\"(reg_{r})' for r in inputs_sorted])}\n")
            h_out.write(f"        : {', '.join(clob_strs)}\n")
            h_out.write("    );\n")
            h_out.write("    return reg_xmm0;\n")
            h_out.write("}\n\n")

if __name__ == '__main__':
    optimize_musl_math(sys.argv[1], sys.argv[2], sys.argv[3])
