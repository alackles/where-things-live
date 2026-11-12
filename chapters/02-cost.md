# What Does It Cost? {#ch:cost}

::: goals
By the end of this week you should be able to:

- count the basic steps a piece of code performs as a function of its input size, in the best and worst cases;
- state the definitions of $O$, $\Omega$, and $\Theta$, and use them correctly;
- determine the growth rate of loops, including nested loops and loops that halve;
- write binary search from its invariant, and explain why it takes about $\log_2 n$ steps;
- time a program honestly, estimate its growth rate from a doubling experiment, and say why two programs with the same Big-O can differ by a factor of thirty.
:::

I am thinking of a whole number between 1 and 1,000,000. After each guess I will tell you whether my number is higher, lower, or equal.

::: check
How many guesses do you need, at most, to be certain of finding the number? Decide on a strategy and an actual figure before reading on.

::: answer
Twenty. Guess the middle of the range that remains; each answer rules out half of it. One million possibilities become 500,000, then 250,000, and after twenty halvings there is one left, because $2^{20} = 1{,}048{,}576$ is just over a million. If your strategy was to guess 1, 2, 3, ... you might need a million guesses.
:::
:::

A million guesses or twenty: same problem, same answer, and a difference in cost of a factor of fifty thousand. That gap had nothing to do with typing speed or processor speed. It came from the method, and from one fact about how the data was arranged: the numbers were in order, so a single comparison could discard half of them.

This chapter is about the third of the book's four questions. We need a way to talk about what an operation costs that does not depend on whose laptop it runs on, that lets us compare two approaches before writing either, and that tells us what will happen when the data gets a thousand times bigger. Then, having built that tool, we will take out a stopwatch and find out where it is wrong.

## Counting steps {#sec:counting}

Here is the guess-in-order strategy as code: search a vector for a value by looking at every element in turn.

```{.cpp include="ch02/search.hpp" region="linear"}
```

The function returns the index where it found the target. If the target is absent it returns `v.size()`, which cannot be a valid index and so serves as "not found." (The standard library uses the same convention, as we will see.)

How long does `linear_search` take? In seconds, the question has no good answer: it depends on the machine, the compiler, and what else the computer is doing. So we ask a better question. How many *steps* does it take, as a function of the size of the input?

By "step" we mean any operation that takes a fixed amount of time regardless of the data: comparing two integers, adding, assigning, following a pointer, indexing an array (recall from [[sec:pointers]] that indexing is one multiplication and one addition). The loop above does a handful of such steps per iteration: compare `i` with the size, index `v`, compare with the target, increment `i`. Whether a "handful" is four or seven turns out not to matter, so we simplify further and count just one representative operation, the comparison of an element against the target.

Let $n$ be the number of elements. Then the number of comparisons depends on where the target is.

- **Best case.** The target is the first element: 1 comparison.
- **Worst case.** The target is last, or absent: $n$ comparisons.
- **Average case.** If the target is present and equally likely to be anywhere, the search examines $(n+1)/2$ elements on average.

We will usually care most about the worst case. It is a guarantee: the operation never takes longer than this. It is also frequently the common case in practice, because searching for something that is not there is hardly unusual. The average case matters too, but it requires an assumption about what inputs are typical, and such assumptions have a way of being wrong. The best case is rarely interesting. Any algorithm can get lucky.

So: in the worst case, linear search takes a number of steps proportional to $n$. Double the data and you double the work. That simple statement is already the kind of thing we want to be able to say about every operation on every data structure.

## Growth rates {#sec:growth}

The reason to count steps as a function of $n$ is to learn how the cost *grows*. Here is what several common growth rates mean in practice, on a machine that performs a billion steps per second.

| $n$ | $\log_2 n$ | $n$ | $n \log_2 n$ | $n^2$ | $2^n$ |
|--:|--:|--:|--:|--:|--:|
| 10 | 3 ns | 10 ns | 33 ns | 100 ns | 1 μs |
| 100 | 7 ns | 100 ns | 660 ns | 10 μs | $4 \times 10^{13}$ years |
| 1,000 | 10 ns | 1 μs | 10 μs | 1 ms | |
| 1,000,000 | 20 ns | 1 ms | 20 ms | 17 minutes | |
| 1,000,000,000 | 30 ns | 1 s | 30 s | 32 years | |

Read down the columns. A logarithmic-time operation barely notices that the input grew from a thousand to a billion. A linear one goes from instantaneous to perceptible. A quadratic one goes from instantaneous to *thirty-two years*. And the exponential column runs out of universe almost at once. [[fig:growth]] plots the same functions; in the web edition you can change the scale and see how quickly the faster-growing curves leave the picture.

::: {.viz #fig:growth name="growth"}
Five growth rates. On any scale large enough to matter, each curve eventually towers over the ones below it, regardless of constant factors.
:::

Notice what the table did *not* need: the exact number of steps per loop iteration. If linear search took $7n$ steps and not $n$, the linear column would be seven times larger and the story would be exactly the same. The *shape* of the growth dominates everything else once $n$ is large. That observation is the basis for the notation used throughout computer science to describe costs.

### Big-O, and its relatives {#sec:bigo .unnumbered}

We say that a function $f(n)$ is $O(g(n))$, read "big-oh of $g$," if, for large enough $n$, $f$ is at most a constant multiple of $g$. Precisely:

> $f(n)$ is $O(g(n))$ if there are constants $c > 0$ and $n_0$ such that $f(n) \le c \cdot g(n)$ for every $n \ge n_0$.

The two constants each excuse something. The multiplier $c$ says that we do not care about constant factors. The threshold $n_0$ says that we do not care what happens for small inputs. [[fig:bigo]] shows the picture.

::: {.figure #fig:bigo}
The definition of Big-O in a picture. The function $f(n) = 3n + 40$ is $O(n)$: with $c = 5$, the line $5n$ lies above $f$ for every $n$ beyond $n_0 = 20$. What happens to the left of $n_0$ is ignored.
:::

For example, suppose a careful count shows that some function performs $f(n) = 3n^2 + 10n + 50$ steps. Then $f(n)$ is $O(n^2)$. To prove it, find the constants: for $n \ge 10$ we have $10n \le n^2$ and $50 \le n^2$, so $f(n) \le 3n^2 + n^2 + n^2 = 5n^2$. Take $c = 5$ and $n_0 = 10$.

In practice nobody hunts for constants. Two rules of thumb get you there directly: **keep only the fastest-growing term, and drop its coefficient.** So $3n^2 + 10n + 50$ becomes $n^2$; $n \log n + 100n$ becomes $n \log n$; $5$ becomes $1$, and we write $O(1)$ for anything that takes a bounded number of steps regardless of $n$, called **constant time**.

Big-O is an *upper bound*, and that leads to a subtlety. Linear search is $O(n)$. It is also, according to the definition, $O(n^2)$, and $O(2^n)$, in the same way that a person who is under two meters tall is also under three meters tall. True, but not informative. Two companion notations tighten things up.

- $f(n)$ is $\Omega(g(n))$ ("big-omega") if $f$ is *at least* a constant multiple of $g$ for large $n$: a lower bound.
- $f(n)$ is $\Theta(g(n))$ ("big-theta") if it is both $O(g(n))$ and $\Omega(g(n))$: $f$ grows at the *same rate* as $g$.

The worst-case cost of linear search is $\Theta(n)$: proportional to $n$, no more and no less. When programmers say "this is $O(n)$" they almost always mean $\Theta(n)$, and this book will follow that loose convention when no confusion can result, using $\Theta$ when the distinction matters.

::: warning
Three misreadings of Big-O are common enough to be worth naming.

*"$O(n)$ means the worst case."* No. Big-O describes a function; best, worst, and average case describe *which* function you are talking about. Linear search's best case is $\Theta(1)$ and its worst case is $\Theta(n)$. Both statements use the notation; they bound different things.

*"An $O(n)$ algorithm is faster than an $O(n^2)$ one."* Only for large enough $n$. The constants that Big-O discards are real. An algorithm that takes $1000n$ steps loses to one that takes $n^2$ steps for every $n$ below a thousand. Several real libraries switch to a "slow" quadratic sort when the input is small, for exactly this reason ([[ch:heaps]]).

*"Big-O tells you the running time."* It tells you how the running time *scales*. Two programs with the same Big-O can differ in speed by an order of magnitude, as the end of this chapter will demonstrate.
:::

The growth rates that turn up constantly have names.

| Notation | Name | Typical source |
|:--|:--|:--|
| $\Theta(1)$ | constant | indexing an array; arithmetic |
| $\Theta(\log n)$ | logarithmic | repeatedly halving: binary search |
| $\Theta(n)$ | linear | one pass over the data |
| $\Theta(n \log n)$ | "n log n," linearithmic | good sorting algorithms |
| $\Theta(n^2)$ | quadratic | all pairs; two nested loops |
| $\Theta(2^n)$ | exponential | trying every subset |

A note on logarithms, since they are about to be everywhere. $\log_2 n$ is the number of times you can halve $n$ before reaching 1, or equivalently the power to which 2 must be raised to give $n$. The base does not matter inside Big-O, because logarithms to different bases differ only by a constant factor ($\log_{10} n = \log_2 n / \log_2 10$), so we write simply $O(\log n)$. Outside Big-O, in this book, $\log$ with no base means base 2. [[app:math]] reviews the handful of facts about logarithms and sums that the book uses.

## Analyzing code {#sec:analyzing}

With practice you can read the growth rate of most code straight off its structure. A few patterns cover nearly everything in this course.

**Statements in sequence add.** A $\Theta(n)$ loop followed by another $\Theta(n)$ loop is $\Theta(n)$ in total. A $\Theta(n)$ loop followed by a $\Theta(n^2)$ loop is $\Theta(n^2)$: the larger term wins.

**Nested loops multiply.** If an outer loop runs $n$ times and its body is a loop that runs $n$ times, the innermost statement executes $n^2$ times.

**A loop whose inner bound depends on the outer variable needs a sum.** This function checks whether any value appears twice, by comparing every pair.

```{.cpp include="ch02/analysis.cpp" region="has_duplicate"}
```

In the worst case, with no duplicates, the inner loop runs $n-1$ times when $i = 0$, then $n-2$ times, and so on down to zero. The total number of comparisons is

$$(n-1) + (n-2) + \cdots + 1 + 0 = \frac{n(n-1)}{2}.$$

That sum, of the first so-many whole numbers, appears so often in the analysis of algorithms that you should recognize it on sight. It works out to $\frac{1}{2}n^2 - \frac{1}{2}n$, which is $\Theta(n^2)$. Comparing all pairs is quadratic even though each pair is examined only once.

**A loop that halves runs $\log n$ times.**

```{.cpp include="ch02/analysis.cpp" region="halvings"}
```

Starting from $n$ and halving until you reach 1 takes $\lfloor \log_2 n \rfloor$ steps: about 10 for a thousand, 20 for a million, 30 for a billion. Whenever an algorithm can throw away a constant *fraction* of its remaining work at each step, a logarithm appears.

::: check
What is the growth rate of this fragment, in terms of `n`?

```cpp
int count = 0;
for (int i = 1; i < n; i = i * 2) {
    for (int j = 0; j < n; ++j) {
        ++count;
    }
}
```

::: answer
$\Theta(n \log n)$. The outer loop variable doubles each time (1, 2, 4, 8, ...), so the outer loop runs about $\log_2 n$ times. The inner loop runs $n$ times on every one of those passes. Multiply.
:::
:::

### An example with two answers: insertion sort {#sec:insertion-sort .unnumbered}

Sorting is the subject of [[ch:heaps]], but one simple sorting algorithm is worth meeting now, because its analysis shows why we distinguish best and worst cases. **Insertion sort** is how most people sort a hand of playing cards: take the cards one at a time, and slide each into its proper place among the cards already in order.

```{.cpp include="ch02/analysis.cpp" region="insertion_sort"}
```

The outer loop always runs $n-1$ times. The inner loop is where the cost varies.

If the input is *already sorted*, then `v[j - 1] > x` is false immediately, every time. The inner loop body never runs. Total: about $n$ comparisons, $\Theta(n)$.

If the input is in *reverse order*, each new element is smaller than everything before it and must be slid all the way to the front: 1 shift, then 2, then 3, up to $n-1$. That is the same sum as before, $n(n-1)/2$ shifts, $\Theta(n^2)$.

So insertion sort is linear in the best case and quadratic in the worst, and on a random input it does about half the worst-case work, which is still quadratic. An algorithm like this, which does less work on inputs that are closer to sorted, is called *adaptive*, and that property will make insertion sort useful later despite its poor worst case.

::: llm
Reading growth rates off loops is a skill that comes from repetition, and a model makes a tireless drill partner. Use it as an examiner:

> I am practicing asymptotic analysis. Show me one short C++ loop nest at a time and ask me for its running time in Theta notation, in terms of n. Do not tell me the answer until I have given mine and my reasoning. Start easy and get harder. Include some where the inner loop's bound depends on the outer loop's variable, and some where a variable is multiplied or divided each time through.

Answer each one on paper first. When the model says you are wrong, do not simply accept it: make it show the sum.
:::

::: byhand
When you want to know the growth rate of a loop *you* wrote, work it out yourself before asking a model. For textbook shapes the model will be right. For unusual shapes it is easy to be led astray by a confident answer. Consider:

```cpp
for (int i = 1; i <= n; ++i)
    for (int j = i; j <= n; j += i)
        ++count;
```

Two nested loops that each "go up to `n`" look quadratic, and that is a tempting answer for person and model alike. But the inner loop takes steps of size `i`, so it runs about $n/i$ times, and the total is $n/1 + n/2 + n/3 + \cdots + n/n$, which is approximately $n \ln n$. The honest way to settle any such question is the one in [[sec:measuring]]: add a counter, run it for several values of $n$, and look at how the count grows.
:::

### Space is a cost too {#sec:space .unnumbered}

Everything above was about time, but the same notation describes memory. `linear_search` uses a couple of extra variables no matter how long the vector is: $O(1)$ extra space. A function that builds a copy of its input uses $\Theta(n)$ extra space. Time and space can often be traded against each other, and several data structures in this book (hash tables most of all) are, at bottom, schemes for spending memory to buy speed.

## Binary search {#sec:binary-search}

Now for the twenty-guesses strategy. It rests on a precondition: **the data must be sorted.** Given that, a single comparison against the middle element tells you which half the target must be in, and the other half can be discarded.

The idea is simple. Getting the details right is notoriously not. Jon Bentley, who taught the algorithm to professional programmers for years, reported that when he asked them to write it, given ample time, about nine in ten produced a version with a bug [@bentley2000]. He adds, drawing on Knuth's history of the algorithm [@knuth1998], that although the first binary search was published in 1946, it was 1962 before a published version was free of bugs. The trouble is always at the boundaries: an off-by-one error that skips an element, or a range that fails to shrink and loops forever.

The way to get it right is to decide, before writing any code, exactly what the variables *mean*, and then to make every line respect that meaning. A statement about the variables that is true every time execution reaches the top of a loop is called a **loop invariant**. Here is ours:

> If the target is anywhere in the vector, it is in the range of indices from `lo` up to but not including `hi`.

The range that includes its lower end and excludes its upper end is called a **half-open range**, written $[\mathit{lo}, \mathit{hi})$. Half-open ranges are the convention everywhere in C++ (recall "one past the end" from [[sec:pointers]]), and they make the arithmetic clean: the number of elements in the range is `hi - lo`, and the range is empty exactly when `lo == hi`.

```{.cpp include="ch02/search.hpp" region="binary"}
```

Check each line against the invariant.

*At the start*, `lo` is 0 and `hi` is `v.size()`, so the range is the whole vector, and the invariant holds trivially.

*In the loop*, `mid` is some index in the range. If `v[mid]` is the target, we are done. If `v[mid]` is too small then, because the vector is sorted, so is everything before it; the target can only be in $[\mathit{mid}+1, \mathit{hi})$, so we set `lo = mid + 1`. If `v[mid]` is too big, the target can only be in $[\mathit{lo}, \mathit{mid})$, so we set `hi = mid`. Note the asymmetry: `mid + 1` in one case and plain `mid` in the other. It comes directly from the range being half-open, and it is where most broken versions go wrong.

*At the end*, the loop exits when `lo == hi`: the range is empty. The invariant says the target, if present, is in an empty range. So it is not present.

One more thing has to be true: the loop must actually stop. It does, because `mid` is always a valid index within the range, and both branches produce a strictly smaller range.

[[fig:bsearch]] steps through a search and shows the invariant as a shaded region.

::: {.viz #fig:bsearch name="bsearch"}
Binary search for 57 in a sorted array of fifteen elements. The shaded cells are the half-open range `[lo, hi)` that might still contain the target. In the web edition, pick a target and step through the search; switch to linear search to compare the number of comparisons.
:::

::: warning
The midpoint is computed as `lo + (hi - lo) / 2` and not as the more obvious `(lo + hi) / 2`. When indices are stored in a 32-bit `int` and the array has more than about a billion elements, `lo + hi` can exceed the largest `int` and overflow. This is not a hypothetical: the binary search in Java's standard library had exactly this bug for nine years before anyone noticed [@bloch2006]. With 64-bit `std::size_t` indices the sum cannot realistically overflow, but the safe form costs nothing and is the idiom you should write by habit.
:::

### How fast? {#sec:binary-cost .unnumbered}

Each pass through the loop at least halves the size of the range. A range of $n$ elements can be halved only about $\log_2 n$ times before it is empty; the exact bound is $\lfloor \log_2 n \rfloor + 1$ passes. Binary search is $\Theta(\log n)$ in the worst case.

For a million elements that is 20 comparisons, against a million for linear search. For a billion elements, 30. The benefit of a logarithm is hard to overstate: you can double the size of the data and pay for it with *one* additional comparison.

### A more useful version {#sec:lower-bound-index .unnumbered}

The function above answers "is it there, and where?" A small change yields something more broadly useful: the position of the *first element that is not less than the target*. If the target is present, that is its first occurrence. If it is absent, that is where it would have to be inserted to keep the vector sorted.

```{.cpp include="ch02/search.hpp" region="lower_bound"}
```

The loop is even simpler, having two cases instead of three, and it has a new invariant, given in the comment. When the loop ends, `lo == hi`, and the invariant then says that everything before `lo` is smaller than the target and everything from `lo` onward is not. That is exactly the position we wanted.

The standard library provides this as `std::lower_bound` (and a sibling, `std::upper_bound`, that finds the first element *greater* than the target). In real programs, use those. They are correct, and they work on any sorted range.

::: byhand
Close the book and write binary search from scratch, starting from the invariant. Do it more than once this week, on different days. This is one of a small number of algorithms that you should be able to produce from understanding, without memory and without a model, because the reasoning pattern (state an invariant; make every line preserve it; check that the loop makes progress) is how every data structure in the rest of the book is built and debugged. If you let a model write it you will have a binary search, which you did not need, since the standard library already has one. What you need is the ability to think this way.
:::

### How do you know it is right? {#sec:differential .unnumbered}

Given the algorithm's history, your own confidence is not evidence. The test file for this chapter checks a few cases chosen by hand (an empty vector, a single element, targets at both ends, targets that are absent) and then does something more powerful.

```{.cpp include="ch02/search_test.cpp" region="differential"}
```

It generates thousands of small random sorted vectors and random targets, and for each one compares our function's answer with the answer from the standard library, which we trust. Any disagreement stops the program. This technique is called **differential testing**: checking an implementation against a reference, on many generated inputs [@mckeeman1998]. It finds boundary bugs that hand-picked examples miss, because random small inputs hit the awkward cases (empty ranges, duplicates, targets off either end) far more often than a person would think to. Small sizes are deliberate; bugs in this kind of code live at sizes zero through three.

We will use differential testing in every chapter from here on. It is the single most effective answer this book has to the question *how do you know?*, and it works just as well on code that a model wrote as on code that you wrote.

## Measuring {#sec:measuring}

Analysis predicts. Measurement checks the prediction, and sometimes overturns it. You should be able to do both.

The standard library's `<chrono>` header provides clocks. Here is the small timing function this book uses.

```{.cpp include="ch02/bench/timing.cpp" region="timer"}
```

It takes a piece of work, packaged as a function object (in practice a *lambda*, written `[&] { ... }`), runs it, and returns the elapsed time. `steady_clock` is the right clock for measuring intervals because it never jumps backward or gets adjusted.

A single timing tells you little. What you want is the growth rate, and the cleanest way to see it is a **doubling experiment**: time the code at size $n$, then $2n$, then $4n$, and look at the *ratio* of each time to the one before.

```{.cpp include="ch02/bench/timing.cpp" region="doubling"}
```

Here is what that program printed for `has_duplicate`, on the laptop this book was written on (a 2023 Intel Core i9-13900H, compiled with GCC 13 at `-O2`).

| $n$ | seconds | ratio to previous |
|--:|--:|--:|
| 4,000 | 0.0031 | |
| 8,000 | 0.0077 | 2.5 |
| 16,000 | 0.0284 | 3.7 |
| 32,000 | 0.1033 | 3.6 |
| 64,000 | 0.4246 | 4.1 |
| 128,000 | 1.6728 | 3.9 |

Doubling the input multiplies the time by about four. That is the signature of quadratic growth, since $(2n)^2 = 4n^2$. In general, if the time grows like $n^b$, then doubling $n$ multiplies the time by $2^b$: a ratio near 2 means linear, near 4 quadratic, near 8 cubic, and a ratio that hovers just above 1 means logarithmic or constant. The first ratios in the table are off because the runs are so short that noise and processor warm-up dominate; the pattern settles as the times get longer. With one more line of arithmetic you can extrapolate: at this rate a million elements would take about $1.67 \times (1{,}000{,}000 / 128{,}000)^2 \approx 100$ seconds.

And here are linear search and binary search, side by side, as average nanoseconds per search. (The binary search timed is `lower_bound_index`.)

| $n$ | linear search | binary search |
|--:|--:|--:|
| 1,000 | 165 ns | 38 ns |
| 10,000 | 1,600 ns | 51 ns |
| 100,000 | 16,200 ns | 67 ns |
| 1,000,000 | 162,000 ns | 100 ns |
| 10,000,000 | 2,190,000 ns | 214 ns |
| 100,000,000 | 20,200,000 ns | 402 ns |

The linear column grows tenfold with each row, as predicted. The binary column barely moves: a hundred thousand times more data, and the search takes about ten times as long. At a hundred million elements, binary search is fifty thousand times faster.

Look more closely at that second column, though. Theory says each tenfold increase in $n$ should add about 3.3 comparisons, a steady, gentle climb. From a thousand to a million elements that is roughly what happens. But from a million to a hundred million, $\log_2 n$ grows by only a third, while the measured time *quadruples*. Something that the step-counting model does not capture has started to matter. Hold that thought for one section.

::: warning
Timing code is easy to do badly. Four rules will keep you out of the worst trouble.

1. **Turn on optimization and turn off sanitizers.** Compile benchmarks with `-O2` and without `-fsanitize`. Unoptimized code can be ten times slower in ways that have nothing to do with your algorithm, and sanitizers add their own overhead. (Run your *tests* with sanitizers. Run your *timings* without.)
2. **Use the result.** If a program computes something and never uses it, the optimizer may delete the computation, and you will proudly measure an empty loop. The programs here print a checksum of their results for that reason.
3. **Make the run long enough.** Timings shorter than a few milliseconds are mostly noise. Use a bigger input or repeat the work many times.
4. **Run it more than once.** Other programs, power management, and the processor's own heat all perturb timings. Look for a pattern across runs and sizes, never at one number.
:::

::: llm
A model has never run your program on your machine, so any specific timing it offers ("binary search will be about 400 times faster") is a guess dressed as a fact. It can, however, review a benchmark you have written:

> Here is a C++ micro-benchmark I wrote. Without rewriting it, list the ways its measurements could be misleading: dead-code elimination, too-short runs, cache warm-up, anything else you see.

Then decide for yourself which of its objections apply. Performance questions are settled by measurement, on the machine that matters, and by nothing else.
:::

## When the model lies {#sec:model-lies}

The step-counting model made a quiet assumption: that every basic step costs about the same. For arithmetic that is close enough to true. For memory access it is false, and the way in which it is false will shape half the design decisions in this book.

Consider adding up every element of a square grid of numbers. We store the grid the way C++ programs usually do, as one long vector holding the first row, then the second row, and so on, so that the element in row $r$ and column $c$ of an $n \times n$ grid is at index $r \cdot n + c$. Here are two ways to compute the sum.

```{.cpp include="ch02/bench/rowcol.cpp" region="rows"}
```

```{.cpp include="ch02/bench/rowcol.cpp" region="cols"}
```

The two differ only in which loop is on the outside. Each visits every element exactly once and does one addition per element. By any step count they are identical: $\Theta(n^2)$ with the same constant.

::: check
Will the two versions take the same time? If not, which will be faster, and by roughly how much: ten percent? A factor of two? More?

::: answer
See the table that follows. Very few people guess the size of the effect the first time.
:::
:::

| $n$ | by rows | by columns | columns ÷ rows |
|--:|--:|--:|--:|
| 1,024 | 0.0004 s | 0.0027 s | 7 |
| 2,048 | 0.0012 s | 0.0095 s | 8 |
| 4,096 | 0.0056 s | 0.0755 s | 13 |
| 8,192 | 0.0228 s | 0.6936 s | 30 |

The column-order version is between seven and thirty times slower, and the gap widens as the grid grows. Same algorithm, same Big-O, same number of additions.

The explanation is in the hardware. Main memory is slow compared with the processor: fetching a value from it takes on the order of a hundred nanoseconds, time enough for the processor to execute hundreds of instructions. To hide this, processors keep copies of recently used memory in small, fast **caches**, arranged in levels; the fastest can supply a value in about a nanosecond. Two aspects of how caches work matter to us.

First, caches do not fetch single bytes. They fetch **cache lines**, blocks of (typically) 64 consecutive bytes. When your program touches one `int`, the fifteen neighboring `int`s on the same line arrive with it and are ready, essentially free, if you ask for them next.

Second, processors watch for patterns. If a program is marching through memory in order, the hardware notices and begins fetching the lines ahead before they are requested. This is called **prefetching**.

Now look at [[fig:rowcol]]. The row-order loop walks straight through the vector from beginning to end. Every cache line it pays for delivers sixteen useful values, and the prefetcher keeps the next lines arriving on schedule. The column-order loop, by contrast, takes a stride of $n$ elements between consecutive accesses. For large $n$, every single access lands on a different cache line, far from the last one, and by the time the loop returns to the neighborhood of an earlier access, that line has long since been evicted to make room for others. Nearly every access is a **cache miss**, a full-price trip to main memory. (The compiler can also use the processor's vector instructions on the simple row-order loop, which accounts for some of the difference. That, too, is a benefit of walking memory in order.)

::: {.figure #fig:rowcol}
The same grid, stored row after row in one block of memory, visited in two orders. Row order (top) walks through memory consecutively, so each 64-byte cache line that is fetched serves sixteen accesses in a row. Column order (bottom) jumps a whole row's width between accesses and lands on a different cache line every time.
:::

The property that the fast loop has and the slow loop lacks is called **locality**: its memory accesses are close together, and in a predictable order. This also explains the mystery from the last section. A binary search over a hundred million elements hops all over a 400-megabyte array; almost every probe is a cache miss. The count of comparisons grew gently, as the theory said. The cost of each one did not stay fixed.

Here, then, is the working attitude for the rest of the book.

**Big-O comes first.** No amount of cache-friendliness will rescue a quadratic algorithm from a linearithmic one when $n$ is large. The table at the start of [[sec:growth]] is still the most important table in the chapter.

**Among structures with the same Big-O, layout often decides.** A structure that keeps its data contiguous and visits it in order can be many times faster than one that scatters its data across memory and reaches it through pointers, even when a step count rates them equal. This is the first of the four questions, *where does it live?*, turning up inside the third. It is the main reason that the array-based structures of the next chapter are the default choice in modern software, and it is the lens through which [[ch:links]] will look at linked lists.

**When it matters, measure.** The theory tells you what to expect; the stopwatch tells you what happened. A professional uses both.

::: {.fourq title="A sorted array, with binary search"}
Where does it live?
:   One contiguous block. Any element can be reached by index in constant time, which is what lets binary search jump to the middle.

What does it promise?
:   The elements are in increasing order. That invariant is what makes a single comparison able to rule out half the data. The search itself is built around a loop invariant on the half-open range `[lo, hi)`.

What does it cost?
:   Searching: $\Theta(\log n)$ comparisons, against $\Theta(n)$ for an unsorted array. But keeping the array sorted is not free: inserting a new element means shifting everything after it, $\Theta(n)$. Much of this book is a search for structures that make *both* cheap.

How do you know?
:   Differential testing against `std::lower_bound` on thousands of small random inputs, plus hand-chosen boundary cases. For costs: count operations, and run a doubling experiment.
:::

## Exercises {#sec:cost-ex .unnumbered}

1. For each function, give the simplest $g(n)$ such that the function is $\Theta(g(n))$: (a) $5n^3 + 2n^2 + 100$; (b) $n \log n + 50n$; (c) $2^{10}$; (d) $n(n+1)/2$; (e) $\log_2(n^3)$; (f) $n + \sqrt{n}$; (g) $2^{n+1}$; (h) $(n + 1)(n + 2)(n + 3)$.

2. Prove from the definition that $4n^2 + 20n + 7$ is $O(n^2)$ by exhibiting constants $c$ and $n_0$. Then prove that it is *not* $O(n)$. (For the second part, suppose constants existed, and derive a contradiction for large $n$.)

3. Put these in order from slowest-growing to fastest-growing: $n^2$, $\log n$, $n!$, $2^n$, $n \log n$, $\sqrt{n}$, $n$, $1$, $n^3$, $\log \log n$.

4. Give the worst-case running time of each fragment in $\Theta$ notation, in terms of `n`. Show the sum where there is one.

    ```cpp
    // (a)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < 10; ++j) ++count;
    // (b)
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < i; ++j) ++count;
    // (c)
    for (int i = n; i > 0; i /= 2) ++count;
    // (d)
    for (int i = 0; i < n; ++i)
        for (int j = 1; j < n; j *= 2) ++count;
    // (e)
    for (int i = 1; i < n; i *= 2)
        for (int j = 0; j < i; ++j) ++count;
    ```

    Part (e) is the subtle one. Its answer is *not* $\Theta(n \log n)$.

5. Add a counter to the "harmonic" loop in the By hand first box of [[sec:analyzing]] and run it for $n$ = 1,000, 10,000, 100,000, and 1,000,000. Compare the counts with $n^2$, with $n \log_2 n$, and with $n \ln n$. Which fits?

6. Trace `binary_search` by hand on the vector `{3, 8, 12, 17, 21, 30, 44}` for targets 17, 3, 44, and 20. For each, list the values of `lo`, `hi`, and `mid` at every iteration.

7. Each of these changes to `binary_search` introduces a bug. For each, find a small input on which the function gives a wrong answer or never returns. (a) Replace `hi = mid` with `hi = mid - 1`. (b) Replace `lo = mid + 1` with `lo = mid`. (c) Replace `while (lo < hi)` with `while (lo <= hi)`.

8. Write `std::size_t upper_bound_index(const std::vector<int>& v, int target)`, returning the first index whose element is strictly greater than `target`. State its loop invariant in a comment. Then use it together with `lower_bound_index` to write a function that counts how many times `target` occurs in a sorted vector, in $O(\log n)$ time. Test both against the standard library using the differential-testing pattern from this chapter.

9. A sorted vector of $n$ distinct integers has been *rotated*: some number of elements were moved from the front to the back, as in `{30, 44, 3, 8, 12, 17, 21}`. Write a function that finds the index of the smallest element in $O(\log n)$ time. What is your loop invariant?

10. Binary search works on anything with the right "false, false, ..., false, true, true, ..., true" shape, not only on arrays. Write `int integer_sqrt(int n)` that returns the largest integer $r$ with $r^2 \le n$, using binary search on the range of possible answers. Take care that your arithmetic cannot overflow.

11. Run the row-versus-column experiment on your own machine. Do you get similar ratios to the table in [[sec:model-lies]]? Then change the grid's element type from `int` to `char`, and separately to `double`. Predict before you measure: how should the element size affect the gap?

12. Insertion sort does about $n^2/4$ shifts on a random input. Write a doubling experiment that confirms the quadratic growth. Then time it on inputs that are *already sorted*, and on inputs that are sorted except for ten randomly chosen elements. What do you conclude about when insertion sort is a reasonable choice?

13. **With a model, as critic.** Write your own binary search *without looking at this chapter's code*, and test it. Then ask a model: "Here is my binary search. Without rewriting it, state the loop invariant it appears to maintain, and tell me whether every line preserves that invariant." Compare the invariant it states with the one you had in mind. If they differ, who is right?

14. **Lab.** You are given three functions (your instructor will supply them, or use `std::sort`, `has_duplicate`, and a function of your own) and must determine each one's growth rate *empirically*. Build a timing harness using `seconds` that runs a doubling experiment on each, prints a table of sizes, times, and ratios, and reports an estimated exponent $b = \log_2(\text{ratio})$. Your write-up should state, for each function, your estimate and how confident you are, and should identify at least one thing you did to make the measurements trustworthy.

## Chapter notes {#sec:cost-notes .unnumbered}

Asymptotic notation was brought into the analysis of algorithms largely by Knuth, whose *The Art of Computer Programming* [@knuth1997; @knuth1998] remains the deepest treatment of the mathematics. For a thorough modern textbook account, with many more techniques than a ten-week course can cover, see Cormen, Leiserson, Rivest, and Stein [@clrs2022]. Sedgewick and Wayne's *Algorithms* [@sedgewick2011] puts unusual emphasis on doubling experiments and the scientific method, and influenced this chapter's approach to measurement.

Bentley's *Programming Pearls* [@bentley2000] has a wonderful chapter on writing binary search correctly, organized around the invariant; the story of the overflow bug in Java's library is told by the engineer who wrote that code [@bloch2006]. Differential testing was named and described by @mckeeman1998.

On the memory hierarchy, the standard reference for programmers is Drepper's long article *What Every Programmer Should Know About Memory* [@drepper2007]; Bryant and O'Hallaron [@bryant2016] cover the same ground at textbook pace, and Hennessy and Patterson [@hennessy2017] explain why the hardware is built this way.
