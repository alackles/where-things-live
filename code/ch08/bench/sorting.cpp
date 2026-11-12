// Timings for the sorting algorithms of Chapter 8.
//   g++ -std=c++20 -O2 -I. ch08/bench/sorting.cpp -o sorting
#include "ch08/sorts.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

// A textbook quicksort that always uses the FIRST element as its pivot,
// instrumented to count comparisons.
std::uint64_t comparisons = 0;
void quick_first(std::vector<int>& a, std::size_t lo, std::size_t hi) {
    while (hi - lo >= 2) {            // loop on one side to bound the stack depth
        int pivot = a[lo];
        std::size_t last_small = lo;  // a[lo+1..last_small] are < pivot
        for (std::size_t j = lo + 1; j < hi; ++j) {
            ++comparisons;
            if (a[j] < pivot) std::swap(a[++last_small], a[j]);
        }
        std::swap(a[lo], a[last_small]);   // pivot goes between the two parts
        if (last_small - lo < hi - last_small - 1) {
            quick_first(a, lo, last_small);
            lo = last_small + 1;
        } else {
            quick_first(a, last_small + 1, hi);
            hi = last_small;
        }
    }
}

int main() {
    std::mt19937 rng(270);
    std::printf("seconds to sort n random ints\n%10s %10s %10s %10s %10s %10s %10s\n", "n",
                "insertion", "heap", "merge", "quick", "std::sort", "counting");
    for (std::size_t n = 1000; n <= 10000000; n *= 10) {
        std::vector<int> data(n);
        for (int& x : data) x = static_cast<int>(rng() % 1000000);
        auto run = [&](auto sort) {
            std::vector<int> a = data;
            double t = seconds([&] { sort(a); });
            if (!std::is_sorted(a.begin(), a.end())) std::printf("NOT SORTED\n");
            return t;
        };
        double ti = n <= 100000 ? run([](auto& a) { insertion_sort(a); }) : -1;
        double th = run([](auto& a) { heap_sort(a); });
        double tm = run([](auto& a) { merge_sort(a); });
        double tq = run([](auto& a) { quick_sort(a); });
        double ts = run([](auto& a) { std::sort(a.begin(), a.end()); });
        double tc = run([](auto& a) { counting_sort(a, 1000000); });
        if (ti >= 0) std::printf("%10zu %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f\n", n, ti, th, tm, tq, ts, tc);
        else std::printf("%10zu %10s %10.4f %10.4f %10.4f %10.4f %10.4f\n", n, "-", th, tm, tq, ts, tc);
    }

    std::printf("\nquicksort with first-element pivot: comparisons\n%10s %16s %16s %14s\n", "n",
                "random input", "sorted input", "n log2 n");
    for (std::size_t n : {1000u, 10000u, 100000u}) {
        std::vector<int> a(n);
        for (int& x : a) x = static_cast<int>(rng());
        comparisons = 0;
        quick_first(a, 0, n);
        std::uint64_t random_cmp = comparisons;
        comparisons = 0;
        quick_first(a, 0, n);   // a is now sorted
        std::printf("%10zu %16llu %16llu %14.0f\n", n, static_cast<unsigned long long>(random_cmp),
                    static_cast<unsigned long long>(comparisons),
                    static_cast<double>(n) * std::log2(static_cast<double>(n)));
    }
}
