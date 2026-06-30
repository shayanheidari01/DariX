#pragma once

#include "darix/token.hpp"
#include <string>

namespace darix {

class Lexer {
public:
    Lexer(const std::string& input, const std::string& file = "");

    Token nextToken();

private:
    void readChar();
    char peekChar() const;
    char peekCharAt(int offset) const;

    void skipCommentsAndWhitespace();
    void skipWhitespace();
    void skipLineComment();
    void skipSeparator();
    void skipBlockComment();
    void skipUntilNewline();
    void skipUntilClosingBlock();

    Token newToken(TokenType type);
    Token makeTwoCharToken(char secondChar, TokenType twoCharType, TokenType oneCharType);
    Token tokenWithLiteral(TokenType type, const std::string& literal, int line, int column, int offset);

    std::string readNumber();
    std::string readString();
    std::string readIdentifier();

    std::string input_;
    int position_ = 0;
    int readPosition_ = 0;
    char ch_ = 0;
    int line_ = 1;
    int column_ = 0;
    std::string file_;
};

} // namespace darix
