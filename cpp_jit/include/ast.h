#pragma once

#include "token.h"
#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <optional>

namespace darix {

// Forward declarations for visitor pattern
class ExprVisitor;
class StmtVisitor;

// ============================================================================
// EXPRESSIONS - Using variant-based AST for better cache locality
// ============================================================================

// Expression types using tagged union for memory efficiency
class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) const = 0;
    
    // Polymorphic cast helpers
    template<typename T>
    [[nodiscard]] const T* as() const noexcept {
        return dynamic_cast<const T*>(this);
    }
};

// Literal value that can hold different types
struct LiteralValue {
    std::variant<std::monostate, bool, long long, double, std::string> value;
    
    LiteralValue() : value(std::monostate{}) {}
    explicit LiteralValue(bool b) : value(b) {}
    explicit LiteralValue(long long i) : value(i) {}
    explicit LiteralValue(double d) : value(d) {}
    explicit LiteralValue(std::string s) : value(std::move(s)) {}
    explicit LiteralValue(const char* s) : value(std::string(s)) {}
    
    [[nodiscard]] bool isNull() const noexcept { return std::holds_alternative<std::monostate>(value); }
    [[nodiscard]] bool isBool() const noexcept { return std::holds_alternative<bool>(value); }
    [[nodiscard]] bool isInt() const noexcept { return std::holds_alternative<long long>(value); }
    [[nodiscard]] bool isFloat() const noexcept { return std::holds_alternative<double>(value); }
    [[nodiscard]] bool isString() const noexcept { return std::holds_alternative<std::string>(value); }
    
    [[nodiscard]] bool getBool() const { return std::get<bool>(value); }
    [[nodiscard]] long long getInt() const { return std::get<long long>(value); }
    [[nodiscard]] double getFloat() const { return std::get<double>(value); }
    [[nodiscard]] const std::string& getString() const { return std::get<std::string>(value); }
};

class LiteralExpr final : public Expr {
public:
    explicit LiteralExpr(LiteralValue val) : value_(std::move(val)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const LiteralValue& value() const noexcept { return value_; }
private:
    LiteralValue value_;
};

class VariableExpr final : public Expr {
public:
    explicit VariableExpr(std::string name, uint32_t line = 0, uint32_t col = 0)
        : name_(std::move(name)), line_(line), col_(col) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] uint32_t line() const noexcept { return line_; }
    [[nodiscard]] uint32_t column() const noexcept { return col_; }
private:
    std::string name_;
    uint32_t line_ = 0;
    uint32_t col_ = 0;
};

class BinaryExpr final : public Expr {
public:
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left_(std::move(left)), op_(op), right_(std::move(right)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const Expr& left() const noexcept { return *left_; }
    [[nodiscard]] const Expr& right() const noexcept { return *right_; }
    [[nodiscard]] const Token& op() const noexcept { return op_; }
    [[nodiscard]] std::unique_ptr<Expr>& leftPtr() noexcept { return left_; }
    [[nodiscard]] std::unique_ptr<Expr>& rightPtr() noexcept { return right_; }
private:
    std::unique_ptr<Expr> left_;
    Token op_;
    std::unique_ptr<Expr> right_;
};

class UnaryExpr final : public Expr {
public:
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op_(op), right_(std::move(right)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const Token& op() const noexcept { return op_; }
    [[nodiscard]] const Expr& right() const noexcept { return *right_; }
    [[nodiscard]] std::unique_ptr<Expr>& rightPtr() noexcept { return right_; }
private:
    Token op_;
    std::unique_ptr<Expr> right_;
};

class CallExpr final : public Expr {
public:
    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee_(std::move(callee)), arguments_(std::move(arguments)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const Expr& callee() const noexcept { return *callee_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& arguments() const noexcept { return arguments_; }
    [[nodiscard]] std::unique_ptr<Expr>& calleePtr() noexcept { return callee_; }
    [[nodiscard]] size_t arity() const noexcept { return arguments_.size(); }
private:
    std::unique_ptr<Expr> callee_;
    std::vector<std::unique_ptr<Expr>> arguments_;
};

class ArrayExpr final : public Expr {
public:
    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements_(std::move(elements)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& elements() const noexcept { return elements_; }
    [[nodiscard]] size_t size() const noexcept { return elements_.size(); }
private:
    std::vector<std::unique_ptr<Expr>> elements_;
};

class MapExpr final : public Expr {
public:
    struct KeyValue {
        std::unique_ptr<Expr> key;
        std::unique_ptr<Expr> value;
    };
    explicit MapExpr(std::vector<KeyValue> pairs)
        : pairs_(std::move(pairs)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const std::vector<KeyValue>& pairs() const noexcept { return pairs_; }
private:
    std::vector<KeyValue> pairs_;
};

class MemberExpr final : public Expr {
public:
    MemberExpr(std::unique_ptr<Expr> object, std::string property)
        : object_(std::move(object)), property_(std::move(property)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const Expr& object() const noexcept { return *object_; }
    [[nodiscard]] const std::string& property() const noexcept { return property_; }
    [[nodiscard]] std::unique_ptr<Expr>& objectPtr() noexcept { return object_; }
private:
    std::unique_ptr<Expr> object_;
    std::string property_;
};

class IndexExpr final : public Expr {
public:
    IndexExpr(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index)
        : array_(std::move(array)), index_(std::move(index)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const Expr& array() const noexcept { return *array_; }
    [[nodiscard]] const Expr& index() const noexcept { return *index_; }
    [[nodiscard]] std::unique_ptr<Expr>& arrayPtr() noexcept { return array_; }
    [[nodiscard]] std::unique_ptr<Expr>& indexPtr() noexcept { return index_; }
private:
    std::unique_ptr<Expr> array_;
    std::unique_ptr<Expr> index_;
};

class AssignExpr final : public Expr {
public:
    AssignExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
        : target_(std::move(target)), value_(std::move(value)) {}
    void accept(ExprVisitor& visitor) const override;
    [[nodiscard]] const Expr& target() const noexcept { return *target_; }
    [[nodiscard]] const Expr& value() const noexcept { return *value_; }
    [[nodiscard]] std::unique_ptr<Expr>& targetPtr() noexcept { return target_; }
    [[nodiscard]] std::unique_ptr<Expr>& valuePtr() noexcept { return value_; }
private:
    std::unique_ptr<Expr> target_;
    std::unique_ptr<Expr> value_;
};

// ============================================================================
// STATEMENTS
// ============================================================================

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) const = 0;
    
    template<typename T>
    [[nodiscard]] const T* as() const noexcept {
        return dynamic_cast<const T*>(this);
    }
};

class ExprStmt final : public Stmt {
public:
    explicit ExprStmt(std::unique_ptr<Expr> expression)
        : expression_(std::move(expression)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const Expr& expression() const noexcept { return *expression_; }
private:
    std::unique_ptr<Expr> expression_;
};

class VarDecl final : public Stmt {
public:
    VarDecl(std::string name, std::unique_ptr<Expr> initializer)
        : name_(std::move(name)), initializer_(std::move(initializer)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const Expr* initializer() const noexcept { return initializer_.get(); }
    [[nodiscard]] std::unique_ptr<Expr>& initializerPtr() noexcept { return initializer_; }
private:
    std::string name_;
    std::unique_ptr<Expr> initializer_;
};

class FuncDecl final : public Stmt {
public:
    FuncDecl(std::string name, std::vector<std::string> params, 
             std::vector<std::unique_ptr<Stmt>> body, uint32_t line = 0)
        : name_(std::move(name)), params_(std::move(params)), 
          body_(std::move(body)), line_(line) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::vector<std::string>& params() const noexcept { return params_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& body() const noexcept { return body_; }
    [[nodiscard]] size_t paramCount() const noexcept { return params_.size(); }
    [[nodiscard]] uint32_t line() const noexcept { return line_; }
private:
    std::string name_;
    std::vector<std::string> params_;
    std::vector<std::unique_ptr<Stmt>> body_;
    uint32_t line_ = 0;
};

class ClassDecl final : public Stmt {
public:
    ClassDecl(std::string name, std::vector<std::unique_ptr<FuncDecl>> methods)
        : name_(std::move(name)), methods_(std::move(methods)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::vector<std::unique_ptr<FuncDecl>>& methods() const noexcept { return methods_; }
private:
    std::string name_;
    std::vector<std::unique_ptr<FuncDecl>> methods_;
};

class ReturnStmt final : public Stmt {
public:
    explicit ReturnStmt(std::unique_ptr<Expr> value)
        : value_(std::move(value)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const Expr* value() const noexcept { return value_.get(); }
    [[nodiscard]] std::unique_ptr<Expr>& valuePtr() noexcept { return value_; }
private:
    std::unique_ptr<Expr> value_;
};

class IfStmt final : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> condition, 
           std::vector<std::unique_ptr<Stmt>> thenBranch,
           std::vector<std::unique_ptr<Stmt>> elseBranch)
        : condition_(std::move(condition)), thenBranch_(std::move(thenBranch)),
          elseBranch_(std::move(elseBranch)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const Expr& condition() const noexcept { return *condition_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& thenBranch() const noexcept { return thenBranch_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& elseBranch() const noexcept { return elseBranch_; }
    [[nodiscard]] std::unique_ptr<Expr>& conditionPtr() noexcept { return condition_; }
private:
    std::unique_ptr<Expr> condition_;
    std::vector<std::unique_ptr<Stmt>> thenBranch_;
    std::vector<std::unique_ptr<Stmt>> elseBranch_;
};

class WhileStmt final : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
        : condition_(std::move(condition)), body_(std::move(body)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const Expr& condition() const noexcept { return *condition_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& body() const noexcept { return body_; }
    [[nodiscard]] std::unique_ptr<Expr>& conditionPtr() noexcept { return condition_; }
private:
    std::unique_ptr<Expr> condition_;
    std::vector<std::unique_ptr<Stmt>> body_;
};

class ForStmt final : public Stmt {
public:
    ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition,
            std::unique_ptr<Expr> increment, std::vector<std::unique_ptr<Stmt>> body)
        : initializer_(std::move(initializer)), condition_(std::move(condition)),
          increment_(std::move(increment)), body_(std::move(body)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const Stmt* initializer() const noexcept { return initializer_.get(); }
    [[nodiscard]] const Expr* condition() const noexcept { return condition_.get(); }
    [[nodiscard]] const Expr* increment() const noexcept { return increment_.get(); }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& body() const noexcept { return body_; }
private:
    std::unique_ptr<Stmt> initializer_;
    std::unique_ptr<Expr> condition_;
    std::unique_ptr<Expr> increment_;
    std::vector<std::unique_ptr<Stmt>> body_;
};

class TryStmt final : public Stmt {
public:
    TryStmt(std::vector<std::unique_ptr<Stmt>> tryBody, std::string catchVar,
            std::vector<std::unique_ptr<Stmt>> catchBody, 
            std::vector<std::unique_ptr<Stmt>> finallyBody)
        : tryBody_(std::move(tryBody)), catchVar_(std::move(catchVar)),
          catchBody_(std::move(catchBody)), finallyBody_(std::move(finallyBody)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& tryBody() const noexcept { return tryBody_; }
    [[nodiscard]] const std::string& catchVar() const noexcept { return catchVar_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& catchBody() const noexcept { return catchBody_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& finallyBody() const noexcept { return finallyBody_; }
private:
    std::vector<std::unique_ptr<Stmt>> tryBody_;
    std::string catchVar_;
    std::vector<std::unique_ptr<Stmt>> catchBody_;
    std::vector<std::unique_ptr<Stmt>> finallyBody_;
};

class BlockStmt final : public Stmt {
public:
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements_(std::move(statements)) {}
    void accept(StmtVisitor& visitor) const override;
    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& statements() const noexcept { return statements_; }
private:
    std::vector<std::unique_ptr<Stmt>> statements_;
};

// ============================================================================
// VISITOR INTERFACES
// ============================================================================

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit(const LiteralExpr& expr) = 0;
    virtual void visit(const VariableExpr& expr) = 0;
    virtual void visit(const BinaryExpr& expr) = 0;
    virtual void visit(const UnaryExpr& expr) = 0;
    virtual void visit(const CallExpr& expr) = 0;
    virtual void visit(const ArrayExpr& expr) = 0;
    virtual void visit(const MapExpr& expr) = 0;
    virtual void visit(const MemberExpr& expr) = 0;
    virtual void visit(const IndexExpr& expr) = 0;
    virtual void visit(const AssignExpr& expr) = 0;
};

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(const ExprStmt& stmt) = 0;
    virtual void visit(const VarDecl& stmt) = 0;
    virtual void visit(const FuncDecl& stmt) = 0;
    virtual void visit(const ClassDecl& stmt) = 0;
    virtual void visit(const ReturnStmt& stmt) = 0;
    virtual void visit(const IfStmt& stmt) = 0;
    virtual void visit(const WhileStmt& stmt) = 0;
    virtual void visit(const ForStmt& stmt) = 0;
    virtual void visit(const TryStmt& stmt) = 0;
    virtual void visit(const BlockStmt& stmt) = 0;
};

} // namespace darix
