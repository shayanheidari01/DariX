#define _USE_MATH_DEFINES
#include "darix/native/native.hpp"
#include <cmath>
#include <random>

namespace darix::native {

static double getFloat(ObjectPtr obj) {
    if (auto i = std::dynamic_pointer_cast<Integer>(obj)) return static_cast<double>(i->value);
    if (auto f = std::dynamic_pointer_cast<Float>(obj)) return f->value;
    return 0;
}

static bool isNumber(ObjectPtr obj) {
    return obj && (obj->type() == ObjectType::INTEGER || obj->type() == ObjectType::FLOAT);
}

static ObjectPtr makeFloat(double val) { return newFloat(val); }
static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initMathModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    funcs["sqrt"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_sqrt: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_sqrt: argument must be number");
        double val = getFloat(args[0]);
        if (val < 0) return makeError("math_sqrt: square root of negative number");
        return makeFloat(std::sqrt(val));
    };

    funcs["pow"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("math_pow: expected 2 arguments");
        if (!isNumber(args[0]) || !isNumber(args[1])) return makeError("math_pow: arguments must be numbers");
        return makeFloat(std::pow(getFloat(args[0]), getFloat(args[1])));
    };

    funcs["exp"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_exp: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_exp: argument must be number");
        return makeFloat(std::exp(getFloat(args[0])));
    };

    funcs["log"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_log: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_log: argument must be number");
        double val = getFloat(args[0]);
        if (val <= 0) return makeError("math_log: logarithm of non-positive number");
        return makeFloat(std::log(val));
    };

    funcs["log10"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_log10: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_log10: argument must be number");
        double val = getFloat(args[0]);
        if (val <= 0) return makeError("math_log10: logarithm of non-positive number");
        return makeFloat(std::log10(val));
    };

    funcs["log2"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_log2: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_log2: argument must be number");
        double val = getFloat(args[0]);
        if (val <= 0) return makeError("math_log2: logarithm of non-positive number");
        return makeFloat(std::log2(val));
    };

    funcs["sin"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_sin: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_sin: argument must be number");
        return makeFloat(std::sin(getFloat(args[0])));
    };

    funcs["cos"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_cos: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_cos: argument must be number");
        return makeFloat(std::cos(getFloat(args[0])));
    };

    funcs["tan"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_tan: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_tan: argument must be number");
        return makeFloat(std::tan(getFloat(args[0])));
    };

    funcs["asin"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_asin: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_asin: argument must be number");
        double val = getFloat(args[0]);
        if (val < -1 || val > 1) return makeError("math_asin: argument out of range [-1, 1]");
        return makeFloat(std::asin(val));
    };

    funcs["acos"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_acos: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_acos: argument must be number");
        double val = getFloat(args[0]);
        if (val < -1 || val > 1) return makeError("math_acos: argument out of range [-1, 1]");
        return makeFloat(std::acos(val));
    };

    funcs["atan"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_atan: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_atan: argument must be number");
        return makeFloat(std::atan(getFloat(args[0])));
    };

    funcs["atan2"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("math_atan2: expected 2 arguments");
        if (!isNumber(args[0]) || !isNumber(args[1])) return makeError("math_atan2: arguments must be numbers");
        return makeFloat(std::atan2(getFloat(args[0]), getFloat(args[1])));
    };

    funcs["sinh"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_sinh: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_sinh: argument must be number");
        return makeFloat(std::sinh(getFloat(args[0])));
    };

    funcs["cosh"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_cosh: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_cosh: argument must be number");
        return makeFloat(std::cosh(getFloat(args[0])));
    };

    funcs["tanh"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_tanh: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_tanh: argument must be number");
        return makeFloat(std::tanh(getFloat(args[0])));
    };

    funcs["ceil"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_ceil: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_ceil: argument must be number");
        return makeFloat(std::ceil(getFloat(args[0])));
    };

    funcs["floor"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_floor: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_floor: argument must be number");
        return makeFloat(std::floor(getFloat(args[0])));
    };

    funcs["round"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_round: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_round: argument must be number");
        return makeFloat(std::round(getFloat(args[0])));
    };

    funcs["trunc"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_trunc: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_trunc: argument must be number");
        return makeFloat(std::trunc(getFloat(args[0])));
    };

    funcs["max"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2) return makeError("math_max: expected at least 2 arguments");
        double max = getFloat(args[0]);
        for (size_t i = 1; i < args.size(); i++) {
            if (!isNumber(args[i])) return makeError("math_max: all arguments must be numbers");
            double v = getFloat(args[i]);
            if (v > max) max = v;
        }
        return makeFloat(max);
    };

    funcs["min"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2) return makeError("math_min: expected at least 2 arguments");
        double min = getFloat(args[0]);
        for (size_t i = 1; i < args.size(); i++) {
            if (!isNumber(args[i])) return makeError("math_min: all arguments must be numbers");
            double v = getFloat(args[i]);
            if (v < min) min = v;
        }
        return makeFloat(min);
    };

    funcs["pi"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 0) return makeError("math_pi: expected 0 arguments");
        return makeFloat(M_PI);
    };

    funcs["e"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 0) return makeError("math_e: expected 0 arguments");
        return makeFloat(M_E);
    };

    funcs["abs"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("math_abs: expected 1 argument");
        if (!isNumber(args[0])) return makeError("math_abs: argument must be number");
        return makeFloat(std::abs(getFloat(args[0])));
    };

    funcs["mod"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("math_mod: expected 2 arguments");
        if (!isNumber(args[0]) || !isNumber(args[1])) return makeError("math_mod: arguments must be numbers");
        double y = getFloat(args[1]);
        if (y == 0) return makeError("math_mod: division by zero");
        return makeFloat(std::fmod(getFloat(args[0]), y));
    };

    static std::mt19937 rng(std::random_device{}());
    funcs["random"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 0) return makeError("math_random: expected 0 arguments");
        static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return makeFloat(dist(rng));
    };

    Registry::instance().registerModule("math", funcs);
}

} // namespace darix::native
