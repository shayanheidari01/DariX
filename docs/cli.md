# DariX CLI Reference

## Usage

```
darix <command> [arguments]
```

## Commands

### `run` — Execute a DariX script

```bash
darix run script.dax
darix run examples/hello_world.dax
```

Reads and executes the specified `.dax` file.

### `eval` — Evaluate an expression

```bash
darix eval "print(1 + 2)"
darix eval "var x = 42; print(x)"
darix eval "import math; print(math.sqrt(16))"
```

Evaluates a single expression or statement from the command line.

### `repl` — Interactive REPL

```bash
darix repl
```

Starts an interactive Read-Eval-Print Loop with:
- Tab completion for keywords, builtins, and user-defined names
- Command history (up/down arrows)
- REPL commands (`:help`, `:clear`, `:vars`, `:funcs`, `:history`, `:backend`, `:cpu`, `:reset`, `:time`, `:exit`)
- Backend selection (auto/vm/interp)
- Multiline input with bracket counting

### `disasm` — Disassemble bytecode

```bash
darix disasm script.dax
```

Compiles the script and prints the bytecode instructions. Useful for debugging the compiler.

### `version` — Show version

```bash
darix version
darix -v
darix --version
```

### `help` — Show help

```bash
darix help
darix -h
darix --help
```

## REPL Commands

| Command | Description |
|---------|-------------|
| `:help` | Show REPL help |
| `:clear` | Clear screen |
| `:vars` | List all variables |
| `:funcs` | List all functions |
| `:history` | Show command history |
| `:backend` | Show/change backend (auto/vm/interp) |
| `:cpu` | Show/set instruction budget |
| `:reset` | Reset environment |
| `:time` | Toggle execution timing |
| `:exit` | Exit REPL |

## Exit Codes

| Code | Description |
|------|-------------|
| 0 | Success |
| 1 | Error (parse error, runtime error, file not found) |
