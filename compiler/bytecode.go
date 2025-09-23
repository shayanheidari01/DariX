package compiler

import (
	"darix/code"
	"darix/object"
)

// BytecodeMagic is a short identifier to verify the origin/format of bytecode
const BytecodeMagic = "DRXB1"

type Bytecode struct {
	Magic        string
	Version      string
	Instructions code.Instructions
	Constants    []object.Object
	Debug        DebugInfo
}

// DebugEntry maps a bytecode program counter to a source position
type DebugEntry struct {
	PC       int
	File     string
	Line     int
	Column   int
	Function string
}

// DebugInfo contains the full mapping for a compiled unit
type DebugInfo struct {
	Entries []DebugEntry
}
