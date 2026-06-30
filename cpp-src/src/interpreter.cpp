#include "darix/interpreter.hpp"
#include "darix/lexer.hpp"
#include "darix/parser.hpp"
#include "darix/native/native.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace darix {

static ObjectPtr nativeBoolToBooleanObject(bool b) { return b ? getTrue() : getFalse(); }

static int64_t asInt(ObjectPtr obj) {
    if (auto i = std::dynamic_pointer_cast<Integer>(obj)) return i->value;
    if (auto f = std::dynamic_pointer_cast<Float>(obj)) return static_cast<int64_t>(f->value);
    return 0;
}

static int compareObjects(ObjectPtr a, ObjectPtr b) {
    if (auto ai = std::dynamic_pointer_cast<Integer>(a)) {
        if (auto bi = std::dynamic_pointer_cast<Integer>(b))
            return (ai->value > bi->value) - (ai->value < bi->value);
        if (auto bf = std::dynamic_pointer_cast<Float>(b)) {
            double diff = ai->value - bf->value;
            return (diff > 0) - (diff < 0);
        }
    }
    if (auto af = std::dynamic_pointer_cast<Float>(a)) {
        if (auto bi = std::dynamic_pointer_cast<Integer>(b)) {
            double diff = af->value - bi->value;
            return (diff > 0) - (diff < 0);
        }
        if (auto bf = std::dynamic_pointer_cast<Float>(b))
            return (af->value > bf->value) - (af->value < bf->value);
    }
    if (auto as = std::dynamic_pointer_cast<String>(a))
        if (auto bs = std::dynamic_pointer_cast<String>(b))
            return as->value.compare(bs->value);
    return 0;
}

// ============ Interpreter ============

Interpreter::Interpreter() {
    env_ = newEnvironment();
    native::Registry::instance().initAll();
    // Provide callback so native modules can evaluate user-defined functions
    native::Registry::instance().setEvalCallback(
        [this](ObjectPtr callable, const std::vector<ObjectPtr>& args) -> ObjectPtr {
            return applyFunction(callable, args);
        });
    initBuiltins();
}
ObjectPtr Interpreter::interpret(Program* program) { return evalProgram(program, env_); }

bool Interpreter::isError(ObjectPtr obj) { return obj && obj->type() == ObjectType::ERROR; }
bool Interpreter::isSignal(ObjectPtr obj) {
    if (!obj) return false;
    auto t = obj->type();
    return t == ObjectType::EXCEPTION_SIGNAL || t == ObjectType::BREAK_SIGNAL || t == ObjectType::CONTINUE_SIGNAL;
}
ObjectPtr Interpreter::builtinError(const std::string& name, const std::string& format) {
    return newError("%s: %s", name.c_str(), format.c_str());
}

// ============ Main eval dispatcher ============

ObjectPtr Interpreter::eval(Node* node, std::shared_ptr<Environment> env) {
    if (!node) return getNull();

    if (auto p = dynamic_cast<Program*>(node)) return evalProgram(p, env);
    if (auto es = dynamic_cast<ExpressionStatement*>(node)) {
        auto r = eval(es->expression.get(), env);
        if (auto s = std::dynamic_pointer_cast<ExceptionSignal>(r)) return s;
        return r;
    }
    if (dynamic_cast<BreakStatement*>(node)) return std::make_shared<BreakSignal>();
    if (dynamic_cast<ContinueStatement*>(node)) return std::make_shared<ContinueSignal>();
    if (auto ws = dynamic_cast<WhileStatement*>(node)) return evalWhile(ws, env);
    if (auto fs = dynamic_cast<ForStatement*>(node)) return evalFor(fs, env);
    if (auto ls = dynamic_cast<LetStatement*>(node)) {
        auto val = eval(ls->value.get(), env);
        if (isError(val) || isSignal(val)) return val;
        env->set(ls->name->value, val);
        return getNull();
    }
    if (auto as = dynamic_cast<AssignStatement*>(node)) return evalAssignStatement(as, env);
    if (auto rs = dynamic_cast<ReturnStatement*>(node)) {
        auto val = eval(rs->returnValue.get(), env);
        if (isError(val) || isSignal(val)) return val;
        auto rv = std::make_shared<ReturnValue>(); rv->value = val; return rv;
    }
    if (auto bs = dynamic_cast<BlockStatement*>(node)) return evalBlockStatement(bs, env);
    if (auto sbs = dynamic_cast<StandaloneBlockStatement*>(node)) return evalBlockStatementWithScoping(sbs->block.get(), env, false);
    if (dynamic_cast<PassStatement*>(node)) return getNull();
    if (auto ds = dynamic_cast<DelStatement*>(node)) return evalDelStatement(ds, env);
    if (auto as = dynamic_cast<AssertStatement*>(node)) return evalAssertStatement(as, env);
    if (dynamic_cast<NullLiteral*>(node)) return getNull();
    if (auto il = dynamic_cast<IntegerLiteral*>(node)) return newInteger(il->value);
    if (auto fl = dynamic_cast<FloatLiteral*>(node)) return newFloat(fl->value);
    if (auto bl = dynamic_cast<BooleanLiteral*>(node)) return nativeBoolToBooleanObject(bl->value);
    if (auto sl = dynamic_cast<StringLiteral*>(node)) return newString(sl->value);
    if (auto px = dynamic_cast<PrefixExpression*>(node)) {
        auto r = eval(px->right.get(), env);
        if (isError(r)) return r;
        return evalPrefixExpression(px->op, r);
    }
    if (auto ix = dynamic_cast<InfixExpression*>(node)) {
        if (ix->op == "&&" || ix->op == "and") {
            auto l = eval(ix->left.get(), env); if (isError(l)) return l;
            if (!isTruthy(l)) return getFalse();
            auto r = eval(ix->right.get(), env); if (isError(r)) return r;
            return nativeBoolToBooleanObject(isTruthy(r));
        }
        if (ix->op == "||" || ix->op == "or") {
            auto l = eval(ix->left.get(), env); if (isError(l)) return l;
            if (isTruthy(l)) return getTrue();
            auto r = eval(ix->right.get(), env); if (isError(r)) return r;
            return nativeBoolToBooleanObject(isTruthy(r));
        }
        auto l = eval(ix->left.get(), env); if (isError(l)) return l;
        auto r = eval(ix->right.get(), env); if (isError(r)) return r;
        return evalInfixExpression(ix->op, l, r);
    }
    if (auto ie = dynamic_cast<IfExpression*>(node)) return evalIfExpression(ie, env);
    if (auto id = dynamic_cast<Identifier*>(node)) return evalIdentifier(id, env);
    if (auto fd = dynamic_cast<FunctionDeclaration*>(node)) {
        auto fn = std::make_shared<Function>();
        fn->name = fd->name->value; fn->parameters = fd->parameters; fn->env = env; fn->body = fd->body;
        ObjectPtr decorated = fn;
        if (!fd->decorators.empty()) { decorated = applyDecorators(fd->decorators, decorated, env); if (isSignal(decorated) || isError(decorated)) return decorated; }
        env->set(fd->name->value, decorated);
        return getNull();
    }
    if (auto fl = dynamic_cast<FunctionLiteral*>(node)) {
        auto fn = std::make_shared<Function>();
        fn->parameters = fl->parameters; fn->env = env; fn->body = fl->body;
        return fn;
    }
    if (auto ce = dynamic_cast<CallExpression*>(node)) {
        auto function = eval(ce->function.get(), env);
        if (isError(function)) return function;
        auto args = evalExpressions(ce->arguments, env);
        if (args.size() == 1 && isError(args[0])) return args[0];
        return applyFunction(function, args);
    }
    if (auto al = dynamic_cast<ArrayLiteral*>(node)) {
        auto elems = evalExpressions(al->elements, env);
        if (elems.size() == 1 && isError(elems[0])) return elems[0];
        return newArray(elems);
    }
    if (auto ml = dynamic_cast<MapLiteral*>(node)) return evalMapLiteral(ml, env);
    if (auto idx = dynamic_cast<IndexExpression*>(node)) {
        auto l = eval(idx->left.get(), env); if (isError(l)) return l;
        auto i = eval(idx->index.get(), env); if (isError(i)) return i;
        return evalIndexExpression(l, i);
    }
    if (auto imp = dynamic_cast<ImportStatement*>(node)) return evalImportStatement(imp, env);
    if (auto ae = dynamic_cast<AssignExpression*>(node)) return evalAssignExpression(ae, env);
    if (auto ts = dynamic_cast<TryStatement*>(node)) return evalTryStatement(ts, env);
    if (auto ts = dynamic_cast<ThrowStatement*>(node)) return evalThrowStatement(ts, env);
    if (auto cd = dynamic_cast<ClassDeclaration*>(node)) return evalClassDeclaration(cd, env);
    if (auto me = dynamic_cast<MemberExpression*>(node)) return evalMemberExpression(me, env);
    if (auto ie = dynamic_cast<InExpression*>(node)) return evalInExpression(ie, env);
    if (auto ie = dynamic_cast<IsExpression*>(node)) return evalIsExpression(ie, env);
    if (auto ws = dynamic_cast<WithStatement*>(node)) return evalWithStatement(ws, env);
    if (auto gs = dynamic_cast<GlobalStatement*>(node)) return evalGlobalStatement(gs, env);
    if (auto ns = dynamic_cast<NonlocalStatement*>(node)) return evalNonlocalStatement(ns, env);
    if (auto lam = dynamic_cast<LambdaExpression*>(node)) {
        auto block = std::make_shared<BlockStatement>();
        block->token = lam->token;
        auto es = std::make_shared<ExpressionStatement>();
        es->token = lam->token; es->expression = lam->body;
        block->statements.push_back(es);
        auto fn = std::make_shared<Function>();
        fn->parameters = lam->parameters; fn->env = env; fn->body = block;
        return fn;
    }
    return builtinError("Runtime", "unknown node type");
}

// ============ Statements ============

ObjectPtr Interpreter::evalProgram(Program* program, std::shared_ptr<Environment> env) {
    ObjectPtr result = getNull();
    for (auto& stmt : program->statements) {
        result = eval(stmt.get(), env);
        if (auto rv = std::dynamic_pointer_cast<ReturnValue>(result)) return rv->value;
        if (isError(result) || isSignal(result)) return result;
    }
    return result;
}

ObjectPtr Interpreter::evalBlockStatement(BlockStatement* block, std::shared_ptr<Environment> env) {
    return evalBlockStatementWithScoping(block, env, true);
}

ObjectPtr Interpreter::evalBlockStatementWithScoping(BlockStatement* block, std::shared_ptr<Environment> env, bool createNewScope) {
    if (!block) return getNull();
    auto blockEnv = createNewScope ? newEnclosedEnvironment(env) : env;
    ObjectPtr result = getNull();
    for (auto& stmt : block->statements) {
        result = eval(stmt.get(), blockEnv);
        if (result && (result->type() == ObjectType::RETURN_VALUE || result->type() == ObjectType::ERROR ||
                       result->type() == ObjectType::BREAK_SIGNAL || result->type() == ObjectType::CONTINUE_SIGNAL ||
                       result->type() == ObjectType::EXCEPTION_SIGNAL)) return result;
    }
    return result;
}

ObjectPtr Interpreter::evalAssignStatement(AssignStatement* node, std::shared_ptr<Environment> env) {
    auto val = eval(node->value.get(), env);
    if (isError(val) || isSignal(val)) return val;
    if (auto t = std::dynamic_pointer_cast<Identifier>(node->target)) {
        if (!env->update(t->value, val)) env->set(t->value, val);
        return getNull();
    }
    if (auto t = std::dynamic_pointer_cast<IndexExpression>(node->target)) return evalIndexAssignment(t.get(), val, env);
    if (auto t = std::dynamic_pointer_cast<MemberExpression>(node->target)) return evalMemberAssignment(t.get(), val, env);
    return builtinError("Runtime", "invalid assignment target");
}

ObjectPtr Interpreter::evalIndexAssignment(IndexExpression* idx, ObjectPtr val, std::shared_ptr<Environment> env) {
    auto left = eval(idx->left.get(), env); if (isError(left)) return left;
    auto index = eval(idx->index.get(), env); if (isError(index)) return index;
    if (auto arr = std::dynamic_pointer_cast<Array>(left)) {
        auto idxObj = std::dynamic_pointer_cast<Integer>(index);
        if (!idxObj) return builtinError("TypeError", "array index must be integer");
        if (idxObj->value < 0 || idxObj->value >= (int64_t)arr->elements.size())
            return newExceptionSignal(std::dynamic_pointer_cast<Exception>(newException(INDEX_ERROR, "array index out of range")));
        arr->elements[idxObj->value] = val;
        return getNull();
    }
    if (auto m = std::dynamic_pointer_cast<Map>(left)) {
        for (auto it = m->pairs.begin(); it != m->pairs.end(); ++it)
            if (equals(it->first, index)) { m->pairs.erase(it); m->pairs.push_back({index, val}); return getNull(); }
        m->pairs.push_back({index, val}); return getNull();
    }
    return builtinError("TypeError", "index assignment not supported on " + std::string(ObjectTypeToString(left->type())));
}

ObjectPtr Interpreter::evalWhile(WhileStatement* node, std::shared_ptr<Environment> env) {
    while (true) {
        auto cond = eval(node->condition.get(), env);
        if (isError(cond) || isSignal(cond)) return cond;
        if (!isTruthy(cond)) break;
        auto result = evalBlockStatementWithScoping(node->body.get(), env, true);
        if (std::dynamic_pointer_cast<BreakSignal>(result)) break;
        if (std::dynamic_pointer_cast<ContinueSignal>(result)) continue;
        if (isError(result) || isSignal(result)) return result;
    }
    return getNull();
}

ObjectPtr Interpreter::evalFor(ForStatement* node, std::shared_ptr<Environment> env) {
    auto forEnv = newEnclosedEnvironment(env);
    if (node->init) eval(node->init.get(), forEnv);
    while (true) {
        if (node->condition) {
            auto cond = eval(node->condition.get(), forEnv);
            if (isError(cond) || isSignal(cond)) return cond;
            if (!isTruthy(cond)) break;
        }
        auto result = evalBlockStatementWithScoping(node->body.get(), forEnv, true);
        if (std::dynamic_pointer_cast<BreakSignal>(result)) break;
        if (!std::dynamic_pointer_cast<ContinueSignal>(result)) {
            if (isError(result) || isSignal(result)) return result;
        }
        if (node->post) eval(node->post.get(), forEnv);
    }
    return getNull();
}

ObjectPtr Interpreter::evalTryStatement(TryStatement* node, std::shared_ptr<Environment> env) {
    auto tryResult = evalBlockStatementWithScoping(node->tryBlock.get(), env, true);
    auto exSig = std::dynamic_pointer_cast<ExceptionSignal>(tryResult);
    if (!exSig) {
        if (node->finallyBlock) {
            auto fr = evalBlockStatementWithScoping(node->finallyBlock.get(), env, true);
            if (isError(fr) || isSignal(fr)) return fr;
        }
        return tryResult;
    }
    for (auto& cc : node->catchClauses) {
        if (!cc->exceptionType) {
            auto catchEnv = newEnclosedEnvironment(env);
            if (cc->variable) catchEnv->set(cc->variable->value, exSig->exception);
            auto cr = evalBlockStatementWithScoping(cc->catchBlock.get(), catchEnv, false);
            if (node->finallyBlock) { auto fr = evalBlockStatementWithScoping(node->finallyBlock.get(), env, true); if (isError(fr) || isSignal(fr)) return fr; }
            return cr;
        }
        if (exSig->exception && exSig->exception->exceptionType == cc->exceptionType->value) {
            auto catchEnv = newEnclosedEnvironment(env);
            if (cc->variable) catchEnv->set(cc->variable->value, exSig->exception);
            auto cr = evalBlockStatementWithScoping(cc->catchBlock.get(), catchEnv, false);
            if (node->finallyBlock) { auto fr = evalBlockStatementWithScoping(node->finallyBlock.get(), env, true); if (isError(fr) || isSignal(fr)) return fr; }
            return cr;
        }
    }
    if (node->finallyBlock) { auto fr = evalBlockStatementWithScoping(node->finallyBlock.get(), env, true); if (isError(fr) || isSignal(fr)) return fr; }
    return exSig;
}

ObjectPtr Interpreter::evalThrowStatement(ThrowStatement* node, std::shared_ptr<Environment> env) {
    auto exc = eval(node->exception.get(), env);
    if (isError(exc)) return exc;
    if (auto exObj = std::dynamic_pointer_cast<Exception>(exc)) return newExceptionSignal(exObj);
    if (std::dynamic_pointer_cast<ExceptionSignal>(exc)) return exc;
    auto ex = std::dynamic_pointer_cast<Exception>(newException(RUNTIME_ERROR, exc->inspect()));
    return newExceptionSignal(ex);
}

ObjectPtr Interpreter::evalClassDeclaration(ClassDeclaration* node, std::shared_ptr<Environment> env) {
    auto cls = std::make_shared<Class>();
    cls->name = node->name->value;
    auto classEnv = newEnclosedEnvironment(env);
    evalBlockStatementWithScoping(node->body.get(), classEnv, false);
    for (auto& [k, v] : classEnv->getAll()) cls->members[k] = v;
    ObjectPtr result = cls;
    if (!node->decorators.empty()) result = applyDecorators(node->decorators, cls, env);
    env->set(node->name->value, result);
    return getNull();
}

ObjectPtr Interpreter::evalImportStatement(ImportStatement* node, std::shared_ptr<Environment> env) {
    if (!node->path) return builtinError("ImportError", "import requires a path");
    std::string path = node->path->value;
    if (auto it = loadedModules_.find(path); it != loadedModules_.end()) return it->second;

    // Native modules: import math  OR  import "go:math"
    std::string modName = path;
    if (path.substr(0, 3) == "go:") {
        modName = path.substr(3);
    }

    auto modEnv = newEnclosedEnvironment(env);
    const auto* nativeMod = native::Registry::instance().get(modName);
    if (nativeMod) {
        for (auto& [fnName, fn] : nativeMod->functions) {
            auto builtin = std::make_shared<Builtin>();
            builtin->fn = fn;
            modEnv->set(fnName, builtin);
        }
    }

    auto mod = std::make_shared<Module>();
    mod->path = path;
    mod->env = modEnv;
    env->set(modName, mod);
    loadedModules_[path] = mod;
    return mod;
}

ObjectPtr Interpreter::evalDelStatement(DelStatement* node, std::shared_ptr<Environment> env) {
    if (auto t = std::dynamic_pointer_cast<Identifier>(node->target)) {
        if (!env->erase(t->value)) return builtinError("NameError", "name '" + t->value + "' is not defined");
        return getNull();
    }
    if (auto t = std::dynamic_pointer_cast<IndexExpression>(node->target)) {
        auto left = eval(t->left.get(), env); if (isError(left)) return left;
        auto index = eval(t->index.get(), env); if (isError(index)) return index;
        if (auto arr = std::dynamic_pointer_cast<Array>(left)) {
            auto idx = std::dynamic_pointer_cast<Integer>(index);
            if (!idx) return builtinError("TypeError", "array index must be integer");
            if (idx->value < 0 || idx->value >= (int64_t)arr->elements.size())
                return newExceptionSignal(std::dynamic_pointer_cast<Exception>(newException(INDEX_ERROR, "array index out of range")));
            arr->elements.erase(arr->elements.begin() + idx->value); return getNull();
        }
        if (auto m = std::dynamic_pointer_cast<Map>(left)) {
            for (auto it = m->pairs.begin(); it != m->pairs.end(); ++it)
                if (equals(it->first, index)) { m->pairs.erase(it); return getNull(); }
            return getNull();
        }
        return builtinError("TypeError", "index delete not supported on " + std::string(ObjectTypeToString(left->type())));
    }
    return builtinError("Runtime", "invalid del target");
}

ObjectPtr Interpreter::evalAssertStatement(AssertStatement* node, std::shared_ptr<Environment> env) {
    auto cond = eval(node->condition.get(), env);
    if (isError(cond) || isSignal(cond)) return cond;
    if (!isTruthy(cond)) {
        std::string msg = "assertion failed";
        if (node->message) { auto m = eval(node->message.get(), env); if (isError(m) || isSignal(m)) return m; msg = m->inspect(); }
        return newExceptionSignal(std::dynamic_pointer_cast<Exception>(newException(ASSERTION_ERROR, msg)));
    }
    return getNull();
}

ObjectPtr Interpreter::evalWithStatement(WithStatement* node, std::shared_ptr<Environment> env) {
    auto ctx = eval(node->context.get(), env);
    if (isError(ctx) || isSignal(ctx)) return ctx;
    auto withEnv = newEnclosedEnvironment(env);
    if (node->variable) withEnv->set(node->variable->value, ctx);
    auto bodyResult = evalBlockStatementWithScoping(node->body.get(), withEnv, false);
    if (auto inst = std::dynamic_pointer_cast<Instance>(ctx)) {
        if (auto it = inst->fields.find("__exit__"); it != inst->fields.end()) {
            if (auto exitFn = std::dynamic_pointer_cast<Function>(it->second)) {
                auto exc = getNull();
                if (isSignal(bodyResult) || isError(bodyResult)) exc = bodyResult;
                applyFunction(exitFn, {exc});
            }
        }
    }
    if (isError(bodyResult) || isSignal(bodyResult)) return bodyResult;
    return getNull();
}

ObjectPtr Interpreter::evalGlobalStatement(GlobalStatement*, std::shared_ptr<Environment>) { return getNull(); }
ObjectPtr Interpreter::evalNonlocalStatement(NonlocalStatement*, std::shared_ptr<Environment>) { return getNull(); }

ObjectPtr Interpreter::evalMapLiteral(MapLiteral* node, std::shared_ptr<Environment> env) {
    std::vector<std::pair<ObjectPtr, ObjectPtr>> pairs;
    for (auto& [k, v] : node->pairs) {
        auto key = eval(k.get(), env); if (isError(key)) return key;
        auto val = eval(v.get(), env); if (isError(val)) return val;
        pairs.push_back({key, val});
    }
    return newMap(pairs);
}

// ============ Expressions ============

ObjectPtr Interpreter::evalInfixExpression(const std::string& op, ObjectPtr left, ObjectPtr right) {
    // Null comparisons
    bool leftNull = (!left) || (left->type() == ObjectType::NULL_OBJ);
    bool rightNull = (!right) || (right->type() == ObjectType::NULL_OBJ);
    if (leftNull && rightNull) {
        if (op == "==") return getTrue();
        if (op == "!=") return getFalse();
    }
    if (leftNull != rightNull) {
        if (op == "==") return getFalse();
        if (op == "!=") return getTrue();
    }
    if (left->type() == ObjectType::INTEGER && right->type() == ObjectType::INTEGER) {
        auto l = std::dynamic_pointer_cast<Integer>(left); auto r = std::dynamic_pointer_cast<Integer>(right);
        if (op == "+") return newInteger(l->value + r->value);
        if (op == "-") return newInteger(l->value - r->value);
        if (op == "*") return newInteger(l->value * r->value);
        if (op == "/") { if (r->value == 0) return newExceptionSignal(std::dynamic_pointer_cast<Exception>(newException(ZERO_DIV_ERROR, "division by zero"))); return newInteger(l->value / r->value); }
        if (op == "%") { if (r->value == 0) return newExceptionSignal(std::dynamic_pointer_cast<Exception>(newException(ZERO_DIV_ERROR, "modulo by zero"))); return newInteger(l->value % r->value); }
        if (op == "<") return nativeBoolToBooleanObject(l->value < r->value);
        if (op == ">") return nativeBoolToBooleanObject(l->value > r->value);
        if (op == "<=") return nativeBoolToBooleanObject(l->value <= r->value);
        if (op == ">=") return nativeBoolToBooleanObject(l->value >= r->value);
        if (op == "==") return nativeBoolToBooleanObject(l->value == r->value);
        if (op == "!=") return nativeBoolToBooleanObject(l->value != r->value);
    }
    if (left->type() == ObjectType::FLOAT || right->type() == ObjectType::FLOAT) {
        double l = (left->type() == ObjectType::FLOAT) ? std::dynamic_pointer_cast<Float>(left)->value : std::dynamic_pointer_cast<Integer>(left)->value;
        double r = (right->type() == ObjectType::FLOAT) ? std::dynamic_pointer_cast<Float>(right)->value : std::dynamic_pointer_cast<Integer>(right)->value;
        if (op == "+") return newFloat(l + r); if (op == "-") return newFloat(l - r);
        if (op == "*") return newFloat(l * r);
        if (op == "/") { if (r == 0) return newExceptionSignal(std::dynamic_pointer_cast<Exception>(newException(ZERO_DIV_ERROR, "division by zero"))); return newFloat(l / r); }
        if (op == "<") return nativeBoolToBooleanObject(l < r); if (op == ">") return nativeBoolToBooleanObject(l > r);
        if (op == "<=") return nativeBoolToBooleanObject(l <= r); if (op == ">=") return nativeBoolToBooleanObject(l >= r);
        if (op == "==") return nativeBoolToBooleanObject(l == r); if (op == "!=") return nativeBoolToBooleanObject(l != r);
    }
    if (left->type() == ObjectType::STRING && right->type() == ObjectType::STRING) {
        auto l = std::dynamic_pointer_cast<String>(left); auto r = std::dynamic_pointer_cast<String>(right);
        if (op == "+") return newString(l->value + r->value);
        if (op == "==") return nativeBoolToBooleanObject(l->value == r->value);
        if (op == "!=") return nativeBoolToBooleanObject(l->value != r->value);
        if (op == "<") return nativeBoolToBooleanObject(l->value < r->value);
        if (op == ">") return nativeBoolToBooleanObject(l->value > r->value);
        if (op == "<=") return nativeBoolToBooleanObject(l->value <= r->value);
        if (op == ">=") return nativeBoolToBooleanObject(l->value >= r->value);
    }
    if (left->type() == ObjectType::BOOLEAN && right->type() == ObjectType::BOOLEAN) {
        auto l = std::dynamic_pointer_cast<Boolean>(left); auto r = std::dynamic_pointer_cast<Boolean>(right);
        if (op == "==") return nativeBoolToBooleanObject(l->value == r->value);
        if (op == "!=") return nativeBoolToBooleanObject(l->value != r->value);
    }
    // Map equality
    if (left->type() == ObjectType::MAP && right->type() == ObjectType::MAP) {
        if (op == "==") return nativeBoolToBooleanObject(equals(left, right));
        if (op == "!=") return nativeBoolToBooleanObject(!equals(left, right));
    }
    // Array equality
    if (left->type() == ObjectType::ARRAY && right->type() == ObjectType::ARRAY) {
        if (op == "==") return nativeBoolToBooleanObject(equals(left, right));
        if (op == "!=") return nativeBoolToBooleanObject(!equals(left, right));
    }
    return builtinError("TypeError", "unsupported operator " + op + " for " + ObjectTypeToString(left->type()) + " and " + ObjectTypeToString(right->type()));
}

ObjectPtr Interpreter::evalPrefixExpression(const std::string& op, ObjectPtr right) {
    if (op == "!") return nativeBoolToBooleanObject(!isTruthy(right));
    if (op == "-") {
        if (auto i = std::dynamic_pointer_cast<Integer>(right)) return newInteger(-i->value);
        if (auto f = std::dynamic_pointer_cast<Float>(right)) return newFloat(-f->value);
    }
    return builtinError("TypeError", "unknown prefix operator " + op);
}

ObjectPtr Interpreter::evalIfExpression(IfExpression* node, std::shared_ptr<Environment> env) {
    auto cond = eval(node->condition.get(), env);
    if (isError(cond) || isSignal(cond)) return cond;
    if (isTruthy(cond)) return evalBlockStatementWithScoping(node->consequence.get(), env, true);
    if (node->alternative) {
        if (auto altIf = std::dynamic_pointer_cast<IfExpression>(node->alternative)) return evalIfExpression(altIf.get(), env);
        return evalBlockStatementWithScoping(std::dynamic_pointer_cast<BlockStatement>(node->alternative).get(), env, true);
    }
    return getNull();
}

ObjectPtr Interpreter::evalIdentifier(Identifier* node, std::shared_ptr<Environment> env) {
    auto val = env->get(node->value);
    if (val) return val;
    auto it = builtins_.find(node->value);
    if (it != builtins_.end()) return it->second;
    auto ex = std::dynamic_pointer_cast<Exception>(newException(NAME_ERROR, "name '" + node->value + "' is not defined"));
    return newExceptionSignal(ex);
}

std::vector<ObjectPtr> Interpreter::evalExpressions(const std::vector<ExpressionPtr>& exps, std::shared_ptr<Environment> env) {
    std::vector<ObjectPtr> result;
    for (auto& e : exps) {
        auto val = eval(e.get(), env);
        if (isError(val) || isSignal(val)) return {val};
        result.push_back(val);
    }
    return result;
}

ObjectPtr Interpreter::evalIndexExpression(ObjectPtr left, ObjectPtr index) {
    if (left->type() == ObjectType::ARRAY && index->type() == ObjectType::INTEGER) {
        auto arr = std::dynamic_pointer_cast<Array>(left); auto idx = std::dynamic_pointer_cast<Integer>(index)->value;
        if (idx < 0 || idx >= (int64_t)arr->elements.size()) return getNull();
        return arr->elements[idx];
    }
    if (left->type() == ObjectType::MAP) {
        auto m = std::dynamic_pointer_cast<Map>(left);
        for (auto& [k, v] : m->pairs) if (equals(k, index)) return v;
        return getNull();
    }
    if (left->type() == ObjectType::STRING && index->type() == ObjectType::INTEGER) {
        auto s = std::dynamic_pointer_cast<String>(left); auto idx = std::dynamic_pointer_cast<Integer>(index)->value;
        if (idx < 0 || idx >= (int64_t)s->value.size()) return getNull();
        return newString(std::string(1, s->value[idx]));
    }
    return builtinError("TypeError", "index operator not supported on " + std::string(ObjectTypeToString(left->type())));
}

ObjectPtr Interpreter::evalAssignExpression(AssignExpression* node, std::shared_ptr<Environment> env) {
    auto val = eval(node->value.get(), env);
    if (isError(val) || isSignal(val)) return val;
    if (auto nameIdent = std::dynamic_pointer_cast<Identifier>(node->name)) {
        if (!env->update(nameIdent->value, val)) env->set(nameIdent->value, val);
        return val;
    }
    if (auto nameIdx = std::dynamic_pointer_cast<IndexExpression>(node->name)) {
        evalIndexAssignment(nameIdx.get(), val, env); return val;
    }
    return builtinError("Runtime", "invalid assignment target");
}

ObjectPtr Interpreter::evalMemberExpression(MemberExpression* node, std::shared_ptr<Environment> env) {
    auto left = eval(node->left.get(), env);
    if (isError(left) || isSignal(left)) return left;
    std::string prop = node->property->value;
    if (auto inst = std::dynamic_pointer_cast<Instance>(left)) {
        if (auto it = inst->fields.find(prop); it != inst->fields.end()) return it->second;
        if (auto it = inst->cls->members.find(prop); it != inst->cls->members.end()) {
            if (auto fn = std::dynamic_pointer_cast<Function>(it->second)) return newBoundMethod(inst, fn);
            return it->second;
        }
        return builtinError("AttributeError", "attribute '" + prop + "' not found on instance of '" + inst->cls->name + "'");
    }
    if (auto cls = std::dynamic_pointer_cast<Class>(left)) {
        if (auto it = cls->members.find(prop); it != cls->members.end()) return it->second;
        return builtinError("AttributeError", "attribute '" + prop + "' not found on class '" + cls->name + "'");
    }
    if (auto mod = std::dynamic_pointer_cast<Module>(left)) {
        if (auto val = mod->env->get(prop)) return val;
        return builtinError("AttributeError", "attribute '" + prop + "' not found on module");
    }
    return builtinError("AttributeError", "attribute access not supported on " + std::string(ObjectTypeToString(left->type())));
}

ObjectPtr Interpreter::evalMemberAssignment(MemberExpression* memberExpr, ObjectPtr val, std::shared_ptr<Environment> env) {
    auto left = eval(memberExpr->left.get(), env);
    if (isError(left)) return left;
    std::string prop = memberExpr->property->value;
    if (auto inst = std::dynamic_pointer_cast<Instance>(left)) { inst->fields[prop] = val; return val; }
    if (auto cls = std::dynamic_pointer_cast<Class>(left)) { cls->members[prop] = val; return val; }
    return builtinError("TypeError", "member assignment not supported on " + std::string(ObjectTypeToString(left->type())));
}

ObjectPtr Interpreter::evalInExpression(InExpression* node, std::shared_ptr<Environment> env) {
    auto left = eval(node->left.get(), env); if (isError(left) || isSignal(left)) return left;
    auto right = eval(node->right.get(), env); if (isError(right) || isSignal(right)) return right;
    if (auto arr = std::dynamic_pointer_cast<Array>(right)) {
        for (auto& elem : arr->elements) if (equals(elem, left)) return getTrue();
        return getFalse();
    }
    if (auto s = std::dynamic_pointer_cast<String>(right))
        if (auto ls = std::dynamic_pointer_cast<String>(left))
            return nativeBoolToBooleanObject(s->value.find(ls->value) != std::string::npos);
    if (auto m = std::dynamic_pointer_cast<Map>(right)) {
        for (auto& [k, v] : m->pairs) if (equals(k, left)) return getTrue();
        return getFalse();
    }
    return builtinError("TypeError", "'in' operator not supported for " + std::string(ObjectTypeToString(right->type())));
}

ObjectPtr Interpreter::evalIsExpression(IsExpression* node, std::shared_ptr<Environment> env) {
    auto left = eval(node->left.get(), env); if (isError(left) || isSignal(left)) return left;
    auto right = eval(node->right.get(), env); if (isError(right) || isSignal(right)) return right;
    return nativeBoolToBooleanObject(left.get() == right.get());
}

// ============ Function application ============

ObjectPtr Interpreter::applyFunction(ObjectPtr fn, const std::vector<ObjectPtr>& args) {
    if (auto builtin = std::dynamic_pointer_cast<Builtin>(fn)) return builtin->fn(args);
    if (auto func = std::dynamic_pointer_cast<Function>(fn)) {
        auto funcEnv = newEnclosedEnvironment(func->env);
        for (size_t i = 0; i < func->parameters.size(); i++)
            funcEnv->set(func->parameters[i]->value, (i < args.size()) ? args[i] : getNull());
        auto result = evalBlockStatementWithScoping(func->body.get(), funcEnv, false);
        if (auto rv = std::dynamic_pointer_cast<ReturnValue>(result)) return rv->value;
        return result;
    }
    if (auto bm = std::dynamic_pointer_cast<BoundMethod>(fn)) {
        auto funcEnv = newEnclosedEnvironment(bm->fn->env);
        funcEnv->set("self", bm->self);
        for (size_t i = 0; i < bm->fn->parameters.size(); i++) {
            std::string name = bm->fn->parameters[i]->value;
            if (name == "self") continue;
            funcEnv->set(name, (i < args.size()) ? args[i] : getNull());
        }
        auto result = evalBlockStatementWithScoping(bm->fn->body.get(), funcEnv, false);
        if (auto rv = std::dynamic_pointer_cast<ReturnValue>(result)) return rv->value;
        return result;
    }
    if (auto cls = std::dynamic_pointer_cast<Class>(fn)) {
        auto inst = newInstance(cls);
        auto initIt = cls->members.find("__init__");
        if (initIt != cls->members.end()) {
            if (auto initFn = std::dynamic_pointer_cast<Function>(initIt->second)) {
                auto funcEnv = newEnclosedEnvironment(initFn->env);
                funcEnv->set("self", inst);
                for (size_t i = 0; i < initFn->parameters.size(); i++) {
                    std::string name = initFn->parameters[i]->value;
                    if (name == "self") continue;
                    funcEnv->set(name, (i < args.size()) ? args[i] : getNull());
                }
                evalBlockStatementWithScoping(initFn->body.get(), funcEnv, false);
            }
        }
        return inst;
    }
    return builtinError("TypeError", "not a function: " + std::string(ObjectTypeToString(fn->type())));
}

ObjectPtr Interpreter::applyDecorators(const std::vector<ExpressionPtr>& decorators, ObjectPtr fn, std::shared_ptr<Environment> env) {
    ObjectPtr result = fn;
    for (int i = (int)decorators.size() - 1; i >= 0; i--) {
        auto decorator = eval(decorators[i].get(), env);
        if (isError(decorator) || isSignal(decorator)) return decorator;
        auto decorated = applyFunction(decorator, {result});
        if (isError(decorated) || isSignal(decorated)) return decorated;
        result = decorated;
    }
    return result;
}

// ============ Builtins ============

void Interpreter::initBuiltins() {
    auto makeBuiltin = [](auto fn) { auto b = std::make_shared<Builtin>(); b->fn = fn; return b; };
    builtins_["print"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.empty()) { std::printf("\n"); return getNull(); }
        std::string out;
        for (size_t i = 0; i < args.size(); i++) { if (i > 0) out += " "; out += args[i]->inspect(); }
        std::printf("%s\n", out.c_str()); return getNull();
    });
    builtins_["len"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("len: expected 1 argument");
        if (auto s = std::dynamic_pointer_cast<String>(args[0])) return newInteger((int64_t)s->value.size());
        if (auto a = std::dynamic_pointer_cast<Array>(args[0])) return newInteger((int64_t)a->elements.size());
        if (auto m = std::dynamic_pointer_cast<Map>(args[0])) return newInteger((int64_t)m->pairs.size());
        return newError("len: unsupported type");
    });
    builtins_["str"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("str: expected 1 argument");
        if (auto s = std::dynamic_pointer_cast<String>(args[0])) return s;
        return newString(args[0]->inspect());
    });
    builtins_["int"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("int: expected 1 argument");
        if (auto i = std::dynamic_pointer_cast<Integer>(args[0])) return i;
        if (auto f = std::dynamic_pointer_cast<Float>(args[0])) return newInteger((int64_t)f->value);
        if (auto s = std::dynamic_pointer_cast<String>(args[0])) {
            try { return newInteger(std::stoll(s->value)); } catch (...) { return newError("int: cannot convert"); }
        }
        return newError("int: unsupported type");
    });
    builtins_["float"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("float: expected 1 argument");
        if (auto f = std::dynamic_pointer_cast<Float>(args[0])) return f;
        if (auto i = std::dynamic_pointer_cast<Integer>(args[0])) return newFloat((double)i->value);
        if (auto s = std::dynamic_pointer_cast<String>(args[0])) {
            try { return newFloat(std::stod(s->value)); } catch (...) { return newError("float: cannot convert"); }
        }
        return newError("float: unsupported type");
    });
    builtins_["bool"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("bool: expected 1 argument");
        return nativeBoolToBooleanObject(isTruthy(args[0]));
    });
    builtins_["type"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("type: expected 1 argument");
        return newString(ObjectTypeToString(args[0]->type()));
    });
    builtins_["range"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.empty() || args.size() > 3) return newError("range: expected 1-3 arguments");
        int64_t start = 0, stop = 0, step = 1;
        if (args.size() == 1) stop = asInt(args[0]);
        else if (args.size() == 2) { start = asInt(args[0]); stop = asInt(args[1]); }
        else { start = asInt(args[0]); stop = asInt(args[1]); step = asInt(args[2]); }
        if (step == 0) return newError("range: step cannot be 0");
        std::vector<ObjectPtr> elems;
        if (step > 0) { for (int64_t i = start; i < stop; i += step) elems.push_back(newInteger(i)); }
        else { for (int64_t i = start; i > stop; i += step) elems.push_back(newInteger(i)); }
        return newArray(elems);
    });
    builtins_["abs"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("abs: expected 1 argument");
        if (auto i = std::dynamic_pointer_cast<Integer>(args[0])) return newInteger(i->value < 0 ? -i->value : i->value);
        if (auto f = std::dynamic_pointer_cast<Float>(args[0])) return newFloat(f->value < 0 ? -f->value : f->value);
        return newError("abs: unsupported type");
    });
    builtins_["max"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.empty()) return newError("max: expected at least 1 argument");
        ObjectPtr max = args[0];
        for (size_t i = 1; i < args.size(); i++) if (compareObjects(args[i], max) > 0) max = args[i];
        return max;
    });
    builtins_["min"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.empty()) return newError("min: expected at least 1 argument");
        ObjectPtr min = args[0];
        for (size_t i = 1; i < args.size(); i++) if (compareObjects(args[i], min) < 0) min = args[i];
        return min;
    });
    builtins_["sum"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("sum: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return newError("sum: argument must be an array");
        int64_t intSum = 0; double floatSum = 0; bool hasFloat = false;
        for (auto& elem : arr->elements) {
            if (auto i = std::dynamic_pointer_cast<Integer>(elem)) { if (hasFloat) floatSum += i->value; else intSum += i->value; }
            else if (auto f = std::dynamic_pointer_cast<Float>(elem)) { if (!hasFloat) { floatSum = intSum + f->value; hasFloat = true; } else floatSum += f->value; }
            else return newError("sum: all elements must be numbers");
        }
        return hasFloat ? newFloat(floatSum) : newInteger(intSum);
    });
    builtins_["sorted"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("sorted: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return newError("sorted: argument must be an array");
        auto sorted = arr->elements;
        std::sort(sorted.begin(), sorted.end(), [](const ObjectPtr& a, const ObjectPtr& b) { return compareObjects(a, b) < 0; });
        return newArray(sorted);
    });
    builtins_["reverse"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("reverse: expected 1 argument");
        if (auto s = std::dynamic_pointer_cast<String>(args[0])) { std::string rev = s->value; std::reverse(rev.begin(), rev.end()); return newString(rev); }
        if (auto arr = std::dynamic_pointer_cast<Array>(args[0])) { auto r = arr->elements; std::reverse(r.begin(), r.end()); return newArray(r); }
        return newError("reverse: unsupported type");
    });
    builtins_["append"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return newError("append: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return newError("append: first argument must be an array");
        arr->elements.push_back(args[1]); return getNull();
    });
    builtins_["contains"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return newError("contains: expected 2 arguments");
        if (auto s = std::dynamic_pointer_cast<String>(args[0]))
            if (auto sub = std::dynamic_pointer_cast<String>(args[1]))
                return nativeBoolToBooleanObject(s->value.find(sub->value) != std::string::npos);
        if (auto arr = std::dynamic_pointer_cast<Array>(args[0])) {
            for (auto& elem : arr->elements) if (equals(elem, args[1])) return getTrue();
            return getFalse();
        }
        return newError("contains: unsupported type");
    });
    builtins_["exit"] = makeBuiltin([](const std::vector<ObjectPtr>&) -> ObjectPtr { std::exit(0); return getNull(); });
    builtins_["ValueError"] = makeBuiltin([](const std::vector<ObjectPtr>& a) -> ObjectPtr {
        return newException(VALUE_ERROR, a.size() > 0 ? a[0]->inspect() : "");
    });
    builtins_["TypeError"] = makeBuiltin([](const std::vector<ObjectPtr>& a) -> ObjectPtr {
        return newException(TYPE_ERROR, a.size() > 0 ? a[0]->inspect() : "");
    });
    builtins_["RuntimeError"] = makeBuiltin([](const std::vector<ObjectPtr>& a) -> ObjectPtr {
        return newException(RUNTIME_ERROR, a.size() > 0 ? a[0]->inspect() : "");
    });
    builtins_["IndexError"] = makeBuiltin([](const std::vector<ObjectPtr>& a) -> ObjectPtr {
        return newException(INDEX_ERROR, a.size() > 0 ? a[0]->inspect() : "");
    });
    builtins_["KeyError"] = makeBuiltin([](const std::vector<ObjectPtr>& a) -> ObjectPtr {
        return newException(KEY_ERROR, a.size() > 0 ? a[0]->inspect() : "");
    });
    builtins_["ZeroDivisionError"] = makeBuiltin([](const std::vector<ObjectPtr>& a) -> ObjectPtr {
        return newException(ZERO_DIV_ERROR, a.size() > 0 ? a[0]->inspect() : "");
    });
    builtins_["keys"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("keys: expected 1 argument");
        if (auto m = std::dynamic_pointer_cast<Map>(args[0])) {
            std::vector<ObjectPtr> keys;
            for (auto& [k, v] : m->pairs) keys.push_back(k);
            return newArray(keys);
        }
        if (auto s = std::dynamic_pointer_cast<String>(args[0])) {
            std::vector<ObjectPtr> keys;
            for (size_t i = 0; i < s->value.size(); i++) keys.push_back(newInteger((int64_t)i));
            return newArray(keys);
        }
        return newError("keys: unsupported type");
    });
    builtins_["values"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("values: expected 1 argument");
        if (auto m = std::dynamic_pointer_cast<Map>(args[0])) {
            std::vector<ObjectPtr> vals;
            for (auto& [k, v] : m->pairs) vals.push_back(v);
            return newArray(vals);
        }
        return newError("values: unsupported type");
    });
    builtins_["items"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("items: expected 1 argument");
        if (auto m = std::dynamic_pointer_cast<Map>(args[0])) {
            std::vector<ObjectPtr> pairs;
            for (auto& [k, v] : m->pairs) {
                auto pair = newArray({k, v});
                pairs.push_back(pair);
            }
            return newArray(pairs);
        }
        return newError("items: unsupported type");
    });
    builtins_["sort"] = makeBuiltin([](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return newError("sort: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return newError("sort: argument must be an array");
        auto sorted = arr->elements;
        std::sort(sorted.begin(), sorted.end(), [](const ObjectPtr& a, const ObjectPtr& b) { return compareObjects(a, b) < 0; });
        return newArray(sorted);
    });
}

} // namespace darix
