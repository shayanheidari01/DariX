// darix/object.go

package object

import (
	"bytes"
	"darix/ast"
	"fmt"
	"hash/fnv"
	"strconv"
	"strings"
)

type ObjectType string

const (
	INTEGER_OBJ      = "INTEGER"
	FLOAT_OBJ        = "FLOAT"
	BOOLEAN_OBJ      = "BOOLEAN"
	NULL_OBJ         = "NULL"
	RETURN_VALUE_OBJ = "RETURN_VALUE"
	ERROR_OBJ        = "ERROR"
	FUNCTION_OBJ     = "FUNCTION"
	STRING_OBJ       = "STRING"
	ARRAY_OBJ        = "ARRAY"
	MAP_OBJ          = "MAP"
	BUILTIN_OBJ      = "BUILTIN"
	HASH_OBJ         = "HASH"
	// Exception types
	EXCEPTION_OBJ   = "EXCEPTION"
	STACK_TRACE_OBJ = "STACK_TRACE"
	// Class system
	CLASS_OBJ        = "CLASS"
	INSTANCE_OBJ     = "INSTANCE"
	BOUND_METHOD_OBJ = "BOUND_METHOD"
)

const (
	BREAK_SIGNAL    = "BREAK_SIGNAL"
	CONTINUE_SIGNAL = "CONTINUE_SIGNAL"
	// Exception control signal
	EXCEPTION_SIGNAL = "EXCEPTION_SIGNAL"
)

type Object interface {
	Type() ObjectType
	Inspect() string
	// Free آزاد کردن منابع (در صورت نیاز) - برای پول‌ها
	Free()
}

// Class represents a Python-like class with methods and class attributes
type Class struct {
    Name    string
    Members map[string]Object // methods and class attributes
}

func NewClass(name string) *Class { return &Class{Name: name, Members: make(map[string]Object)} }

func (c *Class) Type() ObjectType { return CLASS_OBJ }
func (c *Class) Inspect() string  { return fmt.Sprintf("<class %s>", c.Name) }
func (c *Class) Free()            { c.Members = nil }

// Instance represents an instance of a Class with its own fields
type Instance struct {
    Class  *Class
    Fields map[string]Object
}

func NewInstance(cls *Class) *Instance { return &Instance{Class: cls, Fields: make(map[string]Object)} }

func (inst *Instance) Type() ObjectType { return INSTANCE_OBJ }
func (inst *Instance) Inspect() string  { return fmt.Sprintf("<%s instance>", inst.Class.Name) }
func (inst *Instance) Free()            { inst.Class = nil; inst.Fields = nil }

// BoundMethod binds a function to an instance (self)
type BoundMethod struct {
    Self *Instance
    Fn   *Function
}

func (bm *BoundMethod) Type() ObjectType { return BOUND_METHOD_OBJ }
func (bm *BoundMethod) Inspect() string  { return fmt.Sprintf("<bound method %s of %s>", bm.Fn.Name, bm.Self.Class.Name) }
func (bm *BoundMethod) Free()            { bm.Self = nil; bm.Fn = nil }

// Pool for small integers (0-255) for better performance
var (
	smallIntegers [256]*Integer
)

// Initialize small integer cache
func init() {
	for i := 0; i < 256; i++ {
		smallIntegers[i] = &Integer{Value: int64(i)}
	}
}

type Integer struct {
	Value int64
}

func NewInteger(value int64) *Integer {
	// Use cached small integers for better performance
	if value >= 0 && value < 256 {
		return smallIntegers[value]
	}
	return &Integer{Value: value}
}

func (i *Integer) Type() ObjectType { return INTEGER_OBJ }
func (i *Integer) Inspect() string  { return strconv.FormatInt(i.Value, 10) }
func (i *Integer) Free() {
	// no-op: integers are immutable
}

type Boolean struct {
	Value bool
}

func NewBoolean(value bool) *Boolean {
	// Return canonical singletons to avoid multiple boolean instances
	if value {
		return TRUE
	}
	return FALSE
}

func (b *Boolean) Type() ObjectType { return BOOLEAN_OBJ }
func (b *Boolean) Inspect() string  { return strconv.FormatBool(b.Value) }
func (b *Boolean) Free() {
	// no-op: booleans are immutable singletons in this runtime
}

type Null struct{}

func (n *Null) Type() ObjectType { return NULL_OBJ }
func (n *Null) Inspect() string  { return "null" }
func (n *Null) Free()            {} // چیزی برای آزاد کردن ندارد

type ReturnValue struct {
	Value Object
}

func (rv *ReturnValue) Type() ObjectType { return RETURN_VALUE_OBJ }
func (rv *ReturnValue) Inspect() string  { return rv.Value.Inspect() }
func (rv *ReturnValue) Free() {
	if rv.Value != nil {
		rv.Value.Free()
	}
	// مقداردهی اولیه مجدد
	rv.Value = nil
}

type Error struct {
	Message string
}

func (e *Error) Type() ObjectType { return ERROR_OBJ }
func (e *Error) Inspect() string  { return "Runtime error: " + e.Message }
func (e *Error) Free() {
	// مقداردهی اولیه مجدد
	e.Message = ""
}

type Function struct {
	Name       string
	Parameters []*ast.Identifier
	Body       *ast.BlockStatement
	Env        *Environment
}

func (f *Function) Type() ObjectType { return FUNCTION_OBJ }
func (f *Function) Inspect() string {
	var out bytes.Buffer
	params := make([]string, len(f.Parameters))
	for i, p := range f.Parameters {
		params[i] = p.String()
	}
	out.WriteString("func")
	if f.Name != "" {
		out.WriteString(" ")
		out.WriteString(f.Name)
	}
	out.WriteString("(")
	out.WriteString(strings.Join(params, ", "))
	out.WriteString(") {\n")
	out.WriteString(f.Body.String())
	out.WriteString("\n}")
	return out.String()
}
func (f *Function) Free() {
	// Function ممکن است شامل AST باشد که نیاز به آزادسازی دارد
	// اما در این سطح ساده، فقط فیلدها را صفر می‌کنیم.
	// یک سیستم RC پیشرفته‌تر نیاز دارد.
	f.Parameters = nil
	f.Body = nil
	f.Env = nil
}

// String 
type String struct {
	Value string
}

func NewString(value string) *String { return &String{Value: value} }

func (s *String) Type() ObjectType { return STRING_OBJ }
func (s *String) Inspect() string {
	return s.Value
}

// Free method added
func (s *String) Free() { /* no-op */ }

// Array 
type Array struct {
	Elements []Object
}

func NewArray(elements []Object) *Array { return &Array{Elements: elements} }

func (a *Array) Type() ObjectType {
	return ARRAY_OBJ
}

func (a *Array) Inspect() string {
	var out bytes.Buffer
	elements := make([]string, 0, len(a.Elements))
	for _, e := range a.Elements {
		// استفاده از Inspect برای هر عنصر
		elements = append(elements, e.Inspect())
	}
	out.WriteString("[")
	out.WriteString(strings.Join(elements, ", "))
	out.WriteString("]")
	return out.String()
}

// Free method added
func (a *Array) Free() { a.Elements = nil }

// Map 
type Map struct {
	Pairs map[Object]Object
}

func NewMap(pairs map[Object]Object) *Map { return &Map{Pairs: pairs} }

func (m *Map) Type() ObjectType { return MAP_OBJ }
func (m *Map) Inspect() string {
	var out bytes.Buffer
	pairs := make([]string, 0, len(m.Pairs))
	for key, value := range m.Pairs {
		pairs = append(pairs, key.Inspect()+": "+value.Inspect())
	}
	out.WriteString("{")
	out.WriteString(strings.Join(pairs, ", "))
	out.WriteString("}")
	return out.String()
}

// Free method added
func (m *Map) Free() { m.Pairs = nil }

type Environment struct {
	store map[string]Object
	outer *Environment
}

func NewEnvironment() *Environment {
	return &Environment{
		store: make(map[string]Object),
		outer: nil,
	}
}

func NewEnclosedEnvironment(outer *Environment) *Environment {
	env := NewEnvironment()
	env.outer = outer
	return env
}

func (e *Environment) Get(name string) (Object, bool) {
	obj, ok := e.store[name]
	if !ok && e.outer != nil {
		obj, ok = e.outer.Get(name)
	}
	return obj, ok
}

func (e *Environment) Set(name string, val Object) Object {
	e.store[name] = val
	return val
}

// Update updates an existing variable in the scope where it was defined
func (e *Environment) Update(name string, val Object) bool {
	// First check current scope
	if _, exists := e.store[name]; exists {
		e.store[name] = val
		return true
	}
	// If not found in current scope, check outer scope
	if e.outer != nil {
		return e.outer.Update(name, val)
	}
	// Variable not found in any scope
	return false
}

// GetAll returns all variables in the current environment (not including outer scopes)
func (e *Environment) GetAll() map[string]Object {
	result := make(map[string]Object)
	for k, v := range e.store {
		result[k] = v
	}
	return result
}

// تابع Free برای Environment اضافه نشده است زیرا معمولاً توسط
// مکانیزم scope management در interpreter مدیریت می‌شود.

type BuiltinFunction func(args ...Object) Object

type Builtin struct {
	Fn BuiltinFunction
}

func (b *Builtin) Type() ObjectType { return BUILTIN_OBJ }
func (b *Builtin) Inspect() string  { return "builtin function" }
func (b *Builtin) Free() {
	// Builtin functions معمولاً توسط interpreter مدیریت می‌شوند
	// و نیازی به آزادسازی ندارند.
	b.Fn = nil
}

type Float struct {
	Value float64
}

func NewFloat(v float64) *Float {
	return &Float{Value: v}
}

func (f *Float) Type() ObjectType { return FLOAT_OBJ }
func (f *Float) Inspect() string  { return fmt.Sprintf("%g", f.Value) }
func (f *Float) Free()            {} // در این مثال نیازی به پول نداریم

// BreakSignal stops loops
type BreakSignal struct{}

func (b *BreakSignal) Type() ObjectType { return ObjectType(BREAK_SIGNAL) }
func (b *BreakSignal) Inspect() string  { return "break" }
func (b *BreakSignal) Free()            {}

// ContinueSignal skips to next iteration
type ContinueSignal struct{}

func (c *ContinueSignal) Type() ObjectType { return ObjectType(CONTINUE_SIGNAL) }
func (c *ContinueSignal) Inspect() string  { return "continue" }
func (c *ContinueSignal) Free()            {}

func NewError(format string, args ...any) *Error {
	return &Error{Message: fmt.Sprintf(format, args...)}
}

func Equals(a, b Object) bool {
	if a.Type() != b.Type() {
		return false
	}

	switch aVal := a.(type) {
	case *Integer:
		return aVal.Value == b.(*Integer).Value
	case *Float:
		return aVal.Value == b.(*Float).Value
	case *String:
		return aVal.Value == b.(*String).Value
	case *Boolean:
		return aVal.Value == b.(*Boolean).Value
	default:
		return false
	}
}

type HashKey struct {
	Type  ObjectType
	Value uint64
}

type HashPair struct {
	Key   Object
	Value Object
}

type Hash struct {
	Pairs map[HashKey]HashPair
}

func (h *Hash) Type() ObjectType { return HASH_OBJ }
func (h *Hash) Inspect() string {
	var out strings.Builder
	pairs := []string{}
	for _, pair := range h.Pairs {
		pairs = append(pairs, fmt.Sprintf("%s:%s", pair.Key.Inspect(), pair.Value.Inspect()))
	}
	out.WriteString("{")
	out.WriteString(strings.Join(pairs, ", "))
	out.WriteString("}")
	return out.String()
}
func (h *Hash) Free() {
	// آزادسازی اشیاء داخل Pairs
	for _, pair := range h.Pairs {
		pair.Key.Free()
		pair.Value.Free()
	}
}

type Hashable interface {
	HashKey() HashKey
}

func NewHash(pairs map[Object]Object) *Hash {
	hashPairs := make(map[HashKey]HashPair)
	for k, v := range pairs {
		hashKey, ok := k.(Hashable)
		if !ok {
			continue // یا خطا تولید کنید
		}
		hashPairs[hashKey.HashKey()] = HashPair{Key: k, Value: v}
	}
	return &Hash{Pairs: hashPairs}
}

// اطمینان از اینکه String و Integer به‌عنوان Hashable پیاده‌سازی شده‌اند
func (s *String) HashKey() HashKey {
	h := fnv.New64a()
	h.Write([]byte(s.Value))
	return HashKey{Type: s.Type(), Value: h.Sum64()}
}

func (i *Integer) HashKey() HashKey {
	return HashKey{Type: i.Type(), Value: uint64(i.Value)}
}

// در فایل object.go، بعد از تعاریف دیگر:
type Module struct {
	Env  *Environment
	Path string
}

func (m *Module) Type() ObjectType { return "MODULE" }
func (m *Module) Inspect() string  { return fmt.Sprintf("<module %s>", m.Path) }
func (m *Module) Free() {
	// در صورت نیاز، منابع را آزاد کنید
}

var (
	NULL  = &Null{}
	TRUE  = &Boolean{Value: true}
	FALSE = &Boolean{Value: false}
)

// ===== EXCEPTION SYSTEM =====

// StackFrame represents a single frame in the call stack
type StackFrame struct {
	Function string
	File     string
	Line     int
	Column   int
}

func (sf *StackFrame) String() string {
	return fmt.Sprintf("  at %s (%s:%d:%d)", sf.Function, sf.File, sf.Line, sf.Column)
}

// StackTrace represents the call stack
type StackTrace struct {
	Frames []*StackFrame
}

func (st *StackTrace) Type() ObjectType { return STACK_TRACE_OBJ }
func (st *StackTrace) Inspect() string {
	var out strings.Builder
	out.WriteString("Stack trace:\n")
	for _, frame := range st.Frames {
		out.WriteString(frame.String())
		out.WriteString("\n")
	}
	return out.String()
}
func (st *StackTrace) Free() {
	st.Frames = nil
}

// Exception represents a runtime exception
type Exception struct {
	ExceptionType string      // Exception type (e.g., "ValueError", "TypeError")
	Message       string      // Error message
	StackTrace    *StackTrace // Call stack
	Cause         *Exception  // Chained exception
}

func (e *Exception) Type() ObjectType { return EXCEPTION_OBJ }
func (e *Exception) Inspect() string {
	var out strings.Builder
	out.WriteString(fmt.Sprintf("%s: %s", e.ExceptionType, e.Message))
	if e.StackTrace != nil {
		out.WriteString("\n")
		out.WriteString(e.StackTrace.Inspect())
	}
	if e.Cause != nil {
		out.WriteString("\nCaused by: ")
		out.WriteString(e.Cause.Inspect())
	}
	return out.String()
}
func (e *Exception) Free() {
	if e.StackTrace != nil {
		e.StackTrace.Free()
	}
	if e.Cause != nil {
		e.Cause.Free()
	}
	e.ExceptionType = ""
	e.Message = ""
	e.StackTrace = nil
	e.Cause = nil
}

// ExceptionSignal is used for exception control flow
type ExceptionSignal struct {
	Exception *Exception
}

func (es *ExceptionSignal) Type() ObjectType { return EXCEPTION_SIGNAL }
func (es *ExceptionSignal) Inspect() string {
	if es.Exception != nil {
		return es.Exception.Inspect()
	}
	return "Unhandled exception"
}
func (es *ExceptionSignal) Free() {
	if es.Exception != nil {
		es.Exception.Free()
	}
	es.Exception = nil
}

// Utility functions for creating exceptions
func NewException(exceptionType, message string) *Exception {
	return &Exception{
		ExceptionType: exceptionType,
		Message:       message,
	}
}

func NewExceptionWithCause(exceptionType, message string, cause *Exception) *Exception {
	return &Exception{
		ExceptionType: exceptionType,
		Message:       message,
		Cause:         cause,
	}
}

func NewExceptionSignal(exception *Exception) *ExceptionSignal {
	return &ExceptionSignal{Exception: exception}
}

// Built-in exception types
const (
	VALUE_ERROR     = "ValueError"
	TYPE_ERROR      = "TypeError"
	NAME_ERROR      = "NameError"
	INDEX_ERROR     = "IndexError"
	KEY_ERROR       = "KeyError"
	ZERO_DIV_ERROR  = "ZeroDivisionError"
	RUNTIME_ERROR   = "RuntimeError"
	SYNTAX_ERROR    = "SyntaxError"
	ATTRIBUTE_ERROR = "AttributeError"
	ASSERTION_ERROR = "AssertionError"
)
