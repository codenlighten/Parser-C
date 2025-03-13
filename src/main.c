#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leancc.h"

static void print_usage(void) {
    printf("Usage: leancc [options] input_file -o output_file\n");
    printf("Options:\n");
    printf("  -o <file>    Output file name\n");
    printf("  -v           Print version information\n");
    printf("  -h           Print this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const char *input_file = NULL;
    const char *output_file = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            printf("LeanCC version %s\n", get_version_string());
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires an output file\n");
                return 1;
            }
        } else if (input_file == NULL) {
            input_file = argv[i];
        }
    }

    if (input_file == NULL) {
        fprintf(stderr, "Error: no input file specified\n");
        return 1;
    }

    if (output_file == NULL) {
        fprintf(stderr, "Error: no output file specified\n");
        return 1;
    }

    Error error = {0};
    if (!compile_file(input_file, output_file, &error)) {
        fprintf(stderr, "Error: %s (line %d, column %d)\n",
                error.message, error.line, error.column);
        return 1;
    }

    return 0;
}
