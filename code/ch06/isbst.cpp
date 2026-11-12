// A popular wrong way to check the BST property, and the right way.
#include <cassert>
#include <iostream>

struct Node {
    int key;
    Node* left = nullptr;
    Node* right = nullptr;
};

// [wrong]
// WRONG: checks each node only against its own two children.
bool looks_ordered(const Node* n) {
    if (n == nullptr) return true;
    if (n->left && !(n->left->key < n->key)) return false;
    if (n->right && !(n->key < n->right->key)) return false;
    return looks_ordered(n->left) && looks_ordered(n->right);
}
// [/wrong]

// [right]
// Right: every key must lie within bounds inherited from ALL its ancestors.
bool is_ordered(const Node* n, const int* lo, const int* hi) {
    if (n == nullptr) return true;
    if (lo && !(*lo < n->key)) return false;
    if (hi && !(n->key < *hi)) return false;
    return is_ordered(n->left, lo, &n->key) && is_ordered(n->right, &n->key, hi);
}
// [/right]

int main() {
    //        50
    //       /  \       60 is greater than its parent 30, so the local
    //     30    70     check is satisfied; but it sits in the LEFT
    //    /  \          subtree of 50, where everything must be below 50.
    //  20    60
    Node n20{20}, n60{60}, n70{70};
    Node n30{30, &n20, &n60};
    Node n50{50, &n30, &n70};

    assert(looks_ordered(&n50));                  // fooled
    assert(!is_ordered(&n50, nullptr, nullptr));  // not fooled
    std::cout << "isbst: ok\n";
}
