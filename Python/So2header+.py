from capstone import *
from capstone.x86 import *

def generate_ultimate_optimized_header(func_name, machine_code, vaddr):
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True

    # Dictionary mapping standard System V input channels to their internal aliases
    # e.g., If the code does `mov r10, rdi`, then r10 is the internal alias for rdi
    register_aliases = {}
    
    # Track which registers are ultimately used as the actual input targets
    ultimate_inputs = set()
    all_written_regs = set()
    has_global_pointer_write = False

    instructions = list(md.disasm(machine_code, vaddr))
    
    # PHASE 1: Scan for parameter loading and register shuffling at the prologue
    sys_v_inputs = {'rdi', 'rsi', 'rdx', 'rcx', 'r8', 'r9'}
    
    for insn in instructions[:3]:  # Check the first few instructions for structural shifts
        if insn.id == X86_INS_MOV:
            dest_op = insn.operands[0]
            src_op = insn.operands[1]
            
            if dest_op.type == X86_OP_REG and src_op.type == X86_OP_REG:
                dest_reg = insn.reg_name(dest_op.reg)
                src_reg = insn.reg_name(src_op.reg)
                
                if src_reg in sys_v_inputs:
                    # We caught an optimization opportunity!
                    # The function immediately moves an incoming parameter into a scratch register.
                    register_aliases[src_reg] = dest_reg
                    ultimate_inputs.add(dest_reg)

    # PHASE 2: Standard Liveness & Memory Write Tracking for the remaining block
    for insn in instructions:
        # Ignore the original setup movs when calculating clobbers/inputs
        is_prologue_shuffle = (insn.id == X86_INS_MOV and 
                               insn.reg_name(insn.operands[1].reg) in register_aliases if insn.operands[1].type == X86_OP_REG else False)
        
        if not is_prologue_shuffle:
            for reg in insn.regs_read:
                reg_name = insn.reg_name(reg)
                # If it reads a base register, log it
                if reg_name in sys_v_inputs and reg_name not in register_aliases:
                    ultimate_inputs.add(reg_name)
                    
            for reg in insn.regs_write:
                all_written_regs.add(insn.reg_name(reg))

        # Check for pointer writes
        if len(insn.operands) > 0 and insn.operands[0].type == X86_OP_MEM:
            base_reg = insn.reg_name(insn.operands[0].mem.base)
            if base_reg not in ['rsp', 'rbp']:
                has_global_pointer_write = True

    # Map outputs
    discovered_outputs = sorted(list(all_written_regs.intersection({'rax', 'rdx', 'rsi'})))
    
    # Calculate pure clobbers (Modified registers minus outputs and our newly claimed inputs)
    all_clobbers = all_written_regs.difference(set(discovered_outputs)).difference(ultimate_inputs)
    gcc_clobbers = set()
    for reg in all_clobbers:
        if reg == 'eflags':
            gcc_clobbers.add('cc')
        elif reg not in ['rsp', 'rbp']:
            gcc_clobbers.add(reg)
            
    if has_global_pointer_write:
        gcc_clobbers.add('memory')

    # PHASE 3: Emit the Ultimate Macro Binding
    print(f"\n/* ======================================================== */")
    print(f"/* ULTIMATE OPTIMIZED WRAPPER FOR: {func_name} */")
    print(f"/* ======================================================== */")
    
    # Map the arguments to their direct physical internal target registers
    final_input_list = sorted(list(ultimate_inputs))
    args = ", ".join([f"long param_{r}" for r in final_input_list])
    out_ptrs = ", ".join([f"long *out_{r}" for r in discovered_outputs])
    sig_p2 = f", {out_ptrs}" if out_ptrs else ""
    
    print(f"static inline void call_{func_name}({args}{sig_p2}) {{")
    
    # Tell the compiler to drop variables directly into the destination registers!
    for reg in final_input_list:
        print(f"    register long reg_{reg} __asm__(\"{reg}\") = param_{reg};")
    for reg in discovered_outputs:
        if reg not in final_input_list:
            print(f"    register long reg_{reg} __asm__(\"{reg}\");")
            
    print("    __asm__ __volatile__ (")
    # Instead of calling the function start address, we bypass the prologue shuffle entirely!
    # We add an offset equal to the size of the shuffled instructions we bypassed.
    bypass_offset = len(register_aliases) * 3 # Rough estimate of x86_64 mov reg, reg byte size
    print(f"        \"call {func_name} + {bypass_offset}\"") 
    
    out_strs = [f'"+r"(reg_{r})' if r in final_input_list else f'"=r"(reg_{r})' for r in discovered_outputs]
    print(f"        : {', '.join(out_strs)}")
    
    in_strs = [f'"r"(reg_{r})' for r in final_input_list if r not in discovered_outputs]
    print(f"        : {', '.join(in_strs)}")
    
    print(f"        : {', '.join([f'\"{c}\"' for c in sorted(list(gcc_clobbers))])}")
    print("    );")
    
    for reg in discovered_outputs:
        print(f"    *out_{reg} = reg_{reg};")
    print("}")
    
