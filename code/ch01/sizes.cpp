#include <iostream>

int main() {
    // [sizes]
    std::cout << "char:      " << sizeof(char) << '\n'
              << "bool:      " << sizeof(bool) << '\n'
              << "int:       " << sizeof(int) << '\n'
              << "long long: " << sizeof(long long) << '\n'
              << "double:    " << sizeof(double) << '\n'
              << "int*:      " << sizeof(int*) << '\n';
    // [/sizes]

    // [addresses]
    int n = 270;
    char c = 'A';
    double d = 2.7;
    std::cout << "n lives at " << &n << '\n'
              << "c lives at " << static_cast<void*>(&c) << '\n'
              << "d lives at " << &d << '\n';
    // [/addresses]
}
