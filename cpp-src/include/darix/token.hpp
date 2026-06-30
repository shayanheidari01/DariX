#pragma once

#include <string>
#include <unordered_map>

namespace darix {

enum class TokenType {
    ILLEGAL,
    EOF_TOKEN,

    // Identifiers + literals
    IDENT,
    INT,
    FLOAT,
    STRING,

    // Operators
    ASSIGN,
    PLUS,
    MINUS,
    BANG,
    OR,
    AND,
    ASTERISK,
    SLASH,
    MODULO,
    LT,
    GT,
    LE,
    GE,
    EQ,
    NOT_EQ,

    // Delimiters
    COMMA,
    SEMICOLON,
    COLON,
    DOT,
    AT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,

    // Keywords
    FUNCTION,
    CLASS,
    VAR,
    TRUE,
    FALSE,
    NULL_TOKEN,
    IF,
    ELSE,
    ELIF,
    RETURN,
    WHILE,
    FOR,
    BREAK,
    CONTINUE,
    IMPORT,
    FROM,
    AS,
    TRY,
    CATCH,
    FINALLY,
    THROW,
    RAISE,
    DEL,
    ASSERT,
    PASS,
    AND_KW,
    OR_KW,
    NOT_KW,
    IN,
    IS,
    WITH,
    YIELD,
    GLOBAL,
    NONLOCAL,
    LAMBDA,
};

const char* TokenTypeToString(TokenType type);

struct Token {
    TokenType type;
    std::string literal;
    std::string file;
    int line = 0;
    int column = 0;
    int offset = 0;
};

TokenType LookupIdent(const std::string& ident);
void RegisterKeyword(const std::string& literal, TokenType type);

} // namespace darix
