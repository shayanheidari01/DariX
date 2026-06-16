# DariX Programming Language

**DariX** is a modern, interpreted programming language that intelligently combines the best features of Python and Dart. Built with C++23, it offers a clean syntax, powerful type system, and excellent developer experience.

## Features

### Python-Inspired
- Clean, indentation-based syntax (with optional braces)
- List and dictionary comprehensions
- Decorators (`@` syntax)
- Dynamic typing by default
- Generators and iterators

### Dart-Inspired
- Sound null safety (`?`, `!`, `??` operators)
- First-class functions and closures
- Async/await for asynchronous programming
- Optional named parameters with defaults
- Extension methods
- Mixin-based inheritance

### DariX Innovations
- **Hybrid Type System**: Gradual typing allowing both dynamic and static types
- **Advanced Pattern Matching**: Switch expressions + structural matching
- **Pipe Operator**: Functional programming with `|>`
- **Built-in Immutables**: `let` and `const` declarations
- **Coroutines**: Lightweight concurrency support

## Quick Start

### Building

```bash
cd darix
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running

```bash
# Run a script
./darix examples/hello.drx

# Start REPL
./darix --repl

# Syntax check only
./darix --check script.drx

# Debug mode (show tokens)
./darix --debug script.drx
```

## Syntax Examples

### Variables and Types

```darix
var x = 10              # Dynamic typing
let y: int = 20         # Static typing (immutable)
const PI = 3.14159      # Constant

var nullable: String? = null    # Nullable type
var nonNull: String! = "Hello"  # Non-null assertion
```

### Functions

```darix
# Regular function
fn add(a: int, b: int): int {
    return a + b
}

# Arrow function
fn multiply = (a, b) => a * b

# Async function
async fn fetchData(url: String) {
    var response = await http.get(url)
    return response.json()
}

# Default parameters
fn greet(name: String, greeting: String = "Hello") {
    print("{greeting}, {name}!")
}
```

### Classes and OOP

```darix
class Animal {
    var name: String
    
    fn init(name: String) {
        this.name = name
    }
    
    fn speak(): String => "..."
}

class Dog extends Animal {
    fn init(name: String) {
        super.init(name)
    }
    
    @override
    fn speak(): String => "{name} says Woof!"
}

# Mixins
mixin Flyable {
    fn fly() => print("Flying!")
}

class Bird extends Animal with Flyable {}
```

### Pattern Matching

```darix
match value {
    0 => print("Zero"),
    n if n < 0 => print("Negative: {n}"),
    default => print("Positive: {value}")
}
```

### Functional Programming

```darix
var numbers = [1, 2, 3, 4, 5]

# Pipe operator
var result = numbers
    |> filter(fn(x) => x % 2 == 0)
    |> map(fn(x) => x * x)
    |> reduce(fn(acc, x) => acc + x, 0)

# Comprehensions
var squares = [x * x for x in numbers]
var evenSquares = [x * x for x in numbers if x % 2 == 0]
```

## Project Structure

```
darix/
├── src/
│   ├── lexer/          # Tokenizer (DFA-based)
│   ├── parser/         # Recursive descent parser
│   ├── semantic/       # Type checker and analyzer
│   ├── interpreter/    # Tree-walking interpreter
│   ├── runtime/        # Value types and environment
│   ├── stdlib/         # Standard library
│   └── main.cpp        # CLI entry point
├── tests/              # Test suite
├── docs/               # Documentation
├── examples/           # Sample programs
├── CMakeLists.txt      # Build configuration
└── README.md           # This file
```

## Implementation Status

### Phase 1: Foundation ✅
- [x] Lexer with DFA
- [x] Parser (Recursive Descent + Pratt)
- [x] AST with visitor pattern
- [x] CLI tool with REPL
- [x] Example programs

### Phase 2: Core Interpreter 🚧
- [ ] Expression evaluation
- [ ] Variable management
- [ ] Function calls
- [ ] Basic I/O

### Future Phases
- [ ] Control flow statements
- [ ] Classes and inheritance
- [ ] Type system and semantic analysis
- [ ] Async/await and concurrency
- [ ] Standard library
- [ ] Garbage collector

## License

MIT License - See LICENSE file for details.

## Contributing

Contributions are welcome! Please read our contributing guidelines before submitting PRs.
