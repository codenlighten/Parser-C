#include "leancc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* program) {
    fprintf(stderr, "Usage: %s <input_file> [-o <output_file>]\n", program);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* input_file = NULL;
    const char* output_file = "a.out";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -o requires an output file\n");
                return 1;
            }
            output_file = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            return 1;
        } else {
            if (input_file) {
                fprintf(stderr, "Error: Multiple input files not supported\n");
                return 1;
            }
            input_file = argv[i];
        }
    }
    
    if (!input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        return 1;
    }
    
    return compile_file(input_file, output_file);
}
