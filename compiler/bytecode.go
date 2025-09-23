package compiler

import (
	"darix/code"
	"darix/object"
)

type Bytecode struct {
	Instructions code.Instructions
	Constants    []object.Object
}
