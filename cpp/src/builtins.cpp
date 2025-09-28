#include "builtins.h"
#include <iostream>
#include <cmath>

namespace darix {

void defineBuiltins(Interpreter& interpreter) {
    // print function
    auto printFunc = makeFunction("print", [](const std::vector<std::shared_ptr<Object>>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << args[i]->toString();
        }
        std::cout << std::endl;
        return makeNull();
    });

    interpreter.globals_->define("print", printFunc);

    // len function
    auto lenFunc = makeFunction("len", [](const std::vector<std::shared_ptr<Object>>& args) -> std::shared_ptr<Object> {
        if (args.size() != 1) return makeNull();

        auto obj = args[0];
        if (obj->type() == ObjectType::STRING) {
            auto str = static_cast<StringObject*>(obj.get());
            return makeInteger(str->value().length());
        } else if (obj->type() == ObjectType::ARRAY) {
            auto arr = static_cast<ArrayObject*>(obj.get());
            return makeInteger(arr->size());
        }

        return makeInteger(0);
    }, 1);

    interpreter.globals_->define("len", lenFunc);

    // type function
    auto typeFunc = makeFunction("type", [](const std::vector<std::shared_ptr<Object>>& args) -> std::shared_ptr<Object> {
        if (args.size() != 1) return makeString("unknown");

        auto obj = args[0];
        switch (obj->type()) {
            case ObjectType::INTEGER: return makeString("int");
            case ObjectType::FLOAT: return makeString("float");
            case ObjectType::STRING: return makeString("string");
            case ObjectType::BOOLEAN: return makeString("bool");
            case ObjectType::ARRAY: return makeString("array");
            case ObjectType::MAP: return makeString("map");
            case ObjectType::FUNCTION: return makeString("function");
            case ObjectType::CLASS: return makeString("class");
            case ObjectType::INSTANCE: return makeString("instance");
            default: return makeString("null");
        }
    }, 1);

    interpreter.globals_->define("type", typeFunc);

    // abs function
    auto absFunc = makeFunction("abs", [](const std::vector<std::shared_ptr<Object>>& args) -> std::shared_ptr<Object> {
        if (args.size() != 1) return makeNull();

        auto obj = args[0];
        if (obj->type() == ObjectType::INTEGER) {
            auto num = static_cast<IntegerObject*>(obj.get());
            return makeInteger(std::abs(num->value()));
        } else if (obj->type() == ObjectType::FLOAT) {
            auto num = static_cast<FloatObject*>(obj.get());
            return makeFloat(std::abs(num->value()));
        }

        return makeNull();
    }, 1);

    interpreter.globals_->define("abs", absFunc);

    // str function
    auto strFunc = makeFunction("str", [](const std::vector<std::shared_ptr<Object>>& args) -> std::shared_ptr<Object> {
        if (args.size() != 1) return makeString("");

        return makeString(args[0]->toString());
    }, 1);

    interpreter.globals_->define("str", strFunc);

    // int function
    auto intFunc = makeFunction("int", [](const std::vector<std::shared_ptr<Object>>& args) -> std::shared_ptr<Object> {
        if (args.size() != 1) return makeInteger(0);

        auto obj = args[0];
        if (obj->type() == ObjectType::INTEGER) {
            return obj;
        } else if (obj->type() == ObjectType::FLOAT) {
            auto num = static_cast<FloatObject*>(obj.get());
            return makeInteger(static_cast<long long>(num->value()));
        } else if (obj->type() == ObjectType::STRING) {
            auto str = static_cast<StringObject*>(obj.get());
            try {
                return makeInteger(std::stoll(str->value()));
            } catch (...) {
                return makeInteger(0);
            }
        }

        return makeInteger(0);
    }, 1);

    interpreter.globals_->define("int", intFunc);

    // float function
    auto floatFunc = makeFunction("float", [](const std::vector<std::shared_ptr<Object>>& args) -> std::shared_ptr<Object> {
        if (args.size() != 1) return makeFloat(0.0);

        auto obj = args[0];
        if (obj->type() == ObjectType::FLOAT) {
            return obj;
        } else if (obj->type() == ObjectType::INTEGER) {
            auto num = static_cast<IntegerObject*>(obj.get());
            return makeFloat(static_cast<double>(num->value()));
        } else if (obj->type() == ObjectType::STRING) {
            auto str = static_cast<StringObject*>(obj.get());
            try {
                return makeFloat(std::stod(str->value()));
            } catch (...) {
                return makeFloat(0.0);
            }
        }

        return makeFloat(0.0);
    }, 1);

    interpreter.globals_->define("float", floatFunc);
}

} // namespace darix
