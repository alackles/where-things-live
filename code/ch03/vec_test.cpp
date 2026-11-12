#include "ch03/vec.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// [same]
// Do our Vec and the reference std::vector hold the same sequence?
template <typename T>
bool same(const Vec<T>& ours, const std::vector<T>& ref) {
    if (!ours.check_invariant()) return false;
    if (ours.size() != ref.size()) return false;
    for (std::size_t i = 0; i < ref.size(); ++i) {
        if (ours[i] != ref[i]) return false;
    }
    return true;
}
// [/same]

// [fuzz]
// Apply the same random operations to both; they must never disagree.
void fuzz(unsigned seed, int steps) {
    std::mt19937 rng(seed);
    Vec<int> ours;
    std::vector<int> ref;

    for (int step = 0; step < steps; ++step) {
        int value = static_cast<int>(rng() % 1000);
        switch (rng() % 6) {
        case 0:
        case 1:   // push_back is the common case, so give it two slots
            ours.push_back(value);
            ref.push_back(value);
            break;
        case 2:
            if (!ref.empty()) {
                ours.pop_back();
                ref.pop_back();
            }
            break;
        case 3: {
            std::size_t at = rng() % (ref.size() + 1);   // may equal size()
            ours.insert(at, value);
            ref.insert(ref.begin() + static_cast<std::ptrdiff_t>(at), value);
            break;
        }
        case 4:
            if (!ref.empty()) {
                std::size_t at = rng() % ref.size();
                ours.erase(at);
                ref.erase(ref.begin() + static_cast<std::ptrdiff_t>(at));
            }
            break;
        case 5:
            if (!ref.empty()) {
                std::size_t at = rng() % ref.size();
                ours[at] = value;
                ref[at] = value;
            }
            break;
        }
        assert(same(ours, ref));
    }
}
// [/fuzz]

int main() {
    Vec<int> v{1, 2, 3};
    assert(v.size() == 3 && v[0] == 1 && v[2] == 3);

    // range-for works because Vec has begin() and end()
    // [range_for]
    int total = 0;
    for (int x : v) total += x;
    // [/range_for]
    assert(total == 6);

    // at() throws on a bad index
    bool threw = false;
    try {
        v.at(3) = 0;
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    // copy, move, assign
    Vec<int> a = v;
    a.push_back(4);
    assert(v.size() == 3 && a.size() == 4);
    Vec<int> b = std::move(a);
    assert(b.size() == 4 && a.size() == 0);   // NOLINT
    v = b;
    assert(v.size() == 4 && v[3] == 4);
    v = Vec<int>{9, 8};
    assert(v.size() == 2 && v[0] == 9);
    Vec<int>& self = v;
    v = self;
    assert(v.size() == 2 && v[1] == 8);

    // push_back of our own element while growing: safe, because the
    // parameter is taken by value
    Vec<int> g{7};
    for (int i = 0; i < 20; ++i) g.push_back(g[0]);
    for (int x : g) assert(x == 7);

    // a type that owns memory itself: this is what the sanitizer is for
    Vec<std::string> words{"heap", "stack"};
    for (int i = 0; i < 50; ++i) words.push_back("word" + std::to_string(i));
    words.insert(0, "first");
    words.erase(1);
    assert(words[0] == "first" && words[1] == "stack" && words.size() == 52);
    Vec<std::string> more = words;
    more[0] = "changed";
    assert(words[0] == "first");

    // [algorithms]
    Vec<int> nums{31, 4, 15, 9, 26};
    std::sort(nums.begin(), nums.end());              // {4, 9, 15, 26, 31}
    auto where = std::find(nums.begin(), nums.end(), 15);
    assert(where != nums.end() && where - nums.begin() == 2);
    // [/algorithms]
    assert(nums[0] == 4 && nums[4] == 31);

    for (unsigned seed = 1; seed <= 200; ++seed) fuzz(seed, 400);
    std::cout << "vec_test: ok\n";
}
