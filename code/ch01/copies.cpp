#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// [point]
struct Point {
    double x;
    double y;
};
// [/point]

// [three_ways]
// By value: the function gets its own copy. The caller's vector is safe,
// but every element is copied on every call.
int sum_by_value(std::vector<int> v) {
    int total = 0;
    for (int x : v) total += x;
    return total;
}

// By const reference: no copy, and a promise not to modify.
// This is the default way to pass anything bigger than a few bytes.
int sum(const std::vector<int>& v) {
    int total = 0;
    for (int x : v) total += x;
    return total;
}

// By (non-const) reference: no copy, and the function may modify
// the caller's object. Use it when modification is the point.
void double_all(std::vector<int>& v) {
    for (int& x : v) x *= 2;
}
// [/three_ways]

int main() {
    // [copy]
    std::vector<int> a{1, 2, 3};
    std::vector<int> b = a;   // b is a new vector: a copy of a
    b.push_back(4);

    std::cout << a.size() << ' ' << b.size() << '\n';   // prints 3 4
    // [/copy]
    assert(a.size() == 3 && b.size() == 4);

    // [struct_copy]
    Point p{1.0, 2.0};
    Point q = p;      // copies both doubles
    q.x = 99.0;       // p.x is still 1.0
    // [/struct_copy]
    assert(p.x == 1.0 && q.x == 99.0);

    // [reference]
    int score = 10;
    int& alias = score;   // alias is another name for score
    alias = 11;           // score is now 11
    // [/reference]
    assert(score == 11);

    assert(sum_by_value(a) == 6);
    assert(sum(a) == 6);
    double_all(a);
    assert(sum(a) == 12);
    std::cout << "copies: ok\n";
}
