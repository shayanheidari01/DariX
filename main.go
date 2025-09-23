// main.go

package main

import (
	"bufio"
	"darix/ast"
	"darix/compiler"
	"darix/interpreter"
	"darix/lexer"
	"darix/object"
	"darix/parser"
	"darix/internal/version"
	"darix/vm"
	"fmt"
	"io"
	"os"
	"strings"
)

func main() {
	if len(os.Args) > 1 {
		cmd := os.Args[1]
		switch cmd {
		case "run":
			backend, file, err := parseRunArgs(os.Args[2:])
			if err != nil || file == "" {
				fmt.Println("Usage: darix run [--backend=auto|vm|interp] <file.dax|->")
				os.Exit(1)
			}
			runFileWithOptions(file, backend)
			return
		case "repl":
			startRepl()
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
	startRepl()
}

func runCode(code string) {
	l := lexer.NewWithFile(code, "<eval>")
	p := parser.New(l)
	program := p.ParseProgram()

	if len(p.Errors()) != 0 {
		for _, msg := range p.Errors() {
			fmt.Printf("Parse error: %s\n", msg)
		}
		os.Exit(1)
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
		fmt.Printf("Error reading file: %s\n", err)
		os.Exit(1)
	}

	l := lexer.NewWithFile(string(content), displayName)
	p := parser.New(l)
	program := p.ParseProgram()
	if len(p.Errors()) != 0 {
		for _, msg := range p.Errors() {
			fmt.Printf("Parse error: %s\n", msg)
		}
		os.Exit(1)
	}

	executeProgram(program, displayName, backend)
}

func startRepl() {
	scanner := bufio.NewScanner(os.Stdin)
	inter := interpreter.New()
	fmt.Println("DariX Language REPL")
	fmt.Println("Type 'exit' to quit")
	var buffer strings.Builder

	var parenCount, braceCount, bracketCount int

	for {
		fmt.Print(">>> ")
		scanner.Scan()
		line := scanner.Text()
		trimmedLine := strings.TrimSpace(line)

		if trimmedLine == "exit" && buffer.Len() == 0 && parenCount == 0 && braceCount == 0 && bracketCount == 0 {
			break
		}

		// شمارش گروه‌بندی‌های باز و بسته در خط فعلی
		for _, ch := range line {
			switch ch {
			case '(':
				parenCount++
			case ')':
				parenCount--
			case '{':
				braceCount++
			case '}':
				braceCount--
			case '[':
				bracketCount++
			case ']':
				bracketCount--
			}
		}

		// جمع‌آوری خط در buffer
		buffer.WriteString(line)
		buffer.WriteString("\n") // اضافه کردن خط جدید برای حفظ ساختار

		// اگر هنوز گروه‌بندی‌های بازی وجود دارد، منتظر بقیه ورودی‌ها بمان
		if parenCount > 0 || braceCount > 0 || bracketCount > 0 {
			continue
		}

		// اکنون فرض می‌کنیم که یک دستور کامل جمع‌آوری شده است
		input := buffer.String()
		// ریست کردن buffer و شمارنده‌ها برای دستور بعدی
		buffer.Reset()
		parenCount = 0
		braceCount = 0
		bracketCount = 0

		// لکس و پارس کردن ورودی
		l := lexer.NewWithFile(input, "<repl>")
		p := parser.New(l)
		p.SetReplMode(true) // فعال کردن حالت REPL
		program := p.ParseProgram()

		// چک کردن خطاها
		if len(p.Errors()) != 0 {
			for _, msg := range p.Errors() {
				// حذف هشدارهای مربوط به پرانتز/براکت ناکامل
				if !strings.Contains(msg, "warning: missing closing") {
					fmt.Printf("Parse error: %s\n", msg)
				}
			}
			continue
		}

		// تفسیر و اجرای برنامه
		result := inter.Interpret(program)
		if result != nil {
			switch result.Type() {
			case object.ERROR_OBJ:
				// Already formatted by runtime; print as-is
				fmt.Println(result.Inspect())
			case object.NULL_OBJ:
				// خالی
			default:
				fmt.Println(result.Inspect()) // ← این‌جا رشتهٔ برگردانده‌شده را چاپ می‌کند
			}
		}

		// اطمینان از پاک شدن بافر خروجی
		os.Stdout.Sync()
	}
}

func printHelp() {
	fmt.Println("DariX command line")
	fmt.Println()
	fmt.Println("Usage:")
	fmt.Println("  darix run [--backend=auto|vm|interp] <file.dax|->  Run a script (use '-' for stdin)")
	fmt.Println("  darix disasm <file.dax>                            Disassemble bytecode")
	fmt.Println("  darix repl                                         Start interactive REPL")
	fmt.Println("  darix eval \"<code>\"                                 Evaluate a code snippet")
	fmt.Println("  darix version                                      Show version info")
	fmt.Println("  darix help                                         Show this help")
}

// parseRunArgs parses flags for the 'run' subcommand
func parseRunArgs(args []string) (backend, file string, err error) {
	backend = "auto"
	for _, a := range args {
		if strings.HasPrefix(a, "--backend=") {
			v := strings.TrimPrefix(a, "--backend=")
			switch v {
			case "auto", "vm", "interp":
				backend = v
			default:
				return "", "", fmt.Errorf("invalid backend: %s", v)
			}
			continue
		}
		if strings.HasPrefix(a, "--") {
			return "", "", fmt.Errorf("unknown flag: %s", a)
		}
		if file == "" {
			file = a
		} else {
			return "", "", fmt.Errorf("unexpected argument: %s", a)
		}
	}
	return backend, file, nil
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
		result := machine.Run()
		handleRuntimeResult(result)
		return
	}
	// auto
	comp := compiler.New()
	if err := comp.Compile(program); err == nil {
		bc := comp.Bytecode()
		machine := vm.New(bc)
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
func disasmFile(filename string) {
	content, err := os.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file: %s\n", err)
		os.Exit(1)
	}
	l := lexer.NewWithFile(string(content), filename)
	p := parser.New(l)
	program := p.ParseProgram()
	if len(p.Errors()) != 0 {
		for _, msg := range p.Errors() {
			fmt.Printf("Parse error: %s\n", msg)
		}
		os.Exit(1)
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
