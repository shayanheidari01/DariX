#include "interpreter.h"
#include "builtins.h"
#include <iostream>
#include <sstream>

namespace darix {

Interpreter::Interpreter()
    : environment_(std::make_shared<Environment>()),
      globals_(environment_) {
    // Set up built-in functions
    defineBuiltins(*this);
}

std::shared_ptr<Object> Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (const auto& statement : statements) {
            execute(*statement);
        }
    } catch (const std::runtime_error& e) {
        std::cout << "Runtime error: " << e.what() << std::endl;
    }
    return lastResult_;
}

// Expression evaluation

std::shared_ptr<Object> Interpreter::evaluate(const Expr& expr) {
    if (auto* literal = dynamic_cast<const LiteralExpr*>(&expr)) {
        return evaluateLiteral(*literal);
    } else if (auto* number = dynamic_cast<const NumberExpr*>(&expr)) {
        return evaluateNumber(*number);
    } else if (auto* str = dynamic_cast<const StringExpr*>(&expr)) {
        return evaluateString(*str);
    } else if (auto* boolean = dynamic_cast<const BoolExpr*>(&expr)) {
        return evaluateBool(*boolean);
    } else if (auto* null = dynamic_cast<const NullExpr*>(&expr)) {
        return evaluateNull(*null);
    } else if (auto* variable = dynamic_cast<const VariableExpr*>(&expr)) {
        return evaluateVariable(*variable);
    } else if (auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
        return evaluateBinary(*binary);
    } else if (auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return evaluateUnary(*unary);
    } else if (auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        return evaluateCall(*call);
    } else if (auto* array = dynamic_cast<const ArrayExpr*>(&expr)) {
        return evaluateArray(*array);
    } else if (auto* map = dynamic_cast<const MapExpr*>(&expr)) {
        return evaluateMap(*map);
    } else if (auto* member = dynamic_cast<const MemberExpr*>(&expr)) {
        return evaluateMember(*member);
    } else if (auto* index = dynamic_cast<const IndexExpr*>(&expr)) {
        return evaluateIndex(*index);
    } else if (auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
        return evaluateAssign(*assign);
    }

    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateLiteral(const LiteralExpr& expr) {
    // Try to parse as number
    try {
        if (expr.toString().find('.') != std::string::npos) {
            return makeFloat(std::stod(expr.toString()));
        } else {
            return makeInteger(std::stoll(expr.toString()));
        }
    } catch (...) {
        return makeString(expr.toString());
    }
}

std::shared_ptr<Object> Interpreter::evaluateNumber(const NumberExpr& expr) {
    return makeFloat(expr.value_);
}

std::shared_ptr<Object> Interpreter::evaluateString(const StringExpr& expr) {
    return makeString(expr.value_);
}

std::shared_ptr<Object> Interpreter::evaluateBool(const BoolExpr& expr) {
    return makeBoolean(expr.value_);
}

std::shared_ptr<Object> Interpreter::evaluateNull(const NullExpr& expr) {
    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateVariable(const VariableExpr& expr) {
    return environment_->get(expr.name_);
}

std::shared_ptr<Object> Interpreter::evaluateBinary(const BinaryExpr& expr) {
    auto left = evaluate(*expr.left_);
    auto right = evaluate(*expr.right_);

    switch (expr.op_.type) {
        case TokenType::PLUS:
            if (left->type() == ObjectType::INTEGER && right->type() == ObjectType::INTEGER) {
                return makeInteger(static_cast<IntegerObject*>(left.get())->value() +
                                 static_cast<IntegerObject*>(right.get())->value());
            } else if (left->type() == ObjectType::FLOAT || right->type() == ObjectType::FLOAT) {
                double leftVal = left->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(left.get())->value() :
                    static_cast<IntegerObject*>(left.get())->value();
                double rightVal = right->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(right.get())->value() :
                    static_cast<IntegerObject*>(right.get())->value();
                return makeFloat(leftVal + rightVal);
            } else if (left->type() == ObjectType::STRING && right->type() == ObjectType::STRING) {
                return makeString(static_cast<StringObject*>(left.get())->value() +
                                static_cast<StringObject*>(right.get())->value());
            }
            break;
        case TokenType::MINUS:
            checkNumberOperands(expr.op_, left, right);
            if (left->type() == ObjectType::INTEGER && right->type() == ObjectType::INTEGER) {
                return makeInteger(static_cast<IntegerObject*>(left.get())->value() -
                                 static_cast<IntegerObject*>(right.get())->value());
            } else {
                double leftVal = left->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(left.get())->value() :
                    static_cast<IntegerObject*>(left.get())->value();
                double rightVal = right->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(right.get())->value() :
                    static_cast<IntegerObject*>(right.get())->value();
                return makeFloat(leftVal - rightVal);
            }
        case TokenType::MULTIPLY:
            checkNumberOperands(expr.op_, left, right);
            if (left->type() == ObjectType::INTEGER && right->type() == ObjectType::INTEGER) {
                return makeInteger(static_cast<IntegerObject*>(left.get())->value() *
                                 static_cast<IntegerObject*>(right.get())->value());
            } else {
                double leftVal = left->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(left.get())->value() :
                    static_cast<IntegerObject*>(left.get())->value();
                double rightVal = right->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(right.get())->value() :
                    static_cast<IntegerObject*>(right.get())->value();
                return makeFloat(leftVal * rightVal);
            }
        case TokenType::DIVIDE:
            checkNumberOperands(expr.op_, left, right);
            double leftVal = left->type() == ObjectType::FLOAT ?
                static_cast<FloatObject*>(left.get())->value() :
                static_cast<IntegerObject*>(left.get())->value();
            double rightVal = right->type() == ObjectType::FLOAT ?
                static_cast<FloatObject*>(right.get())->value() :
                static_cast<IntegerObject*>(right.get())->value();
            return makeFloat(leftVal / rightVal);
        case TokenType::MODULO:
            checkNumberOperands(expr.op_, left, right);
            if (left->type() == ObjectType::INTEGER && right->type() == ObjectType::INTEGER) {
                return makeInteger(static_cast<IntegerObject*>(left.get())->value() %
                                 static_cast<IntegerObject*>(right.get())->value());
            }
            break;
        case TokenType::EQUAL_EQUAL:
            return makeBoolean(isEqual(left, right));
        case TokenType::BANG_EQUAL:
            return makeBoolean(!isEqual(left, right));
        case TokenType::LESS:
            checkNumberOperands(expr.op_, left, right);
            if (left->type() == ObjectType::INTEGER && right->type() == ObjectType::INTEGER) {
                return makeBoolean(static_cast<IntegerObject*>(left.get())->value() <
                                 static_cast<IntegerObject*>(right.get())->value());
            } else {
                double leftVal = left->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(left.get())->value() :
                    static_cast<IntegerObject*>(left.get())->value();
                double rightVal = right->type() == ObjectType::FLOAT ?
                    static_cast<FloatObject*>(right.get())->value() :
                    static_cast<IntegerObject*>(right.get())->value();
                return makeBoolean(leftVal < rightVal);
            }
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
            checkNumberOperands(expr.op_, left, right);
            // Similar to LESS but with different comparisons
            break;
        case TokenType::AND:
            return makeBoolean(isTruthy(left) && isTruthy(right));
        case TokenType::OR:
            return makeBoolean(isTruthy(left) || isTruthy(right));
        default:
            break;
    }

    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateUnary(const UnaryExpr& expr) {
    auto right = evaluate(*expr.right_);

    switch (expr.op_.type) {
        case TokenType::MINUS:
            checkNumberOperand(expr.op_, right);
            if (right->type() == ObjectType::INTEGER) {
                return makeInteger(-static_cast<IntegerObject*>(right.get())->value());
            } else {
                return makeFloat(-static_cast<FloatObject*>(right.get())->value());
            }
        case TokenType::BANG:
            return makeBoolean(!isTruthy(right));
        default:
            break;
    }

    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateCall(const CallExpr& expr) {
    auto callee = evaluate(*expr.callee_);

    std::vector<std::shared_ptr<Object>> arguments;
    for (const auto& arg : expr.arguments_) {
        arguments.push_back(evaluate(*arg));
    }

    if (callee->type() == ObjectType::FUNCTION) {
        return callFunction(std::static_pointer_cast<FunctionObject>(callee), arguments);
    } else if (callee->type() == ObjectType::CLASS) {
        auto classObj = std::static_pointer_cast<ClassObject>(callee);
        auto instance = classObj->instantiate();
        // Call constructor if it exists
        auto initMethod = instance->getMethod("__init__");
        if (initMethod) {
            callFunction(initMethod, arguments);
        }
        return instance;
    }

    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateArray(const ArrayExpr& expr) {
    std::vector<std::shared_ptr<Object>> elements;
    for (const auto& elem : expr.elements_) {
        elements.push_back(evaluate(*elem));
    }
    return makeArray(std::move(elements));
}

std::shared_ptr<Object> Interpreter::evaluateMap(const MapExpr& expr) {
    std::unordered_map<std::string, std::shared_ptr<Object>> map;
    for (const auto& pair : expr.pairs_) {
        auto key = evaluate(*pair.first);
        if (key->type() == ObjectType::STRING) {
            auto value = evaluate(*pair.second);
            map[static_cast<StringObject*>(key.get())->value()] = value;
        }
    }
    return makeMap(std::move(map));
}

std::shared_ptr<Object> Interpreter::evaluateMember(const MemberExpr& expr) {
    auto object = evaluate(*expr.object_);
    if (object->type() == ObjectType::INSTANCE) {
        auto instance = std::static_pointer_cast<InstanceObject>(object);
        return instance->get(expr.property_);
    }
    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateIndex(const IndexExpr& expr) {
    auto array = evaluate(*expr.array_);
    auto index = evaluate(*expr.index_);

    if (array->type() == ObjectType::ARRAY && index->type() == ObjectType::INTEGER) {
        auto arr = std::static_pointer_cast<ArrayObject>(array);
        size_t idx = static_cast<IntegerObject*>(index.get())->value();
        if (idx < arr->size()) {
            return arr->elements()[idx];
        }
    }

    return makeNull();
}

std::shared_ptr<Object> Interpreter::evaluateAssign(const AssignExpr& expr) {
    auto value = evaluate(*expr.value_);

    if (auto* variable = dynamic_cast<VariableExpr*>(expr.target_.get())) {
        environment_->assign(variable->name_, value);
    } else if (auto* member = dynamic_cast<MemberExpr*>(expr.target_.get())) {
        auto object = evaluate(*member->object_);
        if (object->type() == ObjectType::INSTANCE) {
            auto instance = std::static_pointer_cast<InstanceObject>(object);
            instance->set(member->property_, value);
        }
    }

    return value;
}

// Statement execution

void Interpreter::execute(const Stmt& stmt) {
    if (auto* exprStmt = dynamic_cast<const ExprStmt*>(&stmt)) {
        executeExprStmt(*exprStmt);
    } else if (auto* varDecl = dynamic_cast<const VarDecl*>(&stmt)) {
        executeVarDecl(*varDecl);
    } else if (auto* funcDecl = dynamic_cast<const FuncDecl*>(&stmt)) {
        executeFuncDecl(*funcDecl);
    } else if (auto* classDecl = dynamic_cast<const ClassDecl*>(&stmt)) {
        executeClassDecl(*classDecl);
    } else if (auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        executeIfStmt(*ifStmt);
    } else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        executeWhileStmt(*whileStmt);
    } else if (auto* forStmt = dynamic_cast<const ForStmt*>(&stmt)) {
        executeForStmt(*forStmt);
    } else if (auto* returnStmt = dynamic_cast<const ReturnStmt*>(&stmt)) {
        executeReturnStmt(*returnStmt);
    } else if (auto* tryStmt = dynamic_cast<const TryStmt*>(&stmt)) {
        executeTryStmt(*tryStmt);
    } else if (auto* blockStmt = dynamic_cast<const BlockStmt*>(&stmt)) {
        executeBlockStmt(*blockStmt);
    }
}

void Interpreter::executeExprStmt(const ExprStmt& stmt) {
    lastResult_ = evaluate(*stmt.expression_);
}

void Interpreter::executeVarDecl(const VarDecl& stmt) {
    std::shared_ptr<Object> value = makeNull();
    if (stmt.initializer_) {
        value = evaluate(*stmt.initializer_);
    }
    environment_->define(stmt.name_, value);
}

void Interpreter::executeFuncDecl(const FuncDecl& stmt) {
    auto function = makeFunction(stmt.name_, [this, &stmt](const std::vector<std::shared_ptr<Object>>& args) {
        // Create function environment
        auto funcEnv = std::make_shared<Environment>(environment_);

        // Bind parameters
        for (size_t i = 0; i < stmt.params_.size(); ++i) {
            funcEnv->define(stmt.params_[i], args[i]);
        }

        // Execute function body
        auto previousEnv = environment_;
        environment_ = funcEnv;

        std::shared_ptr<Object> result = makeNull();
        try {
            for (const auto& bodyStmt : stmt.body_) {
                execute(*bodyStmt);
            }
        } catch (...) {
            // Handle return
        }

        environment_ = previousEnv;
        return result;
    }, stmt.params_.size());

    environment_->define(stmt.name_, function);
}

void Interpreter::executeClassDecl(const ClassDecl& stmt) {
    std::unordered_map<std::string, std::shared_ptr<FunctionObject>> methods;
    for (const auto& method : stmt.methods_) {
        auto func = std::make_shared<FunctionObject>(
            method->name_,
            [this, method = method.get()](const std::vector<std::shared_ptr<Object>>& args) {
                // Method implementation similar to function
                return makeNull();
            },
            method->params_.size()
        );
        methods[method->name_] = func;
    }

    auto classObj = std::make_shared<ClassObject>(stmt.name_, methods);
    environment_->define(stmt.name_, classObj);
}

void Interpreter::executeIfStmt(const IfStmt& stmt) {
    if (isTruthy(evaluate(*stmt.condition_))) {
        for (const auto& bodyStmt : stmt.thenBranch_) {
            execute(*bodyStmt);
        }
    } else {
        for (const auto& bodyStmt : stmt.elseBranch_) {
            execute(*bodyStmt);
        }
    }
}

void Interpreter::executeWhileStmt(const WhileStmt& stmt) {
    while (isTruthy(evaluate(*stmt.condition_))) {
        for (const auto& bodyStmt : stmt.body_) {
            execute(*bodyStmt);
        }
    }
}

void Interpreter::executeForStmt(const ForStmt& stmt) {
    auto loopEnv = std::make_shared<Environment>(environment_);
    auto previousEnv = environment_;
    environment_ = loopEnv;

    if (stmt.initializer_) {
        execute(*stmt.initializer_);
    }

    while (!stmt.condition_ || isTruthy(evaluate(*stmt.condition_))) {
        for (const auto& bodyStmt : stmt.body_) {
            execute(*bodyStmt);
        }

        if (stmt.increment_) {
            evaluate(*stmt.increment_);
        }
    }

    environment_ = previousEnv;
}

void Interpreter::executeReturnStmt(const ReturnStmt& stmt) {
    if (stmt.value_) {
        lastResult_ = evaluate(*stmt.value_);
    }
    // Throw to exit function
    throw std::runtime_error("return");
}

void Interpreter::executeTryStmt(const TryStmt& stmt) {
    try {
        for (const auto& bodyStmt : stmt.tryBody_) {
            execute(*bodyStmt);
        }
    } catch (...) {
        // Exception handling would go here
        for (const auto& bodyStmt : stmt.catchBody_) {
            execute(*bodyStmt);
        }
    }

    for (const auto& bodyStmt : stmt.finallyBody_) {
        execute(*bodyStmt);
    }
}

void Interpreter::executeBlockStmt(const BlockStmt& stmt) {
    auto blockEnv = std::make_shared<Environment>(environment_);
    auto previousEnv = environment_;
    environment_ = blockEnv;

    for (const auto& bodyStmt : stmt.statements_) {
        execute(*bodyStmt);
    }

    environment_ = previousEnv;
}

// Helper methods

bool Interpreter::isTruthy(const std::shared_ptr<Object>& object) const {
    if (object->type() == ObjectType::NULL_OBJ) return false;
    if (object->type() == ObjectType::BOOLEAN) {
        return static_cast<BooleanObject*>(object.get())->value();
    }
    return true;
}

bool Interpreter::isEqual(const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b) const {
    if (a->type() != b->type()) return false;
    return a->equals(b.get());
}

void Interpreter::checkNumberOperand(const Token& op, const std::shared_ptr<Object>& operand) const {
    if (operand->type() == ObjectType::INTEGER || operand->type() == ObjectType::FLOAT) return;
    throw std::runtime_error("Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token& op, const std::shared_ptr<Object>& left,
                                    const std::shared_ptr<Object>& right) const {
    if ((left->type() == ObjectType::INTEGER || left->type() == ObjectType::FLOAT) &&
        (right->type() == ObjectType::INTEGER || right->type() == ObjectType::FLOAT)) return;
    throw std::runtime_error("Operands must be numbers.");
}

std::shared_ptr<Object> Interpreter::callFunction(const std::shared_ptr<FunctionObject>& function,
                                                 const std::vector<std::shared_ptr<Object>>& arguments) {
    return function->call(arguments);
}

} // namespace darix
