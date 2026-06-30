#include "darix/lexer.hpp"
#include <cctype>

namespace darix {

Lexer::Lexer(const std::string& input, const std::string& file)
    : input_(input), file_(file) {
    readChar();
}

void Lexer::readChar() {
    if (readPosition_ >= static_cast<int>(input_.size())) {
        ch_ = 0;
        position_ = readPosition_;
        readPosition_++;
        return;
    }
    ch_ = input_[readPosition_];
    position_ = readPosition_;
    readPosition_++;
    if (ch_ == '\n') {
        line_++;
        column_ = 0;
    } else if (ch_ != 0) {
        column_++;
    }
}

char Lexer::peekChar() const {
    if (readPosition_ >= static_cast<int>(input_.size())) {
        return 0;
    }
    return input_[readPosition_];
}

char Lexer::peekCharAt(int offset) const {
    int pos = readPosition_ + offset - 1;
    if (pos >= static_cast<int>(input_.size())) {
        return 0;
    }
    return input_[pos];
}

Token Lexer::nextToken() {
    Token tok;

    skipCommentsAndWhitespace();

    int startLine = line_;
    int startColumn = column_;
    int startOffset = position_;

    switch (ch_) {
        case '=':
            tok = makeTwoCharToken('=', TokenType::EQ, TokenType::ASSIGN);
            break;
        case '+':
            tok = newToken(TokenType::PLUS);
            break;
        case '-':
            tok = newToken(TokenType::MINUS);
            break;
        case '!':
            tok = makeTwoCharToken('=', TokenType::NOT_EQ, TokenType::BANG);
            break;
        case '/':
            tok = newToken(TokenType::SLASH);
            break;
        case '*':
            tok = newToken(TokenType::ASTERISK);
            break;
        case '%':
            tok = newToken(TokenType::MODULO);
            break;
        case '<':
            tok = makeTwoCharToken('=', TokenType::LE, TokenType::LT);
            break;
        case '>':
            tok = makeTwoCharToken('=', TokenType::GE, TokenType::GT);
            break;
        case '&':
            if (peekChar() == '&') {
                readChar();
                tok = tokenWithLiteral(TokenType::AND, "&&", startLine, startColumn, startOffset);
            } else {
                tok = tokenWithLiteral(TokenType::ILLEGAL, std::string(1, ch_), startLine, startColumn, startOffset);
            }
            break;
        case '|':
            if (peekChar() == '|') {
                readChar();
                tok = tokenWithLiteral(TokenType::OR, "||", startLine, startColumn, startOffset);
            } else {
                tok = tokenWithLiteral(TokenType::ILLEGAL, std::string(1, ch_), startLine, startColumn, startOffset);
            }
            break;
        case ',': tok = newToken(TokenType::COMMA); break;
        case ';': tok = newToken(TokenType::SEMICOLON); break;
        case ':': tok = newToken(TokenType::COLON); break;
        case '.': tok = newToken(TokenType::DOT); break;
        case '@': tok = newToken(TokenType::AT); break;
        case '(': tok = newToken(TokenType::LPAREN); break;
        case ')': tok = newToken(TokenType::RPAREN); break;
        case '{': tok = newToken(TokenType::LBRACE); break;
        case '}': tok = newToken(TokenType::RBRACE); break;
        case '[': tok = newToken(TokenType::LBRACKET); break;
        case ']': tok = newToken(TokenType::RBRACKET); break;
        case '"':
            tok = tokenWithLiteral(TokenType::STRING, readString(), startLine, startColumn, startOffset);
            return tok;
        case 0:
            tok = tokenWithLiteral(TokenType::EOF_TOKEN, "", startLine, startColumn, startOffset);
            break;
        default:
            if (std::isalpha(static_cast<unsigned char>(ch_)) || ch_ == '_') {
                std::string literal = readIdentifier();
                return tokenWithLiteral(LookupIdent(literal), literal, startLine, startColumn, startOffset);
            } else if (std::isdigit(static_cast<unsigned char>(ch_))) {
                std::string number = readNumber();
                TokenType tokType = TokenType::INT;
                if (number.find('.') != std::string::npos) {
                    tokType = TokenType::FLOAT;
                }
                return tokenWithLiteral(tokType, number, startLine, startColumn, startOffset);
            } else {
                tok = tokenWithLiteral(TokenType::ILLEGAL, std::string(1, ch_), startLine, startColumn, startOffset);
            }
            break;
    }

    readChar();
    return tok;
}

void Lexer::skipCommentsAndWhitespace() {
    for (;;) {
        skipWhitespace();

        if (ch_ == '/') {
            char next = peekChar();
            if (next == '/') {
                if (peekCharAt(2) == '-') {
                    skipSeparator();
                } else {
                    skipLineComment();
                }
                continue;
            } else if (next == '*') {
                skipBlockComment();
                continue;
            }
        }
        break;
    }
}

void Lexer::skipWhitespace() {
    while (ch_ == ' ' || ch_ == '\t' || ch_ == '\n' || ch_ == '\r') {
        readChar();
    }
}

void Lexer::skipLineComment() {
    skipUntilNewline();
}

void Lexer::skipSeparator() {
    skipUntilNewline();
}

void Lexer::skipBlockComment() {
    readChar(); // skip '/'
    readChar(); // skip '*'
    skipUntilClosingBlock();
}

void Lexer::skipUntilNewline() {
    while (ch_ != '\n' && ch_ != 0) {
        readChar();
    }
}

void Lexer::skipUntilClosingBlock() {
    for (;;) {
        if (ch_ == 0) return;
        if (ch_ == '*' && peekChar() == '/') {
            readChar(); // skip '*'
            readChar(); // skip '/'
            return;
        }
        readChar();
    }
}

Token Lexer::newToken(TokenType type) {
    return tokenWithLiteral(type, std::string(1, ch_), line_, column_, position_);
}

Token Lexer::makeTwoCharToken(char secondChar, TokenType twoCharType, TokenType oneCharType) {
    if (peekChar() == secondChar) {
        char first = ch_;
        int startLine = line_;
        int startColumn = column_;
        int startOffset = position_;
        readChar();
        return tokenWithLiteral(twoCharType, std::string(1, first) + std::string(1, ch_), startLine, startColumn, startOffset);
    }
    return tokenWithLiteral(oneCharType, std::string(1, ch_), line_, column_, position_);
}

Token Lexer::tokenWithLiteral(TokenType type, const std::string& literal, int line, int column, int offset) {
    return Token{type, literal, file_, line, column, offset};
}

std::string Lexer::readNumber() {
    int pos = position_;
    while (std::isdigit(static_cast<unsigned char>(ch_))) {
        readChar();
    }
    if (ch_ == '.' && std::isdigit(static_cast<unsigned char>(peekChar()))) {
        readChar(); // consume '.'
        while (std::isdigit(static_cast<unsigned char>(ch_))) {
            readChar();
        }
    }
    return input_.substr(pos, position_ - pos);
}

std::string Lexer::readString() {
    std::string result;
    readChar(); // skip opening quote

    while (ch_ != '"' && ch_ != 0) {
        if (ch_ == '\\') {
            readChar(); // consume backslash
            if (ch_ == 0) {
                result += '\\';
                break;
            }
            switch (ch_) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case 'r':  result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"':  result += '"'; break;
                default:
                    result += '\\';
                    result += ch_;
                    break;
            }
            readChar();
        } else {
            result += ch_;
            readChar();
        }
    }

    if (ch_ == '"') {
        readChar();
    }

    return result;
}

std::string Lexer::readIdentifier() {
    int pos = position_;
    while (std::isalpha(static_cast<unsigned char>(ch_)) || std::isdigit(static_cast<unsigned char>(ch_)) || ch_ == '_') {
        readChar();
    }
    return input_.substr(pos, position_ - pos);
}

} // namespace darix
