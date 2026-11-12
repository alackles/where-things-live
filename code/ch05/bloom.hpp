// BloomFilter: approximate set membership in a few bits per element.
#pragma once
#include "ch05/hash.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// [bloom]
class BloomFilter {
public:
    // bits: the size of the bit array. hashes: how many bits each key sets.
    BloomFilter(std::size_t bits, int hashes) : bits_(bits, false), k_(hashes) {}

    void add(const std::string& key) {
        for (int i = 0; i < k_; ++i) bits_[position(key, i)] = true;
    }

    // false: the key was definitely never added.
    // true:  the key was probably added (or its bits were set by others).
    bool might_contain(const std::string& key) const {
        for (int i = 0; i < k_; ++i) {
            if (!bits_[position(key, i)]) return false;
        }
        return true;
    }

private:
    // The i-th hash of key. Two real hash values are combined to simulate
    // as many hash functions as we need:  h1 + i * h2.
    std::size_t position(const std::string& key, int i) const {
        std::uint64_t h1 = fnv1a(key);
        std::uint64_t h2 = mix(h1) | 1;   // odd, so it is never zero
        return (h1 + static_cast<std::uint64_t>(i) * h2) % bits_.size();
    }

    std::vector<bool> bits_;
    int k_;
};
// [/bloom]
