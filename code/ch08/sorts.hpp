// Sorting algorithms of Chapter 8. Each sorts into increasing order
// and needs only operator< on the element type.
#pragma once
#include <cstddef>
#include <random>
#include <utility>
#include <vector>

// [heap_sort]
// Sift a[i] down within the max-heap a[0..n).
template <typename T>
void sift_down_max(std::vector<T>& a, std::size_t i, std::size_t n) {
    while (true) {
        std::size_t left = 2 * i + 1;
        if (left >= n) break;
        std::size_t larger = left;
        if (left + 1 < n && a[left] < a[left + 1]) larger = left + 1;
        if (!(a[i] < a[larger])) break;
        std::swap(a[i], a[larger]);
        i = larger;
    }
}

template <typename T>
void heap_sort(std::vector<T>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = n / 2; i-- > 0;) sift_down_max(a, i, n);   // build a max-heap
    for (std::size_t end = n; end > 1; --end) {
        std::swap(a[0], a[end - 1]);        // largest goes to its final place
        sift_down_max(a, 0, end - 1);       // restore the heap on what is left
    }
}
// [/heap_sort]

// [merge_sort]
// Sort a[lo..hi), using scratch[lo..hi) as working space.
template <typename T>
void merge_sort(std::vector<T>& a, std::vector<T>& scratch,
                std::size_t lo, std::size_t hi) {
    if (hi - lo < 2) return;                 // 0 or 1 elements: already sorted
    std::size_t mid = lo + (hi - lo) / 2;
    merge_sort(a, scratch, lo, mid);         // sort the left half
    merge_sort(a, scratch, mid, hi);         // sort the right half

    // Merge the two sorted halves into scratch.
    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (a[j] < a[i]) scratch[k++] = std::move(a[j++]);
        else scratch[k++] = std::move(a[i++]);   // on a tie, take from the left
    }
    while (i < mid) scratch[k++] = std::move(a[i++]);
    while (j < hi) scratch[k++] = std::move(a[j++]);
    for (k = lo; k < hi; ++k) a[k] = std::move(scratch[k]);
}

template <typename T>
void merge_sort(std::vector<T>& a) {
    std::vector<T> scratch(a.size());
    merge_sort(a, scratch, 0, a.size());
}
// [/merge_sort]

// [quick_sort]
// Sort a[lo..hi).
template <typename T>
void quick_sort(std::vector<T>& a, std::size_t lo, std::size_t hi, std::mt19937& rng) {
    if (hi - lo < 2) return;
    T pivot = a[lo + rng() % (hi - lo)];     // a randomly chosen element

    // Partition into three regions. Invariant:
    //   a[lo..lt) < pivot,   a[lt..i) == pivot,   a[gt..hi) > pivot,
    //   and a[i..gt) has not been examined yet.
    std::size_t lt = lo, i = lo, gt = hi;
    while (i < gt) {
        if (a[i] < pivot) std::swap(a[lt++], a[i++]);
        else if (pivot < a[i]) std::swap(a[i], a[--gt]);
        else ++i;
    }
    quick_sort(a, lo, lt, rng);              // everything smaller than the pivot
    quick_sort(a, gt, hi, rng);              // everything larger
}

template <typename T>
void quick_sort(std::vector<T>& a) {
    std::mt19937 rng(std::random_device{}());
    quick_sort(a, 0, a.size(), rng);
}
// [/quick_sort]

// [counting_sort]
// Sort integers known to lie in the range [0, k), without comparing them.
inline void counting_sort(std::vector<int>& a, int k) {
    std::vector<std::size_t> count(static_cast<std::size_t>(k), 0);
    for (int x : a) ++count[static_cast<std::size_t>(x)];     // tally each value
    std::size_t out = 0;
    for (int v = 0; v < k; ++v) {                             // write them back in order
        for (std::size_t c = 0; c < count[static_cast<std::size_t>(v)]; ++c) a[out++] = v;
    }
}
// [/counting_sort]

template <typename T>
void insertion_sort(std::vector<T>& v) {
    for (std::size_t i = 1; i < v.size(); ++i) {
        T x = std::move(v[i]);
        std::size_t j = i;
        while (j > 0 && x < v[j - 1]) {
            v[j] = std::move(v[j - 1]);
            --j;
        }
        v[j] = std::move(x);
    }
}
