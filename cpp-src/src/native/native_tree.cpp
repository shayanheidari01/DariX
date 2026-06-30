#include "darix/native/native.hpp"
#include <algorithm>
#include <queue>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

// Helper: create a node [value, children]
static ObjectPtr makeNode(ObjectPtr value, std::vector<ObjectPtr> children) {
    return newArray({value, newArray(children)});
}

// Helper: get value from node
static ObjectPtr nodeValue(ObjectPtr node) {
    auto arr = std::dynamic_pointer_cast<Array>(node);
    if (!arr || arr->elements.empty()) return getNull();
    return arr->elements[0];
}

// Helper: get children from node
static std::vector<ObjectPtr> nodeChildren(ObjectPtr node) {
    auto arr = std::dynamic_pointer_cast<Array>(node);
    if (!arr || arr->elements.size() < 2) return {};
    auto children = std::dynamic_pointer_cast<Array>(arr->elements[1]);
    if (!children) return {};
    return children->elements;
}

// Helper: set children on node (returns new node)
static ObjectPtr setNodeChildren(ObjectPtr node, const std::vector<ObjectPtr>& children) {
    return newArray({nodeValue(node), newArray(children)});
}

void initTreeModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // node(value, children_array) -> tree node
    funcs["node"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() < 1 || args.size() > 2) return makeError("node: expected 1-2 arguments");
        if (args.size() == 1) return makeNode(args[0], {});
        auto children = std::dynamic_pointer_cast<Array>(args[1]);
        if (children) return makeNode(args[0], children->elements);
        return makeNode(args[0], {args[1]});
    };

    // leaf(value) -> node with no children
    funcs["leaf"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("leaf: expected 1 argument");
        return makeNode(args[0], {});
    };

    // value(node) -> node's value
    funcs["value"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("value: expected 1 argument");
        return nodeValue(args[0]);
    };

    // children(node) -> array of children
    funcs["children"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("children: expected 1 argument");
        return newArray(nodeChildren(args[0]));
    };

    // is_leaf(node) -> bool
    funcs["is_leaf"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_leaf: expected 1 argument");
        return newBoolean(nodeChildren(args[0]).empty());
    };

    // is_root(node) -> bool (always true for a standalone node - helper for consistency)
    funcs["is_root"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_root: expected 1 argument");
        return newBoolean(true);
    };

    // add_child(parent, child) -> new parent with child appended
    funcs["add_child"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_child: expected 2 arguments");
        auto kids = nodeChildren(args[0]);
        kids.push_back(args[1]);
        return setNodeChildren(args[0], kids);
    };

    // add_children(parent, children_array) -> new parent with all children appended
    funcs["add_children"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_children: expected 2 arguments");
        auto kids = nodeChildren(args[0]);
        auto newKids = std::dynamic_pointer_cast<Array>(args[1]);
        if (newKids)
            for (auto& c : newKids->elements) kids.push_back(c);
        return setNodeChildren(args[0], kids);
    };

    // set_value(node, value) -> new node with updated value
    funcs["set_value"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("set_value: expected 2 arguments");
        return makeNode(args[1], nodeChildren(args[0]));
    };

    // size(node) -> total number of nodes in subtree
    funcs["size"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("size: expected 1 argument");
        std::vector<ObjectPtr> stack = {args[0]};
        int64_t total = 0;
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            total++;
            auto ch = nodeChildren(current);
            for (auto& c : ch) stack.push_back(c);
        }
        return newInteger(total);
    };

    // depth(node) -> max depth of tree
    funcs["depth"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("depth: expected 1 argument");
        // BFS to find max depth
        std::vector<std::pair<ObjectPtr, int>> queue = {{args[0], 1}};
        int maxDepth = 1;
        while (!queue.empty()) {
            auto [node, d] = queue.front();
            queue.erase(queue.begin());
            if (d > maxDepth) maxDepth = d;
            for (auto& c : nodeChildren(node)) queue.push_back({c, d + 1});
        }
        return newInteger(maxDepth);
    };

    // height(node) -> height (depth - 1, or 0 for leaf)
    funcs["height"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("height: expected 1 argument");
        std::vector<std::pair<ObjectPtr, int>> queue = {{args[0], 0}};
        int maxHeight = 0;
        while (!queue.empty()) {
            auto [node, h] = queue.front();
            queue.erase(queue.begin());
            auto kids = nodeChildren(node);
            if (!kids.empty() && h + 1 > maxHeight) maxHeight = h + 1;
            for (auto& c : kids) queue.push_back({c, h + 1});
        }
        return newInteger(maxHeight);
    };

    // depth_of(node, target_value) -> depth of first node with matching value, -1 if not found
    funcs["depth_of"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("depth_of: expected 2 arguments");
        std::vector<std::pair<ObjectPtr, int>> queue = {{args[0], 0}};
        while (!queue.empty()) {
            auto [node, d] = queue.front();
            queue.erase(queue.begin());
            if (equals(nodeValue(node), args[1])) return newInteger(d);
            for (auto& c : nodeChildren(node)) queue.push_back({c, d + 1});
        }
        return newInteger(-1);
    };

    // find(node, value) -> first node with matching value, or null
    funcs["find"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("find: expected 2 arguments");
        std::vector<ObjectPtr> stack = {args[0]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            if (equals(nodeValue(current), args[1])) return current;
            auto kids = nodeChildren(current);
            for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
        }
        return getNull();
    };

    // find_all(node, predicate) -> array of nodes matching predicate
    funcs["find_all"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("find_all: expected 2 arguments");
        ObjectPtr fn = args[1];
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> stack = {args[0]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            if (isTruthy(callCallable(fn, {nodeValue(current)}))) result.push_back(current);
            auto kids = nodeChildren(current);
            for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
        }
        return newArray(result);
    };

    // preorder(node) -> array of values in preorder traversal
    funcs["preorder"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("preorder: expected 1 argument");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> stack = {args[0]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            result.push_back(nodeValue(current));
            auto kids = nodeChildren(current);
            for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
        }
        return newArray(result);
    };

    // inorder(node) -> array of values in inorder traversal (left-root-right)
    funcs["inorder"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("inorder: expected 1 argument");
        std::vector<ObjectPtr> result;
        // Use explicit stack for inorder
        std::vector<ObjectPtr> stack;
        ObjectPtr current = args[0];
        while (current || !stack.empty()) {
            while (current) {
                stack.push_back(current);
                auto kids = nodeChildren(current);
                current = kids.empty() ? getNull() : kids[0];
            }
            current = stack.back();
            stack.pop_back();
            result.push_back(nodeValue(current));
            auto kids = nodeChildren(current);
            current = (kids.size() > 1) ? kids[1] : getNull();
        }
        return newArray(result);
    };

    // postorder(node) -> array of values in postorder traversal
    funcs["postorder"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("postorder: expected 1 argument");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> stack1 = {args[0]};
        std::vector<ObjectPtr> stack2;
        while (!stack1.empty()) {
            ObjectPtr current = stack1.back();
            stack1.pop_back();
            stack2.push_back(current);
            auto kids = nodeChildren(current);
            for (auto& c : kids) stack1.push_back(c);
        }
        while (!stack2.empty()) {
            result.push_back(nodeValue(stack2.back()));
            stack2.pop_back();
        }
        return newArray(result);
    };

    // levelorder(node) -> array of arrays, each level as a sub-array
    funcs["levelorder"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("levelorder: expected 1 argument");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> currentLevel = {args[0]};
        while (!currentLevel.empty()) {
            std::vector<ObjectPtr> values;
            std::vector<ObjectPtr> nextLevel;
            for (auto& node : currentLevel) {
                values.push_back(nodeValue(node));
                for (auto& c : nodeChildren(node)) nextLevel.push_back(c);
            }
            result.push_back(newArray(values));
            currentLevel = nextLevel;
        }
        return newArray(result);
    };

    // map_tree(node, fn) -> new tree with fn applied to every value
    funcs["map_tree"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("map_tree: expected 2 arguments");
        ObjectPtr fn = args[1];
        ObjectPtr newVal = callCallable(fn, {nodeValue(args[0])});
        auto kids = nodeChildren(args[0]);
        std::vector<ObjectPtr> newKids;
        for (auto& c : kids) newKids.push_back(callCallable({fn}, {c}));
        // Actually need recursive map on children
        std::function<ObjectPtr(ObjectPtr)> mapNode = [&](ObjectPtr n) -> ObjectPtr {
            ObjectPtr v = callCallable(fn, {nodeValue(n)});
            auto ch = nodeChildren(n);
            std::vector<ObjectPtr> mapped;
            for (auto& c : ch) mapped.push_back(mapNode(c));
            return makeNode(v, mapped);
        };
        return mapNode(args[0]);
    };

    // filter_tree(node, predicate) -> tree with only matching nodes kept (pruned)
    funcs["filter_tree"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("filter_tree: expected 2 arguments");
        ObjectPtr fn = args[1];
        std::function<ObjectPtr(ObjectPtr)> filterNode = [&](ObjectPtr n) -> ObjectPtr {
            if (!isTruthy(callCallable(fn, {nodeValue(n)}))) return getNull();
            auto kids = nodeChildren(n);
            std::vector<ObjectPtr> filtered;
            for (auto& c : kids) {
                auto fc = filterNode(c);
                if (fc) filtered.push_back(fc);
            }
            return makeNode(nodeValue(n), filtered);
        };
        return filterNode(args[0]);
    };

    // to_array_preorder(node) -> alias for preorder
    funcs["to_array_preorder"] = funcs["preorder"];

    // to_array_levelorder(node) -> flattened level order
    funcs["to_array_levelorder"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_array_levelorder: expected 1 argument");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> currentLevel = {args[0]};
        while (!currentLevel.empty()) {
            std::vector<ObjectPtr> nextLevel;
            for (auto& node : currentLevel) {
                result.push_back(nodeValue(node));
                for (auto& c : nodeChildren(node)) nextLevel.push_back(c);
            }
            currentLevel = nextLevel;
        }
        return newArray(result);
    };

    // sibling_values(node, value) -> array of sibling values for first node matching value
    funcs["sibling_values"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("sibling_values: expected 2 arguments");
        // BFS to find parent of target
        std::vector<std::pair<ObjectPtr, ObjectPtr>> queue; // {node, parent}
        auto rootKids = nodeChildren(args[0]);
        for (auto& c : rootKids) queue.push_back({c, args[0]});

        while (!queue.empty()) {
            auto [node, parent] = queue.front();
            queue.erase(queue.begin());
            if (equals(nodeValue(node), args[1])) {
                auto parentKids = nodeChildren(parent);
                std::vector<ObjectPtr> sibs;
                for (auto& c : parentKids)
                    if (!equals(nodeValue(c), args[1]) || c.get() != node.get())
                        sibs.push_back(nodeValue(c));
                return newArray(sibs);
            }
            for (auto& c : nodeChildren(node)) queue.push_back({c, node});
        }
        return newArray({});
    };

    // parent(node, target_value) -> value of parent of first node matching target, or null
    funcs["parent"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("parent: expected 2 arguments");
        std::vector<std::pair<ObjectPtr, ObjectPtr>> queue;
        auto rootKids = nodeChildren(args[0]);
        for (auto& c : rootKids) queue.push_back({c, args[0]});

        while (!queue.empty()) {
            auto [node, par] = queue.front();
            queue.erase(queue.begin());
            if (equals(nodeValue(node), args[1])) return nodeValue(par);
            for (auto& c : nodeChildren(node)) queue.push_back({c, node});
        }
        return getNull();
    };

    // leaves(node) -> array of leaf values
    funcs["leaves"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("leaves: expected 1 argument");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> stack = {args[0]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            auto kids = nodeChildren(current);
            if (kids.empty()) result.push_back(nodeValue(current));
            else for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
        }
        return newArray(result);
    };

    // count_leaves(node) -> int
    funcs["count_leaves"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("count_leaves: expected 1 argument");
        int64_t count = 0;
        std::vector<ObjectPtr> stack = {args[0]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            auto kids = nodeChildren(current);
            if (kids.empty()) count++;
            else for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
        }
        return newInteger(count);
    };

    // count_internal(node) -> int (non-leaf nodes)
    funcs["count_internal"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("count_internal: expected 1 argument");
        int64_t count = 0;
        std::vector<ObjectPtr> stack = {args[0]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            auto kids = nodeChildren(current);
            if (!kids.empty()) count++;
            for (auto it = kids.rbegin(); it != kids.rend(); ++it) stack.push_back(*it);
        }
        return newInteger(count);
    };

    // to_string(node) -> string representation
    funcs["to_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_string: expected 1 argument");
        // Simple recursive string representation
        std::function<std::string(ObjectPtr)> toString = [&](ObjectPtr n) -> std::string {
            auto val = nodeValue(n);
            auto kids = nodeChildren(n);
            if (kids.empty()) return val->inspect();
            std::string out = val->inspect() + " [";
            for (size_t i = 0; i < kids.size(); i++) {
                if (i > 0) out += ", ";
                out += toString(kids[i]);
            }
            out += "]";
            return out;
        };
        return newString(toString(args[0]));
    };

    // equals(node1, node2) -> bool (structural equality)
    funcs["equals"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("equals: expected 2 arguments");
        std::function<bool(ObjectPtr, ObjectPtr)> eq = [&](ObjectPtr a, ObjectPtr b) -> bool {
            if (!equals(nodeValue(a), nodeValue(b))) return false;
            auto ka = nodeChildren(a);
            auto kb = nodeChildren(b);
            if (ka.size() != kb.size()) return false;
            for (size_t i = 0; i < ka.size(); i++)
                if (!eq(ka[i], kb[i])) return false;
            return true;
        };
        return newBoolean(eq(args[0], args[1]));
    };

    // clone(node) -> deep copy of tree
    funcs["clone"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("clone: expected 1 argument");
        std::function<ObjectPtr(ObjectPtr)> cloneNode = [&](ObjectPtr n) -> ObjectPtr {
            auto kids = nodeChildren(n);
            std::vector<ObjectPtr> cloned;
            for (auto& c : kids) cloned.push_back(cloneNode(c));
            return makeNode(nodeValue(n), cloned);
        };
        return cloneNode(args[0]);
    };

    Registry::instance().registerModule("tree", funcs);
}

} // namespace darix::native
