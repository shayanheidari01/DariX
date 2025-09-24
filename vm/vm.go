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
}

const (
	StackSize   = 2048
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
    }
    return vm
}

// A value <= 0 disables the budget (unlimited).
func (vm *VM) SetInstructionBudget(n int) {
	vm.instrBudget = n
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
		op := code.Opcode(vm.instructions[vm.ip])
		switch op {
		case code.OpNop:
			// do nothing
		case code.OpConstant:
			operand := readUint16(vm.instructions[vm.ip+1:])
			vm.ip += 2
			if err := vm.push(vm.constants[operand]); err != nil {
				return err
			}
		case code.OpAdd, code.OpSub, code.OpMul, code.OpDiv:
			right := vm.pop()
			left := vm.pop()
			res := vm.execBinary(op, left, right)
			if isSignal(res) || isError(res) {
				return res
			}
			if err := vm.push(res); err != nil {
				return err
			}
		case code.OpEqual, code.OpNotEqual, code.OpGreaterThan:
			right := vm.pop()
			left := vm.pop()
			res := vm.execCompare(op, left, right)
			if isError(res) {
				return res
			}
			if err := vm.push(res); err != nil {
				return err
			}
		case code.OpMinus:
			operand := vm.pop()
			res := vm.execMinus(operand)
			if isError(res) {
				return res
			}
			if err := vm.push(res); err != nil {
				return err
			}
		case code.OpBang:
			operand := vm.pop()
			res := nativeBoolToBooleanObject(!isTruthy(operand))
			if err := vm.push(res); err != nil {
				return err
			}
		case code.OpTrue:
			if err := vm.push(object.TRUE); err != nil { return err }
		case code.OpFalse:
			if err := vm.push(object.FALSE); err != nil { return err }
		case code.OpNull:
			if err := vm.push(object.NULL); err != nil { return err }
		case code.OpPop:
			_ = vm.pop()
		case code.OpSetGlobal:
			idx := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			val := vm.pop()
			vm.setGlobal(idx, val)
		case code.OpGetGlobal:
			idx := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			val := vm.getGlobal(idx)
			if err := vm.push(val); err != nil { return err }
		case code.OpPrint:
			argc := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			if argc < 0 || vm.sp < argc {
				return vm.errorWithLoc("print: invalid argc or stack underflow")
			}
			start := vm.sp - argc
			var b strings.Builder
			for i := start; i < vm.sp; i++ {
				if i > start { b.WriteByte(' ') }
				b.WriteString(vm.stack[i].Inspect())
			}
			out := b.String()
			// pop argc items
			for i := start; i < vm.sp; i++ { vm.stack[i] = nil }
			vm.sp = start
			// faster stdout write than fmt.Println
			_, _ = io.WriteString(os.Stdout, out)
			_, _ = io.WriteString(os.Stdout, "\n")
			if err := vm.push(object.NULL); err != nil { return err }
		case code.OpJump:
			pos := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip = pos - 1
		case code.OpJumpNotTruthy:
			pos := int(readUint16(vm.instructions[vm.ip+1:]))
			vm.ip += 2
			cond := vm.pop()
			if !isTruthy(cond) {
				vm.ip = pos - 1
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
	switch l := left.(type) {
	case *object.Integer:
		r := right.(*object.Integer)
		switch op {
		case code.OpAdd:
			return object.NewInteger(l.Value + r.Value)
		case code.OpSub:
			return object.NewInteger(l.Value - r.Value)
		case code.OpMul:
			return object.NewInteger(l.Value * r.Value)
		case code.OpDiv:
			if r.Value == 0 {
				ex := object.NewException(object.ZERO_DIV_ERROR, "division by zero")
				ex.StackTrace = vm.buildStackTrace()
				return object.NewExceptionSignal(ex)
			}
			return object.NewInteger(l.Value / r.Value)
		}
	case *object.Float:
		r := right.(*object.Float)
		switch op {
		case code.OpAdd:
			return object.NewFloat(l.Value + r.Value)
		case code.OpSub:
			return object.NewFloat(l.Value - r.Value)
		case code.OpMul:
			return object.NewFloat(l.Value * r.Value)
		case code.OpDiv:
			if r.Value == 0 {
				ex := object.NewException(object.ZERO_DIV_ERROR, "division by zero")
				ex.StackTrace = vm.buildStackTrace()
				return object.NewExceptionSignal(ex)
			}
			return object.NewFloat(l.Value / r.Value)
		}
	case *object.String:
		// only OpAdd supported for strings
		if op == code.OpAdd {
			if rs, ok := right.(*object.String); ok {
				return object.NewString(l.Value + rs.Value)
			}
		}
	}
	return vm.errorWithLoc("unsupported operands for binary op: %s and %s", left.Type(), right.Type())
}

func (vm *VM) execCompare(op code.Opcode, left, right object.Object) object.Object {
	switch l := left.(type) {
	case *object.Integer:
		r := right.(*object.Integer)
		switch op {
		case code.OpEqual:
			return nativeBoolToBooleanObject(l.Value == r.Value)
		case code.OpNotEqual:
			return nativeBoolToBooleanObject(l.Value != r.Value)
		case code.OpGreaterThan:
			return nativeBoolToBooleanObject(l.Value > r.Value)
		}
	case *object.Float:
		r := right.(*object.Float)
		switch op {
		case code.OpEqual:
			return nativeBoolToBooleanObject(l.Value == r.Value)
		case code.OpNotEqual:
			return nativeBoolToBooleanObject(l.Value != r.Value)
		case code.OpGreaterThan:
			return nativeBoolToBooleanObject(l.Value > r.Value)
		}
	case *object.String:
		r := right.(*object.String)
		switch op {
		case code.OpEqual:
			return nativeBoolToBooleanObject(l.Value == r.Value)
		case code.OpNotEqual:
			return nativeBoolToBooleanObject(l.Value != r.Value)
		case code.OpGreaterThan:
			return nativeBoolToBooleanObject(l.Value > r.Value)
		}
	case *object.Boolean:
		r := right.(*object.Boolean)
		switch op {
		case code.OpEqual:
			return nativeBoolToBooleanObject(l.Value == r.Value)
		case code.OpNotEqual:
			return nativeBoolToBooleanObject(l.Value != r.Value)
		}
	}
	return vm.errorWithLoc("unsupported operands for compare: %s and %s", left.Type(), right.Type())
}

func (vm *VM) execMinus(operand object.Object) object.Object {
	switch o := operand.(type) {
	case *object.Integer:
		return object.NewInteger(-o.Value)
	case *object.Float:
		return object.NewFloat(-o.Value)
	default:
		return vm.errorWithLoc("unsupported operand for prefix -: %s", operand.Type())
	}
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
	return &object.StackFrame{Function: fn, File: file, Line: line, Column: col}
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
