#pragma once

#include "token.h"
#include <memory>
#include <vector>
#include <string>
#include <sstream>

namespace darix {

// Forward declarations
class Expr;
class Stmt;
class ClassDecl;
class FuncDecl;

// Expression base class
class Expr {
public:
    virtual ~Expr() = default;
    virtual std::string toString() const = 0;
};

// Statement base class
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual std::string toString() const = 0;
};

// Literal expressions
class LiteralExpr : public Expr {
public:
    explicit LiteralExpr(std::any value) : value_(value) {}
    std::string toString() const override;
private:
    std::any value_;
};

class VariableExpr : public Expr {
public:
    explicit VariableExpr(std::string name) : name_(std::move(name)) {}
    std::string toString() const override { return name_; }
private:
    std::string name_;
};

// Binary operations
class BinaryExpr : public Expr {
public:
    BinaryExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left_(std::move(left)), op_(op), right_(std::move(right)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> left_;
    Token op_;
    std::shared_ptr<Expr> right_;
};

// Unary operations
class UnaryExpr : public Expr {
public:
    UnaryExpr(Token op, std::shared_ptr<Expr> right)
        : op_(op), right_(std::move(right)) {}
    std::string toString() const override;
private:
    Token op_;
    std::shared_ptr<Expr> right_;
};

// Function call
class CallExpr : public Expr {
public:
    CallExpr(std::shared_ptr<Expr> callee, std::vector<std::shared_ptr<Expr>> arguments)
        : callee_(std::move(callee)), arguments_(std::move(arguments)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> callee_;
    std::vector<std::shared_ptr<Expr>> arguments_;
};

// Array literal
class ArrayExpr : public Expr {
public:
    explicit ArrayExpr(std::vector<std::shared_ptr<Expr>> elements)
        : elements_(std::move(elements)) {}
    std::string toString() const override;
private:
    std::vector<std::shared_ptr<Expr>> elements_;
};

// Map literal
class MapExpr : public Expr {
public:
    using KeyValue = std::pair<std::shared_ptr<Expr>, std::shared_ptr<Expr>>;
    explicit MapExpr(std::vector<KeyValue> pairs)
        : pairs_(std::move(pairs)) {}
    std::string toString() const override;
private:
    std::vector<KeyValue> pairs_;
};

// Member access (obj.prop)
class MemberExpr : public Expr {
public:
    MemberExpr(std::shared_ptr<Expr> object, std::string property)
        : object_(std::move(object)), property_(std::move(property)) {}
    std::string toString() const override {
        return object_->toString() + "." + property_;
    }
private:
    std::shared_ptr<Expr> object_;
    std::string property_;
};

// Index access (array[index])
class IndexExpr : public Expr {
public:
    IndexExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index)
        : object_(std::move(object)), index_(std::move(index)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> object_;
    std::shared_ptr<Expr> index_;
};

// Assignment
class AssignExpr : public Expr {
public:
    AssignExpr(std::string name, std::shared_ptr<Expr> value)
        : name_(std::move(name)), value_(std::move(value)) {}
    std::string toString() const override;
private:
    std::string name_;
    std::shared_ptr<Expr> value_;
};

// Logical operations (and, or)
class LogicalExpr : public Expr {
public:
    LogicalExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left_(std::move(left)), op_(op), right_(std::move(right)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> left_;
    Token op_;
    std::shared_ptr<Expr> right_;
};

// Statements
class ExpressionStmt : public Stmt {
public:
    explicit ExpressionStmt(std::shared_ptr<Expr> expr) : expr_(std::move(expr)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> expr_;
};

class PrintStmt : public Stmt {
public:
    explicit PrintStmt(std::shared_ptr<Expr> expr) : expr_(std::move(expr)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> expr_;
};

class VarDecl : public Stmt {
public:
    VarDecl(std::string name, std::shared_ptr<Expr> initializer)
        : name_(std::move(name)), initializer_(std::move(initializer)) {}
    std::string toString() const override;
private:
    std::string name_;
    std::shared_ptr<Expr> initializer_;
};

class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<std::shared_ptr<Stmt>> statements)
        : statements_(std::move(statements)) {}
    std::string toString() const override;
private:
    std::vector<std::shared_ptr<Stmt>> statements_;
};

class IfStmt : public Stmt {
public:
    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch,
           std::shared_ptr<Stmt> elseBranch = nullptr)
        : condition_(std::move(condition)), thenBranch_(std::move(thenBranch)),
          elseBranch_(std::move(elseBranch)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> condition_;
    std::shared_ptr<Stmt> thenBranch_;
    std::shared_ptr<Stmt> elseBranch_;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
        : condition_(std::move(condition)), body_(std::move(body)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> condition_;
    std::shared_ptr<Stmt> body_;
};

class ForStmt : public Stmt {
public:
    ForStmt(std::shared_ptr<Stmt> initializer, std::shared_ptr<Expr> condition,
            std::shared_ptr<Expr> increment, std::shared_ptr<Stmt> body)
        : initializer_(std::move(initializer)), condition_(std::move(condition)),
          increment_(std::move(increment)), body_(std::move(body)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Stmt> initializer_;
    std::shared_ptr<Expr> condition_;
    std::shared_ptr<Expr> increment_;
    std::shared_ptr<Stmt> body_;
};

class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(std::shared_ptr<Expr> value = nullptr)
        : value_(std::move(value)) {}
    std::string toString() const override;
private:
    std::shared_ptr<Expr> value_;
};

class BreakStmt : public Stmt {
public:
    std::string toString() const override { return "break;"; }
};

class ContinueStmt : public Stmt {
public:
    std::string toString() const override { return "continue;"; }
};

class FuncDecl : public Stmt {
public:
    FuncDecl(std::string name, std::vector<std::string> params,
             std::shared_ptr<BlockStmt> body)
        : name_(std::move(name)), params_(std::move(params)), body_(std::move(body)) {}
    std::string toString() const override;
private:
    std::string name_;
    std::vector<std::string> params_;
    std::shared_ptr<BlockStmt> body_;
};

class ClassDecl : public Stmt {
public:
    ClassDecl(std::string name, std::vector<std::shared_ptr<FuncDecl>> methods)
        : name_(std::move(name)), methods_(std::move(methods)) {}
    std::string toString() const override;
private:
    std::string name_;
    std::vector<std::shared_ptr<FuncDecl>> methods_;
};

} // namespace darix
