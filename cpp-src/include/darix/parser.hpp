#pragma once

#include "darix/ast.hpp"
#include "darix/lexer.hpp"
#include "darix/token.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace darix {

enum Precedence {
    LOWEST = 0,
    ASSIGN,
    EQUALS,
    LESSGREATER,
    SUM,
    OR,
    AND,
    PRODUCT,
    PREFIX,
    CALL,
    INDEX,
    MEMBER,
};

class Parser {
public:
    explicit Parser(Lexer& lexer);

    void setReplMode(bool mode);
    std::shared_ptr<Program> parseProgram();
    const std::vector<std::string>& errors() const;

private:
    using PrefixParseFn = std::function<ExpressionPtr()>;
    using InfixParseFn = std::function<ExpressionPtr(ExpressionPtr)>;

    void registerParseFns();
    void nextToken();

    StatementPtr parseStatement();
    ExpressionPtr parseExpression(int precedence);

    bool isAssignment();

    // Prefix parse functions
    ExpressionPtr parseIdentifier();
    ExpressionPtr parseIntegerLiteral();
    ExpressionPtr parseFloatLiteral();
    ExpressionPtr parseStringLiteral();
    ExpressionPtr parseBoolean();
    ExpressionPtr parseNull();
    ExpressionPtr parsePrefixExpression();
    ExpressionPtr parseGroupedExpression();
    ExpressionPtr parseIfExpression();
    ExpressionPtr parseFunctionLiteral();
    ExpressionPtr parseArrayLiteral();
    ExpressionPtr parseMapLiteral();
    ExpressionPtr parseWhileExpression();
    ExpressionPtr parseForExpression();
    ExpressionPtr parseLambdaExpression();
    ExpressionPtr parseYieldExpression();

    // Infix parse functions
    ExpressionPtr parseInfixExpression(ExpressionPtr left);
    ExpressionPtr parseCallExpression(ExpressionPtr fn);
    ExpressionPtr parseIndexExpression(ExpressionPtr left);
    ExpressionPtr parseMemberExpression(ExpressionPtr left);
    ExpressionPtr parseAssignmentExpression(ExpressionPtr left);
    ExpressionPtr parseInExpression(ExpressionPtr left);
    ExpressionPtr parseIsExpression(ExpressionPtr left);

    // Statement parse functions
    StatementPtr parseLetStatement();
    StatementPtr parseClassDeclaration();
    StatementPtr parseReturnStatement();
    StatementPtr parseExpressionStatement();
    StatementPtr parseBlockStatementAsStatement();
    StatementPtr parseAssignStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseForStatement();
    StatementPtr parseBreakStatement();
    StatementPtr parseContinueStatement();
    StatementPtr parseTryStatement();
    StatementPtr parseThrowStatement();
    StatementPtr parseImportStatement();
    StatementPtr parseFunctionDeclaration();
    StatementPtr parseDelStatement();
    StatementPtr parseAssertStatement();
    StatementPtr parsePassStatement();
    StatementPtr parseGlobalStatement();
    StatementPtr parseNonlocalStatement();
    StatementPtr parseWithStatement();
    StatementPtr parseDecoratedDefinition();

    // Helpers
    std::shared_ptr<BlockStatement> parseBlockStatement();
    std::vector<IdentifierPtr> parseFunctionParameters();
    std::vector<ExpressionPtr> parseExpressionList(TokenType end);
    std::vector<IdentifierPtr> parseIdentifierList(TokenType end);
    std::shared_ptr<CatchClause> parseCatchClause();

    bool curTokenIs(TokenType t) const;
    bool peekTokenIs(TokenType t) const;
    bool expectPeek(TokenType t);
    bool expectCurrent(TokenType t);
    void consumeOptionalSemicolon();
    int curPrecedence() const;
    int peekPrecedence() const;
    bool isValidAssignmentTarget(const ExpressionPtr& expr) const;

    void addError(const std::string& msg);

    Lexer& lexer_;
    Token curToken_;
    Token peekToken_;
    std::vector<std::string> errors_;
    std::unordered_map<TokenType, PrefixParseFn> prefixParseFns_;
    std::unordered_map<TokenType, InfixParseFn> infixParseFns_;
    bool isReplMode_ = false;
};

} // namespace darix
