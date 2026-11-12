// Union-find variants, and Dijkstra with and without a priority queue.
#include "ch09/algorithms.hpp"
#include "ch09/union_find.hpp"

#include <chrono>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>

template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

// The naive version: no union by size, no path compression.
struct NaiveUF {
    std::vector<int> parent;
    explicit NaiveUF(int n) : parent(static_cast<std::size_t>(n)) { std::iota(parent.begin(), parent.end(), 0); }
    int find(int x) { while (parent[idx(x)] != x) x = parent[idx(x)]; return x; }
    void unite(int a, int b) { parent[idx(find(a))] = find(b); }
};

// Dijkstra without a heap: scan all vertices for the closest unsettled one.
std::vector<double> dijkstra_scan(const Graph& g, int source) {
    std::vector<double> dist(idx(g.size()), INF);
    std::vector<bool> done(idx(g.size()), false);
    dist[idx(source)] = 0;
    for (int round = 0; round < g.size(); ++round) {
        int u = -1;
        for (int v = 0; v < g.size(); ++v)
            if (!done[idx(v)] && (u == -1 || dist[idx(v)] < dist[idx(u)])) u = v;
        if (dist[idx(u)] == INF) break;
        done[idx(u)] = true;
        for (const Edge& e : g.neighbors(u))
            if (dist[idx(u)] + e.weight < dist[idx(e.to)]) dist[idx(e.to)] = dist[idx(u)] + e.weight;
    }
    return dist;
}

int main() {
    std::printf("n unions in increasing order (i with i+1), then n finds\n%10s %14s %14s\n", "n",
                "naive (s)", "by size+compr");
    for (int n : {10000, 20000, 40000, 80000}) {
        long long sink = 0;
        double tn = seconds([&] {
            NaiveUF uf(n);
            for (int i = 0; i + 1 < n; ++i) uf.unite(i, i + 1);
            for (int i = 0; i < n; ++i) sink += uf.find(i);
        });
        double tg = seconds([&] {
            UnionFind uf(n);
            for (int i = 0; i + 1 < n; ++i) uf.unite(i, i + 1);
            for (int i = 0; i < n; ++i) sink += uf.find(i);
        });
        std::printf("%10d %14.4f %14.6f   (%lld)\n", n, tn, tg, sink % 10);
    }
    std::mt19937 rng(270);
    std::printf("\nn random unions + n finds, good version only\n%10s %12s\n", "n", "ns per op");
    for (int n : {100000, 1000000, 10000000}) {
        UnionFind uf(n);
        long long sink = 0;
        double t = seconds([&] {
            for (int i = 0; i < n; ++i)
                uf.unite(static_cast<int>(rng() % static_cast<unsigned>(n)), static_cast<int>(rng() % static_cast<unsigned>(n)));
            for (int i = 0; i < n; ++i) sink += uf.find(i);
        });
        std::printf("%10d %12.1f   (%lld)\n", n, t / (2.0 * n) * 1e9, sink % 10);
    }

    std::printf("\nDijkstra on a random graph with 5 edges per vertex\n%10s %14s %14s\n", "vertices",
                "array scan (s)", "heap (s)");
    for (int n : {2000, 8000, 32000, 128000}) {
        Graph g(n);
        for (int u = 0; u < n; ++u)
            for (int k = 0; k < 5; ++k)
                g.add_edge(u, static_cast<int>(rng() % static_cast<unsigned>(n)), 1 + rng() % 100);
        std::vector<double> a, b;
        double ts = n <= 32000 ? seconds([&] { a = dijkstra_scan(g, 0); }) : -1;
        double th = seconds([&] { b = dijkstra(g, 0); });
        if (ts >= 0) std::printf("%10d %14.4f %14.4f   %s\n", n, ts, th, a == b ? "agree" : "DISAGREE");
        else std::printf("%10d %14s %14.4f\n", n, "-", th);
    }
}
