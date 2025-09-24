// darix/interpreter.go

package interpreter

import (
	"darix/ast"
	"darix/internal/native"
	"darix/lexer"
	"darix/object"
	"darix/parser"
	"fmt"
	"math"
	"os"
	"sort"
	"strconv"
	"strings"
	"time"
)

var (
	NULL  = object.NULL
	TRUE  = object.TRUE
	FALSE = object.FALSE
)

type Interpreter struct {
	env           *object.Environment
	builtins      map[string]*object.Builtin
	loadedModules map[string]object.Object
	callStack     []*object.StackFrame
}

func New() *Interpreter {
	inter := &Interpreter{
		env:           object.NewEnvironment(),
		builtins:      make(map[string]*object.Builtin, 32),
		loadedModules: make(map[string]object.Object, 8),
		callStack:     []*object.StackFrame{},
	}
	inter.initBuiltins()
	return inter
}

// GetEnvironment returns the current environment for REPL introspection
func (i *Interpreter) GetEnvironment() *object.Environment {
	return i.env
}

func (i *Interpreter) initBuiltins() {
	i.builtins = map[string]*object.Builtin{
		"print": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) == 0 {
					fmt.Println()
					return NULL
				}
				// Optimize for single argument case
				if len(args) == 1 {
					result := args[0].Inspect()
					fmt.Println(result)
					return NULL
				}
				// Use strings.Builder for multiple arguments
				var builder strings.Builder
				for i, arg := range args {
					if i > 0 {
						builder.WriteByte(' ')
					}
					builder.WriteString(arg.Inspect())
				}
				result := builder.String()
				fmt.Println(result)
				return NULL
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
				case *object.Hash:
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
					return object.NewString(strconv.FormatInt(v.Value, 10))
				case *object.Float:
					return object.NewString(strconv.FormatFloat(v.Value, 'g', -1, 64))
				case *object.Boolean:
					return object.NewString(strconv.FormatBool(v.Value))
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
				case *object.Float:
					return object.NewInteger(int64(v.Value))
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
		"float": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("float: expected 1 argument, got %d", len(args))
				}
				switch v := args[0].(type) {
				case *object.Float:
					return v
				case *object.Integer:
					return object.NewFloat(float64(v.Value))
				case *object.String:
					val, err := strconv.ParseFloat(v.Value, 64)
					if err != nil {
						return object.NewError("float: cannot convert %s", v.Value)
					}
					return object.NewFloat(val)
				default:
					return object.NewError("float: unsupported type %s", v.Type())
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
				return object.NewString(string(args[0].Type()))
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
				return object.NewString(s)
			},
		},
		"range": {
			Fn: func(args ...object.Object) object.Object {
				argCount := len(args)
				if argCount == 0 || argCount > 3 {
					return object.NewError("range: expected 1-3 arguments, got %d", argCount)
				}

				var start, stop, step int64
				switch argCount {
				case 1:
					start, stop, step = 0, asInt(args[0]), 1
				case 2:
					start, stop, step = asInt(args[0]), asInt(args[1]), 1
				case 3:
					start, stop, step = asInt(args[0]), asInt(args[1]), asInt(args[2])
				}

				if step == 0 {
					return object.NewError("range: step cannot be 0")
				}

				// Pre-calculate capacity
				var capacity int64
				if step > 0 && stop > start {
					capacity = (stop - start + step - 1) / step
				} else if step < 0 && start > stop {
					capacity = (start - stop - step - 1) / (-step)
				}

				elems := make([]object.Object, 0, capacity)
				if step > 0 {
					for i := start; i < stop; i += step {
						elems = append(elems, object.NewInteger(i))
					}
				} else {
					for i := start; i > stop; i += step {
						elems = append(elems, object.NewInteger(i))
					}
				}
				return object.NewArray(elems)
			},
		},
		"abs": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("abs: expected 1 argument, got %d", len(args))
				}
				switch v := args[0].(type) {
				case *object.Integer:
					if v.Value < 0 {
						return object.NewInteger(-v.Value)
					}
					return v
				case *object.Float:
					if v.Value < 0 {
						return object.NewFloat(-v.Value)
					}
					return v
				default:
					return object.NewError("abs: unsupported type %s", v.Type())
				}
			},
		},
		"max": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) < 1 {
					return object.NewError("max: expected at least 1 argument")
				}
				max := args[0]
				for _, arg := range args[1:] {
					if compareObjects(arg, max) > 0 {
						max = arg
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
				for _, arg := range args[1:] {
					if compareObjects(arg, min) < 0 {
						min = arg
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

				var intSum int64
				var floatSum float64
				var hasFloat bool

				for _, elem := range arr.Elements {
					switch v := elem.(type) {
					case *object.Integer:
						if hasFloat {
							floatSum += float64(v.Value)
						} else {
							intSum += v.Value
						}
					case *object.Float:
						if !hasFloat {
							floatSum = float64(intSum) + v.Value
							hasFloat = true
						} else {
							floatSum += v.Value
						}
					default:
						return object.NewError("sum: all elements must be numbers")
					}
				}

				if hasFloat {
					return object.NewFloat(floatSum)
				}
				return object.NewInteger(intSum)
			},
		},
		"reverse": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("reverse: expected 1 argument")
				}
				switch val := args[0].(type) {
				case *object.String:
					runes := []rune(val.Value)
					for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
						runes[i], runes[j] = runes[j], runes[i]
					}
					return object.NewString(string(runes))
				case *object.Array:
					reversed := make([]object.Object, len(val.Elements))
					for i, elem := range val.Elements {
						reversed[len(val.Elements)-1-i] = elem
					}
					return object.NewArray(reversed)
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
					return compareObjects(elements[i], elements[j]) < 0
				})

				return object.NewArray(elements)
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
				return object.NewString(strings.ToUpper(s.Value))
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
				return object.NewString(strings.ToLower(s.Value))
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
				return object.NewString(strings.TrimSpace(s.Value))
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

				newElements := make([]object.Object, len(arr.Elements)+len(args)-1)
				copy(newElements, arr.Elements)
				copy(newElements[len(arr.Elements):], args[1:])

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

				base, exp := asNumber(args[0]), asNumber(args[1])
				if base == nil || exp == nil {
					return object.NewError("pow: both arguments must be numbers")
				}

				// If exponent is integer
				if expInt, ok := exp.(*object.Integer); ok {
					// Base is integer and exponent non-negative: integer fast power
					if baseInt, ok2 := base.(*object.Integer); ok2 && expInt.Value >= 0 {
						// Exponentiation by squaring
						var res int64 = 1
						b := baseInt.Value
						e := expInt.Value
						for e > 0 {
							if (e & 1) == 1 {
								res *= b
							}
							b *= b
							e >>= 1
						}
						return object.NewInteger(res)
					}
					// For negative integer exponent or float base, return float result
					bf := toFloat(base)
					rf := math.Pow(bf, float64(expInt.Value))
					return object.NewFloat(rf)
				}

				// General float exponent case
				bf := toFloat(base)
				ef := toFloat(exp)
				return object.NewFloat(math.Pow(bf, ef))
			},
		},
		"clamp": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 3 {
					return object.NewError("clamp: expected 3 arguments (val, min, max)")
				}
				val, min, max := asInt(args[0]), asInt(args[1]), asInt(args[2])
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
				return object.NewString(time.Now().Format(time.RFC3339))
			},
		},
		"timestamp": {
			Fn: func(args ...object.Object) object.Object {
				return object.NewInteger(time.Now().Unix())
			},
		},
		"exit": {
			Fn: func(args ...object.Object) object.Object {
				code := 0
				if len(args) == 1 {
					if i, ok := args[0].(*object.Integer); ok {
						code = int(i.Value)
					}
				}
				os.Exit(code)
				return NULL
			},
		},
		// Exception creation functions
		"Exception": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) < 1 || len(args) > 2 {
					return object.NewError("Exception: expected 1-2 arguments (type, message), got %d", len(args))
				}

				exceptionType := "Exception"
				message := ""

				if len(args) >= 1 {
					if str, ok := args[0].(*object.String); ok {
						message = str.Value
					} else {
						message = args[0].Inspect()
					}
				}

				if len(args) == 2 {
					// First argument is type, second is message
					if str, ok := args[0].(*object.String); ok {
						exceptionType = str.Value
					}
					if str, ok := args[1].(*object.String); ok {
						message = str.Value
					} else {
						message = args[1].Inspect()
					}
				}

				return object.NewException(exceptionType, message)
			},
		},
		"ValueError": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("ValueError: expected 1 argument (message), got %d", len(args))
				}

				message := ""
				if str, ok := args[0].(*object.String); ok {
					message = str.Value
				} else {
					message = args[0].Inspect()
				}

				return object.NewException(object.VALUE_ERROR, message)
			},
		},
		"TypeError": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("TypeError: expected 1 argument (message), got %d", len(args))
				}

				message := ""
				if str, ok := args[0].(*object.String); ok {
					message = str.Value
				} else {
					message = args[0].Inspect()
				}

				return object.NewException(object.TYPE_ERROR, message)
			},
		},
		"RuntimeError": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("RuntimeError: expected 1 argument (message), got %d", len(args))
				}

				message := ""
				if str, ok := args[0].(*object.String); ok {
					message = str.Value
				} else {
					message = args[0].Inspect()
				}

				return object.NewException(object.RUNTIME_ERROR, message)
			},
		},
		"IndexError": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("IndexError: expected 1 argument (message), got %d", len(args))
				}

				message := ""
				if str, ok := args[0].(*object.String); ok {
					message = str.Value
				} else {
					message = args[0].Inspect()
				}

				return object.NewException(object.INDEX_ERROR, message)
			},
		},
		"KeyError": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("KeyError: expected 1 argument (message), got %d", len(args))
				}

				message := ""
				if str, ok := args[0].(*object.String); ok {
					message = str.Value
				} else {
					message = args[0].Inspect()
				}

				return object.NewException(object.KEY_ERROR, message)
			},
		},
		"ZeroDivisionError": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("ZeroDivisionError: expected 1 argument (message), got %d", len(args))
				}

				message := ""
				if str, ok := args[0].(*object.String); ok {
					message = str.Value
				} else {
					message = args[0].Inspect()
				}

				return object.NewException(object.ZERO_DIV_ERROR, message)
			},
		},
		// Map utility functions
		"keys": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("keys: expected 1 argument (map), got %d", len(args))
				}
				
				switch obj := args[0].(type) {
				case *object.Map:
					keys := make([]object.Object, 0, len(obj.Pairs))
					for key := range obj.Pairs {
						keys = append(keys, key)
					}
					return object.NewArray(keys)
				case *object.Hash:
					keys := make([]object.Object, 0, len(obj.Pairs))
					for _, pair := range obj.Pairs {
						keys = append(keys, pair.Key)
					}
					return object.NewArray(keys)
				default:
					return object.NewError("keys: argument must be a map, got %s", obj.Type())
				}
			},
		},
		"values": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("values: expected 1 argument (map), got %d", len(args))
				}
				
				switch obj := args[0].(type) {
				case *object.Map:
					values := make([]object.Object, 0, len(obj.Pairs))
					for _, value := range obj.Pairs {
						values = append(values, value)
					}
					return object.NewArray(values)
				case *object.Hash:
					values := make([]object.Object, 0, len(obj.Pairs))
					for _, pair := range obj.Pairs {
						values = append(values, pair.Value)
					}
					return object.NewArray(values)
				default:
					return object.NewError("values: argument must be a map, got %s", obj.Type())
				}
			},
		},
		"items": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("items: expected 1 argument (map), got %d", len(args))
				}
				
				switch obj := args[0].(type) {
				case *object.Map:
					items := make([]object.Object, 0, len(obj.Pairs))
					for key, value := range obj.Pairs {
						item := []object.Object{key, value}
						items = append(items, object.NewArray(item))
					}
					return object.NewArray(items)
				case *object.Hash:
					items := make([]object.Object, 0, len(obj.Pairs))
					for _, pair := range obj.Pairs {
						item := []object.Object{pair.Key, pair.Value}
						items = append(items, object.NewArray(item))
					}
					return object.NewArray(items)
				default:
					return object.NewError("items: argument must be a map, got %s", obj.Type())
				}
			},
		},
		// Array utility functions
		"sort": {
			Fn: func(args ...object.Object) object.Object {
				if len(args) != 1 {
					return object.NewError("sort: expected 1 argument (array), got %d", len(args))
				}
				arr, ok := args[0].(*object.Array)
				if !ok {
					return object.NewError("sort: argument must be an array, got %s", args[0].Type())
				}

				elements := make([]object.Object, len(arr.Elements))
				copy(elements, arr.Elements)

				sort.Slice(elements, func(i, j int) bool {
					return compareObjects(elements[i], elements[j]) < 0
				})

				return object.NewArray(elements)
			},
		},
	}
}

// Helper functions for type conversion and comparison
func asInt(obj object.Object) int64 {
	switch o := obj.(type) {
	case *object.Integer:
		return o.Value
	case *object.Float:
		return int64(o.Value)
	case *object.String:
		if i, err := strconv.ParseInt(o.Value, 10, 64); err == nil {
			return i
		}
	}
	return 0
}

func asNumber(obj object.Object) object.Object {
	switch obj.(type) {
	case *object.Integer, *object.Float:
		return obj
	default:
		return nil
	}
}

func toFloat(obj object.Object) float64 {
	switch o := obj.(type) {
	case *object.Integer:
		return float64(o.Value)
	case *object.Float:
		return o.Value
	default:
		return 0.0
	}
}

func compareObjects(a, b object.Object) int {
	switch av := a.(type) {
	case *object.Integer:
		if bv, ok := b.(*object.Integer); ok {
			if av.Value < bv.Value {
				return -1
			} else if av.Value > bv.Value {
				return 1
			}
			return 0
		}
	case *object.Float:
		if bv, ok := b.(*object.Float); ok {
			if av.Value < bv.Value {
				return -1
			} else if av.Value > bv.Value {
				return 1
			}
			return 0
		}
	case *object.String:
		if bv, ok := b.(*object.String); ok {
			return strings.Compare(av.Value, bv.Value)
		}
	}
	return 0
}

func (i *Interpreter) Interpret(program *ast.Program) object.Object {
	return i.evalProgram(program, i.env)
}

func (i *Interpreter) eval(node ast.Node, env *object.Environment) object.Object {
	switch node := node.(type) {
	case *ast.Program:
		return i.evalProgram(node, env)

	case *ast.ExpressionStatement:
		result := i.eval(node.Expression, env)
		// Check if the expression resulted in an exception signal
		if exceptionSignal, isException := result.(*object.ExceptionSignal); isException {
			return exceptionSignal
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
		val := i.eval(node.Value, env)
		if isError(val) {
			return val
		}
		// Check if the expression resulted in an exception signal
		if exceptionSignal, isException := val.(*object.ExceptionSignal); isException {
			return exceptionSignal
		}
		env.Set(node.Name.Value, val)
		return NULL

	case *ast.AssignStatement:
		return i.evalAssignStatement(node, env)

	case *ast.ReturnStatement:
		val := i.eval(node.ReturnValue, env)
		if isError(val) {
			return val
		}
		// Check if the expression resulted in an exception signal
		if exceptionSignal, isException := val.(*object.ExceptionSignal); isException {
			return exceptionSignal
		}
		return &object.ReturnValue{Value: val}

	case *ast.BlockStatement:
		return i.evalBlockStatement(node, env)

	case *ast.StandaloneBlockStatement:
		// Standalone blocks don't create new scope - just execute statements in current environment
		return i.evalBlockStatementWithScoping(node.Block, env, false)

	case *ast.Null:
		return NULL

	case *ast.IntegerLiteral:
		return object.NewInteger(node.Value)

	case *ast.FloatLiteral:
		return object.NewFloat(node.Value)

	case *ast.Boolean:
		return nativeBoolToBooleanObject(node.Value)

	case *ast.StringLiteral:
		return object.NewString(node.Value)

	case *ast.PrefixExpression:
		right := i.eval(node.Right, env)
		if isError(right) {
			return right
		}
		return i.evalPrefixExpression(node.Operator, right)

	case *ast.InfixExpression:
		// Handle logical operators with short-circuit evaluation
		if node.Operator == "&&" {
			left := i.eval(node.Left, env)
			if isError(left) {
				return left
			}
			if !isTruthy(left) {
				return FALSE
			}
			right := i.eval(node.Right, env)
			if isError(right) {
				return right
			}
			return nativeBoolToBooleanObject(isTruthy(right))
		} else if node.Operator == "||" {
			left := i.eval(node.Left, env)
			if isError(left) {
				return left
			}
			if isTruthy(left) {
				return TRUE
			}
			right := i.eval(node.Right, env)
			if isError(right) {
				return right
			}
			return nativeBoolToBooleanObject(isTruthy(right))
		} else {
			// Regular infix operators
			left := i.eval(node.Left, env)
			if isError(left) {
				return left
			}
			right := i.eval(node.Right, env)
			if isError(right) {
				return right
			}
			return i.evalInfixExpression(node.Operator, left, right)
		}

	case *ast.IfExpression:
		return i.evalIfExpression(node, env)

	case *ast.Identifier:
		return i.evalIdentifier(node, env)

	case *ast.FunctionDeclaration:
		fn := &object.Function{
			Name:       node.Name.Value,
			Parameters: node.Parameters,
			Env:        env,
			Body:       node.Body,
		}
		env.Set(node.Name.Value, fn)
		return NULL

	case *ast.FunctionLiteral:
		return &object.Function{
			Parameters: node.Parameters,
			Env:        env,
			Body:       node.Body,
		}

	case *ast.CallExpression:
		function := i.eval(node.Function, env)
		if isError(function) {
			return function
		}
		args := i.evalExpressions(node.Arguments, env)
		if len(args) == 1 && isError(args[0]) {
			return args[0]
		}
		// Push a call frame (Python-like traceback style) using call site position
		fnName := "<call>"
		switch f := function.(type) {
		case *object.Function:
			if f.Name != "" {
				fnName = f.Name
			} else {
				fnName = "<lambda>"
			}
		case *object.Builtin:
			fnName = "<builtin>"
		default:
			fnName = string(function.Type())
		}
		i.pushFrame(fnName, node.Token.File, node.Token.Line, node.Token.Column)
		res := i.applyFunction(function, args)
		// Attach stack trace if exception and none set yet
		if exSig, ok := res.(*object.ExceptionSignal); ok && exSig.Exception != nil {
			if exSig.Exception.StackTrace == nil {
				exSig.Exception.StackTrace = i.currentStackTrace()
			}
		}
		i.popFrame()
		return res

	case *ast.ArrayLiteral:
		elems := i.evalExpressions(node.Elements, env)
		if len(elems) == 1 && isError(elems[0]) {
			return elems[0]
		}
		return object.NewArray(elems)

	case *ast.MapLiteral:
		return i.evalMapLiteral(node, env)

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
		return i.evalImportStatement(node, env)

	case *ast.AssignExpression:
		return i.evalAssignExpression(node, env)

	case *ast.TryStatement:
		return i.evalTryStatement(node, env)

	case *ast.ThrowStatement:
		return i.evalThrowStatement(node, env)

	case *ast.ClassDeclaration:
		return i.evalClassDeclaration(node, env)

	case *ast.MemberExpression:
		return i.evalMemberExpression(node, env)

	default:
		return object.NewError("unknown node type: %T", node)
	}
}

func (i *Interpreter) evalProgram(program *ast.Program, env *object.Environment) object.Object {
	var result object.Object = NULL

	for _, statement := range program.Statements {
		result = i.eval(statement, env)

		switch result := result.(type) {
		case *object.ReturnValue:
			return result.Value
		case *object.Error:
			return result
		case *object.ExceptionSignal:
			// Unhandled exception at program level
			return result
		}
	}

	return result
}

func (i *Interpreter) evalBlockStatement(block *ast.BlockStatement, env *object.Environment) object.Object {
	return i.evalBlockStatementWithScoping(block, env, true)
}

// Helper function to control whether to create new scope
func (i *Interpreter) evalBlockStatementWithScoping(block *ast.BlockStatement, env *object.Environment, createNewScope bool) object.Object {
	var result object.Object = NULL
	var blockEnv *object.Environment

	if createNewScope {
		// Create new enclosed environment for block scope
		blockEnv = object.NewEnclosedEnvironment(env)
	} else {
		// Use the same environment (for loop bodies)
		blockEnv = env
	}

	for _, stmt := range block.Statements {
		result = i.eval(stmt, blockEnv)

		switch result.(type) {
		case *object.ReturnValue, *object.Error, *object.BreakSignal, *object.ContinueSignal, *object.ExceptionSignal:
			return result
		}
	}

	return result
}

func (i *Interpreter) evalAssignStatement(node *ast.AssignStatement, env *object.Environment) object.Object {
	val := i.eval(node.Value, env)
	if isError(val) {
		return val
	}

	switch target := node.Target.(type) {
	case *ast.Identifier:
		// Try to update existing variable first, if not found then create new one
		if !env.Update(target.Value, val) {
			env.Set(target.Value, val)
		}
	case *ast.IndexExpression:
		return i.evalIndexAssignment(target, val, env)
	case *ast.MemberExpression:
		return i.evalMemberAssignment(target, val, env)
	default:
		return object.NewError("invalid assignment target: expected identifier, index expression, or member expression, got %T", target)
	}

	return val
}

func (i *Interpreter) evalIndexAssignment(indexExpr *ast.IndexExpression, val object.Object, env *object.Environment) object.Object {
	left := i.eval(indexExpr.Left, env)
	if isError(left) {
		return left
	}

	index := i.eval(indexExpr.Index, env)
	if isError(index) {
		return index
	}

	switch container := left.(type) {
	case *object.Array:
		idx, ok := index.(*object.Integer)
		if !ok {
			exception := object.NewException(object.TYPE_ERROR, fmt.Sprintf("array index must be integer, got %s", index.Type()))
			return object.NewExceptionSignal(exception)
		}
		if idx.Value < 0 || int(idx.Value) >= len(container.Elements) {
			exception := object.NewException(object.INDEX_ERROR, fmt.Sprintf("array index out of range: %d", idx.Value))
			return object.NewExceptionSignal(exception)
		}
		container.Elements[int(idx.Value)] = val
		return NULL

	case *object.Hash:
		hashKey, ok := index.(object.Hashable)
		if !ok {
			return object.NewError("unusable as hash key: %s", index.Type())
		}
		container.Pairs[hashKey.HashKey()] = object.HashPair{Key: index, Value: val}
		return NULL

	case *object.Map:
		// Find existing key using value comparison
		found := false
		for k := range container.Pairs {
			if object.Equals(k, index) {
				// Update existing key
				delete(container.Pairs, k)
				container.Pairs[index] = val
				found = true
				break
			}
		}
		if !found {
			// Add new key
			container.Pairs[index] = val
		}
		return NULL

	default:
		return object.NewError("index assignment not supported on %s", left.Type())
	}
}

func (i *Interpreter) evalMapLiteral(node *ast.MapLiteral, env *object.Environment) object.Object {
	// Pre-allocate with expected capacity for better performance
	pairs := make(map[object.Object]object.Object, len(node.Pairs))

	for keyNode, valNode := range node.Pairs {
		key := i.eval(keyNode, env)
		if isError(key) {
			return key
		}

		val := i.eval(valNode, env)
		if isError(val) {
			return val
		}

		pairs[key] = val
	}

	return object.NewMap(pairs)
}

func (i *Interpreter) evalImportStatement(node *ast.ImportStatement, env *object.Environment) object.Object {
	path := node.Path.Value

	// Check if module already loaded
	if cached, exists := i.loadedModules[path]; exists {
		return cached
	}

	// Native module: import "go:<name>"
	if strings.HasPrefix(path, "go:") {
		name := strings.TrimPrefix(path, "go:")
		// Capability gating
		if !native.ModuleAllowed(name) {
			return object.NewError("import: native module %q not allowed by policy", name)
		}
		if mod, ok := native.Get(name); ok {
			modEnv := object.NewEnclosedEnvironment(env)
			// Export into module env; inject to global if policy allows
			for fname, fn := range mod.Functions {
				modEnv.Set(fname, fn)
				if native.GetPolicy().InjectToGlobal {
					env.Set(fname, fn)
				}
			}
			module := &object.Module{Env: modEnv, Path: path}
			i.loadedModules[path] = module
			return module
		}
		return object.NewError("import: native module %q not found", name)
	}

	// File-based module fallback
	content, err := os.ReadFile(path)
	if err != nil {
		return object.NewError("import: cannot read module %q: %s", path, err)
	}

	l := lexer.NewWithFile(string(content), path)
	p := parser.New(l)
	program := p.ParseProgram()

	if errors := p.Errors(); len(errors) > 0 {
		return object.NewError("import: parse errors in module %s: %s", path, strings.Join(errors, ", "))
	}

	moduleEnv := object.NewEnclosedEnvironment(env)
	modInter := New()
	modInter.env = moduleEnv
	modInter.loadedModules = i.loadedModules // Share loaded modules

	result := modInter.Interpret(program)
	if isError(result) {
		return object.NewError("import: runtime error in %q: %s", path, result.(*object.Error).Message)
	}

	module := &object.Module{Env: moduleEnv, Path: path}
	i.loadedModules[path] = module
	return module
}

func nativeBoolToBooleanObject(input bool) *object.Boolean {
	if input {
		return TRUE
	}
	return FALSE
}

func (i *Interpreter) evalPrefixExpression(operator string, right object.Object) object.Object {
	switch operator {
	case "!":
		return i.evalBangOperator(right)
	case "-":
		return i.evalMinusPrefix(right)
	default:
		return object.NewError("unknown operator: %s%s", operator, right.Type())
	}
}

func (i *Interpreter) evalBangOperator(right object.Object) object.Object {
	if isTruthy(right) {
		return FALSE
	}
	return TRUE
}

func (i *Interpreter) evalMinusPrefix(right object.Object) object.Object {
	switch obj := right.(type) {
	case *object.Integer:
		return object.NewInteger(-obj.Value)
	case *object.Float:
		return object.NewFloat(-obj.Value)
	default:
		return object.NewError("unknown operator: -%s", right.Type())
	}
}

func (i *Interpreter) evalInfixExpression(op string, left, right object.Object) object.Object {
	// Handle null comparisons
	if left.Type() == object.NULL_OBJ || right.Type() == object.NULL_OBJ {
		switch op {
		case "==":
			return nativeBoolToBooleanObject(left.Type() == right.Type())
		case "!=":
			return nativeBoolToBooleanObject(left.Type() != right.Type())
		default:
			return object.NewError("null can only be compared with == or !=")
		}
	}

	// Type-based evaluation
	switch l := left.(type) {
	case *object.Integer:
		if r, ok := right.(*object.Integer); ok {
			return evalIntegerInfix(op, l.Value, r.Value)
		}
		if r, ok := right.(*object.Float); ok {
			return evalFloatInfix(op, float64(l.Value), r.Value)
		}

	case *object.Float:
		rf := toFloat(right)
		return evalFloatInfix(op, l.Value, rf)

	case *object.String:
		if r, ok := right.(*object.String); ok {
			return evalStringInfix(op, l.Value, r.Value)
		}

	case *object.Boolean:
		if r, ok := right.(*object.Boolean); ok {
			return evalBooleanInfix(op, l.Value, r.Value)
		}
	}

	return object.NewError("unknown operator: %s %s %s", left.Type(), op, right.Type())
}

func evalIntegerInfix(op string, lv, rv int64) object.Object {
	switch op {
	case "+":
		return object.NewInteger(lv + rv)
	case "-":
		return object.NewInteger(lv - rv)
	case "*":
		return object.NewInteger(lv * rv)
	case "/":
		if rv == 0 {
			// Create and throw ZeroDivisionError
			exception := object.NewException(object.ZERO_DIV_ERROR, "division by zero")
			return object.NewExceptionSignal(exception)
		}
		return object.NewInteger(lv / rv)
	case "%":
		if rv == 0 {
			// Create and throw ZeroDivisionError
			exception := object.NewException(object.ZERO_DIV_ERROR, "modulo by zero")
			return object.NewExceptionSignal(exception)
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
	default:
		return object.NewError("unknown integer operator: %s", op)
	}
}

func evalFloatInfix(op string, lv, rv float64) object.Object {
	switch op {
	case "+":
		return object.NewFloat(lv + rv)
	case "-":
		return object.NewFloat(lv - rv)
	case "*":
		return object.NewFloat(lv * rv)
	case "/":
		if rv == 0.0 {
			// Create and throw ZeroDivisionError
			exception := object.NewException(object.ZERO_DIV_ERROR, "division by zero")
			return object.NewExceptionSignal(exception)
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
	default:
		return object.NewError("unknown float operator: %s", op)
	}
}

func evalStringInfix(op string, lv, rv string) object.Object {
	switch op {
	case "+":
		return object.NewString(lv + rv)
	case "==":
		return nativeBoolToBooleanObject(lv == rv)
	case "!=":
		return nativeBoolToBooleanObject(lv != rv)
	case "<":
		return nativeBoolToBooleanObject(lv < rv)
	case ">":
		return nativeBoolToBooleanObject(lv > rv)
	case "<=":
		return nativeBoolToBooleanObject(lv <= rv)
	case ">=":
		return nativeBoolToBooleanObject(lv >= rv)
	default:
		return object.NewError("unknown string operator: %s", op)
	}
}

func evalBooleanInfix(op string, lv, rv bool) object.Object {
	switch op {
	case "==":
		return nativeBoolToBooleanObject(lv == rv)
	case "!=":
		return nativeBoolToBooleanObject(lv != rv)
	default:
		return object.NewError("unknown boolean operator: %s", op)
	}
}

func (i *Interpreter) evalIfExpression(ie *ast.IfExpression, env *object.Environment) object.Object {
	condition := i.eval(ie.Condition, env)
	if isError(condition) {
		return condition
	}

	if isTruthy(condition) {
		return i.eval(ie.Consequence, env)
	} else if ie.Alternative != nil {
		return i.eval(ie.Alternative, env)
	}

	return NULL
}

func (i *Interpreter) evalIdentifier(node *ast.Identifier, env *object.Environment) object.Object {
	if val, ok := env.Get(node.Value); ok {
		return val
	}
	if builtin, ok := i.builtins[node.Value]; ok {
		return builtin
	}
	return object.NewError("identifier not found: " + node.Value)
}

func (i *Interpreter) evalExpressions(exps []ast.Expression, env *object.Environment) []object.Object {
	results := make([]object.Object, 0, len(exps))

	for _, expr := range exps {
		evaluated := i.eval(expr, env)
		if isError(evaluated) {
			return []object.Object{evaluated}
		}
		results = append(results, evaluated)
	}

	return results
}

func (i *Interpreter) applyFunction(fn object.Object, args []object.Object) object.Object {
	switch fn := fn.(type) {
	case *object.Function:
		if len(args) != len(fn.Parameters) {
			return object.NewError("wrong number of arguments: expected %d, got %d", len(fn.Parameters), len(args))
		}

		extendedEnv := object.NewEnclosedEnvironment(fn.Env)
		for idx, param := range fn.Parameters {
			extendedEnv.Set(param.Value, args[idx])
		}

		evaluated := i.eval(fn.Body, extendedEnv)
		return i.unwrapReturnValue(evaluated)

	case *object.Builtin:
		return fn.Fn(args...)

	case *object.Class:
		// Class instantiation - create new instance
		instance := &object.Instance{
			Class:  fn,
			Fields: make(map[string]object.Object),
		}
		
		// Call __init__ method if it exists
		if initMethod, exists := fn.Members["__init__"]; exists {
			if initFn, ok := initMethod.(*object.Function); ok {
				// Prepend 'self' (instance) to arguments
				selfArgs := append([]object.Object{instance}, args...)
				
				// Call __init__ with self as first argument
				extendedEnv := object.NewEnclosedEnvironment(initFn.Env)
				if len(selfArgs) != len(initFn.Parameters) {
					return object.NewError("wrong number of arguments for __init__: expected %d, got %d", len(initFn.Parameters), len(selfArgs))
				}
				
				for idx, param := range initFn.Parameters {
					extendedEnv.Set(param.Value, selfArgs[idx])
				}
				
				result := i.eval(initFn.Body, extendedEnv)
				if isError(result) {
					return result
				}
			}
		}
		
		return instance

	case *object.BoundMethod:
		// Call bound method with self as first argument
		selfArgs := append([]object.Object{fn.Self}, args...)
		
		if len(selfArgs) != len(fn.Fn.Parameters) {
			return object.NewError("wrong number of arguments: expected %d, got %d", len(fn.Fn.Parameters), len(selfArgs))
		}

		extendedEnv := object.NewEnclosedEnvironment(fn.Fn.Env)
		for idx, param := range fn.Fn.Parameters {
			extendedEnv.Set(param.Value, selfArgs[idx])
		}

		evaluated := i.eval(fn.Fn.Body, extendedEnv)
		return i.unwrapReturnValue(evaluated)

	default:
		return object.NewError("not a function: %s", fn.Type())
	}
}

func (i *Interpreter) unwrapReturnValue(obj object.Object) object.Object {
	if returnValue, ok := obj.(*object.ReturnValue); ok {
		return returnValue.Value
	}
	return obj
}

func (i *Interpreter) evalIndexExpression(left, index object.Object) object.Object {
	switch {
	case left.Type() == object.ARRAY_OBJ && index.Type() == object.INTEGER_OBJ:
		return i.evalArrayIndex(left, index)
	case left.Type() == object.MAP_OBJ:
		return i.evalMapIndex(left, index)
	case left.Type() == object.HASH_OBJ:
		return i.evalHashIndex(left, index)
	case left.Type() == object.STRING_OBJ && index.Type() == object.INTEGER_OBJ:
		return i.evalStringIndex(left, index)
	default:
		return object.NewError("index operator not supported: %s[%s]", left.Type(), index.Type())
	}
}

func (i *Interpreter) evalArrayIndex(array, index object.Object) object.Object {
	arrayObject := array.(*object.Array)
	idx := int(index.(*object.Integer).Value)

	if idx < 0 || idx >= len(arrayObject.Elements) {
		// Create and throw IndexError
		exception := object.NewException(object.INDEX_ERROR, fmt.Sprintf("array index out of range: %d", idx))
		return object.NewExceptionSignal(exception)
	}

	return arrayObject.Elements[idx]
}

func (i *Interpreter) evalMapIndex(mapObj, key object.Object) object.Object {
	mapObject := mapObj.(*object.Map)
	for k, v := range mapObject.Pairs {
		if object.Equals(k, key) {
			return v
		}
	}
	return NULL
}

func (i *Interpreter) evalHashIndex(hashObj, key object.Object) object.Object {
	hashObject := hashObj.(*object.Hash)
	hashKey, ok := key.(object.Hashable)
	if !ok {
		return object.NewError("unusable as hash key: %s", key.Type())
	}

	if pair, ok := hashObject.Pairs[hashKey.HashKey()]; ok {
		return pair.Value
	}
	return NULL
}

func (i *Interpreter) evalStringIndex(str, index object.Object) object.Object {
	stringObject := str.(*object.String)
	idx := int(index.(*object.Integer).Value)

	if idx < 0 || idx >= len(stringObject.Value) {
		return NULL
	}

	return object.NewString(string(stringObject.Value[idx]))
}

func (i *Interpreter) evalWhile(ws *ast.WhileStatement, env *object.Environment) object.Object {
	for {
		condition := i.eval(ws.Condition, env)
		if isError(condition) {
			return condition
		}

		if !isTruthy(condition) {
			break
		}

		// Use same environment for while loop body (no new scope)
		result := i.evalBlockStatementWithScoping(ws.Body, env, false)
		switch result.(type) {
		case *object.BreakSignal:
			return NULL
		case *object.ContinueSignal:
			continue
		case *object.ReturnValue, *object.Error, *object.ExceptionSignal:
			return result
		}
	}

	return NULL
}

func (i *Interpreter) evalFor(fs *ast.ForStatement, env *object.Environment) object.Object {
	// Create new scope for the loop
	loopEnv := object.NewEnclosedEnvironment(env)

	// Execute initialization
	if fs.Init != nil {
		if result := i.eval(fs.Init, loopEnv); isError(result) {
			return result
		}
	}

	for {
		// Check condition
		if fs.Condition != nil {
			condition := i.eval(fs.Condition, loopEnv)
			if isError(condition) {
				return condition
			}
			if !isTruthy(condition) {
				break
			}
		}

		// Execute body with loop environment (not new scope)
		result := i.evalBlockStatementWithScoping(fs.Body, loopEnv, false)
		switch result.(type) {
		case *object.BreakSignal:
			return NULL
		case *object.ContinueSignal:
			// Continue to post-iteration
		case *object.ReturnValue, *object.Error, *object.ExceptionSignal:
			return result
		}

		// Execute post-iteration
		if fs.Post != nil {
			if result := i.eval(fs.Post, loopEnv); isError(result) {
				return result
			}
		}
	}

	return NULL
}

func isTruthy(obj object.Object) bool {
	switch obj {
	case NULL, FALSE:
		return false
	case TRUE:
		return true
	default:
		switch o := obj.(type) {
		case *object.Integer:
			return o.Value != 0
		case *object.Float:
			return o.Value != 0.0
		case *object.String:
			return o.Value != ""
		default:
			return true
		}
	}
}

func isError(obj object.Object) bool {
	return obj != nil && obj.Type() == object.ERROR_OBJ
}

func (i *Interpreter) evalAssignExpression(node *ast.AssignExpression, env *object.Environment) object.Object {
	val := i.eval(node.Value, env)
	if isError(val) {
		return val
	}

	switch target := node.Name.(type) {
	case *ast.Identifier:
		// Try to update existing variable first, if not found then create new one
		if !env.Update(target.Value, val) {
			env.Set(target.Value, val)
		}
	case *ast.IndexExpression:
		return i.evalIndexAssignment(target, val, env)
	default:
		return object.NewError("invalid assignment target: expected identifier or index expression, got %T", target)
	}

	return val
}

// ===== EXCEPTION HANDLING EVALUATION =====

// evalTryStatement evaluates try-catch-finally blocks
func (i *Interpreter) evalTryStatement(node *ast.TryStatement, env *object.Environment) object.Object {
	var result object.Object = NULL
	var exception *object.ExceptionSignal

	// Execute try block
	tryResult := i.evalBlockStatement(node.TryBlock, env)

	// Check if an exception was thrown
	if exceptionSignal, isException := tryResult.(*object.ExceptionSignal); isException {
		exception = exceptionSignal
		result = NULL

		// Try to find a matching catch clause
		caught := false
		for _, catchClause := range node.CatchClauses {
			if i.matchesExceptionType(exception.Exception, catchClause) {
				// Create new scope for catch block
				catchEnv := object.NewEnclosedEnvironment(env)

				// Bind exception to variable if specified
				if catchClause.Variable != nil {
					catchEnv.Set(catchClause.Variable.Value, exception.Exception)
				}

				// Execute catch block
				catchResult := i.evalBlockStatement(catchClause.CatchBlock, catchEnv)

				// If catch block throws another exception, propagate it
				if exceptionSignal, isException := catchResult.(*object.ExceptionSignal); isException {
					exception = exceptionSignal
				} else {
					// Exception was handled
					exception = nil
					result = catchResult
					caught = true
					break
				}
			}
		}

		// If exception wasn't caught, we'll re-throw it after finally
		if !caught {
			result = exception
		}
	} else {
		// No exception in try block
		result = tryResult
	}

	// Execute finally block if present
	if node.FinallyBlock != nil {
		finallyResult := i.evalBlockStatement(node.FinallyBlock, env)

		// If finally block throws an exception, it takes precedence
		if exceptionSignal, isException := finallyResult.(*object.ExceptionSignal); isException {
			return exceptionSignal
		}

		// If finally block returns a value, it takes precedence
		if returnValue, isReturn := finallyResult.(*object.ReturnValue); isReturn {
			return returnValue
		}

		// If finally block has break/continue, it takes precedence
		if _, isBreak := finallyResult.(*object.BreakSignal); isBreak {
			return finallyResult
		}
		if _, isContinue := finallyResult.(*object.ContinueSignal); isContinue {
			return finallyResult
		}
	}

	return result
}

// evalThrowStatement evaluates throw/raise statements
func (i *Interpreter) evalThrowStatement(node *ast.ThrowStatement, env *object.Environment) object.Object {
	exceptionObj := i.eval(node.Exception, env)
	if isError(exceptionObj) {
		return exceptionObj
	}

	// If the thrown object is already an exception, use it directly
	if exception, ok := exceptionObj.(*object.Exception); ok {
		return object.NewExceptionSignal(exception)
	}

	// If it's a string, create a RuntimeError with the message
	if str, ok := exceptionObj.(*object.String); ok {
		exception := object.NewException(object.RUNTIME_ERROR, str.Value)
		return object.NewExceptionSignal(exception)
	}

	// For other types, convert to string and create RuntimeError
	message := exceptionObj.Inspect()
	exception := object.NewException(object.RUNTIME_ERROR, message)
	return object.NewExceptionSignal(exception)
}

// matchesExceptionType checks if an exception matches a catch clause
func (i *Interpreter) matchesExceptionType(exception *object.Exception, catchClause *ast.CatchClause) bool {
    // If no exception type specified, catch all exceptions
    if catchClause.ExceptionType == nil {
        return true
    }

    // Check if exception type matches
    return exception.ExceptionType == catchClause.ExceptionType.Value
}

// ===== Python-like Traceback Helpers =====
// pushFrame records a call-site frame for better tracebacks
func (i *Interpreter) pushFrame(functionName, file string, line, column int) {
    i.callStack = append(i.callStack, &object.StackFrame{
        Function: functionName,
        File:     file,
        Line:     line,
        Column:   column,
    })
}

// popFrame removes the latest frame
func (i *Interpreter) popFrame() {
    if len(i.callStack) == 0 {
        return
    }
    i.callStack = i.callStack[:len(i.callStack)-1]
}

// currentStackTrace returns a copy of current frames as a StackTrace object
func (i *Interpreter) currentStackTrace() *object.StackTrace {
    if len(i.callStack) == 0 {
        return &object.StackTrace{Frames: nil}
    }
    frames := make([]*object.StackFrame, len(i.callStack))
    copy(frames, i.callStack)
    return &object.StackTrace{Frames: frames}
}

// evalClassDeclaration evaluates class declarations
func (i *Interpreter) evalClassDeclaration(node *ast.ClassDeclaration, env *object.Environment) object.Object {
	// Create a new class object
	class := &object.Class{
		Name:    node.Name.Value,
		Members: make(map[string]object.Object),
	}

	// Create a new environment for the class body
	classEnv := object.NewEnclosedEnvironment(env)
	
	// Evaluate the class body to collect methods and class variables
	for _, stmt := range node.Body.Statements {
		switch s := stmt.(type) {
		case *ast.FunctionDeclaration:
			// Add method to class
			fn := &object.Function{
				Name:       s.Name.Value,
				Parameters: s.Parameters,
				Env:        classEnv,
				Body:       s.Body,
			}
			class.Members[s.Name.Value] = fn
		case *ast.LetStatement:
			// Add class variable
			val := i.eval(s.Value, classEnv)
			if isError(val) {
				return val
			}
			class.Members[s.Name.Value] = val
		}
	}

	// Register the class in the environment
	env.Set(node.Name.Value, class)
	return NULL
}

// evalMemberExpression evaluates member access expressions like obj.prop
func (i *Interpreter) evalMemberExpression(node *ast.MemberExpression, env *object.Environment) object.Object {
	left := i.eval(node.Left, env)
	if isError(left) {
		return left
	}

	switch obj := left.(type) {
	case *object.Instance:
		// Check instance fields first
		if field, exists := obj.Fields[node.Property.Value]; exists {
			return field
		}
		// Then check class methods
		if method, exists := obj.Class.Members[node.Property.Value]; exists {
			// For methods, we need to bind 'self' to the instance
			if fn, ok := method.(*object.Function); ok {
				return &object.BoundMethod{
					Self: obj,
					Fn:   fn,
				}
			}
			return method
		}
		return object.NewError("property '%s' not found on instance of class '%s'", node.Property.Value, obj.Class.Name)
	
	case *object.Class:
		// Access class members directly
		if member, exists := obj.Members[node.Property.Value]; exists {
			return member
		}
		return object.NewError("property '%s' not found on class '%s'", node.Property.Value, obj.Name)
	
	default:
		return object.NewError("member access not supported on type %T", obj)
	}
}

// evalMemberAssignment handles member assignment like obj.prop = value
func (i *Interpreter) evalMemberAssignment(memberExpr *ast.MemberExpression, val object.Object, env *object.Environment) object.Object {
	left := i.eval(memberExpr.Left, env)
	if isError(left) {
		return left
	}

	switch obj := left.(type) {
	case *object.Instance:
		// Set instance field
		obj.Fields[memberExpr.Property.Value] = val
		return val
	case *object.Class:
		// Set class member
		obj.Members[memberExpr.Property.Value] = val
		return val
	default:
		return object.NewError("member assignment not supported on type %T", obj)
	}
}
