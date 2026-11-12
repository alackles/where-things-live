# Memory Is the Material {#ch:memory}

::: goals
By the end of this week you should be able to:

- compile and run a C++ program with warnings and sanitizers turned on;
- explain what an object's type, size, address, and lifetime are;
- predict when C++ copies an object and when it shares one, and choose between passing by value, by reference, and by `const` reference;
- read and write code that uses pointers, including pointer arithmetic on arrays;
- draw the stack and the heap for a short program, and recognize a leak, a dangling pointer, and an out-of-bounds access when you see one.
:::

Here are two programs. The first is Python.

```python
a = [1, 2, 3]
b = a
b.append(4)
print(len(a))
```

The second is C++, and does what looks like the same thing.

```{.cpp include="ch01/copies.cpp" region="copy"}
```

::: check
The Python program prints the length of `a`. The C++ program prints the sizes of `a` and `b`. What does each print? Commit to an answer before you read on.

::: answer
Python prints `4`. C++ prints `3 4`. In Python (and Java, and JavaScript), `b = a` makes `b` a second name for the *same* list, so appending through `b` changes what `a` sees. In C++, `b = a` makes a *new* vector with its own copy of the three elements; `a` never hears about the fourth.
:::
:::

Neither language is wrong. They made different decisions about what a variable *is*. In Python, a variable is a label tied to an object that lives somewhere else, and assignment ties a second label to the same object. In C++, a variable *is* the object: a region of memory with a name, and assignment copies the contents of one region into another.

That difference is the whole reason this course is taught in C++. A data structure is an arrangement of memory, and C++ is a language in which you can see, and must decide, how memory is arranged. This chapter is a fast tour of the machinery for doing that: objects, references, pointers, the stack, and the heap. It moves quickly, because you already know how to program. What you may not know yet is where your programs have been keeping things.

## A first program, and the machine that runs it {#sec:first-program}

Tradition demands this one.

```{.cpp include="ch01/hello.cpp"}
```

To run it you first **compile** it: a program called a compiler translates the text into machine instructions and writes them to a file, which you then run.

```console
$ g++ -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined hello.cpp -o hello
$ ./hello
Hello, CMSC 270!
```

If you have only used Python, notice what is different. There is no interpreter standing between your program and the processor while it runs. The translation happens once, ahead of time, and whole categories of mistakes (a misspelled name, a string where a number should be) are reported by the compiler before the program ever starts. The price is the extra step, and error messages that take some practice to read.

A few words about the pieces. `#include <iostream>` pulls in the declarations for input and output. Execution begins at the function named `main`, which returns an `int` to the operating system: zero for success. `std::cout` is the standard output stream; `<<` sends things to it. The `std::` prefix says that `cout` belongs to the standard library's namespace. Statements end with semicolons, and blocks are marked with braces, not indentation.

The long compile command is worth understanding once, because you will use it all term.

| Flag | Meaning |
|:--|:--|
| `-std=c++20` | use the 2020 version of the language |
| `-Wall -Wextra` | turn on the compiler's warnings about suspicious code |
| `-g` | include debugging information, so that tools can name lines |
| `-fsanitize=address,undefined` | add run-time checks for memory errors and undefined behavior |
| `-o hello` | name the output file `hello` |

The middle three are your safety equipment. [[sec:tools]] shows what they catch, and [[app:toolchain]] covers installation and has more on reading what the compiler says.

::: llm
Compiler errors in C++ can be long and strange, and explaining them is something language models do well. When you are stuck on one, try:

> Here is a C++ compiler error and the code that produced it. Explain in plain language what the compiler is complaining about and which line is the real cause. Do not give me corrected code; I want to fix it myself.

The last sentence matters. An explanation teaches you to read the next error on your own. A silently corrected file teaches you nothing, and you will meet the same error again next week.
:::

## Objects are bytes {#sec:bytes}

A computer's memory is, to a very good approximation, one enormous row of numbered boxes. Each box is a **byte**: eight bits, enough to hold a number from 0 to 255. The number of a box is its **address**. A laptop with 16 gigabytes of memory has about sixteen billion boxes.

That is all there is. Every integer, every string, every image, every tree and hash table in this book is, in the end, a pattern of values in some of those boxes. An **object**, in C++, is a region of memory that the program is using to hold a value. Every object has four properties, and this chapter is organized around them:

- a **type**, which says how to interpret its bytes;
- a **size**, the number of bytes it occupies;
- an **address**, the number of its first byte;
- a **lifetime**, the stretch of the program's execution during which it exists.

The `sizeof` operator reports sizes.

```{.cpp include="ch01/sizes.cpp" region="sizes"}
```

On the machine this book was written on, and very likely on yours, that prints:

```console
char:      1
bool:      1
int:       4
long long: 8
double:    8
int*:      8
```

The standard guarantees only that a `char` is one byte and that the other integer types are at least certain minimum sizes; the numbers above are what current desktop and laptop systems use.

So an `int` is four bytes: 32 bits. [[fig:bytes]] shows an `int` holding 270 and a `char` holding the letter A, sitting in memory.

::: {.figure #fig:bytes}
Two objects in memory. The `int` occupies four consecutive bytes; 270 is `0x0000010E` in hexadecimal, and this machine stores the low-order byte first. The `char` occupies one byte; 65 (`0x41`) is the code for `A`. Nothing in the bytes themselves says which is a number and which is a letter. The type does that.
:::

Two consequences follow at once.

**The type is not stored in memory.** The four bytes of an `int` and the four bytes of a `float` look the same. It is the compiler, which knows the declared type of every variable, that decides to generate integer-addition instructions for one and floating-point instructions for the other. This is why C++ wants every variable's type declared, or deducible, at compile time.

**Fixed size means limited range.** Thirty-two bits can distinguish about 4.3 billion values. For a signed `int` that is the range from −2,147,483,648 to 2,147,483,647. Adding 1 to the largest `int` does not produce a bigger number, because there is no room for one. For *unsigned* types the result wraps around to zero, reliably. For signed types the language says something more alarming, which we will come back to: the behavior is *undefined*.

You can ask where an object lives with the address-of operator, `&`.

```{.cpp include="ch01/sizes.cpp" region="addresses"}
```

```console
n lives at 0x7ffd5ce7b7cc
c lives at 0x7ffd5ce7b7cb
d lives at 0x7ffd5ce7b7d0
```

Addresses are conventionally printed in hexadecimal. The exact values will differ on your machine and from run to run, but look at the pattern: `c` is at an address ending `cb`, `n` occupies the four bytes from `cc` through `cf`, and `d` begins at `d0`. The three variables are packed side by side. Memory is a row of boxes, and you have just read the numbers off three of them. (This output is from a build *without* `-fsanitize`. AddressSanitizer deliberately spaces local variables apart, with guard zones between them, so that it can catch one overrunning into the next; under it you will see larger gaps.)

::: note
One type deserves early mention. Sizes and indices in the standard library are of type `std::size_t`, an unsigned integer big enough to count every byte of memory (eight bytes on current machines). `v.size()` returns one. Because it is unsigned, it cannot be negative, and comparing it with a plain `int` earns a compiler warning, since a negative `int` would be silently reinterpreted as a huge positive number. The habit this book follows: use `std::size_t` for anything that is a size or an index into a container.
:::

## Variables are places {#sec:places}

Return to the puzzle that opened the chapter. In C++ a variable names a region of memory, and initialization or assignment from another variable **copies**: the bytes that make up one object's value are duplicated into the other. Afterward the two objects have nothing to do with each other.

That is obviously what should happen for an `int`. C++ applies the same rule to everything, including types you define yourself.

```{.cpp include="ch01/copies.cpp" region="point"}
```

```{.cpp include="ch01/copies.cpp" region="struct_copy"}
```

A `struct` is a type made by placing other objects side by side; a `Point` is two `double`s, sixteen bytes. Copying it copies the sixteen bytes. For a `std::vector`, copying means allocating a new block of memory and copying every element into it, which is exactly why `a` and `b` in the opening example went their separate ways. This behavior is called **value semantics**: variables hold values, and copies are independent. The left side of [[fig:copy-alias]] shows the result.

::: {.figure #fig:copy-alias}
Left: in C++, `b = a` makes `b` a separate vector with its own storage, so appending to `b` leaves `a` alone. Right: a reference is a second name for the same object. The Python and Java meaning of `b = a` is the picture on the right.
:::

Value semantics is safe and predictable: nobody can change your object behind your back. It is also sometimes exactly what you do not want. A function that is supposed to modify the caller's vector cannot do so if it only receives a copy. And copying a vector of a million elements, merely to add them up, does a million elements' worth of pointless work.

### References {#sec:references .unnumbered}

For those situations C++ has **references**. A reference is another name for an existing object.

```{.cpp include="ch01/copies.cpp" region="reference"}
```

The `&` after the type makes `alias` a reference to an `int` and not an `int` of its own. No new integer is created. From that line on, `alias` and `score` are two names for one region of memory. A reference must be bound to an object when it is created, and it stays bound to that object for its whole life.

References earn their keep in function parameters. There are three ways to pass an argument, and choosing among them is a decision you will make in every function you write for the rest of the course.

```{.cpp include="ch01/copies.cpp" region="three_ways"}
```

The `const` in the middle version is a promise, enforced by the compiler, that the function will not modify the object it has been lent. This gives you the cheapness of sharing with the safety of copying, and it is the right default for anything larger than a number.

| You want to... | Pass by | Spelled |
|:--|:--|:--|
| read a small, cheap thing (`int`, `double`, `char`, a pointer) | value | `int x` |
| read anything bigger | `const` reference | `const std::vector<int>& v` |
| modify the caller's object | reference | `std::vector<int>& v` |

::: check
What is wrong with this function, which is meant to set every element of a vector to zero?

```cpp
void clear_all(std::vector<int> v) {
    for (int& x : v) x = 0;
}
```

::: answer
It takes `v` by value, so it zeroes a private copy and then throws the copy away. The caller's vector is untouched. The parameter should be `std::vector<int>& v`. The compiler will not warn you: modifying your own copy is perfectly legal, merely useless.
:::
:::

The range-based `for` loop follows the same rules. `for (int x : v)` copies each element into `x` in turn; `for (int& x : v)` makes `x` a reference to each element, so assigning to `x` changes the vector; `for (const std::string& s : names)` reads each string without copying it.

## Pointers: addresses as values {#sec:pointers}

You saw that `&n` produces the address of `n`. An address is just a number, and numbers can be stored. A **pointer** is an object whose value is the address of another object.

```{.cpp include="ch01/pointers.cpp" region="basics"}
```

Read `int* p` as "`p` is a pointer to `int`." The unary `*` operator **dereferences** a pointer: `*p` means "the object whose address is in `p`." So `*p = 271` stores 271 into `n` by way of its address.

A pointer can also hold the special value `nullptr`, meaning that it points at nothing. Dereferencing a null pointer is an error; on most systems the program crashes immediately, which is the *good* outcome, as we will see.

For pointers to structs there is a shorthand that you will type thousands of times this term.

```{.cpp include="ch01/pointers.cpp" region="arrow"}
```

Pointers and references are close relatives: both let one piece of code reach an object that lives somewhere else. The differences are what make each useful.

| | Reference | Pointer |
|:--|:--|:--|
| Can be null? | no | yes (`nullptr`) |
| Can be re-aimed at a different object? | no | yes |
| Syntax to reach the object | none; use it like the object | `*p` or `p->member` |
| Typical use | function parameters | links inside data structures; optional or changing targets |

A linked list node must be able to say "there is no next node," and must be able to change which node comes next. Those are things only a pointer can do, and it is why from [[ch:links]] onward this book is full of pointers.

### Arrays and pointer arithmetic {#sec:arrays .unnumbered}

The oldest data structure is the **array**: some number of objects of the same type, placed side by side in memory with no gaps.

```{.cpp include="ch01/pointers.cpp" region="array"}
```

Because the elements are the same size and are contiguous, finding element `i` requires no searching. If the array starts at address $A$ and each element is $s$ bytes, then element $i$ is at address $A + i \cdot s$: one multiplication and one addition, however long the array is. [[fig:array-arith]] shows the arithmetic.

::: {.figure #fig:array-arith}
An array of five `int`s beginning at address 1000. Each `int` is four bytes, so `a[3]` is at 1000 + 3 × 4 = 1012. The pointer expression `a + 3` computes that address; the compiler does the multiplying by four for you.
:::

Adding an integer to a pointer moves it forward by that many *elements*, not that many bytes; the compiler scales by the element size. And the indexing notation `a[i]` is *defined* to mean `*(a + i)`. Indexing is pointer arithmetic with nicer punctuation.

Hold on to this fact. It is the first cost result of the course, and one of the most important:

> **Reaching an element of an array, given its index, takes the same small amount of time no matter how large the array is.**

Everything that makes arrays, vectors, hash tables, and heaps fast comes back to it.

Pointers can walk through an array, too.

```{.cpp include="ch01/pointers.cpp" region="walk"}
```

The address `a + 5` does not belong to the array; it is the position just past the last element. Forming that "one past the end" address is permitted, precisely so that loops like this can use it as a stopping point. Dereferencing it is not. This pattern, a pair of positions marking a beginning and a just-past-the-end, is how all of C++'s standard library describes ranges, and [[ch:vector]] returns to it.

### No one checks {#sec:ub .unnumbered}

What happens if you write `a[7]`, when `a` has five elements?

The compiler computes $A + 7 \cdot 4$ and reads whatever is there. Perhaps it is another variable. Perhaps it is bookkeeping data that some other part of the program depends on. There is no check that the index is in range, because a check costs time on every access, and C++ is built on the principle that you do not pay for what you do not ask for.

The C++ standard calls a program that does this, or dereferences a null pointer, or overflows a signed integer, a program with **undefined behavior**. The term is exact: the language places *no requirements at all* on what such a program does. It may crash. It may print garbage. It may appear to work perfectly for months and then fail when the surrounding code changes. The compiler is allowed to assume that undefined behavior never happens and to optimize on that assumption, which can produce results that seem to defy logic.

::: warning
"It ran and printed the right answer" is not evidence that a C++ program is correct. A program with undefined behavior often prints the right answer, on your machine, today. This is the single most important difference between C++ and the languages you already know, where an out-of-range index reliably raises an error.
:::

This sounds like a terrible deal, and without tools it would be. [[sec:tools]] introduces the tools.

::: llm
Undefined behavior is a good way to calibrate how far to trust a model's reading of code. Take one of the small buggy programs from this chapter's `code/ch01/bugs/` directory and ask:

> What does this C++ program print? Answer precisely.

A careful answer says that the program has undefined behavior and that no particular output is guaranteed. A careless one names a specific number. Try it a few times, and with a few different programs. You are not trying to catch the model out for sport; you are learning what kind of question tempts it to answer more confidently than the facts allow.
:::

## Lifetimes: the stack and the heap {#sec:lifetimes}

The fourth property of an object is its lifetime: when it comes into existence and when it ceases to exist. C++ gives you two main kinds of lifetime, and they correspond to two regions of memory.

### Automatic storage: the stack {#sec:mem-stack .unnumbered}

A variable declared inside a function is created when execution reaches its declaration and destroyed when execution leaves the enclosing block. You do nothing to make this happen, which is why such variables are said to have **automatic** storage.

```{.cpp include="ch01/lifetime.cpp" region="frames"}
```

Automatic variables live in a region called the **stack**. When a function is called, a block of memory called a **stack frame** is set aside for its parameters and local variables, immediately beyond the frame of the function that called it. When the function returns, its frame is released, and the next call will reuse the same memory. Calls nest, so frames pile up and come off in last-in-first-out order, like a stack of plates. That is where the name comes from, and in [[ch:links]] we will meet the stack again as a data structure in its own right.

When `main` calls `sum_of_squares(3, 4)`, which calls `square(3)`, there are three frames on the stack. The variable `result` exists only while `square` is running. Its *value* is copied back to the caller as the return value; then the object itself is gone.

Stack allocation is extremely cheap, since claiming a frame is a single adjustment of one processor register, and it requires no bookkeeping from you. It has two limitations. The stack is small, typically a few megabytes, so it is no place for a million-element array. And lifetimes are tied rigidly to function calls: nothing on the stack can outlive the function that created it.

::: warning
That second limitation has teeth. A function must never return a reference or pointer to one of its own local variables:

```cpp
int& biggest(int a, int b) {
    int result = (a > b) ? a : b;
    return result;   // result dies here; the reference outlives it
}
```

The caller receives a reference to memory that is no longer an `int` and will shortly be overwritten by some other function's frame. A pointer or reference to an object whose lifetime has ended is said to **dangle**. GCC warns about this one (`reference to local variable 'result' returned`), which is one reason we compile with warnings on. Return the value instead: `int biggest(int a, int b)`.
:::

### Dynamic storage: the heap {#sec:mem-heap .unnumbered}

For objects that must outlive the function that creates them, or whose size is not known until the program runs, there is a second region of memory, the **heap** (also called the free store). You request heap memory explicitly with `new`, which returns a pointer to it, and you return it explicitly with `delete`.

```{.cpp include="ch01/lifetime.cpp" region="make_array"}
```

```{.cpp include="ch01/lifetime.cpp" region="new_delete"}
```

Look carefully at what lives where in `make_array`. The pointer variable `block` is a local variable on the stack, eight bytes holding an address. The thousand `int`s it points to are on the heap. When `make_array` returns, `block` is destroyed like any other local, but its value, the address, has been copied out to the caller, and the heap block is unaffected. Heap objects live until someone deletes them. [[fig:stack-heap]] lets you step through this and a few other small programs and watch both regions change.

::: {.viz #fig:stack-heap name="memory"}
The stack and the heap during a call to `make_array`. Local variables, including pointers, live in stack frames that vanish when their function returns. Heap blocks persist until deleted. In the web edition, choose a program and step through it line by line.
:::

With that freedom comes an obligation. Every block obtained with `new` must be released with `delete` exactly once (and with `delete[]` if it was allocated with `new[]`). There are three ways to get this wrong, and each has a name.

A **memory leak** is a block that is never deleted. If the only pointer to a heap block is overwritten or goes out of scope, the block can never be freed; it stays allocated until the program ends. A leak in a short program is harmless. A leak inside a loop in a program that runs for weeks is how servers die.

A **dangling pointer**, or **use after free**, is a pointer that is used after the block it points to has been deleted. The memory may by then belong to some unrelated object. Reading through the pointer gives nonsense; writing through it corrupts whatever now lives there.

A **double free** is deleting the same block twice, which corrupts the allocator's own records.

The second and third are undefined behavior. All three stem from one question that C++ leaves to you: for each block on the heap, **who is responsible for deleting it?** The usual word is **ownership**. In the code above, `make_array` hands ownership of the block to its caller, which is why the comment says so.

::: byhand
For the next few weeks, whenever a program uses pointers or the heap, draw it: a column of stack frames with a box for each variable, a separate area for heap blocks, and an arrow from each pointer to what it points at. Step through the code and update the drawing by hand, crossing out frames as functions return. Do this on paper, without a model.

There are two reasons. First, this drawing *is* the mental model that the rest of the course runs on, and you only acquire it by producing it yourself, slowly, a number of times. Second, language models are text predictors, not memory simulators. Ask one for a step-by-step memory diagram of pointer-heavy code and it will usually be right and sometimes be subtly wrong, with an arrow aimed at the wrong box, and until you can draw the diagram yourself you cannot tell which.
:::

### What modern C++ does about it {#sec:raii-preview .unnumbered}

If managing heap memory by hand is so error-prone, how does anyone write large programs in C++?

By almost never doing it by hand. The central technique of modern C++ is to wrap each heap allocation inside an object whose own lifetime is automatic. `std::vector` is the leading example. A `vector` variable is a small object on the stack holding a pointer to a heap block where the elements live. When the vector's lifetime ends, a special function called its *destructor* runs automatically and deletes the block. The heap memory gets the easy, automatic lifetime of a stack variable, and you never write `delete` at all.

So the rule for everyday code is simple: **use `std::vector` and `std::string`, and do not write `new` or `delete`.**

This course is not everyday code, though. Our business is to understand how types like `vector` are built, so in [[ch:vector]] you will write one, `new` and `delete` and all, and in later chapters you will build lists and trees out of individually allocated nodes. You will do it with the safety equipment on.

## The everyday kit: `vector` and `string` {#sec:kit}

You will lean on two standard library types from the first day. Here is a working knowledge of each.

```{.cpp include="ch01/vector_tour.cpp" region="tour"}
```

A `std::vector<T>` is a sequence of objects of type `T`, stored contiguously, that can grow. The type in angle brackets says what it holds. `push_back` appends; `size` reports the count; square brackets index from zero. Indexing with `[]` performs no bounds check, exactly as with a raw array, while `at()` checks and reports an error if the index is out of range.

`std::string` is similar, specialized for characters.

```{.cpp include="ch01/vector_tour.cpp" region="strings"}
```

Here is a complete small program in the style you will write this week. It reads numbers from standard input, however many there are, and reports on them.

```{.cpp include="ch01/stats.cpp" region="functions"}
```

```{.cpp include="ch01/stats.cpp" region="main"}
```

```console
$ ./stats < stats.in
count:    6
smallest: -3
mean:     13.6667
negative value at index 2
```

Things to notice: the functions take the vector by `const` reference, so nothing is copied. The loop `while (std::cin >> x)` reads one number per iteration and stops when the input is exhausted. The index variable `i` is a `std::size_t`, to match the type of `data.size()`. And `mean` adds into a `long long`, so that a long list of large values does not overflow a 32-bit total.

::: check
The function `smallest` begins with `int best = v[0];`. Under what circumstances is that line a bug, and how does `main` protect against it?

::: answer
If `v` is empty there is no `v[0]`, and reading it is an out-of-bounds access: undefined behavior. `main` checks `data.empty()` and returns before calling `smallest`. A more defensive design would have `smallest` itself check, or state its requirement in a comment. Requirements like "must not be called on an empty vector" are called *preconditions*, and we will be much more deliberate about them starting in [[ch:vector]].
:::
:::

## Tools that catch you {#sec:tools}

The fourth of the book's questions is *how do you know?* This section is its first appearance, and it is about not having to rely on luck.

Here is a program with a classic mistake: the loop condition uses `<=` where it should use `<`, so the last iteration reads one element past the end of the vector.

```{.cpp include="ch01/bugs/oob.cpp"}
```

Compiled plainly, with no special flags, and run three times, it prints:

```console
60
60
60
```

That is the correct sum of 10, 20, and 30. The out-of-bounds read happened to find a zero. The program has undefined behavior, and it looks perfectly healthy. Submit it, and it might fail on the grader's machine, or it might not fail for a year.

Now compile it with the flags from the beginning of the chapter, and run it again.

```console
$ g++ -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined oob.cpp -o oob
$ ./oob
==21309==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x50200000001c
READ of size 4 at 0x50200000001c thread T0
    #0 0x5be00d361731 in main ch01/bugs/oob.cpp:8
    ...
0x50200000001c is located 0 bytes after 12-byte region [0x502000000010,0x50200000001c)
allocated by thread T0 here:
    #0 0x78b4b38fe548 in operator new(unsigned long)
    ...
```

The program now stops at the moment of the bad access and tells you what happened (a read of 4 bytes beyond a heap block), where (`oob.cpp`, line 8), and what it ran into (the end of a 12-byte region, which is three `int`s). This is **AddressSanitizer**, enabled by `-fsanitize=address` [@serebryany2012]. It works by having the compiler insert a check before every memory access and by surrounding every allocation with poisoned "red zones." It roughly doubles a program's running time, which is a bargain.

AddressSanitizer catches all three of the heap errors described in [[sec:lifetimes]]. A use after free:

```{.cpp include="ch01/bugs/dangling.cpp"}
```

```console
==21312==ERROR: AddressSanitizer: heap-use-after-free on address 0x502000000010
READ of size 4 at 0x502000000010 thread T0
    #0 0x5f9fdc021475 in main ch01/bugs/dangling.cpp:6
    ...
freed by thread T0 here:
    #1 0x5f9fdc021309 in main ch01/bugs/dangling.cpp:5
```

Without the sanitizer, the same program printed `-452675544` on one run. A leak:

```{.cpp include="ch01/bugs/leak.cpp"}
```

```console
==21315==ERROR: LeakSanitizer: detected memory leaks
Direct leak of 4000 byte(s) in 1 object(s) allocated from:
    #1 0x5c7e03bb1385 in make_array(int, int) ch01/bugs/leak.cpp:4
    #2 0x5c7e03bb1486 in main ch01/bugs/leak.cpp:10
```

Four thousand bytes is a thousand `int`s: the first block, whose only pointer was overwritten on line 11.

The other half of the flag, `-fsanitize=undefined`, enables **UndefinedBehaviorSanitizer**, which catches a different family of errors, such as signed overflow.

```console
overflow.cpp:5:9: runtime error: signed integer overflow:
2147483647 + 1 cannot be represented in type 'int'
```

Sanitizers only examine the code that actually runs. If your tests never trigger the faulty line, the sanitizer never sees it. So the tools do not relieve you of testing; they make your tests far more informative. A test that passes under the sanitizers has shown not only that the output was right but that no memory error occurred along the way.

One more tool is so simple that it is easy to overlook: `assert`. Writing `assert(condition);` (after `#include <cassert>`) makes the program stop with a message if the condition is false. Assertions are how you write down what you believe to be true at a point in the program, in a form the machine will check. The example programs in this book use them constantly, and so should yours.

::: wild
Memory errors are not a beginner's problem that professionals grow out of. For years, large software vendors have reported that the majority of their serious security vulnerabilities trace back to memory-safety bugs in C and C++ code: out-of-bounds accesses and uses after free, the same two bugs you just watched a sanitizer catch. This is a principal reason for the industry's growing interest in memory-safe languages such as Rust, and for the steady movement of C++ itself toward safer defaults. It is also why this book has you compile with sanitizers from day one. The habit is professional practice, not training wheels.
:::

::: {.fourq title="A C++ object"}
Where does it live?
:   At an address, occupying `sizeof` bytes. Local variables live in their function's stack frame. Objects created with `new` live on the heap. A `vector` or `string` variable is a small object that owns a block on the heap.

What does it promise?
:   Its type says how its bytes are interpreted. A variable is the object itself, so assignment copies. A reference is another name for an existing object. A pointer holds an address and may be null.

What does it cost?
:   Reaching element `i` of an array is one multiplication and one addition, regardless of the array's length. Stack allocation is nearly free. Copying costs time proportional to the size of the thing copied, which is why large objects are passed by `const` reference.

How do you know?
:   Compiler warnings, AddressSanitizer, UndefinedBehaviorSanitizer, and `assert`. A program that merely prints the right answer has not been shown to be correct.
:::

## Exercises {#sec:memory-ex .unnumbered}

1. Compile and run `hello.cpp` with the full compile command from this chapter. Then introduce three different errors, one at a time (delete a semicolon, misspell `cout`, remove the `#include`), and read what the compiler says about each. Which messages point at the actual mistake, and which point somewhere nearby?

2. Write a program that prints `sizeof` for `short`, `long`, `float`, `long double`, `std::size_t`, `std::string`, and `std::vector<int>`. The last two results are small and do not depend on how much the string or vector holds. Explain why, in terms of where their contents live.

3. Without running it, predict the output of this program. Then run it and check.

    ```cpp
    void bump(int a, int& b, int* c) { a += 1; b += 1; *c += 1; }

    int main() {
        int x = 10, y = 20, z = 30;
        bump(x, y, &z);
        std::cout << x << ' ' << y << ' ' << z << '\n';
    }
    ```

4. Write a function `void swap_ints(int& a, int& b)` that exchanges the values of its two arguments. Then write `void swap_ints(int* a, int* b)` that does the same through pointers. How do the two calls look different at the call site? Which version makes it more obvious to a reader of the *calling* code that the arguments may be modified?

5. Given `int a[6] = {5, 10, 15, 20, 25, 30};` and `int* p = a + 2;`, give the value of each expression, or say that it is undefined: `*p`, `p[1]`, `*(p - 2)`, `p[-1]`, `*(a + 6)`, `p - a`, `&a[4] - p`.

6. Draw the stack and heap, by hand, at the moment just before `make_array` returns in the `lifetime.cpp` example when it is called as `make_array(3, 7)`. Then draw them again just after it returns. Which boxes disappeared?

7. Each of these functions has a lifetime or ownership bug. Name the bug (leak, dangling, double free, or out-of-bounds) and say how to fix it.

    ```cpp
    int* f1() { int x = 5; return &x; }

    void f2() { int* p = new int[10]; p = new int[20]; delete[] p; }

    void f3() { int* p = new int{1}; int* q = p; delete p; delete q; }

    int f4() { int* p = new int[4]{1, 2, 3, 4}; int s = p[4]; delete[] p; return s; }
    ```

8. Compile each program in `code/ch01/bugs/` twice, once with only `-std=c++20` and once with the full set of flags, and run both versions. Make a table: for each program, what did the plain build do, and what did the sanitized build report? Was the plain build's behavior the same every time you ran it?

9. Write a function `std::vector<int> evens(const std::vector<int>& v)` that returns a new vector containing only the even elements of `v`, in order. Why is it safe to return a `vector` by value when it was not safe to return a reference to a local variable?

10. Write a function `bool is_palindrome(const std::string& s)` that reports whether `s` reads the same forwards and backwards. Use two indices that start at opposite ends and move toward each other. What should it return for the empty string, and why?

11. The expression `v.size() - 1` is dangerous when `v` is empty. Explain what value it has and why, and rewrite the loop `for (std::size_t i = 0; i <= v.size() - 1; ++i)` so that it is correct for every vector, including the empty one.

12. **With a model, as critic.** Write a function that reverses a `std::vector<int>` in place, and test it yourself on vectors of length 0, 1, 2, and 5. *Then* give your function to a language model with the prompt: "Do not rewrite this. List any inputs for which it misbehaves, and any C++ style problems." Evaluate each of its claims: which were correct, which were wrong, and which were matters of taste?

13. **Lab.** Write a program `histogram` that reads whole numbers between 0 and 100 from standard input and prints a text histogram with one row per range of ten (0–9, 10–19, and so on), using one `*` per value in the range. Store the counts in a `std::vector<int>`. Decide what the program should do with a number outside 0–100 and with the value 100 itself, and document your decision in a comment. The program must run cleanly under both sanitizers on an input file of your own devising that includes those edge cases.

## Chapter notes {#sec:memory-notes .unnumbered}

This chapter compresses a great deal of C++ into a few pages. For a fuller and gentler introduction to the language by its designer, see Stroustrup's *Programming: Principles and Practice Using C++* [@stroustrup2024]; for a brisk overview aimed at people who already program, his *A Tour of C++* [@stroustrup2022]. The reference site cppreference.com [@cppreference] is the place to look up any library function or language rule, and is more reliable than a model's recollection of the same.

The picture of memory given here, a flat row of bytes with a stack and a heap, is a simplification of what the operating system and hardware provide. Bryant and O'Hallaron's *Computer Systems: A Programmer's Perspective* [@bryant2016] gives the real story, and is the natural next book if this chapter made you curious. AddressSanitizer is described by its authors in @serebryany2012. The advice to prefer `vector` and `string` and to avoid naked `new` and `delete` follows the C++ Core Guidelines [@coreguidelines], which are worth browsing as a statement of what experienced C++ programmers consider good practice.
