#include "darix/vm.hpp"
#include <algorithm>
#include <cstring>
#include <sstream>

namespace darix {

// ============ HotPath ============

struct HotPath {
    int startIP = 0;
    int endIP = 0;
    int executeCount = 0;
    Instructions instructions;
    bool compiled = false;
};

// ============ Helpers ============

static uint16_t readUint16(const uint8_t* ins) {
    return static_cast<uint16_t>(ins[0]) << 8 | static_cast<uint16_t>(ins[1]);
}

static bool isError(ObjectPtr obj) { return obj && obj->type() == ObjectType::ERROR; }
static bool isSignal(ObjectPtr obj) { return obj && obj->type() == ObjectType::EXCEPTION_SIGNAL; }
static ObjectPtr nativeBoolToBooleanObject(bool b) { return b ? getTrue() : getFalse(); }

// ============ VM ============

VM::VM(std::shared_ptr<Bytecode> bc)
    : constants_(bc->constants)
    , globals_(InitialGlobs, nullptr)
    , stack_(StackSize, nullptr)
    , instructions_(bc->instructions)
    , bcMagic_(bc->magic)
    , bcVersion_(bc->version)
    , debug_(bc->debug)
{
}

void VM::setInstructionBudget(int n) { instrBudget_ = n; }
void VM::enableJIT(bool) {}
void VM::enableProfiling(bool enabled) { profiling_ = enabled; }

ObjectPtr VM::push(ObjectPtr obj) {
    if (sp_ >= StackSize) return errorWithLoc("stack overflow");
    stack_[sp_] = obj;
    sp_++;
    return nullptr;
}

ObjectPtr VM::pop() {
    if (sp_ == 0) return errorWithLoc("stack underflow");
    sp_--;
    auto obj = stack_[sp_];
    stack_[sp_] = nullptr;
    return obj;
}

std::pair<ObjectPtr, ObjectPtr> VM::popChecked() {
    auto obj = pop();
    if (isError(obj)) return {nullptr, obj};
    return {obj, nullptr};
}

std::tuple<ObjectPtr, ObjectPtr, ObjectPtr> VM::popTwo() {
    auto [right, err] = popChecked(); if (err) return {nullptr, nullptr, err};
    auto [left, err2] = popChecked(); if (err2) return {nullptr, nullptr, err2};
    return {left, right, nullptr};
}

std::tuple<ObjectPtr, ObjectPtr, ObjectPtr, ObjectPtr> VM::popThree() {
    auto [third, err1] = popChecked(); if (err1) return {nullptr, nullptr, nullptr, err1};
    auto [second, err2] = popChecked(); if (err2) return {nullptr, nullptr, nullptr, err2};
    auto [first, err3] = popChecked(); if (err3) return {nullptr, nullptr, nullptr, err3};
    return {first, second, third, nullptr};
}

ObjectPtr VM::pushChecked(ObjectPtr obj) {
    if (auto err = push(obj)) return err;
    return nullptr;
}

ObjectPtr VM::binaryOp(Opcode op) {
    auto [left, right, err] = popTwo();
    if (err) return err;
    auto res = execBinary(op, left, right);
    if (isError(res)) return res;
    return pushChecked(res);
}

ObjectPtr VM::compareOp(Opcode op) {
    auto [left, right, err] = popTwo();
    if (err) return err;
    auto res = execCompare(op, left, right);
    if (isError(res)) return res;
    return pushChecked(res);
}

ObjectPtr VM::run() {
    if (!bcMagic_.empty() && bcMagic_ != BytecodeMagic) {
        return newError("invalid bytecode: magic mismatch");
    }

    for (ip_ = 0; ip_ < static_cast<int>(instructions_.size()); ip_++) {
        if (instrBudget_ > 0) {
            instrBudget_--;
            if (instrBudget_ == 0) {
                auto ex = std::dynamic_pointer_cast<Exception>(newException(RUNTIME_ERROR, "instruction budget exceeded"));
                ex->stackTrace = buildStackTrace();
                return newExceptionSignal(ex);
            }
        }

        auto op = static_cast<Opcode>(instructions_[ip_]);
        if (profiling_) opCounts_[static_cast<int>(op)]++;

        switch (op) {
            case Opcode::OpNop: break;
            case Opcode::OpConstant: {
                int operand = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                if (auto err = pushChecked(constants_[operand])) return err;
                break;
            }
            case Opcode::OpAdd: case Opcode::OpSub: case Opcode::OpMul:
            case Opcode::OpDiv: case Opcode::OpMod:
                if (auto err = binaryOp(op)) return err;
                break;
            case Opcode::OpEqual: case Opcode::OpNotEqual:
            case Opcode::OpGreaterThan: case Opcode::OpLessThan:
            case Opcode::OpGreaterEqual: case Opcode::OpLessEqual:
                if (auto err = compareOp(op)) return err;
                break;
            case Opcode::OpMinus: {
                auto [operand, err] = popChecked();
                if (err) return err;
                auto res = execMinus(operand);
                if (isError(res)) return res;
                if (auto e = pushChecked(res)) return e;
                break;
            }
            case Opcode::OpBang: {
                auto [operand, err] = popChecked();
                if (err) return err;
                if (auto e = pushChecked(nativeBoolToBooleanObject(!isTruthy(operand)))) return e;
                break;
            }
            case Opcode::OpTrue:
                if (auto e = pushChecked(getTrue())) return e;
                break;
            case Opcode::OpFalse:
                if (auto e = pushChecked(getFalse())) return e;
                break;
            case Opcode::OpNull:
                if (auto e = pushChecked(getNull())) return e;
                break;
            case Opcode::OpPop: {
                auto [_, err] = popChecked();
                if (err) return err;
                break;
            }
            case Opcode::OpSetGlobal: {
                int idx = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                auto [val, err] = popChecked();
                if (err) return err;
                setGlobal(idx, val);
                break;
            }
            case Opcode::OpGetGlobal: {
                int idx = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                if (auto e = pushChecked(getGlobal(idx))) return e;
                break;
            }
            case Opcode::OpPrint: {
                int argc = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                if (auto err = opPrint(argc)) return err;
                break;
            }
            case Opcode::OpJump: {
                int pos = readUint16(instructions_.data() + ip_ + 1);
                ip_ = pos - 1;
                break;
            }
            case Opcode::OpJumpNotTruthy: {
                int pos = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                auto [cond, err] = popChecked();
                if (err) return err;
                if (!isTruthy(cond)) ip_ = pos - 1;
                break;
            }
            case Opcode::OpArray: {
                int numElements = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                if (auto err = opArray(numElements)) return err;
                break;
            }
            case Opcode::OpStringConcat: {
                int n = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                if (auto err = opStringConcat(n)) return err;
                break;
            }
            case Opcode::OpIndex: {
                auto [left, right, err] = popTwo();
                if (err) return err;
                auto res = execIndex(left, right);
                if (isError(res)) return res;
                if (auto e = pushChecked(res)) return e;
                break;
            }
            case Opcode::OpSetIndex: {
                auto [target, index, value, err] = popThree();
                if (err) return err;
                auto setErr = execSetIndex(target, index, value);
                if (setErr) return setErr;
                if (auto e = pushChecked(getNull())) return e;
                break;
            }
            case Opcode::OpLen: {
                auto [obj, err] = popChecked();
                if (err) return err;
                auto res = execLen(obj);
                if (isError(res)) return res;
                if (auto e = pushChecked(res)) return e;
                break;
            }
            case Opcode::OpType: {
                auto [obj, err] = popChecked();
                if (err) return err;
                auto res = execType(obj);
                if (auto e = pushChecked(res)) return e;
                break;
            }
            case Opcode::OpSwap:
                if (auto err = opSwap()) return err;
                break;
            case Opcode::OpCall: {
                int argc = readUint16(instructions_.data() + ip_ + 1);
                ip_ += 2;
                if (sp_ < argc + 1) return errorWithLoc("call: stack underflow");
                std::vector<ObjectPtr> args(argc);
                for (int i = argc - 1; i >= 0; i--) {
                    auto [val, err] = popChecked();
                    if (err) return err;
                    args[i] = val;
                }
                auto [callee, calleeErr] = popChecked();
                if (calleeErr) return calleeErr;

                if (auto fn = std::dynamic_pointer_cast<CompiledFunction>(callee)) {
                    if (static_cast<int>(args.size()) != fn->numParameters) {
                        return errorWithLoc("wrong number of arguments");
                    }
                    auto res = runCompiledFunction(fn, args);
                    if (isError(res) || isSignal(res)) return res;
                    if (auto err = push(res)) return err;
                } else if (auto builtin = std::dynamic_pointer_cast<Builtin>(callee)) {
                    auto res = builtin->fn(args);
                    if (isError(res) || isSignal(res)) return res;
                    if (auto err = push(res)) return err;
                } else {
                    return errorWithLoc("not a function");
                }
                break;
            }
            default:
                return errorWithLoc("unknown opcode");
        }
    }
    return getNull();
}

// ============ VM operations ============

ObjectPtr VM::execBinary(Opcode op, ObjectPtr left, ObjectPtr right) {
    if (auto l = std::dynamic_pointer_cast<Integer>(left)) {
        if (auto r = std::dynamic_pointer_cast<Integer>(right)) {
            switch (op) {
                case Opcode::OpAdd: return addIntegers(l, r);
                case Opcode::OpSub: return subIntegers(l, r);
                case Opcode::OpMul: return mulIntegers(l, r);
                case Opcode::OpDiv: return divIntegers(l, r);
                case Opcode::OpMod: return modIntegers(l, r);
                default: break;
            }
        }
    }
    if (auto l = std::dynamic_pointer_cast<Float>(left)) {
        if (auto r = std::dynamic_pointer_cast<Float>(right)) {
            switch (op) {
                case Opcode::OpAdd: return addFloats(l, r);
                case Opcode::OpSub: return subFloats(l, r);
                case Opcode::OpMul: return mulFloats(l, r);
                case Opcode::OpDiv: return divFloats(l, r);
                default: break;
            }
        }
    }
    if (op == Opcode::OpAdd) {
        if (auto l = std::dynamic_pointer_cast<String>(left)) {
            if (auto r = std::dynamic_pointer_cast<String>(right)) {
                return concatStrings(l, r);
            }
        }
    }
    return errorWithLoc("unsupported operands for binary op");
}

ObjectPtr VM::execCompare(Opcode op, ObjectPtr left, ObjectPtr right) {
    if (auto l = std::dynamic_pointer_cast<Integer>(left)) {
        if (auto r = std::dynamic_pointer_cast<Integer>(right)) {
            switch (op) {
                case Opcode::OpEqual: return nativeBoolToBooleanObject(l->value == r->value);
                case Opcode::OpNotEqual: return nativeBoolToBooleanObject(l->value != r->value);
                case Opcode::OpGreaterThan: return nativeBoolToBooleanObject(l->value > r->value);
                case Opcode::OpLessThan: return nativeBoolToBooleanObject(l->value < r->value);
                case Opcode::OpGreaterEqual: return nativeBoolToBooleanObject(l->value >= r->value);
                case Opcode::OpLessEqual: return nativeBoolToBooleanObject(l->value <= r->value);
                default: break;
            }
        }
    }
    if (auto l = std::dynamic_pointer_cast<Float>(left)) {
        if (auto r = std::dynamic_pointer_cast<Float>(right)) {
            switch (op) {
                case Opcode::OpEqual: return nativeBoolToBooleanObject(l->value == r->value);
                case Opcode::OpNotEqual: return nativeBoolToBooleanObject(l->value != r->value);
                case Opcode::OpGreaterThan: return nativeBoolToBooleanObject(l->value > r->value);
                case Opcode::OpLessThan: return nativeBoolToBooleanObject(l->value < r->value);
                case Opcode::OpGreaterEqual: return nativeBoolToBooleanObject(l->value >= r->value);
                case Opcode::OpLessEqual: return nativeBoolToBooleanObject(l->value <= r->value);
                default: break;
            }
        }
    }
    if (auto l = std::dynamic_pointer_cast<String>(left)) {
        if (auto r = std::dynamic_pointer_cast<String>(right)) {
            switch (op) {
                case Opcode::OpEqual: return nativeBoolToBooleanObject(l->value == r->value);
                case Opcode::OpNotEqual: return nativeBoolToBooleanObject(l->value != r->value);
                case Opcode::OpGreaterThan: return nativeBoolToBooleanObject(l->value > r->value);
                case Opcode::OpLessThan: return nativeBoolToBooleanObject(l->value < r->value);
                default: break;
            }
        }
    }
    if (auto l = std::dynamic_pointer_cast<Boolean>(left)) {
        if (auto r = std::dynamic_pointer_cast<Boolean>(right)) {
            if (op == Opcode::OpEqual) return nativeBoolToBooleanObject(l->value == r->value);
            if (op == Opcode::OpNotEqual) return nativeBoolToBooleanObject(l->value != r->value);
        }
    }
    return errorWithLoc("unsupported operands for compare");
}

ObjectPtr VM::execMinus(ObjectPtr operand) {
    if (auto o = std::dynamic_pointer_cast<Integer>(operand)) return newIntegerFromPool(-o->value);
    if (auto o = std::dynamic_pointer_cast<Float>(operand)) return newFloatFromPool(-o->value);
    return errorWithLoc("unsupported operand for prefix -");
}

ObjectPtr VM::execIndex(ObjectPtr left, ObjectPtr index) {
    if (left->type() == ObjectType::ARRAY && index->type() == ObjectType::INTEGER) {
        auto arr = std::dynamic_pointer_cast<Array>(left);
        auto idx = std::dynamic_pointer_cast<Integer>(index)->value;
        if (idx < 0 || idx >= static_cast<int64_t>(arr->elements.size())) return getNull();
        return arr->elements[idx];
    }
    if (left->type() == ObjectType::MAP) {
        auto m = std::dynamic_pointer_cast<Map>(left);
        for (const auto& [k, v] : m->pairs) {
            if (equals(k, index)) return v;
        }
        return getNull();
    }
    if (left->type() == ObjectType::STRING && index->type() == ObjectType::INTEGER) {
        auto s = std::dynamic_pointer_cast<String>(left);
        auto idx = std::dynamic_pointer_cast<Integer>(index)->value;
        if (idx < 0 || idx >= static_cast<int64_t>(s->value.size())) return getNull();
        return newStringFromPool(std::string(1, s->value[idx]));
    }
    return errorWithLoc("index operator not supported");
}

ObjectPtr VM::execSetIndex(ObjectPtr target, ObjectPtr index, ObjectPtr value) {
    if (auto arr = std::dynamic_pointer_cast<Array>(target)) {
        auto idx = std::dynamic_pointer_cast<Integer>(index);
        if (!idx) return errorWithLoc("array index must be integer");
        if (idx->value < 0 || idx->value >= static_cast<int64_t>(arr->elements.size())) {
            auto ex = std::dynamic_pointer_cast<Exception>(newException(INDEX_ERROR, "array index out of range: " + std::to_string(idx->value)));
            ex->stackTrace = buildStackTrace();
            return newExceptionSignal(ex);
        }
        arr->elements[idx->value] = value;
        return nullptr;
    }
    if (auto m = std::dynamic_pointer_cast<Map>(target)) {
        for (auto it = m->pairs.begin(); it != m->pairs.end(); ++it) {
            if (equals(it->first, index)) {
                m->pairs.erase(it);
                m->pairs.push_back({index, value});
                return nullptr;
            }
        }
        m->pairs.push_back({index, value});
        return nullptr;
    }
    return errorWithLoc("index assignment not supported");
}

ObjectPtr VM::execLen(ObjectPtr obj) {
    if (auto arr = std::dynamic_pointer_cast<Array>(obj))
        return newIntegerFromPool(static_cast<int64_t>(arr->elements.size()));
    if (auto s = std::dynamic_pointer_cast<String>(obj))
        return newIntegerFromPool(static_cast<int64_t>(s->value.size()));
    if (auto m = std::dynamic_pointer_cast<Map>(obj))
        return newIntegerFromPool(static_cast<int64_t>(m->pairs.size()));
    return errorWithLoc("argument to len not supported");
}

ObjectPtr VM::execType(ObjectPtr obj) {
    return newStringFromPool(ObjectTypeToString(obj->type()));
}

ObjectPtr VM::opPrint(int argc) {
    std::string out;
    std::vector<ObjectPtr> args(argc);
    for (int i = argc - 1; i >= 0; i--) {
        auto [val, err] = popChecked();
        if (err) return err;
        args[i] = val;
    }
    for (int i = 0; i < argc; i++) {
        if (i > 0) out += " ";
        out += args[i]->inspect();
    }
    std::printf("%s\n", out.c_str());
    return nullptr;
}

ObjectPtr VM::opArray(int numElements) {
    std::vector<ObjectPtr> elements(numElements);
    for (int i = numElements - 1; i >= 0; i--) {
        auto [val, err] = popChecked();
        if (err) return err;
        elements[i] = val;
    }
    return newArrayFromPool(std::move(elements));
}

ObjectPtr VM::opStringConcat(int n) {
    std::vector<std::shared_ptr<String>> parts(n);
    for (int i = n - 1; i >= 0; i--) {
        auto [val, err] = popChecked();
        if (err) return err;
        parts[i] = std::dynamic_pointer_cast<String>(val);
    }
    return concatMultipleStrings(parts);
}

ObjectPtr VM::opSwap() {
    if (sp_ < 2) return errorWithLoc("swap: stack underflow");
    std::swap(stack_[sp_ - 1], stack_[sp_ - 2]);
    return nullptr;
}

ObjectPtr VM::runCompiledFunction(std::shared_ptr<CompiledFunction> fn, const std::vector<ObjectPtr>& args) {
    std::vector<ObjectPtr> locals(fn->numLocals, nullptr);
    for (int i = 0; i < fn->numParameters && i < static_cast<int>(args.size()); i++) {
        locals[i] = args[i];
    }

    int ip = 0;
    const auto& ins = fn->instructions;

    auto read16 = [&](int offset) -> int {
        return static_cast<int>(static_cast<uint16_t>(ins[offset]) << 8 | static_cast<uint16_t>(ins[offset + 1]));
    };

    while (ip < static_cast<int>(ins.size())) {
        if (instrBudget_ > 0) {
            instrBudget_--;
            if (instrBudget_ == 0) {
                auto ex = std::dynamic_pointer_cast<Exception>(newException(RUNTIME_ERROR, "instruction budget exceeded"));
                ex->stackTrace = buildStackTrace();
                return newExceptionSignal(ex);
            }
        }

        auto op = static_cast<Opcode>(ins[ip]);
        switch (op) {
            case Opcode::OpNop: break;
            case Opcode::OpConstant: {
                int idx = read16(ip + 1); ip += 2;
                if (auto err = push(constants_[idx])) return err;
                break;
            }
            case Opcode::OpAdd: case Opcode::OpSub: case Opcode::OpMul:
            case Opcode::OpDiv: case Opcode::OpMod:
                if (auto err = binaryOp(op)) return err;
                break;
            case Opcode::OpEqual: case Opcode::OpNotEqual:
            case Opcode::OpGreaterThan: case Opcode::OpLessThan:
            case Opcode::OpGreaterEqual: case Opcode::OpLessEqual:
                if (auto err = compareOp(op)) return err;
                break;
            case Opcode::OpMinus: {
                auto [operand, err] = popChecked();
                if (err) return err;
                auto res = execMinus(operand);
                if (isError(res)) return res;
                if (auto e = push(res)) return e;
                break;
            }
            case Opcode::OpBang: {
                auto [operand, err] = popChecked();
                if (err) return err;
                if (auto e = push(nativeBoolToBooleanObject(!isTruthy(operand)))) return e;
                break;
            }
            case Opcode::OpTrue: if (auto e = push(getTrue())) return e; break;
            case Opcode::OpFalse: if (auto e = push(getFalse())) return e; break;
            case Opcode::OpNull: if (auto e = push(getNull())) return e; break;
            case Opcode::OpPop: { auto [_, err] = popChecked(); if (err) return err; break; }
            case Opcode::OpSetGlobal: {
                int idx = read16(ip + 1); ip += 2;
                auto [val, err] = popChecked(); if (err) return err;
                setGlobal(idx, val);
                break;
            }
            case Opcode::OpGetGlobal: {
                int idx = read16(ip + 1); ip += 2;
                if (auto err = push(getGlobal(idx))) return err;
                break;
            }
            case Opcode::OpPrint: {
                int argc = read16(ip + 1); ip += 2;
                if (auto err = opPrint(argc)) return err;
                break;
            }
            case Opcode::OpJump: {
                int target = read16(ip + 1);
                ip = target - 1;
                break;
            }
            case Opcode::OpJumpNotTruthy: {
                int target = read16(ip + 1);
                ip += 2;
                auto [cond, err] = popChecked(); if (err) return err;
                if (!isTruthy(cond)) ip = target - 1;
                break;
            }
            case Opcode::OpArray: {
                int num = read16(ip + 1); ip += 2;
                if (auto err = opArray(num)) return err;
                break;
            }
            case Opcode::OpStringConcat: {
                int n = read16(ip + 1); ip += 2;
                if (auto err = opStringConcat(n)) return err;
                break;
            }
            case Opcode::OpIndex: {
                auto [left, right, err] = popTwo(); if (err) return err;
                auto res = execIndex(left, right);
                if (isError(res)) return res;
                if (auto e = push(res)) return e;
                break;
            }
            case Opcode::OpSetIndex: {
                auto [t, i, v, err] = popThree(); if (err) return err;
                if (auto setErr = execSetIndex(t, i, v)) return setErr;
                if (auto e = push(getNull())) return e;
                break;
            }
            case Opcode::OpLen: {
                auto [o, err] = popChecked(); if (err) return err;
                auto res = execLen(o);
                if (isError(res)) return res;
                if (auto e = push(res)) return e;
                break;
            }
            case Opcode::OpType: {
                auto [o, err] = popChecked(); if (err) return err;
                if (auto e = push(execType(o))) return e;
                break;
            }
            case Opcode::OpSwap:
                if (auto err = opSwap()) return err;
                break;
            case Opcode::OpReturnValue: {
                auto [val, err] = popChecked(); if (err) return err;
                return val;
            }
            case Opcode::OpReturn:
                return getNull();
            case Opcode::OpCall: {
                int argc = read16(ip + 1); ip += 2;
                std::vector<ObjectPtr> argv(argc);
                for (int i = argc - 1; i >= 0; i--) {
                    auto [val, err] = popChecked(); if (err) return err;
                    argv[i] = val;
                }
                auto [callee, calleeErr] = popChecked(); if (calleeErr) return calleeErr;

                if (auto fn2 = std::dynamic_pointer_cast<CompiledFunction>(callee)) {
                    if (static_cast<int>(argv.size()) != fn2->numParameters) {
                        return errorWithLoc("wrong number of arguments");
                    }
                    auto res = runCompiledFunction(fn2, argv);
                    if (isError(res) || isSignal(res)) return res;
                    if (auto err = push(res)) return err;
                } else if (auto builtin = std::dynamic_pointer_cast<Builtin>(callee)) {
                    auto res = builtin->fn(argv);
                    if (isError(res) || isSignal(res)) return res;
                    if (auto err = push(res)) return err;
                } else {
                    return errorWithLoc("not a function");
                }
                break;
            }
            case Opcode::OpGetLocal: {
                int idx = read16(ip + 1); ip += 2;
                if (idx < 0 || idx >= static_cast<int>(locals.size())) return errorWithLoc("getlocal: index out of range");
                if (auto err = push(locals[idx])) return err;
                break;
            }
            case Opcode::OpSetLocal: {
                int idx = read16(ip + 1); ip += 2;
                auto [val, err] = popChecked(); if (err) return err;
                if (idx < 0 || idx >= static_cast<int>(locals.size())) return errorWithLoc("setlocal: index out of range");
                locals[idx] = val;
                break;
            }
            default:
                return errorWithLoc("unknown opcode");
        }
        ip++;
    }
    return getNull();
}

void VM::setGlobal(int idx, ObjectPtr val) {
    if (idx >= static_cast<int>(globals_.size())) {
        globals_.resize(idx + 1, nullptr);
    }
    globals_[idx] = val;
}

ObjectPtr VM::getGlobal(int idx) {
    if (idx >= static_cast<int>(globals_.size()) || !globals_[idx]) return getNull();
    return globals_[idx];
}

ObjectPtr VM::errorWithLoc(const std::string& msg) {
    std::string file;
    int line = 0, col = 0;
    std::string fn;
    lookupDebug(ip_, file, line, col, fn);
    std::string fullMsg;
    if (!file.empty() || line > 0 || col > 0) {
        fullMsg = file + ":" + std::to_string(line) + ":" + std::to_string(col) + ": " + msg;
    } else {
        fullMsg = msg;
    }
    return newError("%s", fullMsg.c_str());
}

std::shared_ptr<StackTrace> VM::buildStackTrace() {
    auto f = currentFrame();
    auto st = std::make_shared<StackTrace>();
    if (f) st->frames.push_back(*f);
    return st;
}

std::shared_ptr<StackFrame> VM::currentFrame() {
    std::string file, fn;
    int line = 0, col = 0;
    lookupDebug(ip_, file, line, col, fn);
    if (file.empty() && line == 0 && col == 0) return nullptr;
    auto frame = std::make_shared<StackFrame>();
    frame->functionName = fn;
    frame->position = {file, line, col};
    return frame;
}

void VM::lookupDebug(int ip, std::string& file, int& line, int& col, std::string& fn) {
    int bestIdx = -1;
    int bestPC = -1;
    for (size_t i = 0; i < debug_.entries.size(); i++) {
        int pc = debug_.entries[i].pc;
        if (pc <= ip && pc >= bestPC) {
            bestPC = pc;
            bestIdx = static_cast<int>(i);
        }
    }
    if (bestIdx == -1) { file = ""; line = col = 0; fn = ""; return; }
    auto& e = debug_.entries[bestIdx];
    file = e.file;
    line = e.line;
    col = e.column;
    fn = e.function.empty() ? "<module>" : e.function;
}

// JIT stubs
std::shared_ptr<HotPath> VM::jitGetCompiledPath(int) { return nullptr; }
bool VM::jitRecordExecution(int) { return false; }
std::shared_ptr<HotPath> VM::jitShouldCompile(int) { return nullptr; }
void VM::jitCompileHotPath(std::shared_ptr<HotPath>) {}
ObjectPtr VM::jitExecuteCompiledPath(std::shared_ptr<HotPath>) { return nullptr; }

} // namespace darix
