#include "object.h"
#include <sstream>
#include <algorithm>

namespace darix {

bool IntegerObject::equals(const Object* other) const {
    if (other->type() != ObjectType::INTEGER) return false;
    return value_ == static_cast<const IntegerObject*>(other)->value_;
}

bool FloatObject::equals(const Object* other) const {
    if (other->type() != ObjectType::FLOAT) return false;
    return value_ == static_cast<const FloatObject*>(other)->value_;
}

bool StringObject::equals(const Object* other) const {
    if (other->type() != ObjectType::STRING) return false;
    return value_ == static_cast<const StringObject*>(other)->value_;
}

bool BooleanObject::equals(const Object* other) const {
    if (other->type() != ObjectType::BOOLEAN) return false;
    return value_ == static_cast<const BooleanObject*>(other)->value_;
}

bool NullObject::equals(const Object* other) const {
    return other->type() == ObjectType::NULL_OBJ;
}

std::string ArrayObject::toString() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < elements_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements_[i]->toString();
    }
    ss << "]";
    return ss.str();
}

bool ArrayObject::equals(const Object* other) const {
    if (other->type() != ObjectType::ARRAY) return false;
    const auto* otherArray = static_cast<const ArrayObject*>(other);
    if (elements_.size() != otherArray->elements_.size()) return false;
    for (size_t i = 0; i < elements_.size(); ++i) {
        if (!elements_[i]->equals(otherArray->elements_[i].get())) return false;
    }
    return true;
}

size_t ArrayObject::hash() const {
    size_t h = 0;
    for (const auto& elem : elements_) {
        h ^= elem->hash();
    }
    return h;
}

std::string MapObject::toString() const {
    std::stringstream ss;
    ss << "{";
    size_t i = 0;
    for (const auto& pair : map_) {
        if (i > 0) ss << ", ";
        ss << "\"" << pair.first << "\": " << pair.second->toString();
        ++i;
    }
    ss << "}";
    return ss.str();
}

bool MapObject::equals(const Object* other) const {
    if (other->type() != ObjectType::MAP) return false;
    const auto* otherMap = static_cast<const MapObject*>(other);
    if (map_.size() != otherMap->map_.size()) return false;
    for (const auto& pair : map_) {
        auto it = otherMap->map_.find(pair.first);
        if (it == otherMap->map_.end()) return false;
        if (!pair.second->equals(it->second.get())) return false;
    }
    return true;
}

size_t MapObject::hash() const {
    size_t h = 0;
    for (const auto& pair : map_) {
        h ^= std::hash<std::string>{}(pair.first) ^ pair.second->hash();
    }
    return h;
}

bool FunctionObject::equals(const Object* other) const {
    if (other->type() != ObjectType::FUNCTION) return false;
    return name_ == static_cast<const FunctionObject*>(other)->name_;
}

std::shared_ptr<Object> FunctionObject::call(const std::vector<std::shared_ptr<Object>>& args) const {
    if (args.size() != arity_ && arity_ != 0) {
        // Error handling would go here
        return makeNull();
    }
    return callable_(args);
}

bool ClassObject::equals(const Object* other) const {
    if (other->type() != ObjectType::CLASS) return false;
    return name_ == static_cast<const ClassObject*>(other)->name_;
}

std::shared_ptr<InstanceObject> ClassObject::instantiate() const {
    return std::make_shared<InstanceObject>(std::make_shared<ClassObject>(*this));
}

std::shared_ptr<Object>& InstanceObject::get(const std::string& property) {
    if (fields_.contains(property)) {
        return fields_[property];
    }
    // Check for method
    auto method = getMethod(property);
    if (method) {
        // Bind method to this instance
        return fields_[property] = method;
    }
    // Return null for undefined properties
    static std::shared_ptr<Object> nullObj = makeNull();
    return nullObj;
}

void InstanceObject::set(const std::string& property, std::shared_ptr<Object> value) {
    fields_[property] = std::move(value);
}

std::shared_ptr<FunctionObject> InstanceObject::getMethod(const std::string& name) const {
    auto it = class_->methods().find(name);
    if (it != class_->methods().end()) {
        return it->second;
    }
    return nullptr;
}

bool InstanceObject::equals(const Object* other) const {
    return this == other; // Identity comparison for instances
}

size_t InstanceObject::hash() const {
    return reinterpret_cast<size_t>(this);
}

// Utility functions
std::shared_ptr<IntegerObject> makeInteger(long long value) {
    return std::make_shared<IntegerObject>(value);
}

std::shared_ptr<FloatObject> makeFloat(double value) {
    return std::make_shared<FloatObject>(value);
}

std::shared_ptr<StringObject> makeString(std::string value) {
    return std::make_shared<StringObject>(std::move(value));
}

std::shared_ptr<BooleanObject> makeBoolean(bool value) {
    return std::make_shared<BooleanObject>(value);
}

std::shared_ptr<NullObject> makeNull() {
    return std::make_shared<NullObject>();
}

std::shared_ptr<ArrayObject> makeArray(std::vector<std::shared_ptr<Object>> elements) {
    return std::make_shared<ArrayObject>(std::move(elements));
}

std::shared_ptr<MapObject> makeMap(std::unordered_map<std::string, std::shared_ptr<Object>> map) {
    return std::make_shared<MapObject>(std::move(map));
}

std::shared_ptr<FunctionObject> makeFunction(std::string name,
    FunctionObject::Callable callable, size_t arity) {
    return std::make_shared<FunctionObject>(std::move(name), std::move(callable), arity);
}

} // namespace darix
