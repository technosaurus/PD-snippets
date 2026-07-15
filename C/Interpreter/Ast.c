#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* =========================================================================
   1. INTERNAL MEMORY ALLOCATION HELPER
   ========================================================================= */
static void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: Out of memory in AST allocator.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static ASTNode* allocate_node(NodeType type) {
    ASTNode* node = (ASTNode*)safe_malloc(sizeof(ASTNode));
    node->type = type;
    memset(&node->data, 0, sizeof(node->data)); // Zero out the variant union
    return node;
}


/* =========================================================================
   2. FACTORY FUNCTIONS (CONSTRUCTORS)
   ========================================================================= */

ASTNode* create_literal_node(Value val) {
    ASTNode* node = allocate_node(NODE_LITERAL);
    node->data.literal = val;
    // If it's a string literal, duplicate it so the AST owns its memory safely
    if (val.type == VAL_STRING && val.as.s_val != NULL) {
        node->data.literal.as.s_val = strdup(val.as.s_val);
    }
    return node;
}

ASTNode* create_identifier_node(const char* name) {
    ASTNode* node = allocate_node(NODE_IDENTIFIER);
    node->data.id_name = strdup(name);
    return node;
}

ASTNode* create_unary_node(char op, ASTNode* operand) {
    ASTNode* node = allocate_node(NODE_UNARY_OP);
    node->data.unary.op = op;
    node->data.unary.operand = operand;
    return node;
}

ASTNode* create_binary_node(char op, ASTNode* left, ASTNode* right) {
    ASTNode* node = allocate_node(NODE_BINARY_OP);
    node->data.binary.op = op;
    node->data.binary.left = left;
    node->data.binary.right = right;
    return node;
}

ASTNode* create_logical_node(const char* type_str, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_LOGICAL_OP;
    node->data.logical.type_str = strdup(type_str);
    node->data.logical.left = left;
    node->data.logical.right = right;
    return node;
}

ASTNode* create_assignment_node(const char* name, ASTNode* value) {
    ASTNode* node = allocate_node(NODE_ASSIGNMENT);
    node->data.assign.name = strdup(name);
    node->data.assign.value = value;
    return node;
}

ASTNode* create_if_node(ASTNode* cond, ASTNode* then_b, ASTNode* else_b) {
    ASTNode* node = allocate_node(NODE_IF_STATEMENT);
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_b;
    node->data.if_stmt.else_branch = else_b;
    return node;
}

ASTNode* create_while_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = allocate_node(NODE_WHILE_LOOP);
    node->data.while_loop.condition = cond;
    node->data.while_loop.body = body;
    return node;
}

ASTNode* create_block_node(void) {
    ASTNode* node = allocate_node(NODE_BLOCK);
    node->data.block.count = 0;
    node->data.block.capacity = 4; // Start small for micro-environments
    node->data.block.statements = (ASTNode**)safe_malloc(sizeof(ASTNode*) * node->data.block.capacity);
    return node;
}

ASTNode* create_call_node(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->data.call.name = strdup(name);
    node->data.call.arg_count = 0;
    node->data.call.arguments = (ASTNode**)malloc(sizeof(ASTNode*) * 4); // Initial capacity
    return node;
}

ASTNode* create_index_node(ASTNode* target, ASTNode* index) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_INDEX;
    node->data.index.target = target;
    node->data.index.index = index;
    return node;
}

void append_argument(ASTNode* call_node, ASTNode* arg) {
    if (!call_node || call_node->type != NODE_FUNCTION_CALL || !arg) return;
    int index = call_node->data.call.arg_count++;
    // Basic growable logic simplified for clarity
    call_node->data.call.arguments = (ASTNode**)realloc(
        call_node->data.call.arguments, sizeof(ASTNode*) * (index + 1));
    call_node->data.call.arguments[index] = arg;
}

void append_to_block(ASTNode* block_node, ASTNode* statement) {
    if (!block_node || block_node->type != NODE_BLOCK || !statement) return;

    // Dynamically grow the capacity if full
    if (block_node->data.block.count >= block_node->data.block.capacity) {
        block_node->data.block.capacity *= 2;
        block_node->data.block.statements = (ASTNode**)realloc(
            block_node->data.block.statements, 
            sizeof(ASTNode*) * block_node->data.block.capacity
        );
        if (!block_node->data.block.statements) {
            fprintf(stderr, "Error: Out of memory resizing AST block.\n");
            exit(EXIT_FAILURE);
        }
    }

    block_node->data.block.statements[block_node->data.block.count++] = statement;
}

ASTNode* create_print_node(ASTNode* target) {
    ASTNode* node = allocate_node(NODE_PRINT);
    node->data.print_target = target;
    return node;
}


/* =========================================================================
   3. MEMORY DEALLOCATION (DEEP CLEANUP)
   ========================================================================= */

void free_value(Value val) {
    if (val.type == VAL_STRING && val.as.s_val != NULL) {
        free(val.as.s_val);
    }
    if (val.type == VAL_ARRAY) {
        for (int i = 0; i < val.as.array.count; i++) {
            free_value(val.as.array.elements[i]);
        }
        free(val.as.array.elements);
    }
}

void free_ast_node(ASTNode* node) {
    if (!node) return;

    switch (node->type) { // todo reorder in enum order
        case NODE_LITERAL:
            free_value(node->data.literal);
            break;

        case NODE_IDENTIFIER:
            free(node->data.id_name);
            break;

        case NODE_UNARY_OP:
            free_ast_node(node->data.unary.operand);
            break;

        case NODE_BINARY_OP:
            free_ast_node(node->data.binary.left);
            free_ast_node(node->data.binary.right);
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

        case NODE_IF_STATEMENT:
            free_ast_node(node->data.if_stmt.condition);
            free_ast_node(node->data.if_stmt.then_branch);
            free_ast_node(node->data.if_stmt.else_branch);
            break;

        case NODE_WHILE_LOOP:
            free_ast_node(node->data.while_loop.condition);
            free_ast_node(node->data.while_loop.body);
            break;

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                free_ast_node(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
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

    }

    free(node);
}
