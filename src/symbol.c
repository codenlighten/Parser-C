#define _POSIX_C_SOURCE 200809L  // For strdup
#include "parser.h"
#include <stdlib.h>
#include <string.h>

// Create a new scope with the given parent scope
Scope* create_scope(Scope* parent) {
    Scope* scope = malloc(sizeof(Scope));
    if (!scope) return NULL;
    
    scope->symbols = NULL;
    scope->parent = parent;
    return scope;
}

// Destroy a scope and all its symbols
void destroy_scope(Scope* scope) {
    if (!scope) return;
    
    // Free all symbols in the scope
    Symbol* current = scope->symbols;
    while (current) {
        Symbol* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    
    free(scope);
}

// Create a new symbol with the given name and type
Symbol* create_symbol(const char* name, SymbolType type) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (!symbol) return NULL;
    
    symbol->name = strdup(name);
    if (!symbol->name) {
        free(symbol);
        return NULL;
    }
    
    symbol->type = type;
    symbol->next = NULL;
    return symbol;
}

// Find a symbol in the current scope or parent scopes
Symbol* scope_find(Scope* scope, const char* name) {
    while (scope) {
        Symbol* current = scope->symbols;
        while (current) {
            if (strcmp(current->name, name) == 0) {
                return current;
            }
            current = current->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

// Add a symbol to the current scope
bool scope_add(Scope* scope, Symbol* symbol) {
    if (!scope || !symbol) return false;
    
    // Check for duplicate symbol in current scope
    Symbol* current = scope->symbols;
    while (current) {
        if (strcmp(current->name, symbol->name) == 0) {
            return false;  // Symbol already exists
        }
        current = current->next;
    }
    
    // Add symbol to front of list
    symbol->next = scope->symbols;
    scope->symbols = symbol;
    return true;
}
