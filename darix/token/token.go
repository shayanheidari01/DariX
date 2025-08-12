// darix/token/token.go

package token

type TokenType string

const (
	ILLEGAL = "ILLEGAL"
	EOF     = "EOF"

	// Identifiers + literals
	IDENT  = "IDENT"  // add, foobar, x, y, ...
	INT    = "INT"    // 1343456
	FLOAT  = "FLOAT"  // 1.0
	STRING = "STRING" // "hello world"

	// Operators
	ASSIGN   = "="
	PLUS     = "+"
	MINUS    = "-"
	BANG     = "!"
	OR       = "||"
	AND      = "&&"
	ASTERISK = "*"
	SLASH    = "/"
	MOD      = "%"

	LT = "<"
	GT = ">"
	LE = "<=" // Added
	GE = ">=" // Added

	EQ     = "=="
	NOT_EQ = "!="

	// Delimiters
	MODULO    = "%"
	COMMA     = ","
	SEMICOLON = ";"
	COLON     = ":" // Added

	LPAREN = "("
	RPAREN = ")"
	LBRACE = "{"
	RBRACE = "}"

	LBRACKET = "[" // Added
	RBRACKET = "]" // Added

	// Keywords
	FUNCTION = "FUNCTION"
	VAR      = "VAR" // Changed from LET
	TRUE     = "TRUE"
	FALSE    = "FALSE"
	NULL     = "NULL"
	IF       = "IF"
	ELSE     = "ELSE"
	RETURN   = "RETURN"
	WHILE    = "WHILE"
	FOR      = "FOR"
	BREAK    = "BREAK"
	CONTINUE = "CONTINUE"
	IMPORT   = "IMPORT"
)

var keywords = map[string]TokenType{
	"func":     FUNCTION,
	"var":      VAR, // Changed from let
	"true":     TRUE,
	"false":    FALSE,
	"if":       IF,
	"else":     ELSE,
	"null":     NULL,
	"return":   RETURN,
	"while":    WHILE,
	"for":      FOR,
	"break":    BREAK,
	"continue": CONTINUE,
	"import":   IMPORT,
}

func LookupIdent(ident string) TokenType {
	if tok, ok := keywords[ident]; ok {
		return tok
	}
	return IDENT
}

type Token struct {
	Type    TokenType
	Literal string
}
