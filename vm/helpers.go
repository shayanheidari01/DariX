package vm

import (
	"darix/object"
	"io"
	"os"
	"strings"
)

// Shared opcode helpers to avoid duplication between Run and runCompiledFunction
func (vm *VM) opPrint(argc int) object.Object {
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
	if err := vm.push(object.NULL); err != nil {
		return err
	}
	return nil
}

func (vm *VM) opArray(numElements int) object.Object {
	elements := make([]object.Object, numElements)
	for i := numElements - 1; i >= 0; i-- {
		elem, err := vm.popChecked()
		if err != nil {
			return err
		}
		elements[i] = elem
	}
	if err := vm.push(object.NewArrayFromPool(elements)); err != nil {
		return err
	}
	return nil
}

func (vm *VM) opStringConcat(n int) object.Object {
	if n < 0 || vm.sp < n {
		return vm.errorWithLoc("string concat: invalid count or stack underflow")
	}
	start := vm.sp - n
	// Validate all are strings and precompute size
	var total int
	for i := start; i < vm.sp; i++ {
		s, ok := vm.stack[i].(*object.String)
		if !ok {
			return vm.errorWithLoc("string concat: operand %d is not string (%s)", i-start, vm.stack[i].Type())
		}
		total += len(s.Value)
	}
	var b strings.Builder
	b.Grow(total)
	for i := start; i < vm.sp; i++ {
		b.WriteString(vm.stack[i].(*object.String).Value)
		vm.stack[i] = nil
	}
	vm.sp = start
	if err := vm.push(object.NewStringFromPool(b.String())); err != nil {
		return err
	}
	return nil
}

func (vm *VM) opSwap() object.Object {
	if vm.sp < 2 {
		return vm.errorWithLoc("swap: stack underflow")
	}
	a := vm.stack[vm.sp-1]
	b := vm.stack[vm.sp-2]
	vm.stack[vm.sp-1] = b
	vm.stack[vm.sp-2] = a
	return nil
}
