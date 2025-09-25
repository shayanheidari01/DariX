// darix/object.go

package object

import (
	"bytes"
	"darix/ast"
	"fmt"
	"hash/fnv"
	"sort"
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
	COMPILED_FUNCTION_OBJ = "COMPILED_FUNCTION"
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

type keyValueEntry struct {
	key      string
	rendered string
}

func formatSequence(prefix, suffix string, elements []Object) string {
	if len(elements) == 0 {
		return prefix + suffix
	}
	var builder strings.Builder
	builder.Grow(len(prefix) + len(suffix) + len(elements)*4)
	builder.WriteString(prefix)
	for i, elem := range elements {
		if i > 0 {
			builder.WriteString(", ")
		}
		builder.WriteString(elem.Inspect())
	}
	builder.WriteString(suffix)
	return builder.String()
}

func formatObjectPairs(pairs map[Object]Object) string {
	entries := make([]keyValueEntry, 0, len(pairs))
	for key, value := range pairs {
		keyStr := key.Inspect()
		entries = append(entries, keyValueEntry{
			key:      keyStr,
			rendered: fmt.Sprintf("%s: %s", keyStr, value.Inspect()),
		})
	}
	return formatEntries("{", "}", entries)
}

func formatHashPairs(pairs map[HashKey]HashPair) string {
	entries := make([]keyValueEntry, 0, len(pairs))
	for _, pair := range pairs {
		keyStr := pair.Key.Inspect()
		entries = append(entries, keyValueEntry{
			key:      keyStr,
			rendered: fmt.Sprintf("%s:%s", keyStr, pair.Value.Inspect()),
		})
	}
	return formatEntries("{", "}", entries)
}

func formatEntries(prefix, suffix string, entries []keyValueEntry) string {
	if len(entries) == 0 {
		return prefix + suffix
	}
	sort.Slice(entries, func(i, j int) bool {
		if entries[i].key == entries[j].key {
			return entries[i].rendered < entries[j].rendered
		}
		return entries[i].key < entries[j].key
	})
	var builder strings.Builder
	builder.Grow(len(prefix) + len(suffix) + len(entries)*8)
	builder.WriteString(prefix)
	for i, entry := range entries {
		if i > 0 {
			builder.WriteString(", ")
		}
		builder.WriteString(entry.rendered)
	}
	builder.WriteString(suffix)
	return builder.String()
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
func (bm *BoundMethod) Inspect() string {
	return fmt.Sprintf("<bound method %s of %s>", bm.Fn.Name, bm.Self.Class.Name)
}
func (bm *BoundMethod) Free() { bm.Self = nil; bm.Fn = nil }

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

// Position represents a position in source code
type Position struct {
	Filename string
	Line     int
	Column   int
}

func (p Position) String() string {
	if p.Filename != "" {
		return fmt.Sprintf("%s:%d:%d", p.Filename, p.Line, p.Column)
	}
	return fmt.Sprintf("line %d:%d", p.Line, p.Column)
}

// StackFrame represents a single frame in the call stack
type StackFrame struct {
	FunctionName string
	Position     Position
	Context      string // Source code context around the error
}

func (sf StackFrame) String() string {
	result := fmt.Sprintf("  at %s (%s)", sf.FunctionName, sf.Position.String())
	if sf.Context != "" {
		result += "\n    " + sf.Context
	}
	return result
}

// Enhanced Error struct with detailed information
type Error struct {
	Message    string
	ErrorType  string        // Error type (e.g., "SyntaxError", "RuntimeError", "TypeError")
	Position   Position      // Where the error occurred
	StackTrace []StackFrame  // Call stack
	Suggestion string        // Helpful suggestion for fixing the error
}

func (e *Error) Type() ObjectType { return ERROR_OBJ }

func (e *Error) Inspect() string {
	var out strings.Builder
	
	// Error header
	if e.ErrorType != "" {
		out.WriteString(e.ErrorType)
	} else {
		out.WriteString("Runtime error")
	}
	
	if e.Position.Line > 0 {
		out.WriteString(" at ")
		out.WriteString(e.Position.String())
	}
	
	out.WriteString(": ")
	out.WriteString(e.Message)
	
	// Add suggestion if available
	if e.Suggestion != "" {
		out.WriteString("\n\nSuggestion: ")
		out.WriteString(e.Suggestion)
	}
	
	// Add stack trace if available
	if len(e.StackTrace) > 0 {
		out.WriteString("\n\nStack trace:")
		for _, frame := range e.StackTrace {
			out.WriteString("\n")
			out.WriteString(frame.String())
		}
	}
	
	return out.String()
}

func (e *Error) Free() {
	// Clear all fields
	e.Message = ""
	e.ErrorType = ""
	e.Position = Position{}
	e.StackTrace = nil
	e.Suggestion = ""
}

// Helper functions for creating enhanced errors

// NewError creates a basic error with message
func NewError(format string, args ...interface{}) *Error {
	return &Error{
		Message:   fmt.Sprintf(format, args...),
		ErrorType: "RuntimeError",
	}
}

// NewErrorWithPosition creates an error with position information
func NewErrorWithPosition(pos Position, format string, args ...interface{}) *Error {
	return &Error{
		Message:   fmt.Sprintf(format, args...),
		ErrorType: "RuntimeError",
		Position:  pos,
	}
}

// NewSyntaxError creates a syntax error with position
func NewSyntaxError(pos Position, format string, args ...interface{}) *Error {
	return &Error{
		Message:   fmt.Sprintf(format, args...),
		ErrorType: "SyntaxError",
		Position:  pos,
	}
}

// NewTypeError creates a type error
func NewTypeError(pos Position, format string, args ...interface{}) *Error {
	return &Error{
		Message:   fmt.Sprintf(format, args...),
		ErrorType: "TypeError",
		Position:  pos,
	}
}

// NewNameError creates a name error (undefined variable/function)
func NewNameError(pos Position, name string) *Error {
	return &Error{
		Message:    fmt.Sprintf("name '%s' is not defined", name),
		ErrorType:  "NameError",
		Position:   pos,
		Suggestion: fmt.Sprintf("Did you mean to declare '%s' first?", name),
	}
}

// AddStackFrame adds a stack frame to the error
func (e *Error) AddStackFrame(functionName string, pos Position, context string) {
	frame := StackFrame{
		FunctionName: functionName,
		Position:     pos,
		Context:      context,
	}
	e.StackTrace = append(e.StackTrace, frame)
}

// WithSuggestion adds a helpful suggestion to the error
func (e *Error) WithSuggestion(suggestion string) *Error {
	e.Suggestion = suggestion
	return e
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

// CompiledFunction represents a function compiled to bytecode for the VM
type CompiledFunction struct {
    Instructions []byte // code.Instructions, but avoid import cycle here
    NumLocals     int
    NumParameters int
    Name          string
}

func (cf *CompiledFunction) Type() ObjectType { return COMPILED_FUNCTION_OBJ }
func (cf *CompiledFunction) Inspect() string  {
    if cf.Name != "" {
        return fmt.Sprintf("<compiled func %s params=%d locals=%d>", cf.Name, cf.NumParameters, cf.NumLocals)
    }
    return fmt.Sprintf("<compiled func params=%d locals=%d>", cf.NumParameters, cf.NumLocals)
}
func (cf *CompiledFunction) Free() {
    cf.Instructions = nil
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

func (a *Array) Inspect() string { return formatSequence("[", "]", a.Elements) }

// Free method added
func (a *Array) Free() { a.Elements = nil }

// Map
type Map struct {
	Pairs map[Object]Object
}

func NewMap(pairs map[Object]Object) *Map { return &Map{Pairs: pairs} }

func (m *Map) Type() ObjectType { return MAP_OBJ }
func (m *Map) Inspect() string  { return formatObjectPairs(m.Pairs) }

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

// Delete removes a variable from the scope where it was defined.
// Returns true if the variable was found and deleted, false otherwise.
func (e *Environment) Delete(name string) bool {
    // Check current scope first
    if _, exists := e.store[name]; exists {
        delete(e.store, name)
        return true
    }
    // Recurse into outer scope if present
    if e.outer != nil {
        return e.outer.Delete(name)
    }
    return false
}

// Outer returns the immediately enclosing environment (one scope up), or nil if none.
func (e *Environment) Outer() *Environment {
    return e.outer
}

// HasLocal reports whether the given name exists in the current environment's local store.
// This does not check outer scopes.
func (e *Environment) HasLocal(name string) bool {
    _, exists := e.store[name]
    return exists
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

// Removed duplicate NewError function - using enhanced version above

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
func (h *Hash) Inspect() string  { return formatHashPairs(h.Pairs) }

func (h *Hash) Free() {
	// آزادسازی اشیاء داخل Pairs
	for _, pair := range h.Pairs {
		pair.Key.Free()
		pair.Value.Free()
	}
	h.Pairs = nil
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
