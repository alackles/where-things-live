// Heights of binary search trees built from random and from sorted keys.
#include "ch06/bst.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>

int main() {
    std::mt19937 rng(270);
    std::printf("%10s %8s %12s %12s %12s\n", "n", "log2 n", "height", "avg depth", "1.39 log2 n");
    for (int n : {1000, 10000, 100000, 1000000}) {
        std::vector<int> keys(static_cast<std::size_t>(n));
        std::iota(keys.begin(), keys.end(), 0);
        std::shuffle(keys.begin(), keys.end(), rng);
        Bst<int> t;
        for (int k : keys) t.insert(k);
        std::printf("%10d %8.1f %12d %12.1f %12.1f\n", n, std::log2(n), t.height(),
                    t.average_depth(), 1.39 * std::log2(n));
    }
    for (int n : {1000, 10000}) {
        Bst<int> t;
        for (int k = 0; k < n; ++k) t.insert(k);
        std::printf("sorted: n = %d   height = %d   avg depth = %.1f\n", n, t.height(), t.average_depth());
    }
}
