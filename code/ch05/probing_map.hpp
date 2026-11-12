// ProbingMap<K, V>: a hash table with open addressing and linear probing.
#pragma once
#include "ch05/hash.hpp"

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

template <typename K, typename V, typename Hash = std::hash<K>>
class ProbingMap {
    // [layout]
    enum class State : unsigned char { Empty, Full, Deleted };

    struct Slot {
        K key{};
        V value{};
        State state = State::Empty;
    };

    std::vector<Slot> slots_;   // capacity is always a power of two
    std::size_t size_ = 0;      // slots that are Full
    std::size_t used_ = 0;      // slots that are Full or Deleted
    Hash hash_;
    // [/layout]

    // [home]
    // The slot where key "wants" to live. The capacity is a power of two,
    // so (capacity - 1) is a mask of low bits; mix() makes sure those low
    // bits depend on the whole hash value.
    std::size_t home(const K& key) const {
        return mix(hash_(key)) & (slots_.size() - 1);
    }
    // [/home]

public:
    ProbingMap() : slots_(8) {}

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return slots_.size(); }

    // [find]
    V* find(const K& key) {
        std::size_t i = home(key);
        while (slots_[i].state != State::Empty) {      // an Empty slot ends
            if (slots_[i].state == State::Full && slots_[i].key == key) {
                return &slots_[i].value;
            }
            i = (i + 1) & (slots_.size() - 1);          // next slot, wrapping
        }
        return nullptr;
    }
    // [/find]

    bool contains(const K& key) { return find(key) != nullptr; }

    // [put]
    void put(const K& key, V value) {
        if (V* existing = find(key)) {
            *existing = std::move(value);
            return;
        }
        if (2 * (used_ + 1) > slots_.size()) rebuild();   // keep it half empty
        std::size_t i = home(key);
        while (slots_[i].state == State::Full) {
            i = (i + 1) & (slots_.size() - 1);
        }
        if (slots_[i].state == State::Empty) ++used_;      // else: reuse a tombstone
        slots_[i].key = key;
        slots_[i].value = std::move(value);
        slots_[i].state = State::Full;
        ++size_;
    }
    // [/put]

    // [erase]
    bool erase(const K& key) {
        std::size_t i = home(key);
        while (slots_[i].state != State::Empty) {
            if (slots_[i].state == State::Full && slots_[i].key == key) {
                slots_[i].state = State::Deleted;   // leave a tombstone
                --size_;
                return true;
            }
            i = (i + 1) & (slots_.size() - 1);
        }
        return false;
    }
    // [/erase]

    // Every Full slot must be reachable from its key's home slot without
    // crossing an Empty slot; and the counters must match the slots.
    bool check_invariant() const {
        std::size_t full = 0, used = 0;
        for (std::size_t s = 0; s < slots_.size(); ++s) {
            if (slots_[s].state == State::Empty) continue;
            ++used;
            if (slots_[s].state != State::Full) continue;
            ++full;
            for (std::size_t i = home(slots_[s].key); i != s;
                 i = (i + 1) & (slots_.size() - 1)) {
                if (slots_[i].state == State::Empty) return false;
            }
        }
        return full == size_ && used == used_ && 2 * used_ <= slots_.size();
    }

private:
    // [rebuild]
    // Re-insert every live entry into a fresh table. This drops all
    // tombstones, and doubles the capacity if the live entries need it.
    void rebuild() {
        std::size_t new_capacity = slots_.size();
        if (4 * (size_ + 1) > new_capacity) new_capacity *= 2;
        std::vector<Slot> old = std::move(slots_);
        slots_.assign(new_capacity, Slot{});
        size_ = 0;
        used_ = 0;
        for (Slot& s : old) {
            if (s.state == State::Full) put(s.key, std::move(s.value));
        }
    }
    // [/rebuild]
};
