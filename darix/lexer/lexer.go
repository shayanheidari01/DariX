// darix/lexer.go

package lexer

import (
	"darix/token"
	"strings"
)

type Lexer struct {
	input        string
	position     int  // current position in input (points to current char)
	readPosition int  // current reading position in input (after current char)
	ch           byte // current char under examination
}

func New(input string) *Lexer {
	l := &Lexer{input: input}
	l.readChar()
	return l
}

func (l *Lexer) readChar() {
	if l.readPosition >= len(l.input) {
		l.ch = 0
	} else {
		l.ch = l.input[l.readPosition]
	}
	l.position = l.readPosition
	l.readPosition++
}

func (l *Lexer) peekChar() byte {
	if l.readPosition >= len(l.input) {
		return 0 // یا هر مقدار دیگری که نشان‌دهنده EOF است
	}
	return l.input[l.readPosition]
}

func (l *Lexer) peekTwoChars() byte {
	// بررسی اینکه آیا readPosition+1 از طول رشته کمتر است
	if l.readPosition+1 >= len(l.input) {
		return 0 // یا هر مقدار دیگری که نشان‌دهنده EOF است
	}
	return l.input[l.readPosition+1]
}

// پرش separator خطی '--- ... ---'
func (l *Lexer) skipSeparator() {
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
}

func (l *Lexer) NextToken() token.Token {
	var tok token.Token

	// حذف کامنت‌ها و جداکننده‌ها قبل از هر توکن‌سازی
	for {
		if l.ch == '/' && l.peekChar() == '/' && l.peekTwoChars() == '-' {
			l.skipSeparator()
			l.skipWhitespace()
			continue
		}
		if l.ch == '/' && l.peekChar() == '/' {
			l.skipUntilNewline()
			l.skipWhitespace()
			continue
		}
		if l.ch == '/' && l.peekChar() == '*' {
			l.readChar()
			l.readChar()
			l.skipBlockComment()
			l.skipWhitespace()
			continue
		}
		break
	}

	l.skipWhitespace()

	switch l.ch {
	case '=':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = token.Token{Type: token.EQ, Literal: string(ch) + string(l.ch)}
		} else {
			tok = newToken(token.ASSIGN, l.ch)
		}
	case '+':
		tok = newToken(token.PLUS, l.ch)
	case '-':
		tok = newToken(token.MINUS, l.ch)
	case '!':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = token.Token{Type: token.NOT_EQ, Literal: string(ch) + string(l.ch)}
		} else {
			tok = newToken(token.BANG, l.ch)
		}
	case '/':
		tok = newToken(token.SLASH, l.ch)
	case '*':
		tok = newToken(token.ASTERISK, l.ch)
	case '%':
		tok = newToken(token.MODULO, l.ch)
	case '<':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = token.Token{Type: token.LE, Literal: string(ch) + string(l.ch)}
		} else {
			tok = newToken(token.LT, l.ch)
		}
	case '>':
		if l.peekChar() == '=' {
			ch := l.ch
			l.readChar()
			tok = token.Token{Type: token.GE, Literal: string(ch) + string(l.ch)}
		} else {
			tok = newToken(token.GT, l.ch)
		}
	case '&':
		if l.peekChar() == '&' {
			ch := l.ch
			l.readChar()
			tok = token.Token{Type: token.AND, Literal: string(ch) + string(l.ch)}
		} else {
			tok = newToken(token.ILLEGAL, l.ch)
		}
	case '|':
		if l.peekChar() == '|' {
			ch := l.ch
			l.readChar()
			tok = token.Token{Type: token.OR, Literal: string(ch) + string(l.ch)}
		} else {
			tok = newToken(token.ILLEGAL, l.ch)
		}
	case ',':
		tok = newToken(token.COMMA, l.ch)
	case ';':
		tok = newToken(token.SEMICOLON, l.ch)
	case ':':
		tok = newToken(token.COLON, l.ch)
	case '(':
		tok = newToken(token.LPAREN, l.ch)
	case ')':
		tok = newToken(token.RPAREN, l.ch)
	case '{':
		tok = newToken(token.LBRACE, l.ch)
	case '}':
		tok = newToken(token.RBRACE, l.ch)
	case '[':
		tok = newToken(token.LBRACKET, l.ch)
	case ']':
		tok = newToken(token.RBRACKET, l.ch)
	case '"':
		tok.Type = token.STRING
		tok.Literal = l.readString()
		return tok
	case 0:
		tok.Literal = ""
		tok.Type = token.EOF
	default:
		if isLetter(l.ch) {
			tok.Literal = l.readIdentifier()
			tok.Type = token.LookupIdent(tok.Literal)
			return tok
		} else if isDigit(l.ch) {
			number := l.readNumber()
			if strings.Contains(number, ".") {
				tok.Type = token.FLOAT
			} else {
				tok.Type = token.INT
			}
			tok.Literal = number
			return tok
		} else {
			tok = newToken(token.ILLEGAL, l.ch)
		}
	}

	l.readChar()
	return tok
}

// skipBlockComment skips a '*/' terminated comment
func (l *Lexer) skipBlockComment() {
	for {
		if l.ch == 0 {
			break
		}
		if l.ch == '*' && l.peekChar() == '/' {
			l.readChar()
			l.readChar()
			break
		}
		l.readChar()
	}
}

func newToken(tokenType token.TokenType, ch byte) token.Token {
	return token.Token{Type: tokenType, Literal: string(ch)}
}

// Helper: خواندن یک عدد صحیح یا اعشاری (یک نقطه فقط)
func (l *Lexer) readNumber() string {
	pos := l.position
	// خواندن ارقام اولیه
	for isDigit(l.ch) {
		l.readChar()
	}
	// اگر نقطه دیدیم و بعد از آن هم رقم داریم، ادامه بده (برای اعداد اعشاری)
	// استفاده از peekChar به‌روز شده
	if l.ch == '.' && isDigit(l.peekChar()) {
		l.readChar() // خواندن '.'
		for isDigit(l.ch) {
			l.readChar() // خواندن ارقام اعشاری
		}
	}
	return l.input[pos:l.position]
}

func (l *Lexer) skipWhitespace() {
	for l.ch == ' ' || l.ch == '\t' || l.ch == '\n' || l.ch == '\r' {
		l.readChar()
	}
}

// skipUntilNewline skips until newline or EOF
func (l *Lexer) skipUntilNewline() {
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
}

func (l *Lexer) readString() string {
	// موقعیت اولین " را ذخیره کن (برای رفرنس)
	// startPosition := l.position // ممکن است برای دیباگ لازم باشد

	// برو به کاراکتر بعدی (اولین کاراکتر از محتوای رشته)
	// pos موقعیت اولین کاراکتر از محتوای رشته است
	pos := l.position + 1
	l.readChar()

	// خواندن کاراکترهای محتوای رشته تا به " برسیم یا EOF
	for l.ch != '"' && l.ch != 0 {
		l.readChar()
	}
	// در این نقطه l.ch یا '"' است یا 0 (EOF)
	// مقدار رشته بین " اول و " آخر (یا EOF) است
	value := l.input[pos:l.position]

	// اگر " پیدا کردیم، برو به کاراکتر بعد از "
	// این مهم است تا توکن بعدی (در اینجا ')') را در NextToken ببینیم.
	// اگر EOF پیدا کردیم، l.readChar() کاری نمی‌کند چون l.ch == 0 است.
	if l.ch == '"' {
		l.readChar()
	}
	// در غیر این صورت، l.ch == 0 است (EOF) و l.readChar() آخرین بار 0 را در l.ch نگه داشته.
	// موقعیت‌ها به درستی به‌روز شده‌اند.

	return value
}

func (l *Lexer) readIdentifier() string {
	pos := l.position
	for isLetter(l.ch) {
		l.readChar()
	}
	return l.input[pos:l.position]
}

func isLetter(ch byte) bool {
	return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_'
}

func isDigit(ch byte) bool {
	return '0' <= ch && ch <= '9'
}
