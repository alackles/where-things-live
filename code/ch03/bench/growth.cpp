// How much copying do n push_backs cause under different growth policies?
//   g++ -std=c++20 -O2 ch03/bench/growth.cpp -o growth
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>

// [simulate]
// Counts the element copies caused by n push_backs, where `next` maps
// the current capacity to the capacity after one growth step.
template <typename Policy>
std::uint64_t copies_for(std::uint64_t n, Policy next) {
    std::uint64_t copies = 0, size = 0, capacity = 0;
    for (std::uint64_t i = 0; i < n; ++i) {
        if (size == capacity) {
            copies += size;             // every element moves to the new block
            capacity = next(capacity);
        }
        ++size;
    }
    return copies;
}
// [/simulate]

// A real growable array with a pluggable policy, for timing.
template <typename Policy>
double time_pushes(std::size_t n, Policy next) {
    auto start = std::chrono::steady_clock::now();
    int* data = nullptr;
    std::size_t size = 0, capacity = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (size == capacity) {
            std::size_t bigger_cap = next(capacity);
            int* bigger = new int[bigger_cap];
            for (std::size_t k = 0; k < size; ++k) bigger[k] = data[k];
            delete[] data;
            data = bigger;
            capacity = bigger_cap;
        }
        data[size++] = static_cast<int>(i);
    }
    volatile int sink = data[n / 2];
    (void)sink;
    delete[] data;
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

int main() {
    auto plus_one = [](std::uint64_t c) { return c + 1; };
    auto plus_1000 = [](std::uint64_t c) { return c + 1000; };
    auto times_1_5 = [](std::uint64_t c) { return c < 2 ? c + 1 : c + c / 2; };
    auto doubling = [](std::uint64_t c) { return c == 0 ? 1 : 2 * c; };

    std::printf("%12s %16s %16s %14s %14s\n", "n", "+1", "+1000", "x1.5", "x2");
    for (std::uint64_t n : {1000ull, 10000ull, 100000ull, 1000000ull}) {
        std::printf("%12llu %16llu %16llu %14llu %14llu\n",
                    static_cast<unsigned long long>(n),
                    static_cast<unsigned long long>(copies_for(n, plus_one)),
                    static_cast<unsigned long long>(copies_for(n, plus_1000)),
                    static_cast<unsigned long long>(copies_for(n, times_1_5)),
                    static_cast<unsigned long long>(copies_for(n, doubling)));
    }

    std::printf("\nseconds for n push_backs\n%12s %12s %12s\n", "n", "+1000", "x2");
    for (std::size_t n : {250000u, 500000u, 1000000u, 2000000u, 4000000u}) {
        auto p1000 = [](std::size_t c) { return c + 1000; };
        auto dbl = [](std::size_t c) { return c == 0 ? std::size_t{1} : 2 * c; };
        std::printf("%12zu %12.4f %12.4f\n", n, time_pushes(n, p1000), time_pushes(n, dbl));
    }
}
