#include <iostream>
#include <vector>

int main() {
    std::vector<int> v{10, 20, 30};
    int& first = v[0];          // a reference into the vector's block

    for (int i = 0; i < 100; ++i) {
        v.push_back(i);         // sooner or later: a new, bigger block
    }

    first = 99;                 // writes into the OLD block, long since freed
    std::cout << v[0] << '\n';
}
