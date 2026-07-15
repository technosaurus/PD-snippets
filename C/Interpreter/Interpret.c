#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* =========================================================================
   1. RUNTIME SCOPING HELPERS
   ========================================================================= */

// Lookup a variable by name climbing up the scope chain
static Environment* env_find(Environment* env, const char* name) {
    Environment* current = env;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->outer;
    }
    return NULL;
}

// Update an existing variable, or crash gracefully if it doesn't exist
static void env_assign(Environment* env, const char* name, Value val) {
    Environment* target = env_find(env, name);
    if (!target) {
        fprintf(stderr, "Runtime Error: Undefined variable '%s'\n", name);
        exit(EXIT_FAILURE);
    }
    // Free old string memory if overwriting a string value
    free_value(target->value);
    
    target->value = val;
    // If it's a string, copy the buffer contents safely
    if (val.type == VAL_STRING && val.as.s_val) {
        target->value.as.s_val = strdup(val.as.s_val);
    }
}


/* =========================================================================
   2. OPERATOR COERCION UTILITIES
   ========================================================================= */

// Polyglot truthiness helper (e.g., handles JS/Python/C rules for 'true')
static bool is_truthy(Value val) {
    switch (val.type) {
        case VAL_NULL:   return false;
        case VAL_BOOL:   return val.as.b_val;
        case VAL_INT:    return val.as.i_val != 0;
        case VAL_FLOAT:  return val.as.f_val != 0.0;
        case VAL_STRING: return val.as.s_val != NULL && strlen(val.as.s_val) > 0;
        default:         return false;
    }
}

// Universal numeric and string addition engine
static Value eval_add(Value left, Value right) {
    Value res;
    
    // 1. Pure Integer Math
    if (left.type == VAL_INT && right.type == VAL_INT) {
        res.type = VAL_INT;
        res.as.i_val = left.as.i_val + right.as.i_val;
        return res;
    }
    
    // 2. Coerce to Float Math
    if ((left.type == VAL_INT || left.type == VAL_FLOAT) && 
        (right.type == VAL_INT || right.type == VAL_FLOAT)) {
        double l = (left.type == VAL_INT) ? left.as.i_val : left.as.f_val;
        double r = (right.type == VAL_INT) ? right.as.i_val : right.as.f_val;
        res.type = VAL_FLOAT;
        res.as.f_val = l + r;
        return res;
    }

    // 3. Polyglot String Concatenation (JS / Shell behavior)
    if (left.type == VAL_STRING || right.type == VAL_STRING) {
        res.type = VAL_STRING;
        char l_buf[64] = "";
        char r_buf[64] = "";
        
        // Quick formats to flat strings
        if (left.type == VAL_INT) sprintf(l_buf, "%d", left.as.i_val);
        else if (left.type == VAL_STRING) strcpy(l_buf, left.as.s_val ? left.as.s_val : "");
        
        if (right.type == VAL_INT) sprintf(r_buf, "%d", right.as.i_val);
        else if (right.type == VAL_STRING) strcpy(r_buf, right.as.s_val ? right.as.s_val : "");
        
        res.as.s_val = malloc(strlen(l_buf) + strlen(r_buf) + 1);
        sprintf(res.as.s_val, "%s%s", l_buf, r_buf);
        return res;
    }

    if (left.type == VAL_ARRAY) {
        // Grow runtime vector metrics dynamically
        if (left.as.array.count >= left.as.array.capacity) {
            left.as.array.capacity = left.as.array.capacity == 0 ? 4 : left.as.array.capacity * 2;
            left.as.array.elements = realloc(left.as.array.elements, sizeof(Value) * left.as.array.capacity);
        }
        // Append right item values to target elements array directly
        left.as.array.elements[left.as.array.count++] = right; 
        return left; 
    }

    res.type = VAL_NULL;
    return res;
}

// Explicit runner for stream-based architectures like AWK
void execute_awk_stream(ASTNode* program_ast, Environment* global_env, const char* data_file_path) {
    FILE* stream = fopen(data_file_path, "r");
    if (!stream) {
        fprintf(stderr, "Runtime Error: Could not open data stream file '%s'\n", data_file_path);
        exit(EXIT_FAILURE);
    }

    char line_buffer[1024];
    int line_number = 0;

    // Initialize the line-counter environment variable frame
    Value nr_val = { VAL_INT, .as.i_val = 0 };
    Environment nr_env = { .name = "NR", .value = nr_val, .outer = global_env->outer };
    global_env->outer = &nr_env; // Inject NR into the active lookup scope

    // Also support $0 to reference the entire raw record line string
    Value record_val = { VAL_STRING, .as.s_val = NULL };
    Environment record_env = { .name = "record", .value = record_val, .outer = global_env->outer };
    global_env->outer = &record_env;

    // Stream Loop: Read line by line exactly like native AWK
    while (fgets(line_buffer, sizeof(line_buffer), stream)) {
        line_number++;
        
        // Strip trailing newline character
        line_buffer[strcspn(line_buffer, "\n")] = 0;

        // Update the automatic loop counter state variables
        nr_env.value.as.i_val = line_number;
        
        if (record_env.value.as.s_val) free(record_env.value.as.s_val);
        record_env.value.as.s_val = strdup(line_buffer);

        // Execute the entire AST block against the updated line data context
        Value iteration_result = execute_ast(program_ast, global_env);
        free_value(iteration_result);
    }

    // Clean up data stream hooks
    free(record_env.value.as.s_val);
    fclose(stream);
}

/* =========================================================================
   3. THE MAIN AST VISITOR LOOP
   ========================================================================= */

Value execute_ast(ASTNode* node, Environment* env) {
    Value null_val = { VAL_NULL, {0} };
    if (!node) return null_val;

    switch (node->type) { //todo reorder to enum for jump table
        
        case NODE_LITERAL: {
            // Return literals instantly. Strings are cloned so the pipeline remains pure.
            Value res = node->data.literal;
            if (res.type == VAL_STRING && res.as.s_val) {
                res.as.s_val = strdup(res.as.s_val);
            }
            return res;
        }

        case NODE_IDENTIFIER: {
            Environment* found = env_find(env, node->data.id_name);
            if (!found) {
                fprintf(stderr, "Runtime Error: Variable '%s' is not defined.\n", node->data.id_name);
                exit(EXIT_FAILURE);
            }
            // Clone string variables to avoid multiple references to the same buffer
            Value res = found->value;
            if (res.type == VAL_STRING && res.as.s_val) {
                res.as.s_val = strdup(res.as.s_val);
            }
            return res;
        }

        case NODE_BINARY_OP: {
            Value left = execute_ast(node->data.binary.left, env);
            Value right = execute_ast(node->data.binary.right, env);
            Value res = null_val;

            if (node->data.binary.op == '+') {
                res = eval_add(left, right);
            } else if (node->data.binary.op == '-') {
                if (left.type == VAL_INT && right.type == VAL_INT) {
                    res.type = VAL_INT;
                    res.as.i_val = left.as.i_val - right.as.i_val;
                }
            } else if (node->data.binary.op == '<' || node->data.binary.op == '>' || node->data.binary.op == '=') {
                // Unify numbers to floats for cross-type comparison (e.g. 5 < 5.5)
                double l = (left.type == VAL_INT) ? left.as.i_val : (left.type == VAL_FLOAT ? left.as.f_val : 0.0);
                double r = (right.type == VAL_INT) ? right.as.i_val : (right.type == VAL_FLOAT ? right.as.f_val : 0.0);
                
                res.type = VAL_BOOL;
                if (node->data.binary.op == '<')  res.as.b_val = (l < r);
                if (node->data.binary.op == '>')  res.as.b_val = (l > r);
                if (node->data.binary.op == '=')  res.as.b_val = (l == r); // Matches internal symbol encoding
            }
            
            // Clean up temporary tree evaluation branches
            free_value(left);
            free_value(right);
            return res;
        }

        case NODE_UNARY_OP: {
            Value operand = execute_ast(node->data.unary.operand, env);
            Value res = null_val;
            if (node->data.unary.op == '!') { // Logical NOT
                res.type = VAL_BOOL;
                res.as.b_val = !is_truthy(operand);
            }
            free_value(operand);
            return res;
        }

        case NODE_LOGICAL_OP: {
            Value left = execute_ast(node->data.logical.left, env);
            bool left_truth = is_truthy(left);

            if (strcmp(node->data.logical.type_str, "and") == 0) {
                if (!left_truth) { 
                    return left; // Short circuit: returns falsy left value
                }
                free_value(left);
                return execute_ast(node->data.logical.right, env);
            } 
            else if (strcmp(node->data.logical.type_str, "or") == 0) {
                if (left_truth) {
                    return left; // Short circuit: returns truthy left value
                }
                free_value(left);
                return execute_ast(node->data.logical.right, env);
            }
            free_value(left);
            return null_val;
        }

        case NODE_ASSIGNMENT: {
            Value val = execute_ast(node->data.assign.value, env);
            Environment* target = env_find(env, node->data.assign.name);
            
            if (target) {
                // If it already exists, update it
                env_assign(env, node->data.assign.name, val);
            } else {
                // Micro-optimization: Implicit definition if variable is new (Python/JS mode)
                Environment* new_var = malloc(sizeof(Environment));
                new_var->name = strdup(node->data.assign.name);
                new_var->value = val;
                if (val.type == VAL_STRING && val.as.s_val) {
                    new_var->value.as.s_val = strdup(val.as.s_val);
                }
                // Prepend to current scope node
                new_var->outer = env->outer; 
                env->outer = new_var;
            }
            return val; 
        }

        case NODE_IF_STATEMENT: {
            Value cond = execute_ast(node->data.if_stmt.condition, env);
            Value res = null_val;
            
            if (is_truthy(cond)) {
                res = execute_ast(node->data.if_stmt.then_branch, env);
            } else if (node->data.if_stmt.else_branch) {
                res = execute_ast(node->data.if_stmt.else_branch, env);
            }
            
            free_value(cond);
            return res;
        }

        case NODE_WHILE_LOOP: {
            Value cond = execute_ast(node->data.while_loop.condition, env);
            while (is_truthy(cond)) {
                free_value(cond); // Clear last condition calculation
                Value body_res = execute_ast(node->data.while_loop.body, env);
                free_value(body_res); // Loops yield null sequentially
                cond = execute_ast(node->data.while_loop.condition, env);
            }
            free_value(cond);
            return null_val;
        }

        case NODE_BLOCK: {
            Value last_val = null_val;
            // Native micro-scoping frame setup
            Environment block_scope = { .name = "", .value = null_val, .outer = env };

            for (int i = 0; i < node->data.block.count; i++) {
                free_value(last_val); // Drop memory intermediate statement yields
                last_val = execute_ast(node->data.block.statements[i], &block_scope);
            }

            // Clean up variable maps instantiated inside this local block stack footprint
            Environment* cur = block_scope.outer;
            while (cur != env && cur != NULL) {
                Environment* next = cur->outer;
                free((char*)cur->name);
                free_value(cur->value);
                free(cur);
                cur = next;
            }

            return last_val;
        }

        case NODE_PRINT: {
            Value target = execute_ast(node->data.print_target, env);
            if (target.type == VAL_INT) printf("%d\n", target.as.i_val);
            else if (target.type == VAL_FLOAT) printf("%f\n", target.as.f_val);
            else if (target.type == VAL_STRING) printf("%s\n", target.as.s_val ? target.as.s_val : "null");
            else if (target.type == VAL_BOOL) printf("%s\n", target.as.b_val ? "true" : "false");
            return target; // Returns evaluated values upstream
        }

        case NODE_FUNCTION_CALL: {
            Environment* func_env_entry = env_find(env, node->data.call.name);
            if (!func_env_entry || func_env_entry->value.type != VAL_FUNCTION) {
                fprintf(stderr, "Runtime Error: '%s' is not an executable function.\n", node->data.call.name);
                exit(EXIT_FAILURE);
            }

            FunctionObj function = func_env_entry->value.as.func;
            if (node->data.call.arg_count != function.param_count) {
                fprintf(stderr, "Runtime Error: Function '%s' expects %d arguments, but got %d.\n",
                        node->data.call.name, function.param_count, node->data.call.arg_count);
                exit(EXIT_FAILURE);
            }

            // 1. Evaluate arguments before opening the target execution scope context
            Value* evaluated_args = malloc(sizeof(Value) * node->data.call.arg_count);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                evaluated_args[i] = execute_ast(node->data.call.arguments[i], env);
            }

            // 2. Build local execution scope layer frames linking directly to global frame
            Environment local_frame = { .name = "__local_func__", .value = null_val, .outer = env };
            
            // Inject bound variable states into the newly generated execution context
            for (int i = 0; i < function.param_count; i++) {
                Environment* param_binding = malloc(sizeof(Environment));
                param_binding->name = strdup(function.parameters[i]);
                param_binding->value = evaluated_args[i]; // Binds variables natively
                param_binding->outer = local_frame.outer;
                local_frame.outer = param_binding;
            }

            // 3. Execute the function body code blocks
            Value return_val = execute_ast(function.body, &local_frame);

            // Clean up evaluated storage arrays and parameter environment frames
            Environment* cur = local_frame.outer;
            while (cur != env && cur != NULL) {
                Environment* next = cur->outer;
                free((char*)cur->name);
                free_value(cur->value);
                free(cur);
                cur = next;
            }
            free(evaluated_args);

            return return_val;
        }

        case NODE_INDEX: {
            Value target = execute_ast(node->data.index.target, env);
            Value idx = execute_ast(node->data.index.index, env);
            Value res = null_val;

            if (target.type == VAL_ARRAY && idx.type == VAL_INT) {
                int i = idx.as.i_val;
                // Protection boundary check for embedded environments
                if (i >= 0 && i < target.as.array.count) {
                    res = target.as.array.elements[i];
                    // Duplicate strings if pulling from list bounds
                    if (res.type == VAL_STRING && res.as.s_val) res.as.s_val = strdup(res.as.s_val);
                } else {
                    fprintf(stderr, "Runtime Error: Array index out of bounds.\n");
                    exit(EXIT_FAILURE);
                }
            }
            free_value(target);
            free_value(idx);
            return res;
        }

    }
    return null_val;
}
