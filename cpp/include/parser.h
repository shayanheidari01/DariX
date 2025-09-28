#pragma once

#include "ast.h"
#include "token.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace darix {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    const std::vector<Token> tokens_;
    size_t current_ = 0;

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    void consume(TokenType type, const std::string& message);

    // Expression parsing
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> orExpr();
    std::unique_ptr<Expr> andExpr();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> array();
    std::unique_ptr<Expr> map();

    // Statement parsing
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> expressionStatement();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> funcDeclaration();
    std::unique_ptr<Stmt> classDeclaration();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> forStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> tryStatement();
    std::unique_ptr<Stmt> block();

    // Helper methods
    std::vector<std::unique_ptr<Stmt>> blockStatements();
    std::vector<std::string> parameters();
    std::vector<std::unique_ptr<Expr>> arguments();
    std::vector<std::unique_ptr<Expr>> arrayElements();
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> mapPairs();

    // Error handling
    void synchronize();
};

} // namespace darix
