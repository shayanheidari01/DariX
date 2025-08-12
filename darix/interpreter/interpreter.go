// darix/interpreter.go

package interpreter

import (
	"darix/ast"
	"darix/lexer"
	"darix/object"
	"darix/parser"
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
	"time"
)

var (
	NULL  = &object.Null{}
	TRUE  = &object.Boolean{Value: true}
	FALSE = &object.Boolean{Value: false}
)

const (
	BREAK_SIGNAL    = "BREAK_SIGNAL"
	CONTINUE_SIGNAL = "CONTINUE_SIGNAL"
)

type Interpreter struct {
	env           *object.Environment
	builtins      map[string]*object.Builtin
	loadedModules map[string]object.Object
}

func New() *Interpreter {
	inter := &Interpreter{
		env:           object.NewEnvironment(),
		builtins:      make(map[string]*object.Builtin),
		loadedModules: make(map[string]object.Object), // مقداردهی اولیه
	}
	inter.initBuiltins()
	return inter
}

// optimPrint concatenates and writes directly
func optimPrint(args ...object.Object) object.Object {
	buf := &strings.Builder{}
	for i, arg := range args {
		if i > 0 {
			buf.WriteRune(' ')
		}
		buf.WriteString(arg.Inspect())
	}
	buf.WriteRune('\n')
	os.Stdout.Write([]byte(buf.String()))
	return NULL
}

func (i *Interpreter) initBuiltins() {
	i.builtins = map[string]*object.Builtin{
		"print": {
			Fn: func(args ...object.Object) object.Object {
				// Concatenate all args into a single string
				parts := make([]string, len(args))
				for j, arg := range args {
					parts[j] = arg.Inspect()
				}
				// Return a String object so REPL prints it
				return object.NewString(strings.Join(parts, " "))
			},
		},

		"len": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("len: expected 1 argument, got %d", len(args))
				}
				switch o := args[0].(type) {
				case *object.String:
					return object.NewInteger(int64(len(o.Value)))
				case *object.Array:
					return object.NewInteger(int64(len(o.Elements)))
				case *object.Map:
					return object.NewInteger(int64(len(o.Pairs)))
				default:
					return object.NewError("len: unsupported type %s", o.Type())
				}
			},
		},
		"str": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("str: expected 1 argument, got %d", len(args))
				}
				switch v := args[0].(type) {
				case *object.String:
					return v
				case *object.Integer:
					return &object.String{Value: fmt.Sprintf("%d", v.Value)}
				case *object.Boolean:
					return &object.String{Value: fmt.Sprintf("%t", v.Value)}
				default:
					return object.NewError("str: unsupported type %s", v.Type())
				}
			},
		},
		"int": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("int: expected 1 argument, got %d", len(args))
				}
				switch v := args[0].(type) {
				case *object.Integer:
					return v
				case *object.String:
					val, err := strconv.ParseInt(v.Value, 10, 64)
					if err != nil {
						return object.NewError("int: cannot convert %s", v.Value)
					}
					return object.NewInteger(val)
				default:
					return object.NewError("int: unsupported type %s", v.Type())
				}
			},
		},
		"bool": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("bool: expected 1 argument, got %d", len(args))
				}
				return nativeBoolToBooleanObject(isTruthy(args[0]))
			},
		},
		"type": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("type: expected 1 argument, got %d", len(args))
				}
				return &object.String{Value: string(args[0].Type())}
			},
		},
		"input": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) > 1 {
					return object.NewError("input: expected 0 or 1 arguments, got %d", len(args))
				}
				if len(args) == 1 {
					fmt.Print(args[0].Inspect())
				}
				var s string
				fmt.Scanln(&s)
				return &object.String{Value: s}
			},
		},
		"range": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) == 0 || len(args) > 3 {
					return object.NewError("range: expected 1-3 arguments, got %d", len(args))
				}

				var start, stop, step int64
				switch len(args) {
				case 1:
					start = 0
					stop = asInt(args[0])
					step = 1
				case 2:
					start = asInt(args[0])
					stop = asInt(args[1])
					step = 1
				case 3:
					start = asInt(args[0])
					stop = asInt(args[1])
					step = asInt(args[2])
				}

				var elems []object.Object
				if step == 0 {
					return object.NewError("range: step cannot be 0")
				}
				if step > 0 {
					for i := start; i < stop; i += step {
						elems = append(elems, object.NewInteger(i))
					}
				} else {
					for i := start; i > stop; i += step {
						elems = append(elems, object.NewInteger(i))
					}
				}
				return &object.Array{Elements: elems}
			},
		},
		"abs": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("abs: expected 1 argument, got %d", len(args))
				}
				if i, ok := args[0].(*object.Integer); ok {
					if i.Value < 0 {
						return object.NewInteger(-i.Value)
					}
					return i
				}
				return object.NewError("abs: unsupported type %s", args[0].Type())
			},
		},
		"max": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) < 1 {
					return object.NewError("max: expected at least 1 argument")
				}
				max := args[0]
				for _, a := range args[1:] {
					if left, ok := max.(*object.Integer); ok {
						if right, ok := a.(*object.Integer); ok {
							if right.Value > left.Value {
								max = right
							}
						} else {
							return object.NewError("max: all arguments must be integers")
						}
					}
				}
				return max
			},
		},
		"min": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) < 1 {
					return object.NewError("min: expected at least 1 argument")
				}
				min := args[0]
				for _, a := range args[1:] {
					if left, ok := min.(*object.Integer); ok {
						if right, ok := a.(*object.Integer); ok {
							if right.Value < left.Value {
								min = right
							}
						} else {
							return object.NewError("min: all arguments must be integers")
						}
					}
				}
				return min
			},
		},
		"sum": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("sum: expected 1 argument (array)")
				}
				arr, ok := args[0].(*object.Array)
				if !ok {
					return object.NewError("sum: argument must be an array")
				}
				var total int64
				for _, elem := range arr.Elements {
					if i, ok := elem.(*object.Integer); ok {
						total += i.Value
					} else {
						return object.NewError("sum: all elements must be integers")
					}
				}
				return object.NewInteger(total)
			},
		},

		"reverse": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("reverse: expected 1 argument (string or array)")
				}
				switch val := args[0].(type) {
				case *object.String:
					runes := []rune(val.Value)
					for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
						runes[i], runes[j] = runes[j], runes[i]
					}
					return &object.String{Value: string(runes)}
				case *object.Array:
					reversed := make([]object.Object, len(val.Elements))
					for i, j := 0, len(val.Elements)-1; i <= j; i, j = i+1, j-1 {
						reversed[i], reversed[j] = val.Elements[j], val.Elements[i]
					}
					return &object.Array{Elements: reversed}
				default:
					return object.NewError("reverse: unsupported type %s", val.Type())
				}
			},
		},

		"sorted": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("sorted: expected 1 argument (array)")
				}
				arr, ok := args[0].(*object.Array)
				if !ok {
					return object.NewError("sorted: argument must be an array")
				}
				elements := make([]object.Object, len(arr.Elements))
				copy(elements, arr.Elements)
				sort.Slice(elements, func(i, j int) bool {
					li, lok := elements[i].(*object.Integer)
					ri, rok := elements[j].(*object.Integer)
					return lok && rok && li.Value < ri.Value
				})
				return &object.Array{Elements: elements}
			},
		},
		"upper": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("upper: expected 1 argument, got %d", len(args))
				}

				s, ok := args[0].(*object.String)
				if !ok {
					return object.NewError("upper: argument must be a string, got %s", args[0].Type())
				}

				// استفاده از پول برای ایجاد شیء جدید
				result := object.NewString(strings.ToUpper(s.Value))
				return result
			},
		},
		"lower": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("lower: expected 1 argument, got %d", len(args))
				}

				s, ok := args[0].(*object.String)
				if !ok {
					return object.NewError("lower: argument must be a string, got %s", args[0].Type())
				}

				// استفاده از پول برای ایجاد شیء جدید
				result := object.NewString(strings.ToLower(s.Value))
				return result
			},
		},
		"trim": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("trim: expected 1 argument, got %d", len(args))
				}

				s, ok := args[0].(*object.String)
				if !ok {
					return object.NewError("trim: argument must be a string, got %s", args[0].Type())
				}

				// استفاده از پول برای ایجاد شیء جدید
				result := object.NewString(strings.TrimSpace(s.Value))
				return result
			},
		},
		"append": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) < 2 {
					return object.NewError("append: expected array and at least one value")
				}

				arr, ok := args[0].(*object.Array)
				if !ok {
					return object.NewError("append: first argument must be an array")
				}

				// ایجاد آرایه جدید با ظرفیت کافی
				newLength := len(arr.Elements) + len(args) - 1
				newElements := make([]object.Object, newLength)

				// کپی عناصر موجود
				copy(newElements, arr.Elements)

				// اضافه کردن عناصر جدید
				for i, arg := range args[1:] {
					newElements[len(arr.Elements)+i] = arg
				}

				return object.NewArray(newElements)
			},
		},

		"contains": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 2 {
					return object.NewError("contains: expected array and value")
				}
				arr, ok := args[0].(*object.Array)
				if !ok {
					return object.NewError("contains: first argument must be an array")
				}
				for _, elem := range arr.Elements {
					if object.Equals(elem, args[1]) {
						return TRUE
					}
				}
				return FALSE
			},
		},
		"pow": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 2 {
					return object.NewError("pow: expected 2 arguments")
				}
				base, ok1 := args[0].(*object.Integer)
				exp, ok2 := args[1].(*object.Integer)
				if !ok1 || !ok2 {
					return object.NewError("pow: both arguments must be integers")
				}
				result := int64(1)
				for i := int64(0); i < exp.Value; i++ {
					result *= base.Value
				}
				return object.NewInteger(result)
			},
		},

		"clamp": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 3 {
					return object.NewError("clamp: expected 3 arguments (val, min, max)")
				}
				val := asInt(args[0])
				min := asInt(args[1])
				max := asInt(args[2])
				if val < min {
					return object.NewInteger(min)
				}
				if val > max {
					return object.NewInteger(max)
				}
				return object.NewInteger(val)
			},
		},
		"now": {
			Fn: func(args ...object.Object) object.Object {
				return &object.String{Value: time.Now().Format(time.RFC3339)}
			},
		},

		"timestamp": {
			Fn: func(args ...object.Object) object.Object {
				return object.NewInteger(time.Now().Unix())
			},
		},
		"exit": {
			Fn: func(args ...object.Object) object.Object {
				code := int64(0)
				if len(args) == 1 {
					if i, ok := args[0].(*object.Integer); ok {
						code = i.Value
					}
				}
				os.Exit(int(code))
				return NULL
			},
		},
	}
}

func asInt(obj object.Object) int64 {
	switch o := obj.(type) {
	case *object.Integer:
		return o.Value
	case *object.String:
		i, _ := strconv.ParseInt(o.Value, 10, 64)
		return i
	default:
		return 0
	}
}

// Interpret runs the interpreter on the AST program.
// It evaluates statements and returns the last evaluated object.
// It also handles freeing the final result object.
func (i *Interpreter) Interpret(program *ast.Program) object.Object {
	var result object.Object
	for _, statement := range program.Statements {
		result = i.eval(statement, i.env)
		// Free previous result if it's not the final one and not special (like RETURN_VALUE or ERROR)
		// This is a very basic form of auto-release.
		// A more robust system would track object lifetimes.
		// For now, we assume the final result is managed by the caller (e.g., REPL).
		// if prevResult != nil && prevResult != result && prevResult.Type() != object.RETURN_VALUE_OBJ && prevResult.Type() != object.ERROR_OBJ {
		//     prevResult.Free()
		// }
		// prevResult = result

		switch result := result.(type) {
		case *object.ReturnValue:
			// Unwrap the return value. The ReturnValue object itself should be freed.
			// The value inside might need to be returned.
			val := result.Value
			// result.Free() // Free the wrapper
			return val // Return the inner value
		case *object.Error:
			// Return the error. The Error object itself is returned.
			// result.Free() // Usually not freed if returned, caller handles it.
			return result

		}
	}
	// Caller (e.g., REPL) is responsible for freeing the final 'result'
	// unless it's NULL, TRUE, FALSE which are global constants.
	return result
}

// eval is the main recursive evaluation function.
// eval is the main recursive evaluation function.
func (i *Interpreter) eval(node ast.Node, env *object.Environment) object.Object {
	switch node := node.(type) {
	// Statements
	case *ast.Program:
		return i.evalProgram(node, env)
	case *ast.ExpressionStatement:
		fmt.Fprintln(os.Stderr, "[DEBUG] eval: Found ExpressionStatement")
		result := i.eval(node.Expression, env)
		if result != nil {
			fmt.Fprintln(os.Stderr, "[DEBUG] eval: ExpressionStatement result type:", result.Type())
			if result.Type() == object.NULL_OBJ {
				fmt.Fprintln(os.Stderr, "[DEBUG] eval: Result is NULL_OBJ, not returning")
				return NULL
			}
		}
		return result
	case *ast.BreakStatement:
		return &object.BreakSignal{}
	case *ast.ContinueStatement:
		return &object.ContinueSignal{}
	case *ast.WhileStatement:
		return i.evalWhile(node, env)
	case *ast.ForStatement:
		return i.evalFor(node, env)
	case *ast.LetStatement:
		// ابتدا یک placeholder تنظیم کنید برای پشتیبانی از بازگشتی
		// Note: Setting NULL directly in env might cause issues if NULL is a global.
		// It's better to create a new null object or handle it specially in env.Set.
		// For simplicity, we'll proceed but note the potential issue.
		// A better approach might be to not set it initially or use a special marker.
		env.Set(node.Name.Value, NULL)
		val := i.eval(node.Value, env)
		if isError(val) {
			return val
		}
		env.Set(node.Name.Value, val)
		// LetStatement itself returns NULL
		return NULL
	case *ast.AssignStatement:
		val := i.eval(node.Value, env)
		if isError(val) {
			return val
		}
		// بررسی نوع Target
		switch target := node.Target.(type) {
		case *ast.Identifier:
			// تخصیص به یک شناسه ساده
			env.Set(target.Value, val)
		case *ast.IndexExpression:
			// تخصیص به یک عبارت شاخص‌گذاری (مانند map[key])
			left := i.eval(target.Left, env)
			if isError(left) {
				return left
			}
			index := i.eval(target.Index, env)
			if isError(index) {
				return index
			}

			// مدیریت آرایه‌ها
			if array, ok := left.(*object.Array); ok {
				if idx, ok := index.(*object.Integer); ok {
					if idx.Value < 0 || int(idx.Value) >= len(array.Elements) {
						return newError("index out of range: %d", idx.Value)
					}
					// آزاد کردن مقدار قبلی
					if array.Elements[idx.Value] != nil {
						array.Elements[idx.Value].Free()
					}
					array.Elements[idx.Value] = val
					return NULL
				}
			}

			// مدیریت مپ‌ها
			if hash, ok := left.(*object.Hash); ok {
				hashKey, ok := index.(object.Hashable)
				if !ok {
					return newError("unusable as hash key: %s", index.Type())
				}

				// آزاد کردن مقدار قبلی اگر وجود داشت
				if pair, exists := hash.Pairs[hashKey.HashKey()]; exists {
					pair.Value.Free()
				}

				hash.Pairs[hashKey.HashKey()] = object.HashPair{Key: index, Value: val}
				return NULL
			}

			return newError("invalid assignment target: expected array or hash, got %s", left.Type())
		default:
			return newError("invalid assignment target: expected identifier or index expression, got %T", target)
		}
		return NULL
	case *ast.ReturnStatement:
		val := i.eval(node.ReturnValue, env)
		if isError(val) {
			return val
		}
		// Wrap the value in a ReturnValue object.
		// The ReturnValue object itself needs to be managed.
		// When unwrapped, the value inside is returned, and the wrapper is freed.
		rv := &object.ReturnValue{Value: val}
		// Note: Who frees 'rv'? It should be freed when it's unwrapped in evalProgram/evalBlockStatement.
		return rv
	case *ast.BlockStatement:
		return i.evalBlockStatement(node, env)
	case *ast.Null:
		return NULL
	// Expressions
	case *ast.IntegerLiteral:
		// Use the new pooled object
		return object.NewInteger(node.Value)
	case *ast.FloatLiteral:
		return object.NewFloat(node.Value)
	case *ast.Boolean:
		// Use the helper function which returns global constants
		return nativeBoolToBooleanObject(node.Value)
	case *ast.StringLiteral:
		// Use the new pooled object
		return object.NewString(node.Value)
	case *ast.PrefixExpression:
		right := i.eval(node.Right, env)
		if isError(right) {
			// Free the error if it's not going to be returned directly
			// In this case, it is returned, so we don't free it here.
			return right
		}
		res := i.evalPrefixExpression(node.Operator, right)
		// Free the operand as it's no longer needed (unless res is the same object, which is unlikely here)
		// For safety, we can check if res != right before freeing right.
		// However, evalPrefixExpression typically creates a new object.
		if res != right { // Safety check
			right.Free()
		}
		return res
	case *ast.InfixExpression:
		left := i.eval(node.Left, env)
		if isError(left) {
			return left
		}
		right := i.eval(node.Right, env)
		if isError(right) {
			// Free left if right is an error and we are returning right
			left.Free()
			return right
		}
		res := i.evalInfixExpression(node.Operator, left, right)
		// Free operands
		// Add safety checks if res could be the same as left or right (e.g., for identity operations)
		if res != left { // Safety check
			left.Free()
		}
		if res != right { // Safety check
			right.Free()
		}
		return res
	case *ast.IfExpression:
		// The result of evalIfExpression should be freed by the caller if not kept.
		return i.evalIfExpression(node, env)
	case *ast.Identifier:
		// The object returned by env.Get should not be freed here, as it's still in use.
		return i.evalIdentifier(node, env)
	case *ast.FunctionDeclaration:
		// ایجاد تابع و ثبت در محیط جاری
		fn := &object.Function{
			Parameters: node.Parameters,
			Env:        env,
			Body:       node.Body,
		}
		env.Set(node.Name.Value, fn)
		return NULL
	case *ast.FunctionLiteral:
		// Function object doesn't use pools in this simple example, but could.
		// It captures the current environment.
		return &object.Function{Parameters: node.Parameters, Env: env, Body: node.Body}
	case *ast.CallExpression:
		fmt.Fprintln(os.Stderr, "[DEBUG] eval: Found CallExpression")
		function := i.eval(node.Function, env)
		if isError(function) {
			fmt.Fprintln(os.Stderr, "[DEBUG] eval: Error evaluating function:", function.Inspect())
			return function
		}

		args := i.evalExpressions(node.Arguments, env)
		if len(args) == 1 && isError(args[0]) {
			fmt.Fprintln(os.Stderr, "[DEBUG] eval: Error evaluating arguments:", args[0].Inspect())
			return args[0]
		}

		result := i.applyFunction(function, args)
		fmt.Fprintln(os.Stderr, "[DEBUG] eval: CallExpression evaluation completed")
		return result
	case *ast.ArrayLiteral:
		elems := i.evalExpressions(node.Elements, env)
		return object.NewArray(elems)
	case *ast.MapLiteral:
		pairs := make(map[object.Object]object.Object)
		for keyNode, valNode := range node.Pairs {
			key := i.eval(keyNode, env)
			val := i.eval(valNode, env)
			pairs[key] = val
		}
		return object.NewMap(pairs)
	case *ast.IndexExpression:
		left := i.eval(node.Left, env)
		if isError(left) {
			return left
		}

		index := i.eval(node.Index, env)
		if isError(index) {
			return index
		}

		return i.evalIndexExpression(left, index)
	case *ast.ImportStatement:
		path := node.Path.Value
		content, err := os.ReadFile(path)
		if err != nil {
			return &object.Error{Message: fmt.Sprintf("import: cannot read module %q: %s", path, err)}
		}
		// پارس ماژول
		l2 := lexer.New(string(content))
		p2 := parser.New(l2)
		program := p2.ParseProgram()
		if len(p2.Errors()) != 0 {
			errorMsg := "import: parse errors in module " + path + ": " + strings.Join(p2.Errors(), ", ")
			return &object.Error{Message: errorMsg}
		}
		// تفسیر ماژول در محیط جدید
		moduleEnv := object.NewEnclosedEnvironment(env)
		modInter := New()
		modInter.env = moduleEnv
		res := modInter.Interpret(program)
		if errObj, ok := res.(*object.Error); ok {
			return &object.Error{Message: fmt.Sprintf("import: runtime error in %q: %s", path, errObj.Message)}
		}
		// برگرداندن محیط ماژول به عنوان شیء مخصوص
		return &object.Module{Env: moduleEnv, Path: path}
	}
	// Default case, return NULL
	return NULL
}

// evalProgram evaluates a program node.
func (i *Interpreter) evalProgram(program *ast.Program, env *object.Environment) object.Object {
	var result object.Object
	for _, statement := range program.Statements {
		result = i.eval(statement, env)

		switch result := result.(type) {
		case *object.ReturnValue:
			// Unwrap the return value and free the wrapper
			val := result.Value
			// result.Free() // Free the wrapper
			return val
		case *object.Error:
			// Return the error
			// result.Free() // Usually not freed if returned
			return result
		}
		// Free intermediate results? This is tricky without RC.
		// For now, we rely on the caller (Interpret) or the final consumer (REPL) to manage the final result.
	}
	return result
}

func (i *Interpreter) evalBlockStatement(block *ast.BlockStatement, env *object.Environment) object.Object {
	var result object.Object = NULL
	for _, stmt := range block.Statements {
		res := i.eval(stmt, env)
		if res != nil {
			// برای دستوراتی که چیزی بر نمی‌گردانند (مثل print)
			if res.Type() == object.NULL_OBJ {
				continue
			}

			switch res.(type) {
			case *object.ReturnValue, *object.Error, *object.BreakSignal, *object.ContinueSignal:
				return res
			}
			result = res
		}
	}
	return result
}

// Helper function to convert Go bool to Object bool
func nativeBoolToBooleanObject(input bool) *object.Boolean {
	if input {
		return TRUE
	}
	return FALSE
}

// evalPrefixExpression evaluates prefix expressions.
func (i *Interpreter) evalPrefixExpression(operator string, right object.Object) object.Object {
	switch operator {
	case "!":
		return i.evalBangOperatorExpression(right)
	case "-":
		return i.evalMinusPrefixOperatorExpression(right)
	default:
		// Error object doesn't need Free here as it's returned
		return newError("unknown operator: %s%s", operator, right.Type())
	}
}

// evalBangOperatorExpression evaluates the '!' operator.
func (i *Interpreter) evalBangOperatorExpression(right object.Object) object.Object {
	switch right {
	case TRUE:
		// TRUE.Free() // Global constant
		return FALSE
	case FALSE:
		// FALSE.Free() // Global constant
		return TRUE
	case NULL:
		// NULL.Free() // Global constant
		return TRUE
	default:
		// right.Free() // Free the operand
		return FALSE
	}
}

// evalMinusPrefixOperatorExpression evaluates the unary '-' operator.
func (i *Interpreter) evalMinusPrefixOperatorExpression(right object.Object) object.Object {
	if right.Type() != object.INTEGER_OBJ {
		// right.Free() // Free the operand
		// Error object doesn't need Free here as it's returned
		return newError("unknown operator: -%s", right.Type())
	}
	// Get the value and create a new integer
	value := right.(*object.Integer).Value
	// right.Free() // Free the operand
	// Return a new pooled integer
	return object.NewInteger(-value)
}

// evalInfixExpression evaluates infix expressions.
func (i *Interpreter) evalInfixExpression(op string, left, right object.Object) object.Object {
	return i.evalBasicInfix(op, left, right)
}

// evalIfExpression evaluates an if expression.
func (i *Interpreter) evalIfExpression(ie *ast.IfExpression, env *object.Environment) object.Object {
	condition := i.eval(ie.Condition, env)
	if isError(condition) {
		// condition.Free() // Not freed if returned
		return condition
	}

	if isTruthy(condition) {
		// condition.Free() // Free the condition
		// Evaluate the consequence block
		res := i.eval(ie.Consequence, env)
		// The result 'res' is returned and managed by the caller.
		return res
	} else if ie.Alternative != nil {
		// condition.Free() // Free the condition
		// Evaluate the alternative block
		res := i.eval(ie.Alternative, env)
		// The result 'res' is returned and managed by the caller.
		return res
	} else {
		// condition.Free() // Free the condition
		// No alternative, return NULL
		// NULL.Free() // Global constant
		return NULL
	}
}

// isTruthy determines if an object is considered 'true' in a boolean context.
func isTruthy(obj object.Object) bool {
	switch obj {
	case NULL:
		// NULL.Free() // Global constant
		return false
	case TRUE:
		// TRUE.Free() // Global constant
		return true
	case FALSE:
		// FALSE.Free() // Global constant
		return false
	default:
		// obj.Free() // Free the operand
		return true
	}
}

// evalIdentifier evaluates an identifier by looking it up in the environment.
func (i *Interpreter) evalIdentifier(node *ast.Identifier, env *object.Environment) object.Object {
	if val, ok := env.Get(node.Value); ok {
		// The object 'val' is returned and still in use in the environment.
		// It should not be freed here.
		return val
	}
	if builtin, ok := i.builtins[node.Value]; ok {
		// Builtin is returned and is a global constant.
		// builtin.Free() // Usually not needed for global constants/builtins
		return builtin
	}
	// Error object doesn't need Free here as it's returned
	return newError("identifier not found: " + node.Value)
}

// evalExpressions evaluates a list of expressions.
// It returns a slice of evaluated objects.
// If any expression results in an error, it returns a slice containing just that error.
func (i *Interpreter) evalExpressions(exps []ast.Expression, env *object.Environment) []object.Object {
	results := make([]object.Object, len(exps))
	for idx, expr := range exps {
		results[idx] = i.eval(expr, env)
		if isError(results[idx]) {
			// در صورت خطا، آزادسازی موارد قبلی
			for j := 0; j < idx; j++ {
				results[j].Free()
			}
			return []object.Object{results[idx]}
		}
	}
	return results
}

func (i *Interpreter) applyFunction(fn object.Object, args []object.Object) object.Object {
	fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: Applying function of type", fn.Type())

	switch fn := fn.(type) {
	case *object.Function:
		fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: It's a user-defined function")
		env := object.NewEnclosedEnvironment(fn.Env)
		for idx, param := range fn.Parameters {
			fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: Setting parameter", param.Value, "to", args[idx].Inspect())
			env.Set(param.Value, args[idx])
		}
		result := i.eval(fn.Body, env)
		fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: Function body evaluated")
		return i.unwrapReturnValue(result)
	case *object.Builtin:
		fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: builtin of type", fn.Type())
		result := fn.Fn(args...)
		fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: returned", result.Type())
		return result

	default:
		fmt.Fprintln(os.Stderr, "[DEBUG] applyFunction: Not a function, type is", fn.Type())
		for _, a := range args {
			a.Free()
		}
		return newError("not a function: %s", fn.Type())
	}
}

// unwrapReturnValue برداشتن مقدار از داخل ReturnValue
func (i *Interpreter) unwrapReturnValue(obj object.Object) object.Object {
	if returnValue, ok := obj.(*object.ReturnValue); ok {
		return returnValue.Value
	}
	return obj
}

// evalIndexExpression: پردازش عبارت ایندکس‌گذاری (مثل array[index] یا map[key])
func (i *Interpreter) evalIndexExpression(left, index object.Object) object.Object {
	switch {
	case left.Type() == object.ARRAY_OBJ && index.Type() == object.INTEGER_OBJ:
		return i.evalArrayIndexExpression(left, index)
	case left.Type() == object.MAP_OBJ && index.Type() == object.STRING_OBJ:
		return i.evalMapIndexExpression(left, index)
	default:
		return newError("index operator not supported: %s[%s]", left.Type(), index.Type())
	}
}

// evalArrayIndexExpression: پردازش ایندکس‌گذاری آرایه‌ها
func (i *Interpreter) evalArrayIndexExpression(array, index object.Object) object.Object {
	arrayObject := array.(*object.Array)
	idx := index.(*object.Integer).Value
	max := int64(len(arrayObject.Elements) - 1)

	if idx < 0 || idx > max {
		return NULL
	}

	// برگرداندن مستقیم عنصر آرایه بدون پیچیدگی اضافی
	return arrayObject.Elements[idx]
}

// evalMapIndexExpression: پردازش ایندکس‌گذاری مپ‌ها
func (i *Interpreter) evalMapIndexExpression(mapObj, key object.Object) object.Object {
	// بررسی اینکه آیا شیء یک مپ است
	if mapObject, ok := mapObj.(*object.Map); ok {
		// جستجوی کلید در مپ
		value, ok := mapObject.Pairs[key]
		if !ok {
			return NULL
		}
		return value
	}

	// بررسی اینکه آیا شیء یک هش است
	if hashObject, ok := mapObj.(*object.Hash); ok {
		// بررسی اینکه کلید قابل هش شدن است
		hashableKey, ok := key.(object.Hashable)
		if !ok {
			return newError("unusable as hash key: %s", key.Type())
		}

		// جستجوی کلید در هش
		pair, ok := hashObject.Pairs[hashableKey.HashKey()]
		if !ok {
			return NULL
		}

		// برگرداندن مقدار مربوط به کلید
		return pair.Value
	}

	return newError("index operator not supported: %s[%s]", mapObj.Type(), key.Type())
}

// newError creates a new error object.
func newError(format string, a ...interface{}) *object.Error {
	// Error object doesn't need Free method called on it here as it's being created to be returned.
	return &object.Error{Message: fmt.Sprintf(format, a...)}
}

// isError checks if an object is an error.
func isError(obj object.Object) bool {
	if obj != nil {
		return obj.Type() == object.ERROR_OBJ
	}
	return false
}

// helper for integer-integer operations
func evalIntegerInteger(op string, lv, rv int64) object.Object {
	switch op {
	case "+":
		return object.NewInteger(lv + rv)
	case "-":
		return object.NewInteger(lv - rv)
	case "*":
		return object.NewInteger(lv * rv)
	case "/":
		if rv == 0 {
			return newError("division by zero")
		}
		return object.NewInteger(lv / rv)
	case "%":
		if rv == 0 {
			return newError("division by zero")
		}
		return object.NewInteger(lv % rv)
	case "<":
		return nativeBoolToBooleanObject(lv < rv)
	case ">":
		return nativeBoolToBooleanObject(lv > rv)
	case "<=":
		return nativeBoolToBooleanObject(lv <= rv)
	case ">=":
		return nativeBoolToBooleanObject(lv >= rv)
	case "==":
		return nativeBoolToBooleanObject(lv == rv)
	case "!=":
		return nativeBoolToBooleanObject(lv != rv)
	}
	return newError("unknown integer operator: %s %s %s",
		object.INTEGER_OBJ, op, object.INTEGER_OBJ)
}

// helper for float-float operations
func evalFloatFloat(op string, lv, rv float64) object.Object {
	switch op {
	case "+":
		return object.NewFloat(lv + rv)
	case "-":
		return object.NewFloat(lv - rv)
	case "*":
		return object.NewFloat(lv * rv)
	case "/":
		if rv == 0.0 {
			return newError("division by zero")
		}
		return object.NewFloat(lv / rv)
	case "<":
		return nativeBoolToBooleanObject(lv < rv)
	case ">":
		return nativeBoolToBooleanObject(lv > rv)
	case "<=":
		return nativeBoolToBooleanObject(lv <= rv)
	case ">=":
		return nativeBoolToBooleanObject(lv >= rv)
	case "==":
		return nativeBoolToBooleanObject(lv == rv)
	case "!=":
		return nativeBoolToBooleanObject(lv != rv)
	}
	return newError("unknown float operator: %s %s %s",
		object.FLOAT_OBJ, op, object.FLOAT_OBJ)
}

func (i *Interpreter) evalBasicInfix(op string, left, right object.Object) object.Object {
	switch l := left.(type) {
	case *object.Integer:
		switch r := right.(type) {
		case *object.Integer:
			return evalIntegerInteger(op, l.Value, r.Value)
		case *object.Float:
			// promote left to float
			lf := float64(l.Value)
			return evalFloatFloat(op, lf, r.Value)
		}

	case *object.Float:
		switch r := right.(type) {
		case *object.Float:
			return evalFloatFloat(op, l.Value, r.Value)
		case *object.Integer:
			// promote right to float
			rf := float64(r.Value)
			return evalFloatFloat(op, l.Value, rf)
		}

	case *object.String:
		if op == "+" {
			return object.NewString(l.Value + right.(*object.String).Value)
		}
		if op == "==" || op == "!=" {
			eq := (l.Value == right.(*object.String).Value)
			if op == "!=" {
				eq = !eq
			}
			return nativeBoolToBooleanObject(eq)
		}

	case *object.Boolean:
		if op == "==" || op == "!=" {
			eq := (l.Value == right.(*object.Boolean).Value)
			if op == "!=" {
				eq = !eq
			}
			return nativeBoolToBooleanObject(eq)
		}
	}

	// fallback: unsupported types or operators
	return newError("type mismatch or unknown operator: %s %s %s",
		left.Type(), op, right.Type())
}

// evalWhile: اجرای while تا وقتی شرط true باشد
// Define BreakSignal and ContinueSignal in object package
// and handle in evalWhile and evalFor
// In evalWhile:
// evalWhile: اجرای while تا وقتی شرط true باشد
// evalWhile: اجرای while تا وقتی شرط true باشد
func (i *Interpreter) evalWhile(ws *ast.WhileStatement, env *object.Environment) object.Object {
	for {
		// ارزیابی شرط حلقه
		cond := i.eval(ws.Condition, env)
		if isError(cond) {
			return cond
		}
		if !isTruthy(cond) {
			return NULL
		}

		// ارزیابی بدنه‌ی حلقه
		result := i.eval(ws.Body, env)

		// مدیریت سیگنال‌ها با توجه به نیازهای حافظه
		switch r := result.(type) {
		case *object.BreakSignal:
			r.Free() // آزاد کردن سیگنال
			return NULL
		case *object.ContinueSignal:
			r.Free() // آزاد کردن سیگنال
			continue
		case *object.ReturnValue:
			return r // توسط فراخوانی‌کننده آزاد می‌شود
		case *object.Error:
			return r // توسط فراخوانی‌کننده آزاد می‌شود
		}
	}
}

// evalFor: اجرای for(init; cond; post) { body }
// evalFor evaluates a for loop statement.
// Usage:
//
//	for (var i = 0; i < 5; i = i + 1) { ... }
//
// In evalFor:
// evalFor: اجرای for(init; cond; post) { body }
func (i *Interpreter) evalFor(fs *ast.ForStatement, env *object.Environment) object.Object {
	// اجرای بخش اولیه‌سازی
	if fs.Init != nil {
		initRes := i.eval(fs.Init, env)
		if isError(initRes) {
			return initRes
		}
	}

	for {
		// اگر شرط وجود داشت، آن را ارزیابی کن
		if fs.Condition != nil {
			cond := i.eval(fs.Condition, env)
			if isError(cond) {
				return cond
			}
			if !isTruthy(cond) {
				return NULL
			}
		}

		// ایجاد محیط جدید برای بدنه‌ی حلقه
		blockEnv := object.NewEnclosedEnvironment(env)
		result := i.eval(fs.Body, blockEnv)

		// مدیریت سیگنال‌ها
		switch r := result.(type) {
		case *object.BreakSignal:
			r.Free()
			return NULL
		case *object.ContinueSignal:
			r.Free()
			// پس از continue، حتماً post را اجرا کن
			if fs.Post != nil {
				if postRes := i.eval(fs.Post, env); isError(postRes) {
					return postRes
				}
			}
			continue
		case *object.ReturnValue:
			return r
		case *object.Error:
			return r
		}

		// اجرای بخش post (مثلاً i++)
		if fs.Post != nil {
			postRes := i.eval(fs.Post, env)
			if isError(postRes) {
				return postRes
			}
		}
	}
}
