#include "ch09/algorithms.hpp"
#include "ch09/union_find.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

// [maze]
// Fewest steps from (r0, c0) to (r1, c1) in a grid where '#' is a wall,
// moving up, down, left or right. Returns -1 if there is no route.
// The graph is never built: a cell's neighbours are computed on demand.
int maze_steps(const std::vector<std::string>& grid, int r0, int c0, int r1, int c1) {
    const int rows = static_cast<int>(grid.size());
    const int cols = static_cast<int>(grid[0].size());
    std::vector<std::vector<int>> dist(grid.size(), std::vector<int>(grid[0].size(), -1));
    std::queue<std::pair<int, int>> frontier;
    dist[idx(r0)][idx(c0)] = 0;
    frontier.push({r0, c0});

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    while (!frontier.empty()) {
        auto [r, c] = frontier.front();
        frontier.pop();
        if (r == r1 && c == c1) return dist[idx(r)][idx(c)];
        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;   // off the grid
            if (grid[idx(nr)][idx(nc)] == '#') continue;                  // a wall
            if (dist[idx(nr)][idx(nc)] != -1) continue;                   // seen already
            dist[idx(nr)][idx(nc)] = dist[idx(r)][idx(c)] + 1;
            frontier.push({nr, nc});
        }
    }
    return -1;
}
// [/maze]

// [kruskal]
struct WeightedEdge {
    int u, v;
    double weight;
};

// Total weight of a minimum spanning tree of a connected undirected graph.
double mst_weight(int vertices, std::vector<WeightedEdge> edges) {
    std::ranges::sort(edges, {}, &WeightedEdge::weight);   // cheapest first
    UnionFind uf(vertices);
    double total = 0.0;
    for (const WeightedEdge& e : edges) {
        if (uf.unite(e.u, e.v)) total += e.weight;   // joins two components: keep it
    }                                                // else it would close a cycle
    return total;
}
// [/kruskal]

// Reference implementations: slow, simple, and independent of the above.
std::vector<double> bellman_ford(const Graph& g, int s) {
    std::vector<double> d(idx(g.size()), INF);
    d[idx(s)] = 0;
    for (int round = 0; round < g.size(); ++round)
        for (int u = 0; u < g.size(); ++u)
            for (const Edge& e : g.neighbors(u))
                if (d[idx(u)] + e.weight < d[idx(e.to)]) d[idx(e.to)] = d[idx(u)] + e.weight;
    return d;
}

double prim_weight(int n, const std::vector<std::vector<double>>& w) {
    std::vector<bool> in(idx(n), false);
    std::vector<double> best(idx(n), INF);
    best[0] = 0;
    double total = 0;
    for (int step = 0; step < n; ++step) {
        int u = -1;
        for (int v = 0; v < n; ++v)
            if (!in[idx(v)] && (u == -1 || best[idx(v)] < best[idx(u)])) u = v;
        in[idx(u)] = true;
        total += best[idx(u)];
        for (int v = 0; v < n; ++v)
            if (!in[idx(v)] && w[idx(u)][idx(v)] < best[idx(v)]) best[idx(v)] = w[idx(u)][idx(v)];
    }
    return total;
}

int main() {
    // [build]
    // Six vertices; the undirected edges of the chapter's running example.
    Graph g(6);
    g.add_undirected(0, 1);
    g.add_undirected(0, 2);
    g.add_undirected(1, 3);
    g.add_undirected(2, 3);
    g.add_undirected(3, 4);
    g.add_undirected(4, 5);
    // [/build]
    BfsResult r = bfs(g, 0);
    assert((r.dist == std::vector<int>{0, 1, 1, 2, 3, 4}));
    assert((path_to(r.parent, 0, 5) == std::vector<int>{0, 1, 3, 4, 5}));

    std::vector<bool> visited(6, false);
    std::vector<int> order;
    dfs(g, 0, visited, order);
    assert((order == std::vector<int>{0, 1, 3, 2, 4, 5}));
    assert(count_components(g) == 1);

    Graph two(5);
    two.add_undirected(0, 1);
    two.add_undirected(3, 4);
    assert(count_components(two) == 3);
    assert(bfs(two, 0).dist[4] == -1);
    assert(path_to(bfs(two, 0).parent, 0, 4).empty());

    // [courses]
    // Edges point from a prerequisite to the course that requires it.
    enum { Intro, Systems, DataStructures, DiscreteMath, OperatingSystems, Algorithms };
    Graph prereq(6);
    prereq.add_edge(Intro, Systems);
    prereq.add_edge(Intro, DataStructures);
    prereq.add_edge(Systems, OperatingSystems);
    prereq.add_edge(DataStructures, OperatingSystems);
    prereq.add_edge(DataStructures, Algorithms);
    prereq.add_edge(DiscreteMath, Algorithms);
    std::vector<int> plan = topological_order(prereq);
    // [/courses]
    assert((plan == std::vector<int>{Intro, DiscreteMath, Systems, DataStructures,
                                     OperatingSystems, Algorithms}));
    std::vector<int> when(6);
    for (std::size_t i = 0; i < plan.size(); ++i) when[idx(plan[i])] = static_cast<int>(i);
    for (int u = 0; u < 6; ++u)
        for (const Edge& e : prereq.neighbors(u)) assert(when[idx(u)] < when[idx(e.to)]);
    prereq.add_edge(OperatingSystems, Intro);          // a cycle: no valid order
    assert(topological_order(prereq).empty());

    std::vector<std::string> maze{
        "..#.....",
        ".##.###.",
        "....#...",
        ".##.#.#.",
        "..#...#.",
    };
    assert(maze_steps(maze, 0, 0, 4, 7) == 15);
    assert(maze_steps(maze, 0, 0, 0, 3) == 7);
    maze[2][3] = '#';
    maze[0][1] = '#';
    assert(maze_steps(maze, 0, 0, 0, 3) == -1);

    // [weighted]
    Graph roads(5);
    roads.add_undirected(0, 1, 4);
    roads.add_undirected(0, 2, 1);
    roads.add_undirected(2, 1, 2);
    roads.add_undirected(1, 3, 5);
    roads.add_undirected(2, 3, 8);
    roads.add_undirected(3, 4, 3);
    std::vector<double> d = dijkstra(roads, 0);
    // [/weighted]
    assert((d == std::vector<double>{0, 3, 1, 8, 11}));

    // Random DAGs: every edge goes from a lower to a higher position in a
    // hidden shuffled order, so a topological order must exist and must
    // respect every edge.
    std::mt19937 dag_rng(99);
    for (int trial = 0; trial < 300; ++trial) {
        int n = 1 + static_cast<int>(dag_rng() % 12);
        std::vector<int> label(idx(n));
        for (int i = 0; i < n; ++i) label[idx(i)] = i;
        std::shuffle(label.begin(), label.end(), dag_rng);
        Graph dag(n);
        int m = static_cast<int>(dag_rng() % static_cast<unsigned>(2 * n + 1));
        for (int i = 0; i < m; ++i) {
            int a = static_cast<int>(dag_rng() % static_cast<unsigned>(n));
            int b = static_cast<int>(dag_rng() % static_cast<unsigned>(n));
            if (a == b) continue;
            if (a > b) std::swap(a, b);
            dag.add_edge(label[idx(a)], label[idx(b)]);
        }
        std::vector<int> topo = topological_order(dag);
        assert(topo.size() == idx(n));
        std::vector<int> at(idx(n));
        for (std::size_t i = 0; i < topo.size(); ++i) at[idx(topo[i])] = static_cast<int>(i);
        for (int u = 0; u < n; ++u)
            for (const Edge& e : dag.neighbors(u)) assert(at[idx(u)] < at[idx(e.to)]);
    }

    // Random graphs: Dijkstra against Bellman-Ford, BFS against Dijkstra
    // with unit weights, Kruskal against Prim, union-find against DFS.
    std::mt19937 rng(9);
    for (int trial = 0; trial < 300; ++trial) {
        int n = 2 + static_cast<int>(rng() % 12);
        Graph wg(n), ug(n);
        std::vector<WeightedEdge> edges;
        std::vector<std::vector<double>> w(idx(n), std::vector<double>(idx(n), INF));
        UnionFind uf(n);
        int m = static_cast<int>(rng() % static_cast<unsigned>(3 * n));
        for (int i = 0; i < m; ++i) {
            int u = static_cast<int>(rng() % static_cast<unsigned>(n));
            int v = static_cast<int>(rng() % static_cast<unsigned>(n));
            double wt = 1 + static_cast<double>(rng() % 20);
            wg.add_undirected(u, v, wt);
            ug.add_undirected(u, v);
            uf.unite(u, v);
            if (u != v) {
                edges.push_back({u, v, wt});
                w[idx(u)][idx(v)] = std::min(w[idx(u)][idx(v)], wt);
                w[idx(v)][idx(u)] = std::min(w[idx(v)][idx(u)], wt);
            }
        }
        int s = static_cast<int>(rng() % static_cast<unsigned>(n));
        assert(dijkstra(wg, s) == bellman_ford(wg, s));
        std::vector<double> unit = dijkstra(ug, s);
        BfsResult b = bfs(ug, s);
        for (int v = 0; v < n; ++v) {
            if (b.dist[idx(v)] == -1) assert(unit[idx(v)] == INF);
            else assert(unit[idx(v)] == b.dist[idx(v)]);
        }
        assert(uf.sets() == count_components(ug));
        for (int v = 0; v < n; ++v) assert(uf.same(s, v) == (b.dist[idx(v)] != -1));
        if (uf.sets() == 1) assert(mst_weight(n, edges) == prim_weight(n, w));
    }
    std::cout << "graph_test: ok\n";
}
