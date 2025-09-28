#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>

namespace darix {

// Forward declarations
class Object;
class ArrayObject;
class MapObject;
class ClassObject;
class InstanceObject;
class FunctionObject;

// Object type enumeration
enum class ObjectType {
    INTEGER, FLOAT, STRING, BOOLEAN, NULL_OBJ,
    ARRAY, MAP, FUNCTION, CLASS, INSTANCE
};

// Base object class
class Object {
public:
    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
    virtual std::string toString() const = 0;
    virtual bool equals(const Object* other) const = 0;
    virtual size_t hash() const = 0;
};

// Primitive types
class IntegerObject : public Object {
public:
    explicit IntegerObject(long long value) : value_(value) {}
    ObjectType type() const override { return ObjectType::INTEGER; }
    std::string toString() const override { return std::to_string(value_); }
    bool equals(const Object* other) const override;
    size_t hash() const override { return std::hash<long long>{}(value_); }
    long long value() const { return value_; }
private:
    long long value_;
};

class FloatObject : public Object {
public:
    explicit FloatObject(double value) : value_(value) {}
    ObjectType type() const override { return ObjectType::FLOAT; }
    std::string toString() const override { return std::to_string(value_); }
    bool equals(const Object* other) const override;
    size_t hash() const override { return std::hash<double>{}(value_); }
    double value() const { return value_; }
private:
    double value_;
};

class StringObject : public Object {
public:
    explicit StringObject(std::string value) : value_(std::move(value)) {}
    ObjectType type() const override { return ObjectType::STRING; }
    std::string toString() const override { return "\"" + value_ + "\""; }
    bool equals(const Object* other) const override;
    size_t hash() const override { return std::hash<std::string>{}(value_); }
    const std::string& value() const { return value_; }
private:
    std::string value_;
};

class BooleanObject : public Object {
public:
    explicit BooleanObject(bool value) : value_(value) {}
    ObjectType type() const override { return ObjectType::BOOLEAN; }
    std::string toString() const override { return value_ ? "true" : "false"; }
    bool equals(const Object* other) const override;
    size_t hash() const override { return std::hash<bool>{}(value_); }
    bool value() const { return value_; }
private:
    bool value_;
};

class NullObject : public Object {
public:
    ObjectType type() const override { return ObjectType::NULL_OBJ; }
    std::string toString() const override { return "null"; }
    bool equals(const Object* other) const override;
    size_t hash() const override { return 0; }
};

// Array object
class ArrayObject : public Object {
public:
    explicit ArrayObject(std::vector<std::shared_ptr<Object>> elements)
        : elements_(std::move(elements)) {}
    ObjectType type() const override { return ObjectType::ARRAY; }
    std::string toString() const override;
    bool equals(const Object* other) const override;
    size_t hash() const override;
    const std::vector<std::shared_ptr<Object>>& elements() const { return elements_; }
    std::vector<std::shared_ptr<Object>>& elements() { return elements_; }
    size_t size() const { return elements_.size(); }
    std::shared_ptr<Object>& operator[](size_t index) { return elements_[index]; }
private:
    std::vector<std::shared_ptr<Object>> elements_;
};

// Map object
class MapObject : public Object {
public:
    explicit MapObject(std::unordered_map<std::string, std::shared_ptr<Object>> map)
        : map_(std::move(map)) {}
    ObjectType type() const override { return ObjectType::MAP; }
    std::string toString() const override;
    bool equals(const Object* other) const override;
    size_t hash() const override;
    const std::unordered_map<std::string, std::shared_ptr<Object>>& map() const { return map_; }
    std::unordered_map<std::string, std::shared_ptr<Object>>& map() { return map_; }
    std::shared_ptr<Object>& operator[](const std::string& key) { return map_[key]; }
    bool contains(const std::string& key) const { return map_.contains(key); }
private:
    std::unordered_map<std::string, std::shared_ptr<Object>> map_;
};

// Function object
class FunctionObject : public Object {
public:
    using Callable = std::function<std::shared_ptr<Object>(
        const std::vector<std::shared_ptr<Object>>& args)>;

    explicit FunctionObject(std::string name, Callable callable, size_t arity = 0)
        : name_(std::move(name)), callable_(std::move(callable)), arity_(arity) {}
    ObjectType type() const override { return ObjectType::FUNCTION; }
    std::string toString() const override { return "<function " + name_ + ">"; }
    bool equals(const Object* other) const override;
    size_t hash() const override { return std::hash<std::string>{}(name_); }
    std::shared_ptr<Object> call(const std::vector<std::shared_ptr<Object>>& args) const;
    size_t arity() const { return arity_; }
    const std::string& name() const { return name_; }
private:
    std::string name_;
    Callable callable_;
    size_t arity_;
};

// Class object
class ClassObject : public Object {
public:
    ClassObject(std::string name, std::unordered_map<std::string, std::shared_ptr<FunctionObject>> methods)
        : name_(std::move(name)), methods_(std::move(methods)) {}
    ObjectType type() const override { return ObjectType::CLASS; }
    std::string toString() const override { return "<class " + name_ + ">"; }
    bool equals(const Object* other) const override;
    size_t hash() const override { return std::hash<std::string>{}(name_); }
    std::shared_ptr<InstanceObject> instantiate() const;
    const std::unordered_map<std::string, std::shared_ptr<FunctionObject>>& methods() const {
        return methods_;
    }
    const std::string& name() const { return name_; }
private:
    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<FunctionObject>> methods_;
};

// Instance object
class InstanceObject : public Object {
public:
    explicit InstanceObject(std::shared_ptr<ClassObject> classObj)
        : class_(std::move(classObj)) {}
    ObjectType type() const override { return ObjectType::INSTANCE; }
    std::string toString() const override { return "<" + class_->name() + " instance>"; }
    bool equals(const Object* other) const override;
    size_t hash() const override;
    std::shared_ptr<Object>& get(const std::string& property);
    void set(const std::string& property, std::shared_ptr<Object> value);
    std::shared_ptr<FunctionObject> getMethod(const std::string& name) const;
    const std::shared_ptr<ClassObject>& classObj() const { return class_; }
private:
    std::shared_ptr<ClassObject> class_;
    std::unordered_map<std::string, std::shared_ptr<Object>> fields_;
};

// Utility functions
std::shared_ptr<IntegerObject> makeInteger(long long value);
std::shared_ptr<FloatObject> makeFloat(double value);
std::shared_ptr<StringObject> makeString(std::string value);
std::shared_ptr<BooleanObject> makeBoolean(bool value);
std::shared_ptr<NullObject> makeNull();
std::shared_ptr<ArrayObject> makeArray(std::vector<std::shared_ptr<Object>> elements = {});
std::shared_ptr<MapObject> makeMap(std::unordered_map<std::string, std::shared_ptr<Object>> map = {});
std::shared_ptr<FunctionObject> makeFunction(std::string name,
    FunctionObject::Callable callable, size_t arity = 0);

} // namespace darix
