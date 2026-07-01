# DariX — C++ Implementation

A high-performance, dynamically-typed programming language written in C++17.

## Quick Start

```bash
cd cpp-src
mkdir build && cd build
cmake .. -G "MinGW Makefiles"   # or "Visual Studio 17 2022" on Windows
cmake --build .
./darix run script.dax
```

## Language Features

### Variables
```dax
var x = 42
var name = "DariX"
var pi = 3.14
var flag = true
var nothing = null
```

### Operators
```dax
// Arithmetic: + - * / %
// Comparison: == != < > <= >=
// Logical: && || !
// Keywords: and or not
// Membership: in
// Identity: is
```

### Control Flow
```dax
if (x > 10) {
    print("big")
} elif (x > 5) {
    print("medium")
} else {
    print("small")
}

while (condition) { /* ... */ }

for (var i = 0; i < 10; i = i + 1) { /* ... */ }

// Break and continue work in loops
```

### Functions & Lambdas
```dax
func add(a, b) { return a + b }

var double = lambda x: x * 2

func apply(fn, val) { return fn(val) }
apply(double, 5)  // 10
```

### Classes
```dax
class Person {
    var name = ""
    var age = 0
    
    func __init__(name, age) {
        self.name = name
        self.age = age
    }
    
    func greet() {
        return "Hi, I'm " + self.name
    }
}

var p = Person("Alice", 30)
print(p.greet())
```

### Exception Handling
```dax
try {
    var x = 1 / 0
} catch (ZeroDivisionError e) {
    print("Error:", e)
} finally {
    print("cleanup")
}

throw ValueError("bad input")
assert x > 0, "x must be positive"
```

### Data Structures
```dax
// Arrays
var arr = [1, 2, 3]
arr[0] = 10
print(len(arr))

// Maps
var m = {"key": "value", "num": 42}
m["new"] = "entry"

// Lambda functional operations
print(arr.filter(lambda x: x > 2))
print(arr.map(lambda x: x * 2))
print(arr.reduce(lambda a, b: a + b, 0))
```

## Native Modules

### `import math`
```dax
import math
math.sqrt(16)       // 4
math.pow(2, 10)     // 1024
math.pi()           // 3.14159
math.sin(0)         // 0
math.ceil(3.2)      // 4
math.floor(3.8)     // 3
math.random()       // 0.0 to 1.0
```

### `import string`
```dax
import string
string.upper("hello")           // "HELLO"
string.split("a,b,c", ",")     // ["a", "b", "c"]
string.join(["x","y"], "-")    // "x-y"
string.replace("hello", "l", "r")  // "herro"
string.slice("hello", 1, 4)    // "ell"
string.count("banana", "a")    // 3
string.to_title("hello world") // "Hello World"
```

### `import array`
```dax
import array
array.filter([1,2,3,4], lambda x: x > 2)  // [3, 4]
array.map([1,2,3], lambda x: x * 2)        // [2, 4, 6]
array.reduce([1,2,3], lambda a, b: a + b, 0) // 6
array.sort_by([3,1,2], lambda x: x)        // [1, 2, 3]
array.group_by([1,2,3], lambda x: x % 2)   // {1: [1,3], 0: [2]}
```

### `import map`
```dax
import map
var m = {"a": 1, "b": 2}
map.keys(m)                    // ["a", "b"]
map.values(m)                  // [1, 2]
map.has_key(m, "a")            // true
map.merge(m, {"c": 3})        // {"a": 1, "b": 2, "c": 3}
map.filter(m, lambda k, v: v > 1)  // {"b": 2}
```

### `import set`
```dax
import set
var s = set.from_array([1, 2, 2, 3])
set.union(s, [3, 4])          // [1, 2, 3, 4]
set.intersection(s, [2, 3, 4]) // [2, 3]
set.difference([1,2,3], [2,3,4]) // [1]
```

### `import queue`
```dax
import queue
var q = queue.new()
var q1 = queue.enqueue(q, "a")
var q2 = queue.enqueue(q1, "b")
var [front, rest] = queue.dequeue(q2)  // front="a", rest=["b"]
```

### `import stack`
```dax
import stack
var s = stack.new()
var s1 = stack.push(s, 10)
var s2 = stack.push(s1, 20)
var [top, rest] = stack.pop(s2)  // top=20, rest=[10]
```

### `import linkedlist`
```dax
import linkedlist
var ll = linkedlist.from_array([1, 2, 3, 4, 5])
linkedlist.head(ll)      // 1
linkedlist.tail(ll)      // [2, 3, 4, 5]
linkedlist.cons(ll, 0)   // [0, 1, 2, 3, 4, 5]
```

### `import tree`
```dax
import tree
var t = tree.node(1, [tree.leaf(2), tree.leaf(3)])
tree.size(t)            // 3
tree.preorder(t)        // [1, 2, 3]
tree.map_tree(t, lambda x: x * 10)  // [10, 20, 30]
```

### `import graph`
```dax
import graph
var g = graph.from_edges([[0, 1], [1, 2], [2, 0]])
graph.bfs(g, 0)                      // [0, 1, 2]
graph.shortest_path(g, 0, 2)         // [0, 1, 2]
graph.topological_sort(dag)          // topological order
```

### `import json`
```dax
import json
var data = json.parse("{\"name\": \"Alice\"}")
var str = json.stringify(data, 2)    // pretty-printed
json.is_valid("{\"a\": 1}")         // true
```

### `import fs`
```dax
import fs
fs.write("output.txt", "Hello!")
var content = fs.read("output.txt")
fs.mkdir("data")
var files = fs.list_dir(".")
fs.copy("src.txt", "dst.txt")
```

### `import net`
```dax
import net
var resp = net.http_get("http://example.com")
print(resp["status"])  // 200
var fd = net.tcp_connect("host", 80)
net.tcp_send(fd, "data")
```

### `import crypto`
```dax
import crypto
crypto.md5("hello")           // hash string
crypto.sha256("hello")        // hash string
crypto.base64_encode("data")  // base64
crypto.uuid()                 // UUID v4
```

### `import datetime`
```dax
import datetime
var now = datetime.now()
datetime.format(now, "%Y-%m-%d %H:%M:%S")
datetime.add_days(now, 7)
datetime.diff_days(date2, date1)
```

### `import random`
```dax
import random
random.seed(42)           // reproducible
random.int(100)           // 0-99
random.choice(["a","b"])  // random element
random.shuffle([1,2,3])   // shuffled
```

### `import regex`
```dax
import regex
regex.test("\\d+", "abc123")        // true
regex.match("\\d+", "abc123")       // "123"
regex.matches("\\d+", "a1b2")       // ["1", "2"]
regex.replace_all("\\d+", "a1b2", "X")  // "aXbX"
```

### `import io`
```dax
import io
io.print("Hello, World!")
io.format("Hello, {0}!", "Alice")  // "Hello, Alice!"
print(io.table(["Name","Age"], [["Alice",30]]))
```

### `import os`
```dax
import os
os.platform()        // "windows" / "linux" / "darwin"
os.hostname()        // computer name
os.cpu_count()       // number of cores
var mem = os.memory_info()
```

### `import encoding`
```dax
import encoding
encoding.base64_encode("Hello")      // "SGVsbG8="
encoding.rot13("Hello")              // "Uryyb"
encoding.caesar_encode("abc", 3)     // "def"
encoding.url_encode("hello world")   // "hello%20world"
```

## CLI Commands

```bash
darix run script.dax              # Run a file
darix eval "print(1 + 2)"        # Evaluate expression
darix disasm script.dax           # Disassemble bytecode
darix repl                        # Interactive REPL
darix version                     # Show version
```

## Building from Source

### Requirements
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+

### Build
```bash
cd cpp-src
cmake -S . -B build
cmake --build build
```

### Optional Dependencies
- **OpenSSL** — for crypto module (uses pure C++ fallback if unavailable)
- **libcurl** — for HTTP client (uses raw sockets if unavailable)

## Architecture

```
Lexer → Parser → AST → Compiler → Bytecode → VM
                                ↓
                          Interpreter (fallback)
```

The VM executes compiled bytecode. When a feature isn't supported by the VM, the interpreter falls back automatically.

## License

Apache License 2.0
