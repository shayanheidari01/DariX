package vm

import (
	"testing"

	"darix/compiler"
	"darix/lexer"
	"darix/object"
	"darix/parser"
)

func compileToBytecode(t *testing.T, src string) *compiler.Bytecode {
	t.Helper()

	l := lexer.NewWithFile(src, "<vm-test>")
	p := parser.New(l)
	program := p.ParseProgram()

	if errs := p.Errors(); len(errs) > 0 {
		t.Fatalf("parser errors: %v", errs)
	}

	comp := compiler.New()
	if err := comp.Compile(program); err != nil {
		t.Fatalf("compile failed: %v", err)
	}

	return comp.Bytecode()
}

func TestVMExecutesSimpleLetStatement(t *testing.T) {
	bc := compileToBytecode(t, "var result = 1 + 2;")

	machine := New(bc)
	out := machine.Run()
	if out != object.NULL {
		t.Fatalf("expected NULL result, got %s", out.Inspect())
	}

	if len(machine.globals) == 0 {
		t.Fatalf("expected at least one global to be set")
	}

	value := machine.globals[0]
	intValue, ok := value.(*object.Integer)
	if !ok {
		t.Fatalf("expected first global to be *object.Integer, got %T", value)
	}
	if intValue.Value != 3 {
		t.Fatalf("expected integer value 3, got %d", intValue.Value)
	}
}
