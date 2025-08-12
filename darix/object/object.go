// darix/object.go

package object

import (
	"bytes"
	"darix/ast"
	"fmt"
	"hash/fnv"
	"strconv"
	"strings"
	"sync"
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
)

const (
	BREAK_SIGNAL    = "BREAK_SIGNAL"
	CONTINUE_SIGNAL = "CONTINUE_SIGNAL"
)

type Object interface {
	Type() ObjectType
	Inspect() string
	// Free آزاد کردن منابع (در صورت نیاز) - برای پول‌ها
	Free()
}

// Pool برای اشیاء کوچک
var (
	booleanPool = sync.Pool{
		New: func() interface{} {
			return &Boolean{}
		},
	}
	// اضافه شد
	stringPool = sync.Pool{
		New: func() interface{} {
			return &String{}
		},
	}
	// اضافه شد
	arrayPool = sync.Pool{
		New: func() interface{} {
			return &Array{}
		},
	}
	// اضافه شد
	mapPool = sync.Pool{
		New: func() interface{} {
			return &Map{}
		},
	}
)

type Integer struct {
	Value int64
}

func NewInteger(value int64) *Integer {
	return &Integer{Value: value}
}

func (i *Integer) Type() ObjectType { return INTEGER_OBJ }
func (i *Integer) Inspect() string  { return strconv.FormatInt(i.Value, 10) }
func (i *Integer) Free() {
	// مقداردهی اولیه مجدد برای اطمینان از عدم استفاده دوباره نادرست
	// i.Value = 0
	// integerPool.Put(i)
}

type Boolean struct {
	Value bool
}

func NewBoolean(value bool) *Boolean {
	b := booleanPool.Get().(*Boolean)
	b.Value = value
	return b
}

func (b *Boolean) Type() ObjectType { return BOOLEAN_OBJ }
func (b *Boolean) Inspect() string  { return strconv.FormatBool(b.Value) }
func (b *Boolean) Free() {
	// مقداردهی اولیه مجدد
	b.Value = false
	booleanPool.Put(b)
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

// String - تغییر داده شد برای استفاده از پول
type String struct {
	Value string
}

func NewString(value string) *String {
	s := stringPool.Get().(*String)
	s.Value = value
	return s
}

func (s *String) Type() ObjectType { return STRING_OBJ }
func (s *String) Inspect() string {
	return s.Value
}

// Free method added
func (s *String) Free() {
	// می‌توانید فیلدهای بزرگ‌تر را صفر کنید تا ارجاع حافظه را قطع کنید
	// این ممکن است مفید باشد اما نه همیشه ضروری
	s.Value = ""
	stringPool.Put(s)
}

// Array - جدید
type Array struct {
	Elements []Object
}

func NewArray(elements []Object) *Array {
	a := arrayPool.Get().(*Array)
	// اطمینان از ظرفیت مناسب یا اجازه رشد دینامیک
	// در اینجا فرض می‌کنیم ظرفیت اولیه کافی است یا لازم نیست
	// در عمل ممکن است نیاز به مدیریت ظرفیت باشد
	a.Elements = elements
	return a
}

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
func (a *Array) Free() {
	// آزاد کردن عناصر داخلی (در صورت نیاز)
	for i := range a.Elements {
		// اینجا نیاز به یک سیستم مدیریت حافظه پیشرفته‌تر داریم
		// برای سادگی، فرض می‌کنیم عناصر خودشان از پول استفاده می‌کنند
		// و یا اینکه VM مراقب آزاد کردنشان است.
		// می‌توانیم آرایه را خالی کنیم تا ارجاعات را قطع کنیم.
		if a.Elements[i] != nil {
			a.Elements[i].Free()
			a.Elements[i] = nil
		}
	}
	a.Elements = a.Elements[:0] // یا a.Elements = nil
	arrayPool.Put(a)
}

// Map - جدید
type Map struct {
	Pairs map[Object]Object
}

func NewMap(pairs map[Object]Object) *Map {
	m := mapPool.Get().(*Map)
	m.Pairs = pairs
	return m
}

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
func (m *Map) Free() {
	// آزاد کردن جفت‌ها (در صورت نیاز)
	for k, v := range m.Pairs {
		// مانند Array، مدیریت حافظه داخلی پیچیده است
		// فرض می‌کنیم VM یا سیستم دیگری مراقب است.
		// می‌توانیم مپ را خالی کنیم.
		if k != nil {
			k.Free()
		}
		if v != nil {
			v.Free()
		}
		delete(m.Pairs, k) // یا m.Pairs = nil
	}
	// اطمینان از اینکه مپ خالی است
	if m.Pairs != nil {
		for k := range m.Pairs {
			// اگر کلیدها/مقادیر Free داشته باشند، باید آن‌ها را هم آزاد کنیم
			// اینجا فرض می‌کنیم که در حلقه بالا انجام شده است.
			delete(m.Pairs, k)
		}
	}
	m.Pairs = nil
	mapPool.Put(m)
}

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
	// قبل از تنظیم، اگر مقدار قبلی وجود داشت، آن را آزاد کنید (در صورت نیاز)
	// این یک سیستم RC ساده است.
	if oldVal, exists := e.store[name]; exists && oldVal != val {
		oldVal.Free()
	}
	e.store[name] = val
	return val
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

// و تابع برای ایجاد شیء NULL
func GetNull() Object {
	return NULL
}
