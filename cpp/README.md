# DariX Programming Language - C++ Implementation

This is a complete rewrite of the DariX programming language in C++ with JIT compilation support using LLVM.

## Features Implemented

### ✅ Core Language Features
- **Dynamic typing** with runtime type system
- **C-like syntax** preserving original DariX syntax
- **First-class functions** with closures
- **Arrays and maps** (objects) with dynamic sizing
- **Control flow**: if/else, while, for loops
- **Exception handling**: try/catch/finally blocks
- **Object-oriented programming**: classes, instances, methods

### ✅ Built-in Functions
- `print()` - Output to console
- `len()` - Length of arrays/strings
- `type()` - Get object type
- `abs()` - Absolute value
- `str()` - Convert to string
- `int()` - Convert to integer
- `float()` - Convert to float

### ✅ Language Constructs
- Variable declarations: `var x = 42;`
- Function definitions: `func name(params) { ... }`
- Class definitions: `class Name { func method() { ... } }`
- Array literals: `[1, 2, 3]`
- Map literals: `{"key": "value"}`
- Arithmetic operations: `+`, `-`, `*`, `/`, `%`
- Comparison operations: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Logical operations: `&&`, `||`, `!`

## Project Structure

```
cpp/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── token.h            # Token definitions
│   ├── lexer.h            # Lexer interface
│   ├── ast.h              # AST node definitions
│   ├── parser.h           # Parser interface
│   ├── object.h           # Runtime object system
│   ├── environment.h      # Variable environment
│   ├── interpreter.h      # Interpreter interface
│   └── builtins.h         # Built-in functions
├── src/                   # Source files
│   ├── main.cpp           # Main entry point
│   ├── lexer.cpp          # Tokenization
│   ├── ast.cpp            # AST implementations
│   ├── parser.cpp         # Syntax parsing
│   ├── object.cpp         # Runtime objects
│   ├── environment.cpp    # Environment management
│   ├── interpreter.cpp    # AST evaluation
│   └── builtins.cpp       # Standard library
├── tests/                 # Test files
│   └── test_main.cpp      # Unit tests
└── examples/              # Example programs
    └── hello.dax          # Sample DariX code
```

## Building

### Prerequisites
- C++17 compatible compiler
- CMake 3.16+
- LLVM 10+ (for JIT compilation)

### Build Commands
```bash
mkdir build
cd build
cmake ..
make
```

### Running
```bash
# Run a DariX file
./darix ../examples/hello.dax

# Run tests
./darix_tests
```

## Architecture

### Lexer (`lexer.h/cpp`)
Tokenizes source code into tokens using a simple state machine.

### Parser (`parser.h/cpp`)
Implements recursive descent parsing to build an Abstract Syntax Tree (AST).

### AST (`ast.h/cpp`)
Defines all AST node types for expressions and statements.

### Object System (`object.h/cpp`)
Provides dynamic typing with reference-counted objects for all DariX types.

### Interpreter (`interpreter.h/cpp`)
Tree-walking interpreter that evaluates AST nodes.

### Built-ins (`builtins.h/cpp`)
Standard library functions available in all DariX programs.

## JIT Compilation (Planned)

The next phase will integrate LLVM for Just-In-Time compilation:

1. **Bytecode Compiler**: Convert AST to intermediate bytecode
2. **VM**: Stack-based virtual machine for bytecode execution
3. **JIT Compiler**: LLVM-based native code generation
4. **Optimization**: Constant folding, dead code elimination

## Language Specification

DariX is a dynamically typed language with C-like syntax featuring:

- **Variables**: `var x = value;`
- **Functions**: `func name(params) { return expr; }`
- **Classes**: `class Name { func method(self) { ... } }`
- **Arrays**: `[1, 2, 3]`
- **Maps**: `{"key": "value"}`
- **Control Flow**: `if (cond) { ... } else { ... }`
- **Loops**: `while (cond) { ... }`, `for (init; cond; inc) { ... }`

## Example Program

```dax
// Hello World
var greeting = "Hello, DariX!";
print(greeting);

// Classes
class Point {
    func __init__(self, x, y) {
        self.x = x;
        self.y = y;
    }
    
    func distance(self) {
        return self.x * self.x + self.y * self.y;
    }
}

var p = Point(3, 4);
print("Distance:", p.distance());
```

## Status

**Current Implementation**: Complete interpreter with all core language features
**JIT Compilation**: Planned for next phase
**Performance**: Interpreter ready, JIT will provide significant speedup
**Compatibility**: Maintains syntax compatibility with original DariX

This C++ implementation provides a solid foundation for a high-performance DariX runtime with JIT compilation capabilities.
