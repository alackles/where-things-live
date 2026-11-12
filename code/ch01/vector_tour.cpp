#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    // [tour]
    std::vector<int> v;             // empty
    v.push_back(10);                // {10}
    v.push_back(20);                // {10, 20}
    v.push_back(30);                // {10, 20, 30}

    std::cout << v.size() << '\n';  // 3
    std::cout << v[0] << '\n';      // 10: no bounds check
    std::cout << v.at(2) << '\n';   // 30: at() checks and throws if bad
    v[1] = 25;                      // {10, 25, 30}
    v.pop_back();                   // {10, 25}

    std::vector<double> zeros(5, 0.0);        // five 0.0s
    std::vector<std::string> names{"Ada", "Grace", "Barbara"};

    for (const std::string& name : names) {   // read each, no copies
        std::cout << name << ' ';
    }
    std::cout << '\n';

    for (double& z : zeros) {                 // modify each in place
        z = 1.5;
    }
    // [/tour]
    assert(v.size() == 2 && v[1] == 25);
    assert(zeros[4] == 1.5);

    // [strings]
    std::string s = "data";
    s += " structures";               // strings grow, like vectors
    assert(s.size() == 15);
    assert(s[0] == 'd');
    assert(s.substr(5, 6) == "struct");
    assert(s.find("ruct") == 7);
    assert(s < "database");           // comparison is alphabetical
    // [/strings]
    std::cout << "vector_tour: ok\n";
}
