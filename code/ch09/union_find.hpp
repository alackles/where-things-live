// UnionFind: disjoint sets with union by size and path compression.
#pragma once
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

// [union_find]
class UnionFind {
public:
    // n elements, 0 .. n-1, each initially in a set by itself.
    explicit UnionFind(int n) : parent_(static_cast<std::size_t>(n)),
                                size_(static_cast<std::size_t>(n), 1), sets_(n) {
        std::iota(parent_.begin(), parent_.end(), 0);   // parent_[i] = i
    }

    // The representative ("root") of the set containing x.
    int find(int x) {
        int root = x;
        while (parent(root) != root) root = parent(root);   // climb to the root
        while (parent(x) != root) {                         // then point every node
            int next = parent(x);                           // on the path straight
            parent(x) = root;                               // at it
            x = next;
        }
        return root;
    }

    bool same(int a, int b) { return find(a) == find(b); }

    // Merge the sets containing a and b. Returns false if already together.
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (size(ra) < size(rb)) std::swap(ra, rb);   // ra is the larger tree
        parent(rb) = ra;                              // hang the smaller under it
        size(ra) += size(rb);
        --sets_;
        return true;
    }

    int sets() const { return sets_; }

private:
    int& parent(int x) { return parent_[static_cast<std::size_t>(x)]; }
    int& size(int x) { return size_[static_cast<std::size_t>(x)]; }

    std::vector<int> parent_;   // parent_[x] == x means x is a root
    std::vector<int> size_;     // for a root: how many elements in its tree
    int sets_;
};
// [/union_find]
