// DList<T>: a doubly linked list with a sentinel node (Chapter 4).
#pragma once
#include <cassert>
#include <cstddef>
#include <utility>

template <typename T>
class DList {
    struct Node {
        T value;
        Node* prev;
        Node* next;
    };

public:
    // [iterator]
    class Iterator {
    public:
        T& operator*() const { return node_->value; }
        Iterator& operator++() { node_ = node_->next; return *this; }
        Iterator& operator--() { node_ = node_->prev; return *this; }
        bool operator==(const Iterator& other) const = default;

    private:
        friend class DList;
        explicit Iterator(Node* node) : node_(node) {}
        Node* node_;
    };

    Iterator begin() { return Iterator(sentinel_->next); }
    Iterator end() { return Iterator(sentinel_); }
    // [/iterator]

    // [ctor]
    // An empty list is a sentinel whose prev and next point to itself.
    DList() : sentinel_(new Node{T{}, nullptr, nullptr}) {
        sentinel_->prev = sentinel_;
        sentinel_->next = sentinel_;
    }

    ~DList() {
        while (!empty()) pop_front();
        delete sentinel_;
    }

    DList(const DList&) = delete;              // this class cannot be copied
    DList& operator=(const DList&) = delete;
    // [/ctor]

    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }

    // [insert_erase]
    // Inserts value just before pos. Returns an iterator to the new node.
    Iterator insert(Iterator pos, T value) {
        Node* after = pos.node_;
        Node* before = after->prev;
        Node* fresh = new Node{std::move(value), before, after};
        before->next = fresh;
        after->prev = fresh;
        ++size_;
        return Iterator(fresh);
    }

    // Removes the node at pos. Returns an iterator to the node after it.
    Iterator erase(Iterator pos) {
        Node* doomed = pos.node_;
        assert(doomed != sentinel_);
        Node* after = doomed->next;
        doomed->prev->next = after;
        after->prev = doomed->prev;
        delete doomed;
        --size_;
        return Iterator(after);
    }
    // [/insert_erase]

    // [ends]
    void push_front(T value) { insert(begin(), std::move(value)); }
    void push_back(T value) { insert(end(), std::move(value)); }
    void pop_front() { erase(begin()); }
    void pop_back() { erase(Iterator(sentinel_->prev)); }
    T& front() { return sentinel_->next->value; }
    T& back() { return sentinel_->prev->value; }
    // [/ends]

    // Every node's neighbours must point back at it, and walking the ring
    // from the sentinel must return to it after exactly size_ nodes.
    bool check_invariant() const {
        std::size_t count = 0;
        for (const Node* p = sentinel_->next; p != sentinel_; p = p->next) {
            if (p->next->prev != p || p->prev->next != p) return false;
            if (++count > size_) return false;
        }
        return count == size_ && sentinel_->next->prev == sentinel_;
    }

private:
    Node* sentinel_;
    std::size_t size_ = 0;
};
