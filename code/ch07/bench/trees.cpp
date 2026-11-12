// Heights, rotation counts and timings for the trees of Chapter 7.
#include "ch07/avl.hpp"
#include "ch07/btree.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <random>
#include <set>
#include <vector>

template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

int main() {
    std::mt19937 rng(270);
    std::printf("keys inserted in SORTED order\n%10s %8s %10s %12s %12s\n", "n", "log2 n",
                "AVL height", "rot/insert", "B-tree ht");
    for (int n : {1000, 10000, 100000, 1000000}) {
        AvlTree<int> avl;
        BTree<int> bt;
        AvlTree<int>::rotations = 0;
        for (int k = 0; k < n; ++k) { avl.insert(k); bt.insert(k); }
        std::printf("%10d %8.1f %10d %12.3f %12d\n", n, std::log2(n), avl.height(),
                    static_cast<double>(AvlTree<int>::rotations) / n, bt.height());
    }
    std::printf("\nkeys inserted in RANDOM order\n%10s %8s %10s %12s %12s\n", "n", "log2 n",
                "AVL height", "rot/insert", "B-tree ht");
    for (int n : {1000, 10000, 100000, 1000000}) {
        std::vector<int> keys(static_cast<std::size_t>(n));
        std::iota(keys.begin(), keys.end(), 0);
        std::shuffle(keys.begin(), keys.end(), rng);
        AvlTree<int> avl;
        BTree<int> bt;
        AvlTree<int>::rotations = 0;
        for (int k : keys) { avl.insert(k); bt.insert(k); }
        std::printf("%10d %8.1f %10d %12.3f %12d\n", n, std::log2(n), avl.height(),
                    static_cast<double>(AvlTree<int>::rotations) / n, bt.height());
    }

    std::printf("\nns per operation, random int keys\n%10s | %10s %10s %10s | %10s %10s %10s %10s\n",
                "n", "ins set", "ins AVL", "ins BTree", "find vec", "find set", "find AVL", "find BTree");
    for (std::size_t n = 10000; n <= 10000000; n *= 10) {
        std::vector<int> keys(n);
        for (int& k : keys) k = static_cast<int>(rng());
        std::vector<int> queries(1000000);
        for (int& q : queries) q = keys[rng() % n];
        std::set<int> st;
        AvlTree<int> avl;
        BTree<int> bt;
        double i1 = seconds([&] { for (int k : keys) st.insert(k); });
        double i2 = seconds([&] { for (int k : keys) avl.insert(k); });
        double i3 = seconds([&] { for (int k : keys) bt.insert(k); });
        std::vector<int> sorted = keys;
        std::sort(sorted.begin(), sorted.end());
        long long sink = 0;
        double f0 = seconds([&] { for (int q : queries) sink += std::binary_search(sorted.begin(), sorted.end(), q); });
        double f1 = seconds([&] { for (int q : queries) sink += st.contains(q); });
        double f2 = seconds([&] { for (int q : queries) sink += avl.contains(q); });
        double f3 = seconds([&] { for (int q : queries) sink += bt.contains(q); });
        double pi = 1e9 / static_cast<double>(n), pf = 1e9 / static_cast<double>(queries.size());
        std::printf("%10zu | %10.0f %10.0f %10.0f | %10.0f %10.0f %10.0f %10.0f  (%lld)\n", n, i1 * pi,
                    i2 * pi, i3 * pi, f0 * pf, f1 * pf, f2 * pf, f3 * pf, sink % 1000);
    }
}
