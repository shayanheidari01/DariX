// darix/lexer.go

package lexer

import (
	"darix/token"
	"strings"
)

type Lexer struct {
	input        string
	position     int    // current position
	readPosition int    // next reading position
	ch           byte   // current char
	line         int    // current line (1-based)
	column       int    // current column (1-based)
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

	startLine, startColumn, startOffset := l.line, l.column, l.position

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
			tok = l.tokenWithLiteral(token.AND, "&&", startLine, startColumn, startOffset)
		} else {
			tok = l.tokenWithLiteral(token.ILLEGAL, string(l.ch), startLine, startColumn, startOffset)
		}
	case '|':
		if l.peekChar() == '|' {
			l.readChar()
			tok = l.tokenWithLiteral(token.OR, "||", startLine, startColumn, startOffset)
		} else {
			tok = l.tokenWithLiteral(token.ILLEGAL, string(l.ch), startLine, startColumn, startOffset)
		}
	case ',':
		tok = l.newToken(token.COMMA)
	case ';':
		tok = l.newToken(token.SEMICOLON)
	case ':':
		tok = l.newToken(token.COLON)
	case '.':
		tok = l.newToken(token.DOT)
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
		tok = l.tokenWithLiteral(token.STRING, l.readString(), startLine, startColumn, startOffset)
		return tok
	case 0:
		tok = l.tokenWithLiteral(token.EOF, "", startLine, startColumn, startOffset)
	default:
		if isLetter(l.ch) {
			literal := l.readIdentifier()
			return l.tokenWithLiteral(token.LookupIdent(literal), literal, startLine, startColumn, startOffset)
		} else if isDigit(l.ch) {
			number := l.readNumber()
			tokType := token.TokenType(token.INT)
			if strings.Contains(number, ".") {
				tokType = token.FLOAT
			}
			return l.tokenWithLiteral(tokType, number, startLine, startColumn, startOffset)
		} else {
			tok = l.tokenWithLiteral(token.ILLEGAL, string(l.ch), startLine, startColumn, startOffset)
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
	l.skipUntilNewline()
}

func (l *Lexer) skipSeparator() {
	l.skipUntilNewline()
}

func (l *Lexer) skipBlockComment() {
	l.readChar() // skip '/'
	l.readChar() // skip '*'
	l.skipUntilClosingBlock()
}

func (l *Lexer) skipUntilNewline() {
	for l.ch != '\n' && l.ch != 0 {
		l.readChar()
	}
}

func (l *Lexer) skipUntilClosingBlock() {
	for {
		if l.ch == 0 {
			return
		}
		if l.ch == '*' && l.peekChar() == '/' {
			l.readChar() // skip '*'
			l.readChar() // skip '/'
			return
		}
		l.readChar()
	}
}

func (l *Lexer) newToken(tokenType token.TokenType) token.Token {
	return l.tokenWithLiteral(tokenType, string(l.ch), l.line, l.column, l.position)
}

func (l *Lexer) makeTwoCharToken(secondChar byte, twoCharType, oneCharType token.TokenType) token.Token {
	if l.peekChar() == secondChar {
		first := l.ch
		startLine, startColumn, startOffset := l.line, l.column, l.position
		l.readChar()
		return l.tokenWithLiteral(twoCharType, string([]byte{first, l.ch}), startLine, startColumn, startOffset)
	}
	return l.tokenWithLiteral(oneCharType, string(l.ch), l.line, l.column, l.position)
}

func (l *Lexer) tokenWithLiteral(tokenType token.TokenType, literal string, line, column, offset int) token.Token {
	return token.Token{Type: tokenType, Literal: literal, File: l.file, Line: line, Column: column, Offset: offset}
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
