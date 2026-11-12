// Reads whole numbers from standard input and prints a summary.
// Try:  ./stats < ch01/stats.in
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// [functions]
int smallest(const std::vector<int>& v) {
    int best = v[0];
    for (int x : v) {
        if (x < best) best = x;
    }
    return best;
}

double mean(const std::vector<int>& v) {
    long long total = 0;
    for (int x : v) total += x;
    return static_cast<double>(total) / static_cast<double>(v.size());
}
// [/functions]

// [main]
int main() {
    std::vector<int> data;
    int x;
    while (std::cin >> x) {   // read until input runs out
        data.push_back(x);    // the vector grows as needed
    }

    if (data.empty()) {
        std::cout << "no data\n";
        return 0;
    }

    std::cout << "count:    " << data.size() << '\n'
              << "smallest: " << smallest(data) << '\n'
              << "mean:     " << mean(data) << '\n';

    for (std::size_t i = 0; i < data.size(); ++i) {
        if (data[i] < 0) {
            std::cout << "negative value at index " << i << '\n';
        }
    }
}
// [/main]
