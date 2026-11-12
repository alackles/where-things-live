#include <iostream>

int main() {
    int* p = new int{42};
    delete p;                 // the int is gone...
    std::cout << *p << '\n';  // ...but p still holds its old address
}
