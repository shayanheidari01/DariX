#include "environment.h"
#include <iostream>

namespace darix {

Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing_(std::move(enclosing)) {}

void Environment::define(const std::string& name, std::shared_ptr<Object> value) {
    values_[name] = std::move(value);
}

std::shared_ptr<Object> Environment::get(const std::string& name) const {
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }

    if (enclosing_) {
        return enclosing_->get(name);
    }

    // Undefined variable - return null for now
    return makeNull();
}

void Environment::assign(const std::string& name, std::shared_ptr<Object> value) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = std::move(value);
        return;
    }

    if (enclosing_) {
        enclosing_->assign(name, std::move(value));
        return;
    }

    // Undefined variable assignment - define it
    define(name, std::move(value));
}

std::shared_ptr<Object> Environment::getAt(size_t distance, const std::string& name) const {
    Environment* env = const_cast<Environment*>(this);
    for (size_t i = 0; i < distance; ++i) {
        env = env->enclosing_.get();
    }
    return env->values_[name];
}

void Environment::assignAt(size_t distance, const std::string& name, std::shared_ptr<Object> value) {
    Environment* env = this;
    for (size_t i = 0; i < distance; ++i) {
        env = env->enclosing_.get();
    }
    env->values_[name] = std::move(value);
}

} // namespace darix
