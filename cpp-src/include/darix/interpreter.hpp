#pragma once

#include "darix/ast.hpp"
#include "darix/object.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace darix {

class Interpreter {
public:
    Interpreter();

    ObjectPtr interpret(Program* program);
    std::shared_ptr<Environment> getEnvironment() { return env_; }

private:
    ObjectPtr eval(Node* node, std::shared_ptr<Environment> env);

    // Statement evaluation
    ObjectPtr evalProgram(Program* program, std::shared_ptr<Environment> env);
    ObjectPtr evalBlockStatement(BlockStatement* block, std::shared_ptr<Environment> env);
    ObjectPtr evalBlockStatementWithScoping(BlockStatement* block, std::shared_ptr<Environment> env, bool createNewScope);
    ObjectPtr evalAssignStatement(AssignStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalWhile(WhileStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalFor(ForStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalTryStatement(TryStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalThrowStatement(ThrowStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalClassDeclaration(ClassDeclaration* node, std::shared_ptr<Environment> env);
    ObjectPtr evalImportStatement(ImportStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalDelStatement(DelStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalAssertStatement(AssertStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalWithStatement(WithStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalGlobalStatement(GlobalStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalNonlocalStatement(NonlocalStatement* node, std::shared_ptr<Environment> env);
    ObjectPtr evalMapLiteral(MapLiteral* node, std::shared_ptr<Environment> env);

    // Expression evaluation
    ObjectPtr evalInfixExpression(const std::string& op, ObjectPtr left, ObjectPtr right);
    ObjectPtr evalPrefixExpression(const std::string& op, ObjectPtr right);
    ObjectPtr evalIfExpression(IfExpression* node, std::shared_ptr<Environment> env);
    ObjectPtr evalIdentifier(Identifier* node, std::shared_ptr<Environment> env);
    std::vector<ObjectPtr> evalExpressions(const std::vector<ExpressionPtr>& exps, std::shared_ptr<Environment> env);
    ObjectPtr evalIndexExpression(ObjectPtr left, ObjectPtr index);
    ObjectPtr evalIndexAssignment(IndexExpression* idx, ObjectPtr val, std::shared_ptr<Environment> env);
    ObjectPtr evalAssignExpression(AssignExpression* node, std::shared_ptr<Environment> env);
    ObjectPtr evalMemberExpression(MemberExpression* node, std::shared_ptr<Environment> env);
    ObjectPtr evalMemberAssignment(MemberExpression* memberExpr, ObjectPtr val, std::shared_ptr<Environment> env);
    ObjectPtr evalInExpression(InExpression* node, std::shared_ptr<Environment> env);
    ObjectPtr evalIsExpression(IsExpression* node, std::shared_ptr<Environment> env);

    // Function application
    ObjectPtr applyFunction(ObjectPtr fn, const std::vector<ObjectPtr>& args);
    ObjectPtr applyDecorators(const std::vector<ExpressionPtr>& decorators, ObjectPtr fn, std::shared_ptr<Environment> env);

    // Helpers
    void initBuiltins();
    ObjectPtr pushFrame(const std::string& fnName, const Position& pos, const std::string& ctx);
    void popFrame();
    std::vector<StackFrame> currentStackTrace() const;

    static ObjectPtr builtinError(const std::string& name, const std::string& format);
    static bool isError(ObjectPtr obj);
    static bool isSignal(ObjectPtr obj);

    std::shared_ptr<Environment> env_;
    std::unordered_map<std::string, std::shared_ptr<Builtin>> builtins_;
    std::unordered_map<std::string, ObjectPtr> loadedModules_;
    std::vector<StackFrame> callStack_;
    std::string currentFile_;
};

} // namespace darix
