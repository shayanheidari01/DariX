# DariX Tutorial

## Your First Program

Create a file `hello.dax`:
```dax
print("Hello, World!")
```

Run it:
```bash
darix run hello.dax
```

## Variables and Types

```dax
// DariX is dynamically typed
var name = "DariX"
var version = 1
var pi = 3.14
var active = true
var nothing = null

print(type(name))    // STRING
print(type(version)) // INTEGER
print(type(pi))      // FLOAT
```

## Arithmetic

```dax
var a = 10
var b = 3

print(a + b)    // 13
print(a - b)    // 7
print(a * b)    // 30
print(a / b)    // 3
print(a % b)    // 1
print(-a)       // -10
print(a > b)    // true
```

## Strings

```dax
var s = "Hello"
print(len(s))              // 5
print(s + " World")        // Hello World
print(s[0])                // H

import string
print(string.upper(s))     // HELLO
print(string.split("a,b,c", ","))  // [a, b, c]
print(string.replace(s, "Hello", "Hi"))  // Hi
```

## Control Flow

```dax
// If/elif/else
var score = 85
if (score >= 90) {
    print("A")
} elif (score >= 80) {
    print("B")
} elif (score >= 70) {
    print("C")
} else {
    print("F")
}

// While loop
var i = 0
while (i < 5) {
    print(i)
    i = i + 1
}

// For loop
for (var j = 0; j < 5; j = j + 1) {
    print(j)
}

// Break and continue
for (var k = 0; k < 100; k = k + 1) {
    if (k == 5) break
    if (k % 2 == 0) continue
    print(k)
}
```

## Functions

```dax
func greet(name) {
    return "Hello, " + name + "!"
}

print(greet("DariX"))

// Recursive function
func fib(n) {
    if (n <= 1) return n
    return fib(n - 1) + fib(n - 2)
}

print(fib(10))  // 55

// Lambda functions
var double = lambda x: x * 2
var add = lambda a, b: a + b

print(double(5))    // 10
print(add(3, 4))    // 7

// Closures
func make_adder(n) {
    return lambda x: x + n
}
var add5 = make_adder(5)
print(add5(10))  // 15
```

## Classes

```dax
class Point {
    var x = 0
    var y = 0

    func __init__(x, y) {
        self.x = x
        self.y = y
    }

    func distance_to(other) {
        var dx = self.x - other.x
        var dy = self.y - other.y
        return sqrt(dx * dx + dy * dy)
    }

    func to_string() {
        return "Point(" + str(self.x) + ", " + str(self.y) + ")"
    }
}

var p1 = Point(0, 0)
var p2 = Point(3, 4)
print(p1.distance_to(p2))  // 5
print(p1.to_string())       // Point(0, 0)
```

## Exception Handling

```dax
func divide(a, b) {
    if (b == 0) {
        throw ValueError("division by zero")
    }
    return a / b
}

try {
    print(divide(10, 0))
} catch (ValueError e) {
    print("Error:", e)
} finally {
    print("done")
}

// Custom exceptions
func validate_age(age) {
    assert age >= 0, "age must be non-negative"
    assert age <= 150, "age must be realistic"
}
```

## Data Structures

```dax
// Arrays
var arr = [1, 2, 3, 4, 5]
print(arr[0])       // 1
arr[2] = 99
print(arr)          // [1, 2, 99, 4, 5]

import array
print(array.filter(arr, lambda x: x > 3))  // [99, 4, 5]
print(array.map(arr, lambda x: x * 2))     // [2, 4, 198, 8, 10]

// Maps
var person = {"name": "Alice", "age": 30}
print(person["name"])  // Alice
person["city"] = "Tehran"

import map
print(map.keys(person))   // [name, age, city]
print(map.values(person)) // [Alice, 30, Tehran]
```

## Working with Files

```dax
import fs
import json

// Write data
var data = {"users": [{"name": "Alice"}, {"name": "Bob"}]}
fs.write("data.json", json.stringify(data, 2))

// Read data
var content = fs.read("data.json")
var parsed = json.parse(content)
print(parsed["users"][0]["name"])  // Alice

// Check files
if (fs.exists("data.json")) {
    print("File exists!")
    print("Size:", fs.size("data.json"))
}
```

## Importing Modules

```dax
// Math
import math
print(math.sqrt(2))        // 1.41421
print(math.pi())          // 3.14159
print(math.sin(math.pi() / 2))  // 1

// Datetime
import datetime
var now = datetime.now()
print(datetime.format(now, "%Y-%m-%d %H:%M:%S"))

// Random
import random
random.seed(42)
print(random.int_range(1, 7))  // dice roll

// Regex
import regex
print(regex.test("\\d+", "abc123"))  // true
print(regex.match("\\d+", "abc123")) // 123
```

## Error Handling Patterns

```dax
// Validate input
func process_input(input) {
    if (input == null) {
        throw ValueError("input cannot be null")
    }
    if (type(input) != "STRING") {
        throw TypeError("input must be a string")
    }
    return string.upper(input)
}

// Safe execution
var result = null
try {
    result = process_input(user_input)
} catch (ValueError e) {
    print("Bad input:", e)
} catch (TypeError e) {
    print("Wrong type:", e)
}
```
