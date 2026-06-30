#include "darix/native/native.hpp"
#include <algorithm>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initLinkedListModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // new() -> empty list
    funcs["new"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        return newArray({});
    };

    // from_array(array) -> linked list
    funcs["from_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("from_array: argument must be array");
        return arr;
    };

    // to_array(list) -> array
    funcs["to_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("to_array: argument must be linked list (array)");
        return arr;
    };

    // head(list) -> first element or null
    funcs["head"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("head: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("head: argument must be linked list (array)");
        if (arr->elements.empty()) return getNull();
        return arr->elements.front();
    };

    // tail(list) -> all but first element
    funcs["tail"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("tail: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("tail: argument must be linked list (array)");
        if (arr->elements.size() <= 1) return newArray({});
        return newArray(std::vector<ObjectPtr>(arr->elements.begin() + 1, arr->elements.end()));
    };

    // last(list) -> last element or null
    funcs["last"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("last: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("last: argument must be linked list (array)");
        if (arr->elements.empty()) return getNull();
        return arr->elements.back();
    };

    // init(list) -> all but last element
    funcs["init"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("init: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("init: argument must be linked list (array)");
        if (arr->elements.size() <= 1) return newArray({});
        return newArray(std::vector<ObjectPtr>(arr->elements.begin(), arr->elements.end() - 1));
    };

    // cons(list, elem) -> new list with elem prepended
    funcs["cons"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("cons: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("cons: first argument must be linked list (array)");
        std::vector<ObjectPtr> result;
        result.push_back(args[1]);
        result.insert(result.end(), arr->elements.begin(), arr->elements.end());
        return newArray(result);
    };

    // append(list, elem) -> new list with elem appended
    funcs["append"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("append: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("append: first argument must be linked list (array)");
        std::vector<ObjectPtr> result = arr->elements;
        result.push_back(args[1]);
        return newArray(result);
    };

    // concat(list1, list2) -> concatenated list
    funcs["concat"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("concat: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("concat: both arguments must be linked lists (arrays)");
        std::vector<ObjectPtr> result = a->elements;
        result.insert(result.end(), b->elements.begin(), b->elements.end());
        return newArray(result);
    };

    // length(list) -> int
    funcs["length"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("length: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("length: argument must be linked list (array)");
        return newInteger(static_cast<int64_t>(arr->elements.size()));
    };

    // is_empty(list) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("is_empty: argument must be linked list (array)");
        return newBoolean(arr->elements.empty());
    };

    // nth(list, index) -> element at index or null
    funcs["nth"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("nth: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("nth: first argument must be linked list (array)");
        auto idx = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!idx) return makeError("nth: second argument must be integer");
        int64_t i = idx->value;
        if (i < 0) i = static_cast<int64_t>(arr->elements.size()) + i;
        if (i < 0 || i >= static_cast<int64_t>(arr->elements.size())) return getNull();
        return arr->elements[i];
    };

    // insert_at(list, index, elem) -> new list with elem inserted at index
    funcs["insert_at"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("insert_at: expected 3 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("insert_at: first argument must be linked list (array)");
        auto idx = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!idx) return makeError("insert_at: second argument must be integer");
        int64_t i = idx->value;
        if (i < 0) i = static_cast<int64_t>(arr->elements.size()) + i;
        i = std::max(static_cast<int64_t>(0), std::min(i, static_cast<int64_t>(arr->elements.size())));
        std::vector<ObjectPtr> result = arr->elements;
        result.insert(result.begin() + i, args[2]);
        return newArray(result);
    };

    // remove_at(list, index) -> new list without element at index
    funcs["remove_at"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("remove_at: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("remove_at: first argument must be linked list (array)");
        auto idx = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!idx) return makeError("remove_at: second argument must be integer");
        int64_t i = idx->value;
        if (i < 0) i = static_cast<int64_t>(arr->elements.size()) + i;
        if (i < 0 || i >= static_cast<int64_t>(arr->elements.size())) return arr;
        std::vector<ObjectPtr> result = arr->elements;
        result.erase(result.begin() + i);
        return newArray(result);
    };

    // remove_first(list, elem) -> list with first occurrence removed
    funcs["remove_first"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("remove_first: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("remove_first: first argument must be linked list (array)");
        std::vector<ObjectPtr> result = arr->elements;
        for (auto it = result.begin(); it != result.end(); ++it) {
            if (equals(*it, args[1])) { result.erase(it); break; }
        }
        return newArray(result);
    };

    // remove_all(list, elem) -> list with all occurrences removed
    funcs["remove_all"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("remove_all: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("remove_all: first argument must be linked list (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements)
            if (!equals(e, args[1])) result.push_back(e);
        return newArray(result);
    };

    // contains(list, elem) -> bool
    funcs["contains"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("contains: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("contains: first argument must be linked list (array)");
        for (auto& e : arr->elements)
            if (equals(e, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    // index_of(list, elem) -> int (-1 if not found)
    funcs["index_of"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("index_of: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("index_of: first argument must be linked list (array)");
        for (size_t i = 0; i < arr->elements.size(); i++)
            if (equals(arr->elements[i], args[1])) return newInteger(static_cast<int64_t>(i));
        return newInteger(-1);
    };

    // reverse(list) -> reversed list
    funcs["reverse"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("reverse: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("reverse: argument must be linked list (array)");
        auto result = arr->elements;
        std::reverse(result.begin(), result.end());
        return newArray(result);
    };

    // sort(list) -> sorted list
    funcs["sort"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("sort: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("sort: argument must be linked list (array)");
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

    // unique(list) -> list with duplicates removed
    funcs["unique"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("unique: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("unique: argument must be linked list (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) {
            bool found = false;
            for (auto& r : result) { if (equals(e, r)) { found = true; break; } }
            if (!found) result.push_back(e);
        }
        return newArray(result);
    };

    // take(list, n) -> first n elements
    funcs["take"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("take: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("take: first argument must be linked list (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("take: second argument must be integer");
        size_t count = std::min(static_cast<size_t>(std::max(static_cast<int64_t>(0), n->value)), arr->elements.size());
        return newArray(std::vector<ObjectPtr>(arr->elements.begin(), arr->elements.begin() + count));
    };

    // drop(list, n) -> list without first n elements
    funcs["drop"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("drop: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("drop: first argument must be linked list (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("drop: second argument must be integer");
        size_t skip = std::min(static_cast<size_t>(std::max(static_cast<int64_t>(0), n->value)), arr->elements.size());
        return newArray(std::vector<ObjectPtr>(arr->elements.begin() + skip, arr->elements.end()));
    };

    // slice(list, start, end?) -> sub-list
    funcs["slice"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("slice: expected 2-3 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("slice: first argument must be linked list (array)");
        auto startObj = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!startObj) return makeError("slice: second argument must be integer");
        int64_t start = startObj->value;
        int64_t end = static_cast<int64_t>(arr->elements.size());
        if (args.size() == 3) {
            auto endObj = std::dynamic_pointer_cast<Integer>(args[2]);
            if (endObj) end = endObj->value;
        }
        if (start < 0) start = std::max(static_cast<int64_t>(0), static_cast<int64_t>(arr->elements.size()) + start);
        if (end < 0) end = std::max(static_cast<int64_t>(0), static_cast<int64_t>(arr->elements.size()) + end);
        start = std::min(start, static_cast<int64_t>(arr->elements.size()));
        end = std::min(end, static_cast<int64_t>(arr->elements.size()));
        if (start >= end) return newArray({});
        return newArray(std::vector<ObjectPtr>(arr->elements.begin() + start, arr->elements.begin() + end));
    };

    // zip(list1, list2) -> list of pairs
    funcs["zip"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("zip: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("zip: both arguments must be linked lists (arrays)");
        std::vector<ObjectPtr> result;
        size_t len = std::min(a->elements.size(), b->elements.size());
        for (size_t i = 0; i < len; i++)
            result.push_back(newArray({a->elements[i], b->elements[i]}));
        return newArray(result);
    };

    // flatten(list) -> flattened list (one level)
    funcs["flatten"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("flatten: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("flatten: argument must be linked list (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) {
            if (auto inner = std::dynamic_pointer_cast<Array>(e))
                for (auto& ie : inner->elements) result.push_back(ie);
            else
                result.push_back(e);
        }
        return newArray(result);
    };

    // enumerate(list) -> list of [index, value] pairs
    funcs["enumerate"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("enumerate: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("enumerate: argument must be linked list (array)");
        std::vector<ObjectPtr> result;
        for (size_t i = 0; i < arr->elements.size(); i++)
            result.push_back(newArray({newInteger(static_cast<int64_t>(i)), arr->elements[i]}));
        return newArray(result);
    };

    // min(list) -> smallest element
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

    // max(list) -> largest element
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

    // fold(list, reducer, initial) -> value
    funcs["fold"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("fold: expected 3 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("fold: first argument must be linked list (array)");
        ObjectPtr fn = args[1];
        ObjectPtr acc = args[2];
        for (auto& e : arr->elements) acc = callCallable(fn, {acc, e});
        return acc;
    };

    // map_list(list, fn) -> transformed list
    funcs["map_list"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_list: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("map_list: first argument must be linked list (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        result.reserve(arr->elements.size());
        for (auto& e : arr->elements) result.push_back(callCallable(fn, {e}));
        return newArray(result);
    };

    // filter_list(list, predicate) -> filtered list
    funcs["filter_list"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter_list: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("filter_list: first argument must be linked list (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements)
            if (isTruthy(callCallable(fn, {e}))) result.push_back(e);
        return newArray(result);
    };

    // partition(list, predicate) -> [matching, non-matching]
    funcs["partition"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("partition: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("partition: first argument must be linked list (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> yes, no;
        for (auto& e : arr->elements) {
            if (isTruthy(callCallable(fn, {e}))) yes.push_back(e);
            else no.push_back(e);
        }
        return newArray({newArray(yes), newArray(no)});
    };

    // group_by(list, key_fn) -> map of key -> list
    funcs["group_by"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("group_by: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("group_by: first argument must be linked list (array)");
        ObjectPtr fn = args[1];
        std::vector<std::pair<ObjectPtr, std::vector<ObjectPtr>>> groups;
        for (auto& e : arr->elements) {
            ObjectPtr key = callCallable(fn, {e});
            bool found = false;
            for (auto& g : groups) {
                if (equals(g.first, key)) { g.second.push_back(e); found = true; break; }
            }
            if (!found) groups.push_back({key, {e}});
        }
        auto result = std::make_shared<Map>();
        for (auto& g : groups) result->pairs.push_back({g.first, newArray(g.second)});
        return result;
    };

    // equals(list1, list2) -> bool
    funcs["equals"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("equals: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("equals: both arguments must be linked lists (arrays)");
        if (a->elements.size() != b->elements.size()) return newBoolean(false);
        for (size_t i = 0; i < a->elements.size(); i++)
            if (!equals(a->elements[i], b->elements[i])) return newBoolean(false);
        return newBoolean(true);
    };

    // to_string(list) -> comma-separated string
    funcs["to_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_string: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("to_string: argument must be linked list (array)");
        return newString(arr->inspect());
    };

    // clear(list) -> empty list
    funcs["clear"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("clear: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("clear: argument must be linked list (array)");
        return newArray({});
    };

    Registry::instance().registerModule("linkedlist", funcs);
}

} // namespace darix::native
