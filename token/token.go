// darix/token/token.go

package token

type TokenType string

const (
	ILLEGAL = "ILLEGAL"
	EOF     = "EOF"

	// Identifiers + literals
	IDENT  = "IDENT"
	INT    = "INT"
	FLOAT  = "FLOAT"
	STRING = "STRING"

	// Operators
	ASSIGN   = "="
	PLUS     = "+"
	MINUS    = "-"
	BANG     = "!"
	OR       = "||"
	AND      = "&&"
	ASTERISK = "*"
	SLASH    = "/"
	MODULO   = "%"
	LT       = "<"
	GT       = ">"
	LE       = "<="
	GE       = ">="
	EQ       = "=="
	NOT_EQ   = "!="

	// Delimiters / punctuation
	COMMA     = ","
	SEMICOLON = ";"
	COLON     = ":"
	DOT       = "."
	AT        = "@"
	LPAREN    = "("
	RPAREN    = ")"
	LBRACE    = "{"
	RBRACE    = "}"
	LBRACKET  = "["
	RBRACKET  = "]"

	// Keywords
	FUNCTION = "FUNCTION"
	CLASS    = "CLASS"
	VAR      = "VAR"
	TRUE     = "TRUE"
	FALSE    = "FALSE"
	NULL     = "NULL"
	IF       = "IF"
	ELSE     = "ELSE"
	ELIF     = "ELIF"
	RETURN   = "RETURN"
	WHILE    = "WHILE"
	FOR      = "FOR"
	BREAK    = "BREAK"
	CONTINUE = "CONTINUE"
	IMPORT   = "IMPORT"
	FROM     = "FROM"
	AS       = "AS"
	TRY      = "TRY"
	CATCH    = "CATCH"
	FINALLY  = "FINALLY"
	THROW    = "THROW"
	RAISE    = "RAISE"
	
	// New keywords
	DEL      = "DEL"
	ASSERT   = "ASSERT"
	PASS     = "PASS"
	AND_KW   = "AND_KW"   // 'and' keyword (different from && operator)
	OR_KW    = "OR_KW"    // 'or' keyword (different from || operator)
	NOT_KW   = "NOT_KW"   // 'not' keyword (different from ! operator)
	IN       = "IN"
	IS       = "IS"
	WITH     = "WITH"
	YIELD    = "YIELD"
	GLOBAL   = "GLOBAL"
	NONLOCAL = "NONLOCAL"
	LAMBDA   = "LAMBDA"
)

type keywordEntry struct {
	literal string
	typeID  TokenType
}

var keywordEntries = [...]keywordEntry{
	{"func", FUNCTION},
	{"class", CLASS},
	{"var", VAR},
	{"true", TRUE},
	{"false", FALSE},
	{"if", IF},
	{"else", ELSE},
	{"elif", ELIF},
	{"null", NULL},
	{"return", RETURN},
	{"while", WHILE},
	{"for", FOR},
	{"break", BREAK},
	{"continue", CONTINUE},
	{"import", IMPORT},
	{"from", FROM},
	{"as", AS},
	{"try", TRY},
	{"catch", CATCH},
	{"finally", FINALLY},
	{"throw", THROW},
	{"raise", RAISE},
	
	// New keywords
	{"del", DEL},
	{"assert", ASSERT},
	{"pass", PASS},
	{"and", AND_KW},
	{"or", OR_KW},
	{"not", NOT_KW},
	{"in", IN},
	{"is", IS},
	{"with", WITH},
	{"yield", YIELD},
	{"global", GLOBAL},
	{"nonlocal", NONLOCAL},
	{"lambda", LAMBDA},
}

var keywords = make(map[string]TokenType, len(keywordEntries))

func init() {
	for _, entry := range keywordEntries {
		keywords[entry.literal] = entry.typeID
	}
}

func RegisterKeyword(literal string, tok TokenType) {
	keywords[literal] = tok
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
	File    string
	Line    int
	Column  int
	Offset  int
}
