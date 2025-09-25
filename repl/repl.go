// repl/repl.go - Enhanced REPL implementation for DariX

package repl

import (
	"darix/ast"
	"darix/internal/version"
	"darix/interpreter"
	"darix/lexer"
	"darix/object"
	"darix/parser"
	"fmt"
	"io"
	"sort"
	"strconv"
	"strings"
	"time"
)

// REPL represents an enhanced Read-Eval-Print Loop
type REPL struct {
	inputHandler *InputHandler
	output       io.Writer
	interpreter  *interpreter.Interpreter
	variables    map[string]object.Object
	functions    map[string]*object.Function
	backend      string // "auto", "vm", "interp"
	vmCPUBudget  int
}

// New creates a new enhanced REPL instance
func New(input io.Reader, output io.Writer) *REPL {
	return &REPL{
		inputHandler: NewInputHandler(input, output),
		output:       output,
		interpreter:  interpreter.New(),
		variables:    make(map[string]object.Object),
		functions:    make(map[string]*object.Function),
		backend:      "auto",
		vmCPUBudget:  0,
	}
}

// SetBackend sets the execution backend for the REPL
func (r *REPL) SetBackend(backend string) {
	if backend == "auto" || backend == "vm" || backend == "interp" {
		r.backend = backend
	}
}

// SetVMCPUBudget sets the CPU instruction budget for VM execution
func (r *REPL) SetVMCPUBudget(budget int) {
	r.vmCPUBudget = budget
}

// Start begins the REPL session
func (r *REPL) Start() {
	r.printWelcome()
	r.printHelp()

	var buffer strings.Builder
	var parenCount, braceCount, bracketCount int

	for {
		prompt := r.getPrompt(buffer.Len() > 0)
		
		line, err := r.inputHandler.ReadLineWithFeatures(prompt, r)
		if err != nil {
			break
		}
		trimmedLine := strings.TrimSpace(line)

		// Handle special commands
		if buffer.Len() == 0 && strings.HasPrefix(trimmedLine, ":") {
			if r.handleCommand(trimmedLine) {
				break // Exit command
			}
			continue
		}

		// Handle exit
		if trimmedLine == "exit" && buffer.Len() == 0 && parenCount == 0 && braceCount == 0 && bracketCount == 0 {
			break
		}

		// Count groupings for multiline support
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

		buffer.WriteString(line)
		buffer.WriteString("\n")

		// Continue if we have unclosed groupings
		if parenCount > 0 || braceCount > 0 || bracketCount > 0 {
			continue
		}

		input := strings.TrimSpace(buffer.String())
		if input != "" {
			r.inputHandler.AddToHistory(input)
			r.evaluateInput(input)
		}

		// Reset for next input
		buffer.Reset()
		parenCount = 0
		braceCount = 0
		bracketCount = 0
	}

	fmt.Fprintln(r.output, "Goodbye!")
}

// printWelcome prints the welcome message
func (r *REPL) printWelcome() {
	fmt.Fprintf(r.output, "DariX Language REPL - %s\n", version.String())
	fmt.Fprintln(r.output, "Type ':help' for commands, 'exit' to quit")
	fmt.Fprintf(r.output, "Backend: %s\n", r.backend)
	fmt.Fprintln(r.output, strings.Repeat("-", 50))
}

// printHelp prints help information
func (r *REPL) printHelp() {
	fmt.Fprintln(r.output, "\nREPL Commands:")
	fmt.Fprintln(r.output, "  :help, :h          - Show this help")
	fmt.Fprintln(r.output, "  :clear, :c         - Clear screen")
	fmt.Fprintln(r.output, "  :vars, :v          - Show defined variables")
	fmt.Fprintln(r.output, "  :funcs, :f         - Show defined functions")
	fmt.Fprintln(r.output, "  :history, :hist    - Show command history")
	fmt.Fprintln(r.output, "  :backend <name>    - Set backend (auto/vm/interp)")
	fmt.Fprintln(r.output, "  :cpu <budget>      - Set VM CPU instruction budget")
	fmt.Fprintln(r.output, "  :reset             - Reset REPL state")
	fmt.Fprintln(r.output, "  :time <code>       - Time code execution")
	fmt.Fprintln(r.output, "  exit               - Exit REPL")
	fmt.Fprintln(r.output, "")
}

// getPrompt returns the appropriate prompt
func (r *REPL) getPrompt(isMultiline bool) string {
	if isMultiline {
		return "... "
	}
	return ">>> "
}

// handleCommand processes special REPL commands
func (r *REPL) handleCommand(cmd string) bool {
	parts := strings.Fields(cmd)
	if len(parts) == 0 {
		return false
	}

	command := parts[0]
	args := parts[1:]

	switch command {
	case ":help", ":h":
		r.printHelp()

	case ":clear", ":c":
		fmt.Fprint(r.output, "\033[2J\033[H") // ANSI clear screen
		r.printWelcome()

	case ":vars", ":v":
		r.showVariables()

	case ":funcs", ":f":
		r.showFunctions()

	case ":history", ":hist":
		r.inputHandler.PrintHistory(r.output, 20)

	case ":backend":
		if len(args) > 0 {
			r.SetBackend(args[0])
			fmt.Fprintf(r.output, "Backend set to: %s\n", r.backend)
		} else {
			fmt.Fprintf(r.output, "Current backend: %s\n", r.backend)
		}

	case ":cpu":
		if len(args) > 0 {
			if budget, err := strconv.Atoi(args[0]); err == nil && budget >= 0 {
				r.SetVMCPUBudget(budget)
				fmt.Fprintf(r.output, "VM CPU budget set to: %d\n", budget)
			} else {
				fmt.Fprintln(r.output, "Invalid CPU budget. Use a non-negative integer.")
			}
		} else {
			fmt.Fprintf(r.output, "Current VM CPU budget: %d\n", r.vmCPUBudget)
		}

	case ":reset":
		r.interpreter = interpreter.New()
		r.variables = make(map[string]object.Object)
		r.functions = make(map[string]*object.Function)
		fmt.Fprintln(r.output, "REPL state reset")

	case ":time":
		if len(args) > 0 {
			code := strings.Join(args, " ")
			r.timeExecution(code)
		} else {
			fmt.Fprintln(r.output, "Usage: :time <code>")
		}

	case ":exit", ":quit", ":q":
		return true

	default:
		fmt.Fprintf(r.output, "Unknown command: %s\n", command)
		fmt.Fprintln(r.output, "Type ':help' for available commands")
	}

	return false
}


// showVariables displays defined variables
func (r *REPL) showVariables() {
	env := r.interpreter.GetEnvironment()
	vars := env.GetAll()
	
	if len(vars) == 0 {
		fmt.Fprintln(r.output, "📊 No variables defined")
		return
	}

	fmt.Fprintln(r.output, "📊 Defined Variables:")
	
	// Sort variable names for consistent output
	names := r.getSortedNames(vars)

	for _, name := range names {
		obj := vars[name]
		typeStr := string(obj.Type())
		value := r.truncateValue(obj.Inspect())
		
		fmt.Fprintf(r.output, "  %s: %s = %s\n", name, typeStr, value)
	}
}

// showFunctions displays defined functions
func (r *REPL) showFunctions() {
	env := r.interpreter.GetEnvironment()
	vars := env.GetAll()
	
	functions := r.extractFunctions(vars)

	if len(functions) == 0 {
		fmt.Fprintln(r.output, "🔧 No functions defined")
		return
	}

	fmt.Fprintln(r.output, "🔧 Defined Functions:")
	
	// Sort function names
	names := r.getSortedFunctionNames(functions)

	for _, name := range names {
		fn := functions[name]
		params := r.getFunctionParams(fn)
		fmt.Fprintf(r.output, "  %s(%s)\n", name, strings.Join(params, ", "))
	}
}

// timeExecution times the execution of code
func (r *REPL) timeExecution(code string) {
	start := time.Now()
	result := r.executeCode(code)
	duration := time.Since(start)

	if result != nil {
		switch result.Type() {
		case object.ERROR_OBJ:
			fmt.Fprintf(r.output, "Error: %s\n", result.Inspect())
		case object.NULL_OBJ:
			// ignore
		default:
			fmt.Fprintln(r.output, result.Inspect())
		}
	}

	fmt.Fprintf(r.output, "Execution time: %v\n", duration)
}

// evaluateInput evaluates user input
func (r *REPL) evaluateInput(input string) {
	result := r.executeCode(input)
	
	if result != nil {
		switch result.Type() {
		case object.ERROR_OBJ:
			fmt.Fprintf(r.output, "Error: %s\n", result.Inspect())
		case object.NULL_OBJ:
			// Don't print null results
		default:
			fmt.Fprintln(r.output, result.Inspect())
		}
	}
}

// executeCode executes DariX code with the selected backend
func (r *REPL) executeCode(code string) object.Object {
	program, errors := r.parseCodeForREPL(code)
	if errors != nil {
		for _, msg := range errors {
			if !strings.Contains(msg, "warning: missing closing") {
				fmt.Fprintf(r.output, "Parse error: %s\n", msg)
			}
		}
		return nil
	}

	return r.executeProgram(program)
}

// parseCodeForREPL creates a lexer and parser for REPL mode
func (r *REPL) parseCodeForREPL(code string) (*ast.Program, []string) {
	l := lexer.NewWithFile(code, "<repl>")
	p := parser.New(l)
	p.SetReplMode(true)
	program := p.ParseProgram()
	
	if len(p.Errors()) != 0 {
		return nil, p.Errors()
	}
	
	return program, nil
}

// executeProgram executes an AST program using the selected backend
// In REPL mode, we always use the interpreter to maintain state between evaluations
func (r *REPL) executeProgram(program *ast.Program) object.Object {
	// For REPL, always use interpreter to maintain variable state
	// The VM doesn't share state with the interpreter, so variables would be lost
	return r.interpreter.Interpret(program)
}

// Helper functions to reduce code duplication

// getSortedNames returns sorted names from a map of objects
func (r *REPL) getSortedNames(vars map[string]object.Object) []string {
	names := make([]string, 0, len(vars))
	for name := range vars {
		names = append(names, name)
	}
	sort.Strings(names)
	return names
}

// truncateValue truncates long values for display
func (r *REPL) truncateValue(value string) string {
	if len(value) > 50 {
		return value[:47] + "..."
	}
	return value
}

// extractFunctions extracts function objects from variables
func (r *REPL) extractFunctions(vars map[string]object.Object) map[string]*object.Function {
	functions := make(map[string]*object.Function)
	for name, obj := range vars {
		if fn, ok := obj.(*object.Function); ok {
			functions[name] = fn
		}
	}
	return functions
}

// getSortedFunctionNames returns sorted function names
func (r *REPL) getSortedFunctionNames(functions map[string]*object.Function) []string {
	names := make([]string, 0, len(functions))
	for name := range functions {
		names = append(names, name)
	}
	sort.Strings(names)
	return names
}

// getFunctionParams extracts parameter names from a function
func (r *REPL) getFunctionParams(fn *object.Function) []string {
	params := make([]string, len(fn.Parameters))
	for i, param := range fn.Parameters {
		params[i] = param.Value
	}
	return params
}
