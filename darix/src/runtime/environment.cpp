#include "runtime/environment.hpp"
#include <stdexcept>

namespace darix {

void Environment::define(const std::string& name, ValuePtr value) {
    values_[name] = std::move(value);
}

ValuePtr Environment::get(const std::string& name) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }
    
    auto enc = enclosing_.lock();
    if (enc) {
        return enc->get(name);
    }
    
    throw std::runtime_error("Undefined variable: " + name);
}

ValuePtr Environment::getAt(size_t distance, const std::string& name) {
    if (distance == 0) {
        return get(name);
    }
    
    auto enc = enclosing_.lock();
    if (!enc) {
        throw std::runtime_error("Scope chain ended prematurely");
    }
    
    return enc->getAt(distance - 1, name);
}

void Environment::assign(const std::string& name, ValuePtr value) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = std::move(value);
        return;
    }
    
    auto enc = enclosing_.lock();
    if (enc) {
        enc->assign(name, std::move(value));
        return;
    }
    
    throw std::runtime_error("Undefined variable: " + name);
}

void Environment::assignAt(size_t distance, const std::string& name, ValuePtr value) {
    if (distance == 0) {
        assign(name, std::move(value));
        return;
    }
    
    auto enc = enclosing_.lock();
    if (!enc) {
        throw std::runtime_error("Scope chain ended prematurely");
    }
    
    enc->assignAt(distance - 1, name, std::move(value));
}

} // namespace darix
