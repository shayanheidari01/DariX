#include "darix/native/native.hpp"

namespace darix::native {

Registry& Registry::instance() {
    static Registry reg;
    return reg;
}

void Registry::registerModule(const std::string& name, const std::unordered_map<std::string, NativeFunc>& funcs) {
    NativeModule mod;
    mod.name = name;
    mod.functions = funcs;
    modules_[name] = std::move(mod);
}

const NativeModule* Registry::get(const std::string& name) const {
    auto it = modules_.find(name);
    if (it != modules_.end()) return &it->second;
    return nullptr;
}

void Registry::setEvalCallback(EvalCallback cb) { evalCallback_ = std::move(cb); }
EvalCallback Registry::getEvalCallback() const { return evalCallback_; }

void Registry::initAll() {
    initMathModule();
    initStringModule();
    initArrayModule();
    initMapModule();
    initSetModule();
    initQueueModule();
    initStackModule();
    initLinkedListModule();
    initTreeModule();
    initGraphModule();
    initJsonModule();
    initFsModule();
    initNetModule();
    initCryptoModule();
    initDatetimeModule();
    initRandomModule();
    initRegexModule();
    initIoModule();
    initOsModule();
    initEncodingModule();
}

ObjectPtr callCallable(ObjectPtr callable, const std::vector<ObjectPtr>& args) {
    // Try builtin first
    if (auto builtin = std::dynamic_pointer_cast<Builtin>(callable)) {
        return builtin->fn(args);
    }
    // Try user-defined function via interpreter callback
    auto cb = Registry::instance().getEvalCallback();
    if (cb) {
        return cb(callable, args);
    }
    return newError("cannot call: no evaluator available for function type");
}

} // namespace darix::native
