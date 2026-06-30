#include "darix/native/native.hpp"
#include <algorithm>
#include <queue>

namespace darix::native {

static ObjectPtr makeError(const std::string& msg) { return newError("%s", msg.c_str()); }

// Helper: get adjacency map from graph representation
static std::shared_ptr<Map> getAdj(ObjectPtr graph) {
    return std::dynamic_pointer_cast<Map>(graph);
}

// Helper: get neighbors of a vertex
static std::vector<ObjectPtr> getNeighbors(ObjectPtr graph, ObjectPtr vertex) {
    auto adj = getAdj(graph);
    if (!adj) return {};
    for (auto& [k, v] : adj->pairs) {
        if (equals(k, vertex)) {
            auto arr = std::dynamic_pointer_cast<Array>(v);
            if (arr) return arr->elements;
            return {};
        }
    }
    return {};
}

// Helper: add edge to adjacency map (mutates)
static void addEdgeToAdj(std::shared_ptr<Map> adj, ObjectPtr from, ObjectPtr to) {
    for (auto& [k, v] : adj->pairs) {
        if (equals(k, from)) {
            auto arr = std::dynamic_pointer_cast<Array>(v);
            if (arr) {
                for (auto& e : arr->elements) if (equals(e, to)) return; // already exists
                arr->elements.push_back(to);
            }
            return;
        }
    }
    adj->pairs.push_back({from, newArray({to})});
}

void initGraphModule() {
    std::unordered_map<std::string, NativeFunc> funcs;

    // new() -> empty graph
    funcs["new"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        auto adj = std::make_shared<Map>();
        return adj;
    };

    // from_edges(edges_array) -> graph. edges = [[from, to], ...]
    funcs["from_edges"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("from_edges: expected 1 argument");
        auto edges = std::dynamic_pointer_cast<Array>(args[0]);
        if (!edges) return makeError("from_edges: argument must be array of [from, to] pairs");
        auto adj = std::make_shared<Map>();
        // First pass: ensure all vertices exist
        for (auto& edge : edges->elements) {
            auto pair = std::dynamic_pointer_cast<Array>(edge);
            if (pair && pair->elements.size() >= 2) {
                // Ensure 'from' vertex exists
                bool fromExists = false;
                for (auto& [k, v] : adj->pairs)
                    if (equals(k, pair->elements[0])) { fromExists = true; break; }
                if (!fromExists) adj->pairs.push_back({pair->elements[0], newArray({})});
                // Ensure 'to' vertex exists
                bool toExists = false;
                for (auto& [k, v] : adj->pairs)
                    if (equals(k, pair->elements[1])) { toExists = true; break; }
                if (!toExists) adj->pairs.push_back({pair->elements[1], newArray({})});
            }
        }
        // Second pass: add edges
        for (auto& edge : edges->elements) {
            auto pair = std::dynamic_pointer_cast<Array>(edge);
            if (pair && pair->elements.size() >= 2) {
                addEdgeToAdj(adj, pair->elements[0], pair->elements[1]);
            }
        }
        return adj;
    };

    // add_vertex(graph, vertex) -> graph
    funcs["add_vertex"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("add_vertex: expected 2 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("add_vertex: first argument must be graph");
        for (auto& [k, v] : adj->pairs)
            if (equals(k, args[1])) return args[0]; // already exists
        adj->pairs.push_back({args[1], newArray({})});
        return args[0];
    };

    // add_edge(graph, from, to) -> graph (directed)
    funcs["add_edge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("add_edge: expected 3 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("add_edge: first argument must be graph");
        addEdgeToAdj(adj, args[1], args[2]);
        return args[0];
    };

    // add_undirected_edge(graph, a, b) -> graph
    funcs["add_undirected_edge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("add_undirected_edge: expected 3 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("add_undirected_edge: first argument must be graph");
        addEdgeToAdj(adj, args[1], args[2]);
        addEdgeToAdj(adj, args[2], args[1]);
        return args[0];
    };

    // vertices(graph) -> array of all vertices
    funcs["vertices"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("vertices: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("vertices: argument must be graph");
        std::vector<ObjectPtr> result;
        for (auto& [k, v] : adj->pairs) result.push_back(k);
        return newArray(result);
    };

    // edges(graph) -> array of [from, to] pairs
    funcs["edges"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("edges: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("edges: argument must be graph");
        std::vector<ObjectPtr> result;
        for (auto& [k, v] : adj->pairs) {
            auto neighbors = std::dynamic_pointer_cast<Array>(v);
            if (neighbors)
                for (auto& n : neighbors->elements)
                    result.push_back(newArray({k, n}));
        }
        return newArray(result);
    };

    // neighbors(graph, vertex) -> array of neighbors
    funcs["neighbors"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("neighbors: expected 2 arguments");
        return newArray(getNeighbors(args[0], args[1]));
    };

    // degree(graph, vertex) -> int (out-degree)
    funcs["degree"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("degree: expected 2 arguments");
        return newInteger(static_cast<int64_t>(getNeighbors(args[0], args[1]).size()));
    };

    // in_degree(graph, vertex) -> int
    funcs["in_degree"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("in_degree: expected 2 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("in_degree: first argument must be graph");
        int64_t count = 0;
        for (auto& [k, v] : adj->pairs) {
            auto neighbors = std::dynamic_pointer_cast<Array>(v);
            if (neighbors)
                for (auto& n : neighbors->elements)
                    if (equals(n, args[1])) count++;
        }
        return newInteger(count);
    };

    // has_vertex(graph, vertex) -> bool
    funcs["has_vertex"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("has_vertex: expected 2 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("has_vertex: first argument must be graph");
        for (auto& [k, v] : adj->pairs)
            if (equals(k, args[1])) return newBoolean(true);
        return newBoolean(false);
    };

    // has_edge(graph, from, to) -> bool
    funcs["has_edge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("has_edge: expected 3 arguments");
        auto neighbors = getNeighbors(args[0], args[1]);
        for (auto& n : neighbors)
            if (equals(n, args[2])) return newBoolean(true);
        return newBoolean(false);
    };

    // remove_vertex(graph, vertex) -> graph
    funcs["remove_vertex"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("remove_vertex: expected 2 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("remove_vertex: first argument must be graph");
        // Remove the vertex entry
        for (auto it = adj->pairs.begin(); it != adj->pairs.end(); ++it) {
            if (equals(it->first, args[1])) { adj->pairs.erase(it); break; }
        }
        // Remove references to this vertex from all neighbor lists
        for (auto& [k, v] : adj->pairs) {
            auto arr = std::dynamic_pointer_cast<Array>(v);
            if (arr) {
                std::vector<ObjectPtr> filtered;
                for (auto& e : arr->elements)
                    if (!equals(e, args[1])) filtered.push_back(e);
                arr->elements = filtered;
            }
        }
        return args[0];
    };

    // remove_edge(graph, from, to) -> graph
    funcs["remove_edge"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("remove_edge: expected 3 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("remove_edge: first argument must be graph");
        for (auto& [k, v] : adj->pairs) {
            if (equals(k, args[1])) {
                auto arr = std::dynamic_pointer_cast<Array>(v);
                if (arr) {
                    std::vector<ObjectPtr> filtered;
                    for (auto& e : arr->elements)
                        if (!equals(e, args[2])) filtered.push_back(e);
                    arr->elements = filtered;
                }
            }
        }
        return args[0];
    };

    // vertex_count(graph) -> int
    funcs["vertex_count"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("vertex_count: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("vertex_count: argument must be graph");
        return newInteger(static_cast<int64_t>(adj->pairs.size()));
    };

    // edge_count(graph) -> int
    funcs["edge_count"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("edge_count: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("edge_count: argument must be graph");
        int64_t count = 0;
        for (auto& [k, v] : adj->pairs) {
            auto arr = std::dynamic_pointer_cast<Array>(v);
            if (arr) count += arr->elements.size();
        }
        return newInteger(count);
    };

    // is_empty(graph) -> bool
    funcs["is_empty"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_empty: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("is_empty: argument must be graph");
        return newBoolean(adj->pairs.empty());
    };

    // bfs(graph, start) -> array of visited vertices in BFS order
    funcs["bfs"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("bfs: expected 2 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("bfs: first argument must be graph");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> visited;
        auto inVisited = [&](ObjectPtr v) {
            for (auto& v2 : visited) if (equals(v, v2)) return true;
            return false;
        };
        std::vector<ObjectPtr> queue = {args[1]};
        while (!queue.empty()) {
            ObjectPtr current = queue.front();
            queue.erase(queue.begin());
            if (inVisited(current)) continue;
            visited.push_back(current);
            result.push_back(current);
            for (auto& n : getNeighbors(args[0], current))
                if (!inVisited(n)) queue.push_back(n);
        }
        return newArray(result);
    };

    // dfs(graph, start) -> array of visited vertices in DFS order
    funcs["dfs"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("dfs: expected 2 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("dfs: first argument must be graph");
        std::vector<ObjectPtr> result;
        std::vector<ObjectPtr> visited;
        auto inVisited = [&](ObjectPtr v) {
            for (auto& v2 : visited) if (equals(v, v2)) return true;
            return false;
        };
        std::vector<ObjectPtr> stack = {args[1]};
        while (!stack.empty()) {
            ObjectPtr current = stack.back();
            stack.pop_back();
            if (inVisited(current)) continue;
            visited.push_back(current);
            result.push_back(current);
            auto neighbors = getNeighbors(args[0], current);
            for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it)
                if (!inVisited(*it)) stack.push_back(*it);
        }
        return newArray(result);
    };

    // shortest_path(graph, start, end) -> array of vertices or null
    funcs["shortest_path"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("shortest_path: expected 3 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("shortest_path: first argument must be graph");
        if (equals(args[1], args[2])) return newArray({args[1]});

        std::vector<ObjectPtr> visited;
        std::vector<std::pair<ObjectPtr, ObjectPtr>> parentMap; // child -> parent
        std::vector<ObjectPtr> queue = {args[1]};
        visited.push_back(args[1]);

        bool found = false;
        while (!queue.empty() && !found) {
            ObjectPtr current = queue.front();
            queue.erase(queue.begin());
            for (auto& n : getNeighbors(args[0], current)) {
                bool seen = false;
                for (auto& v : visited) if (equals(n, v)) { seen = true; break; }
                if (!seen) {
                    visited.push_back(n);
                    parentMap.push_back({n, current});
                    queue.push_back(n);
                    if (equals(n, args[2])) { found = true; break; }
                }
            }
        }

        if (!found) return getNull();

        // Reconstruct path
        std::vector<ObjectPtr> path;
        ObjectPtr current = args[2];
        path.push_back(current);
        while (!equals(current, args[1])) {
            for (auto& [child, parent] : parentMap) {
                if (equals(child, current)) { current = parent; path.push_back(current); break; }
            }
        }
        std::reverse(path.begin(), path.end());
        return newArray(path);
    };

    // has_path(graph, start, end) -> bool (BFS reachability)
    funcs["has_path"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 3) return makeError("has_path: expected 3 arguments");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("has_path: first argument must be graph");
        if (equals(args[1], args[2])) return newBoolean(true);
        std::vector<ObjectPtr> visited;
        auto inVisited = [&](ObjectPtr v) {
            for (auto& v2 : visited) if (equals(v, v2)) return true;
            return false;
        };
        std::vector<ObjectPtr> queue = {args[1]};
        while (!queue.empty()) {
            ObjectPtr current = queue.front();
            queue.erase(queue.begin());
            if (inVisited(current)) continue;
            visited.push_back(current);
            for (auto& n : getNeighbors(args[0], current)) {
                if (equals(n, args[2])) return newBoolean(true);
                if (!inVisited(n)) queue.push_back(n);
            }
        }
        return newBoolean(false);
    };

    // is_connected(graph) -> bool (undirected: all vertices reachable from first)
    funcs["is_connected"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("is_connected: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("is_connected: argument must be graph");
        if (adj->pairs.empty()) return newBoolean(true);
        // Get first vertex
        ObjectPtr start = adj->pairs[0].first;
        // BFS from start
        std::vector<ObjectPtr> visited;
        auto inVisited = [&](ObjectPtr v) {
            for (auto& v2 : visited) if (equals(v, v2)) return true;
            return false;
        };
        std::vector<ObjectPtr> queue = {start};
        while (!queue.empty()) {
            ObjectPtr current = queue.front();
            queue.erase(queue.begin());
            if (inVisited(current)) continue;
            visited.push_back(current);
            for (auto& n : getNeighbors(args[0], current))
                if (!inVisited(n)) queue.push_back(n);
        }
        return newBoolean(visited.size() == adj->pairs.size());
    };

    // topological_sort(graph) -> array or null (only for DAGs)
    funcs["topological_sort"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("topological_sort: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("topological_sort: argument must be graph");

        // Compute in-degrees
        std::vector<ObjectPtr> verts;
        for (auto& [k, v] : adj->pairs) verts.push_back(k);

        std::vector<ObjectPtr> inDegreeVerts;
        std::vector<int64_t> inDeg(verts.size(), 0);
        for (size_t i = 0; i < verts.size(); i++) {
            auto neighbors = getNeighbors(args[0], verts[i]);
            for (auto& n : neighbors)
                for (size_t j = 0; j < verts.size(); j++)
                    if (equals(n, verts[j])) inDeg[j]++;
        }

        std::vector<ObjectPtr> queue;
        for (size_t i = 0; i < verts.size(); i++)
            if (inDeg[i] == 0) queue.push_back(verts[i]);

        std::vector<ObjectPtr> result;
        while (!queue.empty()) {
            ObjectPtr v = queue.front();
            queue.erase(queue.begin());
            result.push_back(v);
            for (auto& n : getNeighbors(args[0], v))
                for (size_t j = 0; j < verts.size(); j++)
                    if (equals(n, verts[j])) {
                        inDeg[j]--;
                        if (inDeg[j] == 0) queue.push_back(verts[j]);
                    }
        }

        if (result.size() != verts.size()) return getNull(); // cycle detected
        return newArray(result);
    };

    // transpose(graph) -> graph with all edges reversed
    funcs["transpose"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("transpose: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("transpose: argument must be graph");
        auto result = std::make_shared<Map>();
        // Ensure all vertices exist
        for (auto& [k, v] : adj->pairs) {
            result->pairs.push_back({k, newArray({})});
        }
        // Reverse edges
        for (auto& [k, v] : adj->pairs) {
            auto neighbors = std::dynamic_pointer_cast<Array>(v);
            if (neighbors)
                for (auto& n : neighbors->elements)
                    addEdgeToAdj(result, n, k);
        }
        return result;
    };

    // subgraph(graph, vertices_array) -> induced subgraph
    funcs["subgraph"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("subgraph: expected 2 arguments");
        auto adj = getAdj(args[0]);
        auto verts = std::dynamic_pointer_cast<Array>(args[1]);
        if (!adj || !verts) return makeError("subgraph: arguments must be graph and array");
        auto result = std::make_shared<Map>();
        for (auto& v : verts->elements) result->pairs.push_back({v, newArray({})});
        for (auto& [k, v] : adj->pairs) {
            auto neighbors = std::dynamic_pointer_cast<Array>(v);
            if (!neighbors) continue;
            bool inSet = false;
            for (auto& sv : verts->elements) if (equals(k, sv)) { inSet = true; break; }
            if (!inSet) continue;
            for (auto& n : neighbors->elements) {
                bool nInSet = false;
                for (auto& sv : verts->elements) if (equals(n, sv)) { nInSet = true; break; }
                if (nInSet) addEdgeToAdj(result, k, n);
            }
        }
        return result;
    };

    // to_edges_list(graph) -> [[from, to, weight?], ...] (alias for edges)
    funcs["to_edges_list"] = funcs["edges"];

    // clone(graph) -> deep copy
    funcs["clone"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("clone: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("clone: argument must be graph");
        auto result = std::make_shared<Map>();
        for (auto& [k, v] : adj->pairs) {
            auto neighbors = std::dynamic_pointer_cast<Array>(v);
            if (neighbors) {
                std::vector<ObjectPtr> copiedNeighbors(neighbors->elements.begin(), neighbors->elements.end());
                result->pairs.push_back({k, newArray(copiedNeighbors)});
            } else {
                result->pairs.push_back({k, newArray({})});
            }
        }
        return result;
    };

    // to_string(graph) -> string representation
    funcs["to_string"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 1) return makeError("to_string: expected 1 argument");
        auto adj = getAdj(args[0]);
        if (!adj) return makeError("to_string: argument must be graph");
        std::string out = "{";
        bool first = true;
        for (auto& [k, v] : adj->pairs) {
            if (!first) out += ", ";
            first = false;
            out += k->inspect() + " -> " + v->inspect();
        }
        out += "}";
        return newString(out);
    };

    // equals(graph1, graph2) -> bool
    funcs["equals"] = [](const std::vector<ObjectPtr>& args) -> ObjectPtr {
        if (args.size() != 2) return makeError("equals: expected 2 arguments");
        auto a = getAdj(args[0]);
        auto b = getAdj(args[1]);
        if (!a || !b) return makeError("equals: both arguments must be graphs");
        if (a->pairs.size() != b->pairs.size()) return newBoolean(false);
        for (auto& [k, v] : a->pairs) {
            auto neighborsA = std::dynamic_pointer_cast<Array>(v);
            auto neighborsBArr = std::dynamic_pointer_cast<Array>(args[1]);
            // Find matching vertex in b
            bool foundVertex = false;
            for (auto& [bk, bv] : b->pairs) {
                if (equals(k, bk)) {
                    foundVertex = true;
                    auto bn = std::dynamic_pointer_cast<Array>(bv);
                    if (!neighborsA || !bn) { if (!neighborsA && !bn) break; continue; }
                    if (neighborsA->elements.size() != bn->elements.size()) return newBoolean(false);
                    for (auto& n : neighborsA->elements) {
                        bool found = false;
                        for (auto& bn2 : bn->elements) if (equals(n, bn2)) { found = true; break; }
                        if (!found) return newBoolean(false);
                    }
                    break;
                }
            }
            if (!foundVertex) return newBoolean(false);
        }
        return newBoolean(true);
    };

    Registry::instance().registerModule("graph", funcs);
}

} // namespace darix::native
