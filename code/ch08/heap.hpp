// MinHeap<T>: a binary heap stored in a vector (Chapter 8).
#pragma once
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

template <typename T>
class MinHeap {
public:
    MinHeap() = default;

    // [heapify]
    // Build a heap from an arbitrary vector in linear time: sift down
    // every node that has children, from the last such node back to the root.
    explicit MinHeap(std::vector<T> items) : a_(std::move(items)) {
        for (std::size_t i = a_.size() / 2; i-- > 0;) sift_down(i);
    }
    // [/heapify]

    bool empty() const { return a_.empty(); }
    std::size_t size() const { return a_.size(); }

    // [ops]
    const T& top() const {          // the smallest element
        assert(!a_.empty());
        return a_[0];
    }

    void push(T value) {
        a_.push_back(std::move(value));   // add at the bottom...
        sift_up(a_.size() - 1);           // ...and let it rise
    }

    void pop() {                    // remove the smallest element
        assert(!a_.empty());
        std::swap(a_[0], a_.back());      // last element takes the root's place
        a_.pop_back();
        if (!a_.empty()) sift_down(0);    // ...and sinks to where it belongs
    }
    // [/ops]

    // The heap property: no element is smaller than its parent.
    bool check_invariant() const {
        for (std::size_t i = 1; i < a_.size(); ++i) {
            if (a_[i] < a_[(i - 1) / 2]) return false;
        }
        return true;
    }

private:
    // [sift]
    void sift_up(std::size_t i) {
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;
            if (!(a_[i] < a_[parent])) break;     // parent is no larger: done
            std::swap(a_[i], a_[parent]);
            i = parent;
        }
    }

    void sift_down(std::size_t i) {
        const std::size_t n = a_.size();
        while (true) {
            std::size_t left = 2 * i + 1;
            if (left >= n) break;                 // no children: done
            std::size_t smaller = left;
            std::size_t right = left + 1;
            if (right < n && a_[right] < a_[left]) smaller = right;
            if (!(a_[smaller] < a_[i])) break;    // both children are no smaller
            std::swap(a_[i], a_[smaller]);
            i = smaller;
        }
    }
    // [/sift]

    std::vector<T> a_;   // a_[0] is the root; children of i are 2i+1 and 2i+2
};
