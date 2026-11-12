# Hashing {#ch:hashing}

::: goals
By the end of this week you should be able to:

- explain how a hash table turns a key into an array index, and why collisions are unavoidable;
- say what makes a hash function good, and recognize a bad one;
- implement a hash table with separate chaining and one with linear probing, including growth and deletion;
- define the load factor and use it to state the expected cost of each operation, and the worst case;
- use `std::unordered_map` and `std::unordered_set` fluently, with your own key types;
- explain what a Bloom filter is for, and what it gives up to achieve it.
:::

You have a million names and need to answer, over and over, the question "is this name on the list?" So far the best tool we have is a sorted vector with binary search: about twenty comparisons per question, and painfully slow insertion.

::: check
Could a lookup take one or two steps, *no matter how many names there are*? Not twenty: one or two. What would have to be true for that to be possible? Think about the one operation from [[ch:memory]] that takes constant time regardless of size.

::: answer
Indexing an array takes constant time. So if we could compute, from the name itself, the array index where that name is stored, we would not have to search at all: compute the index, look there. The whole of this chapter is about making that idea work.
:::
:::

The structure that results, the **hash table**, is the workhorse of modern software. It is behind Python's `dict` and `set`, JavaScript's objects, Java's `HashMap`, the symbol tables in compilers, database indexes, caches, and routers. After the array it is probably the most-used data structure there is. Many textbooks place it after several weeks of trees; this one puts it first, because it is more important in practice and needs nothing you do not already have: an array, a little arithmetic, and the linked chains or the modular wraparound of last week.

## From keys to indices {#sec:hash-idea}

Two abstract data types run through the rest of the book.

A **set** stores a collection of distinct *keys* and answers membership questions: insert a key, remove a key, is this key present?

A **map** (also called a dictionary or associative array) stores *key–value pairs* and looks up the value belonging to a key: a word and its definition, a student ID and a transcript, a variable name and its type. A set is a map with nothing in the value slot, so we will mostly talk about maps.

If the keys happen to be small whole numbers, there is a wonderful implementation: use the key as an array index. To count how many times each byte value occurs in a file, make an array of 256 counters and, for each byte `b`, increment `count[b]`. Insert, look up, and remove are single array accesses. This is called **direct addressing**, and when it applies, nothing beats it.

It rarely applies. Keys are usually strings, or large numbers, or structs, and the set of *possible* keys is astronomically larger than the set of keys actually in use. There are thousands of times more possible twenty-letter names than there are stars in the observable universe. We cannot have an array slot for each.

Hashing rescues the idea in two steps.

1. A **hash function** converts a key of any type into a large integer, the key's *hash value*. For a 64-bit hash that is a number between 0 and about $1.8 \times 10^{19}$.
2. The hash value is reduced to an index into an array of $m$ slots, called **buckets**, most simply by taking the remainder: `index = hash % m`.

To store a pair, compute its key's bucket and put it there. To look up a key, compute its bucket and look there. Neither step involves searching, and neither depends on how many keys the table holds.

### What makes a hash function good {#sec:hash-functions .unnumbered}

A hash function must be **deterministic**: the same key must always produce the same hash value, or we would never find anything again. It follows that keys which compare equal must hash equal. It should be **fast**, since it runs on every operation. And it should **spread the keys out**: the hash values of the keys in use should look as though they were chosen at random, with every bucket equally likely, no matter what patterns the keys themselves contain.

The last requirement is the hard one, and getting it wrong is easy. Here is a hash function for strings that many people invent on their first try.

```{.cpp include="ch05/hash.hpp" region="bad_hash"}
```

And here is one that is used in practice, known as FNV-1a after its inventors Fowler, Noll, and Vo.

```{.cpp include="ch05/hash.hpp" region="fnv1a"}
```

They look similar: both walk the string and fold each character into a running value. The difference is that FNV-1a multiplies by a large prime after each character, which spreads every character's influence upward across the higher bits of the result, and makes the order of the characters matter.

To see what the difference means, hash the 10,000 strings `user0` through `user9999` into a table of 16,384 buckets with each function.

| Hash function | Buckets used | Longest chain of colliding keys | Average comparisons to find a key |
|:--|--:|--:|--:|
| `sum_hash` | 91 | 615 | 203 |
| `fnv1a` | 8,072 | 4 | 1.2 |

With `sum_hash`, ten thousand keys land in just 91 of the sixteen thousand buckets. The sum of the character codes of a short string is a small number, and similar strings have similar sums; `user19` and `user91` have the *same* sum. The table has degenerated into a few long lists. With FNV-1a the keys spread over about half the buckets, which is what randomly thrown keys would do, and almost every lookup finds its key at once.

For integer keys the temptation is to use the integer itself as its hash. Sometimes that is fine. But if the keys follow a pattern, say they are all multiples of 1024, and the table size is a power of two, they will all land in a handful of buckets. A good table therefore scrambles integer hashes first.

```{.cpp include="ch05/hash.hpp" region="mix"}
```

::: warning
Do not invent hash functions. Designing one that spreads real-world keys well is a specialty, and plausible-looking homemade functions like `sum_hash` fail in ways that are invisible until the table is under load. Use the standard library's `std::hash`, or a published function with a name. And if you ask a model for one, **check its constants against an authoritative source**: the two long numbers in `fnv1a` above are part of the definition, and a model reproducing them from memory can get a digit wrong while the surrounding code looks impeccable. A hash function with a wrong constant still compiles and still returns numbers. It just quietly does its job worse.
:::

### Collisions are certain {#sec:collisions .unnumbered}

However good the hash function, two different keys will sometimes land in the same bucket. This is a **collision**. If there are more possible keys than buckets, some bucket must receive more than one; that much is the pigeonhole principle. What is surprising is how *soon* collisions begin.

::: check
A table has 365 buckets and a perfect hash function that sends each key to a uniformly random bucket. How many keys can you insert before a collision becomes more likely than not? Guess before you calculate.

::: answer
Twenty-three. This is the famous *birthday paradox*: in a group of 23 people, the chance that two share a birthday is just over one half. In general, collisions become likely once the number of keys reaches about the *square root* of the number of buckets. A table with a million buckets will probably see its first collision at around twelve hundred keys, when it is barely a tenth of one percent full.
:::
:::

So a hash table cannot be designed to avoid collisions. It must be designed to *handle* them, and the two major families of hash table differ in how. Both rest on the same quantity. If a table holds $n$ entries in $m$ buckets, its **load factor** is

$$\alpha = \frac{n}{m},$$

the average number of entries per bucket. Keeping $\alpha$ small keeps collisions rare, and costs memory. Everything about hash-table performance is a function of $\alpha$.

## Separate chaining {#sec:chaining}

The most direct way to handle collisions is to let each bucket hold more than one entry. In **separate chaining**, each bucket is a small collection, traditionally a linked list and hence a "chain," of all the entries whose keys hash there.

::: {.figure #fig:chaining}
Separate chaining. The array holds one chain per bucket. A key's hash selects a bucket; the key is then searched for, or added, within that bucket's short chain. Here `fox` and `cat` have collided in bucket 2.
:::

```{.cpp include="ch05/chained_map.hpp" region="layout"}
```

Our chains are small `std::vector`s instead of linked lists, for the reasons the last chapter gave: a few entries stored contiguously are cheaper to scan than a few nodes scattered in the heap. Since every member manages itself, the class follows the rule of zero.

Finding a key's bucket is one line.

```{.cpp include="ch05/chained_map.hpp" region="bucket_of"}
```

The class takes its hash function as a template parameter, `Hash`, defaulting to the standard library's `std::hash<K>`. A hash function in C++ is an object that can be called like a function; `hash_(key)` calls it.

Lookup scans the one chain where the key must be, if it is anywhere.

```{.cpp include="ch05/chained_map.hpp" region="find"}
```

`find` returns a *pointer* to the value: the address of the value if the key is present, `nullptr` if it is not. This is a handy use for pointers, as a "reference that might not exist," and it lets the caller both test for presence and reach the value with one lookup.

```{.cpp include="ch05/map_test.cpp" region="use"}
```

Insertion looks for the key first, since a map holds at most one value per key.

```{.cpp include="ch05/chained_map.hpp" region="put"}
```

Removal finds the entry in its chain and deletes it. Order within a chain means nothing, so instead of shifting the chain's later entries down, we overwrite the doomed entry with the last one and shrink the chain by one: a constant-time removal from the middle of a vector, and a trick worth remembering.

```{.cpp include="ch05/chained_map.hpp" region="erase"}
```

### Growing {#sec:rehash .unnumbered}

If the number of buckets were fixed, the chains would lengthen as entries were added, and the table would slow to a crawl. So, like a vector, a hash table grows. When an insertion would push the load factor above a threshold (here 1.0), `put` calls `rehash` to move everything into a table with twice as many buckets.

```{.cpp include="ch05/chained_map.hpp" region="rehash"}
```

Notice that entries cannot simply be copied to the same positions in the bigger table. A key's bucket is `hash % m`, and $m$ has changed, so every entry must be assigned to its bucket afresh. Rehashing costs $\Theta(n)$. But because the table doubles, the argument from [[sec:amortized]] applies without change: the cost averages out to a constant per insertion.

### What it costs {#sec:chaining-cost .unnumbered}

Every operation computes a hash, which takes time proportional to the size of the key and is independent of $n$, and then scans one chain. So everything depends on the length of the chains.

*If the hash function spreads keys uniformly*, the average chain has $\alpha$ entries. An unsuccessful search scans a whole chain, about $\alpha$ comparisons. A successful one finds its key after passing, on average, half of the *other* entries in its chain: about $1 + \alpha/2$ comparisons. With $\alpha$ held below a constant by rehashing, every operation takes **expected constant time**: $\Theta(1)$, independent of $n$.

The word "expected" is doing real work, and so is the "if." In the *worst* case, every key lands in one bucket, the table is a single list, and every operation is $\Theta(n)$. The table in the previous section showed a hash function getting a fair way toward that worst case with entirely innocent keys.

| Operation | Expected | Worst case |
|:--|:--|:--|
| find, insert, erase | $\Theta(1)$ | $\Theta(n)$ |
| insert that triggers a rehash | $\Theta(n)$, but amortized $\Theta(1)$ | $\Theta(n)$ |
| visit every entry | $\Theta(n + m)$ | $\Theta(n + m)$ |

This is a different kind of guarantee from any we have seen. A vector's amortized bound was unconditional. A hash table's constant time is a statement about averages, resting on the assumption that the hash function scatters the keys you actually have. Most of the time the assumption is sound and hash tables are spectacularly fast. [[sec:using-hash]] looks at what happens when someone sets out to violate it on purpose.

## Open addressing {#sec:probing}

Separate chaining stores entries outside the bucket array, in chains that each need their own allocation. The alternative is to store every entry *in the array itself*, at most one per slot. When a key's slot is already taken, we look for another slot according to a fixed rule. This is **open addressing**, and the sequence of slots examined is called the **probe sequence**.

The simplest rule is **linear probing**: if the slot is taken, try the next one, and the next, wrapping around at the end of the array, until an empty slot turns up.

```{.cpp include="ch05/probing_map.hpp" region="layout"}
```

Each slot records whether it is empty, full, or (a third state we will need shortly) deleted. The slot where a key would live if nothing were in its way is its *home*.

```{.cpp include="ch05/probing_map.hpp" region="home"}
```

Keeping the capacity a power of two lets us replace the `%` operator, which is a slow division, with a bitwise AND against `capacity - 1`, which keeps just the low bits. The same trick wraps the index around the end of the array. The catch is that only the low bits of the hash are used, so they had better be well scrambled; that is why `home` passes the hash through `mix` first.

To insert a key, start at its home and walk forward to the first slot that is not full. To find a key, start at its home and walk forward, comparing keys, until you find it or reach an **empty** slot.

```{.cpp include="ch05/probing_map.hpp" region="find"}
```

The stopping rule is the heart of the method. If the key were in the table, it would have been placed in the first free slot at or after its home. So if the search reaches a slot that is empty, the key cannot be further on. [[fig:hashtable]] lets you try both kinds of table.

::: {.viz #fig:hashtable name="hashtable" fig="probing"}
Linear probing. The key 29 hashes to slot 5, which is occupied, as are 6 and 7; it is stored in slot 8, the first free slot after its home. A later search for 29 retraces the same path. In the web edition, insert, find, and erase keys in either a chained or a probing table, and watch the load factor and the probe counts.
:::

### Deletion and tombstones {#sec:tombstones .unnumbered}

Removing an entry from a probing table is subtle, and it is a favorite source of bugs.

::: check
Keys A, B, and C all have home slot 3, and were inserted in that order, so they occupy slots 3, 4, and 5. Now B is erased by marking slot 4 *empty*. What happens on a subsequent search for C?

::: answer
The search starts at slot 3 (A, not a match), moves to slot 4, finds it empty, and stops, reporting that C is not in the table. But C is sitting in slot 5. Emptying slot 4 broke the path that leads to it. [[fig:tombstone]] shows the situation.
:::
:::

::: {.figure #fig:tombstone}
Why erasing needs tombstones. A, B, and C share home slot 3. If erasing B simply empties slot 4 (middle), a search for C stops there and wrongly reports C missing. Marking the slot "deleted" instead (bottom) keeps the probe path intact: searches pass over it, and a later insertion may reuse it.
:::

The fix is the third state. Erasing marks the slot **deleted**, leaving what is called a **tombstone**. A search treats a tombstone as occupied and keeps going. An insertion may reuse it.

```{.cpp include="ch05/probing_map.hpp" region="erase"}
```

Tombstones keep the table correct, at a price: they lengthen probe sequences just as live entries do. A table that has seen many insertions and deletions can be clogged with them even if it holds few live entries. So the load factor that matters counts tombstones as well, and when it gets too high the table is rebuilt, which discards them.

```{.cpp include="ch05/probing_map.hpp" region="put"}
```

```{.cpp include="ch05/probing_map.hpp" region="rebuild"}
```

::: byhand
Take a table of eight slots and the hash function $h(k) = k \bmod 8$. By hand, with linear probing: insert 3, 11, 19, 4, 27; erase 11; search for 19; insert 35; search for 12. Draw the table after every step, marking tombstones, and count the probes for each operation. Then check yourself with the interactive figure in the web edition.

Tracing a probing table is precise, mechanical work in which one slip makes everything after it wrong. It is exactly the kind of step-by-step state tracking at which language models are *least* dependable: ask one to carry out a sequence like this and it will often be right and sometimes confidently lose track of a tombstone. You need to be the one who can do it reliably.
:::

### Clustering, and what it costs {#sec:clustering .unnumbered}

Linear probing has a characteristic weakness. Occupied slots tend to form contiguous runs, called **clusters**, and clusters feed on themselves: a key that hashes anywhere into a cluster is added to its end, making it longer, and so a still bigger target. As the table fills, the clusters merge and the probe sequences lengthen rapidly.

Knuth analyzed this in the early 1960s; he has called it the first nontrivial algorithm he ever analyzed satisfactorily, and credits it with setting the direction of his life's work. For a table with load factor $\alpha$, the expected number of slots examined is approximately

$$\frac{1}{2}\left(1 + \frac{1}{1-\alpha}\right) \text{ for a successful search, and } \frac{1}{2}\left(1 + \frac{1}{(1-\alpha)^2}\right) \text{ for an unsuccessful one}$$

[@knuth1998].

| Load factor $\alpha$ | Probes, key present | Probes, key absent |
|--:|--:|--:|
| 0.25 | 1.2 | 1.4 |
| 0.50 | 1.5 | 2.5 |
| 0.75 | 2.5 | 8.5 |
| 0.90 | 5.5 | 50.5 |
| 0.95 | 10.5 | 200.5 |

Up to about half full, linear probing is excellent. Beyond three-quarters it deteriorates quickly, and near full it is awful. That is why our table rebuilds itself whenever it would become more than half full. (With chaining, by contrast, $\alpha$ can exceed 1 without disaster.) Other probing rules, such as *quadratic probing* and *double hashing*, spread the probes out to reduce clustering, at some cost in locality.

### Which is faster? {#sec:hash-measure .unnumbered}

On paper, the two designs are both expected constant time. On hardware they are not equal, for the reason that has now come up in three chapters running. A probing table is one contiguous array, and a probe sequence is a short walk through adjacent slots: usually a single cache line. A chained table follows a pointer from the bucket array to a separately allocated chain.

Here are measured times for a successful lookup of a random integer key, in nanoseconds, in five structures: a sorted vector with binary search; `std::map`, a balanced tree we will study in [[ch:balance]]; `std::unordered_map`, the standard library's chained hash table; and the two tables from this chapter.

| $n$ | sorted vector | `std::map` | `unordered_map` | `ChainedMap` | `ProbingMap` |
|--:|--:|--:|--:|--:|--:|
| 1,000 | 37 | 35 | 7.3 | 6.4 | 4.5 |
| 10,000 | 47 | 63 | 10 | 6.0 | 3.3 |
| 100,000 | 64 | 155 | 11 | 9.2 | 5.1 |
| 1,000,000 | 100 | 558 | 31 | 21 | 11 |
| 10,000,000 | 183 | 1,025 | 39 | 24 | 14 |

All three hash tables beat both logarithmic structures at every size: by a factor of three to six against the sorted vector, and by a factor that grows from about five to more than twenty-five against the tree. And among the hash tables, the probing table is the fastest at every size, roughly one and a half times as fast as our chained table and, from ten thousand keys up, two to three times as fast as `std::unordered_map`. (Even a hash table slows down as it grows, you will notice. The number of steps is constant, but one cache miss per lookup is unavoidable once the table is larger than the cache, since a good hash function sends each lookup to an unpredictable place. That is the price of scattering.)

::: wild
The fastest general-purpose hash tables in use today are open-addressing designs. Google's "Swiss table," published in its Abseil library as `flat_hash_map` [@kulukundis2017], keeps one byte of metadata per slot in a separate dense array and uses the processor's vector instructions to examine sixteen slots at a time; Rust's standard `HashMap` is a port of the same design, and Python's `dict` is also open addressing. C++'s `std::unordered_map`, by contrast, is in effect required to use chaining by guarantees in its interface (for instance, that a reference to an entry stays valid when the table grows), which is why performance-sensitive C++ code so often replaces it with a third-party table.
:::

## Using hash tables well {#sec:using-hash}

In your own programs you will use the standard library's hash containers far more often than you write one: `std::unordered_map<K, V>` and `std::unordered_set<K>`. Here is the canonical first program, counting how often each word occurs in a text.

```{.cpp include="ch05/wordcount.cpp" region="count"}
```

The expression `count[word]` looks up `word` and returns a reference to its value. If the word is not yet in the map, `operator[]` *inserts it* with a default value (zero, for an `int`) and returns a reference to that. So `++count[word]` does the right thing for both new and repeated words.

::: warning
That convenience has a sharp edge: `map[key]` on a missing key *adds the key*. Writing `if (count["zebra"] > 0)` to test whether a word occurs will silently insert `"zebra"` with a count of zero. To look without inserting, use `count.contains(key)` or `count.find(key)`.
:::

To report the most frequent words we need them in order of frequency, and here we meet the main limitation of hashing.

```{.cpp include="ch05/wordcount.cpp" region="report"}
```

A hash table keeps its entries in no meaningful order; the whole point of a good hash function is to destroy order. Iterating over an `unordered_map` visits the entries in an arbitrary sequence that can change whenever the table grows. If you need the smallest key, or all keys in a range, or the entries in sorted order, a hash table cannot help, and you must sort a copy, as here, or use a structure that maintains order. Those are the subject of the next two chapters.

### Your own keys {#sec:custom-keys .unnumbered}

The standard library knows how to hash numbers, strings, and pointers. To use your own type as a key you must supply two things: an equality test, and a hash function consistent with it, so that equal keys always get equal hashes.

```{.cpp include="ch05/map_test.cpp" region="point"}
```

```{.cpp include="ch05/map_test.cpp" region="unordered"}
```

`operator==` with `= default` asks the compiler for member-by-member comparison. `PointHash` is a small class with a function-call operator, which is what the container's `Hash` parameter expects. It packs the two coordinates into one 64-bit number and scrambles it.

Two cautions. First, the obvious ways to combine fields are bad. Hashing a point as `x + y` or as `x ^ y` (exclusive or) makes `(2, 7)` collide with `(7, 2)`, and the second sends every point on the diagonal to zero. Second, **never modify a key while it is in a hash table**. Its position was computed from its old value. Change it and the entry is stranded in the wrong bucket, where no lookup will ever find it again.

::: llm
Models are good at spotting weak hash functions, because the weaknesses are structural. As a critic:

> Here is a hash function I wrote for a struct. Without rewriting it, name specific pairs of distinct keys that collide, and describe any *patterns* of keys that would pile up in few buckets.

Verify what it says by actually hashing the pairs it names. Then fix the function yourself, preferably by combining standard-library hashes with a mixing step, not by inventing new arithmetic.
:::

### When the keys are chosen by an enemy {#sec:hashdos .unnumbered}

The expected-constant-time guarantee assumes that the keys are not correlated with the hash function. Suppose instead that someone who knows your hash function deliberately sends you thousands of keys that all hash to the same bucket. Every insertion then scans one ever-longer chain, and inserting $n$ keys takes $\Theta(n^2)$ time.

This is a real attack. In 2003 Crosby and Wallach showed that many widely used programs could be made to stall by feeding them such keys [@crosby2003], and in 2011 the same idea was used to show that a single small, carefully constructed web request could occupy a server's processor for minutes. Web frameworks, it turned out, obligingly put every parameter of a request into a hash table. The attack is called **hash flooding**.

The standard defense is to make the hash function unpredictable: mix a random secret, chosen when the program starts, into every hash, using a function designed so that an outsider cannot work out which keys collide. SipHash [@aumasson2012] was created for this purpose and became the default string hash in Python, Ruby, and Rust, among others. One visible consequence: where a container's iteration order follows its hash values, as with Rust's `HashMap` or a Python `set` of strings, that order can differ from one run of a program to the next.

The lesson is broader than hash tables. An *average-case* guarantee is only as good as the assumption behind it, and on the open internet, inputs are not random. They are chosen by whoever most wants your program to fail.

## Bloom filters {#sec:bloom}

We end with a structure that shows how far the idea of hashing can be pushed if you are willing to relax what "correct" means.

Suppose you have a hundred million keys, say every URL known to be malicious, and you want to test membership very fast and in very little memory. A hash set of a hundred million strings would occupy gigabytes. A **Bloom filter** [@bloom1970] can do the job in about 120 megabytes, a bit over one byte per key, *without storing the keys at all*. The price is that it sometimes gives a wrong answer, in one direction only.

- If the filter says a key is **absent**, the key is definitely absent.
- If the filter says a key is **present**, the key is *probably* present. Occasionally it is not: a **false positive**.

The structure is an array of $m$ bits, all initially zero, and $k$ different hash functions. To **add** a key, compute its $k$ hash values and set those $k$ bits to one. To **test** a key, compute the same $k$ positions and look: if any of those bits is zero, the key was certainly never added, because adding it would have set that bit. If all $k$ are one, the key was probably added, or else other keys happened to set all $k$ of its bits between them.

::: {.viz #fig:bloom name="bloom"}
A Bloom filter with 32 bits and three hash functions, after adding two keys. Testing a key checks its three bits. A key whose three bits were all set by *other* keys is reported present although it was never added: a false positive. In the web edition, add keys and test others to see the false-positive rate rise as the filter fills.
:::

```{.cpp include="ch05/bloom.hpp" region="bloom"}
```

The code simulates $k$ independent hash functions by combining two real hash values, a standard technique. Note what is *missing*: there is no way to list the keys, no way to count them, and no way to remove one, since clearing a key's bits might clear bits that other keys depend on.

### How wrong is it? {#sec:bloom-math .unnumbered}

After $n$ keys have been added to a filter of $m$ bits with $k$ hash functions, the probability that a particular bit is still zero is about $e^{-kn/m}$. A false positive needs all $k$ of a key's bits to be one, so the false-positive rate is approximately

$$\left(1 - e^{-kn/m}\right)^k.$$

For a given number of bits per key, $m/n$, the rate is lowest when $k \approx (m/n) \ln 2$, about 0.7 times the bits per key. Here is that formula against a measurement, for a hundred thousand keys tested against a million non-members.

| Bits per key | $k$ | Measured false positives | Predicted |
|--:|--:|--:|--:|
| 4 | 3 | 14.6% | 14.7% |
| 8 | 6 | 2.13% | 2.16% |
| 10 | 7 | 0.81% | 0.82% |
| 16 | 11 | 0.042% | 0.046% |

Ten bits per key gives an error rate under one percent, and each further bit and a half or so halves it again. The key itself might be a hundred bytes long; the filter does not care.

::: wild
Bloom filters appear wherever a cheap "definitely not" can save an expensive operation. The storage engines behind many large databases keep their data in a series of sorted files on disk, and keep a Bloom filter for each file in memory. Before reading a file to look for a key, the engine asks the filter, and most of the time the filter says "not in this one," saving a disk read ([[ch:wild]] returns to this). Web browsers have used them to check URLs against lists of dangerous sites without downloading the lists, and content delivery networks use them to avoid caching objects that are requested only once.
:::

The Bloom filter is the first structure in this book whose answer to *what does it promise?* is deliberately weaker than "the right answer." That is a legitimate thing to promise, provided the weakness is stated exactly and the one-sidedness is used properly: the filter's "no" is trusted absolutely, and its "yes" is treated as "go and check."

::: llm
Tracing hash tables by hand is good practice, and a model can generate exercises, though it should not be trusted to grade them. Try:

> Give me a hash table exercise: a table size, the hash function h(k) = k mod m, and a sequence of about ten inserts, erases, and lookups using linear probing with tombstones. Do not show the answer.

Work it on paper, then check your final table against the interactive figure in this chapter, not against the model. If you do ask the model for its answer and it disagrees with yours, trace the disputed step together; you will sometimes find that the mistake is the model's.
:::

::: {.fourq title="Hash tables"}
Where does it live?
:   An array of buckets, indexed by a hash of the key. With chaining, each bucket leads to a separate small chain. With open addressing, the entries are in the array itself, and a key lives at or shortly after its home slot.

What does it promise?
:   A key is always in the bucket (or on the probe path) that its hash selects. Equal keys have equal hashes. No ordering of any kind. In a probing table, no empty slot lies between a key's home and its actual position, which is why erasure leaves tombstones.

What does it cost?
:   Find, insert, erase: expected $\Theta(1)$ when the hash function spreads keys well and the load factor is bounded; $\Theta(n)$ in the worst case. Growth is amortized. Memory: a constant factor more than the entries need. Probing tables are markedly faster in practice because they stay in one block.

How do you know?
:   Differential testing against `std::unordered_map`, with both a small key range (to force repeats, replacements, and erasures of present keys) and a large one (to force growth); an invariant check that every entry is where its hash says; and a look at the chain-length distribution, which reveals a bad hash function at once.
:::

::: {.fourq title="Bloom filter"}
Where does it live?
:   An array of bits. Each key is represented only by the $k$ bit positions its hashes select. The keys themselves are not stored.

What does it promise?
:   No false negatives, ever. False positives at a predictable rate determined by the bits per key. No deletion, no enumeration.

What does it cost?
:   $\Theta(k)$ per operation, independent of the number of keys. About ten bits per key for a one-percent error rate.

How do you know?
:   Test that every added key is reported present. Measure the false-positive rate on keys known to be absent, and compare it with the formula.
:::

## Exercises {#sec:hashing-ex .unnumbered}

1. A table has 10 buckets and uses $h(k) = k \bmod 10$. Insert the keys 12, 44, 13, 88, 23, 94, 11, 39, 5, in that order, (a) using separate chaining, and (b) using linear probing. Draw both tables. For each, what is the largest number of key comparisons any successful search would need?

2. Continuing part (b): erase 23 and then 94, using tombstones. Now search for 5, and count the slots examined. Then insert 33. Which slot does it go in?

3. Explain what is wrong with each of these as a hash function for strings: (a) the length of the string; (b) the character code of the first character; (c) a random number; (d) the address of the string object in memory.

4. Suppose the keys of a `ChainedMap<int, int>` are all multiples of 1024, and the table uses `std::hash<int>`, which on many systems returns the integer unchanged. Our table always has a power-of-two number of buckets. Predict the chain lengths after inserting the first 1,000 multiples of 1024. Verify with `longest_chain()`. Then fix the problem in two different ways: by changing `bucket_of`, and by changing how the number of buckets is chosen. (Many standard library implementations keep the bucket count prime. Why does that help?)

5. In `ProbingMap`, `put` begins by calling `find`, and only then looks for a free slot. Explain why it would be wrong to insert a new key into the first tombstone encountered without first completing the search for the key.

6. Show that if a probing table never rebuilt itself, a long enough sequence of alternating insertions and erasures, of *distinct* keys, would eventually make every unsuccessful search examine every slot, even though the table never holds more than one live entry.

7. Using the formulas in [[sec:probing]], compute the expected probes for successful and unsuccessful searches at load factors 0.6, 0.8, and 0.99. A colleague proposes saving memory by letting a probing table fill to 95% before growing. Write two sentences in reply.

8. Write a function that determines whether two strings are anagrams of each other in expected $\Theta(n)$ time, using a hash map from characters to counts. Then write a function that, given a vector of words, groups them into sets of mutual anagrams. What should the *key* of your map be?

9. Given a vector of integers and a target sum, determine in expected $\Theta(n)$ time whether any two elements add up to the target. (The brute-force method of [[ch:cost]] is quadratic.)

10. Implement a `HashSet<K>` class with `insert`, `contains`, `erase`, and `size`, using either collision strategy. You may *not* simply wrap a map: store keys only. Test it differentially against `std::unordered_set`.

11. A Bloom filter has $m = 80{,}000$ bits and will hold $n = 10{,}000$ keys. What $k$ minimizes the false-positive rate, and what is the rate? If you could afford only $k = 2$ (hashing is expensive for your keys), what would the rate be?

12. A *counting* Bloom filter replaces each bit with a small counter: adding a key increments its $k$ counters, and removing a key decrements them. Explain why this permits deletion. What new way can it now fail? What does it cost in space?

13. **With a model, as critic.** Write a hash function for a struct with three `int` fields. Before testing it, ask a model to predict which patterns of keys will collide. Then *measure*: hash every triple with fields in the range 0 to 49 (125,000 keys) into 262,144 buckets and report the longest chain and the number of empty buckets, alongside the same figures for `mix` applied to a packed combination of the fields. Were the model's predictions right?

14. **Lab.** Build a spell checker. Load a word list (your system probably has one at `/usr/share/dict/words`) into (a) your own probing hash set, (b) a `std::unordered_set`, (c) a sorted `std::vector` searched with binary search, and (d) a Bloom filter sized for a 1% false-positive rate. For each, measure the time to build it, the time to check every word of a large text, and the memory used. Present the results as a table and state which you would ship in a spell checker for a phone, and why. For the Bloom filter, also report how many misspelled words in your test text it wrongly accepted.

## Chapter notes {#sec:hashing-notes .unnumbered}

Hashing was invented at IBM in the early 1950s; Knuth [@knuth1998] gives the history and the definitive analysis of both chaining and linear probing, including the formulas quoted in this chapter. Cormen et al. [@clrs2022] present the analysis under the "simple uniform hashing" assumption used here, and go on to *universal hashing*, due to Carter and Wegman [@carter1979], in which the hash function is chosen at random from a family so that good expected performance holds for every set of keys, not merely for typical ones. That idea is the theoretical ancestor of the keyed hash functions now used to resist hash-flooding attacks [@crosby2003; @aumasson2012].

The Bloom filter is from a 1970 paper [@bloom1970] that is five pages long and still worth reading. For the engineering of a modern open-addressing table, Kulukundis's talk on the design of the Swiss table [@kulukundis2017] is an excellent hour.
