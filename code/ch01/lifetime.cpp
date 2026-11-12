#include <cassert>
#include <iostream>

// [frames]
int square(int x) {
    int result = x * x;   // result lives in square's stack frame
    return result;        // its value is copied out; then the frame is gone
}

int sum_of_squares(int a, int b) {
    int s = square(a);
    int t = square(b);
    return s + t;
}
// [/frames]

// [make_array]
// Returns a pointer to n ints on the heap, all set to `fill`.
// The caller now OWNS that memory and must delete[] it.
int* make_array(int n, int fill) {
    int* block = new int[n];
    for (int i = 0; i < n; ++i) block[i] = fill;
    return block;   // the pointer is copied out; the block lives on
}
// [/make_array]

int main() {
    assert(sum_of_squares(3, 4) == 25);

    // [new_delete]
    int* one = new int{42};   // one int on the heap
    *one += 1;
    delete one;               // give it back

    int* many = make_array(1000, 7);
    assert(many[999] == 7);
    delete[] many;            // arrays are returned with delete[]
    // [/new_delete]
    std::cout << "lifetime: ok\n";
}
