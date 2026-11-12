#include "ch08/heap.hpp"
#include "ch08/sorts.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <vector>

// [student]
struct Student {
    std::string name;
    int year;
    double gpa;
};
// [/student]

// A record that compares by key only, to observe stability.
struct Tagged {
    int key;
    int tag;   // original position
    bool operator<(const Tagged& other) const { return key < other.key; }
};

template <typename Sort>
void check_sort(Sort sort, const char* name) {
    std::mt19937 rng(8);
    for (int trial = 0; trial < 600; ++trial) {
        std::size_t n = rng() % 60;
        int range = (trial % 3 == 0) ? 4 : 1000;       // sometimes many duplicates
        std::vector<int> a(n);
        for (int& x : a) x = static_cast<int>(rng() % static_cast<unsigned>(range));
        if (trial % 7 == 1) std::sort(a.begin(), a.end());                    // already sorted
        if (trial % 7 == 2) std::sort(a.begin(), a.end(), std::greater<>());  // reversed
        std::vector<int> expected = a;
        std::sort(expected.begin(), expected.end());
        sort(a);
        if (a != expected) {
            std::cout << name << " failed\n";
            assert(false);
        }
    }
}

int main() {
    check_sort([](std::vector<int>& a) { heap_sort(a); }, "heap_sort");
    check_sort([](std::vector<int>& a) { merge_sort(a); }, "merge_sort");
    check_sort([](std::vector<int>& a) { quick_sort(a); }, "quick_sort");
    check_sort([](std::vector<int>& a) { insertion_sort(a); }, "insertion_sort");
    check_sort([](std::vector<int>& a) { counting_sort(a, 1000); }, "counting_sort");

    // Merge sort is stable: equal keys keep their original relative order.
    std::mt19937 rng(88);
    std::vector<Tagged> recs(500);
    for (int i = 0; i < 500; ++i) recs[static_cast<std::size_t>(i)] = {static_cast<int>(rng() % 10), i};
    merge_sort(recs);
    for (std::size_t i = 1; i < recs.size(); ++i) {
        assert(recs[i - 1].key <= recs[i].key);
        if (recs[i - 1].key == recs[i].key) assert(recs[i - 1].tag < recs[i].tag);
    }

    // The heap, against the standard library's priority queue.
    for (unsigned seed = 1; seed <= 50; ++seed) {
        std::mt19937 r(seed);
        MinHeap<int> ours;
        std::priority_queue<int, std::vector<int>, std::greater<int>> ref;
        for (int step = 0; step < 1000; ++step) {
            if (r() % 3 != 0) {
                int v = static_cast<int>(r() % 500);
                ours.push(v);
                ref.push(v);
            } else if (!ref.empty()) {
                assert(ours.top() == ref.top());
                ours.pop();
                ref.pop();
            }
            assert(ours.size() == ref.size() && ours.check_invariant());
        }
    }
    // Linear-time construction from arbitrary data, at many sizes.
    for (unsigned seed = 1; seed <= 300; ++seed) {
        std::mt19937 r(seed);
        std::vector<int> items(r() % 40);
        for (int& x : items) x = static_cast<int>(r() % 30);
        MinHeap<int> h(items);
        assert(h.check_invariant() && h.size() == items.size());
        std::sort(items.begin(), items.end());
        for (int expect : items) {
            assert(h.top() == expect);
            h.pop();
            assert(h.check_invariant());
        }
    }

    std::vector<int> raw{9, 4, 7, 1, 8, 2, 6, 3, 5};
    MinHeap<int> built(raw);
    assert(built.check_invariant());
    for (int expect = 1; expect <= 9; ++expect) {
        assert(built.top() == expect);
        built.pop();
    }

    // [std_pq]
    std::priority_queue<int> largest_first;          // a max-heap by default
    for (int x : {3, 9, 4}) largest_first.push(x);
    assert(largest_first.top() == 9);

    // To get the smallest first, supply std::greater as the comparison.
    std::priority_queue<int, std::vector<int>, std::greater<int>> smallest_first;
    for (int x : {3, 9, 4}) smallest_first.push(x);
    assert(smallest_first.top() == 3);
    // [/std_pq]

    // [std_sort]
    std::vector<Student> roster{
        {"Okafor", 2027, 3.6}, {"Lindqvist", 2026, 3.9}, {"Abara", 2027, 3.9},
        {"Tran", 2026, 3.2},   {"Moreau", 2028, 3.6},
    };

    // Sort with a comparison: "a comes before b when..." Highest GPA first.
    std::sort(roster.begin(), roster.end(),
              [](const Student& a, const Student& b) { return a.gpa > b.gpa; });
    assert(roster[0].gpa == 3.9 && roster[4].name == "Tran");

    // C++20 ranges: sort the whole container by one member.
    std::ranges::sort(roster, {}, &Student::name);
    assert(roster[0].name == "Abara" && roster[4].name == "Tran");

    // A stable sort keeps equal elements in their existing order, so the
    // roster is now grouped by year and still alphabetical within each year.
    std::ranges::stable_sort(roster, {}, &Student::year);
    assert(roster[0].name == "Lindqvist" && roster[1].name == "Tran");
    assert(roster[2].name == "Abara" && roster[3].name == "Okafor");
    // [/std_sort]

    // [partial]
    std::vector<int> times{52, 17, 88, 31, 45, 9, 73, 26};

    // The three smallest, in order, without sorting the rest.
    std::partial_sort(times.begin(), times.begin() + 3, times.end());
    assert(times[0] == 9 && times[1] == 17 && times[2] == 26);

    // The median: put the element that belongs at the middle position
    // there, with smaller elements before it and larger ones after.
    std::nth_element(times.begin(), times.begin() + 4, times.end());
    assert(times[4] == 45);
    // [/partial]
    std::cout << "sort_test: ok\n";
}
