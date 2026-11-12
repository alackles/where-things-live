#include "ch10/lru.hpp"
#include "ch10/trie.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

// A deliberately naive LRU cache to test against: a vector, most recent last.
struct SlowLru {
    std::size_t capacity;
    std::vector<std::pair<int, int>> items;
    int* get(int key) {
        for (std::size_t i = 0; i < items.size(); ++i) {
            if (items[i].first == key) {
                auto e = items[i];
                items.erase(items.begin() + static_cast<std::ptrdiff_t>(i));
                items.push_back(e);
                return &items.back().second;
            }
        }
        return nullptr;
    }
    void put(int key, int value) {
        if (int* v = get(key)) { *v = value; return; }
        if (items.size() == capacity) items.erase(items.begin());
        items.push_back({key, value});
    }
};

int main() {
    // [lru_use]
    LruCache<std::string, int> cache(2);
    cache.put("a", 1);
    cache.put("b", 2);
    cache.get("a");            // "a" is now the most recently used
    cache.put("c", 3);         // full: evicts "b", the least recently used
    assert(cache.get("b") == nullptr);
    assert(*cache.get("a") == 1 && *cache.get("c") == 3);
    // [/lru_use]

    LruCache<std::string, int> moved = std::move(cache);   // moving keeps the index valid
    assert(*moved.get("a") == 1 && *moved.get("c") == 3 && moved.get("b") == nullptr);
    moved.put("d", 4);                                     // evicts "a"
    assert(moved.get("a") == nullptr && *moved.get("d") == 4);

    std::mt19937 rng(10);
    for (int trial = 0; trial < 200; ++trial) {
        std::size_t cap = 1 + rng() % 8;
        LruCache<int, int> ours(cap);
        SlowLru ref{cap, {}};
        for (int step = 0; step < 400; ++step) {
            int key = static_cast<int>(rng() % 14);
            if (rng() % 2) {
                int value = static_cast<int>(rng() % 100);
                ours.put(key, value);
                ref.put(key, value);
            } else {
                int* a = ours.get(key);
                int* b = ref.get(key);
                assert((a == nullptr) == (b == nullptr));
                if (a) assert(*a == *b);
            }
            assert(ours.size() == ref.items.size());
        }
    }

    // [trie_use]
    Trie t;
    for (const char* w : {"tree", "trie", "try", "trial", "heap", "hash", "he"}) {
        t.insert(w);
    }
    assert(t.contains("trie") && !t.contains("tri"));   // "tri" is only a prefix
    assert(t.has_prefix("tri") && !t.has_prefix("tra"));
    assert((t.complete("tr", 10) ==
            std::vector<std::string>{"tree", "trial", "trie", "try"}));
    assert((t.complete("he", 10) == std::vector<std::string>{"he", "heap"}));
    // [/trie_use]
    assert(t.size() == 7);
    t.insert("tree");
    assert(t.size() == 7);
    assert((t.complete("", 3) == std::vector<std::string>{"hash", "he", "heap"}));
    assert(t.complete("z", 5).empty());

    // random words: the trie against std::set
    Trie rt;
    std::set<std::string> ref;
    for (int i = 0; i < 3000; ++i) {
        std::string w;
        for (std::size_t len = 1 + rng() % 5; len > 0; --len) w.push_back(static_cast<char>('a' + rng() % 4));
        if (rng() % 3) {
            rt.insert(w);
            ref.insert(w);
        } else {
            assert(rt.contains(w) == ref.contains(w));
            auto it = ref.lower_bound(w);
            bool prefix = it != ref.end() && it->compare(0, w.size(), w) == 0;
            assert(rt.has_prefix(w) == prefix);
        }
    }
    assert(rt.size() == ref.size());
    std::vector<std::string> all = rt.complete("", ref.size() + 1);
    assert(all == std::vector<std::string>(ref.begin(), ref.end()));
    std::cout << "wild_test: ok\n";
}
