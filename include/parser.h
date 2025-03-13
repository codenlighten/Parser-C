#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "leancc.h"

// Token types for lexical analysis
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,
    // Keywords
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    // Single-character tokens
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    TOKEN_ASSIGN,    // =
    // Operators
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_LT,        // <
    TOKEN_GT,        // >
    TOKEN_LTE,       // <=
    TOKEN_GTE        // >=
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* text;
    int line;
    int column;
    union {
        int64_t number;
        char* identifier;
    } value;
} Token;

// Operator precedence levels
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // - !
    PREC_PRIMARY
} Precedence;

// Binary operation types
typedef enum {
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUALS,
    OP_NOT_EQUALS,
    OP_LESS,
    OP_GREATER,
    OP_LESS_EQUAL,
    OP_GREATER_EQUAL,
    OP_ASSIGN
} BinaryOp;

// AST node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_BLOCK,
    NODE_RETURN,
    NODE_IF,
    NODE_WHILE,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_VARIABLE,
    NODE_NUMBER,
    NODE_ASSIGNMENT,
    NODE_CALL,
    NODE_IF_STMT,
    NODE_WHILE_STMT
} NodeType;

// Symbol types
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION
} SymbolType;

// Symbol structure
typedef struct Symbol {
    char* name;
    SymbolType type;
    struct Symbol* next;
} Symbol;

// Scope structure
typedef struct Scope {
    Symbol* symbols;
    struct Scope* parent;
} Scope;

// AST node structure
typedef struct ASTNode {
    NodeType type;
    int line;
    int column;
    union {
        struct {
            char* name;
            struct ASTNode* params;
            int param_count;
            struct ASTNode* body;
        } function;
        struct {
            struct ASTNode** statements;
            size_t count;
            size_t capacity;
        } block;
        struct {
            struct ASTNode* expr;
        } ret;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_loop;
        struct {
            BinaryOp op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary;
        struct {
            TokenType op;
            struct ASTNode* operand;
        } unary;
        struct {
            char* name;
        } variable;
        struct {
            int64_t value;
        } number;
        struct {
            char* name;
            struct ASTNode* value;
        } assignment;
        struct {
            char* name;
            struct ASTNode** args;
            size_t arg_count;
        } call;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;  // Optional, can be NULL
        } if_stmt_node;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt_node;
    } data;
} ASTNode;

// Parser structure
typedef struct Parser {
    const char* source;
    size_t source_length;
    size_t position;
    int current_char;
    int line;
    int column;
    Token current;
    const char* error;
    Scope* current_scope;  // Current scope for symbol resolution
} Parser;

// Symbol table functions
Scope* create_scope(Scope* parent);
void destroy_scope(Scope* scope);
Symbol* create_symbol(const char* name, SymbolType type);
Symbol* scope_find(Scope* scope, const char* name);
bool scope_add(Scope* scope, Symbol* symbol);

// Parser interface
struct Parser* parser_create(const char* source);
void parser_destroy(struct Parser* parser);
ASTNode* parse(struct Parser* parser);
void ast_destroy(ASTNode* node);

#endif // PARSER_H
