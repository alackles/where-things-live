// Insert n random ints, each into its sorted position found by a linear
// scan, into a std::vector and into a std::list.
//   g++ -std=c++20 -O2 ch04/bench/sorted_insert.cpp -o sorted_insert
#include <chrono>
#include <cstdio>
#include <list>
#include <random>
#include <vector>

template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

// [experiment]
template <typename Container>
void insert_all_sorted(Container& c, const std::vector<int>& values) {
    for (int x : values) {
        auto pos = c.begin();
        while (pos != c.end() && *pos < x) ++pos;   // linear scan
        c.insert(pos, x);   // vector: shift the tail. list: relink.
    }
}
// [/experiment]

int main() {
    std::mt19937 rng(270);
    std::printf("%10s %12s %12s %8s\n", "n", "vector s", "list s", "ratio");
    for (std::size_t n : {10000u, 20000u, 40000u, 80000u, 160000u}) {
        std::vector<int> values(n);
        for (int& x : values) x = static_cast<int>(rng() % 1000000);
        std::vector<int> v;
        std::list<int> l;
        double tv = seconds([&] { insert_all_sorted(v, values); });
        double tl = seconds([&] { insert_all_sorted(l, values); });
        std::printf("%10zu %12.4f %12.4f %8.1f   (%d %d)\n", n, tv, tl, tl / tv,
                    v[n / 2], *l.begin());
    }
}
