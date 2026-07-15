#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* =========================================================================
   0. CONFIGURATION MACROS
   ========================================================================= */
#define INITIAL_BLOCK_CAPACITY 4
#define INITIAL_CALL_CAPACITY 4


/* =========================================================================
   1. INTERNAL MEMORY ALLOCATION HELPERS
   ========================================================================= */

/**
 * Safely allocates memory, exiting on OOM.
 * @param size: Bytes to allocate
 * @return: Pointer to allocated memory (never NULL)
 */
static void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: Out of memory allocating %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/**
 * Safely duplicates a string, exiting on OOM.
 * @param str: String to duplicate (may be NULL, returns NULL)
 * @return: Duplicated string (never NULL unless input is NULL)
 */
static char* safe_strdup(const char* str) {
    if (!str) return NULL;
    char* dup = strdup(str);
    if (!dup) {
        fprintf(stderr, "Error: Out of memory duplicating string.\n");
        exit(EXIT_FAILURE);
    }
    return dup;
}

/**
 * Allocates and initializes an AST node of the given type.
 * The union data is zeroed to prevent uninitialized reads.
 * @param type: NodeType to create
 * @return: Initialized ASTNode (never NULL)
 */
static ASTNode* allocate_node(NodeType type) {
    ASTNode* node = (ASTNode*)safe_malloc(sizeof(ASTNode));
    node->type = type;
    memset(&node->data, 0, sizeof(node->data)); // Zero out the variant union
    return node;
}


/* =========================================================================
   2. FACTORY FUNCTIONS (CONSTRUCTORS)
   ========================================================================= */

/**
 * Creates the root node representing the entire program.
 * If init or body are NULL, empty block nodes are created.
 */
ASTNode* create_program_root(ASTNode* init, ASTNode* body) {
    ASTNode* node = allocate_node(NODE_PROGRAM_ROOT);
    node->data.root.init_block = init ? init : create_block_node();
    node->data.root.body_block = body ? body : create_block_node();
    return node;
}

/**
 * Creates a literal node (constant value: number, string, boolean, etc).
 * For string literals, duplicates the string to ensure AST ownership.
 */
ASTNode* create_literal_node(Value val) {
    ASTNode* node = allocate_node(NODE_LITERAL);
    node->data.literal = val;
    // If it's a string literal, duplicate it so the AST owns its memory safely
    if (val.type == VAL_STRING && val.as.s_val != NULL) {
        node->data.literal.as.s_val = safe_strdup(val.as.s_val);
    }
    return node;
}

/**
 * Creates an identifier node (variable reference).
 */
ASTNode* create_identifier_node(const char* name) {
    ASTNode* node = allocate_node(NODE_IDENTIFIER);
    node->data.id_name = safe_strdup(name);
    return node;
}

/**
 * Creates a unary operator node (e.g., -x, !flag).
 */
ASTNode* create_unary_node(char op, ASTNode* operand) {
    ASTNode* node = allocate_node(NODE_UNARY_OP);
    node->data.unary.op = op;
    node->data.unary.operand = operand;
    return node;
}

/**
 * Creates a binary operator node (e.g., x + y, a == b).
 */
ASTNode* create_binary_node(char op, ASTNode* left, ASTNode* right) {
    ASTNode* node = allocate_node(NODE_BINARY_OP);
    node->data.binary.op = op;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

/**
 * Creates a logical operator node (AND/OR).
 */
ASTNode* create_logical_node(const char* type_str, ASTNode* left, ASTNode* right) {
    ASTNode* node = allocate_node(NODE_LOGICAL_OP);
    node->data.logical.type_str = safe_strdup(type_str);
    node->data.logical.left = left;
    node->data.logical.right = right;
    return node;
}

/**
 * Creates an assignment node (e.g., x = 5).
 */
ASTNode* create_assignment_node(const char* name, ASTNode* value) {
    ASTNode* node = allocate_node(NODE_ASSIGNMENT);
    node->data.assign.name = safe_strdup(name);
    node->data.assign.value = value;
    return node;
}

/**
 * Creates an if/else statement node.
 * else_b may be NULL for if-only statements.
 */
ASTNode* create_if_node(ASTNode* cond, ASTNode* then_b, ASTNode* else_b) {
    ASTNode* node = allocate_node(NODE_IF_STATEMENT);
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_b;
    node->data.if_stmt.else_branch = else_b;
    return node;
}

/**
 * Creates a while loop node.
 */
ASTNode* create_while_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = allocate_node(NODE_WHILE_LOOP);
    node->data.while_loop.condition = cond;
    node->data.while_loop.body = body;
    return node;
}

/**
 * Creates a block node (sequence of statements).
 * Blocks are growable with dynamic reallocation.
 */
ASTNode* create_block_node(void) {
    ASTNode* node = allocate_node(NODE_BLOCK);
    node->data.block.count = 0;
    node->data.block.capacity = INITIAL_BLOCK_CAPACITY;
    node->data.block.statements = (ASTNode**)safe_malloc(
        sizeof(ASTNode*) * INITIAL_BLOCK_CAPACITY
    );
    return node;
}

/**
 * Creates a function call node with the given function name.
 * Arguments are added via append_argument().
 */
ASTNode* create_call_node(const char* name) {
    ASTNode* node = allocate_node(NODE_FUNCTION_CALL);
    node->data.call.name = safe_strdup(name);
    node->data.call.arg_count = 0;
    node->data.call.arg_capacity = INITIAL_CALL_CAPACITY;
    node->data.call.arguments = (ASTNode**)safe_malloc(
        sizeof(ASTNode*) * INITIAL_CALL_CAPACITY
    );
    return node;
}

/**
 * Creates an index node (array/object access, e.g., arr[i]).
 */
ASTNode* create_index_node(ASTNode* target, ASTNode* index) {
    ASTNode* node = allocate_node(NODE_INDEX);
    node->data.index.target = target;
    node->data.index.index = index;
    return node;
}

/**
 * Appends an argument to a function call node.
 * Dynamically grows the arguments array as needed.
 */
void append_argument(ASTNode* call_node, ASTNode* arg) {
    if (!call_node || call_node->type != NODE_FUNCTION_CALL || !arg) return;

    // Grow capacity if needed
    if (call_node->data.call.arg_count >= call_node->data.call.arg_capacity) {
        call_node->data.call.arg_capacity *= 2;
        ASTNode** new_args = (ASTNode**)realloc(
            call_node->data.call.arguments,
            sizeof(ASTNode*) * call_node->data.call.arg_capacity
        );
        if (!new_args) {
            fprintf(stderr, "Error: Out of memory resizing function arguments (capacity: %d).\n",
                    call_node->data.call.arg_capacity);
            exit(EXIT_FAILURE);
        }
        call_node->data.call.arguments = new_args;
    }

    call_node->data.call.arguments[call_node->data.call.arg_count++] = arg;
}

/**
 * Appends a statement to a block node.
 * Dynamically grows the statements array as needed.
 */
void append_to_block(ASTNode* block_node, ASTNode* statement) {
    if (!block_node || block_node->type != NODE_BLOCK || !statement) return;

    // Dynamically grow the capacity if full
    if (block_node->data.block.count >= block_node->data.block.capacity) {
        block_node->data.block.capacity *= 2;
        ASTNode** new_statements = (ASTNode**)realloc(
            block_node->data.block.statements, 
            sizeof(ASTNode*) * block_node->data.block.capacity
        );
        if (!new_statements) {
            fprintf(stderr, "Error: Out of memory resizing AST block (capacity: %d).\n",
                    block_node->data.block.capacity);
            exit(EXIT_FAILURE);
        }
        block_node->data.block.statements = new_statements;
    }

    block_node->data.block.statements[block_node->data.block.count++] = statement;
}

/**
 * Creates a print statement node.
 */
ASTNode* create_print_node(ASTNode* target) {
    ASTNode* node = allocate_node(NODE_PRINT);
    node->data.print_target = target;
    return node;
}


/* =========================================================================
   3. MEMORY DEALLOCATION (DEEP CLEANUP)
   ========================================================================= */

/**
 * Frees a Value, handling string and array cleanup.
 * Validates array bounds before freeing to catch corruption.
 */
void free_value(const Value val) {
    if (val.type == VAL_STRING && val.as.s_val != NULL) {
        free(val.as.s_val);
    }
    if (val.type == VAL_ARRAY) {
        // Validate array count to prevent buffer over-read
        if (val.as.array.count < 0 || val.as.array.count > val.as.array.capacity) {
            fprintf(stderr, "Warning: Invalid array count (%d, capacity: %d) in free_value. Skipping element cleanup.\n",
                    val.as.array.count, val.as.array.capacity);
        } else {
            for (int i = 0; i < val.as.array.count; i++) {
                free_value(val.as.array.elements[i]);
            }
        }
        free(val.as.array.elements);
    }
}

/**
 * Recursively frees an AST node and all its children.
 * Handles all node types, ordered to match the NodeType enum.
 */
void free_ast_node(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        // Leaves / Literals
        case NODE_LITERAL:
            free_value(node->data.literal);
            break;

        case NODE_IDENTIFIER:
            free(node->data.id_name);
            break;

        // Expressions
        case NODE_BINARY_OP:
            free_ast_node(node->data.binary.left);
            free_ast_node(node->data.binary.right);
            break;

        case NODE_UNARY_OP:
            free_ast_node(node->data.unary.operand);
            break;

        case NODE_LOGICAL_OP:
            free(node->data.logical.type_str);
            free_ast_node(node->data.logical.left);
            free_ast_node(node->data.logical.right);
            break;

        case NODE_ASSIGNMENT:
            free(node->data.assign.name);
            free_ast_node(node->data.assign.value);
            break;

        // Statements & Control Flow
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                free_ast_node(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;

        case NODE_IF_STATEMENT:
            free_ast_node(node->data.if_stmt.condition);
            free_ast_node(node->data.if_stmt.then_branch);
            free_ast_node(node->data.if_stmt.else_branch);
            break;

        case NODE_WHILE_LOOP:
            free_ast_node(node->data.while_loop.condition);
            free_ast_node(node->data.while_loop.body);
            break;

        case NODE_PRINT:
            free_ast_node(node->data.print_target);
            break;

        case NODE_FUNCTION_CALL:
            free(node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                free_ast_node(node->data.call.arguments[i]);
            }
            free(node->data.call.arguments);
            break;

        case NODE_INDEX:
            free_ast_node(node->data.index.target);
            free_ast_node(node->data.index.index);
            break;

        case NODE_PROGRAM_ROOT:
            free_ast_node(node->data.root.init_block);
            free_ast_node(node->data.root.body_block);
            break;

        default:
            fprintf(stderr, "Warning: Unknown node type (%d) in free_ast_node. Possible enum mismatch.\n",
                    node->type);
            break;
    }

    free(node);
}
