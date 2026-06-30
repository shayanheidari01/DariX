# DariX C++ Architecture

## Overview

DariX is implemented as a tree-walking interpreter with an optional bytecode VM. The architecture follows a classic compiler pipeline:

```
Source Code → Lexer → Tokens → Parser → AST → Compiler → Bytecode → VM
                                                    ↓
                                              Interpreter (fallback)
```

## Components

### Lexer (`lexer.hpp/cpp`)
Single-pass scanner that converts source text into tokens. Handles:
- All operators including two-character tokens (`<=`, `>=`, `==`, `!=`, `||`, `&&`)
- String literals with escape sequences (`\n`, `\t`, `\r`, `\\`, `\"`)
- Line comments (`//`), separator comments (`//---`), block comments (`/* */`)
- Position tracking (line, column, file, offset) for error reporting

### Parser (`parser.hpp/cpp`)
Pratt (top-down operator precedence) parser with 12 precedence levels:
```
LOWEST → ASSIGN → EQUALS → LESSGREATER → SUM → OR → AND → PRODUCT → PREFIX → CALL → INDEX → MEMBER
```
- 18 prefix parse functions (identifiers, literals, prefix operators, grouping, if, function, lambda, while, for, arrays, maps, yield)
- 21 infix parse functions (arithmetic, comparison, assignment, call, index, member, in, is)
- `for` loops parsed as `ForStatement` nodes (interpreter handles directly)
- REPL mode support via `setReplMode()`
- Decorator support via `@decorator` syntax

### AST (`ast.hpp/cpp`)
30+ concrete node types organized into three base interfaces:
- `Node` — base with `tokenLiteral()` and `inspect()`
- `Statement : Node` — statements that don't produce values
- `Expression : Node` — expressions that produce values

Key node types: `Program`, `LetStatement`, `AssignStatement`, `ReturnStatement`, `ExpressionStatement`, `BlockStatement`, `WhileStatement`, `ForStatement`, `FunctionDeclaration`, `ClassDeclaration`, `TryStatement`, `IfExpression`, `CallExpression`, `ArrayLiteral`, `MapLiteral`, `IndexExpression`, `MemberExpression`, `LambdaExpression`, etc.

### Object System (`object.hpp/cpp`)
22 concrete types inheriting from `Object`:
- **Primitives**: `Integer`, `Float`, `Boolean`, `Null`, `String`
- **Collections**: `Array`, `Map`, `Hash`
- **Functions**: `Function`, `CompiledFunction`, `Builtin`, `BoundMethod`
- **Classes**: `Class`, `Instance`
- **Control**: `ReturnValue`, `BreakSignal`, `ContinueSignal`, `ExceptionSignal`
- **Error handling**: `Error`, `Exception`, `StackTrace`
- **Modules**: `Module`

Memory management via `std::shared_ptr<Object>`. Small-integer cache (0-255) for performance.

### Compiler (`compiler.hpp/cpp`)
AST-to-bytecode compiler with:
- Constant folding (evaluates constant expressions at compile time)
- Peephole optimizer (removes dead jumps, eliminates unused constants)
- Symbol table with global/local scope tracking
- Debug info (file, line, column per instruction) for error reporting

### VM (`vm.hpp/cpp`)
Stack-based virtual machine with:
- 2048-slot evaluation stack
- 1024-slot global variable array
- 30 opcodes (arithmetic, comparison, control flow, arrays, indexing, strings, functions, locals)
- Instruction budget enforcement (prevents infinite loops)
- JIT compiler for hot-path optimization (threshold: 100 executions)
- Profiling support (opcode execution counts)
- Debug lookup for error location reporting

### Interpreter (`interpreter.hpp/cpp`)
Tree-walking interpreter as fallback when the VM cannot handle certain features:
- Full evaluation of all AST node types
- 35+ built-in functions
- Import system for native modules
- Class system with methods and decorators
- Exception handling (try/catch/finally)
- Closure support with proper scope chain

## Execution Flow

1. **Run mode**: Source → Lex → Parse → Compile → VM (falls back to Interpreter on error)
2. **Eval mode**: Same as run mode
3. **REPL mode**: Interactive loop with backend selection

### Auto-Selection
The `runAuto()` function tries the VM first. If compilation fails (unsupported feature) or execution errors occur, it falls back to the interpreter automatically.

## Native Module System

Modules are registered at startup via `NativeModule` structs containing function maps. When `import math` is called:
1. Parser creates an `ImportStatement` with path `"math"`
2. Interpreter looks up `"math"` in the `Registry`
3. If found, creates a `Module` object with the module's environment
4. Module functions are accessible via `math.sqrt()` member access

### Module Registration
```cpp
// In native_math.cpp
void initMathModule() {
    std::unordered_map<std::string, NativeFunc> funcs;
    funcs["sqrt"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        // implementation
    };
    Registry::instance().registerModule("math", funcs);
}
```

### EvalCallback for Higher-Order Functions
Native modules can call user-defined functions via `callCallable()`, which uses an `EvalCallback` registered by the interpreter during construction.

## Error Handling

### Parser Errors
Collected in `Parser::errors()` and reported with file:line:col information.

### Runtime Errors
Two systems:
1. **Error objects**: `Error` type returned by builtins (e.g., type errors, name errors)
2. **Exception signals**: `ExceptionSignal` thrown by `throw` statements, caught by `try/catch`

### VM Errors
- Stack overflow/underflow
- Unknown opcode
- Type mismatches
- Instruction budget exceeded

## File Structure

```
cpp-src/
├── CMakeLists.txt
├── include/darix/
│   ├── token.hpp              # Token types and lookup
│   ├── ast.hpp                # AST node types
│   ├── lexer.hpp              # Lexer interface
│   ├── parser.hpp             # Parser interface
│   ├── object.hpp             # Object system
│   ├── code.hpp               # Bytecode opcodes
│   ├── compiler.hpp           # Compiler and symbol table
│   ├── vm.hpp                 # Virtual machine
│   ├── interpreter.hpp        # Tree-walking interpreter
│   ├── version.hpp            # Version string
│   └── native/
│       ├── native.hpp         # Module registry
│       ├── math.hpp           # Math module (not needed, registered in .cpp)
│       └── ...
└── src/
    ├── main.cpp               # CLI entry point
    ├── token.cpp
    ├── ast.cpp
    ├── lexer.cpp
    ├── parser.cpp
    ├── object.cpp
    ├── code.cpp
    ├── compiler.cpp
    ├── vm.cpp
    ├── interpreter.cpp
    └── native/
        ├── native.cpp         # Registry and initAll
        ├── native_math.cpp
        ├── native_string.cpp
        ├── native_array.cpp
        ├── native_map.cpp
        ├── native_set.cpp
        ├── native_queue.cpp
        ├── native_stack.cpp
        ├── native_linkedlist.cpp
        ├── native_tree.cpp
        ├── native_graph.cpp
        ├── native_json.cpp
        ├── native_fs.cpp
        ├── native_net.cpp
        ├── native_crypto.cpp
        ├── native_datetime.cpp
        ├── native_random.cpp
        ├── native_regex.cpp
        ├── native_io.cpp
        ├── native_os.cpp
        └── native_encoding.cpp
```
