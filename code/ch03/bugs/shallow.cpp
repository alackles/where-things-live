// What goes wrong if a class that owns memory has no copy constructor.
#include <cstddef>

class Naive {
public:
    Naive() : data_(new int[4]{1, 2, 3, 4}) {}
    ~Naive() { delete[] data_; }
    // no copy constructor: the compiler supplies one that copies data_,
    // the POINTER, so two objects end up owning one block
private:
    int* data_;
};

int main() {
    Naive a;
    Naive b = a;   // b.data_ == a.data_
}                  // b's destructor frees the block; then a's frees it again
