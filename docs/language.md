# DariX Language Reference

## Overview

DariX is a dynamically-typed, interpreted programming language with Python-inspired syntax. It features garbage collection, closures, classes, exception handling, and a rich standard library.

## Data Types

### Integers
```dax
var x = 42
var neg = -100
var hex = 0xFF       // hex literals not supported, use int()
```

### Floats
```dax
var pi = 3.14
var sci = 1.5e10
```

### Strings
```dax
var s = "hello"
var multi = "line1\nline2"
var escaped = "quote: \"hello\""
```

### Booleans
```dax
var t = true
var f = false
```

### Null
```dax
var n = null
```

## Variables

```dax
var x = 10          // declaration
x = 20              // reassignment
var a, b, c = 1, 2, 3  // multiple declarations
```

Variables are dynamically typed. Type is determined at runtime.

## Operators

### Arithmetic
| Operator | Description |
|----------|-------------|
| `+` | Addition / string concatenation |
| `-` | Subtraction / unary negation |
| `*` | Multiplication |
| `/` | Division |
| `%` | Modulus |

### Comparison
| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `>` | Greater than |
| `<=` | Less or equal |
| `>=` | Greater or equal |

### Logical
| Operator | Keyword | Description |
|----------|---------|-------------|
| `&&` | `and` | Logical AND (short-circuit) |
| `\|\|` | `or` | Logical OR (short-circuit) |
| `!` | `not` | Logical NOT |

### Other
| Operator | Description |
|----------|-------------|
| `in` | Membership test |
| `is` | Identity comparison |
| `@` | Decorator prefix |

## Control Flow

### If / Elif / Else
```dax
if (x > 10) {
    print("big")
} elif (x > 5) {
    print("medium")
} else {
    print("small")
}
```

### While Loops
```dax
var i = 0
while (i < 10) {
    print(i)
    i = i + 1
}
```

### For Loops
```dax
for (var i = 0; i < 10; i = i + 1) {
    print(i)
}
```

### Break and Continue
```dax
while (true) {
    if (condition) break
    if (skip) continue
}
```

## Functions

```dax
func add(a, b) {
    return a + b
}

func factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}

// Void functions return null implicitly
func do_nothing() { var x = 1 }
```

### Lambdas
```dax
var double = lambda x: x * 2
var adder = lambda a, b: a + b

// Higher-order functions
func apply(fn, val) { return fn(val) }
apply(double, 5)  // 10
```

### Closures
```dax
func make_counter() {
    var count = 0
    return lambda x: (count = count + 1, count)
}
var counter = make_counter()
counter()  // 1
counter()  // 2
```

## Classes

```dax
class Animal {
    var name = ""
    var sound = ""

    func __init__(name, sound) {
        self.name = name
        self.sound = sound
    }

    func speak() {
        return self.name + " says " + self.sound
    }
}

class Dog extends Animal {
    func __init__(name) {
        self.__init__(name, "Woof")
    }
}

var dog = Dog("Rex")
print(dog.speak())  // Rex says Woof
```

## Decorators

```dax
func log(fn) {
    return lambda ...args: (
        print("Calling", fn),
        fn(...args)
    )
}

@log
func greet(name) {
    return "Hello, " + name
}
```

## Exception Handling

```dax
try {
    var x = 1 / 0
} catch (ZeroDivisionError e) {
    print("Error:", e)
} catch (TypeError e) {
    print("Type error:", e)
} finally {
    print("cleanup")
}

// Throw exceptions
throw ValueError("bad input")

// Assert
assert x > 0, "x must be positive"
```

## Import System

```dax
import math
print(math.sqrt(16))

import string
print(string.upper("hello"))
```

## Comments

```dax
// Single line comment

/* Multi-line
   comment */
```
