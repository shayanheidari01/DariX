#pragma once

#include "token.h"
#include <vector>
#include <string>
#include <string_view>
#include <span>

namespace darix {

// Lexer - Optimized for performance with minimal allocations
class Lexer {
public:
    explicit Lexer(std::string_view source);
    
    // Returns span of tokens (zero-copy where possible)
    [[nodiscard]] std::vector<Token> scanTokens();
    
private:
    const std::string_view source_;
    std::vector<Token> tokens_;
    size_t start_ = 0;
    size_t current_ = 0;
    uint32_t line_ = 1;
    uint32_t column_ = 1;
    
    // Fast path functions marked inline
    [[nodiscard]] constexpr bool isAtEnd() const noexcept {
        return current_ >= source_.size();
    }
    
    [[nodiscard]] inline char advance() noexcept {
        column_++;
        return source_[current_++];
    }
    
    [[nodiscard]] inline char peek() const noexcept {
        if (isAtEnd()) return '\0';
        return source_[current_];
    }
    
    [[nodiscard]] inline char peekNext() const noexcept {
        if (current_ + 1 >= source_.size()) return '\0';
        return source_[current_ + 1];
    }
    
    inline bool match(char expected) noexcept {
        if (isAtEnd()) return false;
        if (source_[current_] != expected) return false;
        current_++;
        column_++;
        return true;
    }
    
    void addToken(TokenType type);
    void addToken(TokenType type, std::string_view literal);
    
    void scanToken();
    void string();
    void number();
    void identifier();
    
    // Static lookup table for character classification
    static constexpr bool isDigitChar(char c) noexcept {
        return c >= '0' && c <= '9';
    }
    
    static constexpr bool isAlphaChar(char c) noexcept {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_';
    }
    
    static constexpr bool isAlphaNumericChar(char c) noexcept {
        return isAlphaChar(c) || isDigitChar(c);
    }
    
    // Perfect hash function for keyword lookup
    [[nodiscard]] static TokenType identifierType(std::string_view identifier) noexcept;
};

} // namespace darix
