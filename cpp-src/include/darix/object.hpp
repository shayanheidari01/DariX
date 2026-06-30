#pragma once

#include "darix/ast.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace darix {

// Forward declarations
struct Object;
using ObjectPtr = std::shared_ptr<Object>;

// Object types
enum class ObjectType {
    INTEGER,
    FLOAT,
    BOOLEAN,
    NULL_OBJ,
    RETURN_VALUE,
    ERROR,
    FUNCTION,
    STRING,
    ARRAY,
    MAP,
    BUILTIN,
    HASH,
    EXCEPTION,
    STACK_TRACE,
    CLASS,
    INSTANCE,
    BOUND_METHOD,
    COMPILED_FUNCTION,
    MODULE,
    BREAK_SIGNAL,
    CONTINUE_SIGNAL,
    EXCEPTION_SIGNAL,
};

const char* ObjectTypeToString(ObjectType type);

// Position in source code
struct Position {
    std::string filename;
    int line = 0;
    int column = 0;
    std::string str() const;
};

// Stack frame for error traces
struct StackFrame {
    std::string functionName;
    Position position;
    std::string context;
    std::string str() const;
};

// Base Object
struct Object {
    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
    virtual std::string inspect() const = 0;
    virtual void free() {}
};

// ============ Concrete types ============

struct Integer : Object {
    int64_t value = 0;
    ObjectType type() const override { return ObjectType::INTEGER; }
    std::string inspect() const override;
    uint64_t hashKey() const;
};

struct Float : Object {
    double value = 0.0;
    ObjectType type() const override { return ObjectType::FLOAT; }
    std::string inspect() const override;
};

struct Boolean : Object {
    bool value = false;
    ObjectType type() const override { return ObjectType::BOOLEAN; }
    std::string inspect() const override;
};

struct Null : Object {
    ObjectType type() const override { return ObjectType::NULL_OBJ; }
    std::string inspect() const override { return "null"; }
};

struct String : Object {
    std::string value;
    ObjectType type() const override { return ObjectType::STRING; }
    std::string inspect() const override;
    uint64_t hashKey() const;
};

struct Array : Object {
    std::vector<ObjectPtr> elements;
    ObjectType type() const override { return ObjectType::ARRAY; }
    std::string inspect() const override;
};

struct ReturnValue : Object {
    ObjectPtr value;
    ObjectType type() const override { return ObjectType::RETURN_VALUE; }
    std::string inspect() const override;
};

struct BreakSignal : Object {
    ObjectType type() const override { return ObjectType::BREAK_SIGNAL; }
    std::string inspect() const override { return "break"; }
};

struct ContinueSignal : Object {
    ObjectType type() const override { return ObjectType::CONTINUE_SIGNAL; }
    std::string inspect() const override { return "continue"; }
};

// StackTrace
struct StackTrace : Object {
    std::vector<StackFrame> frames;
    ObjectType type() const override { return ObjectType::STACK_TRACE; }
    std::string inspect() const override;
};

// Exception
struct Exception : Object {
    std::string exceptionType;
    std::string message;
    std::shared_ptr<StackTrace> stackTrace;
    std::shared_ptr<Exception> cause;
    ObjectType type() const override { return ObjectType::EXCEPTION; }
    std::string inspect() const override;
};

// ExceptionSignal (control flow)
struct ExceptionSignal : Object {
    std::shared_ptr<Exception> exception;
    ObjectType type() const override { return ObjectType::EXCEPTION_SIGNAL; }
    std::string inspect() const override;
};

// Error
struct Error : Object {
    std::string message;
    std::string errorType;
    Position position;
    std::vector<StackFrame> stackTrace;
    std::string suggestion;
    ObjectType type() const override { return ObjectType::ERROR; }
    std::string inspect() const override;
    void addStackFrame(const std::string& fn, const Position& pos, const std::string& ctx);
};

// Environment
struct Environment {
    std::unordered_map<std::string, ObjectPtr> store;
    std::shared_ptr<Environment> outer;

    ObjectPtr get(const std::string& name) const;
    ObjectPtr set(const std::string& name, ObjectPtr val);
    bool update(const std::string& name, ObjectPtr val);
    bool erase(const std::string& name);
    std::unordered_map<std::string, ObjectPtr> getAll() const;
    bool hasLocal(const std::string& name) const;
    std::shared_ptr<Environment> outerEnv() const { return outer; }
};

std::shared_ptr<Environment> newEnvironment();
std::shared_ptr<Environment> newEnclosedEnvironment(std::shared_ptr<Environment> outer);

// Function
struct Function : Object {
    std::string name;
    std::vector<IdentifierPtr> parameters;
    std::shared_ptr<BlockStatement> body;
    std::shared_ptr<Environment> env;
    ObjectType type() const override { return ObjectType::FUNCTION; }
    std::string inspect() const override;
};

// CompiledFunction
struct CompiledFunction : Object {
    std::vector<uint8_t> instructions;
    int numLocals = 0;
    int numParameters = 0;
    std::string name;
    ObjectType type() const override { return ObjectType::COMPILED_FUNCTION; }
    std::string inspect() const override;
};

// BuiltinFunction
using BuiltinFunction = std::function<ObjectPtr(const std::vector<ObjectPtr>&)>;

struct Builtin : Object {
    BuiltinFunction fn;
    ObjectType type() const override { return ObjectType::BUILTIN; }
    std::string inspect() const override { return "builtin function"; }
};

// Map
struct Map : Object {
    std::vector<std::pair<ObjectPtr, ObjectPtr>> pairs;
    ObjectType type() const override { return ObjectType::MAP; }
    std::string inspect() const override;
};

// Hash
struct HashPair {
    ObjectPtr key;
    ObjectPtr value;
};

struct HashKey {
    ObjectType type;
    uint64_t value;
    bool operator==(const HashKey& other) const { return type == other.type && value == other.value; }
};

struct HashKeyHash {
    size_t operator()(const HashKey& k) const {
        size_t h1 = std::hash<int>{}(static_cast<int>(k.type));
        size_t h2 = std::hash<uint64_t>{}(k.value);
        return h1 ^ (h2 * 0x9e3779b97f4a7c15ULL);
    }
};

struct Hash : Object {
    std::unordered_map<HashKey, HashPair, HashKeyHash> pairs;
    ObjectType type() const override { return ObjectType::HASH; }
    std::string inspect() const override;
};

// Class
struct Class : Object {
    std::string name;
    std::unordered_map<std::string, ObjectPtr> members;
    ObjectType type() const override { return ObjectType::CLASS; }
    std::string inspect() const override;
};

// Instance
struct Instance : Object {
    std::shared_ptr<Class> cls;
    std::unordered_map<std::string, ObjectPtr> fields;
    ObjectType type() const override { return ObjectType::INSTANCE; }
    std::string inspect() const override;
};

// BoundMethod
struct BoundMethod : Object {
    std::shared_ptr<Instance> self;
    std::shared_ptr<Function> fn;
    ObjectType type() const override { return ObjectType::BOUND_METHOD; }
    std::string inspect() const override;
};

// Module
struct Module : Object {
    std::shared_ptr<Environment> env;
    std::string path;
    ObjectType type() const override { return ObjectType::MODULE; }
    std::string inspect() const override;
};

// ============ Singletons ============

ObjectPtr getNull();
ObjectPtr getTrue();
ObjectPtr getFalse();
ObjectPtr newBoolean(bool value);

// ============ Constructors ============

ObjectPtr newInteger(int64_t value);
ObjectPtr newFloat(double value);
ObjectPtr newString(const std::string& value);
ObjectPtr newArray(std::vector<ObjectPtr> elements);
ObjectPtr newMap(std::vector<std::pair<ObjectPtr, ObjectPtr>> pairs);
ObjectPtr newHash(std::unordered_map<HashKey, HashPair, HashKeyHash> pairs);
ObjectPtr newError(const std::string& format, ...);
ObjectPtr newException(const std::string& exType, const std::string& message);
ObjectPtr newExceptionSignal(std::shared_ptr<Exception> ex);
ObjectPtr newClass(const std::string& name);
ObjectPtr newInstance(std::shared_ptr<Class> cls);
ObjectPtr newBoundMethod(std::shared_ptr<Instance> self, std::shared_ptr<Function> fn);

// ============ Helpers ============

bool equals(ObjectPtr a, ObjectPtr b);
bool isTruthy(ObjectPtr obj);

// ============ Pooled constructors ============

ObjectPtr newIntegerFromPool(int64_t value);
ObjectPtr newFloatFromPool(double value);
ObjectPtr newStringFromPool(const std::string& value);
ObjectPtr newArrayFromPool(std::vector<ObjectPtr> elements);

// ============ Fast arithmetic ============

ObjectPtr addIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right);
ObjectPtr subIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right);
ObjectPtr mulIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right);
ObjectPtr divIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right);
ObjectPtr modIntegers(std::shared_ptr<Integer> left, std::shared_ptr<Integer> right);
ObjectPtr addFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right);
ObjectPtr subFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right);
ObjectPtr mulFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right);
ObjectPtr divFloats(std::shared_ptr<Float> left, std::shared_ptr<Float> right);
ObjectPtr concatStrings(std::shared_ptr<String> left, std::shared_ptr<String> right);
ObjectPtr concatMultipleStrings(const std::vector<std::shared_ptr<String>>& parts);

// Exception type constants
constexpr const char* VALUE_ERROR     = "ValueError";
constexpr const char* TYPE_ERROR      = "TypeError";
constexpr const char* NAME_ERROR      = "NameError";
constexpr const char* INDEX_ERROR     = "IndexError";
constexpr const char* KEY_ERROR       = "KeyError";
constexpr const char* ZERO_DIV_ERROR  = "ZeroDivisionError";
constexpr const char* RUNTIME_ERROR   = "RuntimeError";
constexpr const char* SYNTAX_ERROR    = "SyntaxError";
constexpr const char* ATTRIBUTE_ERROR = "AttributeError";
constexpr const char* ASSERTION_ERROR = "AssertionError";

} // namespace darix
