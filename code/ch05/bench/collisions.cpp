// How evenly do two hash functions spread 10,000 similar strings over
// 16,384 buckets?
#include "ch05/hash.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

template <typename HashFn>
void report(const char* name, HashFn h, std::size_t buckets, int keys) {
    std::vector<int> load(buckets, 0);
    for (int i = 0; i < keys; ++i) ++load[h("user" + std::to_string(i)) % buckets];
    int longest = *std::max_element(load.begin(), load.end());
    long empty = std::count(load.begin(), load.end(), 0);
    double probes = 0;   // average comparisons for a successful search
    for (int c : load) probes += c * (c + 1) / 2.0;
    std::printf("%-10s buckets used %5zu of %zu   longest chain %4d   "
                "avg comparisons per hit %6.2f\n",
                name, buckets - static_cast<std::size_t>(empty), buckets, longest, probes / keys);
}

int main() {
    report("sum_hash", sum_hash, 16384, 10000);
    report("fnv1a", fnv1a, 16384, 10000);

    // Bloom filter false-positive rates: measured against theory.
    std::printf("\nBloom filter, 100000 keys\n%14s %4s %12s %12s\n", "bits per key", "k", "measured", "theory");
    for (auto [bpk, k] : {std::pair{4, 3}, {8, 6}, {10, 7}, {16, 11}}) {
        std::size_t m = static_cast<std::size_t>(bpk) * 100000;
        std::vector<bool> bits(m, false);
        auto pos = [&](const std::string& key, int i) {
            std::uint64_t h1 = fnv1a(key), h2 = mix(h1) | 1;
            return (h1 + static_cast<std::uint64_t>(i) * h2) % m;
        };
        for (int i = 0; i < 100000; ++i)
            for (int j = 0; j < k; ++j) bits[pos("in" + std::to_string(i), j)] = true;
        int fp = 0;
        const int trials = 1000000;
        for (int i = 0; i < trials; ++i) {
            bool all = true;
            for (int j = 0; j < k && all; ++j) all = bits[pos("out" + std::to_string(i), j)];
            fp += all;
        }
        double theory = std::pow(1.0 - std::exp(-static_cast<double>(k) / bpk), k);
        std::printf("%14d %4d %11.3f%% %11.3f%%\n", bpk, k, 100.0 * fp / trials, 100.0 * theory);
    }
}
