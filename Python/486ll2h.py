# script: regcall_64bit_shifter.py
import sys
import re

def optimize_64bit_pairs_32bit(input_s_path, output_s_path, output_h_path):
    with open(input_s_path, 'r') as f:
        lines = f.readlines()

    modified_asm_lines = []
    header_functions = {}
    current_func = None

    func_label_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):')
    # Matches any typical stack load: movl X(%esp), %reg
    mov_stack_pattern = re.compile(r'^\s*movl\s+([0-9]+)\(%esp\),\s+(%[a-z]+)')
    # Matches a pointer write: movl %reg, X(%reg)
    ptr_write_pattern = re.compile(r'^\s*movl\s+(%[a-z0-9]+),\s*([0-9]*)\((%[a-z0-9]+)\)')

    # Track structural offsets for the function being transformed
    out_ptr_reg = None

    for line in lines:
        label_match = func_label_pattern.match(line)
        if label_match:
            current_func = label_match.group(1)
            if current_func.startswith('.') or current_func.startswith('__'):
                current_func = None
                modified_asm_lines.append(line)
                continue
                
            header_functions[current_func] = {'valid': True}
            out_ptr_reg = None
            modified_asm_lines.append(line)
            continue

        if current_func:
            if 'ret' in line:
                current_func = None
                modified_asm_lines.append(line)
                continue

            # 1. Strip the standard 32-bit stack argument loading sequences
            stack_match = mov_stack_pattern.match(line)
            if stack_match:
                offset = int(stack_match.group(1))
                dest_reg = stack_match.group(2)
                
                # Assume original function signature was: divmod64(uint64_t a, uint64_t b, uint64_t *out_mod)
                # Input 'a' low/high: offsets 4 and 8 -> We map directly to EAX and EDX
                # Input 'b' low/high: offsets 12 and 16 -> We map directly to ECX and EBX
                # Pointer to output struct/mod: offset 20
                if offset == 4 and dest_reg != '%eax':
                    modified_asm_lines.append(f"    movl %eax, {dest_reg}\n")
                    continue
                elif offset == 8 and dest_reg != '%edx':
                    modified_asm_lines.append(f"    movl %edx, {dest_reg}\n")
                    continue
                elif offset == 12 and dest_reg != '%ecx':
                    modified_asm_lines.append(f"    movl %ecx, {dest_reg}\n")
                    continue
                elif offset == 16 and dest_reg != '%ebx':
                    modified_asm_lines.append(f"    movl %ebx, {dest_reg}\n")
                    continue
                elif offset == 20:
                    out_ptr_reg = dest_reg # Trap the memory pointer destination register
                    continue

            # 2. Intercept memory pointer writes and convert them into our multi-register return matrix
            write_match = ptr_write_pattern.match(line)
            if write_match and out_ptr_reg:
                src_reg = write_match.group(1)
                offset_disp = write_match.group(2)
                offset_disp = int(offset_disp) if offset_disp else 0
                target_ptr = write_match.group(3)
                
                if target_ptr == out_ptr_reg:
                    # Reroute the memory write directly to output registers!
                    # Struct Member 1 (Quotient) returned in EAX:EDX natively by standard compilation
                    # Struct Member 2 (Remainder) low/high bits rerouted to ECX:EBX
                    if offset_disp == 0:
                        modified_asm_lines.append(f"    movl {src_reg}, %ecx\n")
                        continue
                    elif offset_disp == 4:
                        modified_asm_lines.append(f"    movl {src_reg}, %ebx\n")
                        continue

        modified_asm_lines.append(line)

    with open(output_s_path, 'w') as f:
        f.writelines(modified_asm_lines)

    # 3. Emit the 64-bit Struct Multi-Return Header
    with open(output_h_path, 'a') as h_out:
        h_out.write("typedef struct {\n    unsigned long long primary;\n    unsigned long long secondary;\n} uint128_return_t;\n\n")
        
        for f_name in header_functions.keys():
            h_out.write(f"static inline uint128_return_t call_opt_{f_name}(unsigned long long a, unsigned long long b) {{\n")
            h_out.write("    union { unsigned long long val; struct { unsigned long lo; unsigned long hi; } parts; } u_a = { .val = a };\n")
            h_out.write("    union { unsigned long long val; struct { unsigned long lo; unsigned long hi; } parts; } u_b = { .val = b };\n")
            h_out.write("    uint128_return_t result;\n")
            h_out.write("    union { unsigned long long val; struct { unsigned long lo; unsigned long hi; } parts; } res_1;\n")
            h_out.write("    union { unsigned long long val; struct { unsigned long lo; unsigned long hi; } parts; } res_2;\n\n")
            
            # Lock the 32-bit registers explicitly for the inline assembly boundary
            h_out.write("    register unsigned long reg_eax __asm__(\"eax\") = u_a.parts.lo;\n")
            h_out.write("    register unsigned long reg_edx __asm__(\"edx\") = u_a.parts.hi;\n")
            h_out.write("    register unsigned long reg_ecx __asm__(\"ecx\") = u_b.parts.lo;\n")
            h_out.write("    register unsigned long reg_ebx __asm__(\"ebx\") = u_b.parts.hi;\n")
            
            h_out.write("    __asm__ __volatile__ (\n")
            h_out.write(f"        \"call {f_name}\\n\\t\"\n")
            h_out.write("        : \"+a\"(reg_eax), \"+d\"(reg_edx), \"=c\"(reg_ecx), \"=b\"(reg_ebx)\n") # Output Register Matrix
            h_out.write("        : \n")
            h_out.write("        : \"esi\", \"edi\", \"cc\"\n") # Memory clobber is 100% eliminated
            h_out.write("    );\n\n")
            
            h_out.write("    res_1.parts.lo = reg_eax;\n")
            h_out.write("    res_1.parts.hi = reg_edx;\n")
            h_out.write("    res_2.parts.lo = reg_ecx;\n")
            h_out.write("    res_2.parts.hi = reg_ebx;\n")
            h_out.write("    result.primary = res_1.val;\n")
            h_out.write("    result.secondary = res_2.val;\n")
            h_out.write("    return result;\n")
            h_out.write("}\n\n")

if __name__ == '__main__':
    optimize_64bit_pairs_32bit(sys.argv, sys.argv, sys.argv)
      
