# DariX Exception System Documentation

The DariX programming language now includes a comprehensive Python-like exception handling system that provides robust error handling capabilities.

## Overview

The exception system in DariX follows Python's model with:
- Try-catch-finally blocks
- Multiple exception types
- Exception propagation
- Built-in and custom exceptions
- Automatic exception throwing for common errors

## Basic Syntax

### Try-Catch Block
```darix
try {
    // Code that might throw an exception
    var result = riskyOperation()
} catch (ExceptionType e) {
    // Handle specific exception type
    print("Caught exception:", e)
}
```

### Try-Catch-Finally Block
```darix
try {
    // Risky code
    performOperation()
} catch (ValueError e) {
    // Handle ValueError
    print("Value error:", e)
} finally {
    // Always executes
    cleanup()
}
```

### Multiple Catch Clauses
```darix
try {
    // Code that might throw different exceptions
    processData()
} catch (ValueError e) {
    print("Value error:", e)
} catch (TypeError e) {
    print("Type error:", e)  
} catch (e) {
    print("Any other exception:", e)
}
```

## Built-in Exception Types

DariX provides several built-in exception types:

- **ValueError**: Invalid value provided
- **TypeError**: Incorrect type used
- **RuntimeError**: General runtime error
- **IndexError**: Array index out of bounds (thrown automatically)
- **KeyError**: Map key not found
- **ZeroDivisionError**: Division by zero (thrown automatically)

### Creating Exceptions
```darix
// Create specific exception types
var valueError = ValueError("Invalid input value")
var typeError = TypeError("Expected string, got integer")
var runtimeError = RuntimeError("Operation failed")
```

## Throwing Exceptions

### Manual Exception Throwing
```darix
func validateAge(age) {
    if (age < 0) {
        throw ValueError("Age cannot be negative")
    }
    if (age > 150) {
        throw ValueError("Age seems unrealistic")
    }
    return age
}
```

### Automatic Exception Throwing

DariX automatically throws exceptions for common error conditions:

#### ZeroDivisionError
```darix
try {
    var result = 10 / 0  // Automatically throws ZeroDivisionError
} catch (ZeroDivisionError e) {
    print("Division by zero detected")
}
```

#### IndexError
```darix
try {
    var arr = [1, 2, 3]
    var item = arr[10]  // Automatically throws IndexError
} catch (IndexError e) {
    print("Array index out of bounds")
}
```

## Exception Propagation

Exceptions propagate up the call stack until caught:

```darix
func level3() {
    throw RuntimeError("Error from level 3")
}

func level2() {
    level3()  // Exception propagates through here
}

func level1() {
    try {
        level2()
    } catch (RuntimeError e) {
        print("Caught at level 1:", e)
    }
}
```

## Exception Handling in Control Structures

### Loops
Exceptions can interrupt loops and be handled outside:

```darix
try {
    for (var i = 0; i < 10; i = i + 1) {
        if (i == 5) {
            throw RuntimeError("Loop interrupted")
        }
        print("Processing", i)
    }
} catch (RuntimeError e) {
    print("Loop was interrupted:", e)
}
```

### Functions
Functions can throw exceptions that propagate to callers:

```darix
func riskyFunction() {
    throw ValueError("Something went wrong")
}

try {
    riskyFunction()
} catch (ValueError e) {
    print("Function threw exception:", e)
}
```

## Finally Block Behavior

The finally block always executes, regardless of whether an exception occurred:

```darix
func processFile(filename) {
    var file = openFile(filename)
    try {
        // Process file
        return processData(file)
    } catch (e) {
        print("Error processing file:", e)
        return null
    } finally {
        // Always close the file
        closeFile(file)
    }
}
```

## Best Practices

### 1. Specific Exception Handling
Catch specific exception types rather than using generic catches:

```darix
// Good
try {
    processData()
} catch (ValueError e) {
    handleValueError(e)
} catch (TypeError e) {
    handleTypeError(e)
}

// Less preferred (but still valid)
try {
    processData()
} catch (e) {
    handleAnyError(e)
}
```

### 2. Resource Cleanup
Use finally blocks for resource cleanup:

```darix
var resource = acquireResource()
try {
    useResource(resource)
} finally {
    releaseResource(resource)
}
```

### 3. Error Validation
Validate inputs and throw appropriate exceptions:

```darix
func divide(a, b) {
    if (type(a) != "INTEGER" || type(b) != "INTEGER") {
        throw TypeError("Both arguments must be integers")
    }
    if (b == 0) {
        throw ZeroDivisionError("Cannot divide by zero")
    }
    return a / b
}
```

### 4. Exception Messages
Provide clear, descriptive error messages:

```darix
// Good
throw ValueError("Email address must contain @ symbol")

// Less helpful
throw ValueError("Invalid email")
```

## Exception vs Error Objects

DariX distinguishes between:
- **Exceptions**: Structured exception objects with types and messages
- **Errors**: Simple error objects for internal interpreter errors

Exceptions provide better structure and can be caught and handled, while errors typically indicate more serious interpreter-level problems.

## Example: Complete Exception Handling

```darix
func safeCalculator(operation, a, b) {
    try {
        if (operation == "divide") {
            if (b == 0) {
                throw ZeroDivisionError("Cannot divide by zero")
            }
            return a / b
        } else if (operation == "sqrt") {
            if (a < 0) {
                throw ValueError("Cannot take square root of negative number")
            }
            return sqrt(a)
        } else {
            throw ValueError("Unknown operation: " + operation)
        }
    } catch (ZeroDivisionError e) {
        print("Division error:", e)
        return null
    } catch (ValueError e) {
        print("Value error:", e)
        return null
    } catch (e) {
        print("Unexpected error:", e)
        return null
    } finally {
        print("Calculator operation completed")
    }
}

// Usage
var result1 = safeCalculator("divide", 10, 2)  // Returns 5
var result2 = safeCalculator("divide", 10, 0)  // Returns null, handles error
var result3 = safeCalculator("sqrt", -4)       // Returns null, handles error
```

This exception system makes DariX programs more robust and provides clear error handling mechanisms similar to those found in Python and other modern programming languages.