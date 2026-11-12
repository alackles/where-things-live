// IntVec: a growable array of ints, built step by step in Chapter 3.
#pragma once
#include <cassert>
#include <cstddef>
#include <utility>

class IntVec {
public:
    // [ctor_dtor]
    IntVec() = default;             // an empty vector: no block yet

    ~IntVec() { delete[] data_; }   // runs automatically when an IntVec dies
    // [/ctor_dtor]

    // [copy]
    // Copy constructor: build a new IntVec as an independent copy of other.
    IntVec(const IntVec& other)
        : data_(other.size_ > 0 ? new int[other.size_] : nullptr),
          size_(other.size_),
          capacity_(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    // Copy assignment: make an existing IntVec into a copy of other.
    IntVec& operator=(const IntVec& other) {
        if (this != &other) {
            IntVec copy(other);   // may throw; *this is untouched so far
            swap(copy);           // take the copy's block, give it our old one
        }                         // copy's destructor frees our old block
        return *this;
    }
    // [/copy]

    // [move]
    // Move constructor: take over other's block; leave other empty.
    IntVec(IntVec&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move assignment: release our block, then take over other's.
    IntVec& operator=(IntVec&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    // [/move]

    void swap(IntVec& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // [access]
    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    int& operator[](std::size_t i) {
        assert(i < size_);
        return data_[i];
    }
    const int& operator[](std::size_t i) const {
        assert(i < size_);
        return data_[i];
    }
    // [/access]

    // [push_back]
    void push_back(int value) {
        if (size_ == capacity_) grow();
        data_[size_] = value;
        ++size_;
    }

    void pop_back() {
        assert(size_ > 0);
        --size_;
    }
    // [/push_back]

private:
    // [grow]
    // Move to a block twice as big.
    void grow() {
        std::size_t new_capacity = (capacity_ == 0) ? 1 : 2 * capacity_;
        int* bigger = new int[new_capacity];
        for (std::size_t i = 0; i < size_; ++i) bigger[i] = data_[i];
        delete[] data_;
        data_ = bigger;
        capacity_ = new_capacity;
    }
    // [/grow]

    // [fields]
    int* data_ = nullptr;        // the heap block (null when capacity_ is 0)
    std::size_t size_ = 0;       // how many elements are in use
    std::size_t capacity_ = 0;   // how many the block has room for
    // [/fields]
};
