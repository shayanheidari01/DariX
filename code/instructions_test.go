package code

import "testing"

func TestOpPrintDefinition(t *testing.T) {
	def, ok := Lookup(OpPrint)
	if !ok {
		t.Fatalf("OpPrint not found in definitions")
	}
	if def.Name != "OpPrint" {
		t.Fatalf("unexpected name: %s", def.Name)
	}
	if len(def.OperandWidths) != 1 || def.OperandWidths[0] != 2 {
		t.Fatalf("unexpected operand widths: %+v", def.OperandWidths)
	}
}

func TestInstructionsString(t *testing.T) {
	ins := Instructions{}
	ins = append(ins, Make(OpConstant, 0)...)
	ins = append(ins, Make(OpConstant, 1)...)
	ins = append(ins, Make(OpAdd)...)
	ins = append(ins, Make(OpPrint, 1)...)
	// Ensure the disassembler doesn't panic and contains op names
	d := ins.String()
	for _, want := range []string{"OpConstant", "OpAdd", "OpPrint"} {
		if !contains(d, want) {
			t.Fatalf("disassembly missing %s: %s", want, d)
		}
	}
}

func contains(s, sub string) bool {
	return len(s) >= len(sub) && (s == sub || (len(s) > 0 && (s[0:len(sub)] == sub || contains(s[1:], sub))))
}
