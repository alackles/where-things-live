// Graph: a directed, weighted graph stored as adjacency lists (Chapter 9).
// Vertices are the integers 0 .. size()-1.
#pragma once
#include <cstddef>
#include <vector>

// [graph]
struct Edge {
    int to;          // the vertex this edge leads to
    double weight;   // its cost (1 if the graph is unweighted)
};

class Graph {
public:
    explicit Graph(int vertices) : adj_(static_cast<std::size_t>(vertices)) {}

    int size() const { return static_cast<int>(adj_.size()); }

    // A directed edge from u to v.
    void add_edge(int u, int v, double weight = 1.0) {
        adj_[static_cast<std::size_t>(u)].push_back(Edge{v, weight});
    }

    // An undirected edge is a pair of directed ones.
    void add_undirected(int u, int v, double weight = 1.0) {
        add_edge(u, v, weight);
        add_edge(v, u, weight);
    }

    // The edges leaving u.
    const std::vector<Edge>& neighbors(int u) const {
        return adj_[static_cast<std::size_t>(u)];
    }

private:
    std::vector<std::vector<Edge>> adj_;   // adj_[u] lists u's outgoing edges
};
// [/graph]
