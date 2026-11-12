#include "ch02/search.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

int main() {
    // Hand-picked cases first: empty, one element, ends, middle, absent.
    std::vector<int> empty;
    assert(linear_search(empty, 5) == 0);
    assert(binary_search(empty, 5) == 0);
    assert(lower_bound_index(empty, 5) == 0);

    std::vector<int> one{5};
    assert(binary_search(one, 5) == 0 && binary_search(one, 4) == 1 && binary_search(one, 6) == 1);
    assert(lower_bound_index(one, 4) == 0 && lower_bound_index(one, 5) == 0);
    assert(lower_bound_index(one, 6) == 1 && linear_search(one, 5) == 0);

    std::vector<int> v{2, 3, 5, 7, 11, 13};
    assert(linear_search(v, 2) == 0);
    assert(linear_search(v, 13) == 5);
    assert(linear_search(v, 6) == v.size());
    assert(binary_search(v, 2) == 0);
    assert(binary_search(v, 13) == 5);
    assert(binary_search(v, 7) == 3);
    assert(binary_search(v, 6) == v.size());
    assert(binary_search(v, 1) == v.size());
    assert(binary_search(v, 99) == v.size());
    assert(lower_bound_index(v, 7) == 3);
    assert(lower_bound_index(v, 6) == 3);
    assert(lower_bound_index(v, 1) == 0);
    assert(lower_bound_index(v, 99) == 6);

    // [differential]
    // Then thousands of random cases, checked against the standard library.
    std::mt19937 rng(270);
    for (int trial = 0; trial < 5000; ++trial) {
        std::size_t n = rng() % 40;
        std::vector<int> a(n);
        for (int& x : a) x = static_cast<int>(rng() % 50);
        std::sort(a.begin(), a.end());
        int target = static_cast<int>(rng() % 52) - 1;

        auto it = std::lower_bound(a.begin(), a.end(), target);
        auto expected = static_cast<std::size_t>(it - a.begin());
        assert(lower_bound_index(a, target) == expected);

        bool present = std::binary_search(a.begin(), a.end(), target);
        std::size_t where = binary_search(a, target);
        assert(present == (where != a.size()));
        if (present) assert(a[where] == target);
    }
    // [/differential]

    // Comparison counts: at most floor(log2 n) + 1 for the binary search.
    for (std::size_t n : {1u, 2u, 3u, 7u, 8u, 1000u, 1000000u}) {
        std::vector<int> a(n);
        for (std::size_t i = 0; i < n; ++i) a[i] = static_cast<int>(2 * i);
        std::size_t bound = 1;
        for (std::size_t m = n; m > 1; m /= 2) ++bound;
        for (int target : {-1, 0, 5, static_cast<int>(n), static_cast<int>(2 * n)}) {
            std::size_t c = 0;
            lower_bound_count(a, target, c);
            assert(c <= bound);
        }
    }
    std::cout << "search_test: ok\n";
}
