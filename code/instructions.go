// package code provides bytecode instruction definitions and helpers
package code

import (
	"encoding/binary"
	"fmt"
	"strings"
)

type Instructions []byte

type Opcode byte

type Definition struct {
	Name          string
	OperandWidths []int
}

const (
	OpConstant Opcode = iota
	OpAdd
	OpSub
	OpMul
	OpDiv
	OpEqual
	OpNotEqual
	OpGreaterThan
	OpMinus
	OpBang
	OpTrue
	OpFalse
	OpNull
	OpPop
	OpJump
	OpJumpNotTruthy
	OpSetGlobal
	OpGetGlobal
	OpPrint
	OpNop
)

var definitions = map[Opcode]*Definition{
	OpConstant:       {Name: "OpConstant", OperandWidths: []int{2}},
	OpAdd:            {Name: "OpAdd", OperandWidths: []int{}},
	OpSub:            {Name: "OpSub", OperandWidths: []int{}},
	OpMul:            {Name: "OpMul", OperandWidths: []int{}},
	OpDiv:            {Name: "OpDiv", OperandWidths: []int{}},
	OpEqual:          {Name: "OpEqual", OperandWidths: []int{}},
	OpNotEqual:       {Name: "OpNotEqual", OperandWidths: []int{}},
	OpGreaterThan:    {Name: "OpGreaterThan", OperandWidths: []int{}},
	OpMinus:          {Name: "OpMinus", OperandWidths: []int{}},
	OpBang:           {Name: "OpBang", OperandWidths: []int{}},
	OpTrue:           {Name: "OpTrue", OperandWidths: []int{}},
	OpFalse:          {Name: "OpFalse", OperandWidths: []int{}},
	OpNull:           {Name: "OpNull", OperandWidths: []int{}},
	OpPop:            {Name: "OpPop", OperandWidths: []int{}},
	OpJump:           {Name: "OpJump", OperandWidths: []int{2}},
	OpJumpNotTruthy:  {Name: "OpJumpNotTruthy", OperandWidths: []int{2}},
	OpSetGlobal:      {Name: "OpSetGlobal", OperandWidths: []int{2}},
	OpGetGlobal:      {Name: "OpGetGlobal", OperandWidths: []int{2}},
	OpPrint:          {Name: "OpPrint", OperandWidths: []int{2}},
	OpNop:            {Name: "OpNop", OperandWidths: []int{}},
}

func Lookup(op Opcode) (*Definition, bool) {
	def, ok := definitions[op]
	return def, ok
}

func Make(op Opcode, operands ...int) Instructions {
	def, ok := definitions[op]
	if !ok {
		return []byte{}
	}

	instrLen := 1
	for _, w := range def.OperandWidths {
		instrLen += w
	}

	ins := make([]byte, instrLen)
	ins[0] = byte(op)

	offset := 1
	for i, w := range def.OperandWidths {
		operand := operands[i]
		switch w {
		case 2:
			binary.BigEndian.PutUint16(ins[offset:], uint16(operand))
		default:
			panic("unsupported operand width")
		}
		offset += w
	}
	return ins
}

func ReadOperands(def *Definition, ins Instructions) ([]int, int) {
	operands := make([]int, len(def.OperandWidths))
	offset := 0
	for i, w := range def.OperandWidths {
		switch w {
		case 2:
			operands[i] = int(binary.BigEndian.Uint16(ins[offset:]))
		default:
			panic("unsupported operand width")
		}
		offset += w
	}
	return operands, offset
}

func (ins Instructions) String() string {
	var out strings.Builder
	i := 0
	for i < len(ins) {
		op := Opcode(ins[i])
		def, ok := Lookup(op)
		if !ok {
			fmt.Fprintf(&out, "ERROR: unknown opcode %d\n", op)
			i++
			continue
		}
		operands, read := ReadOperands(def, ins[i+1:])
		fmt.Fprintf(&out, "%04d %s\n", i, ins.fmtInstruction(def, operands))
		i += 1 + read
	}
	return out.String()
}

func (ins Instructions) fmtInstruction(def *Definition, operands []int) string {
	opStr := def.Name
	if len(def.OperandWidths) != len(operands) {
		return fmt.Sprintf("ERROR: operand len %d does not match defined %d", len(operands), len(def.OperandWidths))
	}
	switch len(operands) {
	case 0:
		return opStr
	case 1:
		return fmt.Sprintf("%s %d", opStr, operands[0])
	}
	return fmt.Sprintf("%s %v", opStr, operands)
}
