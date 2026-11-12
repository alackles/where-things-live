// Hash functions used in Chapter 5.
#pragma once
#include <cstdint>
#include <string>

// [bad_hash]
// A poor hash: add up the character codes. Anagrams collide, and the
// results for short strings are all crowded into a small range.
inline std::uint64_t sum_hash(const std::string& s) {
    std::uint64_t h = 0;
    for (unsigned char c : s) h += c;
    return h;
}
// [/bad_hash]

// [fnv1a]
// FNV-1a: fold each byte into the running value, then multiply by a
// large prime, which spreads that byte's influence into the higher bits.
inline std::uint64_t fnv1a(const std::string& s) {
    std::uint64_t h = 14695981039346656037ull;   // "offset basis"
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;                   // the FNV prime
    }
    return h;
}
// [/fnv1a]

// [mix]
// Scrambles the bits of an integer, so that keys which differ only in
// their high bits, or follow a regular pattern, still spread out.
// (This is the finalizer of the SplitMix64 generator.)
inline std::uint64_t mix(std::uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ull;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebull;
    x ^= x >> 31;
    return x;
}
// [/mix]
