# Parser-C

A lightweight C compiler frontend implementing a recursive descent parser with robust error handling and comprehensive C language support.

## Features

- Recursive descent parser
- Binary operators with precedence (+, -, *, /)
- Variable declarations and assignments
- Control flow statements (if/else, while)
- Function calls and definitions
- Symbol table with scope management
- Descriptive error messages

## Building

Requirements:
- C11 compliant compiler
- Make build system

```bash
make clean && make
```

The compiler binary will be built as `build/leancc`.

## Project Structure

```
.
├── include/          # Header files
│   ├── leancc.h     # Main compiler definitions
│   └── parser.h     # Parser interface
├── src/             # Source files
│   ├── compiler.c   # Compiler implementation
│   ├── main.c       # Entry point
│   ├── parser.c     # Parser implementation
│   └── symbol.c     # Symbol table management
└── tests/           # Test files
    └── *.c          # Various test cases
```

## Current Status

- Frontend development phase completed
- Moving to testing and optimization phase
- IR design planning to begin soon

## Next Steps

1. Parser Testing and Validation
   - Comprehensive test suite
   - Edge cases and error conditions
   - AST structure validation

2. Parser Optimization
   - Error recovery mechanisms
   - Memory usage optimization
   - Performance profiling

3. IR Design Planning
   - Requirements documentation
   - IR instruction set design
   - Optimization pass planning

## Contributing

This project is under active development. Feel free to submit issues and pull requests.

## License

[MIT License](LICENSE)
