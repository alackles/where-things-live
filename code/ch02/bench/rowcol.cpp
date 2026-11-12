// Same work, same Big-O, different order of memory access.
//   g++ -std=c++20 -O2 ch02/bench/rowcol.cpp -o rowcol
#include <chrono>
#include <cstdio>
#include <vector>

template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

int main() {
    for (std::size_t n : {1024u, 2048u, 4096u, 8192u}) {
        // An n-by-n grid stored as one long vector, row after row:
        // the element in row r, column c is at index r * n + c.
        std::vector<int> grid(n * n, 1);
        long long sum_rows = 0, sum_cols = 0;

        // [rows]
        double by_rows = seconds([&] {
            for (std::size_t r = 0; r < n; ++r)
                for (std::size_t c = 0; c < n; ++c)
                    sum_rows += grid[r * n + c];
        });
        // [/rows]

        // [cols]
        double by_cols = seconds([&] {
            for (std::size_t c = 0; c < n; ++c)
                for (std::size_t r = 0; r < n; ++r)
                    sum_cols += grid[r * n + c];
        });
        // [/cols]

        std::printf("n = %5zu   by rows %7.4f s   by columns %7.4f s   "
                    "ratio %5.1f   (sums %lld %lld)\n",
                    n, by_rows, by_cols, by_cols / by_rows, sum_rows, sum_cols);
    }
}
