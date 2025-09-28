#include "parser.h"
#include <iostream>

namespace darix {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(statement());
        } catch (...) {
            synchronize();
        }
    }
    return statements;
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    // Error handling would go here
    std::cout << "Parse error at line " << peek().line << ": " << message << std::endl;
    throw std::runtime_error(message);
}

// Expression parsing - recursive descent

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = orExpr();

    if (match(TokenType::EQUAL)) {
        auto value = assignment();
        return std::make_unique<AssignExpr>(std::move(expr), std::move(value));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::orExpr() {
    auto expr = andExpr();

    while (match(TokenType::OR)) {
        auto op = previous();
        auto right = andExpr();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::andExpr() {
    auto expr = equality();

    while (match(TokenType::AND)) {
        auto op = previous();
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        auto op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        auto op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        auto op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();

    while (match({TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO})) {
        auto op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        auto op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }

    return call();
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary();

    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            auto arguments = this->arguments();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(arguments));
        } else if (match(TokenType::LEFT_BRACKET)) {
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else if (match(TokenType::DOT)) {
            consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            std::string property = previous().lexeme;
            expr = std::make_unique<MemberExpr>(std::move(expr), property);
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::FALSE)) return std::make_unique<BoolExpr>(false);
    if (match(TokenType::TRUE)) return std::make_unique<BoolExpr>(true);
    if (match(TokenType::NULL)) return std::make_unique<NullExpr>();

    if (match(TokenType::NUMBER)) {
        std::string lexeme = previous().lexeme;
        if (lexeme.find('.') != std::string::npos) {
            return std::make_unique<NumberExpr>(std::stod(lexeme));
        } else {
            return std::make_unique<LiteralExpr>(lexeme);
        }
    }

    if (match(TokenType::STRING)) {
        std::string value = previous().lexeme;
        // Remove quotes
        value = value.substr(1, value.size() - 2);
        return std::make_unique<StringExpr>(value);
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpr>(previous().lexeme);
    }

    if (match(TokenType::LEFT_PAREN)) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    if (match(TokenType::LEFT_BRACKET)) {
        return array();
    }

    if (match(TokenType::LEFT_BRACE)) {
        return map();
    }

    // Error
    std::cout << "Expect expression." << std::endl;
    return nullptr;
}

std::unique_ptr<Expr> Parser::array() {
    std::vector<std::unique_ptr<Expr>> elements;
    if (!check(TokenType::RIGHT_BRACKET)) {
        elements = arrayElements();
    }
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
    return std::make_unique<ArrayExpr>(std::move(elements));
}

std::unique_ptr<Expr> Parser::map() {
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            auto key = expression();
            consume(TokenType::COLON, "Expect ':' after map key.");
            auto value = expression();
            pairs.emplace_back(std::move(key), std::move(value));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after map entries.");
    return std::make_unique<MapExpr>(std::move(pairs));
}

// Statement parsing

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::VAR)) return varDeclaration();
    if (match(TokenType::FUNC)) return funcDeclaration();
    if (match(TokenType::CLASS)) return classDeclaration();
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::WHILE)) return whileStatement();
    if (match(TokenType::FOR)) return forStatement();
    if (match(TokenType::RETURN)) return returnStatement();
    if (match(TokenType::TRY)) return tryStatement();
    if (match(TokenType::LEFT_BRACE)) return block();

    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::string name = previous().lexeme;

    std::unique_ptr<Expr> initializer;
    if (match(TokenType::EQUAL)) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarDecl>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::funcDeclaration() {
    consume(TokenType::IDENTIFIER, "Expect function name.");
    std::string name = previous().lexeme;

    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    auto params = parameters();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    auto body = blockStatements();
    consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");

    return std::make_unique<FuncDecl>(name, params, body);
}

std::unique_ptr<Stmt> Parser::classDeclaration() {
    consume(TokenType::IDENTIFIER, "Expect class name.");
    std::string name = previous().lexeme;

    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

    std::vector<std::unique_ptr<FuncDecl>> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        methods.push_back(std::unique_ptr<FuncDecl>(static_cast<FuncDecl*>(funcDeclaration().release())));
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return std::make_unique<ClassDecl>(name, methods);
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    auto thenBranch = blockStatements();

    std::vector<std::unique_ptr<Stmt>> elseBranch;
    if (match(TokenType::ELSE)) {
        if (check(TokenType::IF)) {
            elseBranch.push_back(ifStatement());
        } else {
            elseBranch = blockStatements();
        }
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");

    auto body = blockStatements();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::unique_ptr<Stmt> initializer;
    if (match(TokenType::VAR)) {
        initializer = varDeclaration();
    } else if (!match(TokenType::SEMICOLON)) {
        initializer = expressionStatement();
    }

    std::unique_ptr<Expr> condition;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::unique_ptr<Expr> increment;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    auto body = blockStatements();

    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    std::unique_ptr<Expr> value;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::tryStatement() {
    auto tryBody = blockStatements();

    consume(TokenType::CATCH, "Expect 'catch' after try block.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'catch'.");
    consume(TokenType::IDENTIFIER, "Expect exception variable name.");
    std::string catchVar = previous().lexeme;
    consume(TokenType::RIGHT_PAREN, "Expect ')' after catch variable.");

    auto catchBody = blockStatements();

    std::vector<std::unique_ptr<Stmt>> finallyBody;
    if (match(TokenType::FINALLY)) {
        finallyBody = blockStatements();
    }

    return std::make_unique<TryStmt>(std::move(tryBody), catchVar, std::move(catchBody), std::move(finallyBody));
}

std::unique_ptr<Stmt> Parser::block() {
    auto statements = blockStatements();
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

// Helper methods

std::vector<std::unique_ptr<Stmt>> Parser::blockStatements() {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(statement());
    }

    return statements;
}

std::vector<std::string> Parser::parameters() {
    std::vector<std::string> params;

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back(previous().lexeme);
        } while (match(TokenType::COMMA));
    }

    return params;
}

std::vector<std::unique_ptr<Expr>> Parser::arguments() {
    std::vector<std::unique_ptr<Expr>> args;

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            args.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    return args;
}

std::vector<std::unique_ptr<Expr>> Parser::arrayElements() {
    std::vector<std::unique_ptr<Expr>> elements;

    do {
        elements.push_back(expression());
    } while (match(TokenType::COMMA));

    return elements;
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUNC:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
            case TokenType::TRY:
                return;
            default:
                break;
        }

        advance();
    }
}

} // namespace darix
