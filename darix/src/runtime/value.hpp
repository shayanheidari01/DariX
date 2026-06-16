#ifndef DARIX_RUNTIME_VALUE_HPP
#define DARIX_RUNTIME_VALUE_HPP

#include "lexer/token.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <functional>
#include <optional>

namespace darix {

// Forward declarations
class Value;
class Object;
class Function;
class Instance;
class Class;

using ValuePtr = std::shared_ptr<Value>;

// Value types
enum class ValueType {
    NIL,
    BOOLEAN,
    INTEGER,
    FLOAT,
    STRING,
    LIST,
    DICT,
    FUNCTION,
    NATIVE_FUNCTION,
    CLASS,
    INSTANCE,
    MODULE,
    ERROR
};

class Value {
public:
    virtual ~Value() = default;
    [[nodiscard]] virtual ValueType type() const = 0;
    [[nodiscard]] virtual std::string toString() const = 0;
    [[nodiscard]] virtual bool isTruthy() const = 0;
    [[nodiscard]] virtual bool equals(const Value& other) const = 0;
};

class NilValue : public Value {
public:
    [[nodiscard]] ValueType type() const override { return ValueType::NIL; }
    [[nodiscard]] std::string toString() const override { return "null"; }
    [[nodiscard]] bool isTruthy() const override { return false; }
    [[nodiscard]] bool equals(const Value& other) const override {
        return other.type() == ValueType::NIL;
    }
};

class BooleanValue : public Value {
public:
    explicit BooleanValue(bool value) : value_(value) {}
    [[nodiscard]] ValueType type() const override { return ValueType::BOOLEAN; }
    [[nodiscard]] std::string toString() const override { return value_ ? "true" : "false"; }
    [[nodiscard]] bool isTruthy() const override { return value_; }
    [[nodiscard]] bool equals(const Value& other) const override {
        if (other.type() != ValueType::BOOLEAN) return false;
        return value_ == static_cast<const BooleanValue&>(other).value_;
    }
    [[nodiscard]] bool value() const { return value_; }
private:
    bool value_;
};

class IntValue : public Value {
public:
    explicit IntValue(int64_t value) : value_(value) {}
    [[nodiscard]] ValueType type() const override { return ValueType::INTEGER; }
    [[nodiscard]] std::string toString() const override { return std::to_string(value_); }
    [[nodiscard]] bool isTruthy() const override { return value_ != 0; }
    [[nodiscard]] bool equals(const Value& other) const override {
        if (other.type() != ValueType::INTEGER) return false;
        return value_ == static_cast<const IntValue&>(other).value_;
    }
    [[nodiscard]] int64_t value() const { return value_; }
private:
    int64_t value_;
};

class FloatValue : public Value {
public:
    explicit FloatValue(double value) : value_(value) {}
    [[nodiscard]] ValueType type() const override { return ValueType::FLOAT; }
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] bool isTruthy() const override { return value_ != 0.0; }
    [[nodiscard]] bool equals(const Value& other) const override {
        if (other.type() != ValueType::FLOAT) return false;
        return value_ == static_cast<const FloatValue&>(other).value_;
    }
    [[nodiscard]] double value() const { return value_; }
private:
    double value_;
};

class StringValue : public Value {
public:
    explicit StringValue(std::string value) : value_(std::move(value)) {}
    [[nodiscard]] ValueType type() const override { return ValueType::STRING; }
    [[nodiscard]] std::string toString() const override { return value_; }
    [[nodiscard]] bool isTruthy() const override { return !value_.empty(); }
    [[nodiscard]] bool equals(const Value& other) const override {
        if (other.type() != ValueType::STRING) return false;
        return value_ == static_cast<const StringValue&>(other).value_;
    }
    [[nodiscard]] const std::string& value() const { return value_; }
private:
    std::string value_;
};

class ListValue;
class DictValue;
class FunctionValue;
class ClassValue;
class InstanceValue;

// Helper factory functions
inline ValuePtr makeNil() { return std::make_shared<NilValue>(); }
inline ValuePtr makeBool(bool b) { return std::make_shared<BooleanValue>(b); }
inline ValuePtr makeInt(int64_t i) { return std::make_shared<IntValue>(i); }
inline ValuePtr makeFloat(double d) { return std::make_shared<FloatValue>(d); }
inline ValuePtr makeString(std::string s) { return std::make_shared<StringValue>(std::move(s)); }

} // namespace darix

#endif // DARIX_RUNTIME_VALUE_HPP
