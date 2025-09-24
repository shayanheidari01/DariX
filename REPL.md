# DariX Enhanced REPL Documentation

## Overview

The DariX Enhanced REPL (Read-Eval-Print Loop) provides a powerful interactive environment for developing and testing DariX code. It includes advanced features like command history, tab completion, special commands, and multiple execution backends.

## Features

### Core Features
- **Interactive Code Execution**: Execute DariX code line by line
- **Multiline Input Support**: Automatic detection of incomplete expressions
- **Multiple Backends**: Choose between VM, interpreter, or auto mode
- **Command History**: Navigate through previous commands
- **Tab Completion**: Auto-complete keywords, variables, and functions
- **Special Commands**: Built-in commands for REPL management
- **Error Recovery**: Graceful error handling and recovery

### Special Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `:help` | `:h` | Show help information |
| `:clear` | `:c` | Clear the screen |
| `:vars` | `:v` | Show defined variables |
| `:funcs` | `:f` | Show defined functions |
| `:history` | `:hist` | Show command history |
| `:backend <name>` | - | Set execution backend (auto/vm/interp) |
| `:cpu <budget>` | - | Set VM CPU instruction budget |
| `:reset` | - | Reset REPL state |
| `:time <code>` | - | Time code execution |
| `:exit` | `:quit`, `:q` | Exit REPL |

## Getting Started

### Starting the REPL

```bash
# Start the enhanced REPL
darix repl

# Or simply run darix without arguments
darix
```

### Basic Usage

```dax
>>> var x = 42
>>> var y = "Hello, DariX!"
>>> print(x, y)
42 Hello, DariX!

>>> func greet(name) {
...   return "Hello, " + name + "!";
... }
>>> greet("World")
Hello, World!
```

### Using Special Commands

```dax
>>> :vars
Defined Variables:
  x: INTEGER = 42
  y: STRING = Hello, DariX!

>>> :funcs
Defined Functions:
  greet(name)

>>> :backend vm
Backend set to: vm

>>> :time print("Performance test")
Performance test
Execution time: 245.7µs
```

## Advanced Features

### 1. Command History

The REPL automatically saves your command history:

- Navigate through history (implementation ready for arrow key support)
- View history with `:history`
- Automatic deduplication of consecutive identical commands
- History persists for the session (up to 1000 entries)

### 2. Tab Completion

Auto-completion is available for:

- **Keywords**: `var`, `func`, `if`, `else`, `while`, `for`, `class`, etc.
- **Built-in Functions**: `print`, `len`, `str`, `int`, `float`, `abs`, etc.
- **User Variables**: All defined variables in current scope
- **User Functions**: All defined functions with parameter hints
- **REPL Commands**: All special commands starting with `:`

### 3. Multiline Input

The REPL automatically detects incomplete expressions:

```dax
>>> if (x > 0) {
...   print("Positive");
... } else {
...   print("Non-positive");
... }
Positive
```

Supports automatic bracket/brace/parenthesis counting for:
- `()` - Parentheses
- `{}` - Braces  
- `[]` - Brackets

### 4. Backend Selection

Choose your execution backend:

```dax
>>> :backend vm      # Use VM for better performance
>>> :backend interp  # Use interpreter for full compatibility
>>> :backend auto    # Auto-select (VM with interpreter fallback)
```

### 5. Performance Monitoring

Time your code execution:

```dax
>>> :time var arr = [1, 2, 3, 4, 5]; sum(arr)
15
⏱️  Execution time: 123.4µs
```

### 6. State Management

Reset or inspect REPL state:

```dax
>>> :vars           # Show all variables
>>> :funcs          # Show all functions
>>> :reset          # Clear all state
REPL state reset
```

## Architecture

### Components

1. **REPL Core** (`repl/repl.go`)
   - Main REPL loop and command handling
   - Backend management (VM/interpreter)
   - State management and introspection

2. **Input Handler** (`repl/input.go`)
   - Enhanced input processing
   - Command history management
   - Basic line editing support

3. **Completion Engine** (`repl/completion.go`)
   - Tab completion logic
   - Keyword and function suggestions
   - Context-aware completions

### Backend Integration

The REPL supports three execution modes:

- **VM Mode**: Uses the DariX bytecode compiler and virtual machine for optimal performance
- **Interpreter Mode**: Uses the tree-walking interpreter for full language compatibility
- **Auto Mode**: Attempts VM compilation first, falls back to interpreter if needed

## User Experience

### Visual Enhancements

- **Clean Interface**: Clear visual indicators for different types of information
- **Color Coding**: Error messages, success indicators, and information types
- **Persian Support**: Bilingual messages (Persian/English)
- **Clean Layout**: Organized output with proper spacing and alignment

### Error Handling

- **Graceful Recovery**: Parse errors don't crash the REPL
- **Helpful Messages**: Clear error descriptions with context
- **State Preservation**: Variables and functions persist across errors

### Accessibility

- **Multiple Languages**: Persian and English interface
- **Clear Prompts**: Distinct prompts for single-line (`>>>`) and multi-line (`...`) input
- **Comprehensive Help**: Built-in help system with examples

## Configuration

### VM CPU Budget

Limit VM instruction execution for safety:

```dax
>>> :cpu 1000000    # Set instruction limit
VM CPU budget set to: 1000000

>>> :cpu            # Show current limit
Current VM CPU budget: 1000000
```

### Backend Preferences

Set your preferred execution backend:

```dax
>>> :backend vm     # Prefer VM for performance
>>> :backend interp # Prefer interpreter for compatibility
>>> :backend auto   # Use automatic selection (default)
```

## Examples

### Object-Oriented Programming

```dax
>>> class Point {
...   func __init__(self, x, y) {
...     self.x = x;
...     self.y = y;
...   }
...   
...   func distance(self, other) {
...     var dx = self.x - other.x;
...     var dy = self.y - other.y;
...     return abs(dx * dx + dy * dy);
...   }
... }

>>> var p1 = Point(0, 0)
>>> var p2 = Point(3, 4)
>>> p1.distance(p2)
25

>>> :vars
Defined Variables:
  Point: CLASS = <class Point>
  p1: INSTANCE = <Point instance>
  p2: INSTANCE = <Point instance>
```

### Functional Programming

```dax
>>> var numbers = [1, 2, 3, 4, 5]
>>> func square(x) { return x * x; }
>>> func map_array(arr, fn) {
...   var result = [];
...   for (var i = 0; i < len(arr); i++) {
...     result = result + [fn(arr[i])];
...   }
...   return result;
... }

>>> map_array(numbers, square)
[1, 4, 9, 16, 25]
```

### Performance Testing

```dax
>>> :time var sum = 0; for (var i = 0; i < 10000; i++) { sum = sum + i; }
Execution time: 2.345ms

>>> :backend vm
Backend set to: vm

>>> :time var sum = 0; for (var i = 0; i < 10000; i++) { sum = sum + i; }
Execution time: 456.7µs
```

## Future Enhancements

### Planned Features

1. **Syntax Highlighting**: Real-time syntax highlighting in input
2. **Arrow Key Navigation**: Full readline-like editing capabilities
3. **Auto-Indentation**: Smart indentation for code blocks
4. **Code Formatting**: Automatic code formatting on input
5. **Session Persistence**: Save and restore REPL sessions
6. **Debugging Integration**: Step-through debugging capabilities
7. **Package Management**: Import and manage external packages
8. **Export Functionality**: Export REPL session to script files

### Integration Opportunities

- **IDE Integration**: VS Code extension REPL integration
- **Web Interface**: Browser-based REPL for online development
- **Jupyter Support**: Jupyter kernel for notebook development
- **Language Server**: Integration with Language Server Protocol

## Contributing

The enhanced REPL is designed to be extensible. Key areas for contribution:

1. **Input Enhancement**: Better readline support with external libraries
2. **Completion Engine**: More sophisticated completion algorithms
3. **Visual Improvements**: Syntax highlighting and themes
4. **Platform Support**: Platform-specific optimizations
5. **Documentation**: Examples and tutorials

## References

- [DariX Language Documentation](README.md)
- [VS Code Extension](vscode-darix/)
- [Language Grammar](parser/)
- [Virtual Machine](vm/)
- [Interpreter](interpreter/)

---

**Happy Coding with DariX!**

*The enhanced REPL makes DariX development more productive and enjoyable.*
