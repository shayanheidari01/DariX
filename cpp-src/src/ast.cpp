#include "darix/ast.hpp"
#include <sstream>
#include <algorithm>

namespace darix {

// ============ Helper functions ============

std::string statementsString(const std::vector<StatementPtr>& stmts) {
    std::ostringstream out;
    for (const auto& stmt : stmts) {
        if (stmt) out << stmt->inspect();
    }
    return out.str();
}

std::string expressionString(const ExpressionPtr& expr) {
    if (!expr) return "";
    return expr->inspect();
}

std::string identifierString(const IdentifierPtr& ident) {
    if (!ident) return "";
    return ident->inspect();
}

std::string blockString(const BlockStatementPtr& block) {
    if (!block) return "{}";
    return block->inspect();
}

static std::vector<std::string> identifierStrings(const std::vector<IdentifierPtr>& idents) {
    std::vector<std::string> result;
    for (const auto& id : idents) {
        if (id) result.push_back(id->inspect());
    }
    return result;
}

static std::vector<std::string> expressionStrings(const std::vector<ExpressionPtr>& exprs) {
    std::vector<std::string> result;
    for (const auto& e : exprs) {
        if (e) result.push_back(e->inspect());
    }
    return result;
}

static std::string joinStrings(const std::vector<std::string>& parts, const std::string& sep) {
    std::string result;
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) result += sep;
        result += parts[i];
    }
    return result;
}

// ============ Program ============

std::string Program::tokenLiteral() const {
    if (!statements.empty() && statements[0]) {
        return statements[0]->tokenLiteral();
    }
    return "";
}

std::string Program::inspect() const {
    return statementsString(statements);
}

// ============ ImportStatement ============

std::string ImportStatement::tokenLiteral() const { return token.literal; }
std::string ImportStatement::inspect() const {
    std::string out = "import";
    if (path) out += " " + expressionString(path);
    out += ";";
    return out;
}

// ============ LetStatement ============

std::string LetStatement::tokenLiteral() const { return token.literal; }
std::string LetStatement::inspect() const {
    return tokenLiteral() + " " + identifierString(name) + " = " + expressionString(value) + ";";
}

// ============ AssignStatement ============

std::string AssignStatement::tokenLiteral() const { return token.literal; }
std::string AssignStatement::inspect() const {
    return expressionString(target) + " = " + expressionString(value) + ";";
}

// ============ ReturnStatement ============

std::string ReturnStatement::tokenLiteral() const { return token.literal; }
std::string ReturnStatement::inspect() const {
    return tokenLiteral() + " " + expressionString(returnValue) + ";";
}

// ============ ExpressionStatement ============

std::string ExpressionStatement::tokenLiteral() const { return token.literal; }
std::string ExpressionStatement::inspect() const {
    return expressionString(expression);
}

// ============ BlockStatement ============

std::string BlockStatement::tokenLiteral() const { return token.literal; }
std::string BlockStatement::inspect() const {
    return "{" + statementsString(statements) + "}";
}

// ============ StandaloneBlockStatement ============

std::string StandaloneBlockStatement::tokenLiteral() const { return token.literal; }
std::string StandaloneBlockStatement::inspect() const {
    return blockString(block);
}

// ============ BreakStatement ============

std::string BreakStatement::tokenLiteral() const { return token.literal; }
std::string BreakStatement::inspect() const { return "break;"; }

// ============ ContinueStatement ============

std::string ContinueStatement::tokenLiteral() const { return token.literal; }
std::string ContinueStatement::inspect() const { return "continue;"; }

// ============ WhileStatement ============

std::string WhileStatement::tokenLiteral() const { return token.literal; }
std::string WhileStatement::inspect() const {
    return "while(" + expressionString(condition) + ") " + blockString(body);
}

// ============ ForStatement ============

std::string ForStatement::tokenLiteral() const { return token.literal; }
std::string ForStatement::inspect() const {
    std::string i = init ? init->inspect() : "";
    std::string c = condition ? condition->inspect() : "";
    std::string p = post ? post->inspect() : "";
    return "for(" + i + "; " + c + "; " + p + ") " + blockString(body);
}

// ============ FunctionDeclaration ============

std::string FunctionDeclaration::tokenLiteral() const { return token.literal; }
std::string FunctionDeclaration::inspect() const {
    std::ostringstream out;
    for (const auto& d : decorators) {
        if (d) out << "@" << d->inspect() << "\n";
    }
    auto params = identifierStrings(parameters);
    out << "func ";
    if (name) out << name->inspect();
    out << "(" << joinStrings(params, ", ") << ") ";
    if (body) out << body->inspect();
    return out.str();
}

// ============ ClassDeclaration ============

std::string ClassDeclaration::tokenLiteral() const { return token.literal; }
std::string ClassDeclaration::inspect() const {
    std::ostringstream out;
    for (const auto& d : decorators) {
        if (d) out << "@" << d->inspect() << "\n";
    }
    out << "class " << identifierString(name) << " " << blockString(body);
    return out.str();
}

// ============ ThrowStatement ============

std::string ThrowStatement::tokenLiteral() const { return token.literal; }
std::string ThrowStatement::inspect() const {
    return tokenLiteral() + " " + expressionString(exception) + ";";
}

// ============ CatchClause ============

std::string CatchClause::inspect() const {
    std::string out = "catch";
    if (exceptionType) {
        out += " (" + exceptionType->inspect();
        if (auto name = identifierString(variable); !name.empty()) {
            out += " " + name;
        }
        out += ")";
    } else if (auto name = identifierString(variable); !name.empty()) {
        out += " (" + name + ")";
    }
    if (catchBlock) out += " " + catchBlock->inspect();
    return out;
}

// ============ TryStatement ============

std::string TryStatement::tokenLiteral() const { return token.literal; }
std::string TryStatement::inspect() const {
    std::string out = "try " + blockString(tryBlock);
    for (const auto& cc : catchClauses) {
        if (cc) out += " " + cc->inspect();
    }
    if (finallyBlock) out += " finally " + finallyBlock->inspect();
    return out;
}

// ============ DelStatement ============

std::string DelStatement::tokenLiteral() const { return token.literal; }
std::string DelStatement::inspect() const {
    return "del " + expressionString(target) + ";";
}

// ============ AssertStatement ============

std::string AssertStatement::tokenLiteral() const { return token.literal; }
std::string AssertStatement::inspect() const {
    std::string out = "assert " + expressionString(condition);
    if (message) out += ", " + expressionString(message);
    out += ";";
    return out;
}

// ============ PassStatement ============

std::string PassStatement::tokenLiteral() const { return token.literal; }
std::string PassStatement::inspect() const { return "pass;"; }

// ============ GlobalStatement ============

std::string GlobalStatement::tokenLiteral() const { return token.literal; }
std::string GlobalStatement::inspect() const {
    auto names = identifierStrings(this->names);
    return "global " + joinStrings(names, ", ") + ";";
}

// ============ NonlocalStatement ============

std::string NonlocalStatement::tokenLiteral() const { return token.literal; }
std::string NonlocalStatement::inspect() const {
    auto names = identifierStrings(this->names);
    return "nonlocal " + joinStrings(names, ", ") + ";";
}

// ============ WithStatement ============

std::string WithStatement::tokenLiteral() const { return token.literal; }
std::string WithStatement::inspect() const {
    std::string out = "with " + expressionString(context);
    if (variable) out += " as " + variable->inspect();
    out += " " + blockString(body);
    return out;
}

// ============ Identifier ============

std::string Identifier::tokenLiteral() const { return token.literal; }
std::string Identifier::inspect() const { return value; }

// ============ IntegerLiteral ============

std::string IntegerLiteral::tokenLiteral() const { return token.literal; }
std::string IntegerLiteral::inspect() const { return token.literal; }

// ============ FloatLiteral ============

std::string FloatLiteral::tokenLiteral() const { return token.literal; }
std::string FloatLiteral::inspect() const { return token.literal; }

// ============ StringLiteral ============

std::string StringLiteral::tokenLiteral() const { return token.literal; }
std::string StringLiteral::inspect() const { return "\"" + value + "\""; }

// ============ Boolean ============

std::string BooleanLiteral::tokenLiteral() const { return token.literal; }
std::string BooleanLiteral::inspect() const { return token.literal; }

// ============ NullLiteral ============

std::string NullLiteral::tokenLiteral() const { return token.literal; }
std::string NullLiteral::inspect() const { return "null"; }

// ============ AssignExpression ============

std::string AssignExpression::tokenLiteral() const { return token.literal; }
std::string AssignExpression::inspect() const {
    return expressionString(name) + " = " + expressionString(value);
}

// ============ PrefixExpression ============

std::string PrefixExpression::tokenLiteral() const { return token.literal; }
std::string PrefixExpression::inspect() const {
    return "(" + op + expressionString(right) + ")";
}

// ============ InfixExpression ============

std::string InfixExpression::tokenLiteral() const { return token.literal; }
std::string InfixExpression::inspect() const {
    return "(" + expressionString(left) + " " + op + " " + expressionString(right) + ")";
}

// ============ IfExpression ============

std::string IfExpression::tokenLiteral() const { return token.literal; }
std::string IfExpression::inspect() const {
    std::string out = "if" + expressionString(condition) + " " + blockString(consequence);
    if (alternative) out += "else " + alternative->inspect();
    return out;
}

// ============ FunctionLiteral ============

std::string FunctionLiteral::tokenLiteral() const { return token.literal; }
std::string FunctionLiteral::inspect() const {
    auto params = identifierStrings(parameters);
    return tokenLiteral() + "(" + joinStrings(params, ", ") + ") " + blockString(body);
}

// ============ CallExpression ============

std::string CallExpression::tokenLiteral() const { return token.literal; }
std::string CallExpression::inspect() const {
    auto args = expressionStrings(arguments);
    return expressionString(function) + "(" + joinStrings(args, ", ") + ")";
}

// ============ ArrayLiteral ============

std::string ArrayLiteral::tokenLiteral() const { return token.literal; }
std::string ArrayLiteral::inspect() const {
    auto elems = expressionStrings(elements);
    return "[" + joinStrings(elems, ", ") + "]";
}

// ============ MapLiteral ============

std::string MapLiteral::tokenLiteral() const { return token.literal; }
std::string MapLiteral::inspect() const {
    if (pairs.empty()) return "{}";
    std::vector<std::string> entries;
    for (const auto& [k, v] : pairs) {
        entries.push_back(expressionString(k) + ":" + expressionString(v));
    }
    std::sort(entries.begin(), entries.end());
    return "{" + joinStrings(entries, ", ") + "}";
}

// ============ IndexExpression ============

std::string IndexExpression::tokenLiteral() const { return token.literal; }
std::string IndexExpression::inspect() const {
    return "(" + expressionString(left) + "[" + expressionString(index) + "])";
}

// ============ MemberExpression ============

std::string MemberExpression::tokenLiteral() const { return token.literal; }
std::string MemberExpression::inspect() const {
    return "(" + expressionString(left) + "." + identifierString(property) + ")";
}

// ============ WhileExpression ============

std::string WhileExpression::tokenLiteral() const { return token.literal; }
std::string WhileExpression::inspect() const {
    return "while(" + expressionString(condition) + ") " + blockString(body);
}

// ============ InExpression ============

std::string InExpression::tokenLiteral() const { return token.literal; }
std::string InExpression::inspect() const {
    return "(" + expressionString(left) + " in " + expressionString(right) + ")";
}

// ============ IsExpression ============

std::string IsExpression::tokenLiteral() const { return token.literal; }
std::string IsExpression::inspect() const {
    return "(" + expressionString(left) + " is " + expressionString(right) + ")";
}

// ============ LambdaExpression ============

std::string LambdaExpression::tokenLiteral() const { return token.literal; }
std::string LambdaExpression::inspect() const {
    auto params = identifierStrings(parameters);
    return "lambda " + joinStrings(params, ", ") + ": " + expressionString(body);
}

// ============ YieldExpression ============

std::string YieldExpression::tokenLiteral() const { return token.literal; }
std::string YieldExpression::inspect() const {
    std::string out = "yield";
    if (value) out += " " + expressionString(value);
    return out;
}

// ============ ExceptionExpression ============

std::string ExceptionExpression::tokenLiteral() const { return token.literal; }
std::string ExceptionExpression::inspect() const {
    return identifierString(type) + "(" + expressionString(message) + ")";
}

} // namespace darix
