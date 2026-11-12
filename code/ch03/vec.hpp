// Vec<T>: a simplified std::vector, the finished product of Chapter 3.
//
// Simplification: the block is allocated with new T[capacity], so T must be
// default-constructible and all `capacity` objects exist at all times. The
// real std::vector allocates raw memory and constructs elements one by one.
#pragma once
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

// [header]
template <typename T>
class Vec {
public:
    Vec() = default;

    Vec(std::initializer_list<T> items) {   // allows  Vec<int> v{1, 2, 3};
        Vec built;                          // owns the block while we fill it
        built.reserve(items.size());
        for (const T& item : items) built.push_back(item);
        swap(built);
    }
    // [/header]

    ~Vec() { delete[] data_; }

    // Build the copy in a local Vec and swap: if copying an element
    // throws, the local's destructor frees the half-built block.
    Vec(const Vec& other) {
        Vec built;
        built.reserve(other.size_);
        for (std::size_t i = 0; i < other.size_; ++i) built.data_[i] = other.data_[i];
        built.size_ = other.size_;
        swap(built);
    }

    Vec(Vec&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // [assign]
    // One assignment operator for both copy and move. The parameter is
    // taken by value: the caller's argument is copied (or moved) into
    // `other`, we swap with it, and its destructor cleans up our old block.
    Vec& operator=(Vec other) noexcept {
        swap(other);
        return *this;
    }
    // [/assign]

    void swap(Vec& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    T& operator[](std::size_t i) {
        assert(i < size_);
        return data_[i];
    }
    const T& operator[](std::size_t i) const {
        assert(i < size_);
        return data_[i];
    }

    // [at]
    // Checked access: reports a bad index by throwing an exception.
    T& at(std::size_t i) {
        if (i >= size_) throw std::out_of_range("Vec::at: index out of range");
        return data_[i];
    }
    // [/at]

    // [reserve]
    // Ensure room for at least `wanted` elements. Never shrinks.
    void reserve(std::size_t wanted) {
        if (wanted <= capacity_) return;
        T* bigger = new T[wanted];
        for (std::size_t i = 0; i < size_; ++i) bigger[i] = std::move(data_[i]);
        delete[] data_;
        data_ = bigger;
        capacity_ = wanted;
    }
    // [/reserve]

    // [push_back]
    void push_back(T value) {
        if (size_ == capacity_) reserve(capacity_ == 0 ? 1 : 2 * capacity_);
        data_[size_] = std::move(value);
        ++size_;
    }
    // [/push_back]

    void pop_back() {
        assert(size_ > 0);
        --size_;
    }

    void clear() { size_ = 0; }

    // [insert]
    // Insert value so that it ends up at position `index`.
    // Precondition: index <= size(). Everything from index on moves right.
    void insert(std::size_t index, T value) {
        assert(index <= size_);
        if (size_ == capacity_) reserve(capacity_ == 0 ? 1 : 2 * capacity_);
        for (std::size_t i = size_; i > index; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[index] = std::move(value);
        ++size_;
    }

    // Remove the element at position `index`.
    // Precondition: index < size(). Everything after it moves left.
    void erase(std::size_t index) {
        assert(index < size_);
        for (std::size_t i = index; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        --size_;
    }
    // [/insert]

    // [iterators]
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
    // [/iterators]

    // [invariant]
    // The representation invariant, as executable code.
    bool check_invariant() const {
        if (size_ > capacity_) return false;
        if ((capacity_ == 0) != (data_ == nullptr)) return false;
        return true;
    }
    // [/invariant]

private:
    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};
