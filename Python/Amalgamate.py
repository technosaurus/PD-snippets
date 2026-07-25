import clang.cindex
from collections import defaultdict

# 1. Initialize Clang Compiler API
# Ensure libclang.so / libclang.dylib is in your system path
# clang.cindex.Config.set_library_path("/usr/lib/llvm-14/lib") 

def generate_short_name(index):
    """Generates variable names sequentially: a, b, c... z, aa, ab..."""
    chars = "abcdefghijklmnopqrstuvwxyz"
    result = []
    while index >= 0:
        result.append(chars[index % 26])
        index = (index // 26) - 1
    return "".join(reversed(result))

def rename_local_variables_in_function(function_node, raw_lines):
    """
    Scans a single function implementation, builds a safe local variable map,
    and replaces their occurrences in the source text slice.
    """
    local_var_map = {}
    var_counter = 0

    # Pass 1: Identify all local variables and function parameters
    def find_locals(node):
        nonlocal var_counter
        if node.kind in [clang.cindex.CursorKind.VAR_DECL, clang.cindex.CursorKind.PARM_DECL]:
            # Ensure it is truly a local variable (has no linkage outside the function)
            if node.linkage == clang.cindex.LinkageKind.NO_LINKAGE:
                orig_name = node.spelling
                if orig_name and orig_name not in local_var_map:
                    local_var_map[orig_name] = generate_short_name(var_counter)
                    var_counter += 1
                    
        # Recursively scan children inside this function scope
        for child in node.get_children():
            find_locals(child)

    find_locals(function_node)

    # If the function has no local variables, return its original source text slice
    if not local_var_map:
        return get_clean_node_text(function_node, raw_lines)

    # Pass 2: Reconstruct the text manually using tokens to perform safe replacements
    # Get all lexical tokens (keywords, identifiers, punctuation) for this function
    tokens = list(function_node.get_tokens())
    output_chunks = []
    
    for token in tokens:
        token_text = token.spelling
        
        # Check if this token is a variable name we marked for compression minification
        if token.kind == clang.cindex.TokenKind.IDENTIFIER and token_text in local_var_map:
            # Replace with our minimized variable name
            output_chunks.append(local_var_map[token_text])
        else:
            output_chunks.append(token_text)
            
        # Optional: Add spacing rules so tokens don't merge illegally (e.g., 'int' and 'a' -> 'inta')
        # A simple trailing space for keywords/identifiers prevents compilation merging.
        if token.kind in [clang.cindex.TokenKind.KEYWORD, clang.cindex.TokenKind.IDENTIFIER]:
            output_chunks.append(" ")

    return "".join(output_chunks)
  
def extract_and_sort_c_file(source_file_path):
    index = clang.cindex.Index.create()
    
    # Parse the file. We pass '-fsyntax-only' because we just want the tree structure.
    translation_unit = index.parse(source_file_path, args=['-fsyntax-only'])
    
    # Store source tokens based on their logical category
    buckets = {
        "defines": [],
        "forward_declarations": [],
        "typedefs": [],
        "structs_unions": [],
        "prototypes": [],
        "implementations": []
    }
    
    # Load the raw file lines so we can slice out the exact text chunks
    with open(source_file_path, 'r') as f:
        raw_source_lines = f.readlines()

    def get_raw_node_text(node):
        """Helper to safely slice out the exact code matching an AST node location."""
        start_line = node.extent.start.line - 1
        start_col = node.extent.start.column - 1
        end_line = node.extent.end.line - 1
        end_col = node.extent.end.column - 1
        
        if start_line == end_line:
            return raw_source_lines[start_line][start_col:end_col]
        
        lines = [raw_source_lines[start_line][start_col:]]
        for l in range(start_line + 1, end_line):
            lines.append(raw_source_lines[l])
        lines.append(raw_source_lines[end_line][:end_col])
        return "".join(lines)

    # 2. Traverse the Abstract Syntax Tree (AST)
    for node in translation_unit.cursor.get_children():
        # Crucial: Only process elements native to this specific file, ignoring system headers
        if node.location.file and node.location.file.name != source_file_path:
            continue
            
        node_text = get_raw_node_text(node)
        
        # Categorize nodes into targeted structural blocks
        if node.kind == clang.cindex.CursorKind.TYPEDEF_DECL:
            buckets["typedefs"].append(node_text)
            
        elif node.kind in [clang.cindex.CursorKind.STRUCT_DECL, clang.cindex.CursorKind.UNION_DECL]:
            if node.is_definition():
                buckets["structs_unions"].append(node_text)
            else: # It's an opaque "struct foo;" forward declaration!
                buckets["forward_declarations"].append(node_text)      

        elif node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
            # Differentiate a standalone prototype signature from a full coding block
            if node.is_definition():
                buckets["implementations"].append(node_text)
            else:
                buckets["prototypes"].append(node_text)
                
        elif node.kind == clang.cindex.CursorKind.MACRO_DEFINITION:
            buckets["defines"].append(node_text)

    # 3. Output the sorted document pipeline
    print("/* --- DEFINES --- */")
    print("\n\n".join(buckets["defines"]))
    #TODO Add forward declarations and global variables
    print("\n/* --- TYPEDEFS --- */")
    print("\n\n".join(buckets["typedefs"]))
    
    print("\n/* --- STRUCTS & UNIONS --- */")
    print("\n\n".join(buckets["structs_unions"]))
    
    print("\n/* --- FUNCTION PROTOTYPES --- */")
    print("\n\n".join(buckets["prototypes"]))
    
    print("\n/* --- FUNCTION IMPLEMENTATIONS --- */")
    print("\n\n".join(buckets["implementations"]))

# Execute the partition script against your targeted unifdef'd source
extract_and_sort_c_file("musl_unifdefed_string.c")
  
