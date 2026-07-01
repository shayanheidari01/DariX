#include "darix/object.hpp"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <functional>
#include <sstream>

namespace darix {

// ============ ObjectType string ============

const char* ObjectTypeToString(ObjectType type) {
    switch (type) {
        case ObjectType::INTEGER:          return "INTEGER";
        case ObjectType::FLOAT:            return "FLOAT";
        case ObjectType::BOOLEAN:          return "BOOLEAN";
        case ObjectType::NULL_OBJ:         return "NULL";
        case ObjectType::RETURN_VALUE:     return "RETURN_VALUE";
        case ObjectType::ERROR:            return "ERROR";
        case ObjectType::FUNCTION:         return "FUNCTION";
        case ObjectType::STRING:           return "STRING";
        case ObjectType::ARRAY:            return "ARRAY";
        case ObjectType::MAP:              return "MAP";
        case ObjectType::BUILTIN:          return "BUILTIN";
        case ObjectType::HASH:             return "HASH";
        case ObjectType::EXCEPTION:        return "EXCEPTION";
        case ObjectType::STACK_TRACE:      return "STACK_TRACE";
        case ObjectType::CLASS:            return "CLASS";
        case ObjectType::INSTANCE:         return "INSTANCE";
        case ObjectType::BOUND_METHOD:     return "BOUND_METHOD";
        case ObjectType::COMPILED_FUNCTION:return "COMPILED_FUNCTION";
        case ObjectType::MODULE:           return "MODULE";
        case ObjectType::BREAK_SIGNAL:     return "BREAK_SIGNAL";
        case ObjectType::CONTINUE_SIGNAL:  return "CONTINUE_SIGNAL";
        case ObjectType::EXCEPTION_SIGNAL: return "EXCEPTION_SIGNAL";
    }
    return "UNKNOWN";
}

// ============ Position ============

std::string Position::str() const {
    if (!filename.empty() && line > 0 && column > 0) {
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
    if (line > 0 && column > 0) {
        return "line " + std::to_string(line) + ":" + std::to_string(column);
    }
    return "";
}

// ============ StackFrame ============

std::string StackFrame::str() const {
    std::string result = "  at " + functionName + " (" + position.str() + ")";
    if (!context.empty()) result += "\n    " + context;
    return result;
}

// ============ Helper functions ============

static std::string formatSequence(const std::string& prefix, const std::string& suffix, const std::vector<ObjectPtr>& elements) {
    if (elements.empty()) return prefix + suffix;
    std::string out = prefix;
    for (size_t i = 0; i < elements.size(); i++) {
        if (i > 0) out += ", ";
        out += elements[i]->inspect();
    }
    out += suffix;
    return out;
}

static std::string formatEntries(const std::string& prefix, const std::string& suffix,
                                  const std::vector<std::pair<std::string, std::string>>& entries) {
    if (entries.empty()) return prefix + suffix;
    auto sorted = entries;
    std::sort(sorted.begin(), sorted.end());
    std::string out = prefix;
    for (size_t i = 0; i < sorted.size(); i++) {
        if (i > 0) out += ", ";
        out += sorted[i].second;
    }
    out += suffix;
    return out;
}

// ============ Concrete type inspect methods ============

std::string Integer::inspect() const { return std::to_string(value); }
std::string Float::inspect() const {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%g", value);
    return buf;
}
std::string Boolean::inspect() const { return value ? "true" : "false"; }
std::string String::inspect() const { return value; }
std::string Array::inspect() const { return formatSequence("[", "]", elements); }

std::string ReturnValue::inspect() const { return value ? value->inspect() : ""; }

std::string StackTrace::inspect() const {
    std::string out = "Stack trace:\n";
    for (const auto& frame : frames) {
        out += frame.str() + "\n";
    }
    return out;
}

std::string Exception::inspect() const {
    std::string out = exceptionType + ": " + message;
    if (stackTrace) out += "\n" + stackTrace->inspect();
    if (cause) out += "\nCaused by: " + cause->inspect();
    return out;
}

std::string ExceptionSignal::inspect() const {
    return exception ? exception->inspect() : "Unhandled exception";
}

std::string Error::inspect() const {
    std::string out;
    out += errorType.empty() ? "Runtime error" : errorType;
    if (position.line > 0) out += " at " + position.str();
    out += ": " + message;
    if (!suggestion.empty()) out += "\n\nSuggestion: " + suggestion;
    if (!stackTrace.empty()) {
        out += "\n\nStack trace:";
        for (const auto& frame : stackTrace) out += "\n" + frame.str();
    }
    return out;
}

void Error::addStackFrame(const std::string& fn, const Position& pos, const std::string& ctx) {
    stackTrace.push_back({fn, pos, ctx});
}

std::string Function::inspect() const {
    std::string out = "func";
    if (!name.empty()) out += " " + name;
    out += "(";
    for (size_t i = 0; i < parameters.size(); i++) {
        if (i > 0) out += ", ";
        out += parameters[i]->inspect();
    }
    out += ") ";
    if (body) out += body->inspect();
    return out;
}

std::string CompiledFunction::inspect() const {
    if (!name.empty()) return "<compiled func " + name + " params=" + std::to_string(numParameters) + " locals=" + std::to_string(numLocals) + ">";
    return "<compiled func params=" + std::to_string(numParameters) + " locals=" + std::to_string(numLocals) + ">";
}

std::string Map::inspect() const {
    std::vector<std::pair<std::string, std::string>> entries;
    for (const auto& [k, v] : pairs) {
        std::string keyStr = k->inspect();
        entries.push_back({keyStr, keyStr + ": " + v->inspect()});
    }
    return formatEntries("{", "}", entries);
}

std::string Hash::inspect() const {
    std::vector<std::pair<std::string, std::string>> entries;
    for (const auto& [hk, pair] : pairs) {
        std::string keyStr = pair.key->inspect();
        entries.push_back({keyStr, keyStr + ":" + pair.value->inspect()});
    }
    return formatEntries("{", "}", entries);
}

std::string Class::inspect() const { return "<class " + name + ">"; }
std::string Instance::inspect() const { return "<" + cls->name + " instance>"; }
std::string BoundMethod::inspect() const { return "<bound method " + fn->name + " of " + self->cls->name + ">"; }
std::string Module::inspect() const { return "<module " + path + ">"; }

// ============ HashKey ============

static uint64_t fnv64a(const std::string& data) {
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char c : data) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return hash;
}

uint64_t Integer::hashKey() const { return static_cast<uint64_t>(value); }
uint64_t String::hashKey() const { return fnv64a(value); }

// ============ Environment ============

ObjectPtr Environment::get(const std::string& name) const {
    // Linear scan - faster than hash map for small environments (1-5 vars)
    for (size_t i = store.size(); i > 0; ) {
        --i;
        if (store[i].first == name) return store[i].second;
    }
    if (outer) return outer->get(name);
    return nullptr;
}

ObjectPtr Environment::set(const std::string& name, ObjectPtr val) {
    // Check if already exists (update in place)
    for (size_t i = 0; i < store.size(); i++) {
        if (store[i].first == name) { store[i].second = val; return val; }
    }
    store.emplace_back(name, val);
    return val;
}

bool Environment::update(const std::string& name, ObjectPtr val) {
    for (size_t i = 0; i < store.size(); i++) {
        if (store[i].first == name) { store[i].second = val; return true; }
    }
    if (outer) return outer->update(name, val);
    return false;
}

bool Environment::erase(const std::string& name) {
    for (auto it = store.begin(); it != store.end(); ++it) {
        if (it->first == name) { store.erase(it); return true; }
    }
    if (outer) return outer->erase(name);
    return false;
}

std::unordered_map<std::string, ObjectPtr> Environment::getAll() const {
    std::unordered_map<std::string, ObjectPtr> result;
    for (auto& [k, v] : store) result[k] = v;
    return result;
}

bool Environment::hasLocal(const std::string& name) const {
    for (auto& [k, v] : store) { if (k == name) return true; }
    return false;
}

std::shared_ptr<Environment> newEnvironment() {
    return std::make_shared<Environment>();
}

std::shared_ptr<Environment> newEnclosedEnvironment(std::shared_ptr<Environment> outer) {
    auto env = std::make_shared<Environment>();
    env->outer = outer;
    return env;
}

// Environment pooling for performance (single-threaded, no mutex needed)
static std::vector<std::shared_ptr<Environment>> envPool;

std::shared_ptr<Environment> getPooledEnvironment(std::shared_ptr<Environment> outer) {
    std::shared_ptr<Environment> env;
    if (!envPool.empty()) {
        env = envPool.back();
        envPool.pop_back();
    }
    if (!env) {
        env = std::make_shared<Environment>();
    }
    env->reset(outer);
    return env;
}

void returnPooledEnvironment(std::shared_ptr<Environment> env) {
    if (!env) return;
    env->store.clear();
    env->outer = nullptr;
    if (envPool.size() < 4096) {
        envPool.push_back(env);
    }
}

void Environment::reset(std::shared_ptr<Environment> newOuter) {
    store.clear();
    outer = newOuter;
}

// ============ Singletons ============

ObjectPtr getNull() {
    static auto null = std::make_shared<Null>();
    return null;
}

ObjectPtr getTrue() {
    static auto t = [] { auto b = std::make_shared<Boolean>(); b->value = true; return b; }();
    return t;
}

ObjectPtr getFalse() {
    static auto f = [] { auto b = std::make_shared<Boolean>(); b->value = false; return b; }();
    return f;
}

ObjectPtr newBoolean(bool value) { return value ? getTrue() : getFalse(); }

// ============ Small int cache ============

static std::shared_ptr<Integer> smallIntegers[256];
static bool smallIntsInit = false;

static void initSmallInts() {
    if (!smallIntsInit) {
        for (int i = 0; i < 256; i++) {
            smallIntegers[i] = std::make_shared<Integer>();
            smallIntegers[i]->value = i;
        }
        smallIntsInit = true;
    }
}

// ============ Constructors ============

ObjectPtr newInteger(int64_t value) {
    initSmallInts();
    if (value >= 0 && value < 256) return smallIntegers[value];
    auto obj = std::make_shared<Integer>();
    obj->value = value;
    return obj;
}

ObjectPtr newFloat(double value) {
    auto obj = std::make_shared<Float>();
    obj->value = value;
    return obj;
}

ObjectPtr newString(const std::string& value) {
    auto obj = std::make_shared<String>();
    obj->value = value;
    return obj;
}

ObjectPtr newArray(std::vector<ObjectPtr> elements) {
    auto obj = std::make_shared<Array>();
    obj->elements = std::move(elements);
    return obj;
}

ObjectPtr newMap(std::vector<std::pair<ObjectPtr, ObjectPtr>> pairs) {
    auto obj = std::make_shared<Map>();
    obj->pairs = std::move(pairs);
    return obj;
}

ObjectPtr newHash(std::unordered_map<HashKey, HashPair, HashKeyHash> pairs) {
    auto obj = std::make_shared<Hash>();
    obj->pairs = std::move(pairs);
    return obj;
}

ObjectPtr newError(const std::string& format, ...) {
    char buf[1024];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buf, sizeof(buf), format.c_str(), args);
    va_end(args);
    auto obj = std::make_shared<Error>();
    obj->message = buf;
    obj->errorType = "RuntimeError";
    return obj;
}

ObjectPtr newException(const std::string& exType, const std::string& message) {
    auto obj = std::make_shared<Exception>();
    obj->exceptionType = exType;
    obj->message = message;
    return obj;
}

ObjectPtr newExceptionSignal(std::shared_ptr<Exception> ex) {
    auto obj = std::make_shared<ExceptionSignal>();
    obj->exception = ex;
    return obj;
}

ObjectPtr newClass(const std::string& name) {
    auto obj = std::make_shared<Class>();
    obj->name = name;
    return obj;
}

ObjectPtr newInstance(std::shared_ptr<Class> cls) {
    auto obj = std::make_shared<Instance>();
    obj->cls = cls;
    return obj;
}

ObjectPtr newBoundMethod(std::shared_ptr<Instance> self, std::shared_ptr<Function> fn) {
    auto obj = std::make_shared<BoundMethod>();
    obj->self = self;
    obj->fn = fn;
    return obj;
}

// ============ Helpers ============

bool equals(ObjectPtr a, ObjectPtr b) {
    if (!a || !b) return false;
    if (a->type() != b->type()) return false;
    switch (a->type()) {
        case ObjectType::INTEGER:
            return std::dynamic_pointer_cast<Integer>(a)->value == std::dynamic_pointer_cast<Integer>(b)->value;
        case ObjectType::FLOAT:
            return std::dynamic_pointer_cast<Float>(a)->value == std::dynamic_pointer_cast<Float>(b)->value;
        case ObjectType::STRING:
            return std::dynamic_pointer_cast<String>(a)->value == std::dynamic_pointer_cast<String>(b)->value;
        case ObjectType::BOOLEAN:
            return std::dynamic_pointer_cast<Boolean>(a)->value == std::dynamic_pointer_cast<Boolean>(b)->value;
        case ObjectType::ARRAY: {
            auto aa = std::dynamic_pointer_cast<Array>(a);
            auto bb = std::dynamic_pointer_cast<Array>(b);
            if (aa->elements.size() != bb->elements.size()) return false;
            for (size_t i = 0; i < aa->elements.size(); i++)
                if (!equals(aa->elements[i], bb->elements[i])) return false;
            return true;
        }
        case ObjectType::MAP: {
            auto ma = std::dynamic_pointer_cast<Map>(a);
            auto mb = std::dynamic_pointer_cast<Map>(b);
            if (ma->pairs.size() != mb->pairs.size()) return false;
            for (auto& [ka, va] : ma->pairs) {
                bool found = false;
                for (auto& [kb, vb] : mb->pairs) {
                    if (equals(ka, kb) && equals(va, vb)) { found = true; break; }
                }
                if (!found) return false;
            }
            return true;
        }
        default:
            return false;
    }
}

bool isTruthy(ObjectPtr obj) {
    if (!obj) return false;
    if (obj == getNull()) return false;
    if (obj == getFalse()) return false;
    if (obj == getTrue()) return true;
    switch (obj->type()) {
        case ObjectType::INTEGER:
            return std::dynamic_pointer_cast<Integer>(obj)->value != 0;
        case ObjectType::FLOAT:
            return std::dynamic_pointer_cast<Float>(obj)->value != 0;
        case ObjectType::STRING:
            return !std::dynamic_pointer_cast<String>(obj)->value.empty();
        default:
            return true;
    }
}

// ============ Pooled constructors ============

ObjectPtr newIntegerFromPool(int64_t value) { return newInteger(value); }
ObjectPtr newFloatFromPool(double value) { return newFloat(value); }
ObjectPtr newStringFromPool(const std::string& value) { return newString(value); }
ObjectPtr newArrayFromPool(std::vector<ObjectPtr> elements) { return newArray(std::move(elements)); }

// ============ Fast arithmetic ============

ObjectPtr addIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right) {
    return newIntegerFromPool(left->value + right->value);
}

ObjectPtr subIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right) {
    return newIntegerFromPool(left->value - right->value);
}

ObjectPtr mulIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right) {
    return newIntegerFromPool(left->value * right->value);
}

ObjectPtr divIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right) {
    if (right->value == 0) return newError("division by zero");
    return newIntegerFromPool(left->value / right->value);
}

ObjectPtr modIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right) {
    if (right->value == 0) return newError("modulo by zero");
    return newIntegerFromPool(left->value % right->value);
}

ObjectPtr addFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right) {
    return newFloatFromPool(left->value + right->value);
}

ObjectPtr subFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right) {
    return newFloatFromPool(left->value - right->value);
}

ObjectPtr mulFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right) {
    return newFloatFromPool(left->value * right->value);
}

ObjectPtr divFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right) {
    if (right->value == 0) return newError("division by zero");
    return newFloatFromPool(left->value / right->value);
}

ObjectPtr concatStrings(std::shared_ptr<String> left, std::shared_ptr<String> right) {
    return newStringFromPool(left->value + right->value);
}

ObjectPtr concatMultipleStrings(const std::vector<std::shared_ptr<String>>& parts) {
    if (parts.empty()) return newStringFromPool("");
    if (parts.size() == 1) return parts[0];
    std::string result;
    for (const auto& s : parts) result += s->value;
    return newStringFromPool(result);
}

} // namespace darix
