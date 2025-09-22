
# DariX Programming Language

**DariX** is a dynamically typed, interpreted programming language implemented in Go. It features a familiar C-like syntax and provides core programming constructs such as variables, functions, control flow, data structures, and a rich standard library. Inspired by Python, DariX also includes a robust exception handling system with try-catch-finally blocks.

---

## Key Features

- **Dynamic Typing:** No need to declare variable types explicitly.
- **Interpreted:** Run your code directly without compilation.
- **C-like Syntax:** Easy to pick up for developers familiar with C, Java, or similar languages.
- **First-Class Functions:** Assign functions to variables, pass them as arguments, and return them.
- **Built-in Data Structures:** Arrays and maps (dictionaries) out of the box.
- **Control Flow:** `if`/`else`, `while`, and C-style `for` loops.
- **Loop Control:** Support for `break` and `continue`.
- **Modularity:** Import DariX files as modules seamlessly.
- **Exception Handling:** Full-fledged try-catch-finally with multiple exception types.
- **Rich Standard Library:** Handy built-in functions for everyday tasks.
- **Enhanced REPL:** Multi-line input with smart grouping detection.

---

## Getting Started

### Prerequisites

- Go 1.16 or higher installed ([download](https://golang.org/dl/))

### Installation

#### Method 1: Automatic Installation (Linux)

```bash
chmod +x install.sh
./install.sh
````

Follow the prompts to choose system-wide or local installation.

#### Method 2: Automatic Installation (Termux on Android)

```bash
chmod +x install_termux.sh
./install_termux.sh
```

This script is tailored for the Termux environment.

#### Method 3: Manual Installation

```bash
git clone https://github.com/shayanheidari01/darix.git
cd darix
go build -o darix main.go
```

(Optional) To install globally:

```bash
sudo cp darix /usr/local/bin/
```

#### Method 4: Local Installation Script

```bash
chmod +x install_local.sh
./install_local.sh
```

This installs DariX to `~/.local/bin`.

---

### Running DariX Programs

* Execute a `.drx` file:

  ```bash
  darix path/to/your_script.drx
  ```

* Launch the interactive REPL:

  ```bash
  darix
  ```

---

## Language Overview

### Hello, World!

```drx
print("Hello, World!");
```

---

### Variables

Declare variables with `var`. Assign values using `=`.

```drx
var x = 5;
var name = "DariX";
var isActive = true;

x = 10;          // Reassign
name = "New Name";
```

---

### Data Types

* **Integer:** `42`, `-10`
* **Float:** `3.14`, `-0.001`
* **Boolean:** `true`, `false`
* **String:** `"Hello"`, `"DariX"`
* **Array:** `[1, 2, 3]`, `["a", "b", "c"]`
* **Map:** `{"key1": "value1", "key2": 100}`
* **Null:** Evaluates as `false` in boolean contexts.

---

### Operators

* **Arithmetic:** `+`, `-`, `*`, `/`, `%`
* **Comparison:** `<`, `>`, `<=`, `>=`, `==`, `!=`
* **Logical:** `!` (NOT), `&&` (AND), `||` (OR)
* **String Concatenation:** `+`
* **Unary:** `-` (negation), `!` (logical NOT)

---

### Control Flow

#### If / Else

```drx
if (x > 5) {
    print("x is greater than 5");
} else if (x == 5) {
    print("x is equal to 5");
} else {
    print("x is less than 5");
}
```

#### While Loop

```drx
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

#### For Loop (C-style)

```drx
for (var j = 0; j < 3; j = j + 1) {
    print("Iteration:", j);
}
```

Infinite loop example with `break`:

```drx
var k = 0;
for (;;) {
    if (k >= 2) break;
    print("Infinite?", k);
    k = k + 1;
}
```

#### Break and Continue

```drx
for (var i = 0; i < 10; i = i + 1) {
    if (i == 3) continue; // Skip 3
    if (i == 7) break;    // Stop before 7
    print(i);
}
// Output: 0, 1, 2, 4, 5, 6
```

---

### Functions

Declare functions with `func` keyword.

```drx
func greet(name) {
    print("Hello,", name);
}

greet("DariX");

func add(a, b) {
    return a + b;
}

var sum = add(2, 3);
print("Sum is:", sum);

var multiply = func(x, y) {
    return x * y;
};

print("Product:", multiply(4, 5));
```

---

### Data Structures

#### Arrays

```drx
var numbers = [1, 2, 3, 4];
print("First number:", numbers[0]);
print("Length:", len(numbers));

numbers = append(numbers, 5);
print("After append:", numbers);

var reversed = reverse(numbers);
print("Reversed:", reversed);

var rangeArray = range(5);       // [0,1,2,3,4]
var rangeArray2 = range(2, 8);   // [2,3,4,5,6,7]
var rangeArray3 = range(0, 10, 2);// [0,2,4,6,8]
```

#### Maps (Dictionaries)

```drx
var person = {"name": "Alice", "age": 30};
print("Name:", person["name"]);

person["city"] = "Wonderland";
print("Person:", person);

print("Map size:", len(person));
```

---

### Modules (Import)

Split your code into multiple files and import them:

**math.drx**

```drx
func square(x) {
    return x * x;
}

PI = 3.14159;
```

**main.drx**

```drx
import "math.drx";

print("PI is:", PI);
print("Square of 4 is:", square(4));
```

---

### Exception Handling

Robust try-catch-finally system with multiple exception types.

```drx
try {
    var result = 10 / 0; // Throws ZeroDivisionError
} catch (ZeroDivisionError e) {
    print("Caught division by zero:", e);
} finally {
    print("This always executes");
}
```

Multiple catch blocks:

```drx
try {
    processData();
} catch (ValueError e) {
    print("Value error:", e);
} catch (TypeError e) {
    print("Type error:", e);
} catch (e) {
    print("Other exception:", e);
}
```

Built-in exceptions include:

* `ValueError`
* `TypeError`
* `RuntimeError`
* `IndexError`
* `KeyError`
* `ZeroDivisionError`

Example of throwing exceptions:

```drx
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

---

### Built-in Functions

Some useful built-ins include:

| Function                                          | Description                                    |
| ------------------------------------------------- | ---------------------------------------------- |
| `print(...args)`                                  | Prints arguments separated by spaces           |
| `len(obj)`                                        | Returns length of string, array, or map        |
| `str(obj)`, `int(obj)`, `float(obj)`, `bool(obj)` | Type conversions                               |
| `type(obj)`                                       | Returns type as string (e.g., "INTEGER")       |
| `input([prompt])`                                 | Reads input from user                          |
| `range([start,] stop[, step])`                    | Creates array of integers, like Python's range |
| `append(array, ...values)`                        | Returns new array with values appended         |
| `reverse(obj)`                                    | Returns reversed string or array               |
| `sorted(array)`                                   | Returns sorted array                           |
| `upper(str)`, `lower(str)`, `trim(str)`           | String manipulation                            |
| `abs(x)`, `max(...)`, `min(...)`, `sum(array)`    | Math utilities                                 |
| `pow(base, exp)`, `clamp(val, min, max)`          | Power and clamping functions                   |
| `now()`, `timestamp()`                            | Current date/time                              |
| `exit([code])`                                    | Terminates program immediately                 |

---

## Contributing

Contributions, bug reports, and feature requests are welcome!
Feel free to fork the repo, open issues, and submit pull requests.

---

## License

This project is licensed under the **Apache License 2.0**. See the [LICENSE](LICENSE) file for details.

---

*Made with ❤️ using Go*

```
