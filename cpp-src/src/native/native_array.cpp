#include "darix/native/native.hpp"
#include <algorithm>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initArrayModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    funcs["filter"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("filter: first argument must be array");
        ObjectPtr fn = args[1];

        std::vector<ObjectPtr> result;
        for (auto& elem : arr->elements) {
            if (isTruthy(callCallable(fn, {elem}))) {
                result.push_back(elem);
            }
        }
        return newArray(result);
    };

    funcs["map"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("map: first argument must be array");
        ObjectPtr fn = args[1];

        std::vector<ObjectPtr> result;
        result.reserve(arr->elements.size());
        for (auto& elem : arr->elements) {
            result.push_back(callCallable(fn, {elem}));
        }
        return newArray(result);
    };

    funcs["reduce"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("reduce: expected 3 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("reduce: first argument must be array");
        ObjectPtr fn = args[1];
        ObjectPtr acc = args[2];

        for (auto& elem : arr->elements) {
            acc = callCallable(fn, {acc, elem});
        }
        return acc;
    };

    funcs["find"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("find: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("find: first argument must be array");
        ObjectPtr fn = args[1];

        for (auto& elem : arr->elements) {
            if (isTruthy(callCallable(fn, {elem}))) return elem;
        }
        return getNull();
    };

    funcs["find_all"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("find_all: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("find_all: first argument must be array");
        ObjectPtr fn = args[1];

        std::vector<ObjectPtr> result;
        for (auto& elem : arr->elements) {
            if (isTruthy(callCallable(fn, {elem}))) result.push_back(elem);
        }
        return newArray(result);
    };

    funcs["unique"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("unique: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("unique: argument must be array");

        std::vector<ObjectPtr> result;
        for (auto& elem : arr->elements) {
            bool found = false;
            for (auto& r : result) {
                if (equals(elem, r)) { found = true; break; }
            }
            if (!found) result.push_back(elem);
        }
        return newArray(result);
    };

    funcs["flatten"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("flatten: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("flatten: argument must be array");

        std::vector<ObjectPtr> result;
        for (auto& elem : arr->elements) {
            if (auto inner = std::dynamic_pointer_cast<Array>(elem)) {
                for (auto& e : inner->elements) result.push_back(e);
            } else {
                result.push_back(elem);
            }
        }
        return newArray(result);
    };

    funcs["chunk"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("chunk: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("chunk: first argument must be array");
        auto sz = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!sz || sz->value <= 0) return makeError("chunk: size must be positive integer");

        std::vector<ObjectPtr> result;
        for (size_t i = 0; i < arr->elements.size(); i += sz->value) {
            std::vector<ObjectPtr> chunk;
            for (size_t j = i; j < std::min(i + static_cast<size_t>(sz->value), arr->elements.size()); j++)
                chunk.push_back(arr->elements[j]);
            result.push_back(newArray(chunk));
        }
        return newArray(result);
    };

    funcs["zip"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("zip: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("zip: both arguments must be arrays");

        std::vector<ObjectPtr> result;
        size_t len = std::min(a->elements.size(), b->elements.size());
        for (size_t i = 0; i < len; i++)
            result.push_back(newArray({a->elements[i], b->elements[i]}));
        return newArray(result);
    };

    funcs["unzip"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("unzip: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("unzip: argument must be array of pairs");

        std::vector<ObjectPtr> first, second;
        for (auto& elem : arr->elements) {
            auto pair = std::dynamic_pointer_cast<Array>(elem);
            if (pair && pair->elements.size() >= 2) {
                first.push_back(pair->elements[0]);
                second.push_back(pair->elements[1]);
            }
        }
        return newArray({newArray(first), newArray(second)});
    };

    funcs["group_by"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("group_by: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("group_by: first argument must be array");
        ObjectPtr fn = args[1];

        std::vector<std::pair<ObjectPtr, std::vector<ObjectPtr>>> groups;
        for (auto& elem : arr->elements) {
            ObjectPtr key = callCallable(fn, {elem});
            bool found = false;
            for (auto& g : groups) {
                if (equals(g.first, key)) { g.second.push_back(elem); found = true; break; }
            }
            if (!found) groups.push_back({key, {elem}});
        }
        auto result = std::make_shared<Map>();
        for (auto& g : groups) result->pairs.push_back({g.first, newArray(g.second)});
        return result;
    };

    funcs["sort_by"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("sort_by: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("sort_by: first argument must be array");
        ObjectPtr fn = args[1];

        auto sorted = arr->elements;
        std::sort(sorted.begin(), sorted.end(), [fn](const ObjectPtr& a, const ObjectPtr& b) {
            auto ka = callCallable(fn, {a});
            auto kb = callCallable(fn, {b});
            if (auto ai = std::dynamic_pointer_cast<Integer>(ka))
                if (auto bi = std::dynamic_pointer_cast<Integer>(kb)) return ai->value < bi->value;
            if (auto af = std::dynamic_pointer_cast<Float>(ka))
                if (auto bf = std::dynamic_pointer_cast<Float>(kb)) return af->value < bf->value;
            if (auto as = std::dynamic_pointer_cast<String>(ka))
                if (auto bs = std::dynamic_pointer_cast<String>(kb)) return as->value < bs->value;
            return false;
        });
        return newArray(sorted);
    };

    funcs["partition"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("partition: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("partition: first argument must be array");
        ObjectPtr fn = args[1];

        std::vector<ObjectPtr> truthy, falsy;
        for (auto& elem : arr->elements) {
            if (isTruthy(callCallable(fn, {elem}))) truthy.push_back(elem);
            else falsy.push_back(elem);
        }
        return newArray({newArray(truthy), newArray(falsy)});
    };

    funcs["diff"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("diff: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("diff: both arguments must be arrays");

        std::vector<ObjectPtr> result;
        for (auto& elem : a->elements) {
            bool found = false;
            for (auto& be : b->elements) { if (equals(elem, be)) { found = true; break; } }
            if (!found) result.push_back(elem);
        }
        return newArray(result);
    };

    funcs["intersect"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("intersect: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("intersect: both arguments must be arrays");

        std::vector<ObjectPtr> result;
        for (auto& elem : a->elements)
            for (auto& be : b->elements)
                if (equals(elem, be)) { result.push_back(elem); break; }
        return newArray(result);
    };

    funcs["union"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("union: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("union: both arguments must be arrays");

        std::vector<ObjectPtr> result = a->elements;
        for (auto& elem : b->elements) {
            bool found = false;
            for (auto& r : result) { if (equals(elem, r)) { found = true; break; } }
            if (!found) result.push_back(elem);
        }
        return newArray(result);
    };

    funcs["each"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("each: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("each: first argument must be array");
        ObjectPtr fn = args[1];

        for (auto& elem : arr->elements) callCallable(fn, {elem});
        return getNull();
    };

    funcs["contains_value"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("contains_value: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("contains_value: first argument must be array");
        for (auto& elem : arr->elements)
            if (equals(elem, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    funcs["index_of"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("index_of: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("index_of: first argument must be array");
        for (size_t i = 0; i < arr->elements.size(); i++)
            if (equals(arr->elements[i], args[1])) return newInteger(static_cast<int64_t>(i));
        return newInteger(-1);
    };

    funcs["last"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("last: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return getNull();
        return arr->elements.back();
    };

    funcs["first"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("first: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return getNull();
        return arr->elements.front();
    };

    funcs["take"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("take: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("take: first argument must be array");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("take: second argument must be integer");
        size_t count = std::min(static_cast<size_t>(std::max(0LL, n->value)), arr->elements.size());
        return newArray(std::vector<ObjectPtr>(arr->elements.begin(), arr->elements.begin() + count));
    };

    funcs["drop"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("drop: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("drop: first argument must be array");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("drop: second argument must be integer");
        size_t skip = std::min(static_cast<size_t>(std::max(0LL, n->value)), arr->elements.size());
        return newArray(std::vector<ObjectPtr>(arr->elements.begin() + skip, arr->elements.end()));
    };

    funcs["min_by"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("min_by: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return getNull();
        ObjectPtr fn = args[1];

        ObjectPtr minElem = arr->elements[0];
        ObjectPtr minKey = callCallable(fn, {minElem});
        for (size_t i = 1; i < arr->elements.size(); i++) {
            ObjectPtr key = callCallable(fn, {arr->elements[i]});
            if (auto ai = std::dynamic_pointer_cast<Integer>(minKey))
                if (auto bi = std::dynamic_pointer_cast<Integer>(key))
                    if (bi->value < ai->value) { minElem = arr->elements[i]; minKey = key; }
            if (auto af = std::dynamic_pointer_cast<Float>(minKey))
                if (auto bf = std::dynamic_pointer_cast<Float>(key))
                    if (bf->value < af->value) { minElem = arr->elements[i]; minKey = key; }
        }
        return minElem;
    };

    funcs["max_by"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("max_by: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return getNull();
        ObjectPtr fn = args[1];

        ObjectPtr maxElem = arr->elements[0];
        ObjectPtr maxKey = callCallable(fn, {maxElem});
        for (size_t i = 1; i < arr->elements.size(); i++) {
            ObjectPtr key = callCallable(fn, {arr->elements[i]});
            if (auto ai = std::dynamic_pointer_cast<Integer>(maxKey))
                if (auto bi = std::dynamic_pointer_cast<Integer>(key))
                    if (bi->value > ai->value) { maxElem = arr->elements[i]; maxKey = key; }
            if (auto af = std::dynamic_pointer_cast<Float>(maxKey))
                if (auto bf = std::dynamic_pointer_cast<Float>(key))
                    if (bf->value > af->value) { maxElem = arr->elements[i]; maxKey = key; }
        }
        return maxElem;
    };

    funcs["all"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("all: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("all: first argument must be array");
        ObjectPtr fn = args[1];

        for (auto& elem : arr->elements)
            if (!isTruthy(callCallable(fn, {elem}))) return newBoolean(false);
        return newBoolean(true);
    };

    funcs["any"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("any: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("any: first argument must be array");
        ObjectPtr fn = args[1];

        for (auto& elem : arr->elements)
            if (isTruthy(callCallable(fn, {elem}))) return newBoolean(true);
        return newBoolean(false);
    };

    funcs["enumerate"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("enumerate: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("enumerate: argument must be array");

        std::vector<ObjectPtr> result;
        for (size_t i = 0; i < arr->elements.size(); i++)
            result.push_back(newArray({newInteger(static_cast<int64_t>(i)), arr->elements[i]}));
        return newArray(result);
    };

    Registry::instance().registerModule("array", funcs);
}

} // namespace darix::native
