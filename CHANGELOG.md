# Changelog

All notable changes to the DariX programming language will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive built-in functions for maps: `keys()`, `values()`, `items()`
- Enhanced `sort()` function for arrays
- Complete test suite with 40+ automated tests
- Project structure reorganization with `examples/` and `tests/` directories
- Build automation with Makefile and build script
- CI/CD pipeline with GitHub Actions
- Multi-platform build support (Linux, Windows, macOS)
- Enhanced documentation with categorized built-in functions
- Code deduplication and refactoring for better maintainability

### Enhanced
- REPL with improved helper functions and emoji indicators
- README with comprehensive project structure and build instructions
- Error handling consolidation across the codebase
- Parser error handling with centralized helper functions

### Fixed
- Missing built-in functions that were referenced in documentation
- Code duplication in main.go and REPL components
- File organization and project structure

## [0.1.1] - Previous Release

### Features
- Complete object-oriented programming support
- Enhanced REPL with history and tab completion
- Exception handling system (try-catch-finally)
- Bytecode compiler and virtual machine
- Native module system (go:fs, go:ffi)
- Comprehensive built-in function library
- Class system with inheritance
- First-class functions and closures
- Dynamic typing with type introspection
- Control flow (if/else, while, for loops)
- Data structures (arrays, maps)
- String and numeric operations
- Module import system

### Technical
- Tree-walking interpreter with VM fallback
- Lexical analysis and parsing
- Abstract syntax tree (AST) representation
- Object system with memory management
- Sandbox capability policies
- Performance optimizations

## [0.1.0] - Initial Release

### Added
- Basic DariX language implementation
- Core syntax and semantics
- Initial interpreter
- Basic built-in functions
- Command-line interface
