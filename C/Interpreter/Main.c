#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Forward declarations for your PackCC-generated parsers
// The prefix strings match the '%prefix' configurations in your .peg files
typedef struct tinyc_context_tag tinyc_context_t;
tinyc_context_t* tinyc_create(void* user_data);
int tinyc_parse(tinyc_context_t* ctx, ASTNode** result);
void tinyc_destroy(tinyc_context_t* ctx);

typedef struct lisp_context_tag lisp_context_t;
lisp_context_t* lisp_create(void* user_data);
int lisp_parse(lisp_context_t* ctx, ASTNode** result);
void lisp_destroy(lisp_context_t* ctx);

typedef struct lua_context_tag lua_context_t;
lua_context_t* lua_create(void* user_data);
int lua_parse(lua_context_t* ctx, ASTNode** result);
void lua_destroy(lua_context_t* ctx);

/* =========================================================================
   1. UTILITY: FILE LOADING INTERFACE
   ========================================================================= */
static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Not enough memory to read \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}


/* =========================================================================
   2. REENTRANT PARSER ROUTERS (PACKCC BRIDGING)
   ========================================================================= */

// PackCC natively looks for text via a custom input stream or character provider.
// This is a minimal wrapper to pass your text buffer context directly into PackCC.
static ASTNode* run_tinyc_parser(const char* source) {
    ASTNode* root_node = NULL;
    
    // Create the PackCC structural parser state context
    // (If using custom streams, pass your custom data structure via the user pointer)
    tinyc_context_t* ctx = tinyc_create((void*)source); 
    
    // Execute the PackCC parsing rules. It builds the AST via ast.c factories.
    if (tinyc_parse(ctx, &root_node) != 0) {
        // Success! Root node is populated by the matched rules
        tinyc_destroy(ctx);
        return root_node;
    }
    
    tinyc_destroy(ctx);
    return NULL;
}

static ASTNode* run_lisp_parser(const char* source) {
    ASTNode* root_node = NULL;
    lisp_context_t* ctx = lisp_create((void*)source);
    
    if (lisp_parse(ctx, &root_node) != 0) {
        lisp_destroy(ctx);
        return root_node;
    }
    
    lisp_destroy(ctx);
    return NULL;
}

static ASTNode* run_lua_parser(const char* source) {
    ASTNode* root_node = NULL;
    lua_context_t* ctx = lua_create((void*)source);
    if (lua_parse(ctx, &root_node) != 0) {
        lua_destroy(ctx);
        return root_node;
    }
    lua_destroy(ctx);
    return NULL;
}

/* =========================================================================
   3. THE MAIN PROGRAM ENTRY POINT
   ========================================================================= */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: polyglot_vm [file_path]\n");
        return EXIT_FAILURE;
    }

    const char* file_path = argv[1];
    
    // 1. Extract the file extension to pick our target language pipeline
    const char* ext = strrchr(file_path, '.');
    if (!ext) {
        fprintf(stderr, "Error: Files must include a valid language extension (e.g., .c, .scm).\n");
        return EXIT_FAILURE;
    }

    // 2. Read the script text code straight into local memory strings
    char* source_code = read_file(file_path);
    ASTNode* program_ast = NULL;

    // 3. Route to the correct PackCC parsing binary
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".tinyc") == 0) {
        program_ast = run_tinyc_parser(source_code);
    } else if (strcmp(ext, ".scm") == 0 || strcmp(ext, ".lisp") == 0) {
        program_ast = run_lisp_parser(source_code);
    } else if (strcmp(ext, ".lua") == 0) { // <-- Added Lua Route
        program_ast = run_lua_parser(source_code);
    } else {
        fprintf(stderr, "Error: Unsupported language target extension '%s'.\n", ext);
        free(source_code);
        return EXIT_FAILURE;
    }

    // Check if the parser encountered syntactical blockers
    if (!program_ast) {
        fprintf(stderr, "Syntax Error: Code failed to compile into a clean AST.\n");
        free(source_code);
        return EXIT_FAILURE;
    }

    // 4. Initialize the Global Virtual Machine Environment Frame
    Value null_val = { VAL_NULL, {0} };
    Environment global_env = { .name = "__global__", .value = null_val, .outer = NULL };

    // 5. Execute! This single loop handles whatever tree was generated upstream.
    printf("--- Executing Program (%s) ---\n", file_path);
    Value execution_result = execute_ast(program_ast, &global_env);
    printf("-------------------------------\n");

    // 6. Deep Memory Garbage Cleanups
    free_value(execution_result);
    free_ast_node(program_ast);
    free(source_code);

    return EXIT_SUCCESS;
}
