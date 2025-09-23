// darix/lexer.go

package lexer

import (
	"darix/token"
	"strings"
)

type Lexer struct {
	input        string
	position     int  // current position
	readPosition int  // next reading position
	ch           byte // current char
	line         int  // current line (1-based)
	column       int  // current column (1-based)
	file         string // source filename (optional)
}

func New(input string) *Lexer {
	return NewWithFile(input, "")
}

func NewWithFile(input, file string) *Lexer {
	l := &Lexer{input: input, line: 1, column: 0, file: file}
	l.readChar()
	return l
}

func (l *Lexer) readChar() {
	if l.readPosition >= len(l.input) {
		l.ch = 0
		l.position = l.readPosition
		l.readPosition++
		return
	}
	l.ch = l.input[l.readPosition]
	l.position = l.readPosition
	l.readPosition++
	if l.ch == '\n' {
		l.line++
		l.column = 0
	} else if l.ch != 0 {
		l.column++
	}
}

func (l *Lexer) peekChar() byte {
	if l.readPosition >= len(l.input) {
		return 0
	}
	return l.input[l.readPosition]
}

func (l *Lexer) peekCharAt(offset int) byte {
	pos := l.readPosition + offset - 1
	if pos >= len(l.input) {
		return 0
	}
	return l.input[pos]
}

func (l *Lexer) NextToken() token.Token {
	var tok token.Token

	// Skip comments and whitespace
	l.skipCommentsAndWhitespace()

	switch l.ch {
	case '=':
		tok = l.makeTwoCharToken('=', token.EQ, token.ASSIGN)
	case '+':
		tok = l.newToken(token.PLUS)
	case '-':
		tok = l.newToken(token.MINUS)
	case '!':
		tok = l.makeTwoCharToken('=', token.NOT_EQ, token.BANG)
	case '/':
		tok = l.newToken(token.SLASH)
	case '*':
		tok = l.newToken(token.ASTERISK)
	case '%':
		tok = l.newToken(token.MODULO)
	case '<':
		tok = l.makeTwoCharToken('=', token.LE, token.LT)
	case '>':
		tok = l.makeTwoCharToken('=', token.GE, token.GT)
	case '&':
		if l.peekChar() == '&' {
			l.readChar()
			tok = token.Token{Type: token.AND, Literal: "&&", File: l.file, Line: l.line, Column: l.column - 1, Offset: l.position - 1}
		} else {
			tok = l.newToken(token.ILLEGAL)
		}
	case '|':
		if l.peekChar() == '|' {
			l.readChar()
			tok = token.Token{Type: token.OR, Literal: "||", File: l.file, Line: l.line, Column: l.column - 1, Offset: l.position - 1}
		} else {
			tok = l.newToken(token.ILLEGAL)
		}
	case ',':
		tok = l.newToken(token.COMMA)
	case ';':
		tok = l.newToken(token.SEMICOLON)
	case ':':
		tok = l.newToken(token.COLON)
	case '(':
		tok = l.newToken(token.LPAREN)
	case ')':
		tok = l.newToken(token.RPAREN)
	case '{':
		tok = l.newToken(token.LBRACE)
	case '}':
		tok = l.newToken(token.RBRACE)
	case '[':
		tok = l.newToken(token.LBRACKET)
	case ']':
		tok = l.newToken(token.RBRACKET)
	case '"':
		startLine, startColumn, startOffset := l.line, l.column, l.position
		tok.Type = token.STRING
		tok.Literal = l.readString()
		tok.File, tok.Line, tok.Column, tok.Offset = l.file, startLine, startColumn, startOffset
		return tok
	case 0:
		tok = token.Token{Type: token.EOF, Literal: "", File: l.file, Line: l.line, Column: l.column, Offset: l.position}
	default:
		if isLetter(l.ch) {
			startLine, startColumn, startOffset := l.line, l.column, l.position
			tok.Literal = l.readIdentifier()
			tok.Type = token.LookupIdent(tok.Literal)
			tok.File, tok.Line, tok.Column, tok.Offset = l.file, startLine, startColumn, startOffset
			return tok
		} else if isDigit(l.ch) {
			startLine, startColumn, startOffset := l.line, l.column, l.position
			number := l.readNumber()
			if strings.Contains(number, ".") {
				tok.Type = token.FLOAT
			} else {
				tok.Type = token.INT
			}
			tok.Literal = number
			tok.File, tok.Line, tok.Column, tok.Offset = l.file, startLine, startColumn, startOffset
			return tok
		} else {
			tok = l.newToken(token.ILLEGAL)
		}
	}

	l.readChar()
	return tok
}

// Optimized comment and whitespace skipping
func (l *Lexer) skipCommentsAndWhitespace() {
	for {
		l.skipWhitespace()

		if l.ch == '/' {
			next := l.peekChar()
			if next == '/' {
				// Check for separator comment
				if l.peekCharAt(2) == '-' {
					l.skipSeparator()
				} else {
					l.skipLineComment()
				}
				continue
			} else if next == '*' {
				l.skipBlockComment()
				continue
			}
		}
		break
	}
}

func (l *Lexer) skipWhitespace() {
	for l.ch == ' ' || l.ch == '\t' || l.ch == '\n' || l.ch == '\r' {
		l.readChar()
	}
}

func (l *Lexer) skipLineComment() {
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
}

func (l *Lexer) skipSeparator() {
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
}

func (l *Lexer) skipBlockComment() {
	l.readChar() // skip '/'
	l.readChar() // skip '*'

	for {
		if l.ch == 0 {
			break
		}
		if l.ch == '*' && l.peekChar() == '/' {
			l.readChar() // skip '*'
			l.readChar() // skip '/'
			break
		}
		l.readChar()
	}
}

// Optimized token creation helpers
func (l *Lexer) newToken(tokenType token.TokenType) token.Token {
	return token.Token{Type: tokenType, Literal: string(l.ch), File: l.file, Line: l.line, Column: l.column, Offset: l.position}
}

func (l *Lexer) makeTwoCharToken(secondChar byte, twoCharType, oneCharType token.TokenType) token.Token {
	if l.peekChar() == secondChar {
		ch := l.ch
		startLine, startColumn, startOffset := l.line, l.column, l.position
		l.readChar()
		return token.Token{Type: twoCharType, Literal: string(ch) + string(l.ch), File: l.file, Line: startLine, Column: startColumn, Offset: startOffset}
	}
	return l.newToken(oneCharType)
}

// Optimized number reading with single pass
func (l *Lexer) readNumber() string {
	pos := l.position

	// Read digits
	for isDigit(l.ch) {
		l.readChar()
	}

	// Check for decimal point
	if l.ch == '.' && isDigit(l.peekChar()) {
		l.readChar() // consume '.'
		for isDigit(l.ch) {
			l.readChar()
		}
	}

	return l.input[pos:l.position]
}

// Optimized string reading with escape sequence support
func (l *Lexer) readString() string {
	var result strings.Builder
	l.readChar() // skip opening quote

	for l.ch != '"' && l.ch != 0 {
		if l.ch == '\\' {
			l.readChar() // consume backslash
			// Check if we've reached end of input after backslash
			if l.ch == 0 {
				// Unterminated string with trailing backslash
				result.WriteByte('\\')
				break
			}
			switch l.ch {
			case 'n':
				result.WriteByte('\n')
			case 't':
				result.WriteByte('\t')
			case 'r':
				result.WriteByte('\r')
			case '\\':
				result.WriteByte('\\')
			case '"':
				result.WriteByte('"')
			default:
				// For unknown escape sequences, include the backslash
				result.WriteByte('\\')
				result.WriteByte(l.ch)
			}
			// Advance after processing escape sequence
			l.readChar()
		} else {
			result.WriteByte(l.ch)
			// Advance for normal characters
			l.readChar()
		}
	}

	// Move past closing quote if found
	if l.ch == '"' {
		l.readChar()
	}

	return result.String()
}

func (l *Lexer) readIdentifier() string {
	pos := l.position
	for isLetter(l.ch) || isDigit(l.ch) { // Allow digits in identifiers after first char
		l.readChar()
	}
	return l.input[pos:l.position]
}

// Optimized character type checking
func isLetter(ch byte) bool {
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_'
}

func isDigit(ch byte) bool {
	return '0' <= ch && ch <= '9'
}
