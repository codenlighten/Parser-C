# LeanCC - A Lightweight C Compiler

A lean LLVM replacement focused on simplicity, speed, and small binary size.

## Overview
LeanCC is a lightweight C compiler targeting x86-64 architecture. It aims to provide fast compilation times and small binary outputs for basic C programs.

## Project Goals
- Fast compilation speed (faster than Clang)
- Small binary size (<1MB)
- Support for essential C features
- x86-64 target architecture
- Minimal dependencies

## Building
```bash
# Build the compiler
make

# Run tests
make test

# Clean build artifacts
make clean
```

## Project Structure
```
.
├── src/          # Compiler source code
├── include/      # Header files
├── tests/        # Test suite
├── docs/         # Documentation
├── examples/     # Example C programs
└── build/        # Build artifacts
```

## Development
See [ROADMAP.md](ROADMAP.md) for development timeline and milestones.

## License
[MIT License](LICENSE)
# Parser-C
