#pragma once

#include "ast.h"
#include "token.h"
#include <vector>
#include <memory>
#include <string_view>
#include <stdexcept>
#include <sstream>

namespace darix {

// Compile-time parsing configuration
namespace config {
    constexpr size_t MAX_PRECEDENCE = 12;
    constexpr size_t SYNC_TOKENS_LIMIT = 50;
}

// Parser error with location information
class ParseError : public std::runtime_error {
public:
    explicit ParseError(std::string msg, uint32_t line, uint32_t col)
        : std::runtime_error(std::move(msg)), line_(line), col_(col) {}
    
    [[nodiscard]] uint32_t line() const noexcept { return line_; }
    [[nodiscard]] uint32_t column() const noexcept { return col_; }
    
private:
    uint32_t line_;
    uint32_t col_;
};

// Operator precedence levels
enum class Precedence : uint8_t {
    NONE = 0,
    ASSIGNMENT,    // =
    OR,            // ||
    AND,           // &&
    EQUALITY,      // == !=
    COMPARISON,    // < > <= >=
    TERM,          // + -
    FACTOR,        // * / %
    UNARY,         // ! -
    CALL,          // () [] .
    PRIMARY
};

// Forward declaration for function body parsing
struct FunctionDecl;

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    
    [[nodiscard]] std::vector<std::unique_ptr<Stmt>> parse();
    [[nodiscard]] bool hasErrors() const noexcept { return hasErrors_; }
    [[nodiscard]] const std::vector<std::string>& errors() const noexcept { return errors_; }
    
private:
    const std::vector<Token>& tokens_;
    size_t current_ = 0;
    bool hasErrors_ = false;
    std::vector<std::string> errors_;
    
    // Navigation helpers - all marked inline for performance
    [[nodiscard]] constexpr bool isAtEnd() const noexcept {
        return current_ >= tokens_.size();
    }
    
    [[nodiscard]] constexpr const Token& peek() const noexcept {
        return tokens_[current_];
    }
    
    [[nodiscard]] constexpr const Token& peekNext() const noexcept {
        if (current_ + 1 >= tokens_.size()) {
            static const Token eof{TokenType::EOF_TOKEN, "", 0, 0};
            return eof;
        }
        return tokens_[current_ + 1];
    }
    
    [[nodiscard]] constexpr const Token& previous() const noexcept {
        return tokens_[current_ - 1];
    }
    
    inline const Token& advance() noexcept {
        if (!isAtEnd()) current_++;
        return previous();
    }
    
    [[nodiscard]] inline bool check(TokenType type) const noexcept {
        if (isAtEnd()) return false;
        return peek().type == type;
    }
    
    inline bool match(TokenType type) noexcept {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }
    
    // Expression parsing with precedence climbing
    [[nodiscard]] std::unique_ptr<Expr> expression();
    [[nodiscard]] std::unique_ptr<Expr> parsePrecedence(Precedence precedence);
    
    // Statement parsing
    [[nodiscard]] std::unique_ptr<Stmt> statement();
    [[nodiscard]] std::unique_ptr<Stmt> declaration();
    [[nodiscard]] std::unique_ptr<Stmt> varDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> functionDeclaration(const std::string& kind);
    [[nodiscard]] std::unique_ptr<Stmt> classDeclaration();
    [[nodiscard]] std::unique_ptr<Stmt> ifStatement();
    [[nodiscard]] std::unique_ptr<Stmt> whileStatement();
    [[nodiscard]] std::unique_ptr<Stmt> forStatement();
    [[nodiscard]] std::unique_ptr<Stmt> returnStatement();
    [[nodiscard]] std::unique_ptr<Stmt> tryStatement();
    [[nodiscard]] std::unique_ptr<Stmt> block();
    [[nodiscard]] std::unique_ptr<Stmt> expressionStatement();
    
    // Specific expression parsers
    [[nodiscard]] std::unique_ptr<Expr> assignment();
    [[nodiscard]] std::unique_ptr<Expr> orExpr();
    [[nodiscard]] std::unique_ptr<Expr> andExpr();
    [[nodiscard]] std::unique_ptr<Expr> equality();
    [[nodiscard]] std::unique_ptr<Expr> comparison();
    [[nodiscard]] std::unique_ptr<Expr> term();
    [[nodiscard]] std::unique_ptr<Expr> factor();
    [[nodiscard]] std::unique_ptr<Expr> unary();
    [[nodiscard]] std::unique_ptr<Expr> call();
    [[nodiscard]] std::unique_ptr<Expr> primary();
    [[nodiscard]] std::unique_ptr<Expr> array();
    [[nodiscard]] std::unique_ptr<Expr> map();
    [[nodiscard]] std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
    
    // Helper parsing methods
    [[nodiscard]] std::vector<std::string> parameters();
    [[nodiscard]] std::vector<std::unique_ptr<Expr>> arguments();
    [[nodiscard]] std::vector<std::unique_ptr<Expr>> arrayElements();
    [[nodiscard]] std::vector<MapExpr::KeyValue> mapPairs();
    [[nodiscard]] std::vector<std::unique_ptr<Stmt>> blockStatements();
    
    // Error handling
    void consume(TokenType type, const std::string& message);
    void synchronize();
    void error(const Token& token, const std::string& message);
    
    // Utility
    [[nodiscard]] static Precedence getPrecedence(TokenType type) noexcept;
};

} // namespace darix
