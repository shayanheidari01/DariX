#pragma once

#include "darix/object.hpp"
#include <string>
#include <unordered_map>
#include <functional>

namespace darix::native {

using NativeFunc = std::function<ObjectPtr(const std::vector<ObjectPtr>&)>;
// Callback for evaluating a callable (Builtin or user-defined Function) with args
using EvalCallback = std::function<ObjectPtr(ObjectPtr callable, const std::vector<ObjectPtr>& args)>;

struct NativeModule {
    std::string name;
    std::unordered_map<std::string, NativeFunc> functions;
};

class Registry {
public:
    static Registry& instance();

    void registerModule(const std::string& name, const std::unordered_map<std::string, NativeFunc>& funcs);
    const NativeModule* get(const std::string& name) const;

    void setEvalCallback(EvalCallback cb);
    EvalCallback getEvalCallback() const;

    void initAll();

private:
    Registry() = default;
    std::unordered_map<std::string, NativeModule> modules_;
    EvalCallback evalCallback_;
};

// Helper: call any callable (builtin or user-defined function)
ObjectPtr callCallable(ObjectPtr callable, const std::vector<ObjectPtr>& args);

void initMathModule();
void initStringModule();
void initArrayModule();
void initMapModule();
void initSetModule();
void initQueueModule();
void initStackModule();
void initLinkedListModule();
void initTreeModule();
void initGraphModule();
void initJsonModule();
void initFsModule();
void initNetModule();
void initCryptoModule();
void initDatetimeModule();
void initRandomModule();
void initRegexModule();
void initIoModule();
void initOsModule();
void initEncodingModule();

} // namespace darix::native
