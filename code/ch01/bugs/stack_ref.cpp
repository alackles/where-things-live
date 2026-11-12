#include <iostream>

int& biggest(int a, int b) {
    int result = (a > b) ? a : b;
    return result;   // bug: a reference to a local that is about to vanish
}

int main() {
    int& r = biggest(3, 4);
    std::cout << r << '\n';
}
