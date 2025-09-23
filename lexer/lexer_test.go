package lexer

import (
	"darix/token"
	"testing"
)

func TestTokenPositions(t *testing.T) {
	input := "var x = 42\nx = x + 1\n\"hi\"\n&&\n"
	l := NewWithFile(input, "test.drx")

	tests := []struct {
		typ   token.TokenType
		lit   string
		line  int
		col   int
	}{
		{token.VAR, "var", 1, 1},
		{token.IDENT, "x", 1, 5},
		{token.ASSIGN, "=", 1, 7},
		{token.INT, "42", 1, 9},
		{token.IDENT, "x", 2, 1},
		{token.ASSIGN, "=", 2, 3},
		{token.IDENT, "x", 2, 5},
		{token.PLUS, "+", 2, 7},
		{token.INT, "1", 2, 9},
		{token.STRING, "hi", 3, 1},
		{token.AND, "&&", 4, 1},
	}

	for i, tt := range tests {
		tok := l.NextToken()
		if tok.Type != tt.typ || tok.Literal != tt.lit || tok.Line != tt.line || tok.Column != tt.col || tok.File != "test.drx" {
			t.Fatalf("test %d: expected %v %q at %s:%d:%d, got %v %q at %s:%d:%d", i, tt.typ, tt.lit, "test.drx", tt.line, tt.col, tok.Type, tok.Literal, tok.File, tok.Line, tok.Column)
		}
	}
}
