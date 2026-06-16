#ifndef DARIX_PARSER_AST_HPP
#define DARIX_PARSER_AST_HPP

#include "lexer/token.hpp"
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace darix {

// Forward declarations
struct Expression;
struct Statement;

// Type definitions for type system
struct TypeHint {
    std::string name;
    bool nullable = false;
    std::vector<TypeHint> generics;
    
    TypeHint() = default;
    explicit TypeHint(std::string n, bool null = false) 
        : name(std::move(n)), nullable(null) {}
};

using TypeHintOpt = std::optional<TypeHint>;

// Expression types using variant pattern
struct LiteralExpr {
    TokenValue value;
};

struct IdentifierExpr {
    std::string name;
};

struct BinaryExpr {
    std::shared_ptr<Expression> left;
    Token op;
    std::shared_ptr<Expression> right;
};

struct UnaryExpr {
    Token op;
    std::shared_ptr<Expression> operand;
};

struct GroupingExpr {
    std::shared_ptr<Expression> expression;
};

struct CallExpr {
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> namedArgs;
};

struct GetExpr {
    std::shared_ptr<Expression> object;
    std::string name;
};

struct SetExpr {
    std::shared_ptr<Expression> object;
    std::string name;
    std::shared_ptr<Expression> value;
};

struct ThisExpr {
    // Represents 'this' keyword
};

struct SuperExpr {
    std::string method;
};

struct LambdaExpr {
    std::vector<std::pair<std::string, TypeHintOpt>> params;
    std::optional<std::string> returnType;
    std::shared_ptr<Statement> body;
    bool isAsync = false;
};

struct ListExpr {
    std::vector<std::shared_ptr<Expression>> elements;
};

struct DictExpr {
    std::vector<std::pair<std::shared_ptr<Expression>, std::shared_ptr<Expression>>> entries;
};

struct IndexExpr {
    std::shared_ptr<Expression> object;
    std::shared_ptr<Expression> index;
};

struct ConditionalExpr {
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Expression> thenBranch;
    std::shared_ptr<Expression> elseBranch;
};

struct NullCoalescingExpr {
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
};

struct IsExpr {
    std::shared_ptr<Expression> operand;
    TypeHint type;
};

struct AsExpr {
    std::shared_ptr<Expression> operand;
    TypeHint type;
};

struct PipeExpr {
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;  // Usually a call expression
};

struct ComprehensionExpr {
    std::shared_ptr<Expression> template_;
    std::string varName;
    std::shared_ptr<Expression> iterable;
    std::optional<std::shared_ptr<Expression>> condition;
    bool isDict = false;
};

struct Expression {
    std::variant<
        LiteralExpr,
        IdentifierExpr,
        BinaryExpr,
        UnaryExpr,
        GroupingExpr,
        CallExpr,
        GetExpr,
        SetExpr,
        ThisExpr,
        SuperExpr,
        LambdaExpr,
        ListExpr,
        DictExpr,
        IndexExpr,
        ConditionalExpr,
        NullCoalescingExpr,
        IsExpr,
        AsExpr,
        PipeExpr,
        ComprehensionExpr
    > expr;
    
    size_t line = 0;
    size_t column = 0;
    
    template<typename T>
    [[nodiscard]] const T* get() const {
        if (auto ptr = std::get_if<T>(&expr)) {
            return ptr;
        }
        return nullptr;
    }
    
    template<typename T>
    [[nodiscard]] bool is() const {
        return std::holds_alternative<T>(expr);
    }
};

// Statement types
struct ExpressionStmt {
    std::shared_ptr<Expression> expression;
};

struct VarDeclStmt {
    std::string name;
    TypeHintOpt typeHint;
    std::shared_ptr<Expression> initializer;
    bool isConst = false;
    bool isFinal = false;
};

struct FunctionStmt {
    std::string name;
    std::vector<std::pair<std::string, TypeHintOpt>> params;
    std::vector<std::pair<std::string, std::shared_ptr<Expression>>> optionalParams;
    TypeHintOpt returnType;
    std::shared_ptr<Statement> body;
    bool isAsync = false;
    bool isStatic = false;
    bool isExternal = false;
};

struct ClassStmt {
    std::string name;
    std::optional<std::string> superClass;
    std::vector<std::string> mixins;
    std::vector<std::shared_ptr<Statement>> methods;
    std::vector<std::shared_ptr<Statement>> fields;
};

struct BlockStmt {
    std::vector<std::shared_ptr<Statement>> statements;
};

struct IfStmt {
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> thenBranch;
    std::vector<std::pair<std::shared_ptr<Expression>, std::shared_ptr<Statement>>> elifBranches;
    std::shared_ptr<Statement> elseBranch;
};

struct MatchStmt {
    std::shared_ptr<Expression> subject;
    struct Case {
        std::shared_ptr<Expression> pattern;
        std::optional<std::string> guard;
        std::shared_ptr<Statement> body;
        bool isDefault = false;
    };
    std::vector<Case> cases;
};

struct WhileStmt {
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
};

struct ForStmt {
    std::string varName;
    TypeHintOpt typeHint;
    std::shared_ptr<Expression> iterable;
    std::shared_ptr<Statement> body;
    bool isAsync = false;
};

struct ReturnStmt {
    std::optional<std::shared_ptr<Expression>> value;
};

struct BreakStmt {};
struct ContinueStmt {};

struct TryStmt {
    std::shared_ptr<Statement> body;
    struct CatchClause {
        std::optional<std::string> exceptionVar;
        TypeHintOpt exceptionType;
        std::shared_ptr<Statement> body;
    };
    std::vector<CatchClause> catchClauses;
    std::optional<std::shared_ptr<Statement>> finallyBody;
};

struct ThrowStmt {
    std::shared_ptr<Expression> exception;
};

struct ImportStmt {
    std::string module;
    std::optional<std::string> alias;
    std::vector<std::string> imports;
    bool isFrom = false;
};

struct DecoratorStmt {
    std::shared_ptr<Expression> expression;
};

struct AssertStmt {
    std::shared_ptr<Expression> condition;
    std::optional<std::shared_ptr<Expression>> message;
};

struct Statement {
    std::variant<
        ExpressionStmt,
        VarDeclStmt,
        FunctionStmt,
        ClassStmt,
        BlockStmt,
        IfStmt,
        MatchStmt,
        WhileStmt,
        ForStmt,
        ReturnStmt,
        BreakStmt,
        ContinueStmt,
        TryStmt,
        ThrowStmt,
        ImportStmt,
        DecoratorStmt,
        AssertStmt
    > stmt;
    
    size_t line = 0;
    size_t column = 0;
    
    template<typename T>
    [[nodiscard]] const T* get() const {
        if (auto ptr = std::get_if<T>(&stmt)) {
            return ptr;
        }
        return nullptr;
    }
    
    template<typename T>
    [[nodiscard]] bool is() const {
        return std::holds_alternative<T>(stmt);
    }
};

// Program structure
struct Program {
    std::vector<std::shared_ptr<Statement>> statements;
};

// Visitor pattern base
template<typename R>
class Visitor {
public:
    virtual ~Visitor() = default;
    
    // Expression visitors
    virtual R visitLiteralExpr(const LiteralExpr& expr) = 0;
    virtual R visitIdentifierExpr(const IdentifierExpr& expr) = 0;
    virtual R visitBinaryExpr(const BinaryExpr& expr) = 0;
    virtual R visitUnaryExpr(const UnaryExpr& expr) = 0;
    virtual R visitGroupingExpr(const GroupingExpr& expr) = 0;
    virtual R visitCallExpr(const CallExpr& expr) = 0;
    virtual R visitGetExpr(const GetExpr& expr) = 0;
    virtual R visitSetExpr(const SetExpr& expr) = 0;
    virtual R visitThisExpr(const ThisExpr& expr) = 0;
    virtual R visitSuperExpr(const SuperExpr& expr) = 0;
    virtual R visitLambdaExpr(const LambdaExpr& expr) = 0;
    virtual R visitListExpr(const ListExpr& expr) = 0;
    virtual R visitDictExpr(const DictExpr& expr) = 0;
    virtual R visitIndexExpr(const IndexExpr& expr) = 0;
    virtual R visitConditionalExpr(const ConditionalExpr& expr) = 0;
    virtual R visitNullCoalescingExpr(const NullCoalescingExpr& expr) = 0;
    virtual R visitIsExpr(const IsExpr& expr) = 0;
    virtual R visitAsExpr(const AsExpr& expr) = 0;
    virtual R visitPipeExpr(const PipeExpr& expr) = 0;
    virtual R visitComprehensionExpr(const ComprehensionExpr& expr) = 0;
    
    // Statement visitors
    virtual R visitExpressionStmt(const ExpressionStmt& stmt) = 0;
    virtual R visitVarDeclStmt(const VarDeclStmt& stmt) = 0;
    virtual R visitFunctionStmt(const FunctionStmt& stmt) = 0;
    virtual R visitClassStmt(const ClassStmt& stmt) = 0;
    virtual R visitBlockStmt(const BlockStmt& stmt) = 0;
    virtual R visitIfStmt(const IfStmt& stmt) = 0;
    virtual R visitMatchStmt(const MatchStmt& stmt) = 0;
    virtual R visitWhileStmt(const WhileStmt& stmt) = 0;
    virtual R visitForStmt(const ForStmt& stmt) = 0;
    virtual R visitReturnStmt(const ReturnStmt& stmt) = 0;
    virtual R visitBreakStmt(const BreakStmt& stmt) = 0;
    virtual R visitContinueStmt(const ContinueStmt& stmt) = 0;
    virtual R visitTryStmt(const TryStmt& stmt) = 0;
    virtual R visitThrowStmt(const ThrowStmt& stmt) = 0;
    virtual R visitImportStmt(const ImportStmt& stmt) = 0;
    virtual R visitDecoratorStmt(const DecoratorStmt& stmt) = 0;
    virtual R visitAssertStmt(const AssertStmt& stmt) = 0;
};

// Helper functions to create expressions
inline std::shared_ptr<Expression> makeLiteral(TokenValue value) {
    auto expr = std::make_shared<Expression>();
    expr->expr = LiteralExpr{std::move(value)};
    return expr;
}

inline std::shared_ptr<Expression> makeIdentifier(const std::string& name) {
    auto expr = std::make_shared<Expression>();
    expr->expr = IdentifierExpr{name};
    return expr;
}

} // namespace darix

#endif // DARIX_PARSER_AST_HPP
