#include <iostream>

int main() {
    int big = 2147483647;   // the largest int
    big = big + 1;          // signed overflow: undefined behavior
    std::cout << big << '\n';
}
