#ifndef DARIX_LEXER_LEXER_HPP
#define DARIX_LEXER_LEXER_HPP

#include "lexer/token.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <stack>

namespace darix {

class LexerError {
public:
    size_t line;
    size_t column;
    std::string message;
    
    LexerError(size_t line, size_t column, std::string message)
        : line(line), column(column), message(std::move(message)) {}
    
    [[nodiscard]] std::string toString() const {
        return "LexerError at line " + std::to_string(line) + 
               ", column " + std::to_string(column) + ": " + message;
    }
};

class Lexer {
public:
    explicit Lexer(std::string_view source);
    
    [[nodiscard]] std::vector<Token> tokenize();
    [[nodiscard]] const std::vector<LexerError>& getErrors() const { return errors_; }
    [[nodiscard]] bool hasErrors() const { return !errors_.empty(); }
    
private:
    std::string_view source_;
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    std::vector<LexerError> errors_;
    std::vector<Token> tokens_;
    std::stack<int> indentStack_;
    std::vector<int> pendingDedents_;
    
    // Keywords map
    static const std::unordered_map<std::string_view, TokenType> keywords_;
    
    void scanToken();
    char advance();
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    bool match(char expected);
    void addToken(TokenType type, TokenValue value = {});
    void addToken(TokenType type, std::string_view lexeme);
    
    // Scanners for specific token types
    void scanNumber();
    void scanString();
    void scanIdentifier();
    void skipWhitespace();
    void skipComment();
    void handleNewline();
    
    [[nodiscard]] std::string_view getLexeme() const {
        return source_.substr(start_, current_ - start_);
    }
};

} // namespace darix

#endif // DARIX_LEXER_LEXER_HPP
