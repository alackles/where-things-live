# Heaps and Sorting {#ch:heaps}

::: goals
By the end of this week you should be able to:

- state the two invariants of a binary heap, and explain how they let a tree be stored in an array with no pointers;
- implement `push`, `pop`, and linear-time heap construction, and use `std::priority_queue`;
- explain and implement heapsort, mergesort, and quicksort, and give the time, space, and stability of each;
- explain why no comparison-based sort can beat $n \log n$, and how counting sort gets around the limit;
- use the standard library's sorting facilities well, and say what `std::sort` actually does.
:::

An emergency room does not treat patients in the order they arrive. It treats the most urgent first. An operating system runs the highest-priority task that is ready. A simulation processes whichever pending event has the earliest timestamp. In each case items arrive in any order, and at any moment we want to remove the *smallest* one (most urgent, earliest), with arrivals and removals interleaved.

::: check
Consider implementing this with structures we already have. For each, give the cost of adding an item and the cost of removing the smallest: (a) an unsorted vector; (b) a vector kept sorted; (c) a balanced search tree. Is there anything unsatisfying about the best of them?

::: answer
(a) Add: $\Theta(1)$, at the end. Remove smallest: $\Theta(n)$, since it must be searched for. (b) Add: $\Theta(n)$, to shift elements; remove smallest: $\Theta(1)$ if the vector is sorted in decreasing order so that the smallest is at the back. (c) Both $\Theta(\log n)$. The tree is the only one with no linear-time operation, but it is doing far more than was asked. It keeps *every* element in order, in a pointer-heavy structure, when all we ever want is the smallest one.
:::
:::

That last observation is the key to this chapter. Fully sorted order is a strong promise, and expensive to maintain. If all we need is fast access to the minimum, a much weaker promise will do, and a weaker promise is cheaper to keep. The structure that makes exactly the right promise is the **heap**. It will turn out to be a tree that lives in an array, which gives it the best of both worlds, and it leads directly into the oldest problem in the field: sorting.

(The word "heap" here has nothing to do with the heap of [[ch:memory]], the region of memory that `new` allocates from. It is an unfortunate coincidence of names.)

## Priority queues and the binary heap {#sec:heap}

The abstract data type is called a **priority queue**.

| Operation | Meaning |
|:--|:--|
| `push(x)` | add an element |
| `top()` | look at the smallest element |
| `pop()` | remove the smallest element |

A **binary heap** implements it with a binary tree that obeys two invariants.

> **Shape.** The tree is *complete*: every level is full except possibly the last, and the last level is filled from the left with no gaps.
>
> **Order.** Every node's key is less than or equal to the keys of its children.

The order invariant is the weak promise. It says nothing about left versus right; siblings are in no particular order, and a key on the left side of the tree can be larger than a key on the right. But it is enough to guarantee one thing: following it up from any node, keys only get smaller, so **the smallest key is at the root**. A heap ordered this way is called a *min-heap*. Reverse the comparison and the largest key is at the root: a *max-heap*.

The shape invariant is what makes the structure remarkable. Because a complete tree has no gaps, its nodes can be numbered level by level, left to right, starting from 0 at the root, and stored in an array in that order. And then the tree structure does not need to be stored at all, because it can be *computed*. For the node at index $i$:

$$\text{left child: } 2i + 1 \qquad \text{right child: } 2i + 2 \qquad \text{parent: } \lfloor (i - 1)/2 \rfloor$$

::: {.viz #fig:heap name="heap"}
A binary min-heap, shown as a tree and as the array that actually stores it. The children of the element at index $i$ are at indices $2i+1$ and $2i+2$. No pointers are stored. In the web edition, push and pop elements and watch them sift up and down in both views.
:::

Look at what this does to the first of our four questions. A binary search tree lives in scattered nodes joined by pointers. A heap is a tree that lives in *one contiguous block*, with no pointers and no per-node allocation, and whose links are a multiplication and an addition. It has the logarithmic height of a balanced tree and the memory layout of a vector. The whole representation is a `std::vector`.

### Push {#sec:heap-push .unnumbered}

To add an element, there is only one place it can go without breaking the shape invariant: the next free position at the end of the array, which is the next gap on the bottom level. That may break the order invariant, if the new element is smaller than its parent. So swap it with its parent. Now it may be smaller than its *new* parent; swap again. The element rises until it meets a parent that is no larger, or reaches the root. This is called **sifting up**.

### Pop {#sec:heap-pop .unnumbered}

The smallest element is at the root, and removing it leaves a hole at the top. Shape again dictates the repair: the only node that can be removed without leaving a gap is the *last* one. So move the last element into the root and shrink the array by one. The shape is now correct, and the order is probably wrong at the root. Compare the root with its children and swap it with the *smaller* child; repeat from there. The element sinks until both its children are no smaller, or it reaches the bottom. This is **sifting down**.

```{.cpp include="ch08/heap.hpp" region="ops"}
```

```{.cpp include="ch08/heap.hpp" region="sift"}
```

::: check
When sifting down, why must the element be swapped with the *smaller* of its two children, and not just any child that is smaller than it?

::: answer
After the swap, the child that moved up becomes the parent of the other child. If the larger child were moved up, it would sit above its smaller sibling, violating the order invariant at that very node. Moving the smaller child up guarantees that the new parent is no larger than both of the nodes below it.
:::
:::

A complete tree with $n$ nodes has height $\lfloor \log_2 n \rfloor + 1$, and each of these operations moves an element along a single root-to-leaf path. So `push` and `pop` are $\Theta(\log n)$ in the worst case, and `top` is $\Theta(1)$. There is no shape problem to worry about and no rebalancing: the shape invariant makes the tree perfectly balanced by construction.

### Building a heap in linear time {#sec:heapify .unnumbered}

Given $n$ elements all at once, we could build a heap by pushing them one at a time, for a total of $\Theta(n \log n)$. There is a better way, found by Robert Floyd [@floyd1964]. Put the elements in the array in any order. Then, working backward from the last node that has children to the root, sift each one down.

```{.cpp include="ch08/heap.hpp" region="heapify"}
```

When a node is sifted down, both of its subtrees are already heaps, since they were handled earlier, so sifting down makes the whole subtree rooted there into a heap. The surprise is the cost. Half the nodes are leaves and are not touched at all. A quarter are one level above the leaves and can sink at most one level. An eighth can sink at most two. The total number of moves is at most

$$\frac{n}{4}\cdot 1 + \frac{n}{8}\cdot 2 + \frac{n}{16}\cdot 3 + \cdots < n.$$

Building a heap takes $\Theta(n)$ time. The trick is that the expensive nodes, the ones near the top with a long way to sink, are few, and the numerous nodes near the bottom are cheap.

### In the standard library {#sec:std-pq .unnumbered}

The standard priority queue is `std::priority_queue`, an adapter over a vector, like `std::stack`.

```{.cpp include="ch08/sort_test.cpp" region="std_pq"}
```

::: warning
`std::priority_queue` is a **max**-heap by default: `top()` is the *largest* element. This is the opposite of the convention in most textbooks and of what many algorithms want. To get a min-heap you must name all three template parameters, as above. Getting this backward is a rite of passage, and the bug is quiet: the program runs, and processes things in the worst possible order.
:::

A heap is not a general-purpose container, and it pays to be clear about what it has given up. There is no efficient way to search it for a particular element, and no way to visit its elements in sorted order short of popping them all. It does one thing.

| Operation | Sorted vector | Balanced tree | Binary heap |
|:--|:--|:--|:--|
| find the minimum | $\Theta(1)$ | $\Theta(\log n)$ | $\Theta(1)$ |
| remove the minimum | $\Theta(1)$ | $\Theta(\log n)$ | $\Theta(\log n)$ |
| insert | $\Theta(n)$ | $\Theta(\log n)$ | $\Theta(\log n)$ |
| build from $n$ elements | $\Theta(n \log n)$ | $\Theta(n \log n)$ | $\Theta(n)$ |
| search for an arbitrary key | $\Theta(\log n)$ | $\Theta(\log n)$ | $\Theta(n)$ |
| extra memory per element | none | two or three pointers | none |

::: wild
Priority queues are everywhere that "next" means "most important" and not "oldest." Operating systems use them to pick the next timer to fire. Network routers use them to schedule packets. Discrete-event simulators, from chip design to epidemiology, are a loop around a priority queue of future events. In the next chapter a priority queue will drive the most famous shortest-path algorithm.

And there is one in the loop that generates text from a language model. At each step the model assigns a score to every token in its vocabulary, perhaps a hundred thousand of them, and a common sampling strategy considers only the $k$ highest-scoring ones. Finding the top $k$ of a hundred thousand numbers does not require sorting them all: a heap, or the selection algorithm at the end of this chapter, does it in a fraction of the time, and it happens for every token of every response.
:::

## Sorting with a heap, and by merging {#sec:sorting}

**Sorting**, putting a sequence into order, is the most studied problem in computer science. It matters in itself, and it matters even more as a preliminary: binary search requires sorted input, finding duplicates is trivial once equal items are adjacent, and many problems that look hard become easy once the data is in order. We met insertion sort in [[ch:cost]] and found it to be $\Theta(n^2)$ in the worst case. We can do much better, and a heap gives the first way.

### Heapsort {#sec:heapsort .unnumbered}

Popping every element from a min-heap delivers them in increasing order, so $n$ pushes followed by $n$ pops is a sort, in $\Theta(n \log n)$ time. J. W. J. Williams, who invented the heap in 1964, invented it for this purpose [@williams1964]. With one twist the sort can be done *in place*, within the array being sorted, using no extra memory at all.

Build a **max**-heap out of the whole array. Its largest element is at index 0. Swap it with the last element of the array: the largest element is now in its final position. Regard the heap as one element shorter, and sift the new root down to restore it. Repeat. The heap shrinks from the back as the sorted region grows behind it.

```{.cpp include="ch08/sorts.hpp" region="heap_sort"}
```

Heapsort takes $\Theta(n \log n)$ time in the worst case and $\Theta(1)$ extra space, a combination no other algorithm in this chapter achieves. Its weakness is one you can now predict from the index arithmetic: sifting down hops from index $i$ to $2i+1$ to $4i+3$, further apart at every step, all over the array. It has poor locality, and on large inputs it is the slowest of the fast sorts in practice.

### Mergesort {#sec:mergesort .unnumbered}

The second fast sort is the divide-and-conquer shape from [[sec:recursion]], applied in earnest. It rests on one observation: two sequences that are *already sorted* can be combined into one sorted sequence quickly.

::: check
You have two stacks of exam papers, each already in alphabetical order, and you want one alphabetized stack. Describe your procedure. If the stacks hold $n$ papers altogether, how many times do you compare two names?

::: answer
Look at the top paper of each stack. Whichever name comes first goes onto the output pile. Repeat until one stack runs out, then add what remains of the other. Each comparison places one paper, so there are at most $n - 1$ comparisons. You never look anywhere but the tops of the two stacks.
:::
:::

That procedure is called a **merge**, and it takes linear time. [[fig:merge]] shows one in progress.

::: {.figure #fig:merge}
Merging two sorted runs. At each step the smaller of the two front elements is moved to the output. Here 9 and 12 are being compared, and 9 will be taken next. Every element is moved exactly once.
:::

**Mergesort** is then three lines of idea. To sort a sequence: sort its left half; sort its right half; merge the two. And a sequence of zero or one elements is already sorted, which is the base case.

```{.cpp include="ch08/sorts.hpp" region="merge_sort"}
```

The two recursive calls sort the halves `[lo, mid)` and `[mid, hi)`. The loop merges them into a scratch array, and the result is copied back. Trusting the recursion is essential here: do not try to follow the calls all the way down. Assume the two halves come back sorted, and check that the merge then produces a sorted whole.

How long does it take? Let $T(n)$ be the time to sort $n$ elements. The function does two recursive calls on half-size inputs and then a linear amount of merging:

$$T(n) = 2\,T(n/2) + \Theta(n).$$

::: {.figure #fig:mergesort-tree}
The recursion tree of mergesort on $n$ elements. Each level of the tree partitions all $n$ elements into pieces, and the merging at that level touches each element once: $n$ work per level. Halving reaches pieces of size 1 after $\log_2 n$ levels. Total: $n \log_2 n$.
:::

The picture in [[fig:mergesort-tree]] solves it. At the top level there is one merge of $n$ elements. At the next, two merges of $n/2$: again $n$ in total. Then four merges of $n/4$. Every level does $n$ work, and there are $\log_2 n$ levels. Mergesort takes $\Theta(n \log n)$ time, and since it does the same splitting regardless of the data, that is its best, worst, and average case alike.

Mergesort has two further properties worth knowing.

It needs $\Theta(n)$ extra space for the scratch array. That is its main cost.

It is **stable**: elements that compare equal come out in the same relative order they went in. The code ensures this by taking from the *left* run when the two front elements are equal. Stability matters whenever records are sorted by one field and already have a meaningful order in another. Sort a roster by name, then stably by graduation year, and each year's students are still in alphabetical order. An unstable sort would scramble them. Heapsort is not stable.

## Quicksort {#sec:quicksort}

The third fast sort also divides and conquers, but it does the hard work *before* the recursive calls instead of after. Tony Hoare invented it in 1959, as a visiting student in Moscow, and it remains the basis of most library sorts [@hoare1962].

Pick one element of the sequence and call it the **pivot**. Rearrange the sequence so that everything smaller than the pivot comes before everything larger, with the pivot's equals in between. This is called **partitioning**. Now the pivot is in its final sorted position, and no element will ever need to cross from one side to the other. Sort the two sides, recursively and independently. There is nothing to merge.

Partitioning can be done in place in a single pass. The version here divides the array into three regions as it goes: less than the pivot, equal to it, and greater.

```{.cpp include="ch08/sorts.hpp" region="quick_sort"}
```

::: {.figure #fig:partition}
The invariant of the three-way partition, part of the way through. Everything left of `lt` is less than the pivot; everything from `lt` up to `i` equals it; everything from `gt` onward is greater. The elements from `i` up to `gt` have not yet been examined. Each step examines `a[i]` and shrinks the unexamined region by one.
:::

The loop is a fine example of programming by invariant, in the manner of binary search. The comment states what is true of the four regions, and [[fig:partition]] draws it. Each iteration looks at the first unexamined element, `a[i]`. If it is smaller than the pivot, it is swapped to the end of the "less" region, and both boundaries advance. If it is larger, it is swapped with the last unexamined element, which extends the "greater" region leftward; `i` does not advance, because the element that was swapped in has not been examined yet. If it is equal, it is already in the right place. The unexamined region shrinks by one each time, and when it is empty the partition is complete.

### How fast is it? {#sec:quick-cost .unnumbered}

Partitioning $n$ elements takes $\Theta(n)$ time. What happens next depends entirely on how evenly the pivot splits the array.

*If the pivot always lands near the middle,* the two sides are each about half the size, and the recurrence is the same as mergesort's: $\Theta(n \log n)$.

*If the pivot is always the smallest or largest element,* one side is empty and the other has $n - 1$ elements. The recursion goes $n$ levels deep, with partitions of size $n$, $n - 1$, $n - 2$, and so on. That is our familiar sum: $\Theta(n^2)$.

So quicksort's worst case is as bad as insertion sort's. How likely is it? That depends on how the pivot is chosen, and the most natural choice is a trap. Here are comparison counts for a textbook quicksort that always uses the *first* element as the pivot.

| $n$ | random input | already-sorted input | $n \log_2 n$ |
|--:|--:|--:|--:|
| 1,000 | 10,684 | 499,500 | 9,966 |
| 10,000 | 157,017 | 49,995,000 | 132,877 |
| 100,000 | 1,965,347 | 4,999,950,000 | 1,660,964 |

On random input it is splendid: only a little more than $n \log_2 n$ comparisons. On input that is already sorted, the first element is always the smallest, every partition is as lopsided as it can be, and the count is exactly $n(n-1)/2$. Five *billion* comparisons to "sort" a hundred thousand elements that were in order to begin with. We have seen this pattern twice before, with binary search trees and with hash tables: fine on random data, disastrous on a pattern, and the pattern (sorted input) is one that real data exhibits all the time.

The remedy is the one used for hashing. Take the choice away from the input. The `quick_sort` above picks its pivot **at random**. Now no particular input is bad. The already-sorted array is no worse than any other, because the pivots are not chosen by position. For *every* input, a quicksort with random pivots is expected to examine about $1.39\, n \log_2 n - 2.85\,n$ elements in its partitioning passes, which is what the random-input column of the table shows, and the probability of running significantly slower than that is vanishingly small. (Our three-way version tests some elements twice, once for "less" and once for "greater", so it performs about half again as many `<` comparisons.) The worst case still exists, but it has been moved from "a bad input," which an unlucky or malicious user can supply, to "a bad run of random numbers," which nobody controls.

Quicksort sorts in place (using only the stack space for the recursion, $O(\log n)$ deep in expectation), and it has excellent locality: partitioning is a pair of sequential sweeps through the array. That is why, despite its worst case, it is usually the fastest of the three in practice. It is not stable.

[[fig:sorting]] animates all of these algorithms on the same data.

::: {.viz #fig:sorting name="sorting"}
Insertion sort part of the way through an array, drawn as bars. The shaded bars at the left are already in order; the next bar will be slid into place among them. In the web edition, choose insertion sort, heapsort, mergesort, or quicksort, and an input that is random, sorted, or reversed, and compare the counts of comparisons and moves.
:::

::: byhand
With pencil and paper: (1) Starting from the array `{9, 4, 7, 1, 8, 2}`, build a min-heap by Floyd's method, drawing the tree after each sift-down. Then pop twice. (2) Partition `{5, 3, 8, 5, 1, 9, 5, 2}` around the pivot 5 using the three-way scheme, writing out the array and the positions of `lt`, `i`, and `gt` after every step. (3) Merge `{2, 5, 9}` with `{1, 5, 7, 12}`, and say which 5 comes out first.

These are the three inner loops of the chapter. Each has an invariant, and you understand the algorithm when you can maintain the invariant by hand. Index arithmetic in a heap and boundary movement in a partition are also places where a model asked to "show the steps" will sometimes drift, producing a trace that looks right and has one swap wrong in the middle.
:::

## How fast can sorting be? {#sec:lower-bound}

Three different algorithms, three different ideas, and all of them arrive at $n \log n$. That is suspicious. Is it a coincidence, or a barrier?

It is a barrier, and the argument that establishes it is one of the prettiest in the subject, because it applies to *every possible* sorting algorithm of a certain kind, including ones not yet invented.

Consider any algorithm that learns about its input only by **comparing** pairs of elements. All the sorts so far are like this. Each comparison has two outcomes, so the algorithm's whole execution on a given input can be described by the sequence of yes-or-no answers it received. After $k$ comparisons there are at most $2^k$ different sequences of answers.

Now, $n$ distinct elements can arrive in $n!$ different orders, and a correct algorithm must do something different for each one, since each needs a different rearrangement. If two input orders produced the *same* sequence of answers, the algorithm could not tell them apart and would rearrange them identically, getting at least one wrong. So the number of distinguishable answer sequences must be at least the number of possible inputs:

$$2^k \ge n!, \qquad\text{that is,}\qquad k \ge \log_2 (n!).$$

And $\log_2(n!)$ is approximately $n \log_2 n - 1.44\,n$. Therefore:

> **Any sorting algorithm based on comparisons must make at least about $n \log_2 n$ comparisons in the worst case.**

Mergesort and heapsort are *optimal*, up to a constant factor. No cleverness will find a comparison sort that runs in linear time, because the limit comes from information: there are $n!$ possibilities to distinguish, and each comparison yields one bit.

### Sorting without comparing {#sec:counting-sort .unnumbered}

The theorem has a loophole, stated right in its hypothesis: *based on comparisons*. An algorithm that examines the keys themselves can do better, when the keys have exploitable structure.

Suppose the keys are integers known to lie between 0 and $k - 1$. Make an array of $k$ counters; pass over the input once, tallying each value; then write out each value as many times as it was seen.

```{.cpp include="ch08/sorts.hpp" region="counting_sort"}
```

This is **counting sort**. It never compares two elements. It uses each key as an array index, the direct-addressing idea from [[sec:hash-idea]], and it runs in $\Theta(n + k)$ time. When $k$ is not much larger than $n$, that is linear. **Radix sort** extends the idea to larger keys by sorting on one digit at a time, with a stable counting sort for each digit.

### The stopwatch {#sec:sort-measure .unnumbered}

Here are the times, in seconds, to sort $n$ random integers between 0 and 999,999 with each algorithm in this chapter, alongside the standard library's `std::sort`.

| $n$ | insertion | heapsort | mergesort | quicksort | `std::sort` | counting |
|--:|--:|--:|--:|--:|--:|--:|
| 1,000 | 0.0001 | 0.0001 | 0.0001 | 0.0001 | 0.0000 | 0.0037 |
| 10,000 | 0.0071 | 0.0006 | 0.0006 | 0.0005 | 0.0003 | 0.0026 |
| 100,000 | 0.73 | 0.0061 | 0.0052 | 0.0060 | 0.0039 | 0.0020 |
| 1,000,000 | | 0.076 | 0.062 | 0.066 | 0.043 | 0.0070 |
| 10,000,000 | | 1.13 | 0.72 | 0.61 | 0.50 | 0.026 |

Several lessons from earlier chapters are visible at once.

Insertion sort is quadratic: each tenfold increase in $n$ multiplies its time by a hundred. At ten million elements it would need about two hours. The three $n \log n$ sorts finish in around a second.

Among those three, the ranking at large $n$ follows locality, not operation counts. Heapsort, which jumps around the array, is nearly twice as slow as quicksort, which sweeps through it.

Counting sort is twenty times faster than `std::sort` at ten million elements, because it is linear and the others are not. But look at the top of its column: for a thousand elements it is the *slowest*, because it must still initialize and scan a million counters. Its cost is $\Theta(n + k)$, and when $k$ dwarfs $n$, the $k$ is what you pay for.

And the standard library's sort is faster than all of our comparison sorts, which is the subject of the next section.

## Sorting in practice {#sec:sorting-practice}

You should understand the algorithms above, and you should essentially never use your own implementations of them. Every language's standard library has a sort that is the product of decades of engineering, and it will be faster and more robust than anything written in an afternoon.

What these library sorts have in common is that they are **hybrids**, combining several of the algorithms in this chapter so that each covers for the others' weaknesses.

- `std::sort` in the common C++ libraries is **introsort** [@musser1997]. It begins as quicksort, with pivots chosen carefully. If the recursion gets suspiciously deep, a sign that the pivots are splitting badly, it switches to heapsort, whose worst case is guaranteed. And when a piece is small, a few dozen elements or fewer, it finishes with insertion sort, which has the lowest overhead on tiny inputs. The result has quicksort's speed and heapsort's guarantee.
- `std::stable_sort` is a mergesort, with the same small-piece optimization.
- Python's `sorted` and Java's sort for objects use **Timsort** [@peters2002], a mergesort that first looks for stretches of the input that are already in order and merges those. On real-world data, which is often partly sorted, it does far fewer comparisons than $n \log n$.

Insertion sort, the "slow" algorithm, is inside every one of them. This is the practical meaning of the warning in [[sec:growth]] that Big-O hides constants: for twenty elements, the quadratic algorithm with the tiny constant wins.

### Using the library {#sec:std-sort .unnumbered}

`std::sort` takes a pair of iterators and, optionally, a comparison that says when one element should come before another.

```{.cpp include="ch08/sort_test.cpp" region="student"}
```

```{.cpp include="ch08/sort_test.cpp" region="std_sort"}
```

The comparison in the first call is a *lambda*, an unnamed function written in place. The square brackets introduce it; then come its parameters and its body.

The second call uses the C++20 **ranges** library: `std::ranges::sort` takes the container itself instead of two iterators, and its third argument is a *projection*, here a pointer to the member that each element should be sorted by. That is often clearer than writing a comparison. (The empty braces in the middle say "use the default comparison, `<`.")

The third call shows what stability is for. The roster was just sorted by name; a stable sort by year leaves each year's students in name order.

When you do not need the whole sequence sorted, there are cheaper tools.

```{.cpp include="ch08/sort_test.cpp" region="partial"}
```

`std::partial_sort` uses a heap to deliver the smallest few elements in order. `std::nth_element` uses the partitioning step of quicksort, repeatedly, on only the side that contains the position of interest; it finds the median, or any other rank, in expected *linear* time, without sorting.

::: warning
The comparison passed to any of these must be a strict "comes before" relation, as discussed for `std::map` in [[sec:ordered-maps]]. If it uses `<=`, or is otherwise inconsistent (for instance, by comparing floating-point values where some are "not a number"), the behavior of `std::sort` is undefined, and in practice it can read outside the array. A sort that crashes mysteriously is very often a sort with a bad comparison.
:::

::: llm
Questions about what a standard library "really does" are a good test of a model's precision, because the true answer has layers. Ask:

> What algorithm does std::sort use in C++? Is that required by the standard, or is it an implementation choice? What does the standard actually guarantee?

A precise answer separates three things: what the C++ standard *requires* (a bound on the number of comparisons, and nothing about which algorithm), what common implementations *do* (introsort or a relative), and what is *not* promised (stability). Check the first against cppreference. An answer that says flatly "std::sort is quicksort" is the kind that is nearly right and would lose marks on an exam.
:::

::: llm
For practice with the mechanics, use the examiner pattern, and let your own tested code be the referee.

> Give me an array of eight small integers and ask me to carry out ONE of: a single three-way partition around a stated pivot; one bottom-up heap construction; or one merge of its two sorted halves. Don't show the result until I answer.

Run the same step in your implementation, printing the array, to see who is right.
:::

### The algorithms compared {#sec:sort-summary .unnumbered}

| Algorithm | Worst case | Typical | Extra space | Stable? | Notes |
|:--|:--|:--|:--|:--|:--|
| Insertion sort | $\Theta(n^2)$ | $\Theta(n^2)$ | $\Theta(1)$ | yes | $\Theta(n)$ on nearly sorted input; best for tiny $n$ |
| Heapsort | $\Theta(n \log n)$ | $\Theta(n \log n)$ | $\Theta(1)$ | no | guaranteed and in place; poor locality |
| Mergesort | $\Theta(n \log n)$ | $\Theta(n \log n)$ | $\Theta(n)$ | yes | predictable; the basis of stable library sorts |
| Quicksort | $\Theta(n^2)$ | $\Theta(n \log n)$ | $\Theta(\log n)$ | no | fastest in practice; choose pivots at random or carefully |
| Counting sort | $\Theta(n + k)$ | $\Theta(n + k)$ | $\Theta(k)$ | can be | small integer keys only; not comparison-based |

::: {.fourq title="Binary heap"}
Where does it live?
:   In a single array, with no pointers. The element at index $i$ has its children at $2i+1$ and $2i+2$ and its parent at $\lfloor (i-1)/2 \rfloor$. It is a complete binary tree that exists only as index arithmetic.

What does it promise?
:   Shape: the tree is complete, so the array has no gaps. Order: no element is smaller than its parent, so the minimum is at index 0. Nothing is promised about the order of siblings, and there is no efficient search.

What does it cost?
:   `top`: $\Theta(1)$. `push` and `pop`: $\Theta(\log n)$ worst case. Building from $n$ elements: $\Theta(n)$. No per-element memory overhead.

How do you know?
:   Check that every element is no smaller than its parent, after every operation; test differentially against `std::priority_queue`; pop everything and confirm that the output is sorted.
:::

## Exercises {#sec:heaps-ex .unnumbered}

1. Which of these arrays are min-heaps? For those that are not, name an element that violates the order invariant. (a) `{1, 3, 2, 7, 4, 5}` (b) `{2, 5, 3, 4, 9, 8}` (c) `{1, 2, 3, 4, 5, 6, 7}` (d) `{4, 6, 5, 9, 8, 3}`.

2. Starting from an empty min-heap, push 8, 3, 10, 1, 6, 14, 4 in that order. Draw the tree and the array after each push. Then pop three times, again drawing each state.

3. In a min-heap of $n$ distinct elements stored in an array: (a) at which indices can the *largest* element be? (b) at which indices can the second-smallest be? (c) at which indices can the third-smallest be?

4. Add a member `void replace_top(T value)` to `MinHeap`: it removes the smallest element and inserts `value`, using a single sift-down. Then use it to write a function that returns the $k$ largest elements of a vector of $n$ elements in $O(n \log k)$ time and $O(k)$ extra space. (Which kind of heap do you need, of what size?)

5. Building a heap by $n$ successive `push` operations is $\Theta(n \log n)$ in the worst case. Describe an input order that actually takes that long. Then explain, in terms of *which* nodes travel far, why Floyd's bottom-up method is faster.

6. Trace mergesort on `{38, 27, 43, 3, 9, 82, 10}` by drawing the recursion tree: show each subarray as it is split on the way down, and each merged result on the way up.

7. Mergesort as written allocates one scratch vector for the whole sort. A simpler version would allocate a new vector inside every call to hold the merged result. How much total memory would that version allocate over the whole sort? Would its Big-O running time change? Why might its measured running time change a lot?

8. Trace the three-way partition on `{6, 2, 9, 6, 1, 8, 6, 3}` with pivot 6, showing the array and the three boundaries after each step.

9. Our `quick_sort` recurses on both sides. In the worst case the recursion can be $n$ levels deep. Rewrite it so that it recurses only on the *smaller* side and loops on the larger. What is the maximum recursion depth now, and why?

10. Explain why a quicksort with a two-way partition ("less than pivot" on one side, "greater than or equal" on the other) takes quadratic time on an array in which all the elements are equal, whatever pivot it chooses. Why does the three-way partition not have this problem?

11. The lower bound says a comparison sort of 5 elements needs at least $\lceil \log_2 5! \rceil = 7$ comparisons in the worst case. How many does mergesort use, in the worst case, on 5 elements? (Count carefully.)

12. You are given a vector of a million people, each with a `birthday` field giving a day of the year from 1 to 366. Describe the fastest way to sort them by birthday. Should your sort be stable? Implement it.

13. Given a vector of integers, find the pair of elements whose values are closest together. Do it in $O(n \log n)$ time. Then say why hashing, the tool that solved the "find a duplicate" problem in linear time, does not obviously help here.

14. **With a model, as critic.** Write a comparison function for sorting `Student` records by GPA, highest first, breaking ties by name alphabetically. Then ask a model: "Is this a valid strict weak ordering for std::sort? If not, give two specific students that expose the problem." Next, deliberately break it by changing a `<` to `<=`, and ask again. Does the model catch it? Does AddressSanitizer, on a large enough roster with many ties?

15. **Lab.** Write a program that finds the $k$ most frequent words in a large text file, for $k$ given on the command line, using a hash map for the counts and a heap of size $k$ for the selection. Then implement a second version that sorts all the distinct words by count. Time both on a large text (several megabytes from Project Gutenberg, concatenated if need be) for $k$ = 10, 1,000, and 100,000, and explain what you see. Your heap must be your own; your hash map may be the library's.

## Chapter notes {#sec:heaps-notes .unnumbered}

Knuth's *Sorting and Searching* [@knuth1998] is the encyclopedic treatment of everything in this chapter, including the history: mergesort goes back to von Neumann in 1945, quicksort to Hoare [@hoare1962], and the heap and heapsort to Williams [@williams1964], with the linear-time construction added by Floyd [@floyd1964] within the year. The decision-tree lower bound is presented carefully by Cormen et al. [@clrs2022], who also analyze randomized quicksort.

The engineering of real sorts is its own literature. The three-way partition used here is Dijkstra's solution to what he called the Dutch national flag problem [@dijkstra1976]. Bentley and McIlroy's "Engineering a Sort Function" [@bentley1993] showed how much three-way partitioning matters in a production quicksort, and is a model of careful experimental work. Musser's introsort [@musser1997] is the basis of `std::sort`; Peters's description of Timsort [@peters2002] is unusually readable for a document that lives in a source tree. Sedgewick and Wayne [@sedgewick2011] give the most thorough textbook treatment of quicksort variants.
