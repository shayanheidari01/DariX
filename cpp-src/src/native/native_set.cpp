#include "darix/native/native.hpp"
#include <algorithm>
#include <set>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

// Helper: check if element exists in array
static bool contains(const std::vector<ObjectPtr>& arr, const ObjectPtr& elem) {
    for (auto& e : arr) if (equals(e, elem)) return true;
    return false;
}

// Helper: add unique element
static void addUnique(std::vector<ObjectPtr>& arr, const ObjectPtr& elem) {
    if (!contains(arr, elem)) arr.push_back(elem);
}

void initSetModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // from_array(array) -> set (deduplicated array)
    funcs["from_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("from_array: argument must be array");
        std::vector<ObjectPtr> result;
        for (auto& elem : arr->elements) addUnique(result, elem);
        return newArray(result);
    };

    // to_array(set) -> array
    funcs["to_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("to_array: argument must be set (array)");
        return arr;
    };

    // size(set) -> int
    funcs["size"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("size: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("size: argument must be set (array)");
        return newInteger(static_cast<int64_t>(arr->elements.size()));
    };

    // is_empty(set) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("is_empty: argument must be set (array)");
        return newBoolean(arr->elements.empty());
    };

    // contains(set, elem) -> bool
    funcs["contains"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("contains: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("contains: first argument must be set (array)");
        return newBoolean(contains(arr->elements, args[1]));
    };

    // add(set, elem) -> new set with elem added
    funcs["add"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("add: first argument must be set (array)");
        std::vector<ObjectPtr> result = arr->elements;
        addUnique(result, args[1]);
        return newArray(result);
    };

    // remove(set, elem) -> new set without elem
    funcs["remove"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("remove: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("remove: first argument must be set (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements)
            if (!equals(e, args[1])) result.push_back(e);
        return newArray(result);
    };

    // union(set1, set2) -> elements in either
    funcs["union"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("union: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("union: both arguments must be sets (arrays)");
        std::vector<ObjectPtr> result = a->elements;
        for (auto& e : b->elements) addUnique(result, e);
        return newArray(result);
    };

    // intersection(set1, set2) -> elements in both
    funcs["intersection"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("intersection: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("intersection: both arguments must be sets (arrays)");
        std::vector<ObjectPtr> result;
        for (auto& e : a->elements)
            if (contains(b->elements, e)) result.push_back(e);
        return newArray(result);
    };

    // difference(set1, set2) -> elements in set1 not in set2
    funcs["difference"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("difference: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("difference: both arguments must be sets (arrays)");
        std::vector<ObjectPtr> result;
        for (auto& e : a->elements)
            if (!contains(b->elements, e)) result.push_back(e);
        return newArray(result);
    };

    // symmetric_difference(set1, set2) -> elements in either but not both
    funcs["symmetric_difference"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("symmetric_difference: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("symmetric_difference: both arguments must be sets (arrays)");
        std::vector<ObjectPtr> result;
        for (auto& e : a->elements)
            if (!contains(b->elements, e)) result.push_back(e);
        for (auto& e : b->elements)
            if (!contains(a->elements, e)) addUnique(result, e);
        return newArray(result);
    };

    // is_subset(set1, set2) -> bool
    funcs["is_subset"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("is_subset: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("is_subset: both arguments must be sets (arrays)");
        for (auto& e : a->elements)
            if (!contains(b->elements, e)) return newBoolean(false);
        return newBoolean(true);
    };

    // is_superset(set1, set2) -> bool (set1 contains all of set2)
    funcs["is_superset"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("is_superset: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("is_superset: both arguments must be sets (arrays)");
        for (auto& e : b->elements)
            if (!contains(a->elements, e)) return newBoolean(false);
        return newBoolean(true);
    };

    // is_disjoint(set1, set2) -> bool (no common elements)
    funcs["is_disjoint"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("is_disjoint: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("is_disjoint: both arguments must be sets (arrays)");
        for (auto& e : a->elements)
            if (contains(b->elements, e)) return newBoolean(false);
        return newBoolean(true);
    };

    // power(set) -> set of all subsets
    funcs["power"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("power: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("power: argument must be set (array)");
        size_t n = arr->elements.size();
        std::vector<ObjectPtr> subsets;
        for (size_t mask = 0; mask < (1ULL << n); mask++) {
            std::vector<ObjectPtr> subset;
            for (size_t i = 0; i < n; i++)
                if (mask & (1ULL << i)) subset.push_back(arr->elements[i]);
            subsets.push_back(newArray(subset));
        }
        return newArray(subsets);
    };

    // cartesian(set1, set2) -> array of all pairs
    funcs["cartesian"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("cartesian: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("cartesian: both arguments must be sets (arrays)");
        std::vector<ObjectPtr> result;
        for (auto& ea : a->elements)
            for (auto& eb : b->elements)
                result.push_back(newArray({ea, eb}));
        return newArray(result);
    };

    // fold(set, reducer, initial) -> value
    funcs["fold"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("fold: expected 3 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("fold: first argument must be set (array)");
        ObjectPtr fn = args[1];
        ObjectPtr acc = args[2];
        for (auto& e : arr->elements) acc = callCallable(fn, {acc, e});
        return acc;
    };

    // map_set(set, fn) -> new set with transformed elements
    funcs["map_set"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_set: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("map_set: first argument must be set (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) addUnique(result, callCallable(fn, {e}));
        return newArray(result);
    };

    // filter_set(set, predicate) -> filtered set
    funcs["filter_set"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter_set: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("filter_set: first argument must be set (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements)
            if (isTruthy(callCallable(fn, {e}))) result.push_back(e);
        return newArray(result);
    };

    // min(set) -> smallest element
    funcs["min"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("min: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return getNull();
        ObjectPtr minElem = arr->elements[0];
        for (size_t i = 1; i < arr->elements.size(); i++) {
            if (auto ai = std::dynamic_pointer_cast<Integer>(minElem))
                if (auto bi = std::dynamic_pointer_cast<Integer>(arr->elements[i]))
                    if (bi->value < ai->value) minElem = arr->elements[i];
            if (auto af = std::dynamic_pointer_cast<Float>(minElem))
                if (auto bf = std::dynamic_pointer_cast<Float>(arr->elements[i]))
                    if (bf->value < af->value) minElem = arr->elements[i];
        }
        return minElem;
    };

    // max(set) -> largest element
    funcs["max"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("max: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr || arr->elements.empty()) return getNull();
        ObjectPtr maxElem = arr->elements[0];
        for (size_t i = 1; i < arr->elements.size(); i++) {
            if (auto ai = std::dynamic_pointer_cast<Integer>(maxElem))
                if (auto bi = std::dynamic_pointer_cast<Integer>(arr->elements[i]))
                    if (bi->value > ai->value) maxElem = arr->elements[i];
            if (auto af = std::dynamic_pointer_cast<Float>(maxElem))
                if (auto bf = std::dynamic_pointer_cast<Float>(arr->elements[i]))
                    if (bf->value > af->value) maxElem = arr->elements[i];
        }
        return maxElem;
    };

    // equals(set1, set2) -> bool
    funcs["equals"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("equals: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("equals: both arguments must be sets (arrays)");
        if (a->elements.size() != b->elements.size()) return newBoolean(false);
        for (auto& e : a->elements)
            if (!contains(b->elements, e)) return newBoolean(false);
        return newBoolean(true);
    };

    // sorted(set) -> sorted array
    funcs["sorted"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sorted: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("sorted: argument must be set (array)");
        auto result = arr->elements;
        std::sort(result.begin(), result.end(), [](const ObjectPtr& a, const ObjectPtr& b) {
            if (auto ai = std::dynamic_pointer_cast<Integer>(a))
                if (auto bi = std::dynamic_pointer_cast<Integer>(b)) return ai->value < bi->value;
            if (auto af = std::dynamic_pointer_cast<Float>(a))
                if (auto bf = std::dynamic_pointer_cast<Float>(b)) return af->value < bf->value;
            if (auto as = std::dynamic_pointer_cast<String>(a))
                if (auto bs = std::dynamic_pointer_cast<String>(b)) return as->value < bs->value;
            return false;
        });
        return newArray(result);
    };

    Registry::instance().registerModule("set", funcs);
}

} // namespace darix::native
