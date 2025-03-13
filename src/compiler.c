#include "leancc.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* get_version_string(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             LEANCC_VERSION_MAJOR,
             LEANCC_VERSION_MINOR,
             LEANCC_VERSION_PATCH);
    return version;
}

// Read entire file content
static char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer for entire file
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(file);
        return NULL;
    }

    // Read file content
    size_t read_size = fread(buffer, 1, size, file);
    fclose(file);

    if (read_size != (size_t)size) {
        fprintf(stderr, "Error: Could not read entire file\n");
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    return buffer;
}

int compile_file(const char* input_file, const char* output_file) {
    if (!input_file || !output_file) {
        fprintf(stderr, "Error: Invalid arguments\n");
        return 1;
    }

    // Read source file
    char* source = read_file(input_file);
    if (!source) {
        return 1;
    }

    // Create parser
    struct Parser* parser = parser_create(source);
    if (!parser) {
        fprintf(stderr, "Error: Could not create parser\n");
        free(source);
        return 1;
    }

    // Parse source
    ASTNode* ast = parse(parser);
    if (!ast) {
        fprintf(stderr, "Error: %s\n", parser->error ? parser->error : "Unknown parse error");
        parser_destroy(parser);
        free(source);
        return 1;
    }

    // TODO: Generate code

    // Clean up
    ast_destroy(ast);
    parser_destroy(parser);
    free(source);

    return 0;
}
