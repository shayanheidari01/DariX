package native

import (
	"darix/object"
	"math"
	"math/rand"
)

func init() {
	Register("math", map[string]*object.Builtin{
		// Basic math functions
		"math_sqrt":  {Fn: mathSqrt},
		"math_pow":   {Fn: mathPow},
		"math_exp":   {Fn: mathExp},
		"math_log":   {Fn: mathLog},
		"math_log10": {Fn: mathLog10},
		"math_log2":  {Fn: mathLog2},
		
		// Trigonometric functions
		"math_sin":  {Fn: mathSin},
		"math_cos":  {Fn: mathCos},
		"math_tan":  {Fn: mathTan},
		"math_asin": {Fn: mathAsin},
		"math_acos": {Fn: mathAcos},
		"math_atan": {Fn: mathAtan},
		"math_atan2": {Fn: mathAtan2},
		
		// Hyperbolic functions
		"math_sinh": {Fn: mathSinh},
		"math_cosh": {Fn: mathCosh},
		"math_tanh": {Fn: mathTanh},
		
		// Rounding and comparison
		"math_ceil":  {Fn: mathCeil},
		"math_floor": {Fn: mathFloor},
		"math_round": {Fn: mathRound},
		"math_trunc": {Fn: mathTrunc},
		"math_max":   {Fn: mathMax},
		"math_min":   {Fn: mathMin},
		
		// Constants
		"math_pi": {Fn: mathPi},
		"math_e":  {Fn: mathE},
		
		// Utility functions
		"math_abs":    {Fn: mathAbs},
		"math_mod":    {Fn: mathMod},
		"math_random": {Fn: mathRandom},
	})
}

// Helper function to get float value from object
func getFloatValue(obj object.Object) (float64, bool) {
	switch v := obj.(type) {
	case *object.Float:
		return v.Value, true
	case *object.Integer:
		return float64(v.Value), true
	default:
		return 0, false
	}
}

// Basic math functions
func mathSqrt(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_sqrt: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_sqrt: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_sqrt: argument must be number, got %s", args[0].Type())
	}
	if val < 0 {
		return object.NewError("math_sqrt: square root of negative number")
	}
	
	return object.NewFloat(math.Sqrt(val))
}

func mathPow(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_pow: access to native module math denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("math_pow: expected 2 arguments, got %d", len(args))
	}
	
	base, ok1 := getFloatValue(args[0])
	exp, ok2 := getFloatValue(args[1])
	if !ok1 || !ok2 {
		return object.NewError("math_pow: arguments must be numbers")
	}
	
	return object.NewFloat(math.Pow(base, exp))
}

func mathExp(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_exp: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_exp: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_exp: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Exp(val))
}

func mathLog(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_log: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_log: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_log: argument must be number, got %s", args[0].Type())
	}
	if val <= 0 {
		return object.NewError("math_log: logarithm of non-positive number")
	}
	
	return object.NewFloat(math.Log(val))
}

func mathLog10(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_log10: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_log10: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_log10: argument must be number, got %s", args[0].Type())
	}
	if val <= 0 {
		return object.NewError("math_log10: logarithm of non-positive number")
	}
	
	return object.NewFloat(math.Log10(val))
}

func mathLog2(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_log2: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_log2: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_log2: argument must be number, got %s", args[0].Type())
	}
	if val <= 0 {
		return object.NewError("math_log2: logarithm of non-positive number")
	}
	
	return object.NewFloat(math.Log2(val))
}

// Trigonometric functions
func mathSin(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_sin: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_sin: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_sin: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Sin(val))
}

func mathCos(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_cos: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_cos: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_cos: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Cos(val))
}

func mathTan(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_tan: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_tan: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_tan: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Tan(val))
}

func mathAsin(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_asin: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_asin: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_asin: argument must be number, got %s", args[0].Type())
	}
	if val < -1 || val > 1 {
		return object.NewError("math_asin: argument out of range [-1, 1]")
	}
	
	return object.NewFloat(math.Asin(val))
}

func mathAcos(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_acos: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_acos: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_acos: argument must be number, got %s", args[0].Type())
	}
	if val < -1 || val > 1 {
		return object.NewError("math_acos: argument out of range [-1, 1]")
	}
	
	return object.NewFloat(math.Acos(val))
}

func mathAtan(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_atan: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_atan: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_atan: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Atan(val))
}

func mathAtan2(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_atan2: access to native module math denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("math_atan2: expected 2 arguments, got %d", len(args))
	}
	
	y, ok1 := getFloatValue(args[0])
	x, ok2 := getFloatValue(args[1])
	if !ok1 || !ok2 {
		return object.NewError("math_atan2: arguments must be numbers")
	}
	
	return object.NewFloat(math.Atan2(y, x))
}

// Hyperbolic functions
func mathSinh(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_sinh: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_sinh: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_sinh: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Sinh(val))
}

func mathCosh(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_cosh: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_cosh: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_cosh: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Cosh(val))
}

func mathTanh(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_tanh: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_tanh: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_tanh: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Tanh(val))
}

// Rounding and comparison functions
func mathCeil(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_ceil: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_ceil: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_ceil: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Ceil(val))
}

func mathFloor(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_floor: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_floor: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_floor: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Floor(val))
}

func mathRound(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_round: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_round: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_round: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Round(val))
}

func mathTrunc(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_trunc: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_trunc: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_trunc: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Trunc(val))
}

func mathMax(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_max: access to native module math denied by policy")
	}
	if len(args) < 2 {
		return object.NewError("math_max: expected at least 2 arguments, got %d", len(args))
	}
	
	max, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_max: all arguments must be numbers")
	}
	
	for i := 1; i < len(args); i++ {
		val, ok := getFloatValue(args[i])
		if !ok {
			return object.NewError("math_max: all arguments must be numbers")
		}
		if val > max {
			max = val
		}
	}
	
	return object.NewFloat(max)
}

func mathMin(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_min: access to native module math denied by policy")
	}
	if len(args) < 2 {
		return object.NewError("math_min: expected at least 2 arguments, got %d", len(args))
	}
	
	min, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_min: all arguments must be numbers")
	}
	
	for i := 1; i < len(args); i++ {
		val, ok := getFloatValue(args[i])
		if !ok {
			return object.NewError("math_min: all arguments must be numbers")
		}
		if val < min {
			min = val
		}
	}
	
	return object.NewFloat(min)
}

// Constants
func mathPi(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_pi: access to native module math denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("math_pi: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewFloat(math.Pi)
}

func mathE(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_e: access to native module math denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("math_e: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewFloat(math.E)
}

// Utility functions
func mathAbs(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_abs: access to native module math denied by policy")
	}
	if len(args) != 1 {
		return object.NewError("math_abs: expected 1 argument, got %d", len(args))
	}
	
	val, ok := getFloatValue(args[0])
	if !ok {
		return object.NewError("math_abs: argument must be number, got %s", args[0].Type())
	}
	
	return object.NewFloat(math.Abs(val))
}

func mathMod(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_mod: access to native module math denied by policy")
	}
	if len(args) != 2 {
		return object.NewError("math_mod: expected 2 arguments, got %d", len(args))
	}
	
	x, ok1 := getFloatValue(args[0])
	y, ok2 := getFloatValue(args[1])
	if !ok1 || !ok2 {
		return object.NewError("math_mod: arguments must be numbers")
	}
	if y == 0 {
		return object.NewError("math_mod: division by zero")
	}
	
	return object.NewFloat(math.Mod(x, y))
}

func mathRandom(args ...object.Object) object.Object {
	if !ModuleAllowed("math") {
		return object.NewError("math_random: access to native module math denied by policy")
	}
	if len(args) != 0 {
		return object.NewError("math_random: expected 0 arguments, got %d", len(args))
	}
	
	return object.NewFloat(rand.Float64())
}
