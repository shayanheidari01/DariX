#pragma once

#include <memory>
#include <vector>
#include <string>
#include "token.h"

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
    explicit LiteralExpr(std::string value) : value_(std::move(value)) {}
    std::string toString() const override { return value_; }
private:
    std::string value_;
};

class NumberExpr : public Expr {
public:
    explicit NumberExpr(double value) : value_(value) {}
    std::string toString() const override { return std::to_string(value_); }
private:
    double value_;
};

class StringExpr : public Expr {
public:
    explicit StringExpr(std::string value) : value_(std::move(value)) {}
    std::string toString() const override { return "\"" + value_ + "\""; }
private:
    std::string value_;
};

class BoolExpr : public Expr {
public:
    explicit BoolExpr(bool value) : value_(value) {}
    std::string toString() const override { return value_ ? "true" : "false"; }
private:
    bool value_;
};

class NullExpr : public Expr {
public:
    std::string toString() const override { return "null"; }
};

// Variable reference
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
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left_(std::move(left)), op_(op), right_(std::move(right)) {}
    std::string toString() const override {
        return "(" + left_->toString() + " " + op_.lexeme + " " + right_->toString() + ")";
    }
private:
    std::unique_ptr<Expr> left_;
    Token op_;
    std::unique_ptr<Expr> right_;
};

// Unary operations
class UnaryExpr : public Expr {
public:
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op_(op), right_(std::move(right)) {}
    std::string toString() const override {
        return "(" + op_.lexeme + right_->toString() + ")";
    }
private:
    Token op_;
    std::unique_ptr<Expr> right_;
};

// Function call
class CallExpr : public Expr {
public:
    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee_(std::move(callee)), arguments_(std::move(arguments)) {}
    std::string toString() const override;
private:
    std::unique_ptr<Expr> callee_;
    std::vector<std::unique_ptr<Expr>> arguments_;
};

// Array literal
class ArrayExpr : public Expr {
public:
    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements_(std::move(elements)) {}
    std::string toString() const override;
private:
    std::vector<std::unique_ptr<Expr>> elements_;
};

// Map literal
class MapExpr : public Expr {
public:
    MapExpr(std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs)
        : pairs_(std::move(pairs)) {}
    std::string toString() const override;
private:
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs_;
};

// Member access (obj.prop)
class MemberExpr : public Expr {
public:
    MemberExpr(std::unique_ptr<Expr> object, std::string property)
        : object_(std::move(object)), property_(std::move(property)) {}
    std::string toString() const override {
        return object_->toString() + "." + property_;
    }
private:
    std::unique_ptr<Expr> object_;
    std::string property_;
};

// Index access (array[index])
class IndexExpr : public Expr {
public:
    IndexExpr(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index)
        : array_(std::move(array)), index_(std::move(index)) {}
    std::string toString() const override {
        return array_->toString() + "[" + index_->toString() + "]";
    }
private:
    std::unique_ptr<Expr> array_;
    std::unique_ptr<Expr> index_;
};

// Assignment
class AssignExpr : public Expr {
public:
    AssignExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
        : target_(std::move(target)), value_(std::move(value)) {}
    std::string toString() const override {
        return target_->toString() + " = " + value_->toString();
    }
private:
    std::unique_ptr<Expr> target_;
    std::unique_ptr<Expr> value_;
};

// Statements

// Expression statement
class ExprStmt : public Stmt {
public:
    explicit ExprStmt(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression)) {}
    std::string toString() const override { return expression_->toString() + ";"; }
private:
    std::unique_ptr<Expr> expression_;
};

// Variable declaration
class VarDecl : public Stmt {
public:
    VarDecl(std::string name, std::unique_ptr<Expr> initializer)
        : name_(std::move(name)), initializer_(std::move(initializer)) {}
    std::string toString() const override {
        return "var " + name_ + " = " + (initializer_ ? initializer_->toString() : "null") + ";";
    }
private:
    std::string name_;
    std::unique_ptr<Expr> initializer_;
};

// Function declaration
class FuncDecl : public Stmt {
public:
    FuncDecl(std::string name, std::vector<std::string> params, std::vector<std::unique_ptr<Stmt>> body)
        : name_(std::move(name)), params_(std::move(params)), body_(std::move(body)) {}
    std::string toString() const override;
private:
    std::string name_;
    std::vector<std::string> params_;
    std::vector<std::unique_ptr<Stmt>> body_;
};

// Class declaration
class ClassDecl : public Stmt {
public:
    ClassDecl(std::string name, std::vector<std::unique_ptr<FuncDecl>> methods)
        : name_(std::move(name)), methods_(std::move(methods)) {}
    std::string toString() const override;
private:
    std::string name_;
    std::vector<std::unique_ptr<FuncDecl>> methods_;
};

// Return statement
class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(std::unique_ptr<Expr> value) : value_(std::move(value)) {}
    std::string toString() const override {
        return "return" + (value_ ? " " + value_->toString() : "") + ";";
    }
private:
    std::unique_ptr<Expr> value_;
};

// If statement
class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> thenBranch,
           std::vector<std::unique_ptr<Stmt>> elseBranch)
        : condition_(std::move(condition)), thenBranch_(std::move(thenBranch)),
          elseBranch_(std::move(elseBranch)) {}
    std::string toString() const override;
private:
    std::unique_ptr<Expr> condition_;
    std::vector<std::unique_ptr<Stmt>> thenBranch_;
    std::vector<std::unique_ptr<Stmt>> elseBranch_;
};

// While loop
class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
        : condition_(std::move(condition)), body_(std::move(body)) {}
    std::string toString() const override;
private:
    std::unique_ptr<Expr> condition_;
    std::vector<std::unique_ptr<Stmt>> body_;
};

// For loop
class ForStmt : public Stmt {
public:
    ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition,
            std::unique_ptr<Expr> increment, std::vector<std::unique_ptr<Stmt>> body)
        : initializer_(std::move(initializer)), condition_(std::move(condition)),
          increment_(std::move(increment)), body_(std::move(body)) {}
    std::string toString() const override;
private:
    std::unique_ptr<Stmt> initializer_;
    std::unique_ptr<Expr> condition_;
    std::unique_ptr<Expr> increment_;
    std::vector<std::unique_ptr<Stmt>> body_;
};

// Try-catch-finally
class TryStmt : public Stmt {
public:
    TryStmt(std::vector<std::unique_ptr<Stmt>> tryBody, std::string catchVar,
            std::vector<std::unique_ptr<Stmt>> catchBody, std::vector<std::unique_ptr<Stmt>> finallyBody)
        : tryBody_(std::move(tryBody)), catchVar_(std::move(catchVar)),
          catchBody_(std::move(catchBody)), finallyBody_(std::move(finallyBody)) {}
    std::string toString() const override;
private:
    std::vector<std::unique_ptr<Stmt>> tryBody_;
    std::string catchVar_;
    std::vector<std::unique_ptr<Stmt>> catchBody_;
    std::vector<std::unique_ptr<Stmt>> finallyBody_;
};

// Block statement
class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements_(std::move(statements)) {}
    std::string toString() const override;
private:
    std::vector<std::unique_ptr<Stmt>> statements_;
};

} // namespace darix
