// List<T>: a singly linked list with raw owning pointers (Chapter 4).
#pragma once
#include <cassert>
#include <cstddef>
#include <utility>

template <typename T>
class List {
    // [node]
    struct Node {
        T value;
        Node* next;   // the following node, or nullptr at the end
    };
    // [/node]

public:
    List() = default;

    // [destructor]
    ~List() { clear(); }

    void clear() {
        while (head_ != nullptr) {
            Node* doomed = head_;
            head_ = head_->next;   // step forward BEFORE deleting
            delete doomed;
        }
        size_ = 0;
    }
    // [/destructor]

    // [copy]
    List(const List& other) {
        List copy;              // owns the new nodes until we are finished
        Node* tail = nullptr;   // last node of the copy so far
        for (Node* p = other.head_; p != nullptr; p = p->next) {
            Node* fresh = new Node{p->value, nullptr};
            if (tail == nullptr) copy.head_ = fresh;
            else tail->next = fresh;
            tail = fresh;
            ++copy.size_;
        }
        std::swap(head_, copy.head_);   // take the finished chain
        std::swap(size_, copy.size_);
    }
    // [/copy]

    List(List&& other) noexcept : head_(other.head_), size_(other.size_) {
        other.head_ = nullptr;
        other.size_ = 0;
    }

    List& operator=(List other) noexcept {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
        return *this;
    }

    bool empty() const { return head_ == nullptr; }
    std::size_t size() const { return size_; }

    // [push_front]
    void push_front(T value) {
        head_ = new Node{std::move(value), head_};
        ++size_;
    }

    void pop_front() {
        assert(head_ != nullptr);
        Node* doomed = head_;
        head_ = head_->next;
        delete doomed;
        --size_;
    }

    T& front() {
        assert(head_ != nullptr);
        return head_->value;
    }
    // [/push_front]

    // [contains]
    bool contains(const T& value) const {
        for (const Node* p = head_; p != nullptr; p = p->next) {
            if (p->value == value) return true;
        }
        return false;
    }
    // [/contains]

    // [remove]
    // Removes the first node holding `value`. Returns whether one was found.
    bool remove(const T& value) {
        Node* prev = nullptr;
        Node* cur = head_;
        while (cur != nullptr && !(cur->value == value)) {
            prev = cur;
            cur = cur->next;
        }
        if (cur == nullptr) return false;            // not found

        if (prev == nullptr) head_ = cur->next;      // removing the first node
        else prev->next = cur->next;                 // route around cur
        delete cur;
        --size_;
        return true;
    }
    // [/remove]

    // [reverse]
    // Reverses the list in place by re-aiming every next pointer.
    void reverse() {
        Node* done = nullptr;      // the already-reversed part
        Node* cur = head_;         // the part still to do
        while (cur != nullptr) {
            Node* rest = cur->next;   // remember where to go next
            cur->next = done;         // flip this node's pointer
            done = cur;
            cur = rest;
        }
        head_ = done;
    }
    // [/reverse]

    // [insert_sorted]
    // Precondition: the list is in increasing order. Inserts value in order.
    void insert_sorted(T value) {
        Node** link = &head_;   // the pointer we might need to change
        while (*link != nullptr && (*link)->value < value) {
            link = &(*link)->next;
        }
        *link = new Node{std::move(value), *link};
        ++size_;
    }
    // [/insert_sorted]

    // [iterator]
    class Iterator {
    public:
        explicit Iterator(Node* node) : node_(node) {}
        T& operator*() const { return node_->value; }
        Iterator& operator++() {     // advance: follow the link
            node_ = node_->next;
            return *this;
        }
        bool operator==(const Iterator& other) const = default;

    private:
        Node* node_;
    };

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
    // [/iterator]

    // The invariant: following next from head_ reaches nullptr after
    // exactly size_ nodes.
    bool check_invariant() const {
        std::size_t count = 0;
        for (const Node* p = head_; p != nullptr; p = p->next) {
            if (++count > size_) return false;
        }
        return count == size_;
    }

private:
    Node* head_ = nullptr;
    std::size_t size_ = 0;
};
