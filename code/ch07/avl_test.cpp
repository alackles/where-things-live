#include "ch07/avl.hpp"
#include "ch07/btree.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

template <typename Tree>
std::vector<int> keys_of(const Tree& t) {
    std::vector<int> out;
    t.in_order([&](int k) { out.push_back(k); });
    return out;
}

void fuzz_avl(unsigned seed, int range) {
    std::mt19937 rng(seed);
    AvlTree<int> ours;
    std::set<int> ref;
    for (int step = 0; step < 2000; ++step) {
        int key = static_cast<int>(rng() % static_cast<unsigned>(range));
        switch (rng() % 4) {
        case 0: case 1: assert(ours.insert(key) == ref.insert(key).second); break;
        case 2: assert(ours.erase(key) == (ref.erase(key) == 1)); break;
        case 3: assert(ours.contains(key) == ref.contains(key)); break;
        }
        assert(ours.size() == ref.size());
        assert(ours.check_invariant());   // order, heights, AND balance, every step
    }
    assert(keys_of(ours) == std::vector<int>(ref.begin(), ref.end()));
}

void fuzz_btree(unsigned seed, int range) {
    std::mt19937 rng(seed);
    BTree<int, 4> ours;   // tiny nodes, so that splits happen constantly
    std::set<int> ref;
    for (int step = 0; step < 2000; ++step) {
        int key = static_cast<int>(rng() % static_cast<unsigned>(range));
        if (rng() % 3 != 0) assert(ours.insert(key) == ref.insert(key).second);
        else assert(ours.contains(key) == ref.contains(key));
        if (step % 20 == 0) assert(ours.check_invariant());
    }
    assert(ours.check_invariant() && ours.size() == ref.size());
    assert(keys_of(ours) == std::vector<int>(ref.begin(), ref.end()));
}

int main() {
    // The four textbook cases, each fixed by one or two rotations.
    for (auto order : {std::vector<int>{3, 2, 1}, {1, 2, 3}, {3, 1, 2}, {1, 3, 2}}) {
        AvlTree<int> t;
        for (int k : order) t.insert(k);
        assert(t.height() == 2 && t.check_invariant());
    }

    // Sorted insertions: the plain BST's worst case.
    AvlTree<int> sorted;
    const int n = 100000;
    for (int k = 0; k < n; ++k) sorted.insert(k);
    assert(sorted.check_invariant());
    assert(sorted.height() <= static_cast<int>(1.4405 * std::log2(n + 2)));
    assert(sorted.height() == 17);

    for (unsigned seed = 1; seed <= 30; ++seed) {
        fuzz_avl(seed, 40);
        fuzz_avl(seed, 3000);
        fuzz_btree(seed, 60);
        fuzz_btree(seed, 5000);
    }

    AvlTree<int> taken = std::move(sorted);
    assert(taken.size() == static_cast<std::size_t>(n) && taken.check_invariant());
    assert(sorted.size() == 0 && sorted.check_invariant());   // NOLINT: moved-from is empty

    BTree<int> big;
    for (int k = 0; k < n; ++k) big.insert(k);
    assert(big.check_invariant() && big.height() <= 5);
    for (int k = 0; k < n; k += 7) assert(big.contains(k));
    assert(!big.contains(-1) && !big.contains(n));

    // [std_map]
    std::map<std::string, int> born{{"Hopper", 1906}, {"Lovelace", 1815},
                                    {"Turing", 1912}, {"Knuth", 1938}};

    // Iteration is in key order, always.
    std::string names;
    for (const auto& [name, year] : born) names += name + ' ';
    assert(names == "Hopper Knuth Lovelace Turing ");

    // Smallest and largest keys.
    assert(born.begin()->first == "Hopper");
    assert(born.rbegin()->first == "Turing");

    // First key that is not less than "L".
    auto it = born.lower_bound("L");
    assert(it->first == "Lovelace");

    // Every key in the half-open range ["H", "L").
    int count = 0;
    for (auto p = born.lower_bound("H"); p != born.lower_bound("L"); ++p) ++count;
    assert(count == 2);   // Hopper, Knuth
    // [/std_map]
    std::cout << "avl_test: ok\n";
}
