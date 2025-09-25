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
	RETURN   = "RETURN"
	WHILE    = "WHILE"
	FOR      = "FOR"
	BREAK    = "BREAK"
	CONTINUE = "CONTINUE"
	IMPORT   = "IMPORT"
	TRY      = "TRY"
	CATCH    = "CATCH"
	FINALLY  = "FINALLY"
	THROW    = "THROW"
	RAISE    = "RAISE"
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
	{"null", NULL},
	{"return", RETURN},
	{"while", WHILE},
	{"for", FOR},
	{"break", BREAK},
	{"continue", CONTINUE},
	{"import", IMPORT},
	{"try", TRY},
	{"catch", CATCH},
	{"finally", FINALLY},
	{"throw", THROW},
	{"raise", RAISE},
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
