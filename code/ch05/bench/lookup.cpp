// Nanoseconds per successful lookup, for n random integer keys, in five
// structures.   g++ -std=c++20 -O2 -I. ch05/bench/lookup.cpp -o lookup
#include "ch05/chained_map.hpp"
#include "ch05/probing_map.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <map>
#include <random>
#include <unordered_map>
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
    std::printf("%10s %12s %12s %14s %12s %12s\n", "n", "sorted vec", "std::map",
                "unordered_map", "ChainedMap", "ProbingMap");
    for (std::size_t n = 1000; n <= 10000000; n *= 10) {
        std::vector<int> keys(n);
        for (int& k : keys) k = static_cast<int>(rng());
        std::vector<int> queries(1000000);
        for (int& q : queries) q = keys[rng() % n];

        std::vector<int> sorted = keys;
        std::sort(sorted.begin(), sorted.end());
        std::map<int, int> tree;
        std::unordered_map<int, int> um;
        ChainedMap<int, int> cm;
        ProbingMap<int, int> pm;
        for (int k : keys) { tree[k] = k; um[k] = k; cm.put(k, k); pm.put(k, k); }

        long long sink = 0;
        double per = 1e9 / static_cast<double>(queries.size());
        double t1 = seconds([&] { for (int q : queries) sink += *std::lower_bound(sorted.begin(), sorted.end(), q); });
        double t2 = seconds([&] { for (int q : queries) sink += tree.find(q)->second; });
        double t3 = seconds([&] { for (int q : queries) sink += um.find(q)->second; });
        double t4 = seconds([&] { for (int q : queries) sink += *cm.find(q); });
        double t5 = seconds([&] { for (int q : queries) sink += *pm.find(q); });
        std::printf("%10zu %12.1f %12.1f %14.1f %12.1f %12.1f   (%lld)\n", n, t1 * per, t2 * per,
                    t3 * per, t4 * per, t5 * per, sink % 1000);
    }
}
