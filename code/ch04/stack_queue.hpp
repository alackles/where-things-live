// Stack<T> and RingQueue<T>: two restricted-access sequences (Chapter 4).
#pragma once
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

// [stack]
// Last in, first out. All the work is done by the vector underneath.
template <typename T>
class Stack {
public:
    void push(T value) { items_.push_back(std::move(value)); }
    void pop() {
        assert(!items_.empty());
        items_.pop_back();
    }
    T& top() {
        assert(!items_.empty());
        return items_.back();
    }
    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<T> items_;
};
// [/stack]

// [ring]
// First in, first out, in a circular buffer.
template <typename T>
class RingQueue {
public:
    void push(T value) {
        if (size_ == buf_.size()) grow();
        buf_[(head_ + size_) % buf_.size()] = std::move(value);
        ++size_;
    }
    void pop() {
        assert(size_ > 0);
        head_ = (head_ + 1) % buf_.size();   // the front moves; nothing shifts
        --size_;
    }
    T& front() {
        assert(size_ > 0);
        return buf_[head_];
    }
    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }

private:
    // Move to a buffer twice as big, unwrapping so the front is at index 0.
    void grow() {
        std::vector<T> bigger(buf_.empty() ? 4 : 2 * buf_.size());
        for (std::size_t i = 0; i < size_; ++i) {
            bigger[i] = std::move(buf_[(head_ + i) % buf_.size()]);
        }
        buf_ = std::move(bigger);
        head_ = 0;
    }

    std::vector<T> buf_;      // the slots; buf_.size() is the capacity
    std::size_t head_ = 0;    // index of the front element
    std::size_t size_ = 0;    // number of elements in the queue
};
// [/ring]
