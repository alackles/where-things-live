# A Little Mathematics {#app:math}

The analysis in this book uses a small amount of mathematics, over and over. This appendix collects it in one place.

## Exponents and logarithms {#sec:math-logs}

The **logarithm** undoes exponentiation: $\log_b x$ is the power to which $b$ must be raised to give $x$.

$$\log_b x = y \quad\text{means exactly}\quad b^y = x.$$

In this book, $\log$ with no base means base 2. Three ways to think of $\log_2 n$ are worth having.

- It is the number of times $n$ can be **halved** before reaching 1.
- It is the number of times 1 must be **doubled** to reach $n$.
- It is, to within one, the number of **bits** needed to write $n$ in binary.

Since doubling grows very quickly, the logarithm grows very slowly.

| $n$ | 1 | 2 | 8 | 16 | 1,024 | about $10^6$ | about $10^9$ | about $10^{18}$ |
|:--|--:|--:|--:|--:|--:|--:|--:|--:|
| $\log_2 n$ | 0 | 1 | 3 | 4 | 10 | 20 | 30 | 60 |

Memorize the middle of that table: $2^{10} = 1024$ is about a thousand, so $2^{20}$ is about a million and $2^{30}$ about a billion.

The rules for logarithms mirror the rules for exponents.

| Exponents | Logarithms |
|:--|:--|
| $b^x \cdot b^y = b^{x+y}$ | $\log(xy) = \log x + \log y$ |
| $b^x / b^y = b^{x-y}$ | $\log(x/y) = \log x - \log y$ |
| $(b^x)^y = b^{xy}$ | $\log(x^y) = y \log x$ |
| $b^0 = 1$ | $\log 1 = 0$ |

Logarithms to different bases differ only by a constant factor:

$$\log_a x = \frac{\log_b x}{\log_b a}.$$

That is why the base is omitted inside Big-O: $O(\log_2 n)$, $O(\log_{10} n)$, and $O(\ln n)$ are all the same class. The *natural* logarithm, $\ln$, has base $e \approx 2.718$, and $\ln x \approx 0.693 \log_2 x$.

## Floors and ceilings {#sec:math-floor}

The **floor** $\lfloor x \rfloor$ is the largest integer not greater than $x$; the **ceiling** $\lceil x \rceil$ is the smallest integer not less than it. So $\lfloor 3.7 \rfloor = 3$ and $\lceil 3.2 \rceil = 4$. In C++, dividing one non-negative integer by another gives the floor of the quotient: `7 / 2` is 3. The number of elements in the left half of a range of $n$ elements split at `mid = n / 2` is $\lfloor n/2 \rfloor$, and the right half has $\lceil n/2 \rceil$.

## Sums {#sec:math-sums}

Three sums account for nearly every analysis in the book.

**The arithmetic sum.** Adding the first $n$ whole numbers:

$$1 + 2 + 3 + \cdots + n = \frac{n(n+1)}{2}.$$

To see why, write the sum forwards and backwards, one above the other. Each of the $n$ columns adds to $n + 1$, and the two rows together are twice the sum. This sum is $\Theta(n^2)$. It appears whenever a loop does 1 unit of work, then 2, then 3: nested loops over pairs, insertion sort, quicksort's worst case, growing a vector by a fixed amount.

**The geometric sum.** Adding successive powers of two:

$$1 + 2 + 4 + \cdots + 2^k = 2^{k+1} - 1.$$

Each term is one more than the sum of all the terms before it, so the total is just under twice the last term. This sum is dominated by its *largest* term. It appears whenever work doubles at each step: the copies made by a doubling vector, the nodes in a complete binary tree.

Run the other way, with terms that *halve*, the sum stays bounded however long it goes on:

$$1 + \frac{1}{2} + \frac{1}{4} + \frac{1}{8} + \cdots < 2, \qquad\text{and so}\qquad n + \frac{n}{2} + \frac{n}{4} + \cdots < 2n.$$

**The harmonic sum.** Adding the reciprocals:

$$1 + \frac{1}{2} + \frac{1}{3} + \cdots + \frac{1}{n} \approx \ln n.$$

It grows logarithmically. It appears in the analysis of quicksort and of random binary search trees.

## Induction {#sec:math-induction}

**Mathematical induction** proves that a statement holds for every whole number $n$ from some starting point on. It has two steps.

1. **Base case.** Show that the statement holds for the smallest $n$.
2. **Inductive step.** Show that *if* the statement holds for some $n$, *then* it holds for $n + 1$.

Together these give the statement for all $n$: the base case gives it for 1; the step then gives it for 2; then for 3; and so on, like a row of dominoes.

*Example.* Claim: $1 + 2 + \cdots + n = n(n+1)/2$ for every $n \ge 1$.

*Base case.* For $n = 1$ the left side is 1 and the right side is $1 \cdot 2 / 2 = 1$.

*Inductive step.* Suppose the claim holds for $n$. Then

$$1 + 2 + \cdots + n + (n+1) = \frac{n(n+1)}{2} + (n+1) = \frac{(n+1)(n+2)}{2},$$

which is the claim for $n + 1$.

Induction is the same idea as two things you do constantly in this course. A **recursive function** is correct for the same reason an inductive proof is: the base case is handled directly, and the recursive case is correct *assuming* the recursive calls are. And a **loop invariant** is an induction over the iterations of a loop: it holds before the first iteration, and if it holds before any iteration it holds after. When you check an invariant, you are doing an inductive proof.

A variant called *strong induction* assumes the statement for *all* smaller values, not just the previous one. It is what justifies recursion on halves, as in mergesort.

## Factorials {#sec:math-factorial}

The **factorial** $n! = 1 \cdot 2 \cdot 3 \cdots n$ is the number of different orders in which $n$ distinct things can be arranged. It grows faster than any exponential. Its logarithm, though, is tame:

$$\log_2 (n!) = \log_2 1 + \log_2 2 + \cdots + \log_2 n \approx n \log_2 n - 1.44\,n,$$

which is $\Theta(n \log n)$. This is the origin of the lower bound for sorting in [[sec:lower-bound]].

## A little probability {#sec:math-prob}

The analysis of hash tables, quicksort, and Bloom filters uses a few facts about chance.

The **expected value** of a random quantity is its average over all the ways the randomness could turn out, each weighted by its probability. If a fair die is rolled, the expected value is $(1 + 2 + 3 + 4 + 5 + 6)/6 = 3.5$.

**Expectations add.** The expected value of a sum is the sum of the expected values, *always*, whether or not the quantities are independent. This one fact does a remarkable amount of work. For example: throw $n$ keys into $m$ buckets uniformly at random. How many keys are expected to land in bucket 0? Each key lands there with probability $1/m$, contributing an expected $1/m$ to the count. Add up over $n$ keys: $n/m$. That is the load factor, with no further calculation needed.

**Waiting for a success.** If each attempt succeeds independently with probability $p$, the expected number of attempts up to and including the first success is $1/p$. If a fraction $\alpha$ of a hash table's slots are full, and probes landed on slots at random, each probe would find an empty slot with probability $1 - \alpha$, so about $1/(1-\alpha)$ probes would be needed. (Linear probing does worse than this, because its probes are not independent; see [[sec:probing]].)

**Collisions come early.** Among $k$ keys thrown into $m$ buckets there are about $k^2/2$ pairs, and each pair collides with probability $1/m$. So the expected number of colliding pairs is about $k^2 / 2m$, which reaches 1 when $k$ is about $\sqrt{2m}$. A collision is likely long before the table is full. This is the birthday paradox of [[sec:hash-idea]].
