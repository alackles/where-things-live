#include <cassert>
#include <iostream>

struct Point {
    double x;
    double y;
};

int main() {
    // [basics]
    int n = 270;
    int* p = &n;     // p holds the address of n
    *p = 271;        // follow the pointer; assign to what it points at
    std::cout << n << '\n';   // prints 271

    int* nobody = nullptr;    // a pointer that points at nothing
    // [/basics]
    assert(n == 271);
    assert(nobody == nullptr);

    // [arrow]
    Point pt{3.0, 4.0};
    Point* pp = &pt;
    (*pp).x = 5.0;   // follow pp, then select member x
    pp->y = 12.0;    // the same thing, spelled the way everyone spells it
    // [/arrow]
    assert(pt.x == 5.0 && pt.y == 12.0);

    // [array]
    int a[5] = {10, 20, 30, 40, 50};   // five ints, side by side
    int* first = a;                    // an array's name converts to
                                       // a pointer to its first element
    assert(*first == 10);
    assert(*(first + 3) == 40);        // three ints further along
    assert(a[3] == *(a + 3));          // indexing IS pointer arithmetic
    // [/array]

    // [walk]
    int total = 0;
    for (int* q = a; q != a + 5; ++q) {   // a + 5 is "one past the end"
        total += *q;
    }
    // [/walk]
    assert(total == 150);
    std::cout << "pointers: ok\n";
}
