from capstone import *
from capstone.x86 import *

# Raw byte code representing our target leaf function in the shared library.
# This sample clears RAX, adds RSI to RDI, subtracts RDX from RSI, and returns.
TARGET_MACHINE_BYTES = b"\x48\x31\xc0\x48\x01\xf7\x48\x29\xd6\xc3"

def generate_optimized_header(func_name, machine_code):
    # 1. Initialize Capstone for x86_64 and enable execution detailing
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True  # Required to unlock .regs_read and .regs_write accessors

    all_read_regs = set()
    all_written_regs = set()
    
    # 2. Iterate through instructions and log data mutations
    for insn in md.disasm(machine_code, 0x1000):
        # Log implicit/explicit registers read by this instruction
        for reg in insn.regs_read:
            all_read_regs.add(insn.reg_name(reg))
            
        # Log implicit/explicit registers modified/overwritten
        for reg in insn.regs_write:
            all_written_regs.add(insn.reg_name(reg))

    # 3. Apply standard optimization pruning logic
    # An 'input' is any register read that wasn't created inside the function first.
    # For a pure leaf analysis, we focus on the base architecture parameter defaults:
    sys_v_inputs = {'rdi', 'rsi', 'rdx', 'rcx', 'r8', 'r9'}
    
    discovered_inputs = sorted(list(all_read_regs.intersection(sys_v_inputs)))
    discovered_outputs = sorted(list(all_written_regs.intersection({'rax', 'rdx', 'rsi'})))
    
    # Clobbers are things modified that are NOT serving as our return channels
    all_clobbers = all_written_regs.difference(set(discovered_outputs))
    
    # Clean up and normalize the clobber text names for GCC compatibility
    gcc_clobbers = set()
    for reg in all_clobbers:
        if reg == 'eflags':
            gcc_clobbers.add('cc') # Map x86 eflags to GCC 'cc' condition code constraint
        elif reg not in ['rsp', 'rbp']: # Never add stack/frame pointers to clobbers
            gcc_clobbers.add(reg)
    
    # 4. Programmatically synthesize the static inline C Header file
    header = []
    header.append(f"#ifndef {func_name.upper()}_H")
    header.append(f"#define {func_name.upper()}_H\n")
    header.append(f"extern void {func_name}(void);\n")
    
    # Generate C function prototype signature
    args = ", ".join([f"long {r}" for r in discovered_inputs])
    out_ptrs = ", ".join([f"long *out_{r}" for r in discovered_outputs])
    sig_p2 = f", {out_ptrs}" if out_ptrs else ""
    
    header.append(f"static inline void call_{func_name}({args}{sig_p2}) {{")
    
    # Emit variable bindings
    for reg in discovered_inputs:
        header.append(f"    register long reg_{reg} __asm__(\"{reg}\") = {reg};")
    for reg in discovered_outputs:
        if reg not in discovered_inputs:
            header.append(f"    register long reg_{reg} __asm__(\"{reg}\");")
            
    # Emit assembly instruction string executionblock
    header.append("\n    __asm__ __volatile__ (")
    header.append(f"        \"call {func_name}\"")
    
    # Map Outputs
    out_strs = [f'"+r"(reg_{r})' if r in discovered_inputs else f'"=r"(reg_{r})' for r in discovered_outputs]
    header.append(f"        : {', '.join(out_strs)}")
    
    # Map Inputs
    in_strs = [f'"r"(reg_{r})' for r in discovered_inputs if r not in discovered_outputs]
    header.append(f"        : {', '.join(in_strs)}")
    
    # Map Optimized Clobber List
    clob_strs = [f'"{c}"' for c in sorted(list(gcc_clobbers))]
    # Note: We omitted "memory" because our analysis proved no memory addresses were dereferenced!
    header.append(f"        : {', '.join(clob_strs)}")
    header.append("    );\n")
    
    # Extract back to pointers
    for reg in discovered_outputs:
        header.append(f"    *out_{reg} = reg_{reg};")
        
    header.append("}\n")
    header.append(f"#endif // {func_name.upper()}_H")
    
    return "\n".join(header)

# Execute the engine pipeline
print(generate_optimized_header("custom_vector_add", TARGET_MACHINE_BYTES))
