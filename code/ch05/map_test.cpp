#include "ch05/bloom.hpp"
#include "ch05/chained_map.hpp"
#include "ch05/hash.hpp"
#include "ch05/probing_map.hpp"

#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>

// [point]
struct Point {
    int x;
    int y;
    bool operator==(const Point& other) const = default;
};

// A hash for Point: combine the two coordinates, then scramble.
struct PointHash {
    std::size_t operator()(const Point& p) const {
        std::uint64_t packed = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(p.x)) << 32) |
                               static_cast<std::uint32_t>(p.y);
        return static_cast<std::size_t>(mix(packed));
    }
};
// [/point]

template <typename Map>
void fuzz(unsigned seed, int key_range) {
    std::mt19937 rng(seed);
    Map ours;
    std::unordered_map<int, int> ref;
    for (int step = 0; step < 2000; ++step) {
        int key = static_cast<int>(rng() % static_cast<unsigned>(key_range));
        int value = static_cast<int>(rng() % 1000);
        switch (rng() % 4) {
        case 0: case 1:
            ours.put(key, value);
            ref[key] = value;
            break;
        case 2:
            assert(ours.erase(key) == (ref.erase(key) == 1));
            break;
        case 3: {
            int* found = ours.find(key);
            auto it = ref.find(key);
            assert((found != nullptr) == (it != ref.end()));
            if (found) assert(*found == it->second);
            break;
        }
        }
        assert(ours.size() == ref.size());
        if (step % 50 == 0) assert(ours.check_invariant());
    }
    assert(ours.check_invariant());
    for (const auto& [k, v] : ref) {
        int* found = ours.find(k);
        assert(found && *found == v);
    }
}

int main() {
    // [use]
    ChainedMap<std::string, int> ages;
    ages.put("Ada", 36);
    ages.put("Grace", 85);
    ages.put("Ada", 37);                  // replaces the earlier value

    if (int* a = ages.find("Ada")) {      // found: a points at the value
        *a += 1;
    }
    assert(ages.find("Alan") == nullptr); // not found
    // [/use]
    assert(*ages.find("Ada") == 38 && ages.size() == 2);
    ages["Alan"] += 41;
    assert(*ages.find("Alan") == 41);

    // [unordered]
    std::unordered_set<Point, PointHash> visited;
    visited.insert(Point{2, 7});
    visited.insert(Point{2, 7});          // already present: no effect
    assert(visited.size() == 1);
    assert(visited.contains(Point{2, 7}));
    assert(!visited.contains(Point{7, 2}));
    // [/unordered]

    ProbingMap<std::string, int> pm;
    for (int i = 0; i < 1000; ++i) pm.put("key" + std::to_string(i), i);
    for (int i = 0; i < 1000; i += 2) assert(pm.erase("key" + std::to_string(i)));
    for (int i = 0; i < 1000; ++i) {
        int* v = pm.find("key" + std::to_string(i));
        assert((v != nullptr) == (i % 2 == 1));
        if (v) assert(*v == i);
    }
    assert(pm.check_invariant());

    for (unsigned seed = 1; seed <= 40; ++seed) {
        fuzz<ChainedMap<int, int>>(seed, 50);     // few keys: many repeats
        fuzz<ChainedMap<int, int>>(seed, 5000);   // many keys: growth
        fuzz<ProbingMap<int, int>>(seed, 50);
        fuzz<ProbingMap<int, int>>(seed, 5000);
    }

    // Bloom filter: never a false negative.
    BloomFilter bf(10000, 7);
    for (int i = 0; i < 1000; ++i) bf.add("member" + std::to_string(i));
    for (int i = 0; i < 1000; ++i) assert(bf.might_contain("member" + std::to_string(i)));
    int false_positives = 0;
    for (int i = 0; i < 10000; ++i) {
        if (bf.might_contain("stranger" + std::to_string(i))) ++false_positives;
    }
    assert(false_positives < 300);   // theory says about 0.8 percent
    std::cout << "map_test: ok\n";
}
