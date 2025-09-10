# DariX Programming Language

DariX is a dynamically typed, interpreted programming language implemented in Go. It features a C-like syntax and supports fundamental programming constructs such as variables, functions, control flow (if/else, while, for), data structures (arrays, maps), and a rich set of built-in functions. DariX now includes a comprehensive exception handling system similar to Python's model.

## Features

*   **Dynamic Typing:** Variables do not require explicit type declarations.
*   **Interpreted:** Code is executed directly by the interpreter.
*   **C-like Syntax:** Familiar syntax for developers coming from C, Java, or similar languages.
*   **First-Class Functions:** Functions can be assigned to variables, passed as arguments, and returned from other functions.
*   **Data Structures:** Built-in support for arrays and maps (dictionaries).
*   **Control Flow:** `if`/`else`, `while`, and C-style `for` loops.
*   **Loop Control:** `break` and `continue` statements.
*   **Modularity:** Import other DariX files as modules.
*   **Exception Handling:** Try-catch-finally blocks with multiple exception types.
*   **Built-in Functions:** A standard library of useful functions for common tasks.
*   **Enhanced REPL:** Multi-line input support with automatic grouping detection.

## Getting Started

### Prerequisites

*   Go (version 1.16 or higher recommended)

### Installation

#### Method 1: Automatic Installation (Linux)

For Linux systems, you can use the provided installation script:

```bash
# Make the script executable and run it
chmod +x install.sh
./install.sh
```

The script will guide you through the installation process and offer options for system-wide or local installation.

#### Method 2: Automatic Installation (Termux on Android)

For Termux users on Android, there's a specific installation script:

```bash
# Make the script executable and run it
chmod +x install_termux.sh
./install_termux.sh
```

This script is optimized for the Termux environment and will install DariX to the appropriate location.

#### Method 3: Manual Installation

1.  Clone the repository:
    ```bash
    git clone https://github.com/your-username/darix.git
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

#### Method 4: Local Installation Script

If you have the source code locally, you can use the provided local installation script:

```bash
chmod +x install_local.sh
./install_local.sh
```

This script will automatically install DariX to `~/.local/bin` directory.

### Running DariX Code

You can run DariX code in two ways:

1.  **Execute a `.drx` file:**
    ```bash
    darix path/to/your/script.drx
    ```
2.  **Start the REPL (Read-Eval-Print Loop):**
    ```bash
    darix
    ```

## Language Guide

### Hello, World!

```drx
print("Hello, World!");
```

### Variables

Variables are declared using the `var` keyword. Assignment to existing variables uses the `=` operator.

```drx
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

```drx
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

```drx
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

#### For Loop (C-style)

```drx
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

```drx
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

```drx
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

```drx
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

```drx
var person = {"name": "Alice", "age": 30};
print("Name:", person["name"]); // Access by key

person["city"] = "Wonderland"; // Add/update key-value pair
print("Person map:", person);

print("Map size:", len(person)); // Built-in len function
```

### Modules (Import)

You can split your code into multiple files and import them using the `import` statement. The imported file is executed in the *same* environment, making its variables and functions available.

**math.drx**
```drx
func square(x) {
    return x * x;
}

PI = 3.14159; // Exported variable
```

**main.drx**
```drx
import "math.drx";

print("PI is:", PI);
print("Square of 4 is:", square(4));
```

### Exception Handling

DariX includes a comprehensive exception handling system with try-catch-finally blocks:

```drx
try {
    var result = 10 / 0; // Automatically throws ZeroDivisionError
} catch (ZeroDivisionError e) {
    print("Caught division by zero:", e);
} finally {
    print("This always executes");
}
```

Multiple catch clauses are supported:

```drx
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