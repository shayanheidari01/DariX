#include "parser.h"
#include <algorithm>

namespace darix {

Parser::Parser(std::vector<Token> tokens) 
    : tokens_(std::move(tokens)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!isAtEnd()) {
        try {
            auto stmt = declaration();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const ParseError&) {
            synchronize();
        }
    }
    
    return statements;
}

Precedence Parser::getPrecedence(TokenType type) noexcept {
    switch (type) {
        case TokenType::EQUAL:
            return Precedence::ASSIGNMENT;
        case TokenType::OR:
            return Precedence::OR;
        case TokenType::AND:
            return Precedence::AND;
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
            return Precedence::EQUALITY;
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
            return Precedence::COMPARISON;
        case TokenType::PLUS:
        case TokenType::MINUS:
            return Precedence::TERM;
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULO:
            return Precedence::FACTOR;
        case TokenType::BANG:
        case TokenType::MINUS:  // Unary minus
            return Precedence::UNARY;
        default:
            return Precedence::NONE;
    }
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = orExpr();
    
    if (match(TokenType::EQUAL)) {
        Token op = previous();
        auto value = assignment();
        return std::make_unique<AssignExpr>(std::move(expr), std::move(value));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::orExpr() {
    auto expr = andExpr();
    
    while (match(TokenType::OR)) {
        Token op = previous();
        auto right = andExpr();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::andExpr() {
    auto expr = equality();
    
    while (match(TokenType::AND)) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    
    while (match(TokenType::BANG_EQUAL) || match(TokenType::EQUAL_EQUAL)) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    
    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) ||
           match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    
    while (match(TokenType::MINUS) || match(TokenType::PLUS)) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || 
           match(TokenType::MODULO)) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenType::BANG) || match(TokenType::MINUS)) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary();
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            expr = finishCall(std::move(expr));
        } else if (match(TokenType::LEFT_BRACKET)) {
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else if (match(TokenType::DOT)) {
            consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            std::string property = std::string(previous().lexeme);
            expr = std::make_unique<MemberExpr>(std::move(expr), std::move(property));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralExpr>(LiteralValue(false));
    }
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralExpr>(LiteralValue(true));
    }
    if (match(TokenType::NULL_)) {
        return std::make_unique<LiteralExpr>(LiteralValue());
    }
    
    if (match(TokenType::NUMBER)) {
        const Token& tok = previous();
        std::string_view lexeme = tok.lexeme;
        
        // Parse number - check for decimal point or exponent
        bool isFloat = false;
        for (char c : lexeme) {
            if (c == '.' || c == 'e' || c == 'E') {
                isFloat = true;
                break;
            }
        }
        
        if (isFloat) {
            double value = std::stod(std::string(lexeme));
            return std::make_unique<LiteralExpr>(LiteralValue(value));
        } else {
            long long value = std::stoll(std::string(lexeme));
            return std::make_unique<LiteralExpr>(LiteralValue(value));
        }
    }
    
    if (match(TokenType::STRING)) {
        return std::make_unique<LiteralExpr>(LiteralValue(std::string(previous().lexeme)));
    }
    
    if (match(TokenType::IDENTIFIER)) {
        const Token& tok = previous();
        return std::make_unique<VariableExpr>(std::string(tok.lexeme), tok.line, tok.column);
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
    
    error(peek(), "Expect expression.");
    return nullptr;
}

std::unique_ptr<Expr> Parser::array() {
    std::vector<std::unique_ptr<Expr>> elements;
    
    if (!check(TokenType::RIGHT_BRACKET)) {
        do {
            elements.push_back(expression());
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
    return std::make_unique<ArrayExpr>(std::move(elements));
}

std::unique_ptr<Expr> Parser::map() {
    std::vector<MapExpr::KeyValue> pairs;
    
    if (!check(TokenType::RIGHT_BRACE)) {
        do {
            MapExpr::KeyValue kv;
            kv.key = expression();
            consume(TokenType::COLON, "Expect ':' after map key.");
            kv.value = expression();
            pairs.push_back(std::move(kv));
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after map entries.");
    return std::make_unique<MapExpr>(std::move(pairs));
}

// Statement parsing

std::unique_ptr<Stmt> Parser::declaration() {
    if (match(TokenType::VAR)) return varDeclaration();
    if (match(TokenType::FUNC)) return functionDeclaration("function");
    if (match(TokenType::CLASS)) return classDeclaration();
    return statement();
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::WHILE)) return whileStatement();
    if (match(TokenType::FOR)) return forStatement();
    if (match(TokenType::RETURN)) return returnStatement();
    if (match(TokenType::TRY)) return tryStatement();
    if (match(TokenType::LEFT_BRACE)) return block();
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::string name = std::string(previous().lexeme);
    
    std::unique_ptr<Expr> initializer;
    if (match(TokenType::EQUAL)) {
        initializer = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarDecl>(std::move(name), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::functionDeclaration(const std::string& kind) {
    consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
    std::string name = std::string(previous().lexeme);
    uint32_t line = previous().line;
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
    auto params = parameters();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    auto body = blockStatements();
    consume(TokenType::RIGHT_BRACE, "Expect '}' after " + kind + " body.");
    
    return std::make_unique<FuncDecl>(std::move(name), std::move(params), 
                                       std::move(body), line);
}

std::unique_ptr<Stmt> Parser::classDeclaration() {
    consume(TokenType::IDENTIFIER, "Expect class name.");
    std::string name = std::string(previous().lexeme);
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");
    
    std::vector<std::unique_ptr<FuncDecl>> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto method = functionDeclaration("method");
        methods.push_back(std::unique_ptr<FuncDecl>(static_cast<FuncDecl*>(method.release())));
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return std::make_unique<ClassDecl>(std::move(name), std::move(methods));
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
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), 
                                     std::move(elseBranch));
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
    } else if (match(TokenType::SEMICOLON)) {
        // No initializer
    } else {
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
    
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                      std::move(increment), std::move(body));
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
    std::string catchVar = std::string(previous().lexeme);
    consume(TokenType::RIGHT_PAREN, "Expect ')' after catch variable.");
    
    auto catchBody = blockStatements();
    
    std::vector<std::unique_ptr<Stmt>> finallyBody;
    if (match(TokenType::FINALLY)) {
        finallyBody = blockStatements();
    }
    
    return std::make_unique<TryStmt>(std::move(tryBody), std::move(catchVar),
                                      std::move(catchBody), std::move(finallyBody));
}

std::unique_ptr<Stmt> Parser::block() {
    auto statements = blockStatements();
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

// Helper methods

std::vector<std::string> Parser::parameters() {
    std::vector<std::string> params;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.emplace_back(previous().lexeme);
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

std::vector<MapExpr::KeyValue> Parser::mapPairs() {
    std::vector<MapExpr::KeyValue> pairs;
    
    do {
        MapExpr::KeyValue kv;
        kv.key = expression();
        consume(TokenType::COLON, "Expect ':' after map key.");
        kv.value = expression();
        pairs.push_back(std::move(kv));
    } while (match(TokenType::COMMA));
    
    return pairs;
}

std::vector<std::unique_ptr<Stmt>> Parser::blockStatements() {
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(statement());
    }
    
    return statements;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    error(peek(), message);
}

void Parser::error(const Token& token, const std::string& message) {
    hasErrors_ = true;
    std::ostringstream oss;
    oss << "[line " << token.line << ", column " << token.column << "] Error";
    if (token.type == TokenType::EOF_TOKEN) {
        oss << " at end";
    } else if (token.type != TokenType::ERROR) {
        oss << " at '" << token.lexeme << "'";
    }
    oss << ": " << message;
    errors_.push_back(oss.str());
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
