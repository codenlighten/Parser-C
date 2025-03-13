#ifndef LEANCC_H
#define LEANCC_H

#include <stdint.h>
#include <stdbool.h>

// Compiler version
#define LEANCC_VERSION_MAJOR 0
#define LEANCC_VERSION_MINOR 1
#define LEANCC_VERSION_PATCH 0

// Forward declarations
typedef struct Parser Parser;
typedef struct AST AST;
typedef struct IR IR;
typedef struct CodeGen CodeGen;

// Error handling
typedef enum {
    ERROR_NONE = 0,
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
    ERROR_CODEGEN,
    ERROR_IO
} ErrorCode;

typedef struct {
    ErrorCode code;
    char message[256];
    int line;
    int column;
} Error;

// Main compiler interface
bool compile_file(const char* input_file, const char* output_file, Error* error);
const char* get_version_string(void);

#endif // LEANCC_H
