#include "darix/native/native.hpp"
#include <algorithm>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initStackModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // new() -> empty stack
    funcs["new"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        return newArray({});
    };

    // from_array(array) -> stack
    funcs["from_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("from_array: argument must be array");
        return arr;
    };

    // to_array(stack) -> array
    funcs["to_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("to_array: argument must be stack (array)");
        return arr;
    };

    // push(stack, elem) -> new stack with elem on top
    funcs["push"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("push: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("push: first argument must be stack (array)");
        std::vector<ObjectPtr> result = arr->elements;
        result.push_back(args[1]);
        return newArray(result);
    };

    // pop(stack) -> [top_element, remaining_stack]
    funcs["pop"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("pop: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("pop: argument must be stack (array)");
        if (arr->elements.empty()) return makeError("pop: stack is empty");
        ObjectPtr top = arr->elements.back();
        std::vector<ObjectPtr> remaining(arr->elements.begin(), arr->elements.end() - 1);
        return newArray({top, newArray(remaining)});
    };

    // peek(stack) -> top element or null
    funcs["peek"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("peek: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("peek: argument must be stack (array)");
        if (arr->elements.empty()) return getNull();
        return arr->elements.back();
    };

    // peek_bottom(stack) -> bottom element or null
    funcs["peek_bottom"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("peek_bottom: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("peek_bottom: argument must be stack (array)");
        if (arr->elements.empty()) return getNull();
        return arr->elements.front();
    };

    // size(stack) -> int
    funcs["size"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("size: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("size: argument must be stack (array)");
        return newInteger(static_cast<int64_t>(arr->elements.size()));
    };

    // is_empty(stack) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("is_empty: argument must be stack (array)");
        return newBoolean(arr->elements.empty());
    };

    // contains(stack, elem) -> bool
    funcs["contains"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("contains: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("contains: first argument must be stack (array)");
        for (auto& e : arr->elements)
            if (equals(e, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    // clear(stack) -> empty stack
    funcs["clear"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("clear: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("clear: argument must be stack (array)");
        return newArray({});
    };

    // reverse(stack) -> reversed stack
    funcs["reverse"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("reverse: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("reverse: argument must be stack (array)");
        auto result = arr->elements;
        std::reverse(result.begin(), result.end());
        return newArray(result);
    };

    // push_many(stack, array) -> stack with all elements pushed
    funcs["push_many"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("push_many: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        auto elems = std::dynamic_pointer_cast<Array>(args[1]);
        if (!arr || !elems) return makeError("push_many: both arguments must be arrays");
        std::vector<ObjectPtr> result = arr->elements;
        result.insert(result.end(), elems->elements.begin(), elems->elements.end());
        return newArray(result);
    };

    // pop_n(stack, n) -> [popped_elements, remaining_stack]
    funcs["pop_n"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("pop_n: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("pop_n: first argument must be stack (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("pop_n: second argument must be integer");
        int64_t count = std::min(n->value, static_cast<int64_t>(arr->elements.size()));
        if (count < 0) count = 0;
        std::vector<ObjectPtr> popped(arr->elements.end() - count, arr->elements.end());
        std::vector<ObjectPtr> remaining(arr->elements.begin(), arr->elements.end() - count);
        return newArray({newArray(popped), newArray(remaining)});
    };

    // peek_n(stack, n) -> top n elements without removing
    funcs["peek_n"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("peek_n: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("peek_n: first argument must be stack (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("peek_n: second argument must be integer");
        int64_t count = std::min(n->value, static_cast<int64_t>(arr->elements.size()));
        if (count < 0) count = 0;
        return newArray(std::vector<ObjectPtr>(arr->elements.end() - count, arr->elements.end()));
    };

    // merge(stack1, stack2) -> combined stack (stack2 on top)
    funcs["merge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("merge: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("merge: both arguments must be stacks (arrays)");
        std::vector<ObjectPtr> result = a->elements;
        result.insert(result.end(), b->elements.begin(), b->elements.end());
        return newArray(result);
    };

    // filter(stack, predicate) -> filtered stack
    funcs["filter"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("filter: first argument must be stack (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements)
            if (isTruthy(callCallable(fn, {e}))) result.push_back(e);
        return newArray(result);
    };

    // map_stack(stack, fn) -> transformed stack
    funcs["map_stack"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_stack: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("map_stack: first argument must be stack (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        result.reserve(arr->elements.size());
        for (auto& e : arr->elements) result.push_back(callCallable(fn, {e}));
        return newArray(result);
    };

    // index_of(stack, elem) -> int (-1 if not found)
    funcs["index_of"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("index_of: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("index_of: first argument must be stack (array)");
        for (size_t i = 0; i < arr->elements.size(); i++)
            if (equals(arr->elements[i], args[1])) return newInteger(static_cast<int64_t>(i));
        return newInteger(-1);
    };

    // flatten(stack) -> flattened stack (one level)
    funcs["flatten"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("flatten: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("flatten: argument must be stack (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) {
            if (auto inner = std::dynamic_pointer_cast<Array>(e))
                for (auto& ie : inner->elements) result.push_back(ie);
            else
                result.push_back(e);
        }
        return newArray(result);
    };

    // unique(stack) -> stack with duplicates removed
    funcs["unique"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("unique: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("unique: argument must be stack (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) {
            bool found = false;
            for (auto& r : result) { if (equals(e, r)) { found = true; break; } }
            if (!found) result.push_back(e);
        }
        return newArray(result);
    };

    // min(stack) -> smallest element
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

    // max(stack) -> largest element
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

    Registry::instance().registerModule("stack", funcs);
}

} // namespace darix::native
