#include "lexer.h"
#include <unordered_map>
#include <cctype>

namespace darix {

static const std::unordered_map<std::string, TokenType> keywords = {
    {"class", TokenType::CLASS},
    {"func", TokenType::FUNC},
    {"var", TokenType::VAR},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"return", TokenType::RETURN},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL}
};

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }

    tokens_.emplace_back(TokenType::EOF_TOKEN, "", line_, column_);
    return tokens_;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

char Lexer::advance() {
    column_++;
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;

    current_++;
    column_++;
    return true;
}

void Lexer::addToken(TokenType type) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, text, line_, column_ - (current_ - start_));
}

void Lexer::addToken(TokenType type, std::string literal) {
    tokens_.emplace_back(type, std::move(literal), line_, column_ - (current_ - start_));
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case ':': addToken(TokenType::COLON); break;
        case '+': addToken(TokenType::PLUS); break;
        case '*': addToken(TokenType::MULTIPLY); break;
        case '/':
            if (match('/')) {
                // Comment
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::DIVIDE);
            }
            break;
        case '%': addToken(TokenType::MODULO); break;
        case '-': addToken(TokenType::MINUS); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '&':
            if (match('&')) {
                addToken(TokenType::AND);
            }
            break;
        case '|':
            if (match('|')) {
                addToken(TokenType::OR);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;
        case '\n':
            line_++;
            column_ = 1;
            break;
        case '"': string(); break;
        default:
            if (isDigit(c)) {
                number();
            } else if (isAlpha(c)) {
                identifier();
            } else {
                // Error handling would go here
            }
            break;
    }
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line_++;
        advance();
    }

    if (isAtEnd()) {
        // Unterminated string error
        return;
    }

    // The closing "
    advance();

    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();

        while (isDigit(peek())) advance();
    }

    std::string value = source_.substr(start_, current_ - start_);
    addToken(TokenType::NUMBER, value);
}

void Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();

    std::string text = source_.substr(start_, current_ - start_);
    TokenType type = identifierType(text);
    addToken(type);
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

TokenType Lexer::identifierType(const std::string& identifier) {
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

} // namespace darix
