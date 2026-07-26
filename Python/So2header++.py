# script: generate_optimized_headers.py
import sys
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from capstone import *
from capstone.x86 import *

def analyze_and_emit(so_path, out_header_path):
    with open(so_path, 'rb') as f:
        elf = ELFFile(f)
        text_sec = elf.get_section_by_name('.text')
        symtab = elf.get_section_by_name('.symtab') or elf.get_section_by_name('.dynsym')
        
        if not text_sec or not symtab:
            return

        text_data = text_sec.data()
        text_base_vaddr = text_sec['sh_addr']
        
        with open(out_header_path, 'w') as h_out:
            h_out.write("#ifndef AUTO_OPTIMIZED_BOUNDARIES_H\n")
            h_out.write("#define AUTO_OPTIMIZED_BOUNDARIES_H\n\n")
            
            for symbol in symtab.iter_symbols():
                if symbol['st_info']['type'] == 'STT_FUNC' and symbol['st_size'] > 0:
                    process_function(symbol.name, text_data, text_base_vaddr, symbol['st_value'], symbol['st_size'], h_out)
            
            h_out.write("#endif\n")

def process_function(name, text_data, base_vaddr, f_vaddr, size, h_out):
    offset = f_vaddr - base_vaddr
    machine_code = text_data[offset : offset + size]
    
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    
    register_aliases = {}
    stack_args_found = {} # Format: {stack_offset: temp_register_to_use}
    ultimate_inputs = set()
    all_written_regs = set()
    has_global_write = False
    instructions = list(md.disasm(machine_code, f_vaddr))
    
    sys_v_inputs = {'rdi', 'rsi', 'rdx', 'rcx', 'r8', 'r9'}
    # Available high scratch registers we can borrow to pass stack parameters safely inside the wrapper
    available_scratch = ['r10', 'r11', 'r12', 'r13'] 
    
    # 1. Look for Register Shuffles AND Stack Parameter Loads in the prologue
    prologue_size = 0
    for insn in instructions[:6]: 
        # Match Register Shuffling: mov dest_reg, src_reg
        if insn.id == X86_INS_MOV and insn.operands[0].type == X86_OP_REG and insn.operands[1].type == X86_OP_REG:
            dest_reg = insn.reg_name(insn.operands[0].reg)
            src_reg = insn.reg_name(insn.operands[1].reg)
            if src_reg in sys_v_inputs:
                register_aliases[src_reg] = dest_reg
                ultimate_inputs.add(dest_reg)
                prologue_size += insn.size
                
        # Match Stack Loads: mov dest_reg, [rsp + offset]
        elif insn.id == X86_INS_MOV and insn.operands[0].type == X86_OP_REG and insn.operands[1].type == X86_OP_MEM:
            dest_reg = insn.reg_name(insn.operands[0].reg)
            mem_op = insn.operands[1].mem
            base_reg = insn.reg_name(mem_op.base)
            
            if base_reg in ['rsp', 'rbp'] and mem_op.disp >= 16: # Beyond standard return address frames
                stack_offset = mem_op.disp
                if available_scratch:
                    assigned_reg = available_scratch.pop(0)
                    stack_args_found[stack_offset] = (assigned_reg, dest_reg)
                    ultimate_inputs.add(assigned_reg)
                    ultimate_inputs.add(dest_reg)
                    prologue_size += insn.size

    # 2. General Register and Pointer write extraction
    for insn in instructions:
        for reg in insn.regs_write:
            all_written_regs.add(insn.reg_name(reg))
        if len(insn.operands) > 0 and insn.operands[0].type == X86_OP_MEM:
            if insn.reg_name(insn.operands[0].mem.base) not in ['rsp', 'rbp']:
                has_global_write = True

    discovered_outputs = sorted(list(all_written_regs.intersection({'rax', 'rdx', 'rsi'})))
    all_clobbers = all_written_regs.difference(set(discovered_outputs)).difference(ultimate_inputs)
    gcc_clobbers = {insn.reg_name(r) for r in all_written_regs if insn.reg_name(r) == 'eflags'}
    gcc_clobbers = {'cc'} if 'eflags' in all_written_regs else set()
    if has_global_write: gcc_clobbers.add('memory')

    # 3. Code Generation to Header File
    h_out.write(f"/* Highly Optimized Boundary Mapping for {name} */\n")
    final_inputs = sorted(list(ultimate_inputs))
    args = ", ".join([f"long param_{r}" for r in final_inputs])
    out_ptrs = ", ".join([f"long *out_{r}" for r in discovered_outputs])
    sig = f"{args}, {out_ptrs}" if out_ptrs else args
    
    h_out.write(f"static inline void call_{name}({sig}) {{\n")
    for reg in final_inputs:
        h_out.write(f"    register long reg_{reg} __asm__(\"{reg}\") = param_{reg};\n")
    for reg in discovered_outputs:
        if reg not in final_inputs:
            h_out.write(f"    register long reg_{reg} __asm__(\"{reg}\");\n")
            
    h_out.write("    __asm__ __volatile__ (\n")
    
    # Pre-inject any needed stack parameters before execution jumps past the prologue
    for offset, (scratch, dest) in stack_args_found.items():
        h_out.write(f"        \"pushq %{scratch}\\n\\t\" // Setup stack argument dynamically\n")
        
    h_out.write(f"        \"call {name} + {prologue_size}\\n\\t\"\n")
    
    # Balance the stack pointers clean up if we added arguments
    if stack_args_found:
        h_out.write(f"        \"addq ${len(stack_args_found)*8}, %%rsp\\n\\t\"\n")
        
    out_strs = [f'"+r"(reg_{r})' if r in final_inputs else f'"=r"(reg_{r})' for r in discovered_outputs]
    in_strs = [f'"r"(reg_{r})' for r in final_inputs if r not in discovered_outputs]
    clob_strs = [f'\"{c}\"' for c in sorted(list(gcc_clobbers))]
    
    h_out.write(f"        : {', '.join(out_strs)}\n")
    h_out.write(f"        : {', '.join(in_strs)}\n")
    h_out.write(f"        : {', '.join(clob_strs)}\n")
    h_out.write("    );\n")
    
    for reg in discovered_outputs:
        h_out.write(f"    *out_{reg} = reg_{reg};\n")
    h_out.write("}\n\n")

if __name__ == '__main__':
    analyze_and_emit(sys.argv[1], sys.argv[2])
  
