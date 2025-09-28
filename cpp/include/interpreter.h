#pragma once

#include "ast.h"
#include "environment.h"
#include "object.h"
#include <memory>
#include <unordered_map>

namespace darix {

class Interpreter {
public:
    Interpreter();

    std::shared_ptr<Object> interpret(const std::vector<std::unique_ptr<Stmt>>& statements);

    // Expression evaluation
    std::shared_ptr<Object> evaluate(const Expr& expr);
    std::shared_ptr<Object> evaluateLiteral(const LiteralExpr& expr);
    std::shared_ptr<Object> evaluateNumber(const NumberExpr& expr);
    std::shared_ptr<Object> evaluateString(const StringExpr& expr);
    std::shared_ptr<Object> evaluateBool(const BoolExpr& expr);
    std::shared_ptr<Object> evaluateNull(const NullExpr& expr);
    std::shared_ptr<Object> evaluateVariable(const VariableExpr& expr);
    std::shared_ptr<Object> evaluateBinary(const BinaryExpr& expr);
    std::shared_ptr<Object> evaluateUnary(const UnaryExpr& expr);
    std::shared_ptr<Object> evaluateCall(const CallExpr& expr);
    std::shared_ptr<Object> evaluateArray(const ArrayExpr& expr);
    std::shared_ptr<Object> evaluateMap(const MapExpr& expr);
    std::shared_ptr<Object> evaluateMember(const MemberExpr& expr);
    std::shared_ptr<Object> evaluateIndex(const IndexExpr& expr);
    std::shared_ptr<Object> evaluateAssign(const AssignExpr& expr);

    // Statement execution
    void execute(const Stmt& stmt);
    void executeExprStmt(const ExprStmt& stmt);
    void executeVarDecl(const VarDecl& stmt);
    void executeFuncDecl(const FuncDecl& stmt);
    void executeClassDecl(const ClassDecl& stmt);
    void executeIfStmt(const IfStmt& stmt);
    void executeWhileStmt(const WhileStmt& stmt);
    void executeForStmt(const ForStmt& stmt);
    void executeReturnStmt(const ReturnStmt& stmt);
    void executeTryStmt(const TryStmt& stmt);
    void executeBlockStmt(const BlockStmt& stmt);

    // Helper methods
    bool isTruthy(const std::shared_ptr<Object>& object) const;
    bool isEqual(const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b) const;
    void checkNumberOperand(const Token& op, const std::shared_ptr<Object>& operand) const;
    void checkNumberOperands(const Token& op, const std::shared_ptr<Object>& left,
                           const std::shared_ptr<Object>& right) const;
    std::shared_ptr<Object> callFunction(const std::shared_ptr<FunctionObject>& function,
                                        const std::vector<std::shared_ptr<Object>>& arguments);

private:
    std::shared_ptr<Environment> environment_;
    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Object> lastResult_;
};

} // namespace darix
