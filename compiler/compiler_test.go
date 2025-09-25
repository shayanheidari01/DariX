package compiler_test

import (
	"strings"
	"testing"

	"darix/compiler"
	"darix/lexer"
	"darix/object"
	"darix/parser"
)

func compileSource(t *testing.T, src string) *compiler.Bytecode {
	t.Helper()

	l := lexer.NewWithFile(src, "<test>")
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

func TestCompileLetConstantFolding(t *testing.T) {
	bc := compileSource(t, "var x = 1 + 2;")

	if len(bc.Constants) != 1 {
		t.Fatalf("expected 1 constant, got %d", len(bc.Constants))
	}

	intConst, ok := bc.Constants[0].(*object.Integer)
	if !ok {
		t.Fatalf("expected integer constant, got %T", bc.Constants[0])
	}

	if intConst.Value != 3 {
		t.Fatalf("expected constant value 3, got %d", intConst.Value)
	}

	expected := "0000 OpConstant 0\n0003 OpSetGlobal 0\n"
	if actual := bc.Instructions.String(); actual != expected {
		t.Fatalf("unexpected instructions:\nexpected:\n%sactual:\n%s", expected, actual)
	}
}

func TestCompileIfStatementEmitsJumps(t *testing.T) {
	src := "if (1 < 2) { var x = 1; } else { var x = 2; }"
	bc := compileSource(t, src)

	instructions := bc.Instructions.String()
	if !strings.Contains(instructions, "OpJumpNotTruthy") {
		t.Fatalf("expected instructions to contain OpJumpNotTruthy, got:\n%s", instructions)
	}
	if !strings.Contains(instructions, "OpJump") {
		t.Fatalf("expected instructions to contain OpJump, got:\n%s", instructions)
	}
}

func TestCompileArrayLiteralAndIndex(t *testing.T) {
	src := "var arr = [1, 2, 3]; arr[1];"
	bc := compileSource(t, src)

	instructions := bc.Instructions.String()
	if !strings.Contains(instructions, "OpArray 3") {
		t.Fatalf("expected OpArray instruction, got:\n%s", instructions)
	}
	if !strings.Contains(instructions, "OpIndex") {
		t.Fatalf("expected OpIndex instruction, got:\n%s", instructions)
	}
}

func TestCompileIndexAssignment(t *testing.T) {
	src := "var arr = [1, 2]; arr[0] = 42;"
	bc := compileSource(t, src)

	instructions := bc.Instructions.String()
	if !strings.Contains(instructions, "OpSetIndex") {
		t.Fatalf("expected OpSetIndex instruction, got:\n%s", instructions)
	}
}
