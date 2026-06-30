#pragma once

#include "darix/ast.hpp"
#include "darix/code.hpp"
#include "darix/object.hpp"
#include <string>
#include <vector>

namespace darix {

constexpr const char* BytecodeMagic = "DRXB1";

// Symbol table
enum class SymbolScope { GLOBAL, LOCAL };

struct Symbol {
    std::string name;
    SymbolScope scope = SymbolScope::GLOBAL;
    int index = 0;
};

class SymbolTable {
public:
    SymbolTable() = default;
    explicit SymbolTable(std::shared_ptr<SymbolTable> outer);

    Symbol define(const std::string& name);
    std::pair<Symbol, bool> resolve(const std::string& name) const;

    int numDefinitions() const { return numDefinitions_; }
    std::shared_ptr<SymbolTable> outer() const { return outer_; }

private:
    std::unordered_map<std::string, Symbol> store_;
    int numDefinitions_ = 0;
    std::shared_ptr<SymbolTable> outer_;
};

// Debug info
struct DebugEntry {
    int pc = 0;
    std::string file;
    int line = 0;
    int column = 0;
    std::string function;
};

struct DebugInfo {
    std::vector<DebugEntry> entries;
};

// Bytecode
struct Bytecode {
    std::string magic;
    std::string version;
    Instructions instructions;
    std::vector<ObjectPtr> constants;
    DebugInfo debug;
};

// Compiler
class Compiler {
public:
    Compiler();

    bool compile(Node* node);
    std::shared_ptr<Bytecode> bytecode();

private:
    int emit(Opcode op, const std::vector<int>& operands = {});
    int emitAt(Node* node, Opcode op, const std::vector<int>& operands = {});
    int addConstant(ObjectPtr obj);
    void compileStatements(const std::vector<StatementPtr>& stmts);
    bool compileBlock(const BlockStatementPtr& block);
    void compileExpressions(const std::vector<ExpressionPtr>& exprs);
    // Returns true if the builtin was handled and the expression does NOT push a value
    bool compileBuiltinCall(CallExpression* node, const std::string& name);
    void replaceOperand(int pos, int operand);
    void replaceInstruction(int pos, const Instructions& newIns);

    Instructions instructions_;
    std::vector<ObjectPtr> constants_;
    std::shared_ptr<SymbolTable> symbolTable_;
    std::vector<DebugEntry> debugEntries_;
    bool lastCompiledPushedValue_ = true;
};

// Constant folding
ObjectPtr foldConstExpr(Node* node, bool* ok);

// Peephole optimizer
Instructions peephole(const Instructions& ins);

// Token info from node
DebugEntry tokenInfoFromNode(Node* node);

} // namespace darix
