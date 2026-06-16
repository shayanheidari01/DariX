# DariX Programming Language - Design Document

## Executive Summary

DariX is a modern, interpreted programming language that synthesizes the best features of Python and Dart, implemented in C++20/23. It targets developers seeking a clean, expressive syntax with powerful type safety and concurrency features.

---

## 1. Language Philosophy

### Core Principles
1. **Readability First**: Clean, indentation-aware syntax (Python-inspired)
2. **Type Safety**: Sound null safety with gradual typing (Dart-inspired)
3. **Developer Productivity**: Minimal boilerplate, maximum expressiveness
4. **Concurrency Native**: Built-in async/await and coroutines
5. **Extensibility**: Modular architecture for future growth

### Target Audience
- Python developers wanting better type safety
- Dart developers wanting cleaner syntax
- Systems programmers needing rapid prototyping
- Educators teaching modern programming concepts

---

## 2. Syntax Specification

### 2.1 Basic Structure

```darix
// Single-line comment
/* Multi-line comment */

// Indentation-based blocks (braces optional)
fn main() {
    print("Hello, DariX!")
}

// Or Python-style
fn main():
    print("Hello, DariX!")
```

### 2.2 Variables and Types

```darix
// Dynamic typing (default)
name = "Alice"
age = 30

// Type hints (optional)
count: int = 42
price: float = 19.99
active: bool = true

// Null safety
nullable: String? = null  // Can be null
nonNull: String = "value" // Cannot be null
maybe = getValue()        // Inferred type

// Null-aware operators
length = name?.length ?? 0  // Null-coalescing
value = data!               // Null assertion
```

### 2.3 Functions

```darix
// Basic function
fn greet(name):
    return "Hello, $name!"

// Type annotations
fn add(a: int, b: int) -> int:
    return a + b

// Optional named parameters
fn createUser(name, {age = 18, active = true}):
    return User(name, age, active)

// Default parameters
fn multiply(a, b = 1):
    return a * b

// Arrow functions (single expression)
square = fn(x) => x * x
```

### 2.4 Control Flow

```darix
// If-else
if score >= 90:
    grade = "A"
elif score >= 80:
    grade = "B"
else:
    grade = "C"

// For loops
for i in range(10):
    print(i)

for item in collection:
    process(item)

// While loops
while condition:
    doWork()

// Pattern matching (switch expressions)
result = match value:
    case 1 => "one"
    case 2 => "two"
    case n if n > 10 => "large"
    _ => "other"

// Structural pattern matching
match point:
    case Point(x: 0, y: 0):
        print("Origin")
    case Point(x: x, y: y):
        print("($x, $y)")
```

### 2.5 Collections

```darix
// Lists
numbers = [1, 2, 3, 4, 5]
first = numbers[0]
slice = numbers[1..3]  // Range slicing

// List comprehensions
squares = [x * x for x in range(10) if x % 2 == 0]

// Dictionaries (Maps)
user = {"name": "Alice", "age": 30}
name = user["name"]

// Dict comprehensions
squared = {k: v * v for k, v in pairs}

// Sets
unique = {1, 2, 3, 4}
```

### 2.6 Classes and Objects

```darix
// Basic class
class Person:
    // Fields
    name: String
    age: int
    
    // Constructor
    fn new(name, age):
        this.name = name
        this.age = age
    
    // Methods
    fn greet():
        print("Hi, I'm $name")
    
    // Static method
    @staticmethod
    fn createAdult(name):
        return Person(name, 18)

// Inheritance
class Employee extends Person:
    company: String
    
    fn new(name, age, company):
        super.new(name, age)
        this.company = company

// Mixins
mixin Loggable:
    fn log(message):
        print("[LOG] $message")

class Service with Loggable:
    fn execute():
        this.log("Executing...")

// Extension methods (add to existing types)
extension StringExtensions on String:
    fn capitalize():
        return this[0].upper() + this[1..]
```

### 2.7 Async/Await and Concurrency

```darix
// Async functions
fn fetchData(url) async:
    response = await http.get(url)
    return response.json()

// Concurrent execution
results = await Future.wait([
    fetchData("/api/users"),
    fetchData("/api/posts")
])

// Generators
fn countUp(limit):
    for i in range(limit):
        yield i

// Coroutines
coroutine worker(id):
    while true:
        task = await channel.receive()
        process(task)
```

### 2.8 Pipe Operator (Functional Style)

```darix
// Pipe for function composition
result = value
    |> transform1
    |> transform2
    |> transform3

// With arguments
result = data |> filter(_ > 0) |> map(_ * 2) |> sum()
```

### 2.9 Decorators

```darix
// Function decorators
@logged
@retry(times=3)
fn apiCall():
    return fetchData()

// Class decorators
@singleton
class ConfigManager:
    pass

// Custom decorator
fn decorator(fn):
    return fn wrapped(...args):
        print("Calling ${fn.name}")
        return fn(...args)
```

### 2.10 Error Handling

```darix
// Try-catch-finally
try:
    riskyOperation()
catch IOException e:
    print("IO Error: $e")
catch Exception e:
    print("Error: $e")
finally:
    cleanup()

// Throw
throw CustomError("Something went wrong")

// Result type (functional error handling)
result = operation().getOrElse(defaultValue)
```

### 2.11 Immutable Data Types

```darix
// Immutable records
record Point(x: int, y: int)

p1 = Point(1, 2)
// p1.x = 3  // Error: immutable

// Copy with modification
p2 = p1.copyWith(x: 3)

// Tuples
coords = (1, 2, 3)
(x, y, z) = coords
```

---

## 3. Type System

### 3.1 Gradual Typing

DariX supports both dynamic and static typing coexisting:

```darix
// Static typing
fn add(a: int, b: int) -> int:
    return a + b

// Dynamic typing
fn dynamicAdd(a, b):
    return a + b

// Mixed
fn process(items: List, count: int):
    for item in items:  // item is dynamic
        print(item)
```

### 3.2 Type Hierarchy

```
Object (root)
├── num
│   ├── int
│   └── double
├── bool
├── String
├── List<T>
├── Map<K, V>
├── Set<T>
├── Function
├── Future<T>
├── Record
└── User-defined classes
```

### 3.3 Null Safety Rules

1. Non-nullable types cannot hold `null`
2. Nullable types (T?) can hold T or null
3. Null-aware operators provide safe access
4. Flow analysis narrows types after null checks

---

## 4. Technical Architecture

### 4.1 System Overview

```
┌─────────────────────────────────────────┐
│              CLI / REPL                  │
├─────────────────────────────────────────┤
│              Frontend                    │
│  ┌─────────┬─────────┬──────────────┐   │
│  │  Lexer  │ Parser  │ Semantic     │   │
│  │         │         │ Analyzer     │   │
│  └────┬────┴────┬────┴──────┬───────┘   │
│       │        │           │            │
│       └────────┴───────────┘            │
│              AST                         │
├─────────────────────────────────────────┤
│              Runtime                     │
│  ┌─────────┬─────────┬──────────────┐   │
│  │ Tree-   │  Env    │  GC          │   │
│  │ Walker  │  Mgmt   │  (Mark-Sweep)│   │
│  ├─────────┼─────────┼──────────────┤   │
│  │  Builtin│  Async  │  Exception   │   │
│  │  Funcs  │  Runtime│  Handling    │   │
│  └─────────┴─────────┴──────────────┘   │
├─────────────────────────────────────────┤
│           Standard Library               │
└─────────────────────────────────────────┘
```

### 4.2 Component Details

#### Lexer
- DFA-based tokenization
- Unicode support
- Interpolation awareness
- Token caching for performance

#### Parser
- Recursive descent with Pratt parsing for expressions
- Operator precedence handling
- Error recovery and reporting
- AST construction with source locations

#### Semantic Analyzer
- Symbol table management
- Type inference and checking
- Scope resolution
- Control flow analysis

#### Interpreter
- Tree-walking execution model
- Environment chain for scopes
- Call stack management
- Exception propagation

#### Garbage Collector
- Mark-and-sweep algorithm
- Root set identification
- Cycle detection
- Incremental collection option

### 4.3 Memory Management

```cpp
// Smart pointer strategy
- std::shared_ptr for AST nodes (shared ownership)
- std::unique_ptr for exclusive resources
- Raw pointers for non-owning references
- Custom allocator for GC-managed objects
```

---

## 5. Implementation Phases

### Phase 1: Foundation (Lexer + Parser + AST)
- Token definitions and DFA
- Grammar specification
- AST node hierarchy
- Basic error reporting

### Phase 2: Core Interpreter
- Expression evaluation
- Variable binding
- Function calls
- Basic I/O

### Phase 3: Control Flow & OOP
- Conditionals and loops
- Class definitions
- Inheritance and mixins
- Method dispatch

### Phase 4: Type System
- Type inference engine
- Static type checking
- Null safety analysis
- Type errors and warnings

### Phase 5: Concurrency
- Async/await implementation
- Future/Promise system
- Coroutine support
- Event loop

### Phase 6: Standard Library & GC
- Core library modules
- Garbage collector
- Performance optimization
- Memory profiling

### Phase 7: Tooling & Documentation
- CLI with flags
- Interactive REPL
- Comprehensive docs
- Example programs

---

## 6. File Structure

```
darix/
├── CMakeLists.txt
├── README.md
├── LANGUAGE_SPEC.md          # Language specification
├── SYNTAX_GUIDE.md           # Syntax reference with examples
├── src/
│   ├── main.cpp              # Entry point
│   ├── darix.hpp             # Main header (includes all)
│   ├── lexer/
│   │   ├── token.hpp         # Token definitions
│   │   ├── lexer.hpp/cpp     # Lexer implementation
│   │   └── keywords.hpp      # Reserved words
│   ├── parser/
│   │   ├── ast.hpp           # AST node definitions
│   │   ├── parser.hpp/cpp    # Parser implementation
│   │   └── grammar.md        # Formal grammar
│   ├── semantic/
│   │   ├── analyzer.hpp/cpp  # Semantic analysis
│   │   ├── scope.hpp/cpp     # Symbol tables
│   │   └── types.hpp/cpp     # Type system
│   ├── interpreter/
│   │   ├── interpreter.hpp/cpp
│   │   ├── environment.hpp/cpp
│   │   ├── callable.hpp/cpp
│   │   └── evaluator/
│   │       ├── expressions.hpp/cpp
│   │       └── statements.hpp/cpp
│   ├── runtime/
│   │   ├── object.hpp/cpp    # Runtime objects
│   │   ├── gc.hpp/cpp        # Garbage collector
│   │   ├── builtin.hpp/cpp   # Built-in functions
│   │   └── async/
│   │       ├── future.hpp/cpp
│   │       └── event_loop.hpp/cpp
│   └── stdlib/
│       ├── core.dr           # Core library
│       ├── math.dr
│       ├── string.dr
│       ├── io.dr
│       ├── collections.dr
│       └── async.dr
├── tests/
│   ├── unit/
│   │   ├── test_lexer.cpp
│   │   ├── test_parser.cpp
│   │   └── test_interpreter.cpp
│   ├── integration/
│   │   └── test_features.cpp
│   └── samples/              # .dr test files
├── examples/
│   ├── hello.dr
│   ├── fibonacci.dr
│   ├── async_demo.dr
│   └── ... (10+ examples)
└── docs/
    ├── architecture.md
    ├── contributing.md
    └── changelog.md
```

---

## 7. Build System

### Requirements
- C++20 or C++23 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20+
- Testing framework: GoogleTest

### Build Commands
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
ctest  # Run tests
```

---

## 8. Quality Standards

### Code Quality
- Follow C++ Core Guidelines
- RAII for resource management
- Zero memory leaks (verified with sanitizers)
- Minimum 80% test coverage
- Comprehensive documentation

### Error Reporting
- Line and column numbers
- Context snippets
- Suggested fixes
- Stack traces for exceptions

### Performance
- Move semantics where applicable
- String view for tokens
- AST caching
- Profile-guided optimization ready

---

## 9. Future Extensions

### Planned Features
- Module system and package manager
- FFI (Foreign Function Interface)
- WebAssembly target
- IDE plugins (VSCode, Vim)
- Hot reloading for development
- Type-level programming

### Research Areas
- JIT compilation option
- Parallel garbage collection
- Advanced type inference (Hindley-Milner)
- Effect systems

---

## 10. Approval Checklist

- [ ] Syntax design reviewed
- [ ] Architecture validated
- [ ] Implementation phases defined
- [ ] File structure approved
- [ ] Quality standards established

**Status**: Ready for implementation upon approval.

---

*Document Version: 1.0*
*Author: DariX Language Team*
*Date: 2024*
