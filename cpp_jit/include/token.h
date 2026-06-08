#pragma once

#include <string>
#include <cstdint>

namespace darix {

// TokenType - Optimized with underlying integer values for faster comparison
enum class TokenType : uint8_t {
    // Keywords (0-15)
    CLASS = 0, FUNC, VAR, IF, ELSE, WHILE, FOR, RETURN, TRY, CATCH, FINALLY,
    TRUE, FALSE, NULL_,
    
    // Operators (16-31)
    PLUS = 16, MINUS, MULTIPLY, DIVIDE, MODULO,
    EQUAL, EQUAL_EQUAL, BANG, BANG_EQUAL,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    AND, OR, NOT,
    
    // Delimiters (32-47)
    LEFT_PAREN = 32, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET, COMMA, DOT, SEMICOLON,
    COLON,
    
    // Literals (48-51)
    IDENTIFIER = 48, STRING, NUMBER,
    
    // Special (52+)
    EOF_TOKEN = 52
};

// Token - Cache-line optimized structure
struct alignas(32) Token {
    TokenType type;
    std::string_view lexeme;  // Use string_view for zero-copy when possible
    uint32_t line;
    uint32_t column;
    
    constexpr Token(TokenType t, std::string_view lex, uint32_t l, uint32_t c)
        : type(t), lexeme(lex), line(l), column(c) {}
    
    // Comparison operators for fast lookup
    [[nodiscard]] constexpr bool isKeyword() const noexcept {
        return static_cast<uint8_t>(type) <= static_cast<uint8_t>(TokenType::FALSE);
    }
    
    [[nodiscard]] constexpr bool isOperator() const noexcept {
        auto t = static_cast<uint8_t>(type);
        return t >= static_cast<uint8_t>(TokenType::PLUS) && 
               t <= static_cast<uint8_t>(TokenType::OR);
    }
};

// Fast keyword hash function for compile-time keyword lookup
constexpr uint32_t hashKeyword(const char* str, size_t len) noexcept {
    uint32_t hash = 0;
    for (size_t i = 0; i < len; ++i) {
        hash = hash * 31 + static_cast<uint32_t>(str[i]);
    }
    return hash;
}

// Compile-time keyword hash helper
#define KEYWORD_HASH(str) darix::hashKeyword(str, sizeof(str) - 1)

} // namespace darix

// Custom hash for TokenType
namespace std {
    template<>
    struct hash<darix::TokenType> {
        size_t operator()(darix::TokenType t) const noexcept {
            return static_cast<size_t>(t);
        }
    };
}
