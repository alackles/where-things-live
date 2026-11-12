// LruCache<K, V>: a fixed-capacity cache that evicts the least recently
// used entry. A hash map and a doubly linked list, working together.
#pragma once
#include <cassert>
#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

// [lru]
template <typename K, typename V>
class LruCache {
    using Entry = std::pair<K, V>;
    using Position = typename std::list<Entry>::iterator;

    std::size_t capacity_;
    std::list<Entry> order_;                 // most recently used at the front
    std::unordered_map<K, Position> where_;  // key -> its node in order_

public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {
        assert(capacity > 0);
    }

    // A member-by-member copy would leave the copy's where_ holding
    // iterators into the ORIGINAL's order_. Forbid copying; moving is
    // fine, because a moved list keeps its nodes and so its iterators.
    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;
    LruCache(LruCache&&) = default;
    LruCache& operator=(LruCache&&) = default;

    std::size_t size() const { return order_.size(); }

    // Returns a pointer to key's value, or nullptr if it is not cached.
    // A successful lookup counts as a use.
    V* get(const K& key) {
        auto found = where_.find(key);
        if (found == where_.end()) return nullptr;
        // Move the entry's node to the front of the list. splice relinks
        // the node in O(1); nothing is copied, and the iterator stored
        // in where_ remains valid.
        order_.splice(order_.begin(), order_, found->second);
        return &found->second->second;
    }

    void put(const K& key, V value) {
        if (V* existing = get(key)) {        // already cached: update it
            *existing = std::move(value);
            return;
        }
        if (order_.size() == capacity_) {    // full: evict the entry at the back
            where_.erase(order_.back().first);
            order_.pop_back();
        }
        order_.emplace_front(key, std::move(value));
        where_[key] = order_.begin();
    }
};
// [/lru]
