#include <cassert>
#include <iostream>
#include <stack>
#include <string>

// [balanced]
// Are the brackets in s properly matched and nested?
bool balanced(const std::string& s) {
    std::stack<char> open;   // the brackets still waiting to be closed
    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') {
            open.push(c);
        } else if (c == ')' || c == ']' || c == '}') {
            if (open.empty()) return false;          // nothing to close
            char o = open.top();
            open.pop();
            bool match = (o == '(' && c == ')') ||
                         (o == '[' && c == ']') ||
                         (o == '{' && c == '}');
            if (!match) return false;                // wrong kind
        }
    }
    return open.empty();                             // anything left unclosed?
}
// [/balanced]

int main() {
    assert(balanced(""));
    assert(balanced("v[i] = f(a, {1, 2});"));
    assert(balanced("([]{})"));
    assert(!balanced("(]"));
    assert(!balanced("(()"));
    assert(!balanced("())"));
    assert(!balanced("([)]"));
    std::cout << "brackets: ok\n";
}
