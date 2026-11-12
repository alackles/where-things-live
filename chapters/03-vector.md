# Building a Vector {#ch:vector}

::: goals
By the end of this week you should be able to:

- implement a growable array as a C++ class with a pointer, a size, and a capacity, and state its representation invariant;
- explain why doubling the capacity makes `push_back` cost $O(1)$ *amortized*, and why growing by a fixed amount does not;
- write a destructor, copy constructor, copy assignment, move constructor, and move assignment for a class that owns memory, and say when a class needs none of them;
- turn a class into a template, and give it iterators so that range-`for` and the standard algorithms work on it;
- test a container by checking its invariant and by comparing it against a trusted reference on random operations.
:::

You have been using `std::vector` since the first day. It behaves like an array that grows: you call `push_back` as many times as you like and it always has room. But [[ch:memory]] established that an array is a block of memory of a fixed size, with other objects living on either side of it. There is no operation for making a block bigger where it stands.

::: check
A vector is holding four `int`s in a heap block with room for exactly four. The program calls `push_back`. What has to happen? And if the program then calls `push_back` a million more times, how many times in total do you think an element gets copied from one place to another?

::: answer
The vector must allocate a new, larger block, copy the four elements into it, delete the old block, and then add the fifth. The answer to the second question depends entirely on *how much* larger each new block is, and working that out is the centerpiece of this chapter. With the policy real vectors use, the answer is: about a million copies. With the most obvious policy, it is about five hundred *billion*.
:::
:::

This week we build `std::vector`. Not because the world needs another one, but because it is the perfect first data structure. It is simple enough to hold in your head and it is the most widely used container in all of C++. And building it properly requires nearly every important idea in the language's approach to memory: classes that own resources, constructors and destructors, copying and moving, templates, and iterators. We will go through all four of the book's questions with care, because the pattern set here repeats in every later chapter.

## Three numbers {#sec:vec-design}

A growable array needs a block of memory on the heap, and it needs to know two things about that block: how many elements it can hold, and how many it currently does hold. The trick that makes growth affordable is to let those two numbers differ. We allocate *more* room than we need, and keep track of how much is in use.

::: {.figure #fig:vec-layout}
The layout of a vector. The object itself is three machine words, wherever the variable lives. The elements live in a separate heap block. `size` counts the elements in use; `capacity` is how many the block can hold before it must be replaced.
:::

So the whole representation is three numbers, shown in [[fig:vec-layout]]: a pointer to the block, the **size**, and the **capacity**. We will build a version that holds `int`s first, called `IntVec`, and generalize later.

```{.cpp include="ch03/intvec.hpp" region="fields"}
```

If you are coming from Java or Python, the shape of a C++ class will be familiar: data members and member functions, some `public` (usable by anyone) and some `private` (usable only by the class's own functions). The trailing underscore on member names is a common convention, used throughout this book, to tell members from parameters and locals. The `= nullptr` and `= 0` are default initializers, so a freshly created `IntVec` starts out empty with no block at all.

### The promise {#sec:vec-invariant .unnumbered}

Before writing any operations, write down what must always be true of those three members. This is the **representation invariant**: the conditions that every public member function may assume when it starts and must re-establish before it returns.

1. `size_ <= capacity_`.
2. If `capacity_` is zero, `data_` is null. Otherwise `data_` points to a heap block with room for exactly `capacity_` elements, and this object is the only one responsible for deleting it.
3. The vector's contents are `data_[0]` through `data_[size_ - 1]`. Slots from `size_` onward are spare room and mean nothing.

Everything else follows from these three sentences. If you are ever unsure whether a line of code in a member function is right, the question to ask is whether the invariant holds afterward.

### Birth and death {#sec:ctor-dtor .unnumbered}

```{.cpp include="ch03/intvec.hpp" region="ctor_dtor"}
```

A **constructor** is a function with the same name as the class; it runs when an object is created, and its job is to establish the invariant. Ours has nothing to do beyond the default initializers, so we ask the compiler to generate it with `= default`.

A **destructor** is named with a tilde. It runs *automatically* at the end of the object's lifetime: when a local variable goes out of scope, when a containing object is destroyed, or when `delete` is applied to a heap object. Ours gives the block back. (`delete[]` on a null pointer is defined to do nothing, so an empty `IntVec` is handled correctly.)

That one-line destructor is the most important line in the chapter. It is what [[sec:lifetimes]] promised: heap memory tied to an object with automatic lifetime. A function that declares an `IntVec`, fills it, and returns, by any path, cannot leak the block, because the destructor runs no matter how the function exits. The general pattern, in which a constructor acquires a resource and the destructor releases it, goes by the unlovely name **RAII**, for "resource acquisition is initialization." It is the central idiom of C++, and the reason well-written modern C++ contains almost no explicit `delete`.

### Access {#sec:vec-access .unnumbered}

```{.cpp include="ch03/intvec.hpp" region="access"}
```

Three things here are new.

The `const` after a member function's parameter list is a promise that the function does not modify the object. Only such functions can be called on a `const IntVec&`, which is how you will usually receive a vector as a function parameter.

`operator[]` is how a class defines what square brackets mean. It returns a *reference* to the element, which is why `v[i] = 7` works: the assignment goes through the reference into the block. There are two versions. The first is used on a modifiable vector and returns a reference you can assign through. The second is used on a `const` vector and returns a reference you can only read.

And each begins with an `assert` stating its **precondition**: the index must be in range. `std::vector`'s own `operator[]` does not check, for speed. Ours does, because we are learning, and an assertion that fires at the scene of the crime is worth an hour of debugging.

### Growing {#sec:vec-push .unnumbered}

```{.cpp include="ch03/intvec.hpp" region="push_back"}
```

`push_back` is short. If there is spare room, the new element goes in the first unused slot and the size goes up by one. If the block is full, we first call a private helper to get a bigger block.

```{.cpp include="ch03/intvec.hpp" region="grow"}
```

`grow` does what the opening question said it must: allocate a larger block, copy the elements across, delete the old block, and update the pointer and the capacity. Check the invariant: afterward `data_` points to a block of `capacity_` slots that we own, the first `size_` of them hold the contents, and `size_ < capacity_`, so `push_back` can proceed.

The one real decision in `grow` is the first line. How much bigger should the new block be? The code doubles. The next section is about why.

## Growing, and what it costs {#sec:amortized}

Most calls to `push_back` do a constant amount of work: one comparison, one write, one increment. But a call that triggers `grow` copies every element: $\Theta(n)$ for a vector of size $n$. So the worst-case cost of `push_back` is $\Theta(n)$.

That sounds bad, and it is misleading. The expensive calls are rare, and they are rare in a very particular way. To see it, count the total work for a whole sequence of calls instead of the worst single call.

The program `growth.cpp` simulates $n$ calls to `push_back` under four growth policies and counts how many element copies `grow` performs in total.

```{.cpp include="ch03/bench/growth.cpp" region="simulate"}
```

| $n$ | grow by 1 | grow by 1000 | grow by half | double |
|--:|--:|--:|--:|--:|
| 1,000 | 499,500 | 0 | 2,137 | 1,023 |
| 10,000 | 49,995,000 | 45,000 | 24,284 | 16,383 |
| 100,000 | 4,999,950,000 | 4,950,000 | 276,521 | 131,071 |
| 1,000,000 | 499,999,500,000 | 499,500,000 | 2,099,753 | 1,048,575 |

Look at the last row. Growing by one slot at a time, a million appends cause half a *trillion* copies. Growing by a thousand slots at a time is a thousand times better and still terrible: half a billion. But doubling causes about one million copies for one million appends, roughly **one copy per element**. And look down the columns: when $n$ goes up by ten, the first two columns go up by a hundred, and the last two go up by ten.

### Why doubling works {#sec:doubling .unnumbered}

Under doubling, the capacities are 1, 2, 4, 8, and so on, and `grow` is called when the size reaches each of those. The copies it makes are

$$1 + 2 + 4 + \cdots + 2^k,$$

where $2^k$ is the last capacity that filled up, which is less than $n$. A sum of doubling terms is always one less than the next term: $1 + 2 + 4 + \cdots + 2^k = 2^{k+1} - 1$. Since $2^k < n$, the total is less than $2n$.

So $n$ calls to `push_back` do $n$ element writes plus fewer than $2n$ copies: fewer than $3n$ steps altogether. The *average* cost per call, over the whole sequence, is less than 3. A constant.

Here is another way to see the same thing, which you may find more convincing. Imagine that every call to `push_back` is charged three coins. One coin pays for writing the new element. The other two are left sitting on that element, saved. Now suppose a block of capacity $c$ has just filled. Half of its elements, $c/2$ of them, were added since the last time the vector grew, and each of those is carrying two saved coins: $c$ coins in all, exactly enough to pay for copying all $c$ elements into the new block. Every expensive operation has been paid for in advance by the cheap ones that led up to it.

[[fig:dynarray]] shows the process; in the web edition you can push elements one at a time under each policy and watch the cost per operation and the running average.

::: {.viz #fig:dynarray name="dynarray"}
Appending to a vector that doubles. Most appends write one element. The appends that find the block full (the fifth and ninth here) also copy everything into a block twice as large. The total number of copies stays below twice the number of elements.
:::

### Amortized cost {#sec:amortized-def .unnumbered}

This kind of reasoning has a name. We say that `push_back` has **amortized** cost $O(1)$: any sequence of $n$ calls takes $O(n)$ time in total, even though an individual call may take much longer than the average [@tarjan1985]. The word comes from accounting, where a large occasional expense is spread over the period that benefits from it.

Be careful to distinguish amortized cost from average-case cost. An average-case bound depends on an assumption about the inputs, that they are random in some way, and can fail if the assumption does. An amortized bound involves no probability and no assumption. It is a *guarantee* about every possible sequence of operations: the total can never exceed the bound. The only thing given up is the guarantee about each individual operation.

Sometimes that matters. If your program must respond within a millisecond every single time (controlling a motor, say, or producing an audio sample) then a `push_back` that occasionally stops to copy a hundred million elements is unacceptable, however good the average. For most programs, total time is what counts, and amortized $O(1)$ is as good as $O(1)$.

### Why growing by a constant fails {#sec:constant-growth .unnumbered}

If the capacity instead grows by a fixed amount $k$, the copies are $k + 2k + 3k + \cdots$, about $n/k$ terms in all. That is our old friend the sum of the first so-many whole numbers, and it comes to about $n^2/2k$. The total work is quadratic. A larger $k$ shrinks the constant but cannot change the shape, as the second column of the table shows.

The difference is easy to measure. Here are the times for $n$ appends under the "grow by 1000" policy and the doubling policy, on the same laptop as before.

| $n$ | grow by 1000 | double |
|--:|--:|--:|
| 250,000 | 0.0055 s | 0.0007 s |
| 500,000 | 0.0219 s | 0.0011 s |
| 1,000,000 | 0.0815 s | 0.0020 s |
| 2,000,000 | 0.3875 s | 0.0039 s |
| 4,000,000 | 2.4104 s | 0.0090 s |

Apply the doubling test from [[sec:measuring]]. The right-hand column roughly doubles from row to row: linear. The left-hand column roughly quadruples, and then some: quadratic. At four million elements the gap is a factor of 270.

The general principle is that growth must be **geometric**: each new capacity must be a constant *multiple* of the old one. Any multiplier greater than 1 gives amortized constant time. The choice of multiplier is a trade between time and space. A factor of 2 means fewer reallocations but up to half the block sitting idle. A factor of 1.5 wastes less space and copies somewhat more, as the table's third column shows.

::: wild
Every mainstream language's growable array uses geometric growth; they differ only in the factor. The GNU and LLVM implementations of `std::vector` double. Microsoft's grows by a factor of 1.5, as does Java's `ArrayList`. Python's `list` over-allocates by about one-eighth. The smaller factors spend a little more time copying in exchange for less idle memory.

When you know in advance roughly how many elements are coming, you can skip the intermediate reallocations altogether by calling `v.reserve(n)` first. It is one of the cheapest optimizations in C++, and you will see it in careful code everywhere.
:::

::: llm
Amortized analysis is an idea that often needs to be heard twice, in different words. Once you have read this section and can state the coin argument in your own words, try the *teach-back* pattern:

> I am going to explain why push_back on a doubling dynamic array is amortized O(1). Do not correct me until I am finished. Then tell me precisely which steps of my explanation were wrong or hand-wavy, and ask me one follow-up question that tests whether I really understand it.

Then explain it, out loud or in writing, without looking at the book. A good follow-up question is "what happens if the array also *shrinks* by half whenever it becomes half empty?" (The answer is in the exercises, and it is a trap.)
:::

## Copying {#sec:rule-of-three}

Our `IntVec` can be created, filled, and destroyed. Now consider this innocent line:

```cpp
IntVec b = a;
```

[[ch:memory]] said that initialization copies. But we have not told the compiler how to copy an `IntVec`, so it does the only thing it can: it copies each member. `b.data_` gets the value of `a.data_`. `b.size_` gets `a.size_`. Member by member, bit for bit.

::: check
After that member-by-member copy, draw the stack and heap. How many heap blocks are there? What happens when `b` and then `a` are destroyed?

::: answer
There is one block, with two `IntVec` objects pointing at it: the left side of [[fig:shallow-deep]]. A `push_back` through `b` would be invisible to `a`'s size, and if `b` grew, it would delete the block out from under `a`. Even if nobody touches either vector, when they go out of scope both destructors run, and both `delete[]` the same block: a double free.
:::
:::

::: {.figure #fig:shallow-deep}
Left: a member-by-member ("shallow") copy duplicates the pointer, so two objects believe they own one block. Right: a proper ("deep") copy allocates a second block and copies the elements, so the invariant holds for both objects.
:::

Here is a minimal class with exactly this flaw, and what the sanitizer says when one object is copied to another and both are destroyed.

```{.cpp include="ch03/bugs/shallow.cpp"}
```

```console
==26587==ERROR: AddressSanitizer: attempting double-free on 0x502000000010
    #1 0x6367f90ed71e in Naive::~Naive() ch03/bugs/shallow.cpp:7
    #2 0x6367f90ed3a1 in main ch03/bugs/shallow.cpp:17
```

The member-by-member copy broke clause 2 of our invariant: two objects are now responsible for the same block. To fix it we must tell the compiler what copying an `IntVec` means, by writing two special member functions.

```{.cpp include="ch03/intvec.hpp" region="copy"}
```

The **copy constructor** builds a brand-new object from an existing one. Its parameter is a `const` reference to an object of the same class. The part between the colon and the opening brace is a *member initializer list*, which sets up the members directly: a new block just big enough for the other vector's elements, and matching size and capacity. The body then copies the elements. The result is the right side of [[fig:shallow-deep]].

**Copy assignment**, `operator=`, handles `c = a` where `c` already exists and already owns a block. It has more to worry about: the old block must be freed, the object might be assigned to itself (`a = a`), and if allocating the new block fails, the object should be left as it was. The version here takes care of all three with a tidy idiom called *copy-and-swap*. It makes a copy of `other` using the copy constructor, then swaps the insides of `*this` with the copy. Now `*this` has the new contents, and the local variable `copy` holds our old block, which its destructor frees as the function returns. (`this` is a pointer to the object the member function was called on; `*this` is that object.)

We have now written three functions that manage the block: a destructor, a copy constructor, and a copy assignment operator. They travel together.

> **The rule of three.** If a class needs a hand-written destructor, copy constructor, or copy assignment operator, it almost certainly needs all three.

The reasoning: needing any one of them means that the class owns something that member-by-member handling gets wrong. The other two then get it wrong as well.

## Moving {#sec:move}

Copying is now correct. It is also sometimes wasteful. Consider a function that builds a vector and returns it.

```{.cpp include="ch03/intvec_test.cpp" region="use"}
```

When `squares(1000000)` returns, the local `result` is about to be destroyed. It would be absurd to copy its million elements into the caller's variable and then immediately throw the original away. The caller could simply *take over* the block.

That is a **move**. Since C++11 a class can define what it means to be moved from, with two more special member functions.

```{.cpp include="ch03/intvec.hpp" region="move"}
```

The parameter type `IntVec&&`, with two ampersands, is an *rvalue reference*. For our purposes it means: "an object that is about to be discarded, whose contents I am therefore allowed to take." The move constructor copies the three members, which costs three machine words, and then sets `other` to the empty state so that its destructor will not free the block we just took. No elements are touched. Moving a vector of a million elements costs the same as moving a vector of none.

The compiler uses these automatically wherever it can see that the source is a temporary or is about to go out of scope, as with `return result;`. When you want to move from a named object that the compiler would otherwise copy, you say so with `std::move`.

```{.cpp include="ch03/intvec_test.cpp" region="move_demo"}
```

Despite its name, `std::move` moves nothing. It is a cast, a note to the compiler that says "I am finished with `b`; you may treat it as discardable." After the move, `b` is still a valid object (its invariant holds) but its contents are gone. The only sensible things to do with a moved-from object are to destroy it or assign it a new value.

The `noexcept` on both functions promises that they will not throw exceptions. That promise matters more than it looks. When a `std::vector<IntVec>` grows, it will *move* its elements to the new block only if their move constructor is `noexcept`; otherwise it falls back to copying, to be safe. Forgetting the keyword silently costs performance.

With five special members, the rule of three becomes the **rule of five**: destructor, copy constructor, copy assignment, move constructor, move assignment. If you write one, consider all five.

### The rule of zero {#sec:rule-of-zero .unnumbered}

Having learned to write all five, you should now learn when to write none of them. Look at this class.

```cpp
struct Student {
    std::string name;
    std::vector<int> scores;
};
```

It has no destructor, no copy constructor, nothing. And it is entirely correct. The compiler-generated versions copy, move, and destroy member by member, and each *member* knows how to copy, move, and destroy itself properly. A `Student` can be copied (deeply), moved (cheaply), and destroyed (without leaks) for free.

> **The rule of zero.** If every member of a class manages itself, write none of the five special member functions.

::: warning
The rule of zero has one trap. The compiler-generated *move* moves each member, and moving a plain number just copies it. If a class keeps a count beside a container, say a `std::vector` and a separate `std::size_t` recording how many slots are in use, then after a move the source's vector is empty but its count is not. The C++ convention is that a moved-from object need only be safe to destroy or to assign a new value to, and that is all the classes in this book promise unless they say otherwise. Do not go on using an object you have moved from.
:::

This is the rule for nearly every class you will write outside this course. Build your classes from `vector`, `string`, and other self-managing types, and all the machinery of this section happens for you. The handful of classes that must follow the rule of five are the ones that directly own a raw resource, such as the containers in this book. That is the reason for building them: so that you understand what the self-managing types are doing on your behalf.

::: byhand
Before you study the copy and move functions in `intvec.hpp` closely, close the book and write all five special members for `IntVec` yourself, with a small `main` that exercises each one, and run it under the sanitizers until it is clean. You will get something wrong: forget to null out the source of a move, mishandle self-assignment, leak the old block in assignment. Each of those mistakes, caught by AddressSanitizer and then understood, teaches a piece of C++'s object model that no amount of reading does.

This is also the first place in the course where asking a model to write the code is seriously tempting, because the functions are fiddly and a model writes them flawlessly. That is exactly why it is the wrong place. The fiddliness *is* the content.
:::

::: llm
Once you have a version that you believe works, use a model as a critic:

> Here is a C++ class that owns a heap array. Do not rewrite it. For each of the five special member functions, tell me whether it is present and correct. If one is wrong, describe the bug and give me a short test program that would expose it under AddressSanitizer.

Run the test programs it gives you. If one of them does *not* fail, the model's claim was wrong, and you have learned something about both your code and the model.
:::

## Templates and iterators {#sec:templates}

`IntVec` holds `int`s. Nothing about its logic depends on that. A vector of `double`s or of `std::string`s would have the same three members and the same functions, with one type changed throughout. C++ lets you write the class once, with the element type left as a parameter.

```{.cpp include="ch03/vec.hpp" region="header"}
```

`template <typename T>` makes `Vec` a **class template**: a pattern from which the compiler generates a separate class for each element type you ask for. `Vec<int>` and `Vec<std::string>` are different classes, each compiled as if you had written it by hand with `T` replaced. This is quite different from generics in Java, where one compiled class serves all element types by treating them as references to objects. A C++ `Vec<int>` stores actual `int`s, side by side, with no indirection: the template costs nothing at run time.

One practical consequence is that the compiler needs to see the whole template wherever it is used, so templates live entirely in header files. That is why the code for this chapter is in `vec.hpp` and not split into a header and a source file.

The complete `Vec<T>` is in `code/ch03/vec.hpp`. It is `IntVec` with `int` replaced by `T`, plus the additions discussed below.

::: note
Our `Vec` takes one shortcut that the real `std::vector` does not. It allocates its block with `new T[capacity]`, which creates `capacity` fully formed `T` objects at once, including the spare ones. That requires `T` to have a default constructor, and it means the spare slots hold real (default-valued) objects. `std::vector` instead allocates *raw* memory and constructs each element individually at the moment it is added, using lower-level tools that are beyond this course. The costs and the interface are the same; the bookkeeping is more delicate. Our `Vec` is also less careful than the real thing about *exceptions*: if copying an element fails partway through a reallocation (a `std::string` running out of memory, say), `std::vector` guarantees that nothing is leaked and the vector is unchanged, and achieving that takes more machinery than we show.
:::

### Passing elements efficiently {#sec:sink .unnumbered}

```{.cpp include="ch03/vec.hpp" region="push_back"}
```

When `T` was `int`, passing by value was obviously fine. For a `T` like `std::string`, we would like to avoid copying when we can. This version takes the parameter by value and then *moves* it into the block. If the caller passes a temporary, such as `words.push_back("word" + suffix)`, the temporary is moved into `value` and then moved into the block: no copy at all. If the caller passes a named variable, it is copied once into `value` (as it must be; the caller still wants its own) and then moved. The growing logic has been factored into `reserve`, which also uses `std::move` to transfer elements to the new block.

Assignment gets the same treatment, and the result is pleasingly short.

```{.cpp include="ch03/vec.hpp" region="assign"}
```

Because the parameter is taken by value, the caller's argument is copied *or moved* into `other`, whichever is appropriate, before the function body runs. Then we swap. One function serves as both copy assignment and move assignment.

### Insert and erase {#sec:insert-erase .unnumbered}

A vector's elements are contiguous and in order. That is the source of its speed, and it has a price: putting a new element anywhere but the end means making a gap, and removing one means closing a gap.

```{.cpp include="ch03/vec.hpp" region="insert"}
```

::: {.figure #fig:insert-shift}
Inserting 25 at index 2. Every element from that position onward must first move one slot to the right, starting from the far end so that nothing is overwritten.
:::

The loop in `insert` runs from the back toward `index`, as [[fig:insert-shift]] shows; run it front to back and each element would overwrite the next before it had been moved. Inserting at position $i$ in a vector of $n$ elements moves $n - i$ elements. At the end, that is zero. At the front, it is all of them.

| Operation | Cost |
|:--|:--|
| `v[i]`, `at(i)` | $\Theta(1)$ |
| `push_back` | $\Theta(1)$ amortized; $\Theta(n)$ worst case |
| `pop_back` | $\Theta(1)$ |
| `insert` or `erase` at index $i$ | $\Theta(n - i)$: constant at the back, linear at the front |
| searching for a value | $\Theta(n)$, or $\Theta(\log n)$ if kept sorted |
| copy | $\Theta(n)$ |
| move | $\Theta(1)$ |

That fourth row is the vector's one real weakness, and the motivation for the linked structures of the next chapter.

### Iterators {#sec:iterators .unnumbered}

How should code outside the class visit every element? Indexing works for a vector, but not every container can be indexed efficiently. The C++ library's answer is the **iterator**: an object that marks a position in a container and can be advanced to the next position. Every container provides two.

- `begin()` returns an iterator to the first element.
- `end()` returns an iterator to the position *one past* the last element.

Together they describe a half-open range, exactly like `[lo, hi)` in binary search. An iterator supports `*it` to reach the element, `++it` to advance, and `!=` to compare. If that list of operations sounds familiar, it should. A pointer into an array supports all of them. For a vector, whose elements are contiguous, a plain pointer *is* a perfectly good iterator.

```{.cpp include="ch03/vec.hpp" region="iterators"}
```

Those four one-line functions buy a great deal. The range-based `for` loop is defined in terms of `begin()` and `end()`, so it now works on our class:

```{.cpp include="ch03/vec_test.cpp" region="range_for"}
```

And so does the whole standard library of algorithms, which are written to operate on *any* pair of iterators, from any container.

```{.cpp include="ch03/vec_test.cpp" region="algorithms"}
```

`std::sort` and `std::find` have never heard of `Vec`. They were handed two pointers and asked to work on the range between them. This separation of algorithms from containers, with iterators as the connecting interface, was the founding idea of the Standard Template Library [@stepanov1995], and it is why learning the iterator conventions pays off across all of C++.

::: warning
An iterator, pointer, or reference to a vector element is an address inside the vector's current block. When the vector grows, that block is deleted. Every iterator, pointer, and reference into it is left dangling. This is called **invalidation**, and it is the classic `std::vector` bug.

```{.cpp include="ch03/bugs/invalidate.cpp"}
```

```console
==26591==ERROR: AddressSanitizer: heap-use-after-free on address 0x502000000010
WRITE of size 4 at 0x502000000010 thread T0
    #0 0x5cc0fc8e69ff in main ch03/bugs/invalidate.cpp:12
```

The rule: after any operation that might change a vector's capacity (`push_back`, `insert`, `reserve`), or that shifts elements (`insert`, `erase`), do not use iterators, pointers, or references obtained before it. Indices remain meaningful; addresses do not. This also means you must never `push_back` onto a vector inside a range-`for` loop over the same vector.
:::

## How do you know? {#sec:vec-testing}

We have a container with a dozen operations, several of which rearrange memory. How confident should we be that it is right, and where does the confidence come from?

Start with the invariant. We wrote it down in English at the beginning of the chapter. Here is part of it as code.

```{.cpp include="ch03/vec.hpp" region="invariant"}
```

Not everything in the English version can be checked from inside the object. There is no portable way to ask whether a pointer really refers to a block of the claimed size, which is what the sanitizer is for. But what can be checked should be, after every operation in the tests.

Next, a reference. `std::vector` already exists and is trusted. If we perform the same sequence of operations on a `Vec<int>` and on a `std::vector<int>`, they should contain the same elements afterward, every time.

```{.cpp include="ch03/vec_test.cpp" region="same"}
```

And finally, volume. Instead of hand-writing a few sequences of operations, generate them at random, by the thousand.

```{.cpp include="ch03/vec_test.cpp" region="fuzz"}
```

This is the differential testing of [[sec:binary-search]], now applied to a whole data structure: a random sequence of operations, mirrored on a reference implementation, with a comparison after every step. The test for this chapter runs 200 such sequences of 400 operations each. All 80,000 operations run under AddressSanitizer, so a test passes only if the answers were right *and* no memory was misused on the way.

A few design points are worth copying into your own tests.

- **Small values, small sizes.** Insert positions are chosen from `0` to `size()` *inclusive*, so the edge cases (front, back, empty) come up constantly.
- **Weight the operations.** `push_back` gets two of the six slots, so the vector tends to grow and exercise reallocation, not hover near empty.
- **Use a seed.** The random generator is seeded with a known number, so a failure can be reproduced exactly by rerunning with the same seed.
- **Check after every step.** When the assertion fires, the bug is in the operation that just ran.

How good is the test? There is a direct way to find out: break the code on purpose. Change `i > index` to `i >= index` in `insert`, or delete the `++size_`, and run the test. If it still passes, the test is not looking where you thought it was. Deliberately injecting bugs to evaluate a test suite is called *mutation testing*, and it is the answer to "how do you know?" asked one level up: how do you know your tests would notice?

::: llm
Models are good at proposing test cases, and a useful second opinion on what you have forgotten to test.

> Here is the public interface of a vector class I wrote, and the tests I have so far. List specific cases my tests do not cover. Do not write the tests; describe each case in one sentence.

Then write those tests yourself. And be wary of the reverse arrangement, in which a model writes both the code and its tests. Tests written by the author of the code tend to share the author's blind spots, and that is as true of a model as of a person. A differential test against an independent reference has no such blind spot, which is why this book leans on it.
:::

::: {.fourq title="Vec<T>, and std::vector"}
Where does it live?
:   The object is three words: a pointer, a size, a capacity. The elements are in one contiguous heap block that the object owns. The block is replaced by a larger one when it fills.

What does it promise?
:   `size <= capacity`; elements `0` through `size - 1` are the contents, in order, contiguous. Any operation that may reallocate or shift elements invalidates iterators, pointers, and references into the vector.

What does it cost?
:   Indexing: $\Theta(1)$. `push_back`: amortized $\Theta(1)$ with geometric growth, though a single call can be $\Theta(n)$. Insert or erase at position $i$: $\Theta(n - i)$. Copy: $\Theta(n)$; move: $\Theta(1)$. Space: up to twice what the elements need. Cache behavior: as good as it gets.

How do you know?
:   An executable invariant check; differential testing against `std::vector` on random operation sequences; AddressSanitizer running under all of it; and mutation testing to check the tests.
:::

## Exercises {#sec:vector-ex .unnumbered}

1. Starting from an empty `IntVec`, list the capacity after each of the first ten calls to `push_back`. At which calls does `grow` run, and how many elements does each copy?

2. Using the coin argument, explain why charging *two* coins per `push_back` is not enough to pay for doubling. Then find the smallest number of coins per `push_back` that suffices when the growth factor is 1.5, and when it is 3.

3. `Vec::pop_back` never releases memory. Suppose we "fix" this by halving the capacity whenever the vector becomes exactly half full. Describe a sequence of `push_back` and `pop_back` calls, starting from a full vector of capacity $c$, in which *every* call triggers a reallocation. What is the amortized cost per operation in that sequence? What shrinking rule avoids the problem? (Hint: shrink at one quarter.)

4. Add `void resize(std::size_t n)` to `IntVec`: afterward the size is `n`; if the vector grew, the new elements are zero. Add `void shrink_to_fit()`, which reallocates so that capacity equals size. State which parts of the invariant each must be careful to restore.

5. What does this print, and why? What would it print if `IntVec` had no user-written copy constructor (assuming the program did not crash)?

    ```cpp
    IntVec a;
    a.push_back(1);
    IntVec b = a;
    b[0] = 2;
    std::cout << a[0] << ' ' << b[0] << '\n';
    ```

6. In `IntVec`'s copy assignment operator, what specifically goes wrong on `a = a` if you remove the `if (this != &other)` test *and* replace the copy-and-swap body with: delete the old block, allocate a new one, copy the elements from `other`?

7. For each class, say whether it should follow the rule of zero or the rule of five, and why: (a) a `Matrix` holding `std::vector<double> cells; int rows; int cols;`; (b) a `Buffer` holding `char* bytes; std::size_t length;` where `bytes` was allocated with `new[]`; (c) a `Roster` holding `std::vector<Student> students; std::string course;`.

8. Suppose `Vec::push_back` were declared as `void push_back(const T& value)` and began by calling `reserve` when full, as ours does. Explain how `v.push_back(v[0])` could then write garbage into the vector. Why is the by-value version in this chapter immune?

9. Implement `Vec<T>::erase(std::size_t first, std::size_t last)`, which removes the elements in the half-open index range `[first, last)` in a single pass, moving each later element exactly once. What is its cost? Why is calling the one-element `erase` in a loop, to remove $k$ elements from the front, much worse?

10. Write a function template `template <typename T> void remove_all(Vec<T>& v, const T& value)` that removes every element equal to `value` in $\Theta(n)$ total time. (Hint: keep two indices, one reading and one writing. This is the idea behind the standard library's `std::erase`.)

11. Add the differential-test operations `clear`, copy construction, and assignment to the `fuzz` function in `vec_test.cpp`. Then perform mutation testing: introduce each of these bugs into `vec.hpp`, one at a time, and record whether the tests catch it and how. (a) `reserve` copies only `size_ - 1` elements. (b) `erase` forgets `--size_`. (c) `insert`'s loop runs `i > index + 1`. (d) the move constructor does not null out `other.data_`.

12. **With a model, as examiner.** Ask a model to quiz you on iterator invalidation: "Show me short snippets that use `std::vector`, one at a time, and ask me whether each one has undefined behavior due to invalidation. Don't reveal the answer until I respond." Do ten. Verify any answer you doubt by compiling the snippet with AddressSanitizer.

13. **Lab.** Implement `Vec<T>` yourself, from an empty file, with the full rule of five, `push_back`, `pop_back`, `operator[]`, `at`, `insert`, `erase`, `reserve`, `begin`, `end`, and `check_invariant`. Write a differential fuzz test against `std::vector` for both `Vec<int>` and `Vec<std::string>`. Then add a constructor parameter or template parameter for the growth factor, measure the time for ten million `push_back`s at factors 1.25, 1.5, 2, and 4, and report both the time and the peak capacity for each. Which factor would you choose, and what does the choice depend on?

## Chapter notes {#sec:vector-notes .unnumbered}

Amortized analysis was given its name and its systematic form by Tarjan [@tarjan1985]; the coin argument here is the "banker's method" from that paper. Cormen et al. [@clrs2022] devote a chapter to the technique and analyze the dynamic table, including the shrinking problem of Exercise 3, in detail.

The design of `std::vector`, and of the iterator-based library around it, is due to Alexander Stepanov and his collaborators [@stepanov1995]. Stroustrup's *A Tour of C++* [@stroustrup2022] walks through the construction of a vector class much as this chapter does, at greater speed, and is a good second account. For move semantics, rvalue references, and the many subtleties passed over here, the standard guide is Meyers's *Effective Modern C++* [@meyers2014]. The rules of three, five, and zero are stated in the C++ Core Guidelines [@coreguidelines].

Testing with randomly generated inputs checked against stated properties was popularized by the QuickCheck library for Haskell [@claessen2000], and the idea has since spread to every major language under the name *property-based testing*.
