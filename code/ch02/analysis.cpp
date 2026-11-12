#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

// [has_duplicate]
bool has_duplicate(const std::vector<int>& v) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        for (std::size_t j = i + 1; j < v.size(); ++j) {
            if (v[i] == v[j]) return true;
        }
    }
    return false;
}
// [/has_duplicate]

// [insertion_sort]
void insertion_sort(std::vector<int>& v) {
    for (std::size_t i = 1; i < v.size(); ++i) {
        int x = v[i];                    // next element to place
        std::size_t j = i;
        while (j > 0 && v[j - 1] > x) {  // slide larger elements right
            v[j] = v[j - 1];
            --j;
        }
        v[j] = x;                        // drop x into the gap
    }
}
// [/insertion_sort]

// [halvings]
int halvings(int n) {
    int steps = 0;
    while (n > 1) {
        n = n / 2;
        ++steps;
    }
    return steps;
}
// [/halvings]

// Same algorithm, counting element shifts, to check the analysis.
std::size_t insertion_sort_shifts(std::vector<int>& v) {
    std::size_t shifts = 0;
    for (std::size_t i = 1; i < v.size(); ++i) {
        int x = v[i];
        std::size_t j = i;
        while (j > 0 && v[j - 1] > x) {
            v[j] = v[j - 1];
            --j;
            ++shifts;
        }
        v[j] = x;
    }
    return shifts;
}

int main() {
    assert(!has_duplicate({}));
    assert(!has_duplicate({1, 2, 3}));
    assert(has_duplicate({1, 2, 1}));

    assert(halvings(1) == 0);
    assert(halvings(8) == 3);
    assert(halvings(1000000) == 19);

    std::mt19937 rng(2);
    for (int trial = 0; trial < 2000; ++trial) {
        std::vector<int> a(rng() % 30);
        for (int& x : a) x = static_cast<int>(rng() % 20);
        std::vector<int> expected = a;
        std::sort(expected.begin(), expected.end());
        insertion_sort(a);
        assert(a == expected);
    }

    // Sorted input: no shifts. Reversed input: n(n-1)/2 shifts.
    const std::size_t n = 100;
    std::vector<int> up(n), down(n);
    for (std::size_t i = 0; i < n; ++i) {
        up[i] = static_cast<int>(i);
        down[i] = static_cast<int>(n - i);
    }
    assert(insertion_sort_shifts(up) == 0);
    assert(insertion_sort_shifts(down) == n * (n - 1) / 2);
    std::cout << "analysis: ok\n";
}
