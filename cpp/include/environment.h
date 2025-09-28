#pragma once

#include "object.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace darix {

class Environment {
public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> enclosing);

    void define(const std::string& name, std::shared_ptr<Object> value);
    std::shared_ptr<Object> get(const std::string& name) const;
    void assign(const std::string& name, std::shared_ptr<Object> value);
    std::shared_ptr<Object> getAt(size_t distance, const std::string& name) const;
    void assignAt(size_t distance, const std::string& name, std::shared_ptr<Object> value);

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> values_;
    std::shared_ptr<Environment> enclosing_;
};

} // namespace darix
