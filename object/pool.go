package object

import (
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
	
	booleanPool = sync.Pool{
		New: func() interface{} {
			return &Boolean{}
		},
	}
)

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

func NewBooleanFromPool(value bool) *Boolean {
	// Use cached TRUE/FALSE for common values
	if value {
		return TRUE
	}
	return FALSE
}

// Return objects to pool for reuse
func (i *Integer) ReturnToPool() {
	if i.Value >= 0 && i.Value < 256 {
		// Don't return cached small integers to pool
		return
	}
	i.Value = 0
	integerPool.Put(i)
}

func (f *Float) ReturnToPool() {
	f.Value = 0.0
	floatPool.Put(f)
}

func (s *String) ReturnToPool() {
	s.Value = ""
	stringPool.Put(s)
}

func (a *Array) ReturnToPool() {
	a.Elements = nil
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
	if right.Value == 0.0 {
		return NewError("division by zero")
	}
	return NewFloatFromPool(left.Value / right.Value)
}

// Fast string concatenation with pooling
func ConcatStrings(left, right *String) *String {
	return NewStringFromPool(left.Value + right.Value)
}

// Batch string concatenation for multiple strings
func ConcatMultipleStrings(strings []*String) *String {
	if len(strings) == 0 {
		return NewStringFromPool("")
	}
	if len(strings) == 1 {
		return strings[0]
	}
	
	// Simple string concatenation
	result := ""
	for _, s := range strings {
		result += s.Value
	}
	
	return NewStringFromPool(result)
}
