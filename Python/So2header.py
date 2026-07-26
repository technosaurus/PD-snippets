import sys
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from capstone import *
from capstone.x86 import *

def analyze_so_file(so_path):
    with open(so_path, 'rb') as f:
        elf = ELFFile(f)
        
        # 1. Locate the .text (code) section and Symbol Table
        text_sec = elf.get_section_by_name('.text')
        if not text_sec:
            print("Error: No .text section found.")
            return
            
        text_data = text_sec.data()
        text_base_vaddr = text_sec['sh_addr']
        
        symtab = elf.get_section_by_name('.symtab')
        if not symtab:
            # Fall back to dynamic symbols if stripped
            symtab = elf.get_section_by_name('.dynsym')
            
        if not symtab or not isinstance(symtab, SymbolTableSection):
            print("Error: No symbol table found. Cannot isolate functions.")
            return

        # 2. Iterate through exported function symbols
        for symbol in symtab.iter_symbols():
            # Filter for function symbols with a valid size
            if symbol['st_info']['type'] == 'STT_FUNC' and symbol['st_size'] > 0:
                func_name = symbol.name
                func_vaddr = symbol['st_value']
                func_size = symbol['st_size']
                
                # Calculate file-offset slicing for this specific function
                offset_in_text = func_vaddr - text_base_vaddr
                func_bytes = text_data[offset_in_text : offset_in_text + func_size]
                
                # Analyze the isolated bytes
                generate_wrapper_from_analysis(func_name, func_bytes, func_vaddr)

def generate_wrapper_from_analysis(func_name, machine_code, vaddr):
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True  # Unlock advanced operand metrics

    all_read_regs = set()
    all_written_regs = set()
    has_global_pointer_write = False

    for insn in md.disasm(machine_code, vaddr):
        # Tracking explicit and implicit register changes
        for reg in insn.regs_read:
            all_read_regs.add(insn.reg_name(reg))
        for reg in insn.regs_write:
            all_written_regs.add(insn.reg_name(reg))

        # --- Semantic Operand Analysis for Pointer Writes ---
        # Look for instructions that write to a destination operand containing memory
        if len(insn.operands) > 0:
            dest_op = insn.operands[0]  # Standard Intel/Capstone target operand order
            
            # Check if the destination operand is a memory reference
            if dest_op.type == X86_OP_MEM:
                base_reg = insn.reg_name(dest_op.mem.base)
                
                # Filter out Stack/Frame pointer writes (rsp/rbp are safe internal memory adjustments)
                if base_reg not in ['rsp', 'rbp']:
                    # This instruction actively modifies memory pointed to by a parameter register (e.g., [rdi])
                    has_global_pointer_write = True

    # Standard System V calling constraints mapping
    sys_v_inputs = {'rdi', 'rsi', 'rdx', 'rcx', 'r8', 'r9'}
    discovered_inputs = sorted(list(all_read_regs.intersection(sys_v_inputs)))
    discovered_outputs = sorted(list(all_written_regs.intersection({'rax', 'rdx', 'rsi'})))
    
    # Isolate dirty scratch registers
    all_clobbers = all_written_regs.difference(set(discovered_outputs))
    gcc_clobbers = set()
    for reg in all_clobbers:
        if reg == 'eflags':
            gcc_clobbers.add('cc')
        elif reg not in ['rsp', 'rbp']:
            gcc_clobbers.add(reg)
            
    # Conditional Insertion of the Memory Barrier
    if has_global_pointer_write:
        gcc_clobbers.add('memory')

    # 4. Generate C Header Wrapper Output
    print(f"\n/* --- Generated Wrapper for {func_name} --- */")
    args = ", ".join([f"long {r}" for r in discovered_inputs])
    out_ptrs = ", ".join([f"long *out_{r}" for r in discovered_outputs])
    sig_p2 = f", {out_ptrs}" if out_ptrs else ""
    
    print(f"static inline void call_{func_name}({args}{sig_p2}) {{")
    for reg in discovered_inputs:
        print(f"    register long reg_{reg} __asm__(\"{reg}\") = {reg};")
    for reg in discovered_outputs:
        if reg not in discovered_inputs:
            print(f"    register long reg_{reg} __asm__(\"{reg}\");")
            
    print("    __asm__ __volatile__ (")
    print(f"        \"call {func_name}\"")
    
    out_strs = [f'"+r"(reg_{r})' if r in discovered_inputs else f'"=r"(reg_{r})' for r in discovered_outputs]
    print(f"        : {', '.join(out_strs)}")
    
    in_strs = [f'"r"(reg_{r})' for r in discovered_inputs if r not in discovered_outputs]
    print(f"        : {', '.join(in_strs)}")
    
    clob_strs = [f'"{c}"' for c in sorted(list(gcc_clobbers))]
    print(f"        : {', '.join(clob_strs)}")
    print("    );")
    
    for reg in discovered_outputs:
        print(f"    *out_{reg} = reg_{reg};")
    print("}")

# Example Usage: Pass a path to a compiled shared object library file
# analyze_so_file("libexample.so")
