#include "ast.h"
#include <sstream>

namespace darix {

std::string CallExpr::toString() const {
    std::stringstream ss;
    ss << callee_->toString() << "(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << arguments_[i]->toString();
    }
    ss << ")";
    return ss.str();
}

std::string ArrayExpr::toString() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < elements_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements_[i]->toString();
    }
    ss << "]";
    return ss.str();
}

std::string MapExpr::toString() const {
    std::stringstream ss;
    ss << "{";
    for (size_t i = 0; i < pairs_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << pairs_[i].first->toString() << ": " << pairs_[i].second->toString();
    }
    ss << "}";
    return ss.str();
}

std::string FuncDecl::toString() const {
    std::stringstream ss;
    ss << "func " << name_ << "(";
    for (size_t i = 0; i < params_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << params_[i];
    }
    ss << ") {\n";
    for (const auto& stmt : body_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string ClassDecl::toString() const {
    std::stringstream ss;
    ss << "class " << name_ << " {\n";
    for (const auto& method : methods_) {
        ss << "  " << method->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string IfStmt::toString() const {
    std::stringstream ss;
    ss << "if (" << condition_->toString() << ") {\n";
    for (const auto& stmt : thenBranch_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    if (!elseBranch_.empty()) {
        ss << " else {\n";
        for (const auto& stmt : elseBranch_) {
            ss << "  " << stmt->toString() << "\n";
        }
        ss << "}";
    }
    return ss.str();
}

std::string WhileStmt::toString() const {
    std::stringstream ss;
    ss << "while (" << condition_->toString() << ") {\n";
    for (const auto& stmt : body_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string ForStmt::toString() const {
    std::stringstream ss;
    ss << "for (";
    if (initializer_) ss << initializer_->toString();
    ss << "; ";
    if (condition_) ss << condition_->toString();
    ss << "; ";
    if (increment_) ss << increment_->toString();
    ss << ") {\n";
    for (const auto& stmt : body_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string TryStmt::toString() const {
    std::stringstream ss;
    ss << "try {\n";
    for (const auto& stmt : tryBody_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "} catch (" << catchVar_ << ") {\n";
    for (const auto& stmt : catchBody_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    if (!finallyBody_.empty()) {
        ss << " finally {\n";
        for (const auto& stmt : finallyBody_) {
            ss << "  " << stmt->toString() << "\n";
        }
        ss << "}";
    }
    return ss.str();
}

std::string BlockStmt::toString() const {
    std::stringstream ss;
    ss << "{\n";
    for (const auto& stmt : statements_) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

} // namespace darix
