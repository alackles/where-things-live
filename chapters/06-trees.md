# Recursion and Trees {#ch:trees}

::: goals
By the end of this week you should be able to:

- write and reason about recursive functions, identifying the base case and the smaller subproblem, and estimate their cost;
- use the vocabulary of trees precisely: root, leaf, depth, height, subtree;
- explain what `std::unique_ptr` is and use it to express ownership of tree nodes;
- implement a binary search tree with search, insertion, removal, and the standard traversals;
- state the binary-search-tree invariant *correctly*, and write a function that checks it;
- explain why the cost of every operation depends on the tree's height, and how badly that can go.
:::

Take stock of where we are. A sorted vector can be searched in $\Theta(\log n)$ time, but inserting into it costs $\Theta(n)$. A linked list can be modified in $\Theta(1)$ time at a known position, but searching it costs $\Theta(n)$. A hash table does both in expected constant time, but in exchange it throws away all information about *order*: it cannot tell you the smallest key, the next key after a given one, or all the keys between two bounds.

::: check
Binary search is fast because each comparison discards half of the remaining elements. A linked structure is flexible because elements never have to move. Can you imagine a *linked* structure in which each comparison discards half of the remaining nodes? Where would the search have to start, and how many `next` pointers would each node need?

::: answer
The search would have to start at the *middle* element, since that is what binary search compares against first. From there it needs to go to the middle of the left half or the middle of the right half. So each node needs two pointers: one to the "middle" of everything smaller, and one to the "middle" of everything larger. That structure is a binary search tree.
:::
:::

Trees are the third great family of data structures, after arrays and hash tables, and they are what you reach for when you need both fast search and order. They are also the natural home of **recursion**, a way of thinking about problems that you may have met briefly before and are about to need constantly. So this chapter begins there.

## Recursion {#sec:recursion}

A function is **recursive** if it calls itself. That sounds circular, but a well-formed recursive function is no more circular than a staircase. It solves a problem by reducing it to a *smaller instance of the same problem*, and at the bottom there is an instance small enough to answer directly.

```{.cpp include="ch06/recursion.cpp" region="factorial"}
```

Every recursive function has the same two parts.

- A **base case**: an input so small that the answer is immediate. Here, the factorial of 0 or 1 is 1.
- A **recursive case**: the function calls itself on a *smaller* input and uses the result. Here, $n! = n \cdot (n-1)!$.

Two things must hold for the recursion to be correct. Every chain of recursive calls must eventually reach a base case; that is why the recursive call must be on a smaller problem. And the function must give the right answer *assuming that its recursive calls give the right answers*. Checking the second condition takes a small act of faith, and it is the habit to cultivate: when you write or read `n * factorial(n - 1)`, do not mentally descend into the inner call. Assume that it returns $(n-1)!$, as advertised, and ask only whether, given that, this call returns $n!$.

You already know the machinery that makes recursion work. It is the call stack of [[sec:lifetimes]]. Each call gets its own stack frame with its own copy of `n`. `factorial(4)` calls `factorial(3)`, which calls `factorial(2)`, which calls `factorial(1)`; four frames are stacked up, each waiting for the one above it. Then the base case returns 1, and the frames come off one at a time, each multiplying on its way out.

That picture also tells you what recursion costs. The *time* is the total number of calls times the work done in each. The *space* is the maximum depth of the stack, since each pending call occupies a frame. `factorial(n)` makes $n$ calls and nests them $n$ deep: $\Theta(n)$ time and $\Theta(n)$ space. A loop would use $\Theta(1)$ space, and for a function this simple a loop is the better choice. Recursion shows its worth on problems that split into *more than one* subproblem.

```{.cpp include="ch06/recursion.cpp" region="sum"}
```

This function sums a range by splitting it in half, summing each half recursively, and adding the results. It is not a sensible way to add up a vector, but its shape, called **divide and conquer**, is the shape of the most important algorithms in the next few chapters. Notice the half-open ranges: the two halves `[lo, mid)` and `[mid, hi)` fit together with no overlap and no gap.

::: check
For a vector of 8 elements, how many calls to `sum` are made in all, counting the first? How deep does the stack get?

::: answer
The calls form a tree: one call on 8 elements, two on 4, four on 2, eight on 1. That is $1 + 2 + 4 + 8 = 15$ calls. The stack depth is only 4, the number of levels, because the two recursive calls happen one after the other, and the first has finished and released its frames before the second begins. In general: $2n - 1$ calls, depth $\log_2 n + 1$.
:::
:::

Recursion can also make a problem dramatically cheaper.

```{.cpp include="ch06/recursion.cpp" region="power"}
```

Computing $x^n$ by repeated multiplication takes $n - 1$ multiplications. This version computes $x^{n/2}$ once and squares it. Each call halves $n$, so there are about $\log_2 n$ calls: $x^{1000}$ in ten steps or so.

And it can make a problem catastrophically expensive.

```{.cpp include="ch06/recursion.cpp" region="fib"}
```

```console
fib(10) = 55 took 177 calls
fib(20) = 6765 took 21891 calls
fib(30) = 832040 took 2692537 calls
```

The Fibonacci function calls itself twice, and the two calls do *overlapping* work: `fib(n - 1)` computes `fib(n - 2)` all over again, and so on down. The number of calls grows exponentially. The lesson is that recursion is safe when the subproblems are *disjoint*, as the two halves of a range are. When they overlap, a plain recursive solution recomputes the same things many times. (Remembering the answers to subproblems, so that each is computed once, turns out to be a powerful general technique, called dynamic programming, that you will study in an algorithms course.)

The costs of the common recursive shapes are worth knowing by sight. Write $T(n)$ for the time on an input of size $n$.

| Shape of the recursion | Example | Time |
|:--|:--|:--|
| one call on a problem smaller by one | `factorial` | $\Theta(n)$ |
| one call on a problem half the size | `power`, binary search | $\Theta(\log n)$ |
| two calls on halves, constant extra work | `sum` | $\Theta(n)$ |
| two calls on halves, linear extra work | mergesort ([[ch:heaps]]) | $\Theta(n \log n)$ |
| two calls on problems smaller by one or two | `fib` | exponential |

::: byhand
Trace `sum(v, 0, 5)` on a five-element vector by drawing its **call tree**: one box per call, labeled with its `lo` and `hi`, with lines down to the calls it makes, and the returned value written beside each box as it finishes. Then do the same for `power(2.0, 13)`.

Being able to trace a recursion by hand is how you debug one, and being able to *stop* tracing and trust the recursive call is how you write one. Both come only from practice, and neither comes from watching a model do it. A model will draw you a call tree instantly and it will usually be right; you will have learned nothing about recursion from it, and recursion is the one idea in this chapter that everything else depends on.
:::

::: llm
Once you can trace small recursions yourself, a model is a good source of fresh ones.

> Give me a short recursive C++ function I have probably not seen before, with a specific call to it. Ask me (1) what it returns and (2) how many calls are made in total. Don't reveal the answers until I reply. If I am wrong, don't just correct me: show me the call tree.

Compile and run the function with a call counter, as `fib` does above, to settle any disagreement. On questions of *counting* calls, a model's arithmetic is not to be taken on trust.
:::

## Trees {#sec:trees}

A **tree** is a collection of nodes connected so that there is exactly one path between any two of them. One node is singled out as the **root**. Computer scientists draw trees upside down, with the root at the top.

::: {.figure #fig:tree-anatomy}
The anatomy of a binary tree. Depth is counted from the root, which is at depth 0. The height of this tree is 4: the number of nodes on the longest path from the root to a leaf. The shaded region is the left subtree of the root, which is itself a tree.
:::

The vocabulary is borrowed from family trees and from botany, with no great consistency.

- Each node except the root has one **parent**, the node directly above it. The nodes directly below a node are its **children**.
- A node with no children is a **leaf**.
- The **depth** of a node is the number of links on the path from the root to it. The root has depth 0.
- The **height** of a tree is the number of nodes on the longest path from the root down to a leaf. An empty tree has height 0, and a tree with one node has height 1. (Some books count links instead of nodes, giving a number one smaller. Check the convention whenever you read about trees.)
- A node together with everything below it is a **subtree**.

In a **binary tree**, each node has at most two children, and they are distinguished: a *left* child and a *right* child.

The last item on the vocabulary list is the important one. It leads to a definition of binary trees that is more useful than any picture:

> A binary tree is either **empty**, or it is a **node** with a left subtree and a right subtree, each of which is a binary tree.

The definition is recursive: it defines trees in terms of smaller trees, with the empty tree as its base case. This is why trees and recursion go together. Almost any function on a tree can be written by following the definition: handle the empty tree; otherwise handle the node, and let recursive calls handle the two subtrees.

### Nodes that own their children {#sec:unique-ptr .unnumbered}

Here is the representation.

```{.cpp include="ch06/bst.hpp" region="node"}
```

In [[ch:links]] the nodes of a list were joined by raw pointers, and we wrote the destructor and copy operations by hand. Here the child pointers are `std::unique_ptr<Node>`. This is the standard library's **owning pointer**, and it is how modern C++ expresses the statement "this pointer is responsible for deleting what it points to."

A `unique_ptr` behaves like a pointer, with three differences.

- When a `unique_ptr` is destroyed, it deletes the object it points to. No `delete` is ever written by hand.
- A `unique_ptr` **cannot be copied**. If it could, two pointers would own the same object and delete it twice, which is the shallow-copy bug of [[sec:rule-of-three]]. The compiler rejects the attempt.
- A `unique_ptr` **can be moved**, with `std::move`. Ownership passes to the destination, and the source becomes null.

You create the owned object with `std::make_unique<Node>(...)`, which allocates it and returns the `unique_ptr`. When you need to look at the object without taking ownership, `.get()` returns an ordinary raw pointer. This book uses raw pointers in exactly that role: as *observers* that walk a structure owned by someone else.

Now consider what happens when a `Bst` is destroyed. Its member `root_` is destroyed, which deletes the root node. Destroying the root node destroys *its* members `left` and `right`, which delete the two children, and so on down. The entire tree is freed, correctly, and we did not write a line of code to do it. Nor can a `Bst` be accidentally copied, since its `unique_ptr` member cannot be. Ownership has been stated in the *types*, and the compiler does the rest.

Moving needs one small piece of help. The compiler's default move would move `root_`, leaving the source's root null, but would merely *copy* the plain integer `size_`, leaving the source claiming to hold nodes it no longer has. So the class defines its two move operations by hand, to set the source's size to zero. Keep an eye out for this whenever a class stores a count beside the thing it counts.

::: note
The automatic destruction is recursive, one stack frame per level of the tree, and so is the `insert` function later in this chapter. For the tall, thin trees we will meet at the end of the chapter, a tree with a hundred thousand nodes can be a hundred thousand levels deep, and inserting into it or destroying it can overflow the stack. This is also why the linked list of [[ch:links]] used raw pointers and a loop: a list is the tallest, thinnest tree there is. Production code that might build degenerate trees destroys them iteratively. Balanced trees, the subject of the next chapter, never get deep enough for this to matter.
:::

### Recursion on trees {#sec:tree-recursion .unnumbered}

Here are two functions written directly from the recursive definition.

```{.cpp include="ch06/bst.hpp" region="height"}
```

Read `height` as a sentence: the height of an empty tree is zero; otherwise it is one, for this node, plus the height of the taller subtree. Try to write it with a loop and you will appreciate what the recursion is doing for you.

How tall must a binary tree be to hold $n$ nodes? At depth 0 there is room for one node; at depth 1, two; at depth 2, four. A tree of height $h$ holds at most $1 + 2 + 4 + \cdots + 2^{h-1} = 2^h - 1$ nodes. Turned around: a tree with $n$ nodes has height **at least** $\log_2(n + 1)$. It has height **at most** $n$, when every node has a single child and the "tree" is really a linked list. A million nodes can form a tree of height 20, or a tree of height 1,000,000. The difference between those two will occupy us for the rest of this chapter and all of the next.

### Traversals {#sec:traversals .unnumbered}

To **traverse** a tree is to visit every node once. For a list there was one natural order. For a tree there are several, and the recursive ones differ only in *when* a node is visited relative to its subtrees.

```{.cpp include="ch06/bst.hpp" region="traversals"}
```

- **Pre-order**: the node, then its left subtree, then its right. Parents come before children. Used for copying a tree, or printing an outline.
- **In-order**: the left subtree, then the node, then the right subtree.
- **Post-order**: both subtrees, then the node. Children come before parents. Used when a node's result depends on its children's, as in `height`, and for deleting a tree.

Each visits $n$ nodes and does constant work at each, so each is $\Theta(n)$.

The fourth traversal is not recursive. **Level-order** visits the root, then all nodes at depth 1, then all at depth 2, and so on. It needs to remember, while working across one level, the children that are waiting on the next, in the order they were discovered. First discovered, first visited: that is a queue.

```{.cpp include="ch06/bst.hpp" region="level_order"}
```

Compare this with the recursive traversals. They use a stack (the call stack) and go *deep* first. Level-order uses a queue and goes *wide* first. The same contrast, between depth-first and breadth-first exploration, will reappear for graphs in [[ch:graphs]].

## Binary search trees {#sec:bst}

A binary tree is just a shape. To make it useful for searching we impose an invariant on where the keys go.

> **The binary search tree invariant.** For every node, all keys in its left subtree are less than the node's key, and all keys in its right subtree are greater.

A binary tree that satisfies this is a **binary search tree**, or BST. Read the invariant carefully: it says *all keys in the subtree*, not merely "the left child." We will see shortly what goes wrong if you remember it the weaker way.

```{.cpp include="ch06/bst_test.cpp" region="build"}
```

::: {.viz #fig:bst name="bst"}
The binary search tree produced by inserting 50, 30, 70, 20, 40, 60, 80, 35 in that order. The highlighted path is a search for 35: less than 50, go left; greater than 30, go right; less than 40, go left; found. In the web edition you can insert and remove keys, run the traversals, and try inserting keys in sorted order.
:::

### Searching {#sec:bst-search .unnumbered}

To search for a key, start at the root. If the key is smaller than the node's key, it can only be in the left subtree, by the invariant. If it is larger, it can only be in the right. Move to that child and repeat. If you arrive at an empty subtree, the key is not in the tree.

```{.cpp include="ch06/bst.hpp" region="contains"}
```

This is binary search, performed by following pointers instead of computing midpoints. Each comparison discards an entire subtree. The code uses only `<` to compare keys: if neither key is less than the other, they are taken to be equal. That is the convention of the C++ standard library, and it means a key type needs only a less-than operator to be stored in a tree.

Because a search only walks downward, a loop is simpler than recursion here, and `n` is a raw observer pointer: we are looking, not owning.

### Inserting {#sec:bst-insert .unnumbered}

A new key always goes in as a new leaf, at the spot where a search for it falls off the tree. The search finds an empty subtree, a null link, where the key would have been. That link is where the new node belongs.

```{.cpp include="ch06/bst.hpp" region="insert"}
```

The parameter is the interesting part. `Link& link` is a *reference to the owning pointer* that leads to the current subtree. At the top it refers to `root_`. One level down it refers to some node's `left` or `right` member. When the recursion reaches a null link, assigning to `link` attaches the new node at exactly the right place, whether that place is the root of an empty tree or deep inside a full one, and no special cases are needed. This is the pointer-to-pointer idea of [[sec:rewiring]], in the more comfortable form of a reference.

Inserting a key that is already present does nothing and returns `false`: this tree represents a *set*, with no duplicates.

### What order buys you {#sec:bst-order .unnumbered}

Consider an in-order traversal of a binary search tree. It visits everything in the left subtree, all of which is smaller than the root; then the root; then everything in the right subtree, all of which is larger. Apply the same reasoning within each subtree, and you find that

> **an in-order traversal of a binary search tree visits the keys in increasing order.**

The tree holds its keys sorted, implicitly, at all times, and delivers them in order in $\Theta(n)$ time. This is the first of several things that a hash table cannot do. Here are two more.

```{.cpp include="ch06/bst.hpp" region="queries"}
```

The smallest key is found by going left until you cannot. `lower_bound` finds the smallest key that is greater than or equal to a given one, the same question that `lower_bound_index` answered for a sorted vector in [[sec:binary-search]]. It walks down the tree, remembering the best candidate seen so far. With it you can find the key nearest to a value, or the first entry on or after a given date. Add the ability to step from one key to the next, and you can list every key in a range.

All of these, like `contains` and `insert`, walk one path from the root downward and do constant work at each node.

## Removal {#sec:bst-erase}

Removing a key is the hardest of the basic operations, because removing a node from the middle of a tree leaves a hole that must be repaired without violating the invariant. There are three cases, in increasing order of difficulty.

**The node is a leaf.** Just remove it. Nothing else is affected.

**The node has one child.** Remove it and attach its only child in its place. Everything in that child's subtree was already on the correct side of the removed node's parent, so the invariant survives.

**The node has two children.** We cannot simply remove it, since there are two subtrees to reattach and only one place to put them. Instead we keep the node and replace its *key* with a different key that can legitimately sit in that position: one that is greater than everything in the left subtree and less than everything else in the right subtree. That key is the smallest key in the right subtree, the node's **in-order successor**. The successor is found by taking one step right and then going left as far as possible. Since it is as far left as possible, it has no left child, so removing it from its old position is one of the two easy cases.

::: {.figure #fig:bst-erase}
Removing 30, a node with two children. Its in-order successor is 35, the leftmost node in its right subtree. The successor's key is moved up to replace 30, and the successor's old node, which has at most a right child, is unlinked.
:::

```{.cpp include="ch06/bst.hpp" region="erase"}
```

The function first finds the node, recursing down with a reference to the owning link, just as `insert` did. When it finds the key, `link` is the pointer that owns the doomed node.

In the easy cases, a single move assignment does everything. `link = std::move(link->right)` transfers ownership of the right child into `link`. The node that `link` previously owned is deleted by the `unique_ptr` as part of the assignment. No `delete`, no temporary, and nothing can leak. (If the node was a leaf, the right child is null, and `link` simply becomes null. The leaf case needs no code of its own.)

In the hard case, `succ` walks down to the link that owns the successor. It is a raw pointer to a `Link`: we are again tracking "the pointer that will need to change." The successor's key is moved up, and the successor's node is replaced by its right child, exactly as in the easy case.

::: check
In the two-child case, the code uses the smallest key of the right subtree. What other key in the tree could be moved into the node's position instead, without breaking the invariant?

::: answer
The largest key in the *left* subtree, the in-order **predecessor**: one step left, then right as far as possible. It is greater than everything else in the left subtree and less than everything in the right. Either choice is correct.
:::
:::

## The shape problem {#sec:shape}

Every operation in this chapter, searching, inserting, removing, finding a minimum or a lower bound, starts at the root and walks down one path. Each takes time proportional to the length of that path, which is at most the height of the tree, $h$.

| Operation | Cost |
|:--|:--|
| `contains`, `insert`, `erase`, `min`, `lower_bound` | $\Theta(h)$ |
| any traversal | $\Theta(n)$ |

So the cost of a binary search tree is $\Theta(h)$, and everything depends on what $h$ is. We know that $h$ lies somewhere between about $\log_2 n$ and $n$. Which is it?

::: check
Starting from an empty tree, insert the keys 1, 2, 3, 4, 5, 6, 7 in that order. Draw the result. What is its height? How many comparisons does it take to find 7?

::: answer
Each new key is larger than every key already present, so each becomes the right child of the previous one. The result is a chain leaning to the right: a linked list in all but name, of height 7. Finding 7 takes seven comparisons. Insert the same keys in the order 4, 2, 6, 1, 3, 5, 7, and you get a perfectly balanced tree of height 3.
:::
:::

**The shape of a binary search tree is determined by the order in which keys are inserted.** [[fig:shapes]] shows the same seven keys in the best shape and the worst.

::: {.figure #fig:shapes}
The same seven keys in two binary search trees. Both satisfy the invariant. Inserted in the order 4, 2, 6, 1, 3, 5, 7, they form a tree of height 3. Inserted in increasing order, they form a chain of height 7, and every operation is a linear scan.
:::

Sorted input is the worst case, and sorted input is extremely common. Data arrives in order all the time: records with increasing ID numbers, timestamps, the contents of a file that someone already sorted. A data structure that collapses on its most likely input is in trouble.

What about input in no particular order? Here are measurements for trees built by inserting the numbers 0 through $n - 1$ in random order.

| $n$ | $\log_2 n$ | height | average depth of a node |
|--:|--:|--:|--:|
| 1,000 | 10.0 | 26 | 11.2 |
| 10,000 | 13.3 | 28 | 15.3 |
| 100,000 | 16.6 | 42 | 21.4 |
| 1,000,000 | 19.9 | 53 | 24.8 |

Random insertion order produces trees that are not perfectly balanced but are *good enough*. The average node is at a depth of roughly $1.39 \log_2 n$, less a small constant, so a typical search costs only about forty percent more comparisons than the ideal [@knuth1998]. The height, too, grows in proportion to $\log n$. The same code built from sorted keys:

```console
sorted: n = 1000   height = 1000   avg depth = 499.5
sorted: n = 10000   height = 10000   avg depth = 4999.5
```

We are in the same position as with hash tables. The structure is excellent on average and terrible in the worst case, and the worst case is not exotic. For hash tables the remedy was to make the hash function unpredictable. For trees there is a better one: change the algorithms so that the tree *cannot* become tall, whatever order the keys arrive in. That is the subject of [[ch:balance]].

### Checking the invariant {#sec:isbst .unnumbered}

How do you know a tree satisfies the binary-search-tree invariant? The obvious check goes like this.

```{.cpp include="ch06/isbst.cpp" region="wrong"}
```

It verifies that every node's left child is smaller than it and its right child larger. It is also wrong, and it is one of the best-known wrong answers in the subject.

::: check
Find a binary tree with five nodes that `looks_ordered` accepts but that is *not* a binary search tree. (Hint: put a key in the left subtree of the root that is larger than the root.)

::: answer
Make 50 the root with children 30 and 70, and give 30 the children 20 and 60. Every parent–child pair is in order: 60 is greater than its parent 30. But 60 is in the left subtree of 50, where every key must be less than 50. A search for 60 would go right at the root and never find it.
:::
:::

The invariant is about whole subtrees. A key must be on the correct side of *every one of its ancestors*, not only its parent. The correct check carries that information down the tree as a pair of bounds.

```{.cpp include="ch06/bst.hpp" region="ordered"}
```

Going left tightens the upper bound to the current key; going right tightens the lower bound. Every node is checked against limits inherited from all the ancestors above it.

Our `check_invariant` uses this function, and the test for this chapter runs it throughout a differential test against `std::set`, the standard library's ordered set, comparing `contains`, `lower_bound`, sizes, and the complete in-order sequence of keys.

::: llm
The parent-and-children check is so common in tutorials and forum answers that it makes a good probe of any source's reliability, human or machine. Try both directions.

First, hand a model the wrong function, without comment:

> Is this function a correct check of the binary search tree property? If not, give the smallest counterexample tree.

Then, in a fresh conversation, ask it to *write* such a check from scratch, and read what it produces. Does it carry bounds down, or compare each node only with its children?

Find the counterexample yourself before you ask. The point of the exercise is not that the model gets it wrong (a good one will probably get it right) but that you now know a precise, checkable fact that a plausible-looking answer can violate, and you know how to test for it.
:::

::: wild
The search trees in this chapter are a small corner of the tree kingdom. A file system is a tree of directories. A web page is a tree of nested elements, and so is any JSON or XML document. A compiler turns your program into a tree, in which `a + b * c` is an addition node whose right child is a multiplication, and then traverses that tree, post-order, to generate code. In all of these, the recursive definition and the recursive traversals carry over directly, and `std::unique_ptr` children are the standard way to represent them in modern C++.
:::

::: {.fourq title="Binary search tree"}
Where does it live?
:   One heap node per key, each owning its left and right subtrees through `unique_ptr`s. The tree object owns the root. Nodes are scattered in memory, as with a linked list.

What does it promise?
:   For every node, all keys in its left subtree are smaller and all keys in its right subtree are larger. Consequently an in-order traversal yields the keys in sorted order. Nothing is promised about the tree's shape.

What does it cost?
:   Search, insert, remove, minimum, lower bound: $\Theta(h)$, where the height $h$ is between $\log_2 n$ and $n$. About $1.39 \log_2 n$ on average for keys inserted in random order; $n$ for keys inserted in sorted order. Traversal: $\Theta(n)$.

How do you know?
:   A recursive invariant check that passes bounds down from every ancestor; differential testing against `std::set`, including the in-order key sequence; tests that deliberately insert sorted keys.
:::

## Exercises {#sec:trees-ex .unnumbered}

1. Write a recursive function `int count_digits(unsigned n)` that returns the number of decimal digits in `n`. Identify the base case and the smaller subproblem. How many calls does it make for an input with $d$ digits?

2. Write a recursive function `bool is_palindrome(const std::string& s, std::size_t lo, std::size_t hi)` that reports whether the characters of `s` in the range `[lo, hi)` read the same in both directions.

3. Write binary search recursively: `std::size_t search(const std::vector<int>& v, int target, std::size_t lo, std::size_t hi)`. What is its stack depth? Is there any advantage over the loop of [[ch:cost]]?

4. Draw the call tree for `fib(5)`. How many calls are there? How many of them are calls to `fib(1)`? Then write a version of `fib` that runs in $\Theta(n)$ time using a loop.

5. Draw the binary search tree that results from inserting these keys, in order, into an empty tree: 40, 20, 60, 10, 30, 50, 70, 25, 35, 65. Give its height, and list the keys in pre-order, in-order, post-order, and level-order.

6. Starting from the tree in the previous exercise, draw the tree after each of these removals, performed one after another: 10, 60, 20, 40.

7. Give an insertion order for the keys 1 through 15 that produces a tree of height 4. How many different insertion orders of the keys 1, 2, 3 produce a tree of height 2? Of height 3?

8. Add these member functions to `Bst<K>`: (a) `const K* max() const`; (b) `std::size_t count_leaves() const`; (c) `const K* floor(const K& key) const`, the largest key less than or equal to `key`; (d) `std::size_t count_in_range(const K& lo, const K& hi) const`, which should not visit subtrees that cannot contain keys in the range.

9. A pre-order listing of a binary search tree is enough to reconstruct it exactly. Explain why, and reconstruct the tree with pre-order 50, 30, 20, 40, 35, 70, 60, 80. Is an in-order listing enough? Is a post-order listing?

10. Write a member function `Bst<K> clone() const` that returns a deep copy of the tree with exactly the same shape. Which traversal order does your helper function use, and why?

11. Write a non-recursive in-order traversal using an explicit `std::stack<const Node*>`. Compare its structure with `level_order`. What changes when the queue becomes a stack?

12. `Bst::erase` always uses the successor in the two-child case. Suppose you insert $n$ random keys and then repeatedly remove a random key and insert a new random key, many times. Predict what happens to the tree's balance, then measure the average depth after $n^2$ such pairs for $n = 1000$. (The answer surprised the researchers who first ran this experiment.)

13. **With a model, as critic.** Write the three-case removal for a BST that uses *raw* pointers (`Node* left; Node* right;`) and an explicit `delete`. Run it under AddressSanitizer on your own tests. Then ask a model: "Without rewriting it, tell me whether any path through this function leaks a node, deletes a node twice, or leaves a dangling pointer." Compare its findings with the sanitizer's. Then write two sentences on what `unique_ptr` saved you from.

14. **Lab.** Turn `Bst<K>` into `BstMap<K, V>`, a map from keys to values, with `put`, `find` (returning a pointer to the value or null), `erase`, and an in-order `for_each`. Use it to rewrite the word-frequency program of [[ch:hashing]] so that it prints *every* word and its count in alphabetical order. Then feed it two inputs, a novel from Project Gutenberg and the first 10,000 lines of the system dictionary file (which is sorted), and report the height of the tree and the running time for each. Explain the difference. (Why only the first 10,000 lines? Try the whole file, under the sanitizers, and explain what happens.)

## Chapter notes {#sec:trees-notes .unnumbered}

Binary search trees were discovered independently by several people around 1960, and the removal algorithm given here is due to Hibbard; Knuth [@knuth1998] gives the history and the analysis of random trees, including the $1.39 \log_2 n$ average depth and the unexpected long-run effect of removals that Exercise 12 explores. Sedgewick and Wayne [@sedgewick2011] give an especially clear treatment of ordered operations such as floor, ceiling, and range counting.

On recursion as a way of thinking, and on trees as recursive data, the classic source remains *Structure and Interpretation of Computer Programs* [@abelson1996]. For `std::unique_ptr` and the idea of expressing ownership in types, see Meyers [@meyers2014] and the C++ Core Guidelines [@coreguidelines].
