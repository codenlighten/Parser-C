#define _POSIX_C_SOURCE 200809L  // For strdup
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Forward declarations
static ASTNode* create_node(NodeType type);
static Token get_next_token(struct Parser* parser);
static bool expect(struct Parser* parser, TokenType type);
static ASTNode* parse_function(struct Parser* parser);
static ASTNode* parse_statement(struct Parser* parser);
static ASTNode* parse_expression(struct Parser* parser);
static ASTNode* parse_primary(struct Parser* parser);
static ASTNode* parse_variable_declaration(struct Parser* parser);
static ASTNode* parse_if_statement(struct Parser* parser);
static ASTNode* parse_while_statement(struct Parser* parser);
static ASTNode* parse_block(struct Parser* parser);
static ASTNode* parse_function_call(struct Parser* parser, const char* name);
static ASTNode* parse_declaration(struct Parser* parser);

// Helper functions
static void set_error(struct Parser* parser, const char* message) {
    if (parser) {
        parser->error = message;
    }
}

static char get_next_char(struct Parser* parser) {
    if (parser->position >= parser->source_length) {
        return EOF;
    }
    return parser->source[parser->position++];
}

static void skip_whitespace(struct Parser* parser) {
    while (parser->position < parser->source_length) {
        char c = parser->source[parser->position];
        
        if (isspace(c)) {
            if (c == '\n') {
                parser->line++;
                parser->column = 0;
            } else {
                parser->column++;
            }
            parser->position++;
            continue;
        }
        
        // Skip single-line comments
        if (c == '/' && parser->position + 1 < parser->source_length) {
            if (parser->source[parser->position + 1] == '/') {
                parser->position += 2;
                parser->column += 2;
                
                while (parser->position < parser->source_length && 
                       parser->source[parser->position] != '\n') {
                    parser->position++;
                    parser->column++;
                }
                continue;
            }
        }
        
        break;
    }
}

static bool is_keyword(const char* text, const char* keyword) {
    return strcmp(text, keyword) == 0;
}

static Token get_next_token(struct Parser* parser) {
    Token token = {0};
    token.line = parser->line;
    token.column = parser->column;
    
    skip_whitespace(parser);
    
    if (parser->position >= parser->source_length) {
        token.type = TOKEN_EOF;
        return token;
    }
    
    char c = get_next_char(parser);
    
    // Handle identifiers and keywords
    if (isalpha(c) || c == '_') {
        size_t start = parser->position - 1;
        while (parser->position < parser->source_length && 
               (isalnum(parser->source[parser->position]) || parser->source[parser->position] == '_')) {
            parser->position++;
            parser->column++;
        }
        
        size_t len = parser->position - start;
        char* text = malloc(len + 1);
        strncpy(text, &parser->source[start], len);
        text[len] = '\0';
        
        // Check for keywords
        if (strcmp(text, "int") == 0) token.type = TOKEN_INT;
        else if (strcmp(text, "return") == 0) token.type = TOKEN_RETURN;
        else if (strcmp(text, "if") == 0) token.type = TOKEN_IF;
        else if (strcmp(text, "else") == 0) token.type = TOKEN_ELSE;
        else if (strcmp(text, "while") == 0) token.type = TOKEN_WHILE;
        else {
            token.type = TOKEN_IDENTIFIER;
            token.value.identifier = text;
            return token;
        }
        
        free(text);
        return token;
    }
    
    // Handle numbers
    if (isdigit(c)) {
        int64_t value = 0;
        size_t start = parser->position - 1;
        
        while (parser->position < parser->source_length && isdigit(parser->source[parser->position])) {
            value = value * 10 + (parser->source[parser->position] - '0');
            parser->position++;
            parser->column++;
        }
        
        size_t len = parser->position - start;
        char* text = malloc(len + 1);
        strncpy(text, &parser->source[start], len);
        text[len] = '\0';
        
        token.type = TOKEN_NUMBER;
        token.value.number = value;
        free(text);
        return token;
    }
    
    // Handle operators and delimiters
    parser->position++;
    parser->column++;
    
    switch (c) {
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '=':
            if (parser->position < parser->source_length && parser->source[parser->position] == '=') {
                parser->position++;
                parser->column++;
                token.type = TOKEN_EQ;
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '!':
            if (parser->position < parser->source_length && parser->source[parser->position] == '=') {
                parser->position++;
                parser->column++;
                token.type = TOKEN_NEQ;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
        case '<':
            if (parser->position < parser->source_length && parser->source[parser->position] == '=') {
                parser->position++;
                parser->column++;
                token.type = TOKEN_LTE;
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (parser->position < parser->source_length && parser->source[parser->position] == '=') {
                parser->position++;
                parser->column++;
                token.type = TOKEN_GTE;
            } else {
                token.type = TOKEN_GT;
            }
            break;
        default:
            token.type = TOKEN_ERROR;
            break;
    }
    
    return token;
}

static int get_precedence(BinaryOp op) {
    switch (op) {
        case OP_ASSIGN:
            return 1;
        case OP_EQUALS:
        case OP_NOT_EQUALS:
            return 2;
        case OP_LESS:
        case OP_LESS_EQUAL:
        case OP_GREATER:
        case OP_GREATER_EQUAL:
            return 3;
        case OP_ADD:
        case OP_SUBTRACT:
            return 4;
        case OP_MULTIPLY:
        case OP_DIVIDE:
            return 5;
        default:
            return 0;
    }
}

static BinaryOp get_binary_op(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:    return OP_ADD;
        case TOKEN_MINUS:   return OP_SUBTRACT;
        case TOKEN_STAR:    return OP_MULTIPLY;
        case TOKEN_SLASH:   return OP_DIVIDE;
        case TOKEN_EQ:      return OP_EQUALS;
        case TOKEN_NEQ:     return OP_NOT_EQUALS;
        case TOKEN_LT:      return OP_LESS;
        case TOKEN_GT:      return OP_GREATER;
        case TOKEN_LTE:     return OP_LESS_EQUAL;
        case TOKEN_GTE:     return OP_GREATER_EQUAL;
        case TOKEN_ASSIGN:  return OP_ASSIGN;
        default:           return -1;
    }
}

static ASTNode* parse_primary(struct Parser* parser) {
    if (parser->current.type == TOKEN_NUMBER) {
        ASTNode* num = create_node(NODE_NUMBER);
        if (!num) return NULL;
        
        num->data.number.value = parser->current.value.number;
        num->line = parser->current.line;
        num->column = parser->current.column;
        parser->current = get_next_token(parser);
        return num;
    }
    
    if (parser->current.type == TOKEN_IDENTIFIER) {
        const char* name = parser->current.value.identifier;
        parser->current = get_next_token(parser);
        
        // Check if this is a function call
        if (parser->current.type == TOKEN_LPAREN) {
            return parse_function_call(parser, name);
        }
        
        // Otherwise, it's a variable reference
        Symbol* symbol = scope_find(parser->current_scope, name);
        if (!symbol) {
            set_error(parser, "Undefined variable");
            return NULL;
        }
        
        ASTNode* var = create_node(NODE_VARIABLE);
        if (!var) return NULL;
        
        var->data.variable.name = strdup(name);
        var->line = parser->current.line;
        var->column = parser->current.column;
        
        // Check for assignment
        if (parser->current.type == TOKEN_ASSIGN) {
            ASTNode* assign = create_node(NODE_ASSIGNMENT);
            if (!assign) {
                ast_destroy(var);
                return NULL;
            }
            
            parser->current = get_next_token(parser);
            ASTNode* value = parse_expression(parser);
            if (!value) {
                ast_destroy(var);
                ast_destroy(assign);
                return NULL;
            }
            
            assign->data.assignment.name = var->data.variable.name;
            assign->data.assignment.value = value;
            free(var);  // Free the variable node but keep its name
            return assign;
        }
        
        return var;
    }
    
    if (parser->current.type == TOKEN_LPAREN) {
        parser->current = get_next_token(parser);
        ASTNode* expr = parse_expression(parser);
        if (!expr || !expect(parser, TOKEN_RPAREN)) {
            ast_destroy(expr);
            return NULL;
        }
        return expr;
    }
    
    set_error(parser, "Expected expression");
    return NULL;
}

static ASTNode* parse_expression_precedence(struct Parser* parser, int min_precedence) {
    ASTNode* left = parse_primary(parser);
    if (!left) return NULL;
    
    while (true) {
        BinaryOp op = get_binary_op(parser->current.type);
        if ((int)op < 0 || get_precedence(op) < min_precedence) break;
        
        parser->current = get_next_token(parser);
        
        int next_min_precedence = get_precedence(op) + 1;
        ASTNode* right = parse_expression_precedence(parser, next_min_precedence);
        if (!right) {
            ast_destroy(left);
            return NULL;
        }
        
        ASTNode* binary = create_node(NODE_BINARY_OP);
        if (!binary) {
            ast_destroy(left);
            ast_destroy(right);
            return NULL;
        }
        
        binary->data.binary.op = op;
        binary->data.binary.left = left;
        binary->data.binary.right = right;
        left = binary;
    }
    
    return left;
}

static ASTNode* parse_expression(struct Parser* parser) {
    return parse_expression_precedence(parser, 0);
}

static bool expect(struct Parser* parser, TokenType type) {
    if (parser->current.type != type) {
        set_error(parser, "Unexpected token");
        return false;
    }
    Token old = parser->current;
    parser->current = get_next_token(parser);
    free((void*)old.text);
    return true;
}

static ASTNode* create_node(NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node) {
        memset(node, 0, sizeof(ASTNode));
        node->type = type;
    }
    return node;
}

static ASTNode* parse_function(struct Parser* parser) {
    // Parse return type (currently only 'int' supported)
    if (!expect(parser, TOKEN_INT)) return NULL;
    
    // Parse function name
    if (parser->current.type != TOKEN_IDENTIFIER) {
        set_error(parser, "Expected function name");
        return NULL;
    }
    
    ASTNode* func = create_node(NODE_FUNCTION);
    if (!func) return NULL;
    
    func->data.function.name = strdup(parser->current.value.identifier);
    parser->current = get_next_token(parser);
    
    // Create new scope for function parameters
    Scope* param_scope = create_scope(parser->current_scope);
    if (!param_scope) {
        ast_destroy(func);
        return NULL;
    }
    parser->current_scope = param_scope;
    
    // Parse parameter list
    if (!expect(parser, TOKEN_LPAREN)) {
        ast_destroy(func);
        return NULL;
    }
    
    // Parse parameters
    while (parser->current.type != TOKEN_RPAREN) {
        // Parameter type (currently only 'int' supported)
        if (!expect(parser, TOKEN_INT)) {
            ast_destroy(func);
            return NULL;
        }
        
        // Parameter name
        if (parser->current.type != TOKEN_IDENTIFIER) {
            set_error(parser, "Expected parameter name");
            ast_destroy(func);
            return NULL;
        }
        
        // Add parameter to symbol table
        Symbol* param = create_symbol(parser->current.value.identifier, SYMBOL_VARIABLE);
        if (!param || !scope_add(parser->current_scope, param)) {
            ast_destroy(func);
            return NULL;
        }
        
        parser->current = get_next_token(parser);
        
        // Check for more parameters
        if (parser->current.type == TOKEN_COMMA) {
            parser->current = get_next_token(parser);
            continue;
        }
        
        if (parser->current.type == TOKEN_RPAREN) break;
        
        set_error(parser, "Expected ',' or ')'");
        ast_destroy(func);
        return NULL;
    }
    
    if (!expect(parser, TOKEN_RPAREN)) {
        ast_destroy(func);
        return NULL;
    }
    
    // Parse function body
    if (!expect(parser, TOKEN_LBRACE)) {
        ast_destroy(func);
        return NULL;
    }
    
    // Create new scope for function body
    Scope* body_scope = create_scope(parser->current_scope);
    if (!body_scope) {
        ast_destroy(func);
        return NULL;
    }
    parser->current_scope = body_scope;
    
    // Parse statements
    ASTNode* body = create_node(NODE_BLOCK);
    if (!body) {
        ast_destroy(func);
        return NULL;
    }
    
    body->data.block.statements = NULL;
    body->data.block.count = 0;
    body->data.block.capacity = 0;
    
    while (parser->current.type != TOKEN_RBRACE) {
        ASTNode* stmt = parse_statement(parser);
        if (!stmt) {
            ast_destroy(func);
            ast_destroy(body);
            return NULL;
        }
        
        // Add statement to block
        if (body->data.block.count >= body->data.block.capacity) {
            size_t new_capacity = body->data.block.capacity == 0 ? 4 : body->data.block.capacity * 2;
            ASTNode** new_statements = realloc(body->data.block.statements, new_capacity * sizeof(ASTNode*));
            if (!new_statements) {
                ast_destroy(func);
                ast_destroy(body);
                ast_destroy(stmt);
                return NULL;
            }
            body->data.block.statements = new_statements;
            body->data.block.capacity = new_capacity;
        }
        
        body->data.block.statements[body->data.block.count++] = stmt;
    }
    
    if (!expect(parser, TOKEN_RBRACE)) {
        ast_destroy(func);
        ast_destroy(body);
        return NULL;
    }
    
    // Restore outer scope
    parser->current_scope = parser->current_scope->parent;
    
    func->data.function.body = body;
    return func;
}

static ASTNode* parse_if_statement(struct Parser* parser) {
    if (!expect(parser, TOKEN_IF)) return NULL;
    if (!expect(parser, TOKEN_LPAREN)) return NULL;
    
    ASTNode* condition = parse_expression(parser);
    if (!condition || !expect(parser, TOKEN_RPAREN)) {
        ast_destroy(condition);
        return NULL;
    }
    
    if (!expect(parser, TOKEN_LBRACE)) {
        ast_destroy(condition);
        return NULL;
    }
    
    ASTNode* then_branch = create_node(NODE_BLOCK);
    if (!then_branch) {
        ast_destroy(condition);
        return NULL;
    }
    
    then_branch->data.block.statements = NULL;
    then_branch->data.block.count = 0;
    then_branch->data.block.capacity = 0;
    
    while (parser->current.type != TOKEN_RBRACE) {
        ASTNode* stmt = parse_statement(parser);
        if (!stmt) {
            ast_destroy(condition);
            ast_destroy(then_branch);
            return NULL;
        }
        
        // Add statement to block
        if (then_branch->data.block.count >= then_branch->data.block.capacity) {
            size_t new_capacity = then_branch->data.block.capacity == 0 ? 4 : then_branch->data.block.capacity * 2;
            ASTNode** new_statements = realloc(then_branch->data.block.statements, new_capacity * sizeof(ASTNode*));
            if (!new_statements) {
                ast_destroy(condition);
                ast_destroy(then_branch);
                ast_destroy(stmt);
                return NULL;
            }
            then_branch->data.block.statements = new_statements;
            then_branch->data.block.capacity = new_capacity;
        }
        
        then_branch->data.block.statements[then_branch->data.block.count++] = stmt;
    }
    
    if (!expect(parser, TOKEN_RBRACE)) {
        ast_destroy(condition);
        ast_destroy(then_branch);
        return NULL;
    }
    
    ASTNode* else_branch = NULL;
    if (parser->current.type == TOKEN_ELSE) {
        parser->current = get_next_token(parser);
        
        if (!expect(parser, TOKEN_LBRACE)) {
            ast_destroy(condition);
            ast_destroy(then_branch);
            return NULL;
        }
        
        else_branch = create_node(NODE_BLOCK);
        if (!else_branch) {
            ast_destroy(condition);
            ast_destroy(then_branch);
            return NULL;
        }
        
        else_branch->data.block.statements = NULL;
        else_branch->data.block.count = 0;
        else_branch->data.block.capacity = 0;
        
        while (parser->current.type != TOKEN_RBRACE) {
            ASTNode* stmt = parse_statement(parser);
            if (!stmt) {
                ast_destroy(condition);
                ast_destroy(then_branch);
                ast_destroy(else_branch);
                return NULL;
            }
            
            // Add statement to block
            if (else_branch->data.block.count >= else_branch->data.block.capacity) {
                size_t new_capacity = else_branch->data.block.capacity == 0 ? 4 : else_branch->data.block.capacity * 2;
                ASTNode** new_statements = realloc(else_branch->data.block.statements, new_capacity * sizeof(ASTNode*));
                if (!new_statements) {
                    ast_destroy(condition);
                    ast_destroy(then_branch);
                    ast_destroy(else_branch);
                    return NULL;
                }
                else_branch->data.block.statements = new_statements;
                else_branch->data.block.capacity = new_capacity;
            }
            
            else_branch->data.block.statements[else_branch->data.block.count++] = stmt;
        }
        
        if (!expect(parser, TOKEN_RBRACE)) {
            ast_destroy(condition);
            ast_destroy(then_branch);
            ast_destroy(else_branch);
            return NULL;
        }
    }
    
    ASTNode* if_stmt = create_node(NODE_IF_STMT);
    if (!if_stmt) {
        ast_destroy(condition);
        ast_destroy(then_branch);
        ast_destroy(else_branch);
        return NULL;
    }
    
    if_stmt->data.if_stmt_node.condition = condition;
    if_stmt->data.if_stmt_node.then_branch = then_branch;
    if_stmt->data.if_stmt_node.else_branch = else_branch;
    return if_stmt;
}

static ASTNode* parse_while_statement(struct Parser* parser) {
    if (!expect(parser, TOKEN_WHILE)) return NULL;
    if (!expect(parser, TOKEN_LPAREN)) return NULL;
    
    ASTNode* condition = parse_expression(parser);
    if (!condition || !expect(parser, TOKEN_RPAREN)) {
        ast_destroy(condition);
        return NULL;
    }
    
    if (!expect(parser, TOKEN_LBRACE)) {
        ast_destroy(condition);
        return NULL;
    }
    
    ASTNode* body = create_node(NODE_BLOCK);
    if (!body) {
        ast_destroy(condition);
        return NULL;
    }
    
    body->data.block.statements = NULL;
    body->data.block.count = 0;
    body->data.block.capacity = 0;
    
    while (parser->current.type != TOKEN_RBRACE) {
        ASTNode* stmt = parse_statement(parser);
        if (!stmt) {
            ast_destroy(condition);
            ast_destroy(body);
            return NULL;
        }
        
        // Add statement to block
        if (body->data.block.count >= body->data.block.capacity) {
            size_t new_capacity = body->data.block.capacity == 0 ? 4 : body->data.block.capacity * 2;
            ASTNode** new_statements = realloc(body->data.block.statements, new_capacity * sizeof(ASTNode*));
            if (!new_statements) {
                ast_destroy(condition);
                ast_destroy(body);
                return NULL;
            }
            body->data.block.statements = new_statements;
            body->data.block.capacity = new_capacity;
        }
        
        body->data.block.statements[body->data.block.count++] = stmt;
    }
    
    if (!expect(parser, TOKEN_RBRACE)) {
        ast_destroy(condition);
        ast_destroy(body);
        return NULL;
    }
    
    ASTNode* while_stmt = create_node(NODE_WHILE_STMT);
    if (!while_stmt) {
        ast_destroy(condition);
        ast_destroy(body);
        return NULL;
    }
    
    while_stmt->data.while_stmt_node.condition = condition;
    while_stmt->data.while_stmt_node.body = body;
    return while_stmt;
}

static ASTNode* parse_variable_declaration(struct Parser* parser) {
    // Skip 'int' keyword, we already checked it
    parser->current = get_next_token(parser);
    
    // Get variable name
    if (parser->current.type != TOKEN_IDENTIFIER) {
        set_error(parser, "Expected variable name");
        return NULL;
    }
    
    const char* name = parser->current.value.identifier;
    
    // Create symbol
    Symbol* symbol = create_symbol(name, SYMBOL_VARIABLE);
    if (!symbol) {
        set_error(parser, "Failed to create symbol");
        return NULL;
    }
    
    // Add to symbol table
    if (!scope_add(parser->current_scope, symbol)) {
        set_error(parser, "Variable already declared in this scope");
        free(symbol);
        return NULL;
    }
    
    // Create variable node
    ASTNode* var = create_node(NODE_VARIABLE);
    if (!var) {
        set_error(parser, "Failed to create variable node");
        return NULL;
    }
    
    var->data.variable.name = strdup(name);
    parser->current = get_next_token(parser);
    
    // Check for initialization
    if (parser->current.type == TOKEN_ASSIGN) {
        parser->current = get_next_token(parser);
        
        // Create assignment node
        ASTNode* assign = create_node(NODE_ASSIGNMENT);
        if (!assign) {
            ast_destroy(var);
            return NULL;
        }
        
        assign->data.assignment.name = strdup(name);
        assign->data.assignment.value = parse_expression(parser);
        
        if (!assign->data.assignment.value) {
            ast_destroy(var);
            ast_destroy(assign);
            return NULL;
        }
        
        free(var->data.variable.name);
        free(var);
        var = assign;
    }
    
    // Expect semicolon
    if (!expect(parser, TOKEN_SEMICOLON)) {
        ast_destroy(var);
        return NULL;
    }
    
    return var;
}

static ASTNode* parse_function_call(struct Parser* parser, const char* name) {
    ASTNode* call = create_node(NODE_CALL);
    if (!call) return NULL;
    
    call->data.call.name = strdup(name);
    call->data.call.args = NULL;
    call->data.call.arg_count = 0;
    
    // Parse arguments
    if (!expect(parser, TOKEN_LPAREN)) {
        ast_destroy(call);
        return NULL;
    }
    
    // Handle argument list
    size_t capacity = 0;
    while (parser->current.type != TOKEN_RPAREN) {
        // Add comma between arguments
        if (call->data.call.arg_count > 0) {
            if (!expect(parser, TOKEN_COMMA)) {
                ast_destroy(call);
                return NULL;
            }
        }
        
        // Parse argument expression
        ASTNode* arg = parse_expression(parser);
        if (!arg) {
            ast_destroy(call);
            return NULL;
        }
        
        // Grow argument array if needed
        if (call->data.call.arg_count >= capacity) {
            size_t new_capacity = capacity == 0 ? 4 : capacity * 2;
            ASTNode** new_args = realloc(call->data.call.args, 
                                       new_capacity * sizeof(ASTNode*));
            if (!new_args) {
                ast_destroy(arg);
                ast_destroy(call);
                return NULL;
            }
            call->data.call.args = new_args;
            capacity = new_capacity;
        }
        
        call->data.call.args[call->data.call.arg_count++] = arg;
    }
    
    // Expect closing parenthesis
    if (!expect(parser, TOKEN_RPAREN)) {
        ast_destroy(call);
        return NULL;
    }
    
    return call;
}

static ASTNode* parse_statement(struct Parser* parser) {
    switch (parser->current.type) {
        case TOKEN_INT:
            return parse_variable_declaration(parser);
            
        case TOKEN_IF:
            return parse_if_statement(parser);
            
        case TOKEN_WHILE:
            return parse_while_statement(parser);
            
        case TOKEN_RETURN: {
            ASTNode* ret = create_node(NODE_RETURN);
            if (!ret) return NULL;
            
            parser->current = get_next_token(parser);
            ret->data.ret.expr = parse_expression(parser);
            
            if (!ret->data.ret.expr || !expect(parser, TOKEN_SEMICOLON)) {
                ast_destroy(ret);
                return NULL;
            }
            return ret;
        }
        
        case TOKEN_IDENTIFIER: {
            ASTNode* expr = parse_expression(parser);
            if (!expr || !expect(parser, TOKEN_SEMICOLON)) {
                ast_destroy(expr);
                return NULL;
            }
            return expr;
        }
        
        default:
            set_error(parser, "Unexpected token in statement");
            return NULL;
    }
}

static ASTNode* parse_declaration(struct Parser* parser) {
    // Currently only support 'int' type
    if (parser->current.type != TOKEN_INT) {
        set_error(parser, "Expected type specifier");
        return NULL;
    }
    
    // Look ahead to see if this is a function or variable declaration
    Token current = parser->current;
    parser->current = get_next_token(parser);
    
    if (parser->current.type != TOKEN_IDENTIFIER) {
        set_error(parser, "Expected identifier after type specifier");
        return NULL;
    }
    
    // Save identifier and look ahead
    const char* name = parser->current.value.identifier;
    parser->current = get_next_token(parser);
    
    // Function declaration if we see a left parenthesis
    if (parser->current.type == TOKEN_LPAREN) {
        // Restore state for function parsing
        parser->current = current;
        return parse_function(parser);
    }
    
    // Otherwise, must be a variable declaration
    // Restore state for variable parsing
    parser->current = current;
    return parse_variable_declaration(parser);
}

// Parse source code into AST
ASTNode* parse(struct Parser* parser) {
    if (!parser) return NULL;
    
    // Create program node
    ASTNode* program = create_node(NODE_PROGRAM);
    if (!program) return NULL;
    
    program->data.block.statements = NULL;
    program->data.block.count = 0;
    program->data.block.capacity = 0;
    
    // Parse declarations and functions
    while (parser->current.type != TOKEN_EOF) {
        ASTNode* node = parse_declaration(parser);
        if (!node) {
            ast_destroy(program);
            return NULL;
        }
        
        // Add to program block
        if (program->data.block.count >= program->data.block.capacity) {
            size_t new_capacity = program->data.block.capacity == 0 ? 
                                4 : program->data.block.capacity * 2;
            ASTNode** new_statements = realloc(program->data.block.statements,
                                             new_capacity * sizeof(ASTNode*));
            if (!new_statements) {
                ast_destroy(node);
                ast_destroy(program);
                return NULL;
            }
            program->data.block.statements = new_statements;
            program->data.block.capacity = new_capacity;
        }
        
        program->data.block.statements[program->data.block.count++] = node;
    }
    
    return program;
}

// AST cleanup
void ast_destroy(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            for (size_t i = 0; i < node->data.block.count; i++) {
                ast_destroy(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
            
        case NODE_FUNCTION:
            free(node->data.function.name);
            ast_destroy(node->data.function.body);
            break;
            
        case NODE_BLOCK:
            for (size_t i = 0; i < node->data.block.count; i++) {
                ast_destroy(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
            
        case NODE_RETURN:
            ast_destroy(node->data.ret.expr);
            break;
            
        case NODE_BINARY_OP:
            ast_destroy(node->data.binary.left);
            ast_destroy(node->data.binary.right);
            break;
            
        case NODE_VARIABLE:
            free(node->data.variable.name);
            break;
            
        case NODE_ASSIGNMENT:
            free(node->data.assignment.name);
            ast_destroy(node->data.assignment.value);
            break;
            
        case NODE_IF_STMT:
            ast_destroy(node->data.if_stmt_node.condition);
            ast_destroy(node->data.if_stmt_node.then_branch);
            if (node->data.if_stmt_node.else_branch) {
                ast_destroy(node->data.if_stmt_node.else_branch);
            }
            break;
            
        case NODE_WHILE_STMT:
            ast_destroy(node->data.while_stmt_node.condition);
            ast_destroy(node->data.while_stmt_node.body);
            break;
            
        case NODE_CALL:
            free(node->data.call.name);
            for (size_t i = 0; i < node->data.call.arg_count; i++) {
                ast_destroy(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;
            
        case NODE_NUMBER:
            // Nothing to free for numbers
            break;
            
        case NODE_UNARY_OP:
            // Not implemented yet
            break;
    }
    
    free(node);
}

// Parser creation and destruction
struct Parser* parser_create(const char* source) {
    struct Parser* parser = malloc(sizeof(struct Parser));
    if (!parser) return NULL;
    
    parser->source = source;
    parser->source_length = strlen(source);
    parser->position = 0;
    parser->line = 1;
    parser->column = 1;
    parser->error = NULL;
    
    // Create global scope
    parser->current_scope = create_scope(NULL);
    if (!parser->current_scope) {
        free(parser);
        return NULL;
    }
    
    // Get first token
    parser->current = get_next_token(parser);
    return parser;
}

void parser_destroy(struct Parser* parser) {
    if (!parser) return;
    
    // Clean up symbol table
    while (parser->current_scope) {
        Scope* parent = parser->current_scope->parent;
        destroy_scope(parser->current_scope);
        parser->current_scope = parent;
    }
    
    free(parser);
}
