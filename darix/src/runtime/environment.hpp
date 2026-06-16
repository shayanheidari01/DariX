#ifndef DARIX_RUNTIME_ENVIRONMENT_HPP
#define DARIX_RUNTIME_ENVIRONMENT_HPP

#include "runtime/value.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

namespace darix {

class Environment;
using EnvironmentPtr = std::shared_ptr<Environment>;

class Environment {
public:
    explicit Environment(EnvironmentPtr enclosing = nullptr) 
        : enclosing_(enclosing) {}
    
    void define(const std::string& name, ValuePtr value);
    ValuePtr get(const std::string& name);
    ValuePtr getAt(size_t distance, const std::string& name);
    void assign(const std::string& name, ValuePtr value);
    void assignAt(size_t distance, const std::string& name, ValuePtr value);
    
    [[nodiscard]] EnvironmentPtr enclosing() const { return enclosing_; }
    [[nodiscard]] const std::unordered_map<std::string, ValuePtr>& values() const { return values_; }
    
    // Create a child environment
    [[nodiscard]] EnvironmentPtr child() const {
        return std::make_shared<Environment>(const_cast<Environment*>(this)->shared_from_this());
    }
    
private:
    std::unordered_map<std::string, ValuePtr> values_;
    std::weak_ptr<Environment> enclosing_;
};

} // namespace darix

#endif // DARIX_RUNTIME_ENVIRONMENT_HPP
