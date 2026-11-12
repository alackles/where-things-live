// Searching a vector of ints: linear search and binary search.
#pragma once
#include <cstddef>
#include <vector>

// [linear]
// Returns the index of the first element equal to target,
// or v.size() if there is none.
inline std::size_t linear_search(const std::vector<int>& v, int target) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] == target) return i;
    }
    return v.size();
}
// [/linear]

// [binary]
// Precondition: v is sorted in increasing order.
// Returns the index of an element equal to target, or v.size() if none.
inline std::size_t binary_search(const std::vector<int>& v, int target) {
    std::size_t lo = 0;
    std::size_t hi = v.size();
    // Invariant: if target is anywhere in v, it is in v[lo..hi),
    // the half-open range that includes lo and excludes hi.
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (v[mid] == target) {
            return mid;
        } else if (v[mid] < target) {
            lo = mid + 1;   // everything up to and including mid is too small
        } else {
            hi = mid;       // mid and everything after it is too big
        }
    }
    return v.size();        // the range is empty, so target is not here
}
// [/binary]

// [lower_bound]
// Precondition: v is sorted in increasing order.
// Returns the first index i with v[i] >= target, or v.size() if every
// element is smaller. This is where target is, or where it would go.
inline std::size_t lower_bound_index(const std::vector<int>& v, int target) {
    std::size_t lo = 0;
    std::size_t hi = v.size();
    // Invariant: everything before lo is < target;
    //            everything at or after hi is >= target.
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (v[mid] < target) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}
// [/lower_bound]

// Instrumented versions that count comparisons against elements.
inline std::size_t linear_search_count(const std::vector<int>& v, int target,
                                       std::size_t& comparisons) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        ++comparisons;
        if (v[i] == target) return i;
    }
    return v.size();
}

inline std::size_t lower_bound_count(const std::vector<int>& v, int target,
                                     std::size_t& comparisons) {
    std::size_t lo = 0, hi = v.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        ++comparisons;
        if (v[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}
