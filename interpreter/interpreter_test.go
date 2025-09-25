package interpreter

import (
	"testing"

	"darix/lexer"
	"darix/object"
	"darix/parser"
)

func interpretSource(t *testing.T, src string) object.Object {
	t.Helper()

	l := lexer.NewWithFile(src, "<interp-test>")
	p := parser.New(l)
	program := p.ParseProgram()
	if errs := p.Errors(); len(errs) > 0 {
		t.Fatalf("parser errors: %v", errs)
	}

	inter := New()
	return inter.Interpret(program)
}

func TestInterpreterEvaluatesExpressions(t *testing.T) {
	result := interpretSource(t, "var x = 41; x + 1;")

	integer, ok := result.(*object.Integer)
	if !ok {
		t.Fatalf("expected *Integer result, got %T", result)
	}
	if integer.Value != 42 {
		t.Fatalf("expected value 42, got %d", integer.Value)
	}
}

func TestInterpreterHandlesExceptions(t *testing.T) {
	result := interpretSource(t, "try { throw ValueError(\"bad\"); } catch (ValueError e) { \"handled\"; }")

	str, ok := result.(*object.String)
	if !ok {
		t.Fatalf("expected *String result, got %T", result)
	}
	if str.Value != "handled" {
		t.Fatalf("expected 'handled', got %q", str.Value)
	}
}
