#include "darix/compiler.hpp"
#include "darix/version.hpp"
#include <charconv>
#include <stdexcept>

namespace darix {

// ============ SymbolTable ============

SymbolTable::SymbolTable(std::shared_ptr<SymbolTable> outer) : outer_(outer) {}

Symbol SymbolTable::define(const std::string& name) {
    SymbolScope scope = outer_ ? SymbolScope::LOCAL : SymbolScope::GLOBAL;
    Symbol s{name, scope, numDefinitions_};
    store_[name] = s;
    numDefinitions_++;
    return s;
}

std::pair<Symbol, bool> SymbolTable::resolve(const std::string& name) const {
    auto it = store_.find(name);
    if (it != store_.end()) return {it->second, true};
    if (outer_) return outer_->resolve(name);
    return {{}, false};
}

// ============ Compiler ============

Compiler::Compiler() : symbolTable_(std::make_shared<SymbolTable>()) {}

int Compiler::emit(Opcode op, const std::vector<int>& operands) {
    auto ins = Make(op, operands);
    int pos = static_cast<int>(instructions_.size());
    instructions_.insert(instructions_.end(), ins.begin(), ins.end());
    return pos;
}

int Compiler::emitAt(Node* node, Opcode op, const std::vector<int>& operands) {
    int pos = emit(op, operands);
    auto entry = tokenInfoFromNode(node);
    if (!entry.file.empty() || entry.line > 0 || entry.column > 0) {
        entry.pc = pos;
        debugEntries_.push_back(entry);
    }
    return pos;
}

int Compiler::addConstant(ObjectPtr obj) {
    constants_.push_back(obj);
    return static_cast<int>(constants_.size()) - 1;
}

std::shared_ptr<Bytecode> Compiler::bytecode() {
    instructions_ = peephole(instructions_);
    auto bc = std::make_shared<Bytecode>();
    bc->magic = BytecodeMagic;
    bc->version = DARIX_VERSION;
    bc->instructions = instructions_;
    bc->constants = constants_;
    bc->debug.entries = debugEntries_;
    return bc;
}

void Compiler::compileStatements(const std::vector<StatementPtr>& stmts) {
    for (const auto& stmt : stmts) {
        compile(stmt.get());
    }
}

bool Compiler::compileBlock(const BlockStatementPtr& block) {
    if (!block) return true;
    compileStatements(block->statements);
    return true;
}

void Compiler::compileExpressions(const std::vector<ExpressionPtr>& exprs) {
    for (const auto& expr : exprs) {
        compile(expr.get());
    }
}

bool Compiler::compileBuiltinCall(CallExpression* node, const std::string& name) {
    if (name == "print") {
        compileExpressions(node->arguments);
        emitAt(node, Opcode::OpPrint, {static_cast<int>(node->arguments.size())});
        lastCompiledPushedValue_ = false;
        return true;
    }
    if (name == "len") {
        if (node->arguments.size() != 1) throw std::runtime_error("len: expected 1 argument");
        compile(node->arguments[0].get());
        emitAt(node, Opcode::OpLen);
        lastCompiledPushedValue_ = true;
        return true;
    }
    if (name == "type") {
        if (node->arguments.size() != 1) throw std::runtime_error("type: expected 1 argument");
        compile(node->arguments[0].get());
        emitAt(node, Opcode::OpType);
        lastCompiledPushedValue_ = true;
        return true;
    }
    return false;
}

bool Compiler::compile(Node* node) {
    if (!node) { lastCompiledPushedValue_ = true; return true; }

    if (auto program = dynamic_cast<Program*>(node)) {
        compileStatements(program->statements);
        lastCompiledPushedValue_ = true;
        return true;
    }
    if (auto block = dynamic_cast<BlockStatement*>(node)) {
        compileStatements(block->statements);
        lastCompiledPushedValue_ = true;
        return true;
    }
    if (auto exprStmt = dynamic_cast<ExpressionStatement*>(node)) {
        compile(exprStmt->expression.get());
        if (lastCompiledPushedValue_) {
            emitAt(node, Opcode::OpPop);
        }
        lastCompiledPushedValue_ = true;
        return true;
    }
    if (auto intLit = dynamic_cast<IntegerLiteral*>(node)) {
        int idx = addConstant(newInteger(intLit->value));
        emitAt(node, Opcode::OpConstant, {idx});
        return true;
    }
    if (auto floatLit = dynamic_cast<FloatLiteral*>(node)) {
        int idx = addConstant(newFloat(floatLit->value));
        emitAt(node, Opcode::OpConstant, {idx});
        return true;
    }
    if (auto strLit = dynamic_cast<StringLiteral*>(node)) {
        int idx = addConstant(newString(strLit->value));
        emitAt(node, Opcode::OpConstant, {idx});
        return true;
    }
    if (auto boolLit = dynamic_cast<BooleanLiteral*>(node)) {
        emitAt(node, boolLit->value ? Opcode::OpTrue : Opcode::OpFalse);
        return true;
    }
    if (dynamic_cast<NullLiteral*>(node)) {
        emitAt(node, Opcode::OpNull);
        return true;
    }
    if (auto arr = dynamic_cast<ArrayLiteral*>(node)) {
        compileExpressions(arr->elements);
        emitAt(node, Opcode::OpArray, {static_cast<int>(arr->elements.size())});
        return true;
    }
    if (auto idx = dynamic_cast<IndexExpression*>(node)) {
        compile(idx->left.get());
        compile(idx->index.get());
        emitAt(node, Opcode::OpIndex);
        return true;
    }
    if (auto prefix = dynamic_cast<PrefixExpression*>(node)) {
        bool ok = false;
        auto folded = foldConstExpr(node, &ok);
        if (ok) {
            int idx = addConstant(folded);
            emitAt(node, Opcode::OpConstant, {idx});
            return true;
        }
        compile(prefix->right.get());
        if (prefix->op == "-") emitAt(node, Opcode::OpMinus);
        else if (prefix->op == "!") emitAt(node, Opcode::OpBang);
        else throw std::runtime_error("unsupported prefix operator " + prefix->op);
        return true;
    }
    if (auto infix = dynamic_cast<InfixExpression*>(node)) {
        bool ok = false;
        auto folded = foldConstExpr(node, &ok);
        if (ok) {
            int idx = addConstant(folded);
            emitAt(node, Opcode::OpConstant, {idx});
            return true;
        }
        if (infix->op == "<=") {
            compile(infix->right.get());
            compile(infix->left.get());
            emitAt(node, Opcode::OpGreaterEqual);
            return true;
        }
        if (infix->op == ">=") {
            compile(infix->left.get());
            compile(infix->right.get());
            emitAt(node, Opcode::OpGreaterEqual);
            return true;
        }
        compile(infix->left.get());
        compile(infix->right.get());
        if (infix->op == "+") emitAt(node, Opcode::OpAdd);
        else if (infix->op == "-") emitAt(node, Opcode::OpSub);
        else if (infix->op == "*") emitAt(node, Opcode::OpMul);
        else if (infix->op == "/") emitAt(node, Opcode::OpDiv);
        else if (infix->op == "%") emitAt(node, Opcode::OpMod);
        else if (infix->op == "==") emitAt(node, Opcode::OpEqual);
        else if (infix->op == "!=") emitAt(node, Opcode::OpNotEqual);
        else if (infix->op == ">") emitAt(node, Opcode::OpGreaterThan);
        else if (infix->op == "<") emitAt(node, Opcode::OpLessThan);
        else throw std::runtime_error("unsupported infix operator " + infix->op);
        return true;
    }
    if (auto letStmt = dynamic_cast<LetStatement*>(node)) {
        compile(letStmt->value.get());
        auto sym = symbolTable_->define(letStmt->name->value);
        emitAt(node, Opcode::OpSetGlobal, {sym.index});
        return true;
    }
    if (auto ident = dynamic_cast<Identifier*>(node)) {
        auto [sym, ok] = symbolTable_->resolve(ident->value);
        if (!ok) throw std::runtime_error("undefined variable " + ident->value);
        emitAt(node, Opcode::OpGetGlobal, {sym.index});
        return true;
    }
    if (auto assign = dynamic_cast<AssignStatement*>(node)) {
        if (auto targetIdent = dynamic_cast<Identifier*>(assign->target.get())) {
            compile(assign->value.get());
            auto [sym, ok] = symbolTable_->resolve(targetIdent->value);
            if (!ok) sym = symbolTable_->define(targetIdent->value);
            emitAt(node, Opcode::OpSetGlobal, {sym.index});
            return true;
        }
        if (auto targetIdx = dynamic_cast<IndexExpression*>(assign->target.get())) {
            compile(targetIdx->left.get());
            compile(targetIdx->index.get());
            compile(assign->value.get());
            emitAt(node, Opcode::OpSetIndex);
            return true;
        }
        throw std::runtime_error("unsupported assignment target");
    }
    if (auto ifExpr = dynamic_cast<IfExpression*>(node)) {
        compile(ifExpr->condition.get());
        int jntPos = emitAt(node, Opcode::OpJumpNotTruthy, {9999});
        compileBlock(ifExpr->consequence);
        int jmpPos = emitAt(node, Opcode::OpJump, {9999});
        replaceOperand(jntPos, static_cast<int>(instructions_.size()));
        if (ifExpr->alternative) compile(ifExpr->alternative.get());
        replaceOperand(jmpPos, static_cast<int>(instructions_.size()));
        emitAt(node, Opcode::OpNull);
        lastCompiledPushedValue_ = true;
        return true;
    }
    if (auto whileStmt = dynamic_cast<WhileStatement*>(node)) {
        int condPos = static_cast<int>(instructions_.size());
        compile(whileStmt->condition.get());
        int jntPos = emitAt(node, Opcode::OpJumpNotTruthy, {9999});
        compileBlock(whileStmt->body);
        emitAt(node, Opcode::OpJump, {condPos});
        replaceOperand(jntPos, static_cast<int>(instructions_.size()));
        return true;
    }
    if (auto call = dynamic_cast<CallExpression*>(node)) {
        if (auto ident = dynamic_cast<Identifier*>(call->function.get())) {
            bool handled = compileBuiltinCall(call, ident->value);
            if (handled) return true;
        }
        throw std::runtime_error("unsupported function call in VM");
    }

    throw std::runtime_error("unsupported AST node in compiler");
}

void Compiler::replaceOperand(int pos, int operand) {
    Opcode op = static_cast<Opcode>(instructions_[pos]);
    auto ins = Make(op, {operand});
    replaceInstruction(pos, ins);
}

void Compiler::replaceInstruction(int pos, const Instructions& newIns) {
    for (size_t i = 0; i < newIns.size(); i++) {
        instructions_[pos + i] = newIns[i];
    }
}

// ============ Debug info from node ============

DebugEntry tokenInfoFromNode(Node* node) {
    DebugEntry entry;
    Token t;

    if (dynamic_cast<Program*>(node)) {
        entry.function = "<module>";
        return entry;
    }

    #define EXTRACT_TOKEN(ASTType, field) \
        if (auto n = dynamic_cast<ASTType*>(node)) { t = n->field; }

    EXTRACT_TOKEN(ExpressionStatement, token)
    else EXTRACT_TOKEN(LetStatement, token)
    else EXTRACT_TOKEN(AssignStatement, token)
    else EXTRACT_TOKEN(ReturnStatement, token)
    else EXTRACT_TOKEN(BlockStatement, token)
    else EXTRACT_TOKEN(StandaloneBlockStatement, token)
    else EXTRACT_TOKEN(WhileStatement, token)
    else EXTRACT_TOKEN(ForStatement, token)
    else EXTRACT_TOKEN(FunctionDeclaration, token)
    else EXTRACT_TOKEN(Identifier, token)
    else EXTRACT_TOKEN(IntegerLiteral, token)
    else EXTRACT_TOKEN(FloatLiteral, token)
    else EXTRACT_TOKEN(StringLiteral, token)
    else EXTRACT_TOKEN(BooleanLiteral, token)
    else EXTRACT_TOKEN(NullLiteral, token)
    else EXTRACT_TOKEN(PrefixExpression, token)
    else EXTRACT_TOKEN(InfixExpression, token)
    else EXTRACT_TOKEN(IfExpression, token)
    else EXTRACT_TOKEN(FunctionLiteral, token)
    else EXTRACT_TOKEN(CallExpression, token)
    else EXTRACT_TOKEN(ArrayLiteral, token)
    else EXTRACT_TOKEN(MapLiteral, token)
    else EXTRACT_TOKEN(IndexExpression, token)

    #undef EXTRACT_TOKEN

    entry.file = t.file;
    entry.line = t.line;
    entry.column = t.column;
    if (auto fd = dynamic_cast<FunctionDeclaration*>(node)) {
        entry.function = fd->name ? fd->name->value : "<func>";
    } else if (dynamic_cast<FunctionLiteral*>(node)) {
        entry.function = "<lambda>";
    } else {
        entry.function = "<module>";
    }
    return entry;
}

// ============ Constant folding ============

ObjectPtr foldConstExpr(Node* node, bool* ok) {
    *ok = false;

    if (auto intLit = dynamic_cast<IntegerLiteral*>(node)) { *ok = true; return newInteger(intLit->value); }
    if (auto floatLit = dynamic_cast<FloatLiteral*>(node)) { *ok = true; return newFloat(floatLit->value); }
    if (auto strLit = dynamic_cast<StringLiteral*>(node)) { *ok = true; return newString(strLit->value); }
    if (auto boolLit = dynamic_cast<BooleanLiteral*>(node)) { *ok = true; return newBoolean(boolLit->value); }

    if (auto prefix = dynamic_cast<PrefixExpression*>(node)) {
        bool rightOk = false;
        auto right = foldConstExpr(prefix->right.get(), &rightOk);
        if (!rightOk) return nullptr;
        if (prefix->op == "-") {
            if (auto i = std::dynamic_pointer_cast<Integer>(right)) { *ok = true; return newInteger(-i->value); }
            if (auto f = std::dynamic_pointer_cast<Float>(right)) { *ok = true; return newFloat(-f->value); }
        }
        if (prefix->op == "!") {
            if (auto b = std::dynamic_pointer_cast<Boolean>(right)) { *ok = true; return newBoolean(!b->value); }
        }
        return nullptr;
    }

    if (auto infix = dynamic_cast<InfixExpression*>(node)) {
        bool leftOk = false, rightOk = false;
        auto left = foldConstExpr(infix->left.get(), &leftOk);
        auto right = foldConstExpr(infix->right.get(), &rightOk);
        if (!leftOk || !rightOk) return nullptr;

        const auto& op = infix->op;

        if (op == "+" || op == "-" || op == "*" || op == "/") {
            if (auto l = std::dynamic_pointer_cast<Integer>(left)) {
                if (auto r = std::dynamic_pointer_cast<Integer>(right)) {
                    *ok = true;
                    if (op == "+") return newInteger(l->value + r->value);
                    if (op == "-") return newInteger(l->value - r->value);
                    if (op == "*") return newInteger(l->value * r->value);
                    if (op == "/" && r->value != 0) return newInteger(l->value / r->value);
                }
                if (auto r = std::dynamic_pointer_cast<Float>(right)) {
                    *ok = true;
                    if (op == "+") return newFloat(l->value + r->value);
                    if (op == "-") return newFloat(l->value - r->value);
                    if (op == "*") return newFloat(l->value * r->value);
                    if (op == "/" && r->value != 0) return newFloat(l->value / r->value);
                }
            }
            if (auto l = std::dynamic_pointer_cast<Float>(left)) {
                if (auto r = std::dynamic_pointer_cast<Integer>(right)) {
                    *ok = true;
                    if (op == "+") return newFloat(l->value + r->value);
                    if (op == "-") return newFloat(l->value - r->value);
                    if (op == "*") return newFloat(l->value * r->value);
                    if (op == "/" && r->value != 0) return newFloat(l->value / r->value);
                }
                if (auto r = std::dynamic_pointer_cast<Float>(right)) {
                    *ok = true;
                    if (op == "+") return newFloat(l->value + r->value);
                    if (op == "-") return newFloat(l->value - r->value);
                    if (op == "*") return newFloat(l->value * r->value);
                    if (op == "/" && r->value != 0) return newFloat(l->value / r->value);
                }
            }
            if (op == "+") {
                if (auto l = std::dynamic_pointer_cast<String>(left)) {
                    if (auto r = std::dynamic_pointer_cast<String>(right)) {
                        *ok = true;
                        return newString(l->value + r->value);
                    }
                }
            }
        }

        if (op == "==") {
            if (auto l = std::dynamic_pointer_cast<Integer>(left)) {
                if (auto r = std::dynamic_pointer_cast<Integer>(right)) { *ok = true; return newBoolean(l->value == r->value); }
                if (auto r = std::dynamic_pointer_cast<Float>(right)) { *ok = true; return newBoolean(l->value == r->value); }
            }
            if (auto l = std::dynamic_pointer_cast<Float>(left)) {
                if (auto r = std::dynamic_pointer_cast<Integer>(right)) { *ok = true; return newBoolean(l->value == r->value); }
                if (auto r = std::dynamic_pointer_cast<Float>(right)) { *ok = true; return newBoolean(l->value == r->value); }
            }
            if (auto l = std::dynamic_pointer_cast<String>(left)) {
                if (auto r = std::dynamic_pointer_cast<String>(right)) { *ok = true; return newBoolean(l->value == r->value); }
            }
            if (auto l = std::dynamic_pointer_cast<Boolean>(left)) {
                if (auto r = std::dynamic_pointer_cast<Boolean>(right)) { *ok = true; return newBoolean(l->value == r->value); }
            }
        }
        if (op == "!=") {
            if (auto l = std::dynamic_pointer_cast<Integer>(left)) {
                if (auto r = std::dynamic_pointer_cast<Integer>(right)) { *ok = true; return newBoolean(l->value != r->value); }
            }
            if (auto l = std::dynamic_pointer_cast<String>(left)) {
                if (auto r = std::dynamic_pointer_cast<String>(right)) { *ok = true; return newBoolean(l->value != r->value); }
            }
        }
        if (op == ">" || op == "<") {
            if (auto l = std::dynamic_pointer_cast<Integer>(left)) {
                if (auto r = std::dynamic_pointer_cast<Integer>(right)) {
                    *ok = true;
                    return newBoolean(op == ">" ? l->value > r->value : l->value < r->value);
                }
            }
        }
    }

    return nullptr;
}

// ============ Peephole optimizer ============

Instructions peephole(const Instructions& ins) {
    if (ins.empty()) return ins;
    Instructions out(ins.size());
    std::copy(ins.begin(), ins.end(), out.begin());

    for (size_t i = 0; i < out.size();) {
        Opcode op = static_cast<Opcode>(out[i]);
        switch (op) {
            case Opcode::OpJump: {
                auto def = Lookup(Opcode::OpJump);
                auto [operands, read] = ReadOperands(def, out.data() + i + 1, out.size() - i - 1);
                int target = operands[0];
                if (target == static_cast<int>(i + 3)) {
                    out[i] = out[i+1] = out[i+2] = static_cast<uint8_t>(Opcode::OpNop);
                }
                i += 3;
                break;
            }
            case Opcode::OpJumpNotTruthy: {
                auto def = Lookup(Opcode::OpJumpNotTruthy);
                auto [operands, read] = ReadOperands(def, out.data() + i + 1, out.size() - i - 1);
                int target = operands[0];
                if (target == static_cast<int>(i + 3)) {
                    out[i] = static_cast<uint8_t>(Opcode::OpPop);
                    out[i+1] = out[i+2] = static_cast<uint8_t>(Opcode::OpNop);
                }
                i += 3;
                break;
            }
            case Opcode::OpConstant: {
                if (i + 3 < out.size() && static_cast<Opcode>(out[i+3]) == Opcode::OpPop) {
                    out[i] = out[i+1] = out[i+2] = out[i+3] = static_cast<uint8_t>(Opcode::OpNop);
                    i += 4;
                    continue;
                }
                i += 3;
                break;
            }
            default: {
                auto def = Lookup(op);
                if (!def) { i++; continue; }
                i += 1;
                for (int w : def->operandWidths) i += w;
                break;
            }
        }
    }
    return out;
}

} // namespace darix
