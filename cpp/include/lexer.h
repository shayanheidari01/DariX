#pragma once

#include "token.h"
#include <vector>
#include <string>

namespace darix {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> scanTokens();

private:
    const std::string source_;
    std::vector<Token> tokens_;
    size_t start_ = 0;
    size_t current_ = 0;
    int line_ = 1;
    int column_ = 1;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    void addToken(TokenType type);
    void addToken(TokenType type, std::string literal);

    void scanToken();
    void string();
    void number();
    void identifier();

    static bool isDigit(char c);
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
    static TokenType identifierType(const std::string& identifier);
};

} // namespace darix
