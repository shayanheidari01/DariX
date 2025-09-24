// main.go

package main

import (
    "darix/ast"
    "darix/compiler"
    "darix/interpreter"
    "darix/lexer"
    "darix/object"
    "darix/parser"
    "darix/repl"
    "darix/internal/native"
    "darix/internal/version"
    "darix/vm"
    "fmt"
    "io"
    "os"
    "strconv"
    "strings"
)

var vmCPUbudget int

func main() {
    if len(os.Args) > 1 {
        cmd := os.Args[1]
        switch cmd {
        case "run":
            backend, file, policy, err := parseRunArgs(os.Args[2:])
            if err != nil || file == "" {
                fmt.Println("Usage: darix run [--backend=auto|vm|interp] [--allow=mod1,mod2|*] [--deny=mod1,mod2] [--fs-root=PATH] [--fs-ro] [--inject=true|false] [--cpu=N] <file.dax|->")
                os.Exit(1)
            }
            // Apply capability policy
            native.SetPolicy(policy)
            runFileWithOptions(file, backend)
            return
        case "repl":
            startEnhancedRepl()
            return
        case "eval":
            if len(os.Args) < 3 {
                fmt.Println("Usage: darix eval \"<code>\"")
                os.Exit(1)
            }
            runCode(os.Args[2])
            return
        case "disasm":
            if len(os.Args) < 3 {
                fmt.Println("Usage: darix disasm <file.dax>")
                os.Exit(1)
            }
            disasmFile(os.Args[2])
            return
        case "version", "-v", "--version":
            fmt.Println(version.String())
            return
        case "help", "-h", "--help":
            printHelp()
            return
        default:
            // If it's a file path, run it; otherwise show help
            if _, err := os.Stat(cmd); err == nil {
                runFile(cmd)
                return
            }
            fmt.Printf("Unknown command or file: %s\n\n", cmd)
            printHelp()
            os.Exit(1)
        }
    }
    startEnhancedRepl()
}

func runCode(code string) {
    program, errors := parseCode(code, "<eval>")
    if errors != nil {
        handleParseErrors(errors)
    }
    executeProgram(program, "<eval>", "auto")
}

func runFile(filename string) {
    runFileWithOptions(filename, "auto")
}

func runFileWithOptions(filename, backend string) {
    var content []byte
    var err error
    displayName := filename
    if filename == "-" {
        content, err = io.ReadAll(os.Stdin)
        displayName = "<stdin>"
    } else {
        content, err = os.ReadFile(filename)
    }
    if err != nil {
        handleFileError(err, "reading")
    }

    program, errors := parseCode(string(content), displayName)
    if errors != nil {
        handleParseErrors(errors)
    }

    executeProgram(program, displayName, backend)
}

// startEnhancedRepl starts the enhanced REPL with advanced features
func startEnhancedRepl() {
    r := repl.New(os.Stdin, os.Stdout)
    
    // Set backend and CPU budget from global variables if available
    if vmCPUbudget > 0 {
        r.SetVMCPUBudget(vmCPUbudget)
    }
    
    r.Start()
}


func printHelp() {
    fmt.Println("DariX command line")
    fmt.Println()
    fmt.Println("Usage:")
    fmt.Println("  darix run [--backend=auto|vm|interp] [--allow=mod1,mod2|*] [--deny=mod1,mod2] [--fs-root=PATH] [--fs-ro] [--inject=true|false] [--cpu=N] <file.dax|->  Run a script (use '-' for stdin)")
    fmt.Println("  darix disasm <file.dax>                            Disassemble bytecode")
    fmt.Println("  darix repl                                         Start interactive REPL")
    fmt.Println("  darix eval \"<code>\"                                 Evaluate a code snippet")
    fmt.Println("  darix version                                      Show version info")
    fmt.Println("  darix help                                         Show this help")
}

// parseRunArgs parses flags for the 'run' subcommand
func parseRunArgs(args []string) (backend, file string, policy *native.CapabilityPolicy, err error) {
    backend = "auto"
    // Start with default (backward-compatible): allow all native, inject to global
    p := native.DefaultAllowAll()
    var seenAllow, seenDeny bool
    for _, a := range args {
        if strings.HasPrefix(a, "--backend=") {
            v := strings.TrimPrefix(a, "--backend=")
            switch v {
            case "auto", "vm", "interp":
                backend = v
            default:
                return "", "", nil, fmt.Errorf("invalid backend: %s", v)
            }
            continue
        }
        if strings.HasPrefix(a, "--allow=") {
            list := strings.TrimPrefix(a, "--allow=")
            p.AllowAllNative = false
            if p.AllowGoModules == nil { p.AllowGoModules = map[string]bool{} }
            if list == "*" {
                // revert to allow-all explicitly
                p.AllowAllNative = true
            } else if list != "" {
                for _, m := range strings.Split(list, ",") {
                    m = strings.TrimSpace(m)
                    if m != "" { p.AllowGoModules[m] = true }
                }
            }
            seenAllow = true
            continue
        }
        if strings.HasPrefix(a, "--deny=") {
            list := strings.TrimPrefix(a, "--deny=")
            // Deny-list works when AllowAllNative is true
            p.AllowAllNative = true
            if p.AllowGoModules == nil { p.AllowGoModules = map[string]bool{} }
            if list != "" {
                for _, m := range strings.Split(list, ",") {
                    m = strings.TrimSpace(m)
                    if m != "" { p.AllowGoModules[m] = false }
                }
            }
            seenDeny = true
            continue
        }
        if strings.HasPrefix(a, "--fs-root=") {
            p.FSRoot = strings.TrimPrefix(a, "--fs-root=")
            continue
        }
        if a == "--fs-ro" {
            p.FSReadOnly = true
            continue
        }
        if strings.HasPrefix(a, "--inject=") {
            v := strings.TrimPrefix(a, "--inject=")
            if v == "true" {
                p.InjectToGlobal = true
            } else if v == "false" {
                p.InjectToGlobal = false
            } else {
                return "", "", nil, fmt.Errorf("invalid value for --inject: %s", v)
            }
            continue
        }
        if strings.HasPrefix(a, "--cpu=") {
            v := strings.TrimPrefix(a, "--cpu=")
            n, e := strconv.Atoi(v)
            if e != nil || n < 0 {
                return "", "", nil, fmt.Errorf("invalid value for --cpu: %s", v)
            }
            vmCPUbudget = n
            continue
        }
        if strings.HasPrefix(a, "--") {
            return "", "", nil, fmt.Errorf("unknown flag: %s", a)
        }
        if file == "" {
            file = a
        } else {
            return "", "", nil, fmt.Errorf("unexpected argument: %s", a)
        }
    }
    // If both allow and deny seen, prefer allow-list mode (explicit only)
    if seenAllow && !seenDeny {
        // already set to allow-only
    } else if !seenAllow && seenDeny {
        // already set to allow-all with deny-list
    }
    return backend, file, p, nil
}

// executeProgram runs an AST program using the requested backend selection
func executeProgram(program *ast.Program, displayName, backend string) {
    if backend == "interp" {
        inter := interpreter.New()
        result := inter.Interpret(program)
        handleRuntimeResult(result)
        return
    }
	if backend == "vm" {
		comp := compiler.New()
		if err := comp.Compile(program); err != nil {
			fmt.Println("VM backend error: unsupported feature for VM")
			os.Exit(1)
		}
		bc := comp.Bytecode()
		machine := vm.New(bc)
		if vmCPUbudget > 0 {
			machine.SetInstructionBudget(vmCPUbudget)
		}
		result := machine.Run()
		handleRuntimeResult(result)
		return
	}
	// auto
	comp := compiler.New()
	if err := comp.Compile(program); err == nil {
		bc := comp.Bytecode()
		machine := vm.New(bc)
		if vmCPUbudget > 0 {
			machine.SetInstructionBudget(vmCPUbudget)
		}
		result := machine.Run()
		if !handledAsErrorOrException(result) {
			return
		}
		os.Exit(1)
	}
	inter := interpreter.New()
	result := inter.Interpret(program)
	handleRuntimeResult(result)
}

// handleRuntimeResult prints error/exception consistently
func handleRuntimeResult(result object.Object) {
	if result == nil {
		return
	}
	switch result.Type() {
	case object.ERROR_OBJ:
		fmt.Println(result.Inspect())
		os.Exit(1)
	case object.ObjectType(object.EXCEPTION_SIGNAL):
		fmt.Println("Unhandled exception:\n" + result.Inspect())
		os.Exit(1)
	default:
		// Do nothing for other results in file/eval modes
	}
}

// handledAsErrorOrException returns true if result is error/exception and got printed
func handledAsErrorOrException(result object.Object) bool {
	if result == nil {
		return false
	}
	switch result.Type() {
	case object.ERROR_OBJ:
		fmt.Println(result.Inspect())
		return true
	case object.ObjectType(object.EXCEPTION_SIGNAL):
		fmt.Println("Unhandled exception:\n" + result.Inspect())
		return true
	default:
		return false
	}
}

// disasmFile compiles a source file and prints its bytecode disassembly
// parseCode creates a lexer and parser, then parses the code
func parseCode(code, filename string) (*ast.Program, []string) {
	l := lexer.NewWithFile(code, filename)
	p := parser.New(l)
	program := p.ParseProgram()
	
	if len(p.Errors()) != 0 {
		return nil, p.Errors()
	}
	
	return program, nil
}

// handleParseErrors prints parse errors and exits
func handleParseErrors(errors []string) {
	for _, msg := range errors {
		fmt.Printf("Parse error: %s\n", msg)
	}
	os.Exit(1)
}

// handleFileError prints file error and exits
func handleFileError(err error, operation string) {
	fmt.Printf("Error %s file: %s\n", operation, err)
	os.Exit(1)
}

func disasmFile(filename string) {
	content, err := os.ReadFile(filename)
	if err != nil {
		handleFileError(err, "reading")
	}
	program, errors := parseCode(string(content), filename)
	if errors != nil {
		handleParseErrors(errors)
	}
	comp := compiler.New()
	if err := comp.Compile(program); err != nil {
		fmt.Println("Compilation failed for disassembly: unsupported feature for VM")
		os.Exit(1)
	}
	bc := comp.Bytecode()
	fmt.Println("# Bytecode Instructions:")
	fmt.Println(bc.Instructions.String())
}
