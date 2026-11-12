// Everyday idioms for the standard containers (Appendix C).
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

int main() {
    // [erase]
    std::vector<int> v{4, 8, 15, 16, 23, 42};

    // Remove every element matching a condition (C++20).
    std::erase_if(v, [](int x) { return x % 2 == 1; });   // {4, 8, 16, 42}

    // Remove one element by position, keeping order: O(n).
    v.erase(v.begin() + 1);                               // {4, 16, 42}

    // Remove one element by position when order does not matter: O(1).
    v[0] = v.back();
    v.pop_back();                                         // {42, 16}
    // [/erase]
    assert((v == std::vector<int>{42, 16}));

    // [maps]
    std::unordered_map<std::string, int> stock{{"apple", 3}, {"pear", 0}};

    // Look up without inserting.
    if (auto it = stock.find("apple"); it != stock.end()) {
        it->second -= 1;                // it->first is the key, it->second the value
    }
    bool has_fig = stock.contains("fig");            // false, and "fig" is not added

    // Insert only if absent; otherwise leave the existing value alone.
    stock.try_emplace("pear", 99);                   // no effect: "pear" exists
    stock.try_emplace("plum", 7);                    // inserted

    // Visit every entry. Structured bindings name the two halves.
    int total = 0;
    for (const auto& [name, count] : stock) total += count;
    // [/maps]
    assert(!has_fig && stock.size() == 3 && stock["pear"] == 0 && total == 9);

    // [algorithms]
    std::vector<int> a{5, 3, 9, 1, 7};

    int sum = std::accumulate(a.begin(), a.end(), 0);                  // 25
    int biggest = *std::max_element(a.begin(), a.end());               // 9
    bool any_neg = std::any_of(a.begin(), a.end(), [](int x) { return x < 0; });
    auto n_big = std::count_if(a.begin(), a.end(), [](int x) { return x > 4; });

    std::sort(a.begin(), a.end());                                     // {1, 3, 5, 7, 9}
    bool has7 = std::binary_search(a.begin(), a.end(), 7);             // needs sorted input
    int next = *std::lower_bound(a.begin(), a.end(), 6);               // 7: first >= 6

    std::reverse(a.begin(), a.end());                                  // {9, 7, 5, 3, 1}
    // [/algorithms]
    assert(sum == 25 && biggest == 9 && !any_neg && n_big == 3 && has7);
    assert(a.front() == 9 && next == 7);
    std::cout << "stl_idioms: ok\n";
}
