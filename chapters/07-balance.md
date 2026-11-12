# Balance {#ch:balance}

::: goals
By the end of this week you should be able to:

- perform left and right rotations on a binary search tree, by hand and in code, and explain why they preserve the search-tree invariant;
- state the AVL balance condition, explain why it forces the height to be logarithmic, and rebalance a tree after an insertion or removal;
- describe how red-black trees and B-trees relate to AVL trees, and why B-trees suit modern memory hierarchies;
- use `std::map` and `std::set` for ordered queries, and choose sensibly among a hash table, a balanced tree, and a sorted vector.
:::

The binary search tree of the last chapter does everything we asked: search, insertion, removal, ordered traversal, range queries. Its one flaw is that all its costs are proportional to its height, and its height depends on the order in which the keys happened to arrive. Feed it sorted data and it becomes a linked list.

::: check
Here is a binary search tree built by inserting 1, 2, 3 in order: a chain leaning right, with 1 at the root. Using the same three nodes, draw a different binary search tree of height 2. Which node is the root? Which pointers changed? And could you make the change *without examining or moving any other part of a larger tree* that this chain might be a piece of?

::: answer
Make 2 the root, with 1 as its left child and 3 as its right. Two pointers changed: 1 no longer points to 2, and 2 now points to 1 (and whatever pointed to 1 from above must now point to 2). The change is entirely local. That local rearrangement is called a rotation, and it is the tool that every balanced binary tree is built from.
:::
:::

This chapter is about keeping trees short. The plan is to add a second invariant to the search-tree invariant, one that limits the height, and to repair it after each modification with a small amount of local surgery. A tree that does this is called **self-balancing**. There are many varieties. We will build one completely, the AVL tree, and then survey its relatives, including the one inside `std::map` and the one inside nearly every database.

## Rotations {#sec:rotations}

A **rotation** rearranges a parent and one of its children so that the child becomes the parent, without disturbing the left-to-right order of the keys.

::: {.figure #fig:rotation}
A right rotation at $y$ lifts its left child $x$ into its place; a left rotation at $x$ undoes it. The triangles are whole subtrees, which move with their roots. The in-order sequence, $A\,x\,B\,y\,C$, is the same on both sides.
:::

Look at [[fig:rotation]]. On the left, $y$ is the root of some subtree and $x$ is its left child. $A$ is everything less than $x$; $B$ is everything between $x$ and $y$; $C$ is everything greater than $y$. A **right rotation** at $y$ makes $x$ the root, with $y$ as its right child. Subtree $A$ stays under $x$ and subtree $C$ stays under $y$. Only $B$ has to move: it was $x$'s right subtree and becomes $y$'s left subtree, which is the one place a group of keys between $x$ and $y$ can go.

Two facts make rotations the perfect repair tool.

**A rotation preserves the search-tree invariant.** Read the keys off in order on either side of the figure and you get the same sequence: $A$, $x$, $B$, $y$, $C$.

**A rotation changes heights.** Everything in $A$ moves up one level, and everything in $C$ moves down one. If the left side was too tall, a right rotation shortens it at the expense of the right.

And it is cheap: three links change, regardless of the size of the tree. Here it is in code, for a tree whose nodes are owned by `unique_ptr`s, as in the last chapter.

```{.cpp include="ch07/avl.hpp" region="rotate"}
```

As in `insert` and `erase` for the plain search tree, `link` is a reference to the owning pointer that leads to the subtree: the root pointer, or a `left` or `right` member of the node above. The four moves are a short dance of ownership, and it is worth following them with a finger on the figure. First the pivot, $x$, is detached from $y$, so that for a moment nobody but the local variable owns it. Then $B$ is moved from $x$ to $y$. Then $y$ itself is moved to become $x$'s right child. Finally $x$ is installed in the link from above. At each step every node has exactly one owner. The compiler helps here: a `unique_ptr` cannot be copied, so the code cannot accidentally leave two pointers owning the same node. (It cannot stop you from forgetting to re-attach a subtree, which would silently delete it; the tests are for that.) (The calls to `update` are bookkeeping that the next section explains.)

A **left rotation** is the mirror image, and each undoes the other.

## AVL trees {#sec:avl}

Rotations are the mechanism. Now we need a policy: a precise rule for when a tree is "balanced enough" and where to rotate when it is not. The oldest such rule, and still one of the best, was published in 1962 by Georgy Adelson-Velsky and Evgenii Landis [@adelson1962], after whom the **AVL tree** is named.

> **The AVL invariant.** For every node, the heights of its left and right subtrees differ by at most one.

An AVL tree is a binary search tree that also satisfies this. Note that the condition is imposed at *every* node, not just the root, and that it does not demand perfection. A node's two subtrees may differ in height by one, and those small allowances can accumulate, so an AVL tree can be somewhat lopsided. What it cannot be is badly lopsided.

To maintain the invariant cheaply, each node stores the height of its own subtree.

```{.cpp include="ch07/avl.hpp" region="node"}
```

The difference between the two subtree heights is called the node's **balance factor**. The AVL invariant says that every balance factor is $-1$, $0$, or $+1$.

### Why the height is logarithmic {#sec:avl-height .unnumbered}

How tall can an AVL tree with $n$ nodes be? It is easier to turn the question around: what is the *smallest* number of nodes an AVL tree of height $h$ can have? Call it $N(h)$.

A tree of height 1 is a single node: $N(1) = 1$. A tree of height 2 needs a root and at least one child: $N(2) = 2$. For a taller tree, the root has one subtree of height $h - 1$ (or the tree would not have height $h$). To use as few nodes as possible, make the other subtree as short as the invariant allows, which is $h - 2$, and make both subtrees as sparse as possible in turn. So

$$N(h) = 1 + N(h-1) + N(h-2).$$

| $h$ | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 20 |
|:--|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| $N(h)$ | 1 | 2 | 4 | 7 | 12 | 20 | 33 | 54 | 88 | 143 | 17,710 |

Each entry is the sum of the two before it, plus one. That is nearly the rule for the Fibonacci numbers, and like them, $N(h)$ grows exponentially: each step multiplies it by about 1.618, the golden ratio. An AVL tree of height $h$ therefore has *at least* an exponential number of nodes, which, turned back around, says that a tree with $n$ nodes has height at most logarithmic in $n$. Working out the constants gives

$$h < 1.45 \log_2 (n + 2).$$

An AVL tree is never more than about 45 percent taller than a perfectly balanced tree with the same number of nodes [@knuth1998]. A million keys: height at most 28. A billion: at most 42. And this is a guarantee, for every sequence of insertions and removals, with no assumption about the input.

### Restoring balance after an insertion {#sec:avl-insert .unnumbered}

Inserting into an AVL tree starts exactly like inserting into any binary search tree: walk down, and attach a new leaf. The new leaf may have made some subtrees one taller. The only nodes whose subtrees changed are the new leaf's ancestors, the nodes on the path back to the root. So we retrace that path upward, and at each node we update its stored height and check its balance factor. If it has become $+2$ or $-2$, we fix it on the spot.

Suppose a node $z$ has become too tall on the left, with balance factor $+2$. Let $y$ be its left child. There are two cases, depending on which of $y$'s subtrees is the tall one, and [[fig:avl-cases]] shows them.

::: {.figure #fig:avl-cases}
The two ways a node can become left-heavy, and their repairs. Top: the extra height is on the *outside* (the left child's left subtree); one right rotation fixes it. Bottom: the extra height is on the *inside* (the left child's right subtree); a left rotation at the child first converts it to the outside case, and then a right rotation finishes the job.
:::

**The outside case.** The extra height is in $y$'s *left* subtree: the path from $z$ goes left, then left. A single right rotation at $z$ lifts $y$ and its tall left subtree up a level, and lowers $z$. The subtree is balanced.

**The inside case.** The extra height is in $y$'s *right* subtree: the path goes left, then right, in a zig-zag. A right rotation at $z$ alone does not help; it would move the tall inner subtree across to hang under $z$, leaving things just as lopsided in the other direction. Instead we first rotate *left* at $y$. That straightens the zig-zag into the outside case. Then a right rotation at $z$ finishes, as before. Two rotations, often called a **double rotation**.

A node that is too tall on the *right* is handled by the mirror images. That makes four cases in all, traditionally named left-left, left-right, right-right, and right-left after the direction of the two steps from the unbalanced node toward the extra height. All four are handled by one short function.

```{.cpp include="ch07/avl.hpp" region="rebalance"}
```

The function reads off the cases directly. If the node is left-heavy by two, look at its left child. If that child leans *right* (the inside case), rotate the child left first. Then rotate the node right.

Insertion calls `rebalance` on each node along the path, on the way back up from the recursion.

```{.cpp include="ch07/avl.hpp" region="insert"}
```

Compare this with the plain `Bst::insert` of [[sec:bst]]. The descent is identical. The only addition is the call to `rebalance` after the recursive call returns, which is to say, as the recursion unwinds through the ancestors of the new node from the bottom up. This is a place where recursion earns its keep: "do something at every node on the path back to the root" is exactly what the unwinding call stack does, for free.

::: check
Insert the keys 10, 20, 30, 40, 50, 25, in that order, into an empty AVL tree. After which insertions is a rotation needed, and which case is each?

::: answer
Inserting 30 unbalances 10 (right-right): rotate left at 10, giving 20 at the root. Inserting 50 unbalances 30 (right-right): rotate left at 30, so 40 has children 30 and 50. Inserting 25 unbalances the root 20, whose right subtree (rooted at 40) is now two taller than its left, and the extra height is on the *inside*, under 40's left child 30. That is the right-left case: rotate right at 40, then left at 20. The final tree has 30 at the root, with children 20 (over 10 and 25) and 40 (over 50). [[fig:avl]] shows the last step.
:::
:::

::: {.viz #fig:avl name="bst" fig="avl" mode="avl"}
An AVL tree after inserting 10, 20, 30, 40, 50, and then 25. Each node shows its balance factor. The last insertion made the root's balance factor $-2$ with the extra height on the inside, and was repaired by a double rotation. In the web edition, insert and remove keys and watch each rotation; try inserting sorted keys, the input that ruined the plain search tree.
:::

::: byhand
Insert the keys 1 through 10, in order, into an empty AVL tree, on paper. Draw the tree after every insertion, write the balance factor beside every node, and name the case for every rotation. Then remove 1, 2, and 3. Use the interactive figure to check each step *after* you have drawn it.

Rotations are hard to get right from a verbal description and easy once your hands know them. The four cases will blur together until you have drawn each a few times; after that they are obvious, and you will be able to reconstruct the code from the pictures instead of memorizing it.
:::

### Removal {#sec:avl-erase .unnumbered}

Removal follows the same pattern. Remove the node as in a plain search tree (the three cases of [[sec:bst-erase]]), then walk back up, rebalancing every node on the path.

```{.cpp include="ch07/avl.hpp" region="erase"}
```

There is one difference from insertion that the code hides. After an insertion, at most one node needs rotating: the repair restores the subtree to its original height, so nothing above it is affected. After a removal, fixing one node can leave its subtree one shorter than before, which may unbalance its parent, and so on up; removal can require a rotation at every level. The code does not care. It calls `rebalance` at every node on the path regardless, and each call does constant work.

In the two-child case the successor is detached by `extract_min`, which rebalances along *its* path as well, since removing the successor may have unbalanced the nodes above it.

### What it costs, and how you know {#sec:avl-cost .unnumbered}

Every operation walks one root-to-leaf path and back, doing constant work per node, and the height is at most $1.45 \log_2 n$.

| Operation | Plain BST | AVL tree |
|:--|:--|:--|
| search, insert, remove | $\Theta(h)$; $\Theta(n)$ worst case | $\Theta(\log n)$ worst case |
| minimum, lower bound | $\Theta(h)$ | $\Theta(\log n)$ |
| in-order traversal | $\Theta(n)$ | $\Theta(n)$ |

Here is what the height looks like in practice, for keys inserted in sorted order (the input that drove the plain tree to height $n$) and in random order, along with the average number of rotations per insertion.

| $n$ | $\log_2 n$ | sorted: height | sorted: rotations per insert | random: height | random: rotations per insert |
|--:|--:|--:|--:|--:|--:|
| 1,000 | 10.0 | 10 | 0.99 | 12 | 0.70 |
| 10,000 | 13.3 | 14 | 1.00 | 16 | 0.67 |
| 100,000 | 16.6 | 17 | 1.00 | 20 | 0.70 |
| 1,000,000 | 19.9 | 20 | 1.00 | 24 | 0.70 |

On sorted input the AVL tree is essentially perfectly balanced. On random input it is within a few levels of perfect. Compare the random-order height of 24 for a million keys with the 53 that the plain tree managed. And the balancing is cheap: on average less than one rotation per insertion.

Now the fourth question, which for balanced trees has a twist. The test for this chapter runs a differential test against `std::set`, as usual. But suppose we made a mistake in `rebalance`, or forgot to call it at all. The tree would still be a valid binary search tree. Every search would still return the right answer. **A differential test, which checks only answers, would pass.** The tree would simply be slow on certain inputs, and nothing would say so.

A balanced tree makes a promise about *cost*, and a promise about cost is invisible to a test of *results*. To check it we must look inside.

```{.cpp include="ch07/avl.hpp" region="check"}
```

This one recursive function verifies three things at every node: that the keys are in search-tree order (using the bounds technique of [[sec:shape]]); that the stored height is correct; and that the two subtree heights differ by at most one. The test calls it after *every* operation. It also builds a tree from 100,000 sorted keys and asserts that its height is 17.

::: warning
Whenever a data structure's value lies in a performance guarantee, test the property that delivers the guarantee, not just the outputs. For a balanced tree that means checking balance. For a hash table it means looking at chain lengths, as we did in [[ch:hashing]]. This matters doubly for code you did not write. A balanced tree with broken rebalancing, whether it came from a colleague, a library, or a model, passes every functional test you can devise, and then falls over in production on the first large sorted input.
:::

::: llm
Models are good at producing test inputs with a specified property, and the AVL cases make a nice exercise in checking them.

> Give me a sequence of exactly seven distinct integer keys which, inserted in order into an empty AVL tree, causes exactly one double rotation and no single rotations. Don't explain where the rotation happens.

Insert the sequence in the interactive figure, or in your own implementation with the rotation counter, and see whether it does what was claimed. Requests with words like "exactly one" are precisely where a model's answer needs checking: the sequence it gives will look plausible whether or not it is right.
:::

## The family of balanced trees {#sec:family}

AVL trees are one answer to the shape problem. There are several others, all offering the same $O(\log n)$ operations through different invariants. You should know that they exist and what distinguishes them. You will rarely need to implement one.

**Red-black trees** color each node red or black and constrain the coloring: a red node has no red child, and every path from the root down to an empty subtree passes through the same number of black nodes [@guibas1978]. Together those rules guarantee that no path is more than twice as long as any other, so the height is at most $2 \log_2 (n+1)$. That is a looser bound than the AVL tree's, so searches can be slightly slower. In exchange, updates do less restructuring: an insertion needs at most two rotations and a removal at most three, with the rest of the repair done by recoloring. The red-black tree is the structure inside `std::map` and `std::set` in all the major C++ standard libraries, and inside Java's `TreeMap`. Its insertion algorithm is manageable; its removal algorithm is famously intricate.

**Treaps** and **skip lists** get balance from randomness instead of from rules. A treap gives every node a random priority and keeps the tree in heap order ([[ch:heaps]]) by priority, which makes it shaped like a search tree built in random order, whatever the actual order of insertion [@seidel1996]. A skip list is not a tree at all but a stack of linked lists, each skipping over more elements than the one below, with each element's height chosen by coin flips [@pugh1990]. Both are much simpler to implement than red-black trees and are logarithmic *in expectation*.

**Splay trees** keep no balance information whatever. Each time a key is accessed, it is rotated all the way to the root. Any single operation may be slow, but every sequence of operations is fast on average, an amortized $O(\log n)$ bound, and recently used keys are found especially quickly [@sleator1985].

All of these are binary, and they all share a weakness that has nothing to do with their height.

## B-trees {#sec:btree}

A balanced binary tree with a million keys is about twenty levels deep. A search visits twenty nodes. Each node is a separate heap allocation, somewhere in memory. You know from [[sec:cache]] what that means: twenty pointers chased, and for a large tree, close to twenty cache misses. The comparisons are few. The memory accesses are what hurt.

The step count cannot be improved: $\log_2 n$ comparisons are necessary to find one key among $n$ by comparing. But nothing says each comparison has to be a separate trip to memory.

::: check
Suppose each node of a search tree held not one key but fifteen, in sorted order, with sixteen children: one for each gap between keys, plus the two ends. How many levels would a tree of a million keys need? How would you search within a node?

::: answer
Each level multiplies the number of reachable keys by sixteen, so the height is about $\log_{16} n$: five levels for a million keys, in place of twenty. Within a node, the keys are a small sorted array, so use binary search (or, for an array this small, even a linear scan). Those comparisons all touch one contiguous block, one or two cache lines, and so cost very little.
:::
:::

That is the **B-tree** [@bayer1972]. It is a search tree with fat nodes.

- Every node holds up to some maximum number of keys, kept in sorted order in an array inside the node.
- A node with $k$ keys that is not a leaf has $k + 1$ children. The keys act as dividers: the first child holds everything less than the first key, the second child everything between the first and second keys, and so on.
- **All leaves are at the same depth.** A B-tree is always perfectly balanced in height.

::: {.viz #fig:btree name="btree"}
A B-tree in which each node holds at most three keys. The keys in a node divide the key space; each child holds the keys that fall in one gap. All leaves are at the same depth. In the web edition, insert keys and watch full nodes split and the tree grow at the root.
:::

```{.cpp include="ch07/btree.hpp" region="node"}
```

Searching descends from the root. At each node, search within the node's array for the key; if it is there, done; if not, the search has found the gap where the key would be, and descends into the corresponding child.

```{.cpp include="ch07/btree.hpp" region="contains"}
```

The same total number of comparisons is performed as in a binary tree, about $\log_2 n$. The difference is that they are grouped: four or five comparisons within one node, all in the same small block of memory, before moving to the next node. A B-tree is a tree of sorted arrays, and it gets the best of both structures: a tree's cheap updates, and an array's locality.

### Insertion: growing from the top {#sec:btree-insert .unnumbered}

How does a B-tree stay perfectly balanced? The answer is one of the most elegant ideas in data structures, and it reverses your expectation of how trees grow.

New keys are always inserted into a leaf, in sorted position within the leaf's array. If the leaf has room, that is all. If the leaf is now overfull, it **splits**: its lower half stays, its upper half moves to a new sibling node, and its *middle key moves up* into the parent, to serve as the divider between the two halves.

::: {.figure #fig:btree-split}
Splitting an overfull leaf. Inserting 95 gives the rightmost leaf four keys, one more than allowed. The node splits around its middle key, 90, which moves up into the parent as the divider between the two new leaves.
:::

The parent has gained a key. If *it* is now overfull, it splits in the same way, pushing a key up to its own parent. The splitting can propagate all the way to the root. When the root itself splits, a new root is created above it, holding the single key that was pushed up.

```{.cpp include="ch07/btree.hpp" region="split"}
```

```{.cpp include="ch07/btree.hpp" region="insert"}
```

That last step is the only way a B-tree ever gets taller, and it adds one level to *every* path at once. A binary search tree grows downward at its leaves, unevenly, which is where its balance problem comes from. A B-tree grows upward at its root, so all leaves stay at the same depth automatically. No rotations, no balance factors, no colors.

(The B-tree in this chapter's code supports insertion and search. Removal uses the reverse operations, borrowing a key from a sibling or merging two nodes, and is left for the exercises to describe.)

### Measurements {#sec:btree-measure .unnumbered}

Here are times in nanoseconds per operation for random integer keys, for the standard library's red-black tree (`std::set`, abbreviated `set` in the table), this chapter's AVL tree, and this chapter's B-tree with up to 31 keys per node. A sorted vector with binary search is included for lookups, as the contiguous baseline.

| $n$ | insert: `set` | insert: AVL | insert: B-tree | find: vector | find: `set` | find: AVL | find: B-tree |
|-------:|-----:|-----:|-----:|-----:|-----:|-----:|-----:|
| 10,000 | 59 | 92 | 60 | 47 | 60 | 54 | 58 |
| 100,000 | 91 | 156 | 82 | 64 | 147 | 121 | 89 |
| 1,000,000 | 232 | 386 | 121 | 103 | 473 | 329 | 138 |
| 10,000,000 | 849 | 1,023 | 250 | 190 | 1,050 | 821 | 305 |

While everything fits in the cache, the structures are within a factor of two of one another. Once the data outgrows it, all of them slow down, but the two binary trees slow by a factor of eleven to eighteen and the B-tree by about five: at ten million keys it finds a key more than three times as fast as `std::set`, and inserts more than three times as fast too. The AVL tree, being more tightly balanced than the red-black tree, is a little faster to search and a little slower to update, as the theory predicts. The B-tree is within a factor of two of the sorted vector for lookups, while supporting fast insertion, which the vector does not.

::: wild
B-trees were invented at Boeing in 1970 for data on disk, where the gap between "in the same block" and "in a different block" is not a factor of a hundred, as with the cache, but a factor of a hundred thousand or more [@bayer1972; @comer1979]. With nodes the size of a disk page, each holding hundreds of keys, a B-tree four levels deep can index billions of records, and any one of them can be found with three or four disk reads. Essentially every relational database and most file systems are built on B-trees or their close variant the B+ tree, which [[ch:wild]] describes.

The same idea is now used in memory, for the cache-related reasons measured above. Rust's standard ordered map, `BTreeMap`, is a B-tree; so is Google's `btree_map` in the Abseil library for C++. When a modern system needs an ordered map and cares about speed, it is increasingly likely to be a B-tree and not a binary tree.
:::

## Ordered maps in practice {#sec:ordered-maps}

In your own programs you will use the standard library's balanced trees: `std::map<K, V>` and `std::set<K>`. Their interface is nearly the same as that of `unordered_map` and `unordered_set`, with the addition of everything that depends on order.

```{.cpp include="ch07/avl_test.cpp" region="std_map"}
```

The loop `for (const auto& [name, year] : born)` visits the entries in increasing order of key; the square brackets unpack each key–value pair into two named variables. `begin()` is the smallest entry. `lower_bound(k)` returns an iterator to the first entry whose key is not less than `k`, and from an iterator you can step forward or backward through the keys in order. A range query is a walk from one `lower_bound` to another.

By default the keys are ordered with `<`. To order them differently, or to use a key type that has no `<`, you supply a comparison function.

::: warning
A comparison given to `std::map`, `std::set`, or `std::sort` must behave like "less than": it must return false when comparing a value with itself, and if it says `a` is before `b` it must not also say `b` is before `a`. A comparison written with `<=` breaks the first rule. The container uses "neither is less than the other" as its test for equality, as our own `contains` does, so with `<=` no key is ever equal to itself, and the tree's behavior is undefined. Always use a strict comparison.
:::

Which associative container should you use? Here are the three candidates side by side.

| | Hash table | Balanced tree | Sorted vector |
|:--|:--|:--|:--|
| Standard type | `unordered_map` | `map` | `vector` + `lower_bound` |
| Find | $\Theta(1)$ expected | $\Theta(\log n)$ | $\Theta(\log n)$ |
| Insert, remove | $\Theta(1)$ expected | $\Theta(\log n)$ | $\Theta(n)$ |
| Worst case | $\Theta(n)$ | $\Theta(\log n)$ | as above |
| Smallest, largest, next, range | no | yes | yes |
| Iteration order | arbitrary | sorted | sorted |
| Needs from the key type | hash and `==` | `<` | `<` |
| Memory per entry | moderate | high (three pointers) | minimal |
| Cache behavior | fair | poor | excellent |

A reasonable policy follows from the table.

- If you only need to look things up by key, use a **hash table**. It is the fastest.
- If you need the keys in order, or the nearest key, or a range, or a guaranteed worst case, use a **balanced tree**.
- If the data is built once and then only queried, sort a **vector** and use binary search. It is smaller and faster than either. (C++23 packages this arrangement as `std::flat_map`.)

::: {.fourq title="AVL tree"}
Where does it live?
:   As a binary search tree: one heap node per key, each owning its children. Each node also stores the height of its subtree.

What does it promise?
:   The search-tree invariant, and in addition: at every node, the two subtrees differ in height by at most one. Hence the height is less than $1.45 \log_2(n+2)$.

What does it cost?
:   Search, insert, remove, minimum, lower bound: $\Theta(\log n)$ in the worst case, with no assumptions about the input. Less than one rotation per insertion on average. Each step is a pointer chase, so for large trees the constant is dominated by cache misses.

How do you know?
:   Differential testing checks the answers but *cannot* detect broken balancing. Check the balance condition and the stored heights at every node after every operation, and assert the height of a tree built from sorted keys.
:::

::: {.fourq title="B-tree"}
Where does it live?
:   Nodes that each hold many keys in a sorted, contiguous array, plus one more child pointer than keys. Few, large nodes in place of many small ones.

What does it promise?
:   Keys in each node are sorted and divide the key ranges of the children. No node holds more than the maximum number of keys. All leaves are at the same depth.

What does it cost?
:   Search and insert: $\Theta(\log n)$ comparisons, but only about $\log_B n$ nodes visited, where $B$ is the number of children per node. Far fewer cache misses, or disk reads, than a binary tree.

How do you know?
:   Differential testing against `std::set` with a tiny node size, so that splits happen constantly; an invariant check for sortedness, node capacity, and equal leaf depth.
:::

## Exercises {#sec:balance-ex .unnumbered}

1. Draw the result of a left rotation at the root of the tree with pre-order 20, 10, 40, 30, 50. Then draw the result of a right rotation at the root of *that* tree. Confirm that the in-order sequence is unchanged throughout.

2. For each of the following, draw the tree and say whether it is a valid AVL tree. If not, name the lowest node at which the invariant fails. (Each is given in pre-order.) (a) 30, 20, 10. (b) 30, 10, 20, 40, 50. (c) 50, 30, 20, 10, 40, 70.

3. Insert the keys 50, 40, 30, 60, 70, 65, 45, 47 into an empty AVL tree. Draw the tree after each insertion, and name each rotation case.

4. Using the recurrence for $N(h)$, extend the table to $h = 15$. What is the tallest AVL tree that could hold 1,000 keys? Construct an AVL tree of height 4 with exactly seven nodes.

5. In `rebalance`, the test for the inside case is `balance(*link->left) < 0`. After an *insertion*, the left child's balance factor at this point is never zero. After a *removal*, it can be. Draw a tree in which removing one key leaves a node with balance factor $+2$ whose left child has balance factor 0, and confirm that the code handles it correctly with a single rotation.

6. Explain why, after an insertion into an AVL tree, at most one call to `rebalance` on the path to the root actually performs a rotation. (Consider the height of the rotated subtree before the insertion and after the rotation.) Then give an example in which a single removal causes rotations at two different levels.

7. Perform mutation testing on `avl.hpp`. For each change, record whether the differential test alone catches it and whether `check_invariant` catches it. (a) Delete the body of `rebalance` except for `update(*link)`. (b) In `rotate_right`, omit the first `update`. (c) Swap the two rotation calls in the left-heavy branch. (d) In `erase`, omit the final call to `rebalance`. What does this tell you about what each kind of test is able to see?

8. Add `const K* lower_bound(const K& key) const` to `AvlTree`. Then add a member `std::size_t rank(const K& key) const` that returns the number of keys less than `key` in $O(\log n)$ time. (Hint: store in each node the size of its subtree, and maintain it wherever the height is maintained.)

9. Insert the keys 10, 20, 30, 40, 50, 60, 70, 80, 90 in order into a B-tree whose nodes hold at most three keys, using the split rule of this chapter. Draw the tree after each split. What is the height at the end?

10. A B-tree's nodes hold at most 255 keys, and every node except the root is at least half full. What is the largest possible height of such a tree holding one billion keys? What is the smallest?

11. Describe, with pictures, how to remove a key from a B-tree *leaf* in each of these situations: (a) the leaf still has enough keys afterward; (b) the leaf is now too empty, but a neighboring sibling has a key to spare; (c) the leaf is too empty and so is its sibling. (You are designing the algorithm; check your design against a textbook afterward.)

12. A program records sensor readings as `(timestamp, value)` pairs and must answer: "what was the most recent reading at or before time $t$?" Readings arrive in increasing order of timestamp. Which of the three containers in the table of [[sec:ordered-maps]] would you use, and how would you answer the query? Would your answer change if readings could arrive out of order?

13. **With a model, as critic.** Without looking at `avl.hpp`, write `rotate_left` and `rotate_right` for a tree with *raw* child pointers and a `parent` pointer in each node, as many textbook implementations have. Count how many pointers each rotation must update. Then ask a model to "list every pointer that a left rotation must update in a tree with parent pointers, including the cases where a subtree is empty or the rotated node is the root," and compare its list with your code. Whose list was complete?

14. **Lab.** Build `AvlMap<K, V>`, an ordered map, with `put`, `find`, `erase`, `lower_bound`, and in-order iteration by a `for_each` function. Use it to implement an *interval scheduler*: store appointments keyed by start time, and support "add an appointment unless it overlaps an existing one" in $O(\log n)$ time. (Which existing appointments could a new one possibly overlap? You need to look at only two.) Test the map differentially against `std::map`, with the invariant check running after every operation.

## Chapter notes {#sec:balance-notes .unnumbered}

The AVL tree is from a four-page paper of 1962 [@adelson1962], the first self-balancing search tree. Red-black trees originate with Bayer's "symmetric binary B-trees" of 1972 and were given their colors, and their modern formulation, by Guibas and Sedgewick [@guibas1978]; they are best understood as binary encodings of B-trees with at most three keys per node, a correspondence that Sedgewick and Wayne [@sedgewick2011] develop at length. Cormen et al. [@clrs2022] give the full red-black algorithms, including removal.

The B-tree is due to Bayer and McCreight [@bayer1972]; Comer's survey [@comer1979], titled "The Ubiquitous B-Tree" when the structure was less than a decade old, remains a fine introduction. On the randomized alternatives, see Pugh on skip lists [@pugh1990] and Seidel and Aragon on treaps [@seidel1996]; on splay trees, Sleator and Tarjan [@sleator1985]. Knuth [@knuth1998] covers balanced trees in depth and derives the AVL height bound quoted here.
