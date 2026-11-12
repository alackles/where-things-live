# Data Structures in the Wild {#ch:wild}

::: goals
By the end of this week you should be able to:

- choose a data structure for a problem by asking what operations it needs and what each candidate promises and costs;
- combine structures, so that each covers for another's weakness, as in an LRU cache;
- implement a trie, and explain when it beats a hash table or a search tree;
- recognize the structures of this course inside real systems: a tokenizer, a database, a version-control system, a vector search engine;
- say what you would check before trusting a data structure you did not write.
:::

Nine weeks ago this book claimed that a data structure is a decision about where things live, together with a set of promises about how you can get at them. You now have a dozen such decisions in hand. This last chapter is about using them: choosing among them, combining them, and recognizing them in the software you depend on every day, including the software that may have helped you study for this course.

::: check
A music service keeps, for each user, the 100 most recently played songs. Playing a song that is already on the list moves it to the top. Playing a new one adds it at the top and, if the list is full, drops the song at the bottom. Both must be fast. Which *single* structure from this book supports "is this song on the list, and where?" and "move this entry to the top" and "drop the bottom entry," all in constant time?

::: answer
None of them. A hash table finds the song in constant time but has no notion of order, so it cannot say which entry is the oldest. A linked list can move an entry to the front and drop the last one in constant time, *given a pointer to the entry*, but finding the entry takes a linear scan. A vector is linear for the move. Each structure is good at half of the job. The solution, in [[sec:lru]], is to use two structures at once and have each do the half it is good at.
:::
:::

## Choosing {#sec:choosing}

When you face a problem that needs a container, resist the urge to pick the one you used last. Ask, in order:

1. **What operations do I need?** Write them down. Lookup by key? By position? Insertion where: at the end, anywhere, in sorted order? Removal? The smallest element? Everything in a range? Iteration in some order?
2. **Which of them are frequent?** A structure that makes the common operation fast and the rare one slow is usually the right trade.
3. **How big will it get?** For twenty elements, nothing matters; use a vector. For twenty million, everything matters.
4. **What guarantees do I need?** Is an average-case bound acceptable, or must every operation be fast? Could an adversary choose the input? Must pointers to elements stay valid while the container changes?

Then consult the table. It summarizes the whole book.

| If you need... | Reach for | Costs to remember |
|:--|:--|:--|
| a sequence; access by position; append | `std::vector` | $\Theta(1)$ index and amortized append; $\Theta(n)$ insert in the middle |
| last in, first out | `std::stack`, or a vector | $\Theta(1)$ |
| first in, first out | `std::queue`, or a ring buffer | $\Theta(1)$ |
| both ends, plus indexing | `std::deque` | $\Theta(1)$ at either end |
| lookup by key, order irrelevant | `std::unordered_map` / `_set` | $\Theta(1)$ expected; $\Theta(n)$ worst |
| lookup by key, *and* order, ranges, nearest key | `std::map` / `std::set` | $\Theta(\log n)$ worst case |
| build once, then look up many times | sorted `std::vector` + binary search | $\Theta(\log n)$; smallest and most cache-friendly |
| repeatedly take the smallest (or largest) | `std::priority_queue` | $\Theta(\log n)$ push and pop |
| membership only, in very little memory, errors tolerable | Bloom filter | $\Theta(k)$; false positives |
| "are these two connected yet?" | union-find | nearly $\Theta(1)$ |
| relationships between pairs of things | adjacency lists | $\Theta(V + E)$ to traverse |
| splice or remove in $O(1)$ given the node; stable addresses | `std::list` | $\Theta(n)$ to find anything |
| keys that are strings, with prefix queries | trie | proportional to key length |

Three pieces of advice go with the table.

**Start with a vector.** It is the right answer more often than everything else combined, and when it is the wrong answer, it is usually an adequate one until you have measurements.

**Then a hash table.** Most problems that a vector does not solve are lookup problems, and for those the hash table is the default. Move to a tree when you find you need order.

**Measure before you get clever.** You have seen, repeatedly, that the structure with the better Big-O is not always the faster one, and that the difference between two structures with the same Big-O can be a factor of ten. Intuition about performance is unreliable, yours and everyone else's. The stopwatch is not.

::: llm
A model is a useful partner for design decisions if you ask it to argue and not to pronounce.

> I need a data structure for the following workload: [describe the operations, how often each happens, and the expected size]. Propose two different designs. For each, give the cost of every operation, the memory overhead, and the strongest argument *against* it. Do not recommend one.

The instruction not to recommend matters. Asked for "the best data structure," a model tends to give a single confident answer, and you learn nothing about the alternatives. Asked for the trade-offs, it gives you what you need to decide for yourself, and then you measure.
:::

## Combining structures: an LRU cache {#sec:lru}

A **cache** keeps the results of expensive operations (a web page fetched over the network, a block read from disk, a database query) so that a repeated request can be answered from memory. Memory is finite, so a cache has a capacity, and when it is full something must be thrown out to make room. A good and very common policy is to evict the entry that was **least recently used**, on the theory that what has not been needed for the longest time is least likely to be needed soon. This is an **LRU cache**, and it is the music-service problem from the start of the chapter.

It needs two things done in constant time.

- *Find an entry by key.* That is a job for a hash table.
- *Keep the entries in order of last use*, moving any entry to the front when it is used and removing the one at the back when space is needed. That is a job for a doubly linked list, provided we can get to an entry's node without searching.

So use both, and connect them. The list holds the entries, in order of use. The hash table maps each key not to the value but to the entry's *position in the list*.

::: {.figure #fig:lru}
An LRU cache of capacity four. The list holds the entries, most recently used first. The hash table maps each key to that key's node in the list, so an entry can be found in constant time and then moved to the front, or removed, in constant time.
:::

```{.cpp include="ch10/lru.hpp" region="lru"}
```

```{.cpp include="ch10/wild_test.cpp" region="lru_use"}
```

Every operation is a hash lookup followed by a constant amount of pointer manipulation: $\Theta(1)$ expected.

This is, at last, a job that a linked list does better than anything else, and it is worth being precise about why. In [[sec:cache]] the list lost to the vector because every operation began with a search, and searching a list is slow. Here the hash table does the searching. The list is asked to do only what it is good at: relink a node, given the node. And the design depends on a promise that only a linked structure makes: a list iterator stays valid when *other* nodes are inserted, removed, or moved. The hash table can hold on to those iterators indefinitely. Positions in a vector would be invalidated by the very first eviction.

The class is built entirely from standard parts and needs no destructor: its members, a `std::list` and a `std::unordered_map`, clean up after themselves. Copying is another matter, and it is a useful warning about the rule of zero. A member-by-member copy would duplicate the list and duplicate the map, but the copied map would hold iterators that still point into the *original's* list. Each member knows how to copy itself; neither knows about the other. So the class declares its copy operations deleted, which makes copying a compile-time error instead of a use-after-free waiting to happen. Whenever one member of a class refers into another, the compiler-generated copy is wrong.

The general lesson is that real problems are often solved by a small *system* of data structures, each compensating for another's weakness: a hash table for finding, with a list for order; a heap for priority, with a hash table for locating entries in it; a sorted array for searching, with a small buffer in front for cheap insertions. When no single structure fits your operations, ask whether two would.

::: wild
LRU caches are everywhere there is something slow to avoid: in operating systems (which pages of memory to keep), in databases (which disk blocks), in web browsers and content delivery networks (which responses), and in processors themselves. The hash-map-plus-linked-list design shown here is the standard one; Python's `functools.lru_cache` and Java's `LinkedHashMap` are both built this way.
:::

## Tries {#sec:trie}

Every structure so far has treated a key as an indivisible thing, to be hashed or compared as a whole. When keys are *strings*, there is another possibility: use the characters of the key, one at a time, to steer through a tree.

A **trie** (the name comes from re*trie*val; most people say "try") is a tree in which each edge is labeled with a character, and each node represents the string spelled out by the path from the root to it [@fredkin1960]. To look up a word, start at the root and follow the edge for its first letter, then the edge for its second, and so on. The word is in the set if the path exists and the node where it ends is marked as the end of a word.

::: {.viz #fig:trie name="trie"}
A trie holding seven words. Each path from the root spells a prefix; double-ringed nodes mark the ends of words. Words with a common prefix share the nodes for it, so "tree", "trie", "trial", and "try" share the path t–r. In the web edition, insert words and type a prefix to see its completions.
:::

```{.cpp include="ch10/trie.hpp" region="node"}
```

Each node has an array of 26 child pointers, one per letter, most of them null. A key's characters are used directly as indices, the direct-addressing idea once more.

```{.cpp include="ch10/trie.hpp" region="walk"}
```

```{.cpp include="ch10/trie.hpp" region="ops"}
```

```{.cpp include="ch10/wild_test.cpp" region="trie_use"}
```

Two properties set tries apart.

**The cost depends on the length of the key, not on the number of keys.** Looking up a word of length $L$ takes $L$ steps whether the trie holds ten words or ten million. A hash table also takes time proportional to $L$, to compute the hash; a balanced tree takes $O(L \log n)$, since each of its $\log n$ comparisons may examine up to $L$ characters.

**Prefixes are first-class.** All the words that begin with a given prefix live in the subtree below that prefix's node. To list them, walk to the node and traverse its subtree. That is `complete` above, and it is how autocompletion works. A hash table cannot do this at all, since hashing scatters words that share a prefix.

The price is memory. Our node holds 26 pointers, 208 bytes, and a trie can have a node for every character of every key. Practical tries economize in various ways: by storing children in a small sorted array or a hash map, or by collapsing chains of single-child nodes into one node labeled with a whole substring, which gives a *radix tree*.

::: wild
Network routers find the destination of every packet with a trie over the bits of its address, looking for the longest matching prefix. The Linux kernel uses radix trees to index cached file contents. And tries are central to how text is fed to language models, which is the first of our case studies.
:::

## Four case studies {#sec:case-studies}

The rest of this chapter looks inside four real systems. Nothing in them is beyond you now. Each is assembled from structures you have built, and in each the interesting part is how the pieces are chosen and combined.

### How a language model reads: tokenizers {#sec:tokenizers .unnumbered}

A language model does not see characters, and it does not see words. It sees **tokens**: integers that index into a fixed vocabulary of tens of thousands to a few hundred thousand text fragments. Before any text reaches the model, a *tokenizer* chops it into fragments from the vocabulary. Common words are single tokens; rarer ones are split into pieces, so that `unbelievably` might become `un`, `believ`, `ably`.

Where does the vocabulary come from? Most current models use a method called **byte-pair encoding**, which began life as a data-compression algorithm [@gage1994] and was adapted for language models in 2016 [@sennrich2016]. It is learned from a large body of text by a strikingly simple loop.

1. Start with every word split into single characters. The vocabulary is the set of characters.
2. Count how often each *pair of adjacent tokens* occurs in the whole corpus.
3. Take the most frequent pair, and merge it into a single new token everywhere it occurs. Add that token to the vocabulary.
4. Repeat from step 2, tens of thousands of times.

```{.cpp include="ch10/bpe.cpp" region="bpe"}
```

Here is the program run on a toy corpus of four words: "low" (occurring 5 times), "lower" (2), "newest" (6), and "widest" (3), with `_` marking the end of each word.

```console
merge 1: e + s -> es   (seen 9 times)
merge 2: es + t -> est   (seen 9 times)
merge 3: est + _ -> est_   (seen 9 times)
merge 4: l + o -> lo   (seen 7 times)
merge 5: lo + w -> low   (seen 7 times)
merge 6: e + w -> ew   (seen 6 times)
merge 7: ew + est_ -> ewest_   (seen 6 times)
merge 8: n + ewest_ -> newest_   (seen 6 times)

the corpus, tokenized:
low _   x5
low e r _   x2
newest_   x6
w i d est_   x3
```

Without being told anything about English, the procedure has discovered that `est` at the end of a word is a unit, and that `low` is one too. Run for fifty thousand merges on billions of words, it produces a vocabulary of common words, word stems, prefixes, and suffixes.

Now look at it as a data structures problem. The pair counts live in a map from pairs to integers: a hash table, in a real implementation. Finding the most frequent pair, over and over, is the job of a priority queue. Our toy version recounts everything after each merge, which is hopelessly slow at scale; production tokenizer trainers update the counts incrementally and keep the pairs in a heap. When a trained byte-pair tokenizer is *used*, it replays its merges on the new text in the order they were learned, which again means looking up pairs in a hash table by their rank. Other families of tokenizer work differently, repeatedly taking the longest vocabulary entry that matches the text at the current position; that is prefix matching, and it is commonly done with a trie. A hash table, a heap, and a trie: three chapters of this book, running every time you send a message to a model.

::: llm
Here is an experiment in the limits of introspection. Ask a model:

> How exactly would your tokenizer split the word "antidisestablishmentarianism"? List the tokens.

Then check the answer with an actual tokenizer or token-counting tool; several model providers publish one. The model's answer is often wrong. A model never sees its own tokenization. It receives a sequence of token numbers and has no direct access to the fragments of text they stand for, so its description of how a word is split is a guess from general knowledge, not an observation. It is a neat illustration of a general rule: a model's account of its own inner workings is a generated text like any other, and deserves the same checking.
:::

### How a database finds a row: B+ trees and LSM trees {#sec:databases .unnumbered}

A database table may hold billions of rows on disk, and a query such as "find the customer with this ID" must not read them all. The table has an **index**: a data structure, also on disk, that maps keys to the locations of rows. For half a century the standard index has been a variant of the B-tree of [[sec:btree]].

In a **B+ tree**, the interior nodes hold only keys and child pointers, and all the actual entries live in the leaves. That makes the interior nodes as small as possible, so that each holds as many keys as possible and the tree is as shallow as possible. A node is one disk page; with several hundred keys per node, four levels reach billions of rows. The leaves are also linked to their neighbors in a chain, like a linked list, so that a range query, "every order placed in March," finds the first matching leaf by descending the tree and then simply walks along the leaves. The upper levels of the tree are small enough to stay in memory, in an LRU cache of pages. A lookup therefore typically costs a single disk read.

B-trees are superb for reading. Writing is harder on them: inserting rows in random key order modifies pages all over the disk. Many modern storage engines, built for workloads with heavy writing, use a different design called a **log-structured merge tree**, or LSM tree [@oneil1996]. Its parts will all be familiar.

- New entries go into a sorted structure *in memory*, usually a skip list or a balanced tree. Writing is fast because the only disk activity is appending a record to the end of a log file, kept so that nothing is lost in a crash.
- When the in-memory structure grows large, it is written to disk in one sequential pass as an immutable **sorted file**.
- To look up a key, check the in-memory structure, then the sorted files from newest to oldest, using binary search within each. To avoid reading files that cannot contain the key, each file has a **Bloom filter** ([[sec:bloom]]) in memory. Most files are ruled out without a disk read.
- In the background, sorted files are combined into larger sorted files, discarding overwritten entries, by exactly the **merge** step of mergesort ([[sec:sorting]]).

A balanced tree, sorted arrays, binary search, Bloom filters, and merging: an LSM tree is practically a review session for this course, and it is what sits beneath many of the largest data stores in the world [@kleppmann2017].

### How Git remembers: hashing and a graph {#sec:git .unnumbered}

Git, the version-control system, stores the entire history of a project, every version of every file, and does so compactly, with a design of real elegance built from two ideas in this book [@chacon2014].

The first is hashing, used in a new way. Git stores everything as *objects*, and the name of an object is **a hash of its contents**. The contents of a file are stored as an object called a *blob*, under a name computed by a cryptographic hash function from the bytes of the file. This is called **content addressing**, and it has an immediate consequence: two files with identical contents have the same name, and so are stored only once, no matter how many directories or versions they appear in.

A directory is stored as a *tree* object: a list of names, each with the hash of the blob or subtree it refers to. A *commit* object holds the hash of the tree for the project's top-level directory at that moment, plus the hash of the previous commit, its parent.

::: {.figure #fig:git}
Two commits in a Git repository. Each commit points to a tree (a directory), which points to blobs (file contents) by their hashes. The second commit changed only `main.cpp`; its tree points to a new blob for that file and to the *same* blob as before for `README`, which is stored once.
:::

Because every object refers to others by the hash of their contents, a change to one file changes that file's hash, which changes the contents, and so the hash, of the directory that lists it, and so on up to the root. Two versions of a project that differ in one file share every object except those on the path from that file to the top. And two directory trees can be compared quickly: if their hashes are equal, they are identical throughout, and there is no need to look inside. A tree whose nodes are identified by hashes of their contents in this way is called a **Merkle tree** [@merkle1988].

The second idea is the graph. Each commit points to its parent, and a merge commit has two parents. The history of a project is therefore a directed acyclic graph of commits. A branch is nothing more than a named pointer to one commit. Finding the common ancestor of two branches, listing the history of a file, deciding what must be sent to another machine: all of these are graph traversals of the kind in [[ch:graphs]], over a DAG whose edges are hashes.

### How a model looks things up: vector search {#sec:vector-search .unnumbered}

Language models are often connected to a store of documents, so that a question can be answered from relevant text and not from the model's memory alone. Finding the relevant text is a search problem of a new kind. Each passage is converted, by a model, into an **embedding**: a list of perhaps a thousand numbers, a point in a space of a thousand dimensions, arranged so that passages with similar meanings are nearby points. A question is converted the same way, and the task is then: among millions or billions of stored points, find the ones **nearest** to the query point.

None of our structures does this. A hash table finds exact matches; "nearby" is meaningless to it. A search tree needs a one-dimensional order, and a thousand-dimensional space has none that helps. The straightforward method is to compute the distance from the query to every stored point: a linear scan. With the embeddings stored contiguously, that is a cache-friendly sweep and is perfectly good for a few hundred thousand points. For a billion it is far too slow.

The most widely used solution takes two ideas from this book and a third from [[sec:bloom]].

::: {.figure #fig:hnsw}
The idea behind a layered proximity graph. Every point appears in the bottom layer, linked to some of its near neighbors. A random subset also appears in each layer above, with longer links. A search starts at the top, hops greedily toward the query, drops down a layer whenever it can get no closer, and finishes with a careful search of the dense bottom layer.
:::

The first idea is a **graph**. Make every stored point a vertex, and connect each to a handful of its nearest neighbors. To search, start at some vertex and repeatedly move to whichever neighbor is closest to the query, until no neighbor is closer: a greedy walk through the graph, examining only a few vertices at each step.

The second is the layering of a **skip list** ([[sec:family]]). A walk through a graph of only short links would take many steps. So a random subset of the points is also placed in a sparser graph "above," with longer links, and a subset of those in a still sparser graph above that. The search begins in the top layer, takes a few long strides toward the right region, and descends layer by layer to the dense bottom layer for the final approach. The lower layers are explored with a best-first search driven by a **priority queue** of candidates. The structure is called a *hierarchical navigable small-world* graph, or HNSW [@malkov2020].

The third idea is the Bloom filter's: **relax the promise**. An HNSW search does not guarantee to find the true nearest neighbors. It finds points that are *very probably* among the nearest, and it examines a tiny fraction of the data to do so. As with the Bloom filter, the trade is stated explicitly (a measured *recall*, such as 95 percent, at a given speed), and for this application an approximate answer is entirely acceptable.

A graph, a skip list's layers, a priority queue, and an approximate promise. The structure was published in 2016. People are still inventing data structures, and they are inventing them out of the parts in this book.

## What we left out, and what to keep {#sec:what-next}

Ten weeks is not long, and this book made choices. Here, briefly, is some of what lies beyond it, so that you will recognize the names.

**Persistent data structures** never modify anything; every "update" produces a new version that shares most of its memory with the old one, as Git's trees do. They are the foundation of functional programming and make it trivial to keep old versions around [@okasaki1998].

**Concurrent data structures** can be used safely by many threads at once. Some use locks; the most intricate, called *lock-free*, use none. This is where linked structures regain ground against arrays, and it is a deep subject.

**Probabilistic sketches** follow the Bloom filter in trading exactness for space. One counts the number of distinct items in a stream of billions using a few kilobytes; another estimates how often each item occurred.

**Spatial structures**, such as k-d trees and R-trees, index points and shapes in two or three dimensions, for maps and graphics. **String structures**, such as suffix arrays, index every position of a long text at once, for search and for genomics.

And beyond data structures lies the study of **algorithms** proper: systematic techniques for designing them (greedy methods, dynamic programming), proofs of their correctness, and the theory of which problems can be solved efficiently at all. That is your next course.

### The four questions, once more {#sec:last-word .unnumbered}

The particular structures in this book will be joined by others over your career, and some will fade. What is meant to last is the habit of asking four questions of every one.

**Where does it live?** Contiguous or linked, one block or many, who owns it. You have seen this question decide performance again and again, in ways that no operation count predicted: the vector against the list, probing against chaining, the B-tree against the binary tree, the heap that needed no pointers.

**What does it promise?** The invariant. Sorted order, heap order, balance, "every key is where its hash says." An invariant is what turns bytes into a data structure, and stating it precisely is most of the work of implementing it. You have also seen structures gain a great deal by promising *less*: a heap that does not sort, a Bloom filter that may err, a search that is only probably right.

**What does it cost?** Worst case, average case, amortized. In steps, and then on a real machine. And always: on *which* inputs? The plain search tree, the naive quicksort, and the unprotected hash table all taught the same lesson, that the dangerous input is not random but patterned, or hostile.

**How do you know?** An invariant you can execute. A differential test against something trusted, on thousands of small random cases. A sanitizer under all of it. A deliberate worst-case input. A stopwatch and a doubling experiment. A mutation to check that the tests can fail.

That last question is the one this book has pressed hardest, for a reason given in its first pages. You are entering a profession in which code is abundant. A model will write you a red-black tree, a lock-free queue, or an LSM tree on request, in seconds, and most of the time it will be right. What is scarce, and what you are now equipped to supply, is the judgment to choose the right structure, the precision to say what it must guarantee, and the skill to find out whether the thing in front of you actually does.

::: byhand
Here is a checklist to keep. Before you rely on a data structure that you did not write yourself, whether it came from a model, a colleague, or the internet:

1. **State its invariant** in a sentence. If you cannot, you do not yet know what it is supposed to do.
2. **Get a checker.** Write a function that verifies the invariant, including any property (balance, load factor) that a performance guarantee rests on, or have one written and read it carefully.
3. **Test it differentially** against a standard container on random operations, with small sizes and a small key range, checking the invariant as you go.
4. **Run it under the sanitizers.**
5. **Feed it the awkward inputs:** empty, one element, all equal, already sorted, reverse sorted, and whatever its particular weakness would be.
6. **Measure its growth** with a doubling experiment, and compare with what was promised.
7. **Read it.** You can, now.

None of the seven steps requires you to have written the code. All of them require you to understand it. That is what this course was for.
:::

## Exercises {#sec:wild-ex .unnumbered}

1. For each workload, choose a structure (or combination) and justify the choice by the costs of the operations involved. (a) A spell checker's dictionary: loaded once, then queried millions of times. (b) The "undo" history of a text editor. (c) A leaderboard showing the top ten scores among a million players, with scores changing constantly. (d) A phone's contact list, searched as the user types a name. (e) Detecting whether any of ten million uploaded files is a duplicate of another. (f) A print server's job list, where urgent jobs may jump ahead.

2. A colleague stores a set of 50 integers in a `std::set` "because lookups are logarithmic." Suggest a simpler structure and predict how the two compare in speed and memory. Then measure.

3. Trace an `LruCache` of capacity 3 through: put A, put B, put C, get A, put D, get B, put E, get A. After each operation give the order of the list, and say which gets fail.

4. Add to `LruCache` a member `bool erase(const K& key)`, and a member that returns the key that would be evicted next without evicting it. Both should be $\Theta(1)$.

5. An **LFU** cache evicts the *least frequently* used entry instead. Explain why keeping the entries in a priority queue ordered by use count makes `get` cost $\Theta(\log n)$, and why a `std::priority_queue` is not sufficient as it stands. (What operation is missing?)

6. Draw the trie containing `car`, `card`, `care`, `cat`, `do`, `dog`, and `dot`. How many nodes does it have? How many would a trie containing only `dog` have?

7. Add `bool erase(const std::string& word)` to `Trie`. A simple version just clears the `is_word` flag. A better version also deletes any nodes that are no longer on the path to any word. Implement the better version, recursively.

8. Our trie node uses 208 bytes for its children. Rewrite `Node` to use a `std::map<char, std::unique_ptr<Node>>` instead. What are the costs of each operation now? Build both versions from a large word list and compare memory use (count nodes and multiply) and lookup time.

9. Run the byte-pair-encoding program by hand on the corpus `aaab` (3 times), `aab` (2 times), and `ab` (4 times), for three merges.

10. Explain how the idea of content addressing lets Git detect that a file has been corrupted on disk. Why must the hash function be *cryptographic* for this to be trustworthy against a deliberate attacker, when the hash functions of [[ch:hashing]] were not?

11. An LSM tree answers a lookup by checking several sorted files, newest first. Suppose there are 20 files and each has a Bloom filter with a 1% false-positive rate. For a key that is in none of the files, what is the expected number of files actually read? What would it be with no Bloom filters?

12. A nearest-neighbor search over one million 1,000-dimensional embeddings, stored as `float`s, is done by linear scan. How much memory do the embeddings occupy? If computing one distance takes 250 nanoseconds, how long does one query take? At what number of queries per second does this stop being acceptable, and what does the approximate method give up to do better?

13. **With a model, as ghostwriter, at last.** Choose a structure this book did not implement: a skip list, a treap, or a red-black tree. Have a model write it, as a set with `insert`, `contains`, and `erase`. Then apply the seven-step checklist from the end of this chapter, in full, and write a one-page report: what you checked, what you found, and whether you would ship it.

14. **Lab.** *A tiny search engine.* Given a directory of text files, build an **inverted index**: a map from each word to the sorted list of documents that contain it. Support queries for a single word, for `A AND B` (intersect two sorted lists, with a merge), and for words beginning with a given prefix (a trie over the vocabulary, or a sorted vector with binary search). Put an LRU cache in front of the query function. Report the structures you chose and why, the time to build the index, and the query time with and without the cache.

## Chapter notes {#sec:wild-notes .unnumbered}

The trie was named by Fredkin [@fredkin1960]; the structure itself had been described a year earlier by René de la Briandais. Byte-pair encoding was introduced as a compression method by Gage [@gage1994] and brought to neural language models by Sennrich, Haddow, and Birch [@sennrich2016], whose paper uses the same four-word example as this chapter. The model architecture that those tokens are fed to is described in the paper that introduced it [@vaswani2017].

For storage engines, B-trees, and LSM trees, Kleppmann's *Designing Data-Intensive Applications* [@kleppmann2017] is the best single source, and a pleasure to read; the LSM tree's original paper is O'Neil et al. [@oneil1996]. The internals of Git are explained in the last chapter of *Pro Git* [@chacon2014], which is free online; hash trees are from Merkle [@merkle1988]. The HNSW graph is from Malkov and Yashunin [@malkov2020]. Okasaki's *Purely Functional Data Structures* [@okasaki1998] is the classic on persistent structures.
