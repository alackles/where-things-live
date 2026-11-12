#include "ch04/dlist.hpp"
#include "ch04/list.hpp"
#include "ch04/stack_queue.hpp"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <list>
#include <queue>
#include <random>
#include <string>
#include <vector>

template <typename OurList, typename Ref>
bool same(OurList& ours, const Ref& ref) {
    if (!ours.check_invariant() || ours.size() != ref.size()) return false;
    auto r = ref.begin();
    for (auto it = ours.begin(); it != ours.end(); ++it, ++r) {
        if (!(*it == *r)) return false;
    }
    return true;
}

void fuzz_list(unsigned seed) {
    std::mt19937 rng(seed);
    List<int> ours;
    std::deque<int> ref;
    for (int step = 0; step < 300; ++step) {
        int value = static_cast<int>(rng() % 20);
        switch (rng() % 6) {
        case 0: case 1:
            ours.push_front(value);
            ref.push_front(value);
            break;
        case 2:
            if (!ref.empty()) { ours.pop_front(); ref.pop_front(); }
            break;
        case 3: {
            auto it = std::find(ref.begin(), ref.end(), value);
            bool found = it != ref.end();
            if (found) ref.erase(it);
            assert(ours.remove(value) == found);
            break;
        }
        case 4:
            ours.reverse();
            std::reverse(ref.begin(), ref.end());
            break;
        case 5:
            assert(ours.contains(value) ==
                   (std::find(ref.begin(), ref.end(), value) != ref.end()));
            break;
        }
        assert(same(ours, ref));
    }
    List<int> copy = ours;
    assert(same(copy, ref));
    copy.push_front(-1);
    assert(same(ours, ref));
    List<int> moved = std::move(copy);
    assert(moved.size() == ref.size() + 1 && copy.empty());   // NOLINT
    ours = moved;
    assert(ours.size() == ref.size() + 1 && ours.front() == -1);
}

void fuzz_dlist(unsigned seed) {
    std::mt19937 rng(seed);
    DList<int> ours;
    std::list<int> ref;
    for (int step = 0; step < 300; ++step) {
        int value = static_cast<int>(rng() % 100);
        switch (rng() % 7) {
        case 0: ours.push_back(value); ref.push_back(value); break;
        case 1: ours.push_front(value); ref.push_front(value); break;
        case 2: if (!ref.empty()) { ours.pop_back(); ref.pop_back(); } break;
        case 3: if (!ref.empty()) { ours.pop_front(); ref.pop_front(); } break;
        case 4: {   // insert before a random position (possibly end)
            std::size_t k = rng() % (ref.size() + 1);
            auto a = ours.begin();
            auto b = ref.begin();
            for (std::size_t i = 0; i < k; ++i) { ++a; ++b; }
            ours.insert(a, value);
            ref.insert(b, value);
            break;
        }
        case 5:     // erase a random position
            if (!ref.empty()) {
                std::size_t k = rng() % ref.size();
                auto a = ours.begin();
                auto b = ref.begin();
                for (std::size_t i = 0; i < k; ++i) { ++a; ++b; }
                ours.erase(a);
                ref.erase(b);
            }
            break;
        case 6:
            if (!ref.empty()) {
                assert(ours.front() == ref.front() && ours.back() == ref.back());
            }
            break;
        }
        assert(same(ours, ref));
    }
}

void fuzz_queue(unsigned seed) {
    std::mt19937 rng(seed);
    RingQueue<int> ours;
    std::queue<int> ref;
    for (int step = 0; step < 500; ++step) {
        if (rng() % 5 < 3) {
            int value = static_cast<int>(rng() % 1000);
            ours.push(value);
            ref.push(value);
        } else if (!ref.empty()) {
            assert(ours.front() == ref.front());
            ours.pop();
            ref.pop();
        }
        assert(ours.size() == ref.size() && ours.empty() == ref.empty());
    }
    while (!ref.empty()) {
        assert(ours.front() == ref.front());
        ours.pop();
        ref.pop();
    }
    assert(ours.empty());
}

int main() {
    // [use_list]
    List<std::string> todo;
    todo.push_front("sleep");
    todo.push_front("debug");
    todo.push_front("code");          // code -> debug -> sleep

    std::string all;
    for (const std::string& item : todo) all += item + ' ';
    // [/use_list]
    assert(all == "code debug sleep ");

    List<int> sorted;
    for (int x : {5, 1, 4, 1, 3}) sorted.insert_sorted(x);
    std::vector<int> got;
    for (int x : sorted) got.push_back(x);
    assert((got == std::vector<int>{1, 1, 3, 4, 5}));
    assert(sorted.check_invariant());

    Stack<int> st;
    for (int i = 0; i < 10; ++i) st.push(i);
    for (int i = 9; i >= 0; --i) { assert(st.top() == i); st.pop(); }
    assert(st.empty());

    for (unsigned seed = 1; seed <= 100; ++seed) {
        fuzz_list(seed);
        fuzz_dlist(seed);
        fuzz_queue(seed);
    }
    std::cout << "list_test: ok\n";
}
