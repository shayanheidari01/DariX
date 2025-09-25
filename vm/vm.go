package vm

import (
	"darix/code"
	"darix/compiler"
	"darix/object"
	"fmt"
	"io"
	"os"
	"strings"
)

type VM struct {
	constants []object.Object
	globals   []object.Object

	stack []object.Object
	sp    int // points to next value, top = sp-1

	ip           int
	instructions code.Instructions
	bcMagic      string
	bcVersion    string
	debug        compiler.DebugInfo

	// Optional instruction budget (0 => unlimited). When >0, the VM will
	// decrement per executed instruction and stop with an exception when it reaches 0.
	instrBudget int

	// JIT compiler for hot path optimization
	jit *JITCompiler
}

func (vm *VM) compareOp(op code.Opcode) object.Object {
	left, right, errObj := vm.popTwo()
	if errObj != nil {
		return errObj
	}
	res := vm.execCompare(op, left, right)
	if isError(res) {
		return res
	}
	return vm.pushChecked(res)
}

const (
	StackSize    = 2048
	InitialGlobs = 1024
)

func New(bc *compiler.Bytecode) *VM {
	vm := &VM{
		constants:    bc.Constants,
		globals:      make([]object.Object, InitialGlobs),
		stack:        make([]object.Object, StackSize),
		instructions: bc.Instructions,
		ip:           0,
		bcMagic:      bc.Magic,
		bcVersion:    bc.Version,
		debug:        bc.Debug,
		jit:          NewJITCompiler(),
	}
	return vm
}

// A value <= 0 disables the budget (unlimited).
func (vm *VM) SetInstructionBudget(n int) {
	vm.instrBudget = n
}

// EnableJIT enables or disables JIT compilation
func (vm *VM) EnableJIT(enabled bool) {
	if vm.jit != nil {
		vm.jit.SetEnabled(enabled)
	}
}

// GetJITStats returns JIT compilation statistics
func (vm *VM) GetJITStats() map[string]interface{} {
	if vm.jit != nil {
		return vm.jit.GetStats()
	}
	return map[string]interface{}{"enabled": false}
}

// ResetJIT clears all JIT compilation data
func (vm *VM) ResetJIT() {
	if vm.jit != nil {
		vm.jit.Reset()
	}
}

func (vm *VM) push(o object.Object) object.Object {
	if vm.sp >= StackSize {
		return vm.errorWithLoc("stack overflow")
	}
	vm.stack[vm.sp] = o
	vm.sp++
	return nil
}

func (vm *VM) pop() object.Object {
	if vm.sp == 0 {
		return vm.errorWithLoc("stack underflow")
	}
	vm.sp--
	obj := vm.stack[vm.sp]
	vm.stack[vm.sp] = nil
	return obj
}

func (vm *VM) popChecked() (object.Object, object.Object) {
	obj := vm.pop()
	if isError(obj) {
		return nil, obj
	}
	return obj, nil
}

func (vm *VM) popTwo() (object.Object, object.Object, object.Object) {
	right, err := vm.popChecked()
	if err != nil {
		return nil, nil, err
	}
	left, err := vm.popChecked()
	if err != nil {
		return nil, nil, err
	}
	return left, right, nil
}

func (vm *VM) popThree() (object.Object, object.Object, object.Object, object.Object) {
	third, err := vm.popChecked()
	if err != nil {
		return nil, nil, nil, err
	}
	second, err := vm.popChecked()
	if err != nil {
		return nil, nil, nil, err
	}
	first, err := vm.popChecked()
	if err != nil {
		return nil, nil, nil, err
	}
	return first, second, third, nil
}

func (vm *VM) pushChecked(obj object.Object) object.Object {
	if err := vm.push(obj); err != nil {
		return err
	}
	return nil
}

func (vm *VM) Run() object.Object {
	if vm.bcMagic != "" && vm.bcMagic != compiler.BytecodeMagic {
		return object.NewError("invalid bytecode: magic mismatch")
	}
	for vm.ip = 0; vm.ip < len(vm.instructions); vm.ip++ {
		// Enforce instruction budget if enabled
		if vm.instrBudget > 0 {
			vm.instrBudget--
			if vm.instrBudget == 0 {
				ex := object.NewException(object.RUNTIME_ERROR, "instruction budget exceeded")
				ex.StackTrace = vm.buildStackTrace()
				return object.NewExceptionSignal(ex)
			}
		}

		// JIT compilation: Record execution and detect hot paths
		if vm.jit.RecordExecution(vm.ip) {
			// New hot path detected - for now just log it
			// In a full implementation, we would compile and optimize here
		}

		op := code.Opcode(vm.instructions[vm.ip])
		switch op {
		case code.OpNop:
			// do nothing
		case code.OpConstant:
			operand := readUint16(vm.instructions[vm.ip+1:])
			vm.ip += 2
			if errObj := vm.pushChecked(vm.constants[operand]); errObj != nil {
				return errObj
			}
		case code.OpAdd, code.OpSub, code.OpMul, code.OpDiv, code.OpMod:
			if errObj := vm.binaryOp(op); errObj != nil {
				return errObj
			}
		case code.OpEqual, code.OpNotEqual, code.OpGreaterThan, code.OpLessThan, code.OpGreaterEqual, code.OpLessEqual:
			if errObj := vm.compareOp(op); errObj != nil {
				return errObj
			}
		case code.OpMinus:
			operand, errObj := vm.popChecked()
			if errObj != nil {
				return errObj
			}
			res := vm.execMinus(operand)
			if isError(res) {
				return res
			}
			if errObj := vm.pushChecked(res); errObj != nil {
				return errObj
			}
		case code.OpBang:
			operand, errObj := vm.popChecked()
			if errObj != nil {
				return errObj
			}
			res := nativeBoolToBooleanObject(!isTruthy(operand))
			if errObj := vm.pushChecked(res); errObj != nil {
				return errObj
			}
		case code.OpTrue:
			if errObj := vm.pushChecked(object.TRUE); errObj != nil {
				return errObj
			}
		case code.OpFalse:
			if errObj := vm.pushChecked(object.FALSE); errObj != nil {
				return errObj
			}
		case code.OpNull:
			if errObj := vm.pushChecked(object.NULL); errObj != nil {
				return errObj
			}
		case code.OpPop:
			if _, errObj := vm.popChecked(); errObj != nil {
				return errObj
			}
		case code.OpSetGlobal:
			idx := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			val, errObj := vm.popChecked()
			if errObj != nil {
				return errObj
			}
			vm.setGlobal(idx, val)
		case code.OpGetGlobal:
			idx := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			if errObj := vm.pushChecked(vm.getGlobal(idx)); errObj != nil {
				return errObj
			}
		case code.OpPrint:
			argc := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			if argc < 0 || vm.sp < argc {
				return vm.errorWithLoc("print: invalid argc or stack underflow")
			}
			start := vm.sp - argc
			var b strings.Builder
			for i := start; i < vm.sp; i++ {
				if i > start {
					b.WriteByte(' ')
				}
				b.WriteString(vm.stack[i].Inspect())
			}
			out := b.String()
			// pop argc items
			for i := start; i < vm.sp; i++ {
				vm.stack[i] = nil
			}
			vm.sp = start
			// faster stdout write than fmt.Println
			_, _ = io.WriteString(os.Stdout, out)
			_, _ = io.WriteString(os.Stdout, "\n")
			if errObj := vm.pushChecked(object.NULL); errObj != nil {
				return errObj
			}
		case code.OpJump:
			pos := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip = pos - 1
		case code.OpJumpNotTruthy:
			pos := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			cond, errObj := vm.popChecked()
			if errObj != nil {
				return errObj
			}
			if !isTruthy(cond) {
				vm.ip = pos - 1
			}
		case code.OpArray:
			numElements := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			elements := make([]object.Object, numElements)
			for i := numElements - 1; i >= 0; i-- {
				elem, errObj := vm.popChecked()
				if errObj != nil {
					return errObj
				}
				elements[i] = elem
			}
			if errObj := vm.pushChecked(object.NewArrayFromPool(elements)); errObj != nil {
				return errObj
			}
		case code.OpIndex:
			left, index, errObj := vm.popTwo()
			if errObj != nil {
				return errObj
			}
			res := vm.execIndex(left, index)
			if isError(res) {
				return res
			}
			if errObj := vm.pushChecked(res); errObj != nil {
				return errObj
			}
		case code.OpSetIndex:
			target, index, value, errObj := vm.popThree()
			if errObj != nil {
				return errObj
			}
			if err := vm.execSetIndex(target, index, value); err != nil {
				return err
			}
			if errObj := vm.pushChecked(object.NULL); errObj != nil {
				return errObj
			}
		case code.OpLen:
			operand, errObj := vm.popChecked()
			if errObj != nil {
				return errObj
			}
			res := vm.execLen(operand)
			if isError(res) {
				return res
			}
			if errObj := vm.pushChecked(res); errObj != nil {
				return errObj
			}
		case code.OpType:
			operand, errObj := vm.popChecked()
			if errObj != nil {
				return errObj
			}
			res := vm.execType(operand)
			if errObj := vm.pushChecked(res); errObj != nil {
				return errObj
			}
		default:
			return vm.errorWithLoc("unknown opcode %d", op)
		}
	}

	return object.NULL
}

func (vm *VM) setGlobal(idx int, val object.Object) {
	if idx >= len(vm.globals) {
		newGlobs := make([]object.Object, idx+1)
		copy(newGlobs, vm.globals)
		vm.globals = newGlobs
	}
	vm.globals[idx] = val
}

func (vm *VM) getGlobal(idx int) object.Object {
	if idx >= len(vm.globals) {
		return object.NULL
	}
	if vm.globals[idx] == nil {
		return object.NULL
	}
	return vm.globals[idx]
}

func (vm *VM) execBinary(op code.Opcode, left, right object.Object) object.Object {
	// Fast path for most common cases - integers
	if l, ok := left.(*object.Integer); ok {
		if r, ok := right.(*object.Integer); ok {
			switch op {
			case code.OpAdd:
				return object.AddIntegers(l, r)
			case code.OpSub:
				return object.SubIntegers(l, r)
			case code.OpMul:
				return object.MulIntegers(l, r)
			case code.OpDiv:
				return object.DivIntegers(l, r)
			case code.OpMod:
				return object.ModIntegers(l, r)
			}
		}
	}

	// Fast path for floats
	if l, ok := left.(*object.Float); ok {
		if r, ok := right.(*object.Float); ok {
			switch op {
			case code.OpAdd:
				return object.AddFloats(l, r)
			case code.OpSub:
				return object.SubFloats(l, r)
			case code.OpMul:
				return object.MulFloats(l, r)
			case code.OpDiv:
				return object.DivFloats(l, r)
			}
		}
	}

	// Fast path for strings (concatenation)
	if op == code.OpAdd {
		if l, ok := left.(*object.String); ok {
			if r, ok := right.(*object.String); ok {
				return object.ConcatStrings(l, r)
			}
		}
	}

	return vm.errorWithLoc("unsupported operands for binary op: %s and %s", left.Type(), right.Type())
}

func (vm *VM) binaryOp(op code.Opcode) object.Object {
	left, right, errObj := vm.popTwo()
	if errObj != nil {
		return errObj
	}
	res := vm.execBinary(op, left, right)
	if isError(res) {
		return res
	}
	return vm.pushChecked(res)
}

func (vm *VM) execCompare(op code.Opcode, left, right object.Object) object.Object {
	// Fast path for integers (most common comparison)
	if l, ok := left.(*object.Integer); ok {
		if r, ok := right.(*object.Integer); ok {
			switch op {
			case code.OpEqual:
				return nativeBoolToBooleanObject(l.Value == r.Value)
			case code.OpNotEqual:
				return nativeBoolToBooleanObject(l.Value != r.Value)
			case code.OpGreaterThan:
				return nativeBoolToBooleanObject(l.Value > r.Value)
			case code.OpLessThan:
				return nativeBoolToBooleanObject(l.Value < r.Value)
			case code.OpGreaterEqual:
				return nativeBoolToBooleanObject(l.Value >= r.Value)
			case code.OpLessEqual:
				return nativeBoolToBooleanObject(l.Value <= r.Value)
			}
		}
	}

	// Fast path for floats
	if l, ok := left.(*object.Float); ok {
		if r, ok := right.(*object.Float); ok {
			switch op {
			case code.OpEqual:
				return nativeBoolToBooleanObject(l.Value == r.Value)
			case code.OpNotEqual:
				return nativeBoolToBooleanObject(l.Value != r.Value)
			case code.OpGreaterThan:
				return nativeBoolToBooleanObject(l.Value > r.Value)
			case code.OpLessThan:
				return nativeBoolToBooleanObject(l.Value < r.Value)
			case code.OpGreaterEqual:
				return nativeBoolToBooleanObject(l.Value >= r.Value)
			case code.OpLessEqual:
				return nativeBoolToBooleanObject(l.Value <= r.Value)
			}
		}
	}

	// Fast path for strings
	if l, ok := left.(*object.String); ok {
		if r, ok := right.(*object.String); ok {
			switch op {
			case code.OpEqual:
				return nativeBoolToBooleanObject(l.Value == r.Value)
			case code.OpNotEqual:
				return nativeBoolToBooleanObject(l.Value != r.Value)
			case code.OpGreaterThan:
				return nativeBoolToBooleanObject(l.Value > r.Value)
			case code.OpLessThan:
				return nativeBoolToBooleanObject(l.Value < r.Value)
			case code.OpGreaterEqual:
				return nativeBoolToBooleanObject(l.Value >= r.Value)
			case code.OpLessEqual:
				return nativeBoolToBooleanObject(l.Value <= r.Value)
			}
		}
	}

	// Fast path for booleans
	if l, ok := left.(*object.Boolean); ok {
		if r, ok := right.(*object.Boolean); ok {
			switch op {
			case code.OpEqual:
				return nativeBoolToBooleanObject(l.Value == r.Value)
			case code.OpNotEqual:
				return nativeBoolToBooleanObject(l.Value != r.Value)
			}
		}
	}

	return vm.errorWithLoc("unsupported operands for compare: %s and %s", left.Type(), right.Type())
}

func (vm *VM) execMinus(operand object.Object) object.Object {
	// Fast path for integers
	if o, ok := operand.(*object.Integer); ok {
		return object.NewIntegerFromPool(-o.Value)
	}
	// Fast path for floats
	if o, ok := operand.(*object.Float); ok {
		return object.NewFloatFromPool(-o.Value)
	}
	return vm.errorWithLoc("unsupported operand for prefix -: %s", operand.Type())
}

func (vm *VM) execIndex(left, index object.Object) object.Object {
	switch {
	case left.Type() == object.ARRAY_OBJ && index.Type() == object.INTEGER_OBJ:
		return vm.execArrayIndex(left, index)
	case left.Type() == object.MAP_OBJ:
		return vm.execMapIndex(left, index)
	case left.Type() == object.STRING_OBJ && index.Type() == object.INTEGER_OBJ:
		return vm.execStringIndex(left, index)
	default:
		return vm.errorWithLoc("index operator not supported: %s", left.Type())
	}
}

func (vm *VM) execArrayIndex(arr, index object.Object) object.Object {
	arrayObject := arr.(*object.Array)
	idx := index.(*object.Integer).Value
	max := int64(len(arrayObject.Elements) - 1)

	if idx < 0 || idx > max {
		return object.NULL
	}

	return arrayObject.Elements[idx]
}

func (vm *VM) execMapIndex(mapObj, index object.Object) object.Object {
	switch m := mapObj.(type) {
	case *object.Map:
		value, ok := m.Pairs[index]
		if !ok {
			return object.NULL
		}
		return value
	case *object.Hash:
		key, ok := index.(object.Hashable)
		if !ok {
			return vm.errorWithLoc("unusable as hash key: %T", index)
		}
		pair, ok := m.Pairs[key.HashKey()]
		if !ok {
			return object.NULL
		}
		return pair.Value
	default:
		return vm.errorWithLoc("index operator not supported: %s", mapObj.Type())
	}
}

func (vm *VM) execStringIndex(str, index object.Object) object.Object {
	stringObject := str.(*object.String)
	idx := index.(*object.Integer).Value
	max := int64(len(stringObject.Value) - 1)

	if idx < 0 || idx > max {
		return object.NULL
	}

	return object.NewStringFromPool(string(stringObject.Value[idx]))
}

func (vm *VM) execSetIndex(target, index, value object.Object) object.Object {
	switch obj := target.(type) {
	case *object.Array:
		idx, ok := index.(*object.Integer)
		if !ok {
			return vm.errorWithLoc("array index must be integer, got %s", index.Type())
		}
		if idx.Value < 0 || int(idx.Value) >= len(obj.Elements) {
			ex := object.NewException(object.INDEX_ERROR, fmt.Sprintf("array index out of range: %d", idx.Value))
			ex.StackTrace = vm.buildStackTrace()
			return object.NewExceptionSignal(ex)
		}
		obj.Elements[int(idx.Value)] = value
		return nil
	case *object.Hash:
		hashKey, ok := index.(object.Hashable)
		if !ok {
			return vm.errorWithLoc("unusable as hash key: %s", index.Type())
		}
		obj.Pairs[hashKey.HashKey()] = object.HashPair{Key: index, Value: value}
		return nil
	case *object.Map:
		// replace existing key if equal, otherwise add
		for k := range obj.Pairs {
			if object.Equals(k, index) {
				delete(obj.Pairs, k)
				obj.Pairs[index] = value
				return nil
			}
		}
		obj.Pairs[index] = value
		return nil
	default:
		return vm.errorWithLoc("index assignment not supported: %s", target.Type())
	}
}

func (vm *VM) execLen(obj object.Object) object.Object {
	switch arg := obj.(type) {
	case *object.Array:
		return object.NewIntegerFromPool(int64(len(arg.Elements)))
	case *object.String:
		return object.NewIntegerFromPool(int64(len(arg.Value)))
	case *object.Map:
		return object.NewIntegerFromPool(int64(len(arg.Pairs)))
	case *object.Hash:
		return object.NewIntegerFromPool(int64(len(arg.Pairs)))
	default:
		return vm.errorWithLoc("argument to `len` not supported, got %s", obj.Type())
	}
}

func (vm *VM) execType(obj object.Object) object.Object {
	return object.NewStringFromPool(string(obj.Type()))
}

func readUint16(ins code.Instructions) uint16 {
	return uint16(ins[0])<<8 | uint16(ins[1])
}

func isTruthy(obj object.Object) bool {
	switch obj {
	case object.NULL, object.FALSE:
		return false
	case object.TRUE:
		return true
	}
	switch o := obj.(type) {
	case *object.Integer:
		return o.Value != 0
	case *object.Float:
		return o.Value != 0
	case *object.String:
		return o.Value != ""
	default:
		return true
	}
}

func nativeBoolToBooleanObject(b bool) *object.Boolean {
	if b {
		return object.TRUE
	}
	return object.FALSE
}

func isError(obj object.Object) bool {
	return obj != nil && obj.Type() == object.ERROR_OBJ
}

func isSignal(obj object.Object) bool {
	return obj != nil && obj.Type() == object.ObjectType(object.EXCEPTION_SIGNAL)
}

func (vm *VM) DebugStack() string {
	var out []string
	for i := 0; i < vm.sp; i++ {
		out = append(out, fmt.Sprintf("%s", vm.stack[i].Inspect()))
	}
	return strings.Join(out, ", ")
}

// errorWithLoc creates an error with file:line:col prefix when debug info is available
func (vm *VM) errorWithLoc(format string, args ...any) *object.Error {
	msg := fmt.Sprintf(format, args...)
	file, line, col, _ := vm.lookupDebug(vm.ip)
	if file != "" || line > 0 || col > 0 {
		msg = fmt.Sprintf("%s:%d:%d: %s", file, line, col, msg)
	}
	return object.NewError("%s", msg)
}

// buildStackTrace captures a simple one-frame stack trace using current IP
func (vm *VM) buildStackTrace() *object.StackTrace {
	f := vm.currentFrame()
	if f == nil {
		return &object.StackTrace{Frames: nil}
	}
	return &object.StackTrace{Frames: []*object.StackFrame{f}}
}

func (vm *VM) currentFrame() *object.StackFrame {
	file, line, col, fn := vm.lookupDebug(vm.ip)
	if file == "" && line == 0 && col == 0 {
		return nil
	}
	return &object.StackFrame{
		FunctionName: fn,
		Position: object.Position{
			Filename: file,
			Line:     line,
			Column:   col,
		},
	}
}

// lookupDebug finds the closest debug entry at or before ip
func (vm *VM) lookupDebug(ip int) (string, int, int, string) {
	// entries are expected in ascending PC order
	bestIdx := -1
	bestPC := -1
	for i := 0; i < len(vm.debug.Entries); i++ {
		pc := vm.debug.Entries[i].PC
		if pc <= ip && pc >= bestPC {
			bestPC = pc
			bestIdx = i
		}
	}
	if bestIdx == -1 {
		return "", 0, 0, ""
	}
	e := vm.debug.Entries[bestIdx]
	fn := e.Function
	if fn == "" {
		fn = "<module>"
	}
	return e.File, e.Line, e.Column, fn
}
