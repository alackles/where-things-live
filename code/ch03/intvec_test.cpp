#include "ch03/intvec.hpp"

#include <cassert>
#include <iostream>
#include <utility>

// [use]
IntVec squares(int n) {
    IntVec result;
    for (int i = 0; i < n; ++i) result.push_back(i * i);
    return result;   // moved (or constructed in place), not copied
}
// [/use]

int main() {
    IntVec v;
    assert(v.empty() && v.capacity() == 0);
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        assert(v.size() <= v.capacity());
    }
    assert(v.size() == 100 && v.capacity() == 128);
    assert(v[0] == 0 && v[99] == 99);
    v[5] = 55;
    v.pop_back();
    assert(v.size() == 99 && v[5] == 55);

    // [copy_demo]
    IntVec a;
    a.push_back(1);
    a.push_back(2);

    IntVec b = a;      // copy constructor
    b.push_back(3);    // a is unaffected: {1, 2}

    IntVec c;
    c = a;             // copy assignment
    c[0] = 100;        // a is still {1, 2}
    // [/copy_demo]
    assert(a.size() == 2 && a[0] == 1);
    assert(b.size() == 3 && b[2] == 3);
    assert(c.size() == 2 && c[0] == 100);

    IntVec& self = a;
    a = self;          // self-assignment must be harmless
    assert(a.size() == 2 && a[1] == 2);

    // [move_demo]
    IntVec d = std::move(b);   // move constructor: d takes b's block
    // [/move_demo]
    assert(d.size() == 3 && d[2] == 3);
    assert(b.size() == 0);     // NOLINT: b is valid but empty after the move

    IntVec e;
    e = squares(10);           // move assignment from a temporary
    assert(e.size() == 10 && e[9] == 81);

    IntVec empty_copy = IntVec();
    IntVec f = empty_copy;
    assert(f.empty());
    std::cout << "intvec_test: ok\n";
}
