#include "darix/native/native.hpp"
#include <algorithm>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initMapModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // keys(map) -> array of keys
    funcs["keys"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("keys: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("keys: argument must be map");
        std::vector<ObjectPtr> result;
        for (auto& [k, v] : m->pairs) result.push_back(k);
        return newArray(result);
    };

    // values(map) -> array of values
    funcs["values"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("values: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("values: argument must be map");
        std::vector<ObjectPtr> result;
        for (auto& [k, v] : m->pairs) result.push_back(v);
        return newArray(result);
    };

    // items(map) -> array of [key, value] pairs
    funcs["items"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("items: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("items: argument must be map");
        std::vector<ObjectPtr> result;
        for (auto& [k, v] : m->pairs) result.push_back(newArray({k, v}));
        return newArray(result);
    };

    // has_key(map, key) -> bool
    funcs["has_key"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("has_key: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("has_key: first argument must be map");
        for (auto& [k, v] : m->pairs)
            if (equals(k, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    // has_value(map, value) -> bool
    funcs["has_value"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("has_value: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("has_value: first argument must be map");
        for (auto& [k, v] : m->pairs)
            if (equals(v, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    // get(map, key, default?) -> value or default
    funcs["get"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("get: expected 2-3 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("get: first argument must be map");
        for (auto& [k, v] : m->pairs)
            if (equals(k, args[1])) return v;
        if (args.size() == 3) return args[2];
        return getNull();
    };

    // put(map, key, value) -> map (mutates and returns)
    funcs["put"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("put: expected 3 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("put: first argument must be map");
        for (auto& [k, v] : m->pairs) {
            if (equals(k, args[1])) { v = args[2]; return m; }
        }
        m->pairs.push_back({args[1], args[2]});
        return m;
    };

    // remove(map, key) -> map (mutates and returns)
    funcs["remove"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("remove: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("remove: first argument must be map");
        for (auto it = m->pairs.begin(); it != m->pairs.end(); ++it) {
            if (equals(it->first, args[1])) { m->pairs.erase(it); return m; }
        }
        return m;
    };

    // merge(map1, map2) -> new map with both
    funcs["merge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("merge: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Map>(args[0]);
        auto b = std::dynamic_pointer_cast<Map>(args[1]);
        if (!a || !b) return makeError("merge: both arguments must be maps");

        auto result = std::make_shared<Map>();
        for (auto& [k, v] : a->pairs) result->pairs.push_back({k, v});
        for (auto& [k, v] : b->pairs) {
            bool found = false;
            for (auto& rk : result->pairs) {
                if (equals(rk.first, k)) { rk.second = v; found = true; break; }
            }
            if (!found) result->pairs.push_back({k, v});
        }
        return result;
    };

    // size(map) -> int
    funcs["size"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("size: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("size: argument must be map");
        return newInteger(static_cast<int64_t>(m->pairs.size()));
    };

    // is_empty(map) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("is_empty: argument must be map");
        return newBoolean(m->pairs.empty());
    };

    // clear(map) -> map (empties and returns)
    funcs["clear"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("clear: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("clear: argument must be map");
        m->pairs.clear();
        return m;
    };

    // map_keys(map, fn) -> map with transformed keys
    funcs["map_keys"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_keys: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("map_keys: first argument must be map");
        ObjectPtr fn = args[1];

        auto result = std::make_shared<Map>();
        for (auto& [k, v] : m->pairs) {
            result->pairs.push_back({callCallable(fn, {k}), v});
        }
        return result;
    };

    // map_values(map, fn) -> map with transformed values
    funcs["map_values"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_values: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("map_values: first argument must be map");
        ObjectPtr fn = args[1];

        auto result = std::make_shared<Map>();
        for (auto& [k, v] : m->pairs) {
            result->pairs.push_back({k, callCallable(fn, {v})});
        }
        return result;
    };

    // filter(map, predicate) -> map with entries matching predicate
    funcs["filter"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("filter: first argument must be map");
        ObjectPtr fn = args[1];

        auto result = std::make_shared<Map>();
        for (auto& [k, v] : m->pairs) {
            if (isTruthy(callCallable(fn, {k, v}))) {
                result->pairs.push_back({k, v});
            }
        }
        return result;
    };

    // find_key(map, predicate) -> key or null
    funcs["find_key"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("find_key: expected 2 arguments");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("find_key: first argument must be map");
        ObjectPtr fn = args[1];

        for (auto& [k, v] : m->pairs)
            if (isTruthy(callCallable(fn, {k, v}))) return k;
        return getNull();
    };

    // to_pairs(map) -> array of [key, value] (alias for items)
    funcs["to_pairs"] = funcs["items"];

    // from_pairs(array) -> map from array of [key, value] pairs
    funcs["from_pairs"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_pairs: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("from_pairs: argument must be array of pairs");

        auto result = std::make_shared<Map>();
        for (auto& elem : arr->elements) {
            auto pair = std::dynamic_pointer_cast<Array>(elem);
            if (pair && pair->elements.size() >= 2) {
                result->pairs.push_back({pair->elements[0], pair->elements[1]});
            }
        }
        return result;
    };

    // invert(map) -> map with keys and values swapped
    funcs["invert"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("invert: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("invert: argument must be map");

        auto result = std::make_shared<Map>();
        for (auto& [k, v] : m->pairs) {
            result->pairs.push_back({v, k});
        }
        return result;
    };

    // equals(map1, map2) -> bool
    funcs["equals"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("equals: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Map>(args[0]);
        auto b = std::dynamic_pointer_cast<Map>(args[1]);
        if (!a || !b) return makeError("equals: both arguments must be maps");
        if (a->pairs.size() != b->pairs.size()) return newBoolean(false);

        for (auto& [k, v] : a->pairs) {
            bool found = false;
            for (auto& [bk, bv] : b->pairs) {
                if (equals(k, bk) && equals(v, bv)) { found = true; break; }
            }
            if (!found) return newBoolean(false);
        }
        return newBoolean(true);
    };

    // keys_array(map) -> sorted array of string keys
    funcs["keys_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("keys_array: expected 1 argument");
        auto m = std::dynamic_pointer_cast<Map>(args[0]);
        if (!m) return makeError("keys_array: argument must be map");

        std::vector<std::string> strs;
        std::vector<ObjectPtr> objs;
        for (auto& [k, v] : m->pairs) {
            strs.push_back(k->inspect());
            objs.push_back(k);
        }
        // Sort by string representation
        std::vector<size_t> indices(strs.size());
        for (size_t i = 0; i < indices.size(); i++) indices[i] = i;
        std::sort(indices.begin(), indices.end(), [&strs](size_t a, size_t b) { return strs[a] < strs[b]; });

        std::vector<ObjectPtr> result;
        for (size_t idx : indices) result.push_back(objs[idx]);
        return newArray(result);
    };

    Registry::instance().registerModule("map", funcs);
}

} // namespace darix::native
