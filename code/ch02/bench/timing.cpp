// Doubling experiments. Build with -O2 and WITHOUT sanitizers:
//   g++ -std=c++20 -O2 -I. ch02/bench/timing.cpp -o timing
#include "ch02/search.hpp"

#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

// [timer]
// Runs `work` once and returns how long it took, in seconds.
template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}
// [/timer]

bool has_duplicate(const std::vector<int>& v) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        for (std::size_t j = i + 1; j < v.size(); ++j) {
            if (v[i] == v[j]) return true;
        }
    }
    return false;
}

int main() {
    // [doubling]
    std::printf("%10s %12s %8s\n", "n", "seconds", "ratio");
    double previous = 0.0;
    for (std::size_t n = 4000; n <= 128000; n *= 2) {
        std::vector<int> v(n);
        for (std::size_t i = 0; i < n; ++i) v[i] = static_cast<int>(i);

        bool answer = false;
        double t = seconds([&] { answer = has_duplicate(v); });

        std::printf("%10zu %12.4f", n, t);
        if (previous > 0.0) std::printf(" %8.2f", t / previous);
        std::printf("%s\n", answer ? "  (?!)" : "");
        previous = t;
    }
    // [/doubling]

    std::printf("\n%10s %14s %14s\n", "n", "linear ns", "binary ns");
    std::mt19937 rng(270);
    for (std::size_t n = 1000; n <= 100000000; n *= 10) {
        std::vector<int> v(n);
        for (std::size_t i = 0; i < n; ++i) v[i] = static_cast<int>(2 * i);
        // Few queries for the slow search, many distinct ones for the fast
        // search (repeating the same queries would only measure the cache).
        std::vector<int> few(n <= 1000000 ? 2000 : 100);
        std::vector<int> many(1000000);
        for (int& t : few) t = static_cast<int>(rng() % (2 * n));
        for (int& t : many) t = static_cast<int>(rng() % (2 * n));

        std::size_t sink = 0;
        double tl = seconds([&] {
            for (int t : few) sink += linear_search(v, t);
        });
        double tb = seconds([&] {
            for (int t : many) sink += lower_bound_index(v, t);
        });
        std::printf("%10zu %14.1f %14.1f   (checksum %zu)\n", n,
                    tl / static_cast<double>(few.size()) * 1e9,
                    tb / static_cast<double>(many.size()) * 1e9, sink % 1000);
    }
}
