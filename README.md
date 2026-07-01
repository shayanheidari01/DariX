<p align="center">
  <img src="docs/DariX.png" alt="DariX Logo" width="200"/>
</p>

<h1 align="center">DariX</h1>

<p align="center">
  <strong>A high-performance, dynamically-typed programming language</strong>
</p>

<p align="center">
  <a href="#quick-start">Quick Start</a> •
  <a href="#features">Features</a> •
  <a href="#examples">Examples</a> •
  <a href="#modules">Modules</a> •
  <a href="#building">Building</a> •
  <a href="docs/language.md">Language Reference</a> •
  <a href="docs/modules.md">Module Reference</a> •
  <a href="docs/architecture.md">Architecture</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-1.0.1-blue" alt="Version">
  <img src="https://img.shields.io/badge/C%2B%2B-17-green" alt="C++17">
  <img src="https://img.shields.io/badge/license-MIT-yellow" alt="License">
  <img src="https://img.shields.io/badge/tests-593-brightgreen" alt="Tests">
  <img src="https://img.shields.io/badge/modules-21-orange" alt="Modules">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey" alt="Platforms">
</p>

---

## Quick Start

### Install

```bash
git clone https://github.com/shayanheidari01/DariX.git
cd DariX/cpp-src
cmake -S . -B build
cmake --build build
```

### Run

```bash
# Run a script
./build/darix run script.dax

# Interactive REPL
./build/darix repl

# Evaluate expression
./build/darix eval "print(1 + 2)"
```

### Hello World

Create `hello.dax`:
```dax
print("Hello, World!")
```

```bash
darix run hello.dax
# Output: Hello, World!
```

---

## Features

### Core Language
| Feature | Status |
|---------|--------|
| Dynamic typing | ✅ |
| Variables & assignment | ✅ |
| Arithmetic operators | ✅ |
| String operations | ✅ |
| Conditionals (if/elif/else) | ✅ |
| While loops | ✅ |
| For loops | ✅ |
| Functions & recursion | ✅ |
| Lambda expressions | ✅ |
| Closures | ✅ |
| Classes & OOP | ✅ |
| Decorators | ✅ |
| Exception handling | ✅ |
| Import system | ✅ |
| Comments (// and /\* \*/) | ✅ |
| Short-circuit evaluation | ✅ |

### Standard Library (21 Modules)

| Module | Functions | Description |
|--------|-----------|-------------|
| `math` | 27 | Mathematical functions |
| `string` | 37 | String manipulation |
| `array` | 28 | Array operations |
| `map` | 22 | Map operations |
| `set` | 27 | Set operations |
| `queue` | 25 | FIFO queue |
| `stack` | 28 | LIFO stack |
| `linkedlist` | 38 | Linked list operations |
| `tree` | 22 | Tree operations |
| `graph` | 22 | Graph operations |
| `json` | 3 | JSON parsing/serialization |
| `fs` | 22 | File system operations |
| `net` | 9 | Networking (TCP/UDP/HTTP) |
| `crypto` | 14 | Cryptographic functions |
| `datetime` | 30 | Date and time operations |
| `random` | 16 | Random number generation |
| `regex` | 13 | Regular expressions |
| `io` | 20 | Input/output operations |
| `os` | 15 | Operating system interface |
| `encoding` | 17 | Encoding/decoding |
| **Total** | **407** | |

### Architecture
- **Lexer**: Single-pass scanner with position tracking
- **Parser**: Pratt (top-down operator precedence) parser
- **Compiler**: AST-to-bytecode with constant folding and peephole optimization
- **VM**: Stack-based virtual machine with 30 opcodes
- **Interpreter**: Tree-walking fallback for full feature support
- **JIT**: Hot-path optimization (threshold: 100 executions)

---

## Examples

### Variables & Arithmetic
```dax
var x = 42
var y = 3.14
var name = "DariX"
print(x + y)      // 45.14
print(x * 2)      // 84
```

### Control Flow
```dax
var score = 85
if (score >= 90) {
    print("A")
} elif (score >= 80) {
    print("B")
} else {
    print("C")
}

for (var i = 0; i < 5; i = i + 1) {
    print(i)
}
```

### Functions & Lambdas
```dax
func factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}
print(factorial(5))  // 120

var double = lambda x: x * 2
print(double(21))    // 42
```

### Classes
```dax
class Animal {
    var name = ""
    func __init__(name) { self.name = name }
    func speak() { return self.name + " speaks" }
}

var cat = Animal("Cat")
print(cat.speak())  // Cat speaks
```

### Data Structures
```dax
// Arrays
var arr = [1, 2, 3, 4, 5]
print(arr.filter(lambda x: x > 3))  // [4, 5]
print(arr.map(lambda x: x * 10))    // [10, 20, 30, 40, 50]

// Maps
var m = {"name": "DariX", "version": 1}
print(m["name"])  // DariX

// Sets
import set
var s = set.from_array([1, 2, 2, 3])
print(s)  // [1, 2, 3]
```

### Exception Handling
```dax
try {
    var result = 10 / 0
} catch (ZeroDivisionError e) {
    print("Error:", e)
} finally {
    print("cleanup")
}
```

### File I/O
```dax
import fs
import json

// Write JSON
var data = {"name": "Alice", "scores": [95, 87, 92]}
fs.write("data.json", json.stringify(data, 2))

// Read and parse
var content = fs.read("data.json")
var parsed = json.parse(content)
print(parsed["name"])  // Alice
```

### HTTP Requests
```dax
import net
import json

var resp = net.http_get("http://httpbin.org/get")
print(resp["status"])  // 200
var data = json.parse(resp["body"])
```

### Math & Science
```dax
import math

print(math.sqrt(16))        // 4
print(math.pi())           // 3.14159
print(math.sin(math.pi()/2))  // 1
print(math.factorial(5))   // 120 (via recursive function)
```

---

## Project Structure

```
DariX/
├── cpp-src/                 # C++ implementation
│   ├── CMakeLists.txt
│   ├── include/darix/       # Headers
│   │   ├── ast.hpp
│   │   ├── code.hpp
│   │   ├── compiler.hpp
│   │   ├── interpreter.hpp
│   │   ├── lexer.hpp
│   │   ├── object.hpp
│   │   ├── parser.hpp
│   │   ├── token.hpp
│   │   ├── vm.hpp
│   │   └── native/          # Native module headers
│   └── src/                 # Source files
│       ├── main.cpp
│       └── native/          # Native module implementations
├── examples/                # Example scripts
├── tests/                   # Test scripts
├── benchmarks/              # Performance benchmarks
├── docs/                    # Documentation
│   ├── language.md
│   ├── modules.md
│   ├── building.md
│   ├── cli.md
│   ├── architecture.md
│   └── tutorial.md
└── README.md
```

---

## Documentation

- [Language Reference](docs/language.md) — Complete language syntax
- [Module Reference](docs/modules.md) — All 21 native modules
- [Build Guide](docs/building.md) — Build instructions for all platforms
- [CLI Reference](docs/cli.md) — Command-line interface
- [Architecture](docs/architecture.md) — Internal design and structure
- [Tutorial](docs/tutorial.md) — Step-by-step learning guide

---

## Building

### Requirements
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+

### Build
```bash
cd cpp-src
cmake -S . -B build
cmake --build build
```

### Platforms
| Platform | Compiler | Status |
|----------|----------|--------|
| Windows (MSYS2) | MinGW GCC | ✅ |
| Windows (VS) | MSVC 2017+ | ✅ |
| Linux | GCC/Clang | ✅ |
| macOS | Clang | ✅ |

See [Build Guide](docs/building.md) for detailed instructions.

---

## Test Results

```
Language features: 133 passed
Math module:        27 passed
String module:      37 passed
Array module:       28 passed
Map module:         22 passed
Set module:         27 passed
Queue module:       25 passed
Stack module:       28 passed
Linked List:        38 passed
Tree module:        22 passed
Graph module:       22 passed
JSON module:        28 passed
Filesystem:         22 passed
Network:             9 passed
Crypto module:      14 passed
DateTime:           30 passed
Random:             16 passed
Regex:              13 passed
IO module:          20 passed
OS module:          15 passed
Encoding:           17 passed
─────────────────────────────
TOTAL:             593 passed
```

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

<p align="center">
  Built with ❤️ by <a href="https://github.com/shayanheidari01">shayanheidari01</a>
</p>
