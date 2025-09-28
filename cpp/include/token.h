#pragma once

#include <string>

namespace darix {

enum class TokenType {
    // Keywords
    CLASS, FUNC, VAR, IF, ELSE, WHILE, FOR, RETURN, TRY, CATCH, FINALLY,
    TRUE, FALSE, NULL,

    // Operators
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    EQUAL, EQUAL_EQUAL, BANG, BANG_EQUAL,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    AND, OR, NOT,

    // Delimiters
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET, COMMA, DOT, SEMICOLON,
    COLON,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Special
    EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}
};

} // namespace darix
