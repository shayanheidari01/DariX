package object

import (
	"strings"
	"sync"
)

// Object pools for frequently allocated objects
var (
	integerPool = sync.Pool{
		New: func() interface{} {
			return &Integer{}
		},
	}
	floatPool = sync.Pool{
		New: func() interface{} {
			return &Float{}
		},
	}
	stringPool = sync.Pool{
		New: func() interface{} {
			return &String{}
		},
	}
	arrayPool = sync.Pool{
		New: func() interface{} {
			return &Array{}
		},
	}
)

func resetInteger(i *Integer) {
	i.Value = 0
}

func resetFloat(f *Float) {
	f.Value = 0
}

func resetString(s *String) {
	s.Value = ""
}

func resetArray(a *Array) {
	a.Elements = nil
}

// Optimized constructors using object pools
func NewIntegerFromPool(value int64) *Integer {
	// Use small integer cache for common values
	if value >= 0 && value < 256 {
		return smallIntegers[value]
	}

	obj := integerPool.Get().(*Integer)
	obj.Value = value
	return obj
}

func NewFloatFromPool(value float64) *Float {
	obj := floatPool.Get().(*Float)
	obj.Value = value
	return obj
}
func NewStringFromPool(value string) *String {
	obj := stringPool.Get().(*String)
	obj.Value = value
	return obj
}

func NewArrayFromPool(elements []Object) *Array {
	obj := arrayPool.Get().(*Array)
	obj.Elements = elements
	return obj
}

func (i *Integer) ReturnToPool() {
	if i.Value >= 0 && i.Value < 256 {
		// Don't return cached small integers to pool
		return
	}
	resetInteger(i)
	integerPool.Put(i)
}

func (f *Float) ReturnToPool() {
	resetFloat(f)
	floatPool.Put(f)
}

func (s *String) ReturnToPool() {
	resetString(s)
	stringPool.Put(s)
}

func (a *Array) ReturnToPool() {
	resetArray(a)
	arrayPool.Put(a)
}

// Fast arithmetic operations with pooling
func AddIntegers(left, right *Integer) *Integer {
	return NewIntegerFromPool(left.Value + right.Value)
}

func SubIntegers(left, right *Integer) *Integer {
	return NewIntegerFromPool(left.Value - right.Value)
}

func MulIntegers(left, right *Integer) *Integer {
	return NewIntegerFromPool(left.Value * right.Value)
}

func DivIntegers(left, right *Integer) Object {
	if right.Value == 0 {
		return NewError("division by zero")
	}
	return NewIntegerFromPool(left.Value / right.Value)
}

func ModIntegers(left, right *Integer) Object {
	if right.Value == 0 {
		return NewError("modulo by zero")
	}
	return NewIntegerFromPool(left.Value % right.Value)
}

// Fast float operations with pooling
func AddFloats(left, right *Float) *Float {
	return NewFloatFromPool(left.Value + right.Value)
}

func SubFloats(left, right *Float) *Float {
	return NewFloatFromPool(left.Value - right.Value)
}

func MulFloats(left, right *Float) *Float {
	return NewFloatFromPool(left.Value * right.Value)
}

func DivFloats(left, right *Float) Object {
	if right.Value == 0 {
		return NewError("division by zero")
	}
	return NewFloatFromPool(left.Value / right.Value)
}

func ConcatStrings(left, right *String) *String {
	return NewStringFromPool(left.Value + right.Value)
}

// Batch string concatenation for multiple strings
func ConcatMultipleStrings(parts []*String) *String {
	if len(parts) == 0 {
		return NewStringFromPool("")
	}
	if len(parts) == 1 {
		return parts[0]
	}

	var builder strings.Builder
	for _, s := range parts {
		builder.WriteString(s.Value)
	}

	return NewStringFromPool(builder.String())
}
