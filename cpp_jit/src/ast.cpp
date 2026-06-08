#include "ast.h"

namespace darix {

// Expr visitor implementations
void LiteralExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void VariableExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void BinaryExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void UnaryExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void CallExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void ArrayExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void MapExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void MemberExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void IndexExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

void AssignExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

// Stmt visitor implementations
void ExprStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void VarDecl::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void FuncDecl::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void ClassDecl::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void ReturnStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void IfStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void WhileStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void ForStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void TryStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

void BlockStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

} // namespace darix
