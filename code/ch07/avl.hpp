// AvlTree<K>: a self-balancing binary search tree (Chapter 7).
#pragma once
#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

template <typename K>
class AvlTree {
    // [node]
    struct Node {
        K key;
        int height = 1;                // height of the subtree rooted here
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        explicit Node(K k) : key(std::move(k)) {}
    };
    using Link = std::unique_ptr<Node>;

    static int height(const Link& link) { return link ? link->height : 0; }

    static void update(Node& n) {
        n.height = 1 + std::max(height(n.left), height(n.right));
    }

    // Positive: left subtree is taller. Negative: right subtree is taller.
    static int balance(const Node& n) { return height(n.left) - height(n.right); }
    // [/node]

    Link root_;
    std::size_t size_ = 0;

public:
    AvlTree() = default;

    // Moving takes over the other tree's nodes. The default move would
    // leave other.size_ unchanged, so reset it by hand.
    AvlTree(AvlTree&& other) noexcept
        : root_(std::move(other.root_)), size_(std::exchange(other.size_, 0)) {}

    AvlTree& operator=(AvlTree&& other) noexcept {
        root_ = std::move(other.root_);
        size_ = std::exchange(other.size_, 0);
        return *this;
    }

    // Rotation counter, for the experiments in the text.
    static inline std::size_t rotations = 0;

    std::size_t size() const { return size_; }
    int height() const { return height(root_); }

    bool contains(const K& key) const {
        const Node* n = root_.get();
        while (n != nullptr) {
            if (key < n->key) n = n->left.get();
            else if (n->key < key) n = n->right.get();
            else return true;
        }
        return false;
    }

    bool insert(K key) {
        bool added = insert(root_, std::move(key));
        if (added) ++size_;
        return added;
    }

    bool erase(const K& key) {
        bool removed = erase(root_, key);
        if (removed) --size_;
        return removed;
    }

    template <typename Visit>
    void in_order(Visit visit) const { in_order(root_.get(), visit); }

    bool check_invariant() const {
        std::size_t count = 0;
        return check(root_.get(), nullptr, nullptr, count) >= 0 && count == size_;
    }

private:
    // [rotate]
    // Rotate right around the node that `link` owns: its left child rises
    // to take its place, and it becomes that child's right child.
    static void rotate_right(Link& link) {
        Link pivot = std::move(link->left);       // the child that will rise
        link->left = std::move(pivot->right);     // its inner subtree changes sides
        update(*link);
        pivot->right = std::move(link);           // the old root goes down
        link = std::move(pivot);                  // the pivot takes its place
        update(*link);
        ++rotations;
    }

    static void rotate_left(Link& link) {         // the mirror image
        Link pivot = std::move(link->right);
        link->right = std::move(pivot->left);
        update(*link);
        pivot->left = std::move(link);
        link = std::move(pivot);
        update(*link);
        ++rotations;
    }
    // [/rotate]

    // [rebalance]
    // Restore the AVL invariant at this node, assuming both of its
    // subtrees already satisfy it and their heights differ by at most 2.
    static void rebalance(Link& link) {
        update(*link);
        int b = balance(*link);
        if (b > 1) {                                   // too tall on the left
            if (balance(*link->left) < 0) {
                rotate_left(link->left);               // left-right: straighten first
            }
            rotate_right(link);
        } else if (b < -1) {                           // too tall on the right
            if (balance(*link->right) > 0) {
                rotate_right(link->right);             // right-left: straighten first
            }
            rotate_left(link);
        }
    }
    // [/rebalance]

    // [insert]
    static bool insert(Link& link, K key) {
        if (!link) {
            link = std::make_unique<Node>(std::move(key));
            return true;
        }
        bool added;
        if (key < link->key) added = insert(link->left, std::move(key));
        else if (link->key < key) added = insert(link->right, std::move(key));
        else return false;

        if (added) rebalance(link);   // on the way back up, fix this node
        return added;
    }
    // [/insert]

    // [erase]
    // Detach and return the smallest node of the subtree `link` owns,
    // rebalancing every node on the path back up.
    static Link extract_min(Link& link) {
        if (!link->left) {
            Link smallest = std::move(link);
            link = std::move(smallest->right);
            return smallest;
        }
        Link smallest = extract_min(link->left);
        rebalance(link);
        return smallest;
    }

    static bool erase(Link& link, const K& key) {
        if (!link) return false;
        bool removed;
        if (key < link->key) {
            removed = erase(link->left, key);
        } else if (link->key < key) {
            removed = erase(link->right, key);
        } else {
            removed = true;
            if (!link->left) {
                link = std::move(link->right);
            } else if (!link->right) {
                link = std::move(link->left);
            } else {
                Link successor = extract_min(link->right);
                link->key = std::move(successor->key);
            }
        }
        if (removed && link) rebalance(link);
        return removed;
    }
    // [/erase]

    template <typename Visit>
    static void in_order(const Node* n, Visit& visit) {
        if (n == nullptr) return;
        in_order(n->left.get(), visit);
        visit(n->key);
        in_order(n->right.get(), visit);
    }

    // [check]
    // Returns the height of n's subtree, or -1 if any invariant fails:
    // search-tree order, stored heights, or the balance condition.
    static int check(const Node* n, const K* lo, const K* hi, std::size_t& count) {
        if (n == nullptr) return 0;
        if (lo && !(*lo < n->key)) return -1;
        if (hi && !(n->key < *hi)) return -1;
        int l = check(n->left.get(), lo, &n->key, count);
        int r = check(n->right.get(), &n->key, hi, count);
        if (l < 0 || r < 0) return -1;
        if (l - r > 1 || r - l > 1) return -1;        // out of balance
        if (n->height != 1 + std::max(l, r)) return -1;   // stale height
        ++count;
        return n->height;
    }
    // [/check]
};
