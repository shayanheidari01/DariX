# DariX Programming Language

![DariX Logo](DariX.png)

DariX is a dynamically typed programming language implemented in Go with a compiler + bytecode VM design and an interpreter fallback. It features a C-like syntax and supports variables, functions, control flow (if/else, while, for), arrays/maps, a rich set of built-ins, and a Python-like exception system with improved diagnostics.

## Features

*   **Dynamic Typing:** No explicit type annotations required.
*   **Compiler + Bytecode VM:** Fast VM execution with compiler optimizations. Interpreter exists as a fallback.
*   **Optimizations:** Constant folding in the compiler and a safe peephole optimizer for common patterns.
*   **C-like Syntax:** Familiar syntax for C/Java-style developers.
*   **First-Class Functions:** Pass/return functions freely.
*   **Data Structures:** Arrays and Maps.
*   **Control Flow:** `if/else`, `while`, C-style `for`, `break`, `continue`.
*   **Exception Handling:** Python-inspired try/catch/finally with built-in exception types and stack traces.
*   **Modules:** Import `.dax` files, plus native Go modules via `import "go:<name>"`.
*   **Native Modules & FFI:** Built-in `go:fs` (filesystem) and `go:ffi` (reflective foreign function interface).
*   **Enhanced REPL:** Multi-line input with grouping awareness.
*   **CLI Tools:** Backend selection (`auto|vm|interp`), disassembler, stdin support.

## Getting Started

### Prerequisites

*   Go (1.21+ recommended)

### Installation

1.  Clone the repository:
    ```bash
    git clone https://github.com/shayanheidari01/darix.git
    cd darix
    ```
2.  Build the interpreter:
    ```bash
    go build -o darix main.go
    ```
3.  (Optional) Install system-wide:
    ```bash
    sudo cp darix /usr/local/bin/
    ```

To test the installation, run the included test file:

```bash
./darix run test.dax
```

This should output the results of various operations in DariX.

### Running DariX Code

Official source file extension is `.dax`.

CLI commands:

```bash
# Run a file (auto backend tries VM, falls back to interpreter)
darix run path/to/your/script.dax

# Select backend explicitly
darix run --backend=vm path/to/your/script.dax
darix run --backend=interp path/to/your/script.dax

# Read program from stdin
type script.dax | darix run -

# Disassemble bytecode (requires VM-compatible code)
darix disasm path/to/your/script.dax

# REPL
darix repl

# Evaluate a snippet
darix eval "print(1 + 2 * 3)"
```

## Language Guide

### Hello, World!

```dax
print("Hello, World!");
```

### Variables

Variables are declared using the `var` keyword. Assignment to existing variables uses the `=` operator.

```dax
var x = 5;
var name = "DariX";
var isActive = true;

// Re-assigning
x = 10;
name = "New Name";
```

### Data Types

DariX supports the following basic data types:

*   **Integer:** `42`, `-10`
*   **Float:** `3.14`, `-0.001`
*   **Boolean:** `true`, `false`
*   **String:** `"Hello"`, `"DariX"`
*   **Array:** `[1, 2, 3]`, `["a", "b", "c"]`
*   **Map:** `{"key1": "value1", "key2": 100}`
*   **Null:** Represented internally, evaluates to `false` in boolean contexts.

### Operators

*   **Arithmetic:** `+`, `-`, `*`, `/`, `%` (Modulo)
*   **Comparison:** `<`, `>`, `<=`, `>=`, `==`, `!=`
*   **Logical:** `!` (NOT), `&&` (AND), `||` (OR)
*   **String Concatenation:** `+`
*   **Unary:** `-` (Negation), `!` (Logical NOT)

### Control Flow

#### If/Else

```dax
var x = 10;

if (x > 5) {
    print("x is greater than 5");
} else if (x == 5) {
    print("x is equal to 5");
} else {
    print("x is less than 5");
}
```

#### While Loop

```dax
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

#### For Loop (C-style)

```dax
// for (initialization; condition; post-expression)
for (var j = 0; j < 3; j = j + 1) {
    print("Iteration:", j);
}

// Infinite loop example (use break to exit)
var k = 0;
for (;;) {
   if (k >= 2) {
       break; // Exit the loop
   }
   print("Infinite?", k);
   k = k + 1;
}
```

#### Break and Continue

*   `break`: Exits the innermost `while` or `for` loop immediately.
*   `continue`: Skips the rest of the current iteration and jumps to the loop's condition/post-expression.

```dax
for (var i = 0; i < 10; i = i + 1) {
    if (i == 3) {
        continue; // Skip printing 3
    }
    if (i == 7) {
        break; // Stop the loop before reaching 10
    }
    print(i);
}
// Output: 0, 1, 2, 4, 5, 6
```

### Functions

Functions are declared using the `func` keyword.

```dax
// Function declaration
func greet(name) {
    print("Hello,", name);
}

// Calling the function
greet("DariX");

// Function with return value
func add(a, b) {
    return a + b;
}

var sum = add(2, 3);
print("Sum is:", sum); // Output: Sum is: 5

// Function assigned to a variable (Function Literal)
var multiply = func(x, y) {
    return x * y;
};
print("Product:", multiply(4, 5)); // Output: Product: 20
```

### Data Structures

#### Arrays

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

### Modules (Import)

You can split your code into multiple files and import them using the `import` statement. The imported file is executed in the *same* environment, making its variables and functions available.

**math.dax**
```dax
func square(x) {
    return x * x;
}

PI = 3.14159; // Exported variable
```

**main.dax**
```dax
import "math.dax";

print("PI is:", PI);
print("Square of 4 is:", square(4));
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

### Exception Handling

DariX includes a comprehensive exception handling system with try-catch-finally blocks:

```dax
try {
    var result = 10 / 0; // Automatically throws ZeroDivisionError
} catch (ZeroDivisionError e) {
    print("Caught division by zero:", e);
} finally {
    print("This always executes");
}
```

Multiple catch clauses are supported:

```dax
try {
    processData();
} catch (ValueError e) {
    print("Value error:", e);
} catch (TypeError e) {
    print("Type error:", e);
} catch (e) {
    print("Any other exception:", e);
}
```

Built-in exception types include:
*   `ValueError` - Invalid value provided
*   `TypeError` - Incorrect type used
*   `RuntimeError` - General runtime error
*   `IndexError` - Array index out of bounds (thrown automatically)
*   `KeyError` - Map key not found
*   `ZeroDivisionError` - Division by zero (thrown automatically)

Creating and throwing exceptions:

```dax
func validateAge(age) {
    if (age < 0) {
        throw ValueError("Age cannot be negative");
    }
    if (age > 150) {
        throw ValueError("Age seems unrealistic");
    }
    return age;
}

try {
    var validAge = validateAge(-5);
} catch (ValueError e) {
    print("Invalid age:", e);
}
```

### Built-in Functions

DariX comes with a set of built-in functions for common tasks:

*   `print(...args)`: Prints arguments to standard output, separated by spaces, followed by a newline.
*   `len(obj)`: Returns the length of a string, array, or map.
*   `str(obj)`: Converts an integer, float, or boolean to its string representation.
*   `int(obj)`: Converts a string (representing an integer) to an integer.
*   `float(obj)`: Converts a string (representing a float) to a float.
*   `bool(obj)`: Converts an object to a boolean (following truthiness rules).
*   `type(obj)`: Returns the type of an object as a string (e.g., "INTEGER", "STRING").
*   `input([prompt])`: Reads a line of input from the user. An optional prompt string can be provided.
*   `range([start,] stop[, step])`: Creates an array of integers. Mimics Python's `range`.
*   `abs(x)`: Returns the absolute value of a number.
*   `max(...args)`: Returns the maximum value among the provided arguments.
*   `min(...args)`: Returns the minimum value among the provided arguments.
*   `sum(array)`: Returns the sum of elements in an array.
*   `reverse(obj)`: Returns the reverse of a string or array.
*   `sorted(array)`: Returns a new sorted array.
*   `upper(str)`: Converts a string to uppercase.
*   `lower(str)`: Converts a string to lowercase.
*   `trim(str)`: Removes leading and trailing whitespace from a string.
*   `append(array, ...values)`: Returns a new array with values appended to the end of the original array.
*   `contains(array, value)`: Checks if an array contains a specific value.
*   `pow(base, exp)`: Calculates `base` raised to the power of `exp`.
*   `clamp(val, min, max)`: Clamps `val` to be within the range [`min`, `max`].
*   `now()`: Returns the current date and time as a string (RFC3339 format).
*   `timestamp()`: Returns the current Unix timestamp as an integer.
*   `exit([code])`: Terminates the program immediately. An optional integer exit code can be provided.
*   `Exception([type,] message)`: Creates a new exception object.
*   `ValueError(message)`: Creates a ValueError exception.
*   `TypeError(message)`: Creates a TypeError exception.
*   `RuntimeError(message)`: Creates a RuntimeError exception.
*   `IndexError(message)`: Creates an IndexError exception.
*   `KeyError(message)`: Creates a KeyError exception.
*   `ZeroDivisionError(message)`: Creates a ZeroDivisionError exception.

## Contributing

Contributions are welcome! Please feel free to submit issues, fork the repository, and send pull requests.

## License

This project is licensed under the Apache License. See the `LICENSE` file for details.
