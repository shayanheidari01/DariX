# DariX Programming Language

![DariX Logo](DariX.png)

**DariX** is a modern, powerful, and comprehensive programming language inspired by Python and Dart, implemented in Go. It features C-like syntax, VM + Compiler execution system with Interpreter fallback, and advanced capabilities including OOP, Exception Handling, Native Libraries, and HTTP Server.

## 🌟 Key Features

### 🚀 **Advanced Architecture:**
- **Bytecode VM + Compiler:** Fast execution with compiler optimizations
- **Interpreter Fallback:** Complete support for all features
- **Auto Backend Selection:** Automatic selection of best backend
- **Performance Optimizations:** Object pooling, constant folding, peephole optimizer

### 💻 **Complete Programming Language:**
- **Dynamic Typing:** No need for explicit type definitions
- **Object-Oriented Programming:** Classes, objects, methods, constructors
- **First-Class Functions:** Functions as first-class values
- **Exception Handling:** Complete try-catch-finally system
- **Module System:** Import files and native modules

### 🌐 **Network and Web Capabilities:**
- **HTTP Server:** Complete HTTP server with routing and middleware
- **HTTP Client:** HTTP client with support for all methods
- **WebSocket:** Real-time communications
- **Socket Programming:** Low-level network programming
- **SMTP:** Email sending
- **DNS:** DNS operations and validation

### 📚 **Native Libraries:**
- **13 Native Libraries** with 100+ functions
- **Math, String, JSON, HTTP, Crypto, Time, OS, Regex**
- **Path, Random, URL, Base, Collections**
- **High Performance** with Go native implementation

### 🛠 **Development Tools:**
- **Enhanced REPL:** Advanced interactive environment with history and completion
- **VS Code Extension:** Complete IDE support
- **CLI Tools:** Various command-line tools
- **Comprehensive Testing:** Complete test suite
- **CI/CD Pipeline:** Automated build pipeline

## 🚀 Quick Start

### 📋 Prerequisites
- **Go 1.21+** (recommended)
- **Git** for cloning the repository

### 📥 Installation

#### Method 1: Automatic Installation (Recommended)

**Termux Android:**
```bash
wget -qO- https://raw.githubusercontent.com/shayanheidari01/DariX/refs/heads/main/install.sh | bash
```

**Linux:**
```bash
wget -qO- https://raw.githubusercontent.com/shayanheidari01/DariX/refs/heads/main/install.sh | sudo bash
```

#### Method 2: Build from Source
```bash
# Clone repository
git clone https://github.com/shayanheidari01/DariX.git
cd DariX

# Build
go build -o darix main.go

# Test installation
./darix run test.dax
```

#### Method 3: Using Makefile
```bash
# Build for current platform
make build

# Build for all platforms
make build-all

# Run tests
make test

# Start REPL
make repl
```

#### Method 4: System Installation
```bash
# Linux/macOS
sudo cp darix /usr/local/bin/

# Windows
copy darix.exe C:\Windows\System32\
```

### 🎯 Running DariX Code

**Official file extension:** `.dax`

#### CLI Commands:
```bash
# Run file (auto backend)
darix run script.dax

# Explicit backend selection
darix run --backend=vm script.dax      # Faster
darix run --backend=interp script.dax  # More complete
darix run --backend=auto script.dax    # Smart

# Run from stdin
echo 'print("Hello!")' | darix run -

# Disassemble bytecode
darix disasm script.dax

# Interactive environment (REPL)
darix repl

# Direct code execution
darix eval 'print(2 + 3 * 4)'

# Show version
darix version

# Help
darix help
```

#### Security and Sandboxing:
```bash
# Restrict access
darix run --allow=go:math,go:string script.dax
darix run --deny=go:os,go:fs script.dax
darix run --fs-root=/safe/path script.dax
darix run --fs-ro script.dax
darix run --cpu=1000000 script.dax
```

## 📖 DariX Language Guide

### 👋 Hello World!

```dax
print("Hello World!");
print("Hello, DariX World!");
```

### 🔢 Variables

Variables are declared using the `var` keyword. To change a variable's value, use the `=` operator.

```dax
// Variable declarations
var x = 5;
var name = "DariX";
var isActive = true;
var price = 99.99;
var items = [1, 2, 3];
var person = {"name": "Ali", "age": 25};

// Changing variable values
x = 10;
name = "New Name";
isActive = false;
```

### 🎯 Data Types

DariX supports the following data types:

#### Primitive Types:
- **Integer:** `42`, `-10`, `0`
- **Float:** `3.14`, `-0.001`, `2.5`
- **Boolean:** `true`, `false`
- **String:** `"Hello"`, `"DariX"`, `'Single quotes'`
- **Null:** `null` - represents absence of value

#### Composite Types:
- **Array:** `[1, 2, 3]`, `["a", "b", "c"]`, `[1, "two", true]`
- **Map/Object:** `{"key": "value", "age": 30}`

```dax
// Examples of different data types
var number = 42;                    // Integer
var pi = 3.14159;                   // Float
var isReady = true;                 // Boolean
var message = "Test message";       // String
var empty = null;                   // Null
var colors = ["red", "blue", "green"]; // Array
var student = {                     // Map
    "name": "Mohammad",
    "grade": 18.5,
    "passed": true
};
```

### ⚡ عملگرها (Operators)

#### عملگرهای ریاضی:
```dax
var a = 10, b = 3;
print(a + b);    // جمع: 13
print(a - b);    // تفریق: 7
print(a * b);    // ضرب: 30
print(a / b);    // تقسیم: 3.333...
print(a % b);    // باقیمانده: 1
```

#### عملگرهای مقایسه:
```dax
var x = 5, y = 10;
print(x < y);    // کمتر از: true
print(x > y);    // بزرگتر از: false
print(x <= 5);   // کمتر یا مساوی: true
print(x >= 5);   // بزرگتر یا مساوی: true
print(x == 5);   // مساوی: true
print(x != y);   // نامساوی: true
```

#### عملگرهای منطقی:
```dax
var isTrue = true, isFalse = false;
print(!isTrue);           // نقیض (NOT): false
print(isTrue && isFalse); // و منطقی (AND): false
print(isTrue || isFalse); // یا منطقی (OR): true
```

#### عملگرهای رشته:
```dax
var first = "سلام";
var second = " دنیا";
print(first + second);    // الحاق رشته: "سلام دنیا"
```

#### عملگرهای یکی (Unary):
```dax
var num = 5;
print(-num);     // منفی کردن: -5
print(!true);    // نقیض منطقی: false
}
// حلقه با شرط پیچیده
var sum = 0;
var num = 1;
while (sum < 100) {
    sum = sum + num;
    num = num + 1;
}
print("مجموع:", sum);
```

#### For Loops (C-style):
```dax
// Simple loop
for (var j = 0; j < 5; j = j + 1) {
    print("Iteration:", j);
}

// Loop with different steps
for (var k = 10; k > 0; k = k - 2) {
    print("Countdown:", k);
}

// Infinite loop (use break to exit)
var counter = 0;
for (;;) {
    if (counter >= 3) {
        break; // Exit loop
    }
    print("Infinite?", counter);
    counter = counter + 1;
}
```

#### Break and Continue:
```dax
// Using break and continue
for (var i = 0; i < 10; i = i + 1) {
    if (i == 3) {
        continue; // Skip number 3
    }
    if (i == 7) {
        break; // Stop loop before reaching 10
    }
    print(i);
}
// Output: 0, 1, 2, 4, 5, 6

// More complex example
var numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
for (var i = 0; i < len(numbers); i = i + 1) {
    if (numbers[i] % 2 == 0) {
        continue; // Skip even numbers
    }
    if (numbers[i] > 7) {
        break; // Stop at numbers greater than 7
    }
    print("Odd number:", numbers[i]);
}
// آرایه‌ها در داریکس از نوع ordered collection هستند

Arrays are ordered collections of values.

```dax
var numbers = [1, 2, 3, 4];
var mixed = [1, "hello", true];

print("First number:", numbers[0]); // Access by index
print("Length:", len(numbers)); // Built-in len function

numbers = append(numbers, 5); // Append element(s)
print("After append:", numbers);

var reversed_nums = reverse(numbers); // Built-in reverse function
print("Reversed:", reversed_nums);

var range_array = range(5); // Creates [0, 1, 2, 3, 4]
print("Range 0-4:", range_array);

var range_array2 = range(2, 8); // Creates [2, 3, 4, 5, 6, 7]
print("Range 2-7:", range_array2);

var range_array3 = range(0, 10, 2); // Creates [0, 2, 4, 6, 8]
print("Range 0-9 step 2:", range_array3);
```

#### Maps

Maps are collections of key-value pairs.

```dax
var person = {"name": "Alice", "age": 30};
print("Name:", person["name"]); // Access by key

person["city"] = "Wonderland"; // Add/update key-value pair
print("Person map:", person);

print("Map size:", len(person)); // Built-in len function
```

### 📦 ماژول‌ها (Modules)

می‌توانید کد خود را در چندین فایل تقسیم کنید و با استفاده از دستور `import` آن‌ها را وارد کنید.

#### مثال ماژول ریاضی:
**math.dax**
```dax
// توابع ریاضی
func square(x) {
    return x * x;
}

func cube(x) {
    return x * x * x;
}

func power(base, exp) {
    var result = 1;
    for (var i = 0; i < exp; i = i + 1) {
        result = result * base;
    }
    return result;
}

// ثابت‌های ریاضی
PI = 3.14159;
E = 2.71828;
```

**main.dax**
```dax
import "math.dax";

print("عدد پی:", PI);
print("مربع 4:", square(4));
print("مکعب 3:", cube(3));
print("2 به توان 8:", power(2, 8));
```

#### مثال ماژول کاربردی:
**utils.dax**
```dax
// توابع کمکی
func isEven(n) {
    return n % 2 == 0;
}

func isOdd(n) {
    return n % 2 != 0;
}

func max(a, b) {
    if (a > b) {
        return a;
    }
    return b;
}

func min(a, b) {
    if (a < b) {
        return a;
    }
    return b;
}
```

**app.dax**
```dax
import "utils.dax";

var numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

for (var i = 0; i < len(numbers); i = i + 1) {
    var num = numbers[i];
    if (isEven(num)) {
        print(num, "زوج است");
    } else {
        print(num, "فرد است");
    }
}

print("بیشترین:", max(15, 23));
print("کمترین:", min(15, 23));
```

## Native Modules & FFI (Go interop)

You can bring Go-powered functionality into DariX in two complementary ways:

- Native modules via `import "go:<name>"`
- Reflective FFI for direct calls into registered Go functions

Today two native modules ship by default:

- `go:fs` — simple filesystem utilities
- `go:ffi` — reflective Foreign Function Interface entry point

Notes:

- Importing `go:<name>` currently injects the module's functions directly into the current environment (no namespace object yet). Use prefix-friendly names (e.g., `fs_read`) to avoid collisions.
- All builtins return either a normal value or an `Error` object on failure; handle errors as needed.

### Using go:fs (filesystem)

```dax
import "go:fs";

fs_write("hello.txt", "salam dari DariX!");
print("exists?", fs_exists("hello.txt"));   // true
print("content:", fs_read("hello.txt"));    // salam dari DariX!
```

APIs:

- `fs_read(path: string) -> string | Error`
- `fs_write(path: string, data: string) -> true | Error`
- `fs_exists(path: string) -> bool | Error`

### Using go:ffi (reflective FFI)

Register your Go function(s) during program init, then call them with `ffi_call` from DariX.

Register in Go (host):

```go
// cmd/register_math.go
package main

import (
    "darix/internal/native"
    "math"
)

func init() {
    native.RegisterFFI("math.Sqrt", math.Sqrt)
}
```

Call from DariX:

```dax
import "go:ffi";

var x = 9;
var r = ffi_call("math.Sqrt", x);
print("sqrt:", r); // 3
```

Type mapping (Go ⇄ DariX):

- Integers ⇄ `INTEGER` (auto-convert among int/uint widths where possible)
- Floats ⇄ `FLOAT`
- Booleans ⇄ `BOOLEAN`
- Strings ⇄ `STRING`
- Null → nil for interface/pointer/slice/map/func targets; nil/pointers → `null`
- Multi-return `(T, error)`: if `error != nil` → `Error`, else return `T`

Limitations (current): arrays/maps are not yet auto-mapped to Go slices/maps; extend `ffi.go` if needed.

### Building your own native module (fast path)

For hot paths, native modules avoid reflection. Implement `args ...object.Object -> object.Object` builtins and register them under a module name.

```go
// internal/native/mycalc.go
package native

import "darix/object"

func init() {
    Register("mycalc", map[string]*object.Builtin{
        "calc_add": {Fn: calcAdd},
    })
}

func calcAdd(args ...object.Object) object.Object {
    if len(args) != 2 {
        return object.NewError("calc_add: expected 2 args")
    }
    a, ok1 := args[0].(*object.Integer)
    b, ok2 := args[1].(*object.Integer)
    if !ok1 || !ok2 {
        return object.NewError("calc_add: both args must be integers")
    }
    return object.NewInteger(a.Value + b.Value)
}
```

Use it in DariX:

```dax
import "go:mycalc";

print(calc_add(2, 40)); // 42
```

### Namespacing & safety

- Namespacing: for now, functions are injected directly. You can prefix names (e.g., `fs_*`, `calc_*`). A future update may keep functions under a module object (e.g., `fs.read`).
- Safety: `go:fs` and `go:ffi` expose host capabilities. For sandboxed environments, add validation layers or restrict which modules are registered.

## ⚠️ **مدیریت خطا (Exception Handling)**

داریکس دارای سیستم کامل مدیریت خطا با الهام از Python است:

```dax
// خطاهای خودکار
try {
    var result = 10 / 0; // خطای تقسیم بر صفر
} catch (ZeroDivisionError e) {
    print("خطای تقسیم بر صفر:", e);
} finally {
    print("این همیشه اجرا می‌شود");
}

// ایجاد و پرتاب خطاهای سفارشی
func validateAge(age) {
    if (age < 0) {
        throw ValueError("سن نمی‌تواند منفی باشد");
    }
    return age;
}

try {
    var validAge = validateAge(-5);
} catch (ValueError e) {
    print("سن نامعتبر:", e);
}
```

## 🏗️ **برنامه‌نویسی شیءگرا (OOP)**

```dax
class Person {
    func __init__(self, name, age) {
        self.name = name;
        self.age = age;
    }
    
    func greet(self) {
        print("سلام، من " + self.name + " هستم.");
    }
}

var person = Person("علی", 25);
person.greet();
```

## 📚 **Built-in Functions**

DariX comes with a comprehensive set of built-in functions:

### Core Functions:
- `print(...args)` - Print to output
- `len(obj)` - Length of string, array, or map
- `type(obj)` - Object type
- `input([prompt])` - Get user input

### Type Conversion:
- `str(obj)`, `int(obj)`, `float(obj)`, `bool(obj)`

### Math Functions:
- `abs(x)`, `max(...args)`, `min(...args)`, `sum(array)`, `pow(base, exp)`

### Array Functions:
- `range([start,] stop[, step])`, `reverse(obj)`, `sort(array)`, `append(array, ...values)`

### Map Functions:
- `keys(map)`, `values(map)`, `items(map)`

### String Functions:
- `upper(str)`, `lower(str)`, `trim(str)`

### Utility Functions:
- `now()`, `timestamp()`, `exit([code])`

## 🏗️ **Build and Testing**

### Build Project:
```bash
# Build for current platform
go build -o darix .

# Or use Makefile
make build
make build-all  # All platforms
```

### Running Tests:
```bash
# Run test suite
./darix run tests/test_runner.dax

# Comprehensive tests
make test
make test-comprehensive
```

### Project Structure:
```
DariX/
├── ast/           # Abstract Syntax Tree
├── compiler/      # Bytecode compiler
├── vm/            # Virtual machine
├── interpreter/   # Tree-walking interpreter
├── lexer/         # Lexical analyzer
├── parser/        # Parser implementation
├── object/        # Object system
├── repl/          # Interactive environment
├── internal/      # Internal modules
├── examples/      # Example programs
├── tests/         # Test suite
└── main.go        # Main entry point
```

## 🤝 **Join the DariX Community - مشارکت در پروژه**

We warmly welcome contributions from developers around the world! DariX is an open-source project that thrives on community collaboration.

### 🌟 **How to Contribute:**

#### 🐛 **Report Issues & Bugs:**
- Found a bug? [Create an issue](https://github.com/shayanheidari01/DariX/issues/new)
- Suggest new features or improvements
- Report documentation errors or unclear sections

#### 💻 **Code Contributions:**
- Fork the repository
- Create a feature branch: `git checkout -b feature/amazing-feature`
- Make your changes and add tests
- Commit with clear messages: `git commit -m 'Add amazing feature'`
- Push to your branch: `git push origin feature/amazing-feature`
- Open a Pull Request with detailed description

#### 📚 **Documentation & Examples:**
- Improve existing documentation
- Add new examples and tutorials
- Translate documentation to other languages
- Create video tutorials or blog posts

#### 🧪 **Testing & Quality Assurance:**
- Write unit tests for new features
- Test on different platforms (Windows, Linux, macOS, Android)
- Performance testing and benchmarking
- Security testing and vulnerability reports

### 🎯 **Areas We Need Help With:**

- **Native Libraries:** Expand the collection of Go-native modules
- **Performance Optimization:** VM improvements and compiler optimizations
- **IDE Support:** Enhance VS Code extension, create plugins for other editors
- **Mobile Development:** Android app development with DariX
- **Web Framework:** Build a comprehensive web framework
- **Package Manager:** Create a package management system
- **Standard Library:** Expand built-in functions and utilities

### 🏆 **Recognition:**
Contributors will be:
- Listed in our [CONTRIBUTORS.md](CONTRIBUTORS.md) file
- Mentioned in release notes
- Given credit in documentation
- Invited to join our core team for significant contributions

### 💬 **Get in Touch:**
- **GitHub Discussions:** Share ideas and ask questions
- **Issues:** Technical problems and feature requests
- **Email:** Contact maintainers for private discussions

### 🌍 **International Contributors Welcome:**
We especially encourage contributions from:
- **Persian/Farsi speakers** for localization
- **Developers from Iran** and Persian-speaking countries
- **Students and educators** for educational content
- **Open source enthusiasts** from all backgrounds

**Let's build the future of programming languages together! 🚀**

---

**Your contribution to the DariX project is highly valuable! We especially invite Iranian and Persian-speaking developers to participate in the development of this programming language.**

## 📄 **License**

This project is licensed under the Apache License.

## 🎯 **Feature Summary**

✅ **Complete Language:** Dynamic typing, OOP, Exception handling  
✅ **High Performance:** VM + Compiler with advanced optimizations  
✅ **Network & Web:** HTTP Server, WebSocket, SMTP, DNS  
✅ **13 Native Libraries:** Math, JSON, Crypto, and more  
✅ **Development Tools:** Enhanced REPL, VS Code Extension  
✅ **Comprehensive Testing:** 40+ automated tests with 100% success  
✅ **CI/CD:** Automated pipeline and multi-platform support  
✅ **Complete Documentation:** English and Persian guides  

**DariX is ready for use in real-world projects! 🚀**
