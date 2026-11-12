// Bst<K>: an (unbalanced) binary search tree of keys, owned by unique_ptr.
#pragma once
#include <cstddef>
#include <memory>
#include <queue>
#include <utility>

template <typename K>
class Bst {
    // [node]
    struct Node {
        K key;
        std::unique_ptr<Node> left;    // owns the whole left subtree
        std::unique_ptr<Node> right;   // owns the whole right subtree

        explicit Node(K k) : key(std::move(k)) {}
    };

    using Link = std::unique_ptr<Node>;   // an owning pointer to a subtree

    Link root_;                // null when the tree is empty
    std::size_t size_ = 0;
    // [/node]

public:
    Bst() = default;

    // Moving takes over the other tree's nodes. The default move would
    // leave other.size_ unchanged, so reset it by hand.
    Bst(Bst&& other) noexcept
        : root_(std::move(other.root_)), size_(std::exchange(other.size_, 0)) {}

    Bst& operator=(Bst&& other) noexcept {
        root_ = std::move(other.root_);
        size_ = std::exchange(other.size_, 0);
        return *this;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // [contains]
    bool contains(const K& key) const {
        const Node* n = root_.get();          // a non-owning "observer" pointer
        while (n != nullptr) {
            if (key < n->key) n = n->left.get();
            else if (n->key < key) n = n->right.get();
            else return true;                 // neither is smaller: equal
        }
        return false;
    }
    // [/contains]

    // Returns true if key was added, false if it was already present.
    bool insert(K key) {
        bool added = insert(root_, std::move(key));
        if (added) ++size_;
        return added;
    }

    // Returns true if key was found and removed.
    bool erase(const K& key) {
        bool removed = erase(root_, key);
        if (removed) --size_;
        return removed;
    }

    // [queries]
    // Smallest key in the tree, or nullptr if the tree is empty.
    const K* min() const {
        const Node* n = root_.get();
        if (n == nullptr) return nullptr;
        while (n->left) n = n->left.get();    // keep going left
        return &n->key;
    }

    // Smallest key that is >= key, or nullptr if there is none.
    const K* lower_bound(const K& key) const {
        const K* best = nullptr;
        const Node* n = root_.get();
        while (n != nullptr) {
            if (n->key < key) {
                n = n->right.get();           // too small: answer is to the right
            } else {
                best = &n->key;               // a candidate; look for a smaller one
                n = n->left.get();
            }
        }
        return best;
    }
    // [/queries]

    int height() const { return height(root_.get()); }

    // Calls visit(key) for every key, in increasing order.
    template <typename Visit>
    void in_order(Visit visit) const { in_order(root_.get(), visit); }

    template <typename Visit>
    void pre_order(Visit visit) const { pre_order(root_.get(), visit); }

    // [level_order]
    // Visits keys level by level, top to bottom, left to right.
    template <typename Visit>
    void level_order(Visit visit) const {
        std::queue<const Node*> waiting;
        if (root_) waiting.push(root_.get());
        while (!waiting.empty()) {
            const Node* n = waiting.front();
            waiting.pop();
            visit(n->key);
            if (n->left) waiting.push(n->left.get());
            if (n->right) waiting.push(n->right.get());
        }
    }
    // [/level_order]

    bool check_invariant() const {
        return ordered(root_.get(), nullptr, nullptr) && count(root_.get()) == size_;
    }

    double average_depth() const {
        return size_ == 0 ? 0.0
                          : static_cast<double>(total_depth(root_.get(), 0)) /
                                static_cast<double>(size_);
    }

private:
    // [insert]
    // Insert key into the subtree that `link` owns. `link` is passed by
    // reference: it may be root_, or some node's left or right member.
    static bool insert(Link& link, K key) {
        if (!link) {                                   // an empty spot: put it here
            link = std::make_unique<Node>(std::move(key));
            return true;
        }
        if (key < link->key) return insert(link->left, std::move(key));
        if (link->key < key) return insert(link->right, std::move(key));
        return false;                                  // already present
    }
    // [/insert]

    // [erase]
    static bool erase(Link& link, const K& key) {
        if (!link) return false;                               // not found
        if (key < link->key) return erase(link->left, key);
        if (link->key < key) return erase(link->right, key);

        // link owns the node to remove.
        if (!link->left) {
            link = std::move(link->right);     // no left child: promote the right
        } else if (!link->right) {
            link = std::move(link->left);      // no right child: promote the left
        } else {
            // Two children: find the successor, the leftmost node of the
            // right subtree. Move its key up here, then unlink it.
            Link* succ = &link->right;
            while ((*succ)->left) succ = &(*succ)->left;
            link->key = std::move((*succ)->key);
            *succ = std::move((*succ)->right);
        }
        return true;
    }
    // [/erase]

    // [height]
    // Number of nodes on the longest path from n down to a leaf.
    static int height(const Node* n) {
        if (n == nullptr) return 0;
        int l = height(n->left.get());
        int r = height(n->right.get());
        return 1 + (l > r ? l : r);
    }

    static std::size_t count(const Node* n) {
        if (n == nullptr) return 0;
        return 1 + count(n->left.get()) + count(n->right.get());
    }
    // [/height]

    // [traversals]
    template <typename Visit>
    static void in_order(const Node* n, Visit& visit) {
        if (n == nullptr) return;
        in_order(n->left.get(), visit);    // everything smaller
        visit(n->key);                     // then this node
        in_order(n->right.get(), visit);   // then everything larger
    }

    template <typename Visit>
    static void pre_order(const Node* n, Visit& visit) {
        if (n == nullptr) return;
        visit(n->key);                     // this node first
        pre_order(n->left.get(), visit);
        pre_order(n->right.get(), visit);
    }
    // [/traversals]

    // [ordered]
    // Is every key in n's subtree strictly between *lo and *hi?
    // (A null bound means "no limit on that side".)
    static bool ordered(const Node* n, const K* lo, const K* hi) {
        if (n == nullptr) return true;
        if (lo != nullptr && !(*lo < n->key)) return false;
        if (hi != nullptr && !(n->key < *hi)) return false;
        return ordered(n->left.get(), lo, &n->key) &&     // left: below this key
               ordered(n->right.get(), &n->key, hi);      // right: above it
    }
    // [/ordered]

    static std::size_t total_depth(const Node* n, std::size_t depth) {
        if (n == nullptr) return 0;
        return depth + total_depth(n->left.get(), depth + 1) +
               total_depth(n->right.get(), depth + 1);
    }
};
