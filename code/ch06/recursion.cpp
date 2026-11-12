#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

// [factorial]
std::uint64_t factorial(unsigned n) {
    if (n <= 1) return 1;              // base case: nothing left to multiply
    return n * factorial(n - 1);       // recursive case: a smaller problem
}
// [/factorial]

// [sum]
// Sum of v[lo..hi), by splitting the range in half.
long long sum(const std::vector<int>& v, std::size_t lo, std::size_t hi) {
    if (hi - lo == 0) return 0;                 // empty range
    if (hi - lo == 1) return v[lo];             // one element
    std::size_t mid = lo + (hi - lo) / 2;
    return sum(v, lo, mid) + sum(v, mid, hi);   // two half-size problems
}
// [/sum]

// [power]
// x to the power n, using about log2(n) multiplications.
double power(double x, unsigned n) {
    if (n == 0) return 1.0;
    double half = power(x, n / 2);
    if (n % 2 == 0) return half * half;
    return half * half * x;
}
// [/power]

// [fib]
std::uint64_t calls = 0;

std::uint64_t fib(unsigned n) {
    ++calls;
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);   // two overlapping subproblems
}
// [/fib]

int main() {
    assert(factorial(0) == 1 && factorial(5) == 120 && factorial(20) == 2432902008176640000ull);
    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    assert(sum(v, 0, v.size()) == 31 && sum(v, 2, 2) == 0 && sum(v, 1, 4) == 6);
    assert(power(2.0, 10) == 1024.0 && power(3.0, 0) == 1.0 && power(1.5, 3) == 3.375);
    for (unsigned n : {10u, 20u, 30u}) {
        calls = 0;
        std::uint64_t f = fib(n);
        std::cout << "fib(" << n << ") = " << f << " took " << calls << " calls\n";
    }
}
