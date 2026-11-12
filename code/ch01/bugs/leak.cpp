#include <iostream>

int* make_array(int n, int fill) {
    int* block = new int[n];
    for (int i = 0; i < n; ++i) block[i] = fill;
    return block;
}

int main() {
    int* scores = make_array(1000, 7);
    scores = make_array(2000, 9);   // the first block is now unreachable
    std::cout << scores[0] << '\n';
    delete[] scores;
}
