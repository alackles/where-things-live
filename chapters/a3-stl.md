# A Field Guide to the Standard Containers {#app:stl}

This appendix is a quick reference to the containers of the C++ standard library, each of which you have now built or studied. For full details of any of them, consult cppreference.com [@cppreference].

## The containers {#sec:stl-containers}

| Container | Built as | Chapter | Strengths | Weaknesses |
|:--|:--|:-:|:--|:--|
| `vector<T>` | contiguous block that doubles | 3 | indexing; append; iteration; cache behavior | insert or erase in the middle |
| `string` | a vector of characters | 1, 3 | as `vector` | as `vector` |
| `array<T, N>` | fixed-size block, no heap | 1 | no allocation at all | size fixed at compile time |
| `deque<T>` | array of fixed-size blocks | 4 | push and pop at both ends; indexing | slower iteration than `vector` |
| `list<T>` | doubly linked list | 4 | $O(1)$ splice and erase given an iterator; iterators stay valid | no indexing; slow to traverse |
| `forward_list<T>` | singly linked list | 4 | as `list`, smaller | forward traversal only |
| `stack<T>`, `queue<T>` | adapters, over `deque` by default | 4 | restricted, intention-revealing interface | nothing else |
| `priority_queue<T>` | binary heap in a `vector` | 8 | $O(1)$ largest; $O(\log n)$ push and pop | no search; no iteration |
| `unordered_map<K,V>`, `unordered_set<K>` | hash table, chained | 5 | $O(1)$ expected lookup, insert, erase | no order; $O(n)$ worst case |
| `map<K,V>`, `set<K>` | red-black tree | 6, 7 | ordered; range queries; $O(\log n)$ worst case | slower than hashing; pointer-heavy |
| `multimap`, `multiset` | red-black tree | 7 | as above, duplicate keys allowed | as above |

## What each operation costs {#sec:stl-costs}

In the table, $n$ is the number of elements; "am." means amortized and "exp." means expected.

| | `vector` | `deque` | `list` | `unordered_map` | `map` | `priority_queue` |
|:--|:-:|:-:|:-:|:-:|:-:|:-:|
| access by position | $1$ | $1$ | $n$ | | | |
| find by key or value | $n$ | $n$ | $n$ | $1$ exp. | $\log n$ | |
| insert or erase at the back | $1$ am. | $1$ am. | $1$ | | | |
| insert or erase at the front | $n$ | $1$ am. | $1$ | | | |
| insert or erase in the middle | $n$ | $n$ | $1$, given an iterator | | | |
| insert or erase by key | | | | $1$ exp. | $\log n$ | |
| smallest or largest | $n$ | $n$ | $n$ | $n$ | $1$ | $1$ (one end only) |
| push; pop the top | | | | | | $\log n$ |

## When iterators and references stop being valid {#sec:stl-invalidation}

An iterator, pointer, or reference to an element refers to a place in memory. Operations that move elements make it dangle ([[sec:templates]]). These rules are worth knowing by heart.

| Container | Operation | What is invalidated |
|:--|:--|:--|
| `vector`, `string` | `push_back`, `insert`, `reserve` that causes reallocation | everything |
| | `insert` without reallocation | the insertion point and everything after it |
| | `erase` | the erased position and everything after it |
| `deque` | insertion or erasure in the middle | everything |
| | `push_back`, `push_front` | all iterators (but not references to elements) |
| `list`, `forward_list` | insert | nothing |
| | erase | only the erased element |
| `map`, `set` | insert | nothing |
| | erase | only the erased element |
| `unordered_map`, `unordered_set` | insert that causes rehashing | all iterators (but not references to elements) |
| | erase | only the erased element |

The pattern: contiguous containers invalidate freely, because their elements move; node-based containers almost never do, because their elements stay put.

## Idioms {#sec:stl-idioms}

These fragments are from `code/appc/stl_idioms.cpp`, which is compiled and tested with the rest of the book's code.

**Removing elements from a vector.**

```{.cpp include="appc/stl_idioms.cpp" region="erase"}
```

Never erase from a container inside a range-`for` loop over it. Use `std::erase_if`, or loop with an explicit iterator and assign it the return value of `erase`.

**Working with maps.**

```{.cpp include="appc/stl_idioms.cpp" region="maps"}
```

Remember that `m[key]` inserts when the key is missing. Use `find` or `contains` to look without changing the map.

**Algorithms instead of loops.** The header `<algorithm>` holds dozens of functions that work on any iterator range. They are tested, fast, and say what they mean.

```{.cpp include="appc/stl_idioms.cpp" region="algorithms"}
```

Notice that `biggest` holds the *value* of the largest element. An iterator obtained before the call to `sort` would still be valid afterward, since sorting does not reallocate, but it would refer to the same *position*, which now holds a different element. Iterators name places, not values.

**General habits.**

- Pass containers to functions by `const` reference, or by reference if the function is to modify them. Return them by value; the move is cheap.
- Call `reserve` on a vector or hash table when you know roughly how many elements are coming.
- Prefer `v.empty()` to `v.size() == 0`, and never write `v.size() - 1` without being sure `v` is not empty.
- Use `std::size_t` for indices and sizes.
- Let `auto` name iterator types for you: `auto it = m.find(key);`.
