#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{10, 20, 30};
    int total = 0;
    for (std::size_t i = 0; i <= v.size(); ++i) {   // bug: <= should be <
        total += v[i];
    }
    std::cout << total << '\n';
}
