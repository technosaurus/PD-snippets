#ifndef AST_H
#define AST_H

#include <stdbool.h>

/* =========================================================================
   1. UNIFIED DYNAMIC VALUE VARIANT
   ========================================================================= */
typedef enum {
    VAL_NULL,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_OBJECT // For extensions (arrays, dicts, etc..
} ValueType;

struct ASTNode;

typedef struct {
    char** parameters; // Array of parameter name strings
    int param_count;
    struct ASTNode* body;  // Point to a NODE_BLOCK statement list
} FunctionObj;

typedef struct {
    ValueType type;
    union {
        bool b_val;
        int i_val;
        double f_val;
        char* s_val; // Dynamically allocated or literal string
        FunctionObj func;
    } as;
} Value;


/* =========================================================================
   2. AST NODE TYPES
   ========================================================================= */
typedef enum {
    // Leaves / Literals
    NODE_LITERAL,
    NODE_IDENTIFIER,

    // Expressions
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGNMENT,

    // Statements & Control Flow
    NODE_BLOCK,
    NODE_IF_STATEMENT,
    NODE_WHILE_LOOP,
    NODE_PRINT,
    NODE_FUNCTION_CALL
} NodeType;


/* =========================================================================
   3. ABSTRACT SYNTAX TREE STRUCTURE
   ========================================================================= */
typedef struct ASTNode {
    NodeType type;
    union {
        // NODE_LITERAL: Constant values (e.g., 42, "hello", true)
        Value literal;

        // NODE_IDENTIFIER: Variable names (e.g., total, x, user_name)
        char* id_name;

        // NODE_UNARY_OP: e.g., !x, -y
        struct {
            char op;
            struct ASTNode* operand;
        } unary;

        // NODE_BINARY_OP: e.g., x + y, a == b
        struct {
            char op; // '+', '-', '*', '/', '=', '>', '<'
            struct ASTNode* left;
            struct ASTNode* right;
        } binary;

        // NODE_ASSIGNMENT: e.g., x = 5
        struct {
            char* name;
            struct ASTNode* value;
        } assign;

        // NODE_IF_STATEMENT: Unified abstraction for if/elif/else chains
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch; // Can be NULL
        } if_stmt;

        // NODE_WHILE_LOOP: Unified representation of loops
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_loop;

        // NODE_BLOCK: Sequence of statements (JS {} or Python indentation blocks)
        struct {
            struct ASTNode** statements; // Dynamic array of ASTNode pointers
            int count;
            int capacity;
        } block;

        // NODE_PRINT: Standard language feature translation target
        struct ASTNode* print_target;

        struct {
            char* name;
            struct ASTNode** arguments; // Dynamic array of ASTNode expression trees
            int arg_count;
        } call;

    } data;
} ASTNode;


/* =========================================================================
   4. MEMORY MANAGEMENT & FACTORY FUNCTIONS
   ========================================================================= */

// Literal / Value Node Constructors
ASTNode* create_literal_node(Value val);
ASTNode* create_identifier_node(const char* name);

// Operator Node Constructors
ASTNode* create_unary_node(char op, ASTNode* operand);
ASTNode* create_binary_node(char op, ASTNode* left, ASTNode* right);
ASTNode* create_assignment_node(const char* name, ASTNode* value);

// Control Flow Constructors
ASTNode* create_if_node(ASTNode* cond, ASTNode* then_b, ASTNode* else_b);
ASTNode* create_while_node(ASTNode* cond, ASTNode* body);

// Block Management (Statements array)
ASTNode* create_block_node(void);
void append_to_block(ASTNode* block_node, ASTNode* statement);

// Helper Factory for Print Statement Translation
ASTNode* create_print_node(ASTNode* target);

ASTNode* create_call_node(const char* name);
void append_argument(ASTNode* call_node, ASTNode* arg);

// Deep Garbage Collection
void free_ast_node(ASTNode* node);
void free_value(Value val);
/* =========================================================================
   ADDITIONS TO ast.h
   ========================================================================= */

// A variable entry in our environment chain
typedef struct Environment {
    const char* name;
    Value value;
    struct Environment* outer; // Link to parent scope (null if global)
} Environment;

// Main entry point for the VM engine
Value execute_ast(ASTNode* node, Environment* env);

#endif // AST_H
