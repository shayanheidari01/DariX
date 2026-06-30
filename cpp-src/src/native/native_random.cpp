#include "darix/native/native.hpp"
#include <random>
#include <algorithm>
#include <ctime>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

static int64_t getInt(ObjectPtr obj) {
    if (auto i = std::dynamic_pointer_cast<Integer>(obj)) return i->value;
    if (auto f = std::dynamic_pointer_cast<Float>(obj)) return static_cast<int64_t>(f->value);
    return 0;
}

static std::mt19937_64& getRng() {
    static std::mt19937_64 rng(static_cast<uint64_t>(std::time(nullptr)));
    return rng;
}

void initRandomModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // seed(n) -> null
    funcs["seed"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("seed: expected 1 argument");
        getRng().seed(static_cast<uint64_t>(getInt(args[0])));
        return getNull();
    };

    // int(max?) -> random integer
    funcs["int"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() > 1) return makeError("int: expected 0-1 arguments");
        auto& rng = getRng();
        if (args.empty()) {
            std::uniform_int_distribution<int64_t> dist;
            return newInteger(dist(rng));
        }
        int64_t max = getInt(args[0]);
        if (max <= 0) return makeError("int: max must be positive");
        std::uniform_int_distribution<int64_t> dist(0, max - 1);
        return newInteger(dist(rng));
    };

    // int_range(min, max) -> random integer in [min, max)
    funcs["int_range"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("int_range: expected 2 arguments");
        int64_t min = getInt(args[0]), max = getInt(args[1]);
        if (min >= max) return makeError("int_range: min must be less than max");
        auto& rng = getRng();
        std::uniform_int_distribution<int64_t> dist(min, max - 1);
        return newInteger(dist(rng));
    };

    // float() -> random float in [0, 1)
    funcs["float"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (!args.empty()) return makeError("float: expected 0 arguments");
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return newFloat(dist(getRng()));
    };

    // float_range(min, max) -> random float in [min, max)
    funcs["float_range"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("float_range: expected 2 arguments");
        double min = (args[0]->type() == ObjectType::FLOAT) ?
            std::dynamic_pointer_cast<Float>(args[0])->value : static_cast<double>(getInt(args[0]));
        double max = (args[1]->type() == ObjectType::FLOAT) ?
            std::dynamic_pointer_cast<Float>(args[1])->value : static_cast<double>(getInt(args[1]));
        if (min >= max) return makeError("float_range: min must be less than max");
        std::uniform_real_distribution<double> dist(min, max);
        return newFloat(dist(getRng()));
    };

    // choice(array) -> random element
    funcs["choice"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("choice: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return makeError("choice: array must not be empty");
        std::uniform_int_distribution<size_t> dist(0, arr->elements.size() - 1);
        return arr->elements[dist(getRng())];
    };

    // choices(array, count) -> array of random elements (with replacement)
    funcs["choices"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("choices: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return makeError("choices: array must not be empty");
        int64_t count = getInt(args[1]);
        if (count <= 0) return makeError("choices: count must be positive");
        auto& rng = getRng();
        std::uniform_int_distribution<size_t> dist(0, arr->elements.size() - 1);
        std::vector<ObjectPtr> result;
        result.reserve(count);
        for (int64_t i = 0; i < count; i++)
            result.push_back(arr->elements[dist(rng)]);
        return newArray(result);
    };

    // sample(array, count) -> array of random elements (without replacement)
    funcs["sample"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("sample: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("sample: first argument must be array");
        int64_t count = getInt(args[1]);
        if (count < 0) return makeError("sample: count must be non-negative");
        count = std::min(count, static_cast<int64_t>(arr->elements.size()));
        std::vector<ObjectPtr> result = arr->elements;
        std::shuffle(result.begin(), result.end(), getRng());
        return newArray(std::vector<ObjectPtr>(result.begin(), result.begin() + count));
    };

    // shuffle(array) -> shuffled array
    funcs["shuffle"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("shuffle: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("shuffle: argument must be array");
        auto result = arr->elements;
        std::shuffle(result.begin(), result.end(), getRng());
        return newArray(result);
    };

    // coin() -> true or false
    funcs["coin"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        std::uniform_int_distribution<int> dist(0, 1);
        return newBoolean(dist(getRng()) == 1);
    };

    // weighted_choice(options, weights) -> chosen option
    funcs["weighted_choice"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("weighted_choice: expected 2 arguments");
        auto opts = std::dynamic_pointer_cast<Array>(args[0]);
        auto weights = std::dynamic_pointer_cast<Array>(args[1]);
        if (!opts || !weights) return makeError("weighted_choice: both arguments must be arrays");
        if (opts->elements.size() != weights->elements.size())
            return makeError("weighted_choice: options and weights must have same length");
        if (opts->elements.empty()) return makeError("weighted_choice: arrays must not be empty");

        double totalWeight = 0;
        for (auto& w : weights->elements) {
            if (auto wi = std::dynamic_pointer_cast<Integer>(w)) totalWeight += wi->value;
            else if (auto wf = std::dynamic_pointer_cast<Float>(w)) totalWeight += wf->value;
        }
        if (totalWeight <= 0) return makeError("weighted_choice: total weight must be positive");

        std::uniform_real_distribution<double> dist(0.0, totalWeight);
        double r = dist(getRng());
        double cumulative = 0;
        for (size_t i = 0; i < opts->elements.size(); i++) {
            double w = 0;
            if (auto wi = std::dynamic_pointer_cast<Integer>(weights->elements[i])) w = wi->value;
            else if (auto wf = std::dynamic_pointer_cast<Float>(weights->elements[i])) w = wf->value;
            cumulative += w;
            if (r < cumulative) return opts->elements[i];
        }
        return opts->elements.back();
    };

    // booleans(count, true_prob?) -> array of booleans
    funcs["booleans"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 1 || args.size() > 2) return makeError("booleans: expected 1-2 arguments");
        int64_t count = getInt(args[0]);
        if (count <= 0) return makeError("booleans: count must be positive");
        double prob = 0.5;
        if (args.size() == 2) {
            if (auto f = std::dynamic_pointer_cast<Float>(args[1])) prob = f->value;
            else if (auto i = std::dynamic_pointer_cast<Integer>(args[1])) prob = i->value;
        }
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        std::vector<ObjectPtr> result;
        result.reserve(count);
        for (int64_t i = 0; i < count; i++)
            result.push_back(newBoolean(dist(getRng()) < prob));
        return newArray(result);
    };

    // ints(count, min, max) -> array of random integers
    funcs["ints"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("ints: expected 3 arguments");
        int64_t count = getInt(args[0]);
        int64_t min = getInt(args[1]);
        int64_t max = getInt(args[2]);
        if (count <= 0) return makeError("ints: count must be positive");
        if (min >= max) return makeError("ints: min must be less than max");
        auto& rng = getRng();
        std::uniform_int_distribution<int64_t> dist(min, max - 1);
        std::vector<ObjectPtr> result;
        result.reserve(count);
        for (int64_t i = 0; i < count; i++)
            result.push_back(newInteger(dist(rng)));
        return newArray(result);
    };

    // normal(mean, stddev) -> random number from normal distribution
    funcs["normal"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("normal: expected 2 arguments");
        double mean = (args[0]->type() == ObjectType::FLOAT) ?
            std::dynamic_pointer_cast<Float>(args[0])->value : static_cast<double>(getInt(args[0]));
        double stddev = (args[1]->type() == ObjectType::FLOAT) ?
            std::dynamic_pointer_cast<Float>(args[1])->value : static_cast<double>(getInt(args[1]));
        std::normal_distribution<double> dist(mean, stddev);
        return newFloat(dist(getRng()));
    };

    // exponential(lambda) -> random number from exponential distribution
    funcs["exponential"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("exponential: expected 1 argument");
        double lambda = (args[0]->type() == ObjectType::FLOAT) ?
            std::dynamic_pointer_cast<Float>(args[0])->value : static_cast<double>(getInt(args[0]));
        if (lambda <= 0) return makeError("exponential: lambda must be positive");
        std::exponential_distribution<double> dist(lambda);
        return newFloat(dist(getRng()));
    };

    Registry::instance().registerModule("random", funcs);
}

} // namespace darix::native
