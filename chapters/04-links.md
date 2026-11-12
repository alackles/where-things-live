# Links, Stacks, and Queues {#ch:links}

::: goals
By the end of this week you should be able to:

- implement a singly linked list, including insertion, removal, reversal, and a correct destructor, and draw what each operation does to the pointers;
- explain how a sentinel node removes the special cases from a doubly linked list;
- say what an abstract data type is, and implement a stack on a vector and a queue on a circular buffer;
- give the costs of the main operations on vectors and linked lists, and explain why measured performance often contradicts a naive reading of those costs;
- decide, with reasons, when a linked structure is the right choice.
:::

The vector of [[ch:vector]] has one weakness. Its elements sit shoulder to shoulder in a single block, so putting a new element anywhere except the end means sliding everything after it over by one. Inserting at the front of a million-element vector moves a million elements.

::: check
Design, in your head, a way of storing a sequence in which a new element can be inserted at the front, or between any two existing elements, *without moving any of the others*. What does each element need to carry besides its value? What ability of the vector do you have to give up?

::: answer
If elements are not going to move, they cannot rely on position in memory to say what order they are in. Each element must instead carry that information itself: the address of the element that follows it. You give up contiguity, and with it the ability to compute where the $i$-th element is. The only way to reach it is to start at the front and follow $i$ links.
:::
:::

That design is the **linked list**, the second fundamental answer to the question *where does it live?* A vector keeps its elements in one place and the order is implicit in the layout. A linked list scatters its elements across the heap and makes the order explicit with pointers. Nearly every structure in the second half of this book (trees, graphs, the chains inside a hash table) is built from the same idea of nodes joined by pointers, so fluency here is not optional.

But this chapter also has a second job, which is to be honest about linked lists. They occupy a place of honor in the traditional curriculum, and a table of Big-O costs makes them look like the natural choice whenever a program inserts and deletes a lot. On the hardware of the last twenty years that is mostly false, and the final section shows the measurements.

## Nodes and links {#sec:nodes}

A linked list is made of **nodes**. Each node holds one element and a pointer to the next node.

```{.cpp include="ch04/list.hpp" region="node"}
```

The list object itself holds a pointer to the first node, traditionally called the **head**, and (for convenience) a count. The last node's `next` is `nullptr`, which is how you know it is the last. An empty list is a null head. Each node is a separate heap allocation; the nodes may be anywhere in memory, in any order. [[fig:linked-list]] shows a list of three.

::: {.viz #fig:linked-list name="linkedlist"}
A singly linked list. The list object holds `head`; each node holds a value and the address of the next node; the last node's `next` is null. In the web edition you can insert, remove, and reverse, and watch each pointer assignment happen in order.
:::

Notice that `Node` refers to itself. A `Node` cannot *contain* a `Node` (how big would it be?), but it can contain a pointer to one, since a pointer is eight bytes whatever it points to.

### Adding at the front {#sec:push-front .unnumbered}

```{.cpp include="ch04/list.hpp" region="push_front"}
```

`push_front` is one line, and it repays slow reading. The expression `new Node{std::move(value), head_}` allocates a node whose `value` is the new element and whose `next` is *the current head*. Only then is the result assigned to `head_`. Two pointer changes, in a definite order: first aim the new node at the old first node, then aim the head at the new node. Do them in the other order and the address of the old first node is overwritten before it has been saved anywhere. The whole existing list becomes unreachable: a leak of every node.

That is the first lesson of linked structures. **Before you overwrite a pointer, make sure the address it held is saved somewhere, or is no longer needed.**

`pop_front` shows the same care from the other side. It saves the address of the doomed node, moves the head past it, and only then deletes it.

No element was moved in either function. The cost is $\Theta(1)$, regardless of the list's length.

### Walking the list {#sec:traversal .unnumbered}

```{.cpp include="ch04/list.hpp" region="contains"}
```

This `for` loop is the idiom for visiting every node, and you will write it hundreds of times: start `p` at the head; continue while `p` is not null; advance with `p = p->next`. Compare it with the loop over an array, `for (i = 0; i < n; ++i)`. They have the same shape. Only the meaning of "the next one" differs.

Here is the price of linking. To reach the element at position $i$ there is no arithmetic to do; you must walk $i$ links from the head. Access by index is $\Theta(n)$, which also rules out binary search: there is no cheap way to jump to the middle.

### Destroying a list {#sec:list-dtor .unnumbered}

A list owns its nodes, so its destructor must delete every one.

```{.cpp include="ch04/list.hpp" region="destructor"}
```

The loop must read `head_->next` *before* deleting the node, because afterward the node's contents no longer exist. Writing `delete head_; head_ = head_->next;` is a use-after-free, and a very common one.

The copy constructor, similarly, must make a new node for every node of the original. It keeps a pointer to the last node of the copy so that each new node can be attached in constant time. It builds the copy in a local `List` and swaps at the end, for a reason worth a moment: if an allocation fails halfway through, the half-built chain belongs to the local variable, whose destructor frees it. Built directly in the object under construction, those nodes would leak, because a destructor does not run for an object whose constructor did not finish.

```{.cpp include="ch04/list.hpp" region="copy"}
```

With the destructor, this copy constructor, a move constructor that steals the head pointer, and a by-value assignment operator like the one in `Vec`, the list follows the rule of five. The complete class is in `code/ch04/list.hpp`.

::: check
The copy constructor walks the original list with `p` and builds the copy by adding each new node at the *end*. Why not use `push_front` for each element, which would be simpler?

::: answer
Pushing each element onto the front as you walk the original would produce a copy in *reverse* order. You could reverse it afterward, or keep a tail pointer as the code does.
:::
:::

## Rewiring {#sec:rewiring}

The characteristic operations on a linked list are not about moving data. They are about changing which node points to which. This is delicate work, and the only reliable way to do it is with a picture.

### Removing a node {#sec:list-remove .unnumbered}

To remove a node from the middle of a singly linked list, the node *before* it must be made to point to the node *after* it. So as we search for the node to remove, we must keep track of its predecessor.

```{.cpp include="ch04/list.hpp" region="remove"}
```

::: {.figure #fig:list-remove}
Removing the node holding 20. The search stops with `cur` at the doomed node and `prev` one behind it. A single assignment, `prev->next = cur->next`, routes the list around the node, which can then be deleted.
:::

The loop advances `prev` and `cur` in step until `cur` reaches the node to remove or runs off the end. Then there are three cases. If `cur` is null the value was not found. If `prev` is null then `cur` is the first node, which has no predecessor; the pointer that must change is `head_` itself. Otherwise the pointer that must change is `prev->next`, as in [[fig:list-remove]].

That middle case, the special treatment of the first node, is the nuisance of singly linked lists. The first node is reached through `head_` and every other node is reached through some node's `next`, so code that changes "the pointer that leads to this node" has to ask which kind it is dealing with.

### Reversing a list {#sec:list-reverse .unnumbered}

A classic exercise, worth doing because it is pure pointer manipulation: reverse a list in place, without allocating anything, by turning every arrow around.

```{.cpp include="ch04/list.hpp" region="reverse"}
```

The loop maintains an invariant: `done` is the head of a correctly reversed list containing the nodes processed so far, and `cur` is the head of the remaining, untouched part. Each step detaches the first node of the remaining part and attaches it to the front of the reversed part. The first line of the loop body is the one people forget. It saves `cur->next` before the second line overwrites it.

::: byhand
Take a sheet of paper. Draw a list of four nodes as boxes with arrows, and draw `done`, `cur`, and `rest` as labeled arrows. Execute `reverse` by hand, one statement at a time, crossing out and redrawing arrows as they change. Then do the same for `remove`, once for a node in the middle, once for the first node, and once for a value that is not present.

This is not optional busywork, and it cannot be delegated. Pointer rewiring is the one skill in this course that is almost entirely *visual and manual*: people who can do it see the boxes and arrows in their heads, and they got that way by drawing them on paper many times. A model can produce a correct `reverse` instantly. What it cannot do is install the picture in your head, and you will need that picture for trees in two weeks.
:::

### Pointers to pointers {#sec:link-trick .unnumbered}

There is a neat way to make the first node's special case disappear. In `remove`, the thing that needs to change is always a `Node*`: sometimes the one named `head_`, sometimes the one named `next` inside some node. A pointer is an object like any other, with an address of its own. So keep track of *the address of the pointer that leads to the current node*.

```{.cpp include="ch04/list.hpp" region="insert_sorted"}
```

Here `link` has type `Node**`, pointer to pointer to node. It begins by pointing at `head_`. Each step of the loop re-aims it at the `next` member of the following node. Whenever the loop stops, `*link` is the pointer that must be changed to splice in the new node, whether that happens to be the head or a `next` field, and the code does not need to know which. The new node's `next` is set to the old value of `*link`, and `*link` is then set to the new node.

This technique is more abstract than the `prev`/`cur` version, and you are free to prefer the latter. But it will come back in [[ch:trees]], where the "pointer that leads to this node" is the natural thing to pass to a recursive function.

### Iterating {#sec:list-iter .unnumbered}

A vector's iterator was a plain pointer, because advancing a pointer moves to the next element of a contiguous block. In a list the next element is wherever `next` says it is, so the iterator must be a small class that knows how to follow a link.

```{.cpp include="ch04/list.hpp" region="iterator"}
```

The iterator wraps a node pointer. Dereferencing it gives the node's value; incrementing it follows `next`; the end iterator wraps a null pointer. With `begin()` and `end()` in place, a range-`for` loop works on our list exactly as on a vector:

```{.cpp include="ch04/list_test.cpp" region="use_list"}
```

Code that uses the loop cannot tell whether it is walking a contiguous block or chasing pointers across the heap. That is the point of iterators: one interface, many arrangements of memory. The difference shows up in what the iterator *can* do. A vector's iterator can jump (`it + 500`) in constant time; a list's can only step. The standard library classifies iterators by these abilities. An algorithm that must jump, such as `std::sort`, requires the stronger kind and will not compile for a list. Binary search will accept a list's iterators, but it has to step where it would have jumped, which throws away the point of it.

::: llm
Finding the bug in a broken linked-list function is excellent practice, and a model can supply an endless stream of broken functions.

> Write a C++ function that operates on a singly linked list (insert, remove, reverse, or similar) and contains exactly one subtle pointer bug. Do not tell me what the bug is or hint at where it is. I will find it by drawing the list and tracing your code, and then you can tell me whether I am right.

Find the bug with pencil and paper, not by running the code. When you think you have it, state a specific list and call that triggers it. That is what distinguishes having found the bug from having a feeling about a line.
:::

## Doubly linked lists and sentinels {#sec:dlist}

A singly linked list can be walked in one direction only, and removing a node requires access to its predecessor. Give each node a second pointer, to the *previous* node, and both limitations go away: the list can be walked backward, and a node can be removed given nothing but its own address, since it knows both of its neighbors.

The cost is another eight bytes per node and twice as many pointers to keep consistent. With the obvious representation (a head pointer, a tail pointer, null at both ends), every operation sprouts special cases: is this the first node? The last? The only one?

There is a classic trick that removes all of them. Add one extra node, a **sentinel**, that holds no element and is never removed, and join the list into a ring: the sentinel's `next` is the first real node, and its `prev` is the last. An empty list is a sentinel pointing to itself.

::: {.figure #fig:dlist}
A doubly linked list of three elements with a sentinel. Every node, including the sentinel, has a valid `prev` and `next`, so there are no null pointers and no special cases at the ends. The sentinel doubles as the "one past the end" position.
:::

Now *every* real node has a real node or the sentinel on each side. No pointer in the structure is ever null. Insertion becomes four pointer assignments and removal two, with no conditionals in either.

```{.cpp include="ch04/dlist.hpp" region="insert_erase"}
```

Trace `insert` on an empty list, where `pos` is `end()`, the sentinel. Then `after` and `before` are *both* the sentinel, and the four assignments correctly produce a ring of two nodes. The empty-list case, usually the most bug-prone, simply works. Every other operation is a one-line call to one of these two.

```{.cpp include="ch04/dlist.hpp" region="ends"}
```

Since position is represented by an iterator, the list hands out iterators and takes them back as arguments. The iterator for a doubly linked list can step backward as well as forward.

```{.cpp include="ch04/dlist.hpp" region="iterator"}
```

One more piece of C++ appears in this class.

```{.cpp include="ch04/dlist.hpp" region="ctor"}
```

The two lines ending in `= delete` declare that a `DList` cannot be copied: any attempt is a compile-time error. This is a respectable third option under the rule of five. A class that owns a resource must either copy it correctly or refuse to be copied. What it must not do is let the compiler's member-by-member default through.

::: wild
The standard library has both kinds of list: `std::forward_list` (singly linked) and `std::list` (doubly linked, implemented with a sentinel much as here). `std::list` offers one operation that no array-based container can match: `splice`, which moves a node, or the entire contents of a list, into another list in constant time, without copying or even touching the elements. It also guarantees that iterators and references to an element stay valid until that element is erased, no matter what else is inserted or removed. Contrast that with the invalidation rules for vectors in [[sec:templates]]. Those two properties, and not raw speed, are the reasons to choose `std::list` when you do. [[ch:wild]] builds a cache that depends on both.
:::

## Stacks and queues {#sec:stacks-queues}

So far we have described containers by how they are built. It is often more useful to describe them by what they *do*. An **abstract data type**, or ADT, is a specification of a set of operations and their behavior, with nothing said about how the data is stored [@liskov1974]. An ADT answers the book's second question, *what does it promise?*, and deliberately declines to answer the first. The same ADT can be implemented by several data structures with different costs, and code written against the ADT does not care which.

The two simplest ADTs restrict a sequence so that elements can be added and removed only at its ends.

### Stacks {#sec:stack .unnumbered}

A **stack** allows insertion and removal at one end only, called the top. The last element pushed is the first one popped: *last in, first out*, or LIFO.

| Operation | Meaning |
|:--|:--|
| `push(x)` | add `x` to the top |
| `pop()` | remove the top element |
| `top()` | look at the top element |
| `empty()` | is the stack empty? |

Which data structure should implement it? A vector's cheap end is its back, so let the back be the top.

```{.cpp include="ch04/stack_queue.hpp" region="stack"}
```

The class is called an *adapter*: it adds nothing but a narrower interface. Every operation is $\Theta(1)$ (amortized, for `push`). Notice also that it declares no destructor and no copy operations: its only member is a `vector`, which manages itself. That is the rule of zero from [[sec:move]] at work. (A singly linked list would serve too, with the head as the top, but it would pay for a heap allocation on every push.) The standard library's version is `std::stack`.

You have already met the most important stack in computing. The call stack of [[sec:lifetimes]] is one: calling a function pushes a frame, returning pops it, and the function that returns is always the most recently called. Stacks turn up wherever things nest: function calls, parentheses, HTML tags, undo histories.

Here is a small, typical application. A compiler, or your editor, needs to check that brackets are balanced. Every closing bracket must match the *most recent* unclosed opening bracket. "Most recent" is the signature of a stack.

```{.cpp include="ch04/brackets.cpp" region="balanced"}
```

::: check
Trace `balanced` on the string `"([)]"`. At which character does it return, and why?

::: answer
It pushes `(`, then pushes `[`. At `)` it pops the top, which is `[`. That does not match `)`, so it returns `false` at the third character. The count of each kind of bracket is right, but the nesting is wrong, and a stack is exactly what detects nesting.
:::
:::

### Queues {#sec:queue .unnumbered}

A **queue** adds at one end, the back, and removes from the other, the front: *first in, first out*, or FIFO. It is a line at a counter. Queues model anything that must be handled in order of arrival: print jobs, network packets, tasks waiting for a processor, and, as [[ch:graphs]] will show, the frontier of a breadth-first search.

| Operation | Meaning |
|:--|:--|
| `push(x)` | add `x` at the back |
| `pop()` | remove the front element |
| `front()` | look at the front element |
| `empty()` | is the queue empty? |

A vector is awkward here. Adding at the back is cheap, but removing from the front shifts everything: $\Theta(n)$ per `pop`. A singly linked list with an extra pointer to its last node handles both ends in constant time, and that is a perfectly good implementation. But there is a way to get constant-time operations at both ends *and* keep the elements in a contiguous block.

The idea: when the front element is removed, do not shift anything. Leave a hole, and remember where the front now is. The occupied part of the block drifts rightward as the queue is used. When it reaches the end of the block, let it wrap around to the beginning, into the space that earlier removals freed. The block is treated as a ring, and the structure is called a **circular buffer** or **ring buffer**.

::: {.viz #fig:ring name="ring"}
A ring buffer of capacity eight holding five elements that have wrapped around the end of the block. `head` is the index of the front element; the back is `(head + size) % capacity`. In the web edition, push and pop to watch the occupied region travel around the ring.
:::

```{.cpp include="ch04/stack_queue.hpp" region="ring"}
```

The representation is a block of slots, the index `head_` of the front element, and the count `size_`. The element $i$ places behind the front lives at index `(head_ + i) % capacity`; the remainder operator `%` is what bends the line into a ring. Pushing writes at offset `size_`. Popping advances `head_` by one, wrapping if necessary.

When the buffer is full, `grow` allocates one twice as large and copies the elements into it *in queue order*, starting from the front, so that the queue is "unwrapped" and `head_` returns to zero. The amortized analysis is identical to the vector's.

::: warning
The classic ring-buffer bug is confusing *full* with *empty*. If you represent the queue by two indices, `head` and `tail`, then `head == tail` when the queue is empty, and also when it is completely full. Implementations that use two indices must either leave one slot permanently unused or keep a separate flag. Storing `head_` and `size_`, as here, avoids the ambiguity entirely.
:::

The standard library's queue is `std::queue`, an adapter like `std::stack`. By default both adapt `std::deque`, a "double-ended queue" that supports constant-time insertion and removal at both ends *and* constant-time indexing. A typical `deque` is implemented as an array of pointers to fixed-size blocks: a hybrid of the contiguous and linked approaches.

## Contiguous versus linked, on real hardware {#sec:cache}

Here is the comparison as a traditional textbook would give it.

| Operation | Vector | Linked list |
|:--|:--|:--|
| access the $i$-th element | $\Theta(1)$ | $\Theta(i)$ |
| insert or remove at the back | $\Theta(1)$ amortized | $\Theta(1)$ (doubly linked) |
| insert or remove at the front | $\Theta(n)$ | $\Theta(1)$ |
| insert or remove at a known position | $\Theta(n)$ | $\Theta(1)$ |
| search for a value | $\Theta(n)$ | $\Theta(n)$ |

Read naively, the table says: if your program mostly indexes, use a vector; if it mostly inserts and removes in the middle, use a list. That advice was given to generations of students. Let us test it.

### Experiment 1: just walking {#sec:exp-traverse .unnumbered}

Both structures can visit all $n$ elements in $\Theta(n)$ time. The program `traverse.cpp` adds up $n$ integers stored three ways: in a vector; in a linked list whose nodes happen to sit in memory in list order (the best case for a list, as when it was built all at once and never modified); and in a linked list whose nodes are scattered through memory in random order (what a list tends to look like after a long life of insertions and removals). The numbers are nanoseconds *per element*.

| $n$ | vector | list, nodes in order | list, nodes scattered |
|--:|--:|--:|--:|
| 1,000 | 0.19 | 0.99 | 0.99 |
| 10,000 | 0.19 | 0.99 | 2.8 |
| 100,000 | 0.19 | 1.0 | 7.4 |
| 1,000,000 | 0.20 | 1.3 | 89 |
| 10,000,000 | 0.35 | 2.3 | 110 |

Same data, same $\Theta(n)$. At ten million elements the scattered list takes more than **three hundred times** as long as the vector to do the same additions.

::: check
Using what you learned in [[sec:model-lies]], explain (a) why the scattered list is no slower than the in-order list at $n$ = 1,000, and (b) why it becomes catastrophically slower as $n$ grows.

::: answer
(a) A thousand 16-byte nodes occupy 16 kilobytes, which fits entirely in the processor's fastest cache. Once loaded, every node is a cache hit regardless of the order in which they are visited. (b) Ten million nodes occupy 160 megabytes, far more than any cache. In list order, consecutive nodes are adjacent in memory, so cache lines and prefetching still help. In scattered order, each `next` pointer leads to an unpredictable address that is almost never in the cache: a full trip to main memory, about a hundred nanoseconds, for every single element.
:::
:::

Three things work against the linked list, and all of them come from where it lives.

**No locality.** Each node may be on a different cache line from the last. A vector of `int`s packs sixteen elements into each cache line; a scattered list gets one node per line fetched, at best.

**No prefetching, and no parallelism.** The address of the next node is not known until the current node has been loaded, because the address is stored *in* the current node. The processor cannot fetch ahead, and it cannot overlap the waiting. This is called **pointer chasing**, and it forces the memory accesses to happen strictly one after another. With a vector, the addresses of the next hundred elements are known in advance, and modern processors exploit that aggressively.

**Overhead.** A node holding a four-byte `int` also holds an eight-byte pointer, is padded to sixteen bytes, and usually carries some bookkeeping from the memory allocator. The list uses four or more times the memory of the vector to hold the same data, which means fewer elements fit in the cache to begin with.

[[fig:cache-sim]] is a small simulation of the effect.

::: {.viz #fig:cache-sim name="cachesim"}
Memory drawn as a grid of cache lines. A vector's elements fill consecutive cells, so each line that is fetched serves many accesses. A linked list's nodes are spread over many lines. In the web edition, run both traversals through a small simulated cache and compare the miss counts.
:::

### Experiment 2: inserting in the middle {#sec:exp-insert .unnumbered}

Traversal is the vector's home ground. Surely the list wins when the workload is insertion in the middle, the very thing it was designed for?

This experiment follows one that Bjarne Stroustrup has used in talks for years [@stroustrup2012]. Take $n$ random integers and insert them one at a time into a sequence that is kept sorted. For each, find the position where it belongs by scanning from the front, then insert it there.

```{.cpp include="ch04/bench/sorted_insert.cpp" region="experiment"}
```

For the vector, each insertion shifts on average half the elements. For the list, each insertion relinks a couple of pointers.

::: check
Predict: which container finishes sooner, and by how much? Does your answer depend on $n$?

::: answer
See the table. If you predicted that the list wins for large $n$, you are in the good company of nearly everyone who has seen only the Big-O table.
:::
:::

| $n$ | `std::vector` | `std::list` | list ÷ vector |
|--:|--:|--:|--:|
| 10,000 | 0.007 s | 0.072 s | 10 |
| 20,000 | 0.025 s | 0.35 s | 14 |
| 40,000 | 0.10 s | 1.9 s | 19 |
| 80,000 | 0.40 s | 12 s | 31 |
| 160,000 | 1.6 s | 68 s | 44 |

The vector wins at every size, and its lead *grows* with $n$.

The reason is in the part of the task that the Big-O table's "known position" quietly assumed away. Before you can insert at a position you must *find* the position, and finding it means walking the sequence. That walk is experiment 1 again, and the list loses it badly. The vector then has to shift its elements, but shifting a contiguous block is among the fastest things a computer can do: it is a straight-line copy through memory, with perfect locality. Both containers do $\Theta(n)$ work per insertion. The vector's $\Theta(n)$ has a far smaller constant.

(The right way to do this particular task is with neither. Binary search finds the position in a vector in $O(\log n)$, and the structures of [[ch:trees]] and [[ch:balance]] make the insertion logarithmic as well.)

### So when is a linked list the right choice? {#sec:when-lists .unnumbered}

Rarely, but not never. A linked list earns its place when one of its particular promises is what you need.

- **Stable addresses.** Nodes never move. If other parts of the program hold pointers or iterators to elements and those must remain valid while the sequence changes, a vector will not do.
- **Constant-time splicing and removal, given the node.** If you *already hold* a pointer to the node, with no search required, a doubly linked list removes or relocates it in constant time. This is how a least-recently-used cache works ([[ch:wild]]), and how operating system kernels keep track of processes and memory pages, with the links embedded directly in the objects.
- **Elements that are large or expensive to move**, where shifting them would cost more than chasing pointers.
- **A worst-case guarantee per operation**, where the occasional $\Theta(n)$ reallocation of a vector is unacceptable.

And the most important reason to have learned them has nothing to do with lists. Trees, graphs, and hash-table chains are all nodes joined by pointers, and everything you practiced this week (allocating nodes, rewiring links in the right order, cleaning up without leaks) carries over to them directly.

For everyday sequences, the guidance from the people who design and implement C++ is blunt: **use `std::vector` unless you have a specific, measured reason not to** [@coreguidelines].

::: llm
The claim that linked lists are the right choice for insertion-heavy workloads is one of the most frequently repeated statements in programming education, so it is heavily represented in the text that language models learned from. Ask one:

> When should I choose std::list over std::vector in C++? Be specific.

Then grade the answer against the measurements in this section. Does it mention cache behavior? Does it distinguish "insertion at a position you already hold" from "insertion at a position you must first find"? Does it give any numbers, and if so, where could they have come from? A good answer exists and a model may well produce it. Your job is to be able to tell, and you can, because you have run the experiment.
:::

::: {.fourq title="Linked lists"}
Where does it live?
:   One heap allocation per element, anywhere in memory, joined by pointers. The list object holds a head pointer (or a sentinel). Order is explicit in the links, not implicit in the layout.

What does it promise?
:   Following `next` from the head visits every element once and ends (at null, or back at the sentinel). In a doubly linked list, `p->next->prev == p` for every node. Nodes never move, so pointers and iterators to an element stay valid until it is removed.

What does it cost?
:   Insert or remove at the front, or at a node you already hold: $\Theta(1)$. Reaching the $i$-th element, or finding a value: $\Theta(n)$, with a large constant because of cache misses and pointer chasing. Memory: one or two extra pointers per element, plus allocator overhead.

How do you know?
:   An invariant check that walks the list and counts; differential tests against `std::list` and `std::deque`; and AddressSanitizer, which is at its most valuable on code that allocates and frees many small nodes.
:::

::: {.fourq title="Stack and queue"}
Where does it live?
:   These are abstract data types; they live wherever the implementation puts them. A stack is naturally a vector with the top at the back. A queue is naturally a ring buffer: a contiguous block used circularly.

What does it promise?
:   Stack: last in, first out. Queue: first in, first out. Nothing else is accessible.

What does it cost?
:   Every operation is $\Theta(1)$, amortized where the underlying block may grow.

How do you know?
:   Differential tests against `std::stack` and `std::queue`; for the ring buffer, tests that force the contents to wrap around the end and to grow while wrapped.
:::

## Exercises {#sec:links-ex .unnumbered}

1. Draw the list after each statement, including where `head_` points and every `next` pointer.

    ```cpp
    List<int> a;
    a.push_front(3);
    a.push_front(1);
    a.push_front(4);
    a.remove(1);
    a.reverse();
    ```

2. Each of these has a bug. Identify it and name its consequence (leak, use after free, lost nodes, or infinite loop).

    ```cpp
    // (a) push_front
    Node* fresh = new Node{value, nullptr};
    head_ = fresh;
    fresh->next = head_;

    // (b) clear
    for (Node* p = head_; p != nullptr; p = p->next) delete p;

    // (c) remove the node after p
    delete p->next;
    p->next = p->next->next;
    ```

3. Add `void push_back(T value)` to `List<T>`. First write it without changing the class's data members; what is its cost? Then add a `tail_` pointer and make it $\Theta(1)$. Which existing member functions must now also maintain `tail_`? (List them all before you look at the code; it is easy to miss one.)

4. Write a member function `T& nth(std::size_t n)` for `List<T>` that returns the element at position `n`. Then explain why this loop is $\Theta(n^2)$, and rewrite it to be $\Theta(n)$.

    ```cpp
    for (std::size_t i = 0; i < list.size(); ++i) total += list.nth(i);
    ```

5. Rewrite `List<T>::remove` using the pointer-to-pointer technique of `insert_sorted`, so that it has no special case for the first node.

6. Write a function that determines whether a singly linked list (given only a pointer to its first node) contains a cycle, using $O(1)$ extra memory. (Hint: use two pointers that advance at different speeds. This is known as Floyd's "tortoise and hare" algorithm.)

7. Write a function that merges two *sorted* singly linked lists into one sorted list by relinking the existing nodes, allocating nothing. What is its running time?

8. In `DList`, the sentinel node contains a `T value` that is never used. What does this require of the type `T`? Suggest a way to restructure `Node` so that the sentinel does not need to hold a `T`.

9. Implement `void splice(Iterator pos, DList& other)` for `DList<T>`: it moves *all* of `other`'s nodes into this list, just before `pos`, in $\Theta(1)$ time, leaving `other` empty. Check your work with `check_invariant` on both lists.

10. Implement a queue using two stacks: `push` pushes onto the first stack; `pop` pops from the second, and when the second is empty it first moves everything from the first stack onto it. Show that although a single `pop` can take $\Theta(n)$ time, the amortized cost of every operation is $\Theta(1)$. (Use the coin argument: how many times is each element moved in its lifetime?)

11. Extend `balanced` so that it ignores brackets inside double-quoted strings, so that `f("(")` is balanced. Then extend it to report the *position* of the first error.

12. Trace the `RingQueue` through: push 1 through 4 (the first push allocates a buffer of capacity 4); pop twice; push 5 and 6; push 7. For each step give `head_`, `size_`, and the contents of every slot. At which push does `grow` run, and what do the slots look like immediately afterward?

13. Run `traverse.cpp` and `sorted_insert.cpp` on your own machine. Then modify `sorted_insert.cpp` so that each element is a struct containing an `int` key and a `char payload[N]`, and repeat the experiment for N = 64, 256, and 1024. As the elements get bigger, shifting them costs more. Is there a size at which the list catches up? Report your numbers.

14. **With a model, as critic.** Write `DList<T>::reverse()`, which reverses the list in place, and test it. Then ask a model to "state the loop invariant of this function, and identify any input for which it fails to restore `p->next->prev == p` for every node." Check its claimed counterexamples by running them.

15. **Lab.** Implement a text editor's *line buffer* in two ways: as a `std::vector<std::string>` and as a `std::list<std::string>`. Support: insert a line at the cursor, delete the line at the cursor, move the cursor up and down, and jump to line $k$. Write a driver that replays a file of editing commands against either implementation. Construct two command files, one on which the vector is clearly faster and one on which the list is clearly faster, and explain each result in terms of this chapter. (The second file is harder to construct than you might expect. Why?)

## Chapter notes {#sec:links-notes .unnumbered}

Linked lists date from the mid-1950s; Knuth [@knuth1997] gives the history along with a thorough treatment of singly, doubly, and circularly linked variants. The idea of specifying a data type by its operations, independent of representation, was formulated by Liskov and Zilles [@liskov1974], and is one of the organizing ideas of software engineering.

The argument that contiguous storage should be the default on modern hardware has been made forcefully by Stroustrup, whose sorted-insertion experiment [@stroustrup2012] is reproduced in this chapter. The underlying facts about caches and memory are laid out by Drepper [@drepper2007].
