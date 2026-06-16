#pragma once

#include "token.h"
#include <string>
#include <vector>

namespace darix {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> scanTokens();

private:
    std::string source_;
    std::vector<Token> tokens_;
    size_t start_ = 0;
    size_t current_ = 0;
    int line_ = 1;

    void scanToken();
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    bool match(char expected);
    Token makeToken(TokenType type, std::any literal = std::monostate{});
    Token errorToken(const std::string& message);
    
    void string();
    void number();
    void identifier();
    
    static bool isAlpha(char c);
    static bool isDigit(char c);
    static bool isAlphaNumeric(char c);
};

} // namespace darix
