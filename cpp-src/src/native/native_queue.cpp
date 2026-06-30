#include "darix/native/native.hpp"
#include <algorithm>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

void initQueueModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // new() -> empty queue
    funcs["new"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        return newArray({});
    };

    // from_array(array) -> queue
    funcs["from_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("from_array: argument must be array");
        return arr;
    };

    // to_array(queue) -> array
    funcs["to_array"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_array: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("to_array: argument must be queue (array)");
        return arr;
    };

    // enqueue(queue, elem) -> new queue with elem at back
    funcs["enqueue"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("enqueue: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("enqueue: first argument must be queue (array)");
        std::vector<ObjectPtr> result = arr->elements;
        result.push_back(args[1]);
        return newArray(result);
    };

    // dequeue(queue) -> [front_element, remaining_queue]
    funcs["dequeue"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("dequeue: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("dequeue: argument must be queue (array)");
        if (arr->elements.empty()) return makeError("dequeue: queue is empty");
        ObjectPtr front = arr->elements[0];
        std::vector<ObjectPtr> remaining(arr->elements.begin() + 1, arr->elements.end());
        return newArray({front, newArray(remaining)});
    };

    // peek(queue) -> front element or null
    funcs["peek"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("peek: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("peek: argument must be queue (array)");
        if (arr->elements.empty()) return getNull();
        return arr->elements.front();
    };

    // peek_back(queue) -> back element or null
    funcs["peek_back"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("peek_back: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("peek_back: argument must be queue (array)");
        if (arr->elements.empty()) return getNull();
        return arr->elements.back();
    };

    // size(queue) -> int
    funcs["size"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("size: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("size: argument must be queue (array)");
        return newInteger(static_cast<int64_t>(arr->elements.size()));
    };

    // is_empty(queue) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("is_empty: argument must be queue (array)");
        return newBoolean(arr->elements.empty());
    };

    // contains(queue, elem) -> bool
    funcs["contains"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("contains: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("contains: first argument must be queue (array)");
        for (auto& e : arr->elements)
            if (equals(e, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    // clear(queue) -> empty queue
    funcs["clear"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("clear: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("clear: argument must be queue (array)");
        return newArray({});
    };

    // reverse(queue) -> reversed queue
    funcs["reverse"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("reverse: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("reverse: argument must be queue (array)");
        auto result = arr->elements;
        std::reverse(result.begin(), result.end());
        return newArray(result);
    };

    // enqueue_front(queue, elem) -> new queue with elem at front
    funcs["enqueue_front"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("enqueue_front: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("enqueue_front: first argument must be queue (array)");
        std::vector<ObjectPtr> result;
        result.push_back(args[1]);
        result.insert(result.end(), arr->elements.begin(), arr->elements.end());
        return newArray(result);
    };

    // dequeue_back(queue) -> [back_element, remaining_queue]
    funcs["dequeue_back"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("dequeue_back: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("dequeue_back: argument must be queue (array)");
        if (arr->elements.empty()) return makeError("dequeue_back: queue is empty");
        ObjectPtr back = arr->elements.back();
        std::vector<ObjectPtr> remaining(arr->elements.begin(), arr->elements.end() - 1);
        return newArray({back, newArray(remaining)});
    };

    // take(queue, n) -> first n elements without removing
    funcs["take"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("take: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("take: first argument must be queue (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("take: second argument must be integer");
        size_t count = std::min(static_cast<size_t>(std::max(0LL, n->value)), arr->elements.size());
        return newArray(std::vector<ObjectPtr>(arr->elements.begin(), arr->elements.begin() + count));
    };

    // drop(queue, n) -> queue without first n elements
    funcs["drop"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("drop: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("drop: first argument must be queue (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("drop: second argument must be integer");
        size_t skip = std::min(static_cast<size_t>(std::max(0LL, n->value)), arr->elements.size());
        return newArray(std::vector<ObjectPtr>(arr->elements.begin() + skip, arr->elements.end()));
    };

    // merge(q1, q2) -> concatenated queue
    funcs["merge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("merge: expected 2 arguments");
        auto a = std::dynamic_pointer_cast<Array>(args[0]);
        auto b = std::dynamic_pointer_cast<Array>(args[1]);
        if (!a || !b) return makeError("merge: both arguments must be queues (arrays)");
        std::vector<ObjectPtr> result = a->elements;
        result.insert(result.end(), b->elements.begin(), b->elements.end());
        return newArray(result);
    };

    // filter(queue, predicate) -> filtered queue
    funcs["filter"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("filter: first argument must be queue (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements)
            if (isTruthy(callCallable(fn, {e}))) result.push_back(e);
        return newArray(result);
    };

    // map_queue(queue, fn) -> transformed queue
    funcs["map_queue"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_queue: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("map_queue: first argument must be queue (array)");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        result.reserve(arr->elements.size());
        for (auto& e : arr->elements) result.push_back(callCallable(fn, {e}));
        return newArray(result);
    };

    // rotate(queue, n) -> queue rotated left by n positions
    funcs["rotate"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("rotate: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("rotate: first argument must be queue (array)");
        auto n = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!n) return makeError("rotate: second argument must be integer");
        if (arr->elements.empty()) return arr;
        int64_t sz = static_cast<int64_t>(arr->elements.size());
        int64_t k = ((n->value % sz) + sz) % sz;
        auto result = arr->elements;
        std::rotate(result.begin(), result.begin() + k, result.end());
        return newArray(result);
    };

    // flatten(queue) -> flattened queue (one level)
    funcs["flatten"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("flatten: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("flatten: argument must be queue (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) {
            if (auto inner = std::dynamic_pointer_cast<Array>(e))
                for (auto& ie : inner->elements) result.push_back(ie);
            else
                result.push_back(e);
        }
        return newArray(result);
    };

    // unique(queue) -> queue with duplicates removed
    funcs["unique"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("unique: expected 1 argument");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("unique: argument must be queue (array)");
        std::vector<ObjectPtr> result;
        for (auto& e : arr->elements) {
            bool found = false;
            for (auto& r : result) { if (equals(e, r)) { found = true; break; } }
            if (!found) result.push_back(e);
        }
        return newArray(result);
    };

    // index_of(queue, elem) -> int (-1 if not found)
    funcs["index_of"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("index_of: expected 2 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("index_of: first argument must be queue (array)");
        for (size_t i = 0; i < arr->elements.size(); i++)
            if (equals(arr->elements[i], args[1])) return newInteger(static_cast<int64_t>(i));
        return newInteger(-1);
    };

    // slice(queue, start, end?) -> sub-queue
    funcs["slice"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 2 || args.size() > 3) return makeError("slice: expected 2-3 arguments");
        auto arr = std::dynamic_pointer_cast<Array>(args[0]);
        if (!arr) return makeError("slice: first argument must be queue (array)");
        auto startObj = std::dynamic_pointer_cast<Integer>(args[1]);
        if (!startObj) return makeError("slice: second argument must be integer");
        int64_t start = startObj->value;
        int64_t end = static_cast<int64_t>(arr->elements.size());
        if (args.size() == 3) {
            auto endObj = std::dynamic_pointer_cast<Integer>(args[2]);
            if (endObj) end = endObj->value;
        }
        if (start < 0) start = std::max(0LL, static_cast<int64_t>(arr->elements.size()) + start);
        if (end < 0) end = std::max(0LL, static_cast<int64_t>(arr->elements.size()) + end);
        start = std::min(start, static_cast<int64_t>(arr->elements.size()));
        end = std::min(end, static_cast<int64_t>(arr->elements.size()));
        if (start >= end) return newArray({});
        return newArray(std::vector<ObjectPtr>(arr->elements.begin() + start, arr->elements.begin() + end));
    };

    Registry::instance().registerModule("queue", funcs);
}

} // namespace darix::native
