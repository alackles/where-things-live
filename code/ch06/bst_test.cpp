#include "ch06/bst.hpp"

#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

template <typename K>
std::vector<K> keys_in_order(const Bst<K>& t) {
    std::vector<K> out;
    t.in_order([&](const K& k) { out.push_back(k); });
    return out;
}

void fuzz(unsigned seed, int range) {
    std::mt19937 rng(seed);
    Bst<int> ours;
    std::set<int> ref;
    for (int step = 0; step < 1500; ++step) {
        int key = static_cast<int>(rng() % static_cast<unsigned>(range));
        switch (rng() % 5) {
        case 0: case 1:
            assert(ours.insert(key) == ref.insert(key).second);
            break;
        case 2:
            assert(ours.erase(key) == (ref.erase(key) == 1));
            break;
        case 3:
            assert(ours.contains(key) == ref.contains(key));
            break;
        case 4: {
            const int* lb = ours.lower_bound(key);
            auto it = ref.lower_bound(key);
            assert((lb != nullptr) == (it != ref.end()));
            if (lb) assert(*lb == *it);
            break;
        }
        }
        assert(ours.size() == ref.size());
        if (step % 25 == 0) {
            assert(ours.check_invariant());
            assert(keys_in_order(ours) == std::vector<int>(ref.begin(), ref.end()));
        }
    }
    while (!ref.empty()) {            // empty it out, smallest first
        assert(*ours.min() == *ref.begin());
        assert(ours.erase(*ref.begin()));
        ref.erase(ref.begin());
    }
    assert(ours.empty() && ours.min() == nullptr && ours.check_invariant());
}

int main() {
    // [build]
    Bst<int> t;
    for (int k : {50, 30, 70, 20, 40, 60, 80, 35}) t.insert(k);
    // [/build]
    assert(t.size() == 8 && t.height() == 4);
    assert(!t.insert(40));
    assert((keys_in_order(t) == std::vector<int>{20, 30, 35, 40, 50, 60, 70, 80}));

    std::vector<int> pre, level;
    t.pre_order([&](int k) { pre.push_back(k); });
    t.level_order([&](int k) { level.push_back(k); });
    assert((pre == std::vector<int>{50, 30, 20, 40, 35, 70, 60, 80}));
    assert((level == std::vector<int>{50, 30, 70, 20, 40, 60, 80, 35}));

    assert(*t.min() == 20);
    assert(*t.lower_bound(36) == 40 && *t.lower_bound(40) == 40);
    assert(t.lower_bound(81) == nullptr);

    assert(t.erase(20));     // a leaf
    assert(t.erase(40));     // one child
    assert(t.erase(50));     // two children: the root
    assert(!t.erase(99));
    assert((keys_in_order(t) == std::vector<int>{30, 35, 60, 70, 80}));
    assert(t.check_invariant());

    // Sorted insertions produce a tree of height n: a linked list.
    Bst<int> chain;
    for (int i = 0; i < 500; ++i) chain.insert(i);
    assert(chain.height() == 500);

    Bst<std::string> words;
    for (const char* w : {"pear", "apple", "fig", "plum", "kiwi"}) words.insert(w);
    Bst<std::string> moved = std::move(words);   // moving works; copying would not compile
    assert(words.size() == 0 && words.empty() && words.check_invariant());   // NOLINT
    words.insert("again");                      // a moved-from tree is empty and usable
    assert(words.size() == 1 && moved.size() == 5);
    assert((keys_in_order(moved) ==
            std::vector<std::string>{"apple", "fig", "kiwi", "pear", "plum"}));

    for (unsigned seed = 1; seed <= 60; ++seed) {
        fuzz(seed, 30);
        fuzz(seed, 2000);
    }
    std::cout << "bst_test: ok\n";
}
