// ChainedMap<K, V>: a hash table with separate chaining (Chapter 5).
#pragma once
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

template <typename K, typename V, typename Hash = std::hash<K>>
class ChainedMap {
    // [layout]
    struct Entry {
        K key;
        V value;
    };
    // One chain per bucket. Each chain is a small vector of entries.
    std::vector<std::vector<Entry>> buckets_;
    std::size_t size_ = 0;   // total number of entries
    Hash hash_;
    // [/layout]

    // [bucket_of]
    std::size_t bucket_of(const K& key) const {
        return hash_(key) % buckets_.size();
    }
    // [/bucket_of]

public:
    ChainedMap() : buckets_(8) {}

    std::size_t size() const { return size_; }
    std::size_t bucket_count() const { return buckets_.size(); }
    double load_factor() const {
        return static_cast<double>(size_) / static_cast<double>(buckets_.size());
    }

    // [find]
    // Returns a pointer to the value stored under key, or nullptr.
    V* find(const K& key) {
        for (Entry& e : buckets_[bucket_of(key)]) {
            if (e.key == key) return &e.value;
        }
        return nullptr;
    }
    // [/find]

    bool contains(const K& key) { return find(key) != nullptr; }

    // [put]
    // Associates value with key, replacing any previous value.
    void put(const K& key, V value) {
        if (V* existing = find(key)) {
            *existing = std::move(value);
            return;
        }
        if (size_ + 1 > buckets_.size()) rehash(2 * buckets_.size());
        buckets_[bucket_of(key)].push_back(Entry{key, std::move(value)});
        ++size_;
    }
    // [/put]

    // [subscript]
    // map[key]: a reference to key's value, inserting a default if absent.
    V& operator[](const K& key) {
        if (V* existing = find(key)) return *existing;
        put(key, V{});
        return *find(key);
    }
    // [/subscript]

    // [erase]
    bool erase(const K& key) {
        std::vector<Entry>& chain = buckets_[bucket_of(key)];
        for (std::size_t i = 0; i < chain.size(); ++i) {
            if (chain[i].key == key) {
                chain[i] = std::move(chain.back());   // order within a chain
                chain.pop_back();                     // does not matter
                --size_;
                return true;
            }
        }
        return false;
    }
    // [/erase]

    // Calls f(key, value) for every entry, in no particular order.
    template <typename F>
    void for_each(F f) const {
        for (const auto& chain : buckets_)
            for (const Entry& e : chain) f(e.key, e.value);
    }

    std::size_t longest_chain() const {
        std::size_t longest = 0;
        for (const auto& chain : buckets_)
            if (chain.size() > longest) longest = chain.size();
        return longest;
    }

    // Every entry must be in the bucket its key hashes to, and the
    // chain lengths must add up to size_.
    bool check_invariant() const {
        std::size_t count = 0;
        for (std::size_t b = 0; b < buckets_.size(); ++b) {
            for (const Entry& e : buckets_[b]) {
                if (bucket_of(e.key) != b) return false;
                ++count;
            }
        }
        return count == size_;
    }

private:
    // [rehash]
    // Move every entry into a table with new_count buckets.
    void rehash(std::size_t new_count) {
        std::vector<std::vector<Entry>> old = std::move(buckets_);
        buckets_.assign(new_count, {});
        for (auto& chain : old) {
            for (Entry& e : chain) {
                buckets_[bucket_of(e.key)].push_back(std::move(e));
            }
        }
    }
    // [/rehash]
};
