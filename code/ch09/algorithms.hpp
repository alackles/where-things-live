// Graph traversals and shortest paths (Chapter 9).
#pragma once
#include "ch09/graph.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

inline std::size_t idx(int v) { return static_cast<std::size_t>(v); }

// [bfs]
struct BfsResult {
    std::vector<int> dist;     // edges on a shortest path from the source; -1 if unreachable
    std::vector<int> parent;   // previous vertex on that path; -1 for the source
};

inline BfsResult bfs(const Graph& g, int source) {
    BfsResult r{std::vector<int>(idx(g.size()), -1), std::vector<int>(idx(g.size()), -1)};
    std::queue<int> frontier;
    r.dist[idx(source)] = 0;
    frontier.push(source);

    while (!frontier.empty()) {
        int u = frontier.front();
        frontier.pop();
        for (const Edge& e : g.neighbors(u)) {
            if (r.dist[idx(e.to)] == -1) {            // first time we have seen e.to
                r.dist[idx(e.to)] = r.dist[idx(u)] + 1;
                r.parent[idx(e.to)] = u;
                frontier.push(e.to);
            }
        }
    }
    return r;
}
// [/bfs]

// [path]
// Follow parent links back from target, then reverse.
inline std::vector<int> path_to(const std::vector<int>& parent, int source, int target) {
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[idx(v)]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    if (path.front() != source) path.clear();   // target was not reachable
    return path;
}
// [/path]

// [dfs]
// Visit everything reachable from u that has not been visited, depth first.
inline void dfs(const Graph& g, int u, std::vector<bool>& visited, std::vector<int>& order) {
    visited[idx(u)] = true;
    order.push_back(u);
    for (const Edge& e : g.neighbors(u)) {
        if (!visited[idx(e.to)]) dfs(g, e.to, visited, order);
    }
}
// [/dfs]

// [components]
// Number of connected components of an undirected graph.
inline int count_components(const Graph& g) {
    std::vector<bool> visited(idx(g.size()), false);
    std::vector<int> order;
    int components = 0;
    for (int v = 0; v < g.size(); ++v) {
        if (!visited[idx(v)]) {       // v is in a component we have not explored
            ++components;
            dfs(g, v, visited, order);
        }
    }
    return components;
}
// [/components]

// [topo]
// An ordering of a directed graph's vertices in which every edge points
// forward. Returns an empty vector if the graph has a cycle (no such order).
inline std::vector<int> topological_order(const Graph& g) {
    std::vector<int> indegree(idx(g.size()), 0);
    for (int u = 0; u < g.size(); ++u)
        for (const Edge& e : g.neighbors(u)) ++indegree[idx(e.to)];

    std::queue<int> ready;                       // vertices with no unmet prerequisites
    for (int v = 0; v < g.size(); ++v)
        if (indegree[idx(v)] == 0) ready.push(v);

    std::vector<int> order;
    while (!ready.empty()) {
        int u = ready.front();
        ready.pop();
        order.push_back(u);
        for (const Edge& e : g.neighbors(u)) {   // u is done: release its successors
            if (--indegree[idx(e.to)] == 0) ready.push(e.to);
        }
    }
    if (order.size() != idx(g.size())) order.clear();   // something never became ready
    return order;
}
// [/topo]

// [dijkstra]
const double INF = std::numeric_limits<double>::infinity();

// Shortest-path distances from source. All edge weights must be >= 0.
inline std::vector<double> dijkstra(const Graph& g, int source) {
    std::vector<double> dist(idx(g.size()), INF);
    using Item = std::pair<double, int>;                       // (distance, vertex)
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;   // min-heap

    dist[idx(source)] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[idx(u)]) continue;          // a stale entry: u was already settled
        for (const Edge& e : g.neighbors(u)) {
            double candidate = d + e.weight;
            if (candidate < dist[idx(e.to)]) {   // found a shorter way to e.to
                dist[idx(e.to)] = candidate;
                pq.push({candidate, e.to});
            }
        }
    }
    return dist;
}
// [/dijkstra]
