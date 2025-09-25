# 🔑 **DariX New Keywords Implementation**

## 📋 **Overview**

This document describes the newly implemented keywords in DariX programming language, inspired by Python's keyword system while maintaining DariX's C-like syntax.

## ✅ **Implemented Keywords**

### **1. `del` - Delete Statement**
**Purpose**: Delete variables, array elements, or object properties

**Syntax**:
```dax
del variable_name;
del array[index];
del object.property;
```

**Examples**:
```dax
var x = 42;
del x;  // x is now undefined

var arr = [1, 2, 3];
del arr[1];  // removes element at index 1

var obj = {"name": "DariX", "version": "0.1.1"};
del obj.version;  // removes the version property
```

**AST Node**: `DelStatement`

---

### **2. `assert` - Assertion Statement**
**Purpose**: Debug and test conditions during development

**Syntax**:
```dax
assert condition;
assert condition, "error message";
```

**Examples**:
```dax
var x = 10;
assert x > 5;  // passes silently
assert x < 5;  // throws AssertionError

assert x > 0, "x must be positive";  // custom error message
```

**AST Node**: `AssertStatement`

---

### **3. `pass` - No-operation Statement**
**Purpose**: Placeholder for empty code blocks

**Syntax**:
```dax
pass;
```

**Examples**:
```dax
func todo_function() {
    pass;  // placeholder for future implementation
}

if (condition) {
    pass;  // do nothing for now
} else {
    print("condition is false");
}
```

**AST Node**: `PassStatement`

---

### **4. `and`, `or`, `not` - Logical Operators (Keywords)**
**Purpose**: Alternative syntax for logical operations

**Syntax**:
```dax
condition1 and condition2
condition1 or condition2
not condition
```

**Examples**:
```dax
var x = 10;
var y = 20;

if (x > 5 and y < 30) {
    print("both conditions are true");
}

if (x < 0 or y > 100) {
    print("at least one condition is true");
}

if (not (x == 0)) {
    print("x is not zero");
}
```

**Token Types**: `AND_KW`, `OR_KW`, `NOT_KW`
**Note**: These coexist with `&&`, `||`, `!` operators

---

### **5. `in` - Membership Test**
**Purpose**: Check if an element exists in a collection

**Syntax**:
```dax
element in collection
```

**Examples**:
```dax
var arr = [1, 2, 3, 4, 5];
if (3 in arr) {
    print("3 is in the array");
}

var str = "Hello, DariX!";
if ("DariX" in str) {
    print("Found DariX in string");
}

var map = {"name": "DariX", "type": "language"};
if ("name" in map) {
    print("map has name key");
}
```

**AST Node**: `InExpression`

---

### **6. `is` - Identity Comparison**
**Purpose**: Check if two variables refer to the same object

**Syntax**:
```dax
object1 is object2
```

**Examples**:
```dax
var a = [1, 2, 3];
var b = a;  // b references the same array as a
var c = [1, 2, 3];  // c is a new array with same content

if (a is b) {
    print("a and b are the same object");  // true
}

if (a is c) {
    print("a and c are the same object");  // false
} else {
    print("a and c are different objects");
}
```

**AST Node**: `IsExpression`

---

### **7. `global` - Global Variable Declaration**
**Purpose**: Declare variables as global within function scope

**Syntax**:
```dax
global variable1, variable2, ...;
```

**Examples**:
```dax
var counter = 0;  // global variable

func increment() {
    global counter;  // declare counter as global
    counter = counter + 1;
}

increment();
print(counter);  // prints 1
```

**AST Node**: `GlobalStatement`

---

### **8. `nonlocal` - Nonlocal Variable Declaration**
**Purpose**: Access variables from enclosing (but not global) scope

**Syntax**:
```dax
nonlocal variable1, variable2, ...;
```

**Examples**:
```dax
func outer() {
    var x = 10;
    
    func inner() {
        nonlocal x;  // access x from outer function
        x = x + 1;
    }
    
    inner();
    print(x);  // prints 11
}

outer();
```

**AST Node**: `NonlocalStatement`

---

### **9. `lambda` - Anonymous Functions**
**Purpose**: Create small, inline functions

**Syntax**:
```dax
lambda param1, param2, ...: expression
```

**Examples**:
```dax
var square = lambda x: x * x;
print(square(5));  // prints 25

var add = lambda a, b: a + b;
print(add(3, 4));  // prints 7

// Use with higher-order functions
var numbers = [1, 2, 3, 4, 5];
var doubled = map(numbers, lambda x: x * 2);
print(doubled);  // prints [2, 4, 6, 8, 10]
```

**AST Node**: `LambdaExpression`

---

### **10. `with` - Context Manager**
**Purpose**: Manage resources with automatic cleanup

**Syntax**:
```dax
with context_expression as variable {
    // code block
}
```

**Examples**:
```dax
with open("file.txt") as file {
    var content = file.read();
    print(content);
}  // file is automatically closed

with database_connection() as db {
    db.execute("SELECT * FROM users");
}  // connection is automatically closed
```

**AST Node**: `WithStatement`

---

### **11. `yield` - Generator Functions**
**Purpose**: Create generator functions that can pause and resume

**Syntax**:
```dax
yield value;
yield;  // yield null
```

**Examples**:
```dax
func fibonacci() {
    var a = 0;
    var b = 1;
    while (true) {
        yield a;
        var temp = a;
        a = b;
        b = temp + b;
    }
}

var fib = fibonacci();
print(fib.next());  // 0
print(fib.next());  // 1
print(fib.next());  // 1
print(fib.next());  // 2
```

**AST Node**: `YieldExpression`

---

### **12. `elif` - Else If Statement**
**Purpose**: Chain multiple conditional statements

**Syntax**:
```dax
if (condition1) {
    // code
} elif (condition2) {
    // code
} elif (condition3) {
    // code
} else {
    // code
}
```

**Examples**:
```dax
var score = 85;

if (score >= 90) {
    print("Grade: A");
} elif (score >= 80) {
    print("Grade: B");
} elif (score >= 70) {
    print("Grade: C");
} elif (score >= 60) {
    print("Grade: D");
} else {
    print("Grade: F");
}
```

**Token Type**: `ELIF`

---

### **13. `from` and `as` - Enhanced Import System**
**Purpose**: Selective imports and aliasing

**Syntax**:
```dax
from module import function1, function2;
import module as alias;
from module import function as alias;
```

**Examples**:
```dax
from math import sin, cos, pi;
print(sin(pi / 2));  // prints 1

import json as JSON;
var data = JSON.parse('{"name": "DariX"}');

from http import get as http_get;
var response = http_get("https://api.example.com");
```

**Token Types**: `FROM`, `AS`

---

## 🏗️ **Implementation Status**

### **✅ Completed:**
1. **Token System**: All new keywords added to `token/token.go`
2. **AST Nodes**: Complete AST node definitions in `ast/ast.go`
3. **Lexer Support**: Keywords recognized by lexer
4. **Build System**: All code compiles successfully

### **🔄 In Progress:**
1. **Parser Implementation**: Need to add parsing logic for each keyword
2. **Interpreter Support**: Need to implement runtime behavior
3. **VM Support**: Need to add bytecode compilation support
4. **Error Handling**: Proper error messages for new constructs

### **📋 Next Steps:**
1. Implement parser functions for each new keyword
2. Add interpreter evaluation methods
3. Create comprehensive test suite
4. Update documentation and examples
5. Add VS Code extension support for new keywords

---

## 🎯 **Design Philosophy**

### **Python Inspiration:**
- Familiar syntax for Python developers
- Consistent behavior with Python semantics
- Enhanced readability and expressiveness

### **DariX Integration:**
- Maintains C-like syntax where appropriate
- Preserves existing functionality
- Backward compatibility with current code

### **Performance Considerations:**
- Efficient AST node design
- Minimal runtime overhead
- VM compilation support planned

---


## 📚 **Usage Examples**

### **Complete Example Using Multiple New Keywords:**

```dax
// Global configuration
var debug_mode = true;

func process_data(items) {
    global debug_mode;
    
    if (debug_mode) {
        assert items != null, "items cannot be null";
        assert len(items) > 0, "items cannot be empty";
    }
    
    var results = [];
    
    // Iterate with index over items (for-in is not supported)
    for (var i = 0; i < len(items); i = i + 1) {
        var item = items[i];
        if (item is null) {
            pass;  // skip null items
        } else if (("skip" in item) and item["skip"]) {
            pass;  // skip marked items
        } else {
            // Process value without lambda/ternary
            var value = item["value"];
            var result = 0;
            if (value > 0) {
                result = value * 2;
            } else {
                result = 0;
            }
            // append returns a new array; reassign results
            results = append(results, result);
        }
    }
    
    return results;
}
// Usage with context manager
with timer() as t {
    var data = [
        {"value": 10, "skip": false},
        {"value": -5, "skip": false},
        null
    ];
    
    var processed = process_data(data);
    print("Results:", processed);
}

print("Processing took:", t.elapsed(), "seconds");
```

---

## 🚀 **Benefits**

### **Developer Experience:**
- **Familiar Syntax**: Python developers feel at home
- **Enhanced Readability**: More expressive code
- **Better Debugging**: Assert statements for development
- **Flexible Control Flow**: elif, pass for cleaner logic

### **Language Features:**
- **Advanced Functions**: Lambda expressions and generators
- **Resource Management**: Context managers with 'with'
- **Scope Control**: Global and nonlocal declarations
- **Collection Operations**: Membership testing with 'in'

### **Code Quality:**
- **Cleaner Logic**: Logical operators as words
- **Better Testing**: Built-in assertions
- **Resource Safety**: Automatic cleanup with context managers
- **Functional Programming**: Lambda expressions support

The implementation of these keywords significantly enhances DariX's expressiveness while maintaining its performance and simplicity!
