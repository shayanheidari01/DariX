// repl/completion.go - Tab completion functionality for DariX REPL

package repl

import (
	"darix/object"
	"sort"
	"strings"
)

// DariX language keywords
var keywords = []string{
	"var", "func", "if", "else", "while", "for", "break", "continue",
	"return", "true", "false", "null", "class", "try", "catch", "finally",
	"throw", "import", "export", "let", "const",
}

// Built-in functions
var builtinFunctions = []string{
	"print", "len", "str", "int", "float", "abs", "sum", "reverse",
	"sort", "keys", "values", "items", "range", "time", "sleep",
	"type", "input",
}

// Special REPL commands
var replCommands = []string{
	":help", ":h", ":clear", ":c", ":vars", ":v", ":funcs", ":f",
	":history", ":hist", ":backend", ":cpu", ":reset", ":time",
	":exit", ":quit", ":q",
}

// CompletionResult represents a completion suggestion
type CompletionResult struct {
	Suggestions []string
	Prefix      string
	StartPos    int
}

// GetCompletions returns completion suggestions for the given input
func (r *REPL) GetCompletions(input string, cursorPos int) *CompletionResult {
	if cursorPos > len(input) {
		cursorPos = len(input)
	}

	// Find the word being completed
	wordStart := cursorPos
	for wordStart > 0 && isIdentifierChar(input[wordStart-1]) {
		wordStart--
	}

	prefix := input[wordStart:cursorPos]
	
	// Handle REPL commands (starting with :)
	if strings.HasPrefix(prefix, ":") {
		return &CompletionResult{
			Suggestions: filterCompletions(replCommands, prefix),
			Prefix:      prefix,
			StartPos:    wordStart,
		}
	}

	var suggestions []string

	// Add keywords
	suggestions = append(suggestions, filterCompletions(keywords, prefix)...)

	// Add built-in functions
	suggestions = append(suggestions, filterCompletions(builtinFunctions, prefix)...)

	// Add user-defined variables and functions
	if r.interpreter != nil {
		env := r.interpreter.GetEnvironment()
		vars := env.GetAll()
		
		userNames := make([]string, 0, len(vars))
		for name := range vars {
			userNames = append(userNames, name)
		}
		suggestions = append(suggestions, filterCompletions(userNames, prefix)...)
	}

	// Remove duplicates and sort
	suggestions = removeDuplicates(suggestions)
	sort.Strings(suggestions)

	return &CompletionResult{
		Suggestions: suggestions,
		Prefix:      prefix,
		StartPos:    wordStart,
	}
}

// filterCompletions filters completions that start with the given prefix
func filterCompletions(completions []string, prefix string) []string {
	if prefix == "" {
		return completions
	}

	var filtered []string
	lowerPrefix := strings.ToLower(prefix)
	
	for _, completion := range completions {
		if strings.HasPrefix(strings.ToLower(completion), lowerPrefix) {
			filtered = append(filtered, completion)
		}
	}
	
	return filtered
}

// removeDuplicates removes duplicate strings from a slice
func removeDuplicates(items []string) []string {
	seen := make(map[string]bool)
	var result []string
	
	for _, item := range items {
		if !seen[item] {
			seen[item] = true
			result = append(result, item)
		}
	}
	
	return result
}

// isIdentifierChar returns true if the character can be part of an identifier
func isIdentifierChar(ch byte) bool {
	return (ch >= 'a' && ch <= 'z') ||
		   (ch >= 'A' && ch <= 'Z') ||
		   (ch >= '0' && ch <= '9') ||
		   ch == '_' || ch == ':'
}

// GetVariableType returns the type of a variable for enhanced completion info
func (r *REPL) GetVariableType(name string) string {
	if r.interpreter == nil {
		return ""
	}

	env := r.interpreter.GetEnvironment()
	if obj, exists := env.Get(name); exists {
		switch obj.Type() {
		case object.INTEGER_OBJ:
			return "int"
		case object.FLOAT_OBJ:
			return "float"
		case object.STRING_OBJ:
			return "string"
		case object.BOOLEAN_OBJ:
			return "bool"
		case object.ARRAY_OBJ:
			return "array"
		case object.MAP_OBJ:
			return "map"
		case object.FUNCTION_OBJ:
			return "function"
		case object.CLASS_OBJ:
			return "class"
		case object.INSTANCE_OBJ:
			return "instance"
		default:
			return string(obj.Type())
		}
	}
	return ""
}

// GetFunctionSignature returns the signature of a function for completion
func (r *REPL) GetFunctionSignature(name string) string {
	if r.interpreter == nil {
		return ""
	}

	env := r.interpreter.GetEnvironment()
	if obj, exists := env.Get(name); exists {
		if fn, ok := obj.(*object.Function); ok {
			params := make([]string, len(fn.Parameters))
			for i, param := range fn.Parameters {
				params[i] = param.Value
			}
			return name + "(" + strings.Join(params, ", ") + ")"
		}
	}

	// Check built-in functions
	signatures := map[string]string{
		"print":   "print(...values)",
		"len":     "len(obj)",
		"str":     "str(value)",
		"int":     "int(value)",
		"float":   "float(value)",
		"abs":     "abs(number)",
		"sum":     "sum(array)",
		"reverse": "reverse(array)",
		"sort":    "sort(array)",
		"keys":    "keys(map)",
		"values":  "values(map)",
		"items":   "items(map)",
		"range":   "range(start, end) or range(end)",
		"time":    "time()",
		"sleep":   "sleep(seconds)",
		"type":    "type(obj)",
		"input":   "input(prompt?)",
	}

	if sig, exists := signatures[name]; exists {
		return sig
	}

	return name + "()"
}
