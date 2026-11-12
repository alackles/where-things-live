// BTree<K, MAX>: a B-tree set supporting insert and lookup (Chapter 7).
// Each node holds up to MAX keys in sorted order in one contiguous array.
#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>

template <typename K, std::size_t MAX = 31>
class BTree {
    // [node]
    struct Node {
        std::size_t n = 0;                                   // keys in use
        std::array<K, MAX + 1> keys;                         // sorted; 1 spare slot
        std::array<std::unique_ptr<Node>, MAX + 2> kids;     // n + 1 children, or none

        bool leaf() const { return !kids[0]; }
    };
    // [/node]

    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;

    // Index of the first key in node that is >= key (node.n if none).
    static std::size_t position(const Node& node, const K& key) {
        auto first = node.keys.begin();
        auto last = first + static_cast<std::ptrdiff_t>(node.n);
        return static_cast<std::size_t>(std::lower_bound(first, last, key) - first);
    }

public:
    std::size_t size() const { return size_; }

    // [contains]
    bool contains(const K& key) const {
        const Node* node = root_.get();
        while (node != nullptr) {
            std::size_t i = position(*node, key);             // search within the node
            if (i < node->n && !(key < node->keys[i])) return true;
            node = node->kids[i].get();                       // descend between keys
        }
        return false;
    }
    // [/contains]

    // [insert]
    bool insert(K key) {
        if (!root_) root_ = std::make_unique<Node>();
        bool added = insert(*root_, std::move(key));
        if (root_->n > MAX) {                  // the root itself overflowed:
            auto top = std::make_unique<Node>();   // grow a new root above it
            top->kids[0] = std::move(root_);
            split_child(*top, 0);
            root_ = std::move(top);            // the tree is one level taller
        }
        if (added) ++size_;
        return added;
    }
    // [/insert]

    int height() const {
        int h = 0;
        for (const Node* n = root_.get(); n != nullptr; n = n->kids[0].get()) ++h;
        return h;
    }

    template <typename Visit>
    void in_order(Visit visit) const {
        if (root_) in_order(*root_, visit);
    }

    // Keys sorted in every node, nodes not overfull, all leaves at the
    // same depth, and the key count matches.
    bool check_invariant() const {
        if (!root_) return size_ == 0;
        std::size_t count = 0;
        int leaf_depth = -1;
        return check(*root_, 1, leaf_depth, count) && count == size_;
    }

private:
    static bool insert(Node& node, K key) {
        std::size_t i = position(node, key);
        if (i < node.n && !(key < node.keys[i])) return false;   // already here
        if (node.leaf()) {
            for (std::size_t j = node.n; j > i; --j) node.keys[j] = std::move(node.keys[j - 1]);
            node.keys[i] = std::move(key);
            ++node.n;
            return true;
        }
        bool added = insert(*node.kids[i], std::move(key));
        if (node.kids[i]->n > MAX) split_child(node, i);
        return added;
    }

    // [split]
    // Child i of parent is overfull. Split it in two around its middle
    // key, and move that middle key up into parent.
    static void split_child(Node& parent, std::size_t i) {
        Node& left = *parent.kids[i];
        auto right = std::make_unique<Node>();
        std::size_t mid = left.n / 2;

        for (std::size_t j = mid + 1; j < left.n; ++j) {          // upper keys
            right->keys[right->n++] = std::move(left.keys[j]);
        }
        if (!left.leaf()) {
            for (std::size_t j = mid + 1; j <= left.n; ++j) {     // and their children
                right->kids[j - mid - 1] = std::move(left.kids[j]);
            }
        }
        for (std::size_t j = parent.n; j > i; --j) {              // make room above
            parent.keys[j] = std::move(parent.keys[j - 1]);
            parent.kids[j + 1] = std::move(parent.kids[j]);
        }
        parent.keys[i] = std::move(left.keys[mid]);               // middle key rises
        parent.kids[i + 1] = std::move(right);
        ++parent.n;
        left.n = mid;
    }
    // [/split]

    template <typename Visit>
    static void in_order(const Node& node, Visit& visit) {
        for (std::size_t i = 0; i < node.n; ++i) {
            if (!node.leaf()) in_order(*node.kids[i], visit);
            visit(node.keys[i]);
        }
        if (!node.leaf()) in_order(*node.kids[node.n], visit);
    }

    static bool check(const Node& node, int depth, int& leaf_depth, std::size_t& count) {
        if (node.n > MAX) return false;
        for (std::size_t i = 1; i < node.n; ++i) {
            if (!(node.keys[i - 1] < node.keys[i])) return false;
        }
        count += node.n;
        if (node.leaf()) {
            if (leaf_depth < 0) leaf_depth = depth;
            return leaf_depth == depth;
        }
        for (std::size_t i = 0; i <= node.n; ++i) {
            if (!node.kids[i]) return false;
            const Node& kid = *node.kids[i];
            if (kid.n == 0) return false;
            if (i > 0 && !(node.keys[i - 1] < kid.keys[0])) return false;
            if (i < node.n && !(kid.keys[kid.n - 1] < node.keys[i])) return false;
            if (!check(kid, depth + 1, leaf_depth, count)) return false;
        }
        return true;
    }
};
