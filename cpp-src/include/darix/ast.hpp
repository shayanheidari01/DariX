#pragma once

#include "darix/token.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace darix {

// Forward declarations
struct Statement;
struct Expression;
struct BlockStatement;
struct Identifier;
struct StringLiteral;

using StatementPtr = std::shared_ptr<Statement>;
using ExpressionPtr = std::shared_ptr<Expression>;
using BlockStatementPtr = std::shared_ptr<BlockStatement>;
using IdentifierPtr = std::shared_ptr<Identifier>;

// Node type tags for fast dispatch
enum class NodeType : uint8_t {
    PROGRAM, EXPRESSION_STATEMENT, BLOCK_STATEMENT, STANDALONE_BLOCK,
    LET_STATEMENT, ASSIGN_STATEMENT, RETURN_STATEMENT,
    WHILE_STATEMENT, FOR_STATEMENT, BREAK_STATEMENT, CONTINUE_STATEMENT,
    FUNCTION_DECLARATION, CLASS_DECLARATION,
    TRY_STATEMENT, THROW_STATEMENT, IMPORT_STATEMENT,
    DEL_STATEMENT, ASSERT_STATEMENT, PASS_STATEMENT,
    GLOBAL_STATEMENT, NONLOCAL_STATEMENT, WITH_STATEMENT,
    IDENTIFIER, INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL,
    BOOLEAN_LITERAL, NULL_LITERAL,
    PREFIX_EXPRESSION, INFIX_EXPRESSION, IF_EXPRESSION,
    FUNCTION_LITERAL, CALL_EXPRESSION, ARRAY_LITERAL, MAP_LITERAL,
    INDEX_EXPRESSION, MEMBER_EXPRESSION, WHILE_EXPRESSION,
    IN_EXPRESSION, IS_EXPRESSION, LAMBDA_EXPRESSION,
    YIELD_EXPRESSION, EXCEPTION_EXPRESSION
};

// Base interfaces
struct Node {
    virtual ~Node() = default;
    virtual std::string tokenLiteral() const = 0;
    virtual std::string inspect() const = 0;
    NodeType tag = NodeType::PROGRAM; // type tag for fast dispatch
};

struct Statement : virtual Node {
    virtual void statementNode() = 0;
};

struct Expression : virtual Node {
    virtual void expressionNode() = 0;
};

// Helper functions
std::string statementsString(const std::vector<StatementPtr>& stmts);
std::string expressionString(const ExpressionPtr& expr);
std::string identifierString(const IdentifierPtr& ident);
std::string blockString(const BlockStatementPtr& block);

// ============ Statements ============

struct Program : Statement {
    std::vector<StatementPtr> statements;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ImportStatement : Statement {
    Token token;
    std::shared_ptr<StringLiteral> path;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct LetStatement : Statement {
    Token token;
    IdentifierPtr name;
    ExpressionPtr value;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct AssignStatement : Statement {
    Token token;
    ExpressionPtr target;
    ExpressionPtr value;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ReturnStatement : Statement {
    Token token;
    ExpressionPtr returnValue;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ExpressionStatement : Statement {
    Token token;
    ExpressionPtr expression;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct BlockStatement : Statement, Expression {
    Token token;
    std::vector<StatementPtr> statements;
    void statementNode() override {}
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct StandaloneBlockStatement : Statement {
    Token token;
    BlockStatementPtr block;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct BreakStatement : Statement {
    Token token;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ContinueStatement : Statement {
    Token token;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct WhileStatement : Statement {
    Token token;
    ExpressionPtr condition;
    BlockStatementPtr body;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ForStatement : Statement {
    Token token;
    StatementPtr init;
    ExpressionPtr condition;
    StatementPtr post;
    BlockStatementPtr body;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct FunctionDeclaration : Statement {
    Token token;
    IdentifierPtr name;
    std::vector<IdentifierPtr> parameters;
    BlockStatementPtr body;
    std::vector<ExpressionPtr> decorators;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ClassDeclaration : Statement {
    Token token;
    IdentifierPtr name;
    BlockStatementPtr body;
    std::vector<ExpressionPtr> decorators;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ThrowStatement : Statement {
    Token token;
    ExpressionPtr exception;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct CatchClause {
    Token token;
    IdentifierPtr exceptionType;
    IdentifierPtr variable;
    BlockStatementPtr catchBlock;
    std::string inspect() const;
};

struct TryStatement : Statement {
    Token token;
    BlockStatementPtr tryBlock;
    std::vector<std::shared_ptr<CatchClause>> catchClauses;
    BlockStatementPtr finallyBlock;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct DelStatement : Statement {
    Token token;
    ExpressionPtr target;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct AssertStatement : Statement {
    Token token;
    ExpressionPtr condition;
    ExpressionPtr message;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct PassStatement : Statement {
    Token token;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct GlobalStatement : Statement {
    Token token;
    std::vector<IdentifierPtr> names;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct NonlocalStatement : Statement {
    Token token;
    std::vector<IdentifierPtr> names;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct WithStatement : Statement {
    Token token;
    ExpressionPtr context;
    IdentifierPtr variable;
    BlockStatementPtr body;
    void statementNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

// ============ Expressions ============

struct Identifier : Expression {
    Token token;
    std::string value;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct IntegerLiteral : Expression {
    Token token;
    int64_t value = 0;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct FloatLiteral : Expression {
    Token token;
    double value = 0.0;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct StringLiteral : Expression {
    Token token;
    std::string value;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct BooleanLiteral : Expression {
    Token token;
    bool value = false;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct NullLiteral : Expression {
    Token token;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct AssignExpression : Expression {
    Token token;
    ExpressionPtr name;
    ExpressionPtr value;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct PrefixExpression : Expression {
    Token token;
    std::string op;
    ExpressionPtr right;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct InfixExpression : Expression {
    Token token;
    ExpressionPtr left;
    std::string op;
    ExpressionPtr right;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct IfExpression : Expression {
    Token token;
    ExpressionPtr condition;
    BlockStatementPtr consequence;
    ExpressionPtr alternative;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct FunctionLiteral : Expression {
    Token token;
    std::vector<IdentifierPtr> parameters;
    BlockStatementPtr body;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct CallExpression : Expression {
    Token token;
    ExpressionPtr function;
    std::vector<ExpressionPtr> arguments;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ArrayLiteral : Expression {
    Token token;
    std::vector<ExpressionPtr> elements;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct MapLiteral : Expression {
    Token token;
    std::vector<std::pair<ExpressionPtr, ExpressionPtr>> pairs;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct IndexExpression : Expression {
    Token token;
    ExpressionPtr left;
    ExpressionPtr index;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct MemberExpression : Expression {
    Token token;
    ExpressionPtr left;
    IdentifierPtr property;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct WhileExpression : Expression {
    Token token;
    ExpressionPtr condition;
    BlockStatementPtr body;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct InExpression : Expression {
    Token token;
    ExpressionPtr left;
    ExpressionPtr right;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct IsExpression : Expression {
    Token token;
    ExpressionPtr left;
    ExpressionPtr right;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct LambdaExpression : Expression {
    Token token;
    std::vector<IdentifierPtr> parameters;
    ExpressionPtr body;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct YieldExpression : Expression {
    Token token;
    ExpressionPtr value;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

struct ExceptionExpression : Expression {
    Token token;
    IdentifierPtr type;
    ExpressionPtr message;
    void expressionNode() override {}
    std::string tokenLiteral() const override;
    std::string inspect() const override;
};

} // namespace darix
