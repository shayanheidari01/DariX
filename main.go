// main.go

package main

import (
	"darix/ast"
	"darix/compiler"
	"darix/internal/native"
	"darix/internal/version"
	"darix/interpreter"
	"darix/lexer"
	"darix/object"
	"darix/parser"
	"darix/repl"
	"darix/vm"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
)

var vmCPUbudget int

func main() {
	if len(os.Args) <= 1 {
		startEnhancedRepl()
		return
	}
	handleCLI(os.Args[1], os.Args[2:])
}

func handleCLI(command string, args []string) {
	switch command {
	case "run":
		backend, file, policy, err := parseRunArgs(args)
		if err != nil || file == "" {
			fmt.Println("Usage: darix run [--backend=auto|vm|interp] [--allow=mod1,mod2|*] [--deny=mod1,mod2] [--fs-root=PATH] [--fs-ro] [--inject=true|false] [--cpu=N] <file.dax|->")
			os.Exit(1)
		}
		native.SetPolicy(policy)
		runFileWithOptions(file, backend)
	case "repl":
		startEnhancedRepl()
	case "eval":
		if len(args) < 1 {
			fmt.Println("Usage: darix eval \"<code>\"")
			os.Exit(1)
		}
		runCode(args[0])
	case "disasm":
		if len(args) < 1 {
			fmt.Println("Usage: darix disasm <file.dax>")
			os.Exit(1)
		}
		disasmFile(args[0])
	case "version", "-v", "--version":
		fmt.Println(version.String())
	case "help", "-h", "--help":
		printHelp()
	default:
		if _, err := os.Stat(command); err == nil {
			runFile(command)
			return
		}
		fmt.Printf("Unknown command or file: %s\n\n", command)
		printHelp()
		os.Exit(1)
	}
}

func runCode(code string) {
	program, errs := parseCode(code, "<eval>")
	if errs != nil {
		handleParseErrors(errs)
	}
	executeProgram(program, "<eval>", "auto")
}

func runFile(filename string) {
	runFileWithOptions(filename, "auto")
}

func runFileWithOptions(filename, backend string) {
	content, displayName, err := readSource(filename)
	if err != nil {
		handleFileError(err, "reading")
	}

	program, errs := parseCode(string(content), displayName)
	if errs != nil {
		handleParseErrors(errs)
	}

	executeProgram(program, displayName, backend)
}

func startEnhancedRepl() {
	replInstance := repl.New(os.Stdin, os.Stdout)
	if vmCPUbudget > 0 {
		replInstance.SetVMCPUBudget(vmCPUbudget)
	}
	replInstance.Start()
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

func parseRunArgs(args []string) (backend, file string, policy *native.CapabilityPolicy, err error) {
	backend = "auto"
	policy = native.DefaultAllowAll()
	var seenAllow, seenDeny bool

	for _, arg := range args {
		switch {
		case strings.HasPrefix(arg, "--backend="):
			value := strings.TrimPrefix(arg, "--backend=")
			switch value {
			case "auto", "vm", "interp":
				backend = value
			default:
				return "", "", nil, fmt.Errorf("invalid backend: %s", value)
			}
		case strings.HasPrefix(arg, "--allow="):
			list := strings.TrimPrefix(arg, "--allow=")
			policy.AllowAllNative = false
			ensureModuleMap(policy)
			if list == "*" {
				policy.AllowAllNative = true
			} else {
				for _, mod := range splitCSV(list) {
					policy.AllowGoModules[mod] = true
				}
			}
			seenAllow = true
		case strings.HasPrefix(arg, "--deny="):
			list := strings.TrimPrefix(arg, "--deny=")
			policy.AllowAllNative = true
			ensureModuleMap(policy)
			for _, mod := range splitCSV(list) {
				policy.AllowGoModules[mod] = false
			}
			seenDeny = true
		case strings.HasPrefix(arg, "--fs-root="):
			policy.FSRoot = strings.TrimPrefix(arg, "--fs-root=")
		case arg == "--fs-ro":
			policy.FSReadOnly = true
		case strings.HasPrefix(arg, "--inject="):
			value := strings.TrimPrefix(arg, "--inject=")
			switch value {
			case "true":
				policy.InjectToGlobal = true
			case "false":
				policy.InjectToGlobal = false
			default:
				return "", "", nil, fmt.Errorf("invalid value for --inject: %s", value)
			}
		case strings.HasPrefix(arg, "--cpu="):
			value := strings.TrimPrefix(arg, "--cpu=")
			n, convErr := strconv.Atoi(value)
			if convErr != nil || n < 0 {
				return "", "", nil, fmt.Errorf("invalid value for --cpu: %s", value)
			}
			vmCPUbudget = n
		case strings.HasPrefix(arg, "--"):
			return "", "", nil, fmt.Errorf("unknown flag: %s", arg)
		default:
			if file == "" {
				file = arg
			} else {
				return "", "", nil, fmt.Errorf("unexpected argument: %s", arg)
			}
		}
	}

	if seenAllow && seenDeny {
		policy.AllowAllNative = false
	}

	return backend, file, policy, nil
}

func executeProgram(program *ast.Program, displayName, backend string) {
	_ = displayName
	switch backend {
	case "interp":
		handleRuntimeResult(runInterpreter(program))
	case "vm":
		result, err := runVM(program)
		if err != nil {
			fmt.Printf("VM backend error: %v\n", err)
			os.Exit(1)
		}
		handleRuntimeResult(result)
	default:
		runAuto(program)
	}
}

func runAuto(program *ast.Program) {
	if result, err := runVM(program); err == nil {
		handleRuntimeResult(result)
		return
	}
	handleRuntimeResult(runInterpreter(program))
}

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
	}
}

func parseCode(code, filename string) (*ast.Program, []string) {
	lex := lexer.NewWithFile(code, filename)
	parserInstance := parser.New(lex)
	program := parserInstance.ParseProgram()

	if errs := parserInstance.Errors(); len(errs) != 0 {
		return nil, errs
	}

	return program, nil
}

func handleParseErrors(errors []string) {
	fmt.Fprintf(os.Stderr, " Parse Errors Detected:\n")
	fmt.Fprintf(os.Stderr, "========================\n")
	for i, err := range errors {
		fmt.Fprintf(os.Stderr, "%d. %s\n", i+1, err)
	}
	fmt.Fprintf(os.Stderr, "\n Suggestion: Check your syntax and ensure all brackets, braces, and parentheses are properly closed.\n")
	os.Exit(1)
}

// Enhanced error display for runtime errors
func displayRuntimeError(err object.Object, filename string) {
	if errorObj, ok := err.(*object.Error); ok {
		fmt.Fprintf(os.Stderr, " Runtime Error:\n")
		fmt.Fprintf(os.Stderr, "=================\n")
		
		// Set filename if not already set
		if errorObj.Position.Filename == "" && filename != "" {
			errorObj.Position.Filename = filename
		}
		
		fmt.Fprintf(os.Stderr, "%s\n", errorObj.Inspect())
		
		// Add helpful tips based on error type
		switch errorObj.ErrorType {
		case "NameError":
			fmt.Fprintf(os.Stderr, "\n Tip: Make sure the variable or function is declared before use.\n")
		case "TypeError":
			fmt.Fprintf(os.Stderr, "\n Tip: Check that you're using the correct data types for this operation.\n")
		case "RuntimeError":
			fmt.Fprintf(os.Stderr, "\n Tip: Check your logic and ensure all operations are valid.\n")
		}
	} else {
		fmt.Fprintf(os.Stderr, " Runtime Error: %s\n", err.Inspect())
	}
}

func handleFileError(err error, operation string) {
	fmt.Printf("Error %s file: %s\n", operation, err)
	os.Exit(1)
}

func disasmFile(filename string) {
	content, err := os.ReadFile(filename)
	if err != nil {
		handleFileError(err, "reading")
	}
	program, errs := parseCode(string(content), filename)
	if errs != nil {
		handleParseErrors(errs)
	}
	compilerInstance := compiler.New()
	if err := compilerInstance.Compile(program); err != nil {
		fmt.Println("Compilation failed for disassembly: unsupported feature for VM")
		os.Exit(1)
	}
	bytecode := compilerInstance.Bytecode()
	fmt.Println("# Bytecode Instructions:")
	fmt.Println(bytecode.Instructions.String())
}

func runInterpreter(program *ast.Program) object.Object {
	return interpreter.New().Interpret(program)
}

func runVM(program *ast.Program) (object.Object, error) {
	compilerInstance := compiler.New()
	if err := compilerInstance.Compile(program); err != nil {
		return nil, err
	}
	machine := vm.New(compilerInstance.Bytecode())
	applyVMCPUBudget(machine)
	return machine.Run(), nil
}

func applyVMCPUBudget(machine *vm.VM) {
	if vmCPUbudget > 0 {
		machine.SetInstructionBudget(vmCPUbudget)
	}
}

func readSource(filename string) ([]byte, string, error) {
	if filename == "-" {
		data, err := io.ReadAll(os.Stdin)
		return data, "<stdin>", err
	}
	data, err := os.ReadFile(filename)
	return data, filename, err
}

func ensureModuleMap(policy *native.CapabilityPolicy) {
	if policy.AllowGoModules == nil {
		policy.AllowGoModules = make(map[string]bool)
	}
}

func splitCSV(input string) []string {
	if input == "" {
		return nil
	}
	parts := strings.Split(input, ",")
	result := make([]string, 0, len(parts))
	for _, part := range parts {
		if trimmed := strings.TrimSpace(part); trimmed != "" {
			result = append(result, trimmed)
		}
	}
	return result
}
