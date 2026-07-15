#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "tinyc.h"
#include "lisp.h"

ASTNode* parse_source(const char* filename, const char* code) {
    // 1. Detect extension
    const char *ext = strrchr(filename, '.');
    
    // 2. Initialize PackCC contexts dynamically
    if (ext && strcmp(ext, ".c") == 0) {
        tinyc_context_t *ctx = tinyc_create(NULL);
        // Assuming you write a string stream adapter for PackCC
        // ASTNode* node = tinyc_parse(ctx, ...); 
        // return node;
    } 
    else if (ext && strcmp(ext, ".scm") == 0) {
        lisp_context_t *ctx = lisp_create(NULL);
        // ...
    }
    
    // 3. Fallback: Quick peek inside the text for scripting languages
    if (strncmp(code, "(", 1) == 0) {
        // Looks like Lisp code structure, route to Lisp parser
    }

    return NULL;
}
