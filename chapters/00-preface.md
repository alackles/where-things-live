# Read This First {#ch:preface}

Every program you have ever written kept its data somewhere. A list of names, a table of scores, the squares of a game board: each one sat in the computer's memory in some particular arrangement, and that arrangement decided what your program could do quickly, what it could do only slowly, and what it could not do at all. Most of the time, in a first programming course, somebody else chose the arrangement for you. You wrote `list` or `ArrayList` or `dict` and got on with your day.

This book is about making that choice yourself. A **data structure** is a decision about where things live in memory, together with a set of promises about how you can get at them. Ten weeks from now you should be able to look at a problem, say what operations it needs, and pick or build an arrangement of memory that makes those operations cheap. You should also be able to look at somebody else's choice, whether that somebody is a library author, a teammate, or a language model, and say whether it was a good one.

## The four questions {#sec:four-questions}

There are a great many data structures. Rather than present them as a catalog, this book asks the same four questions of each one.

**Where does it live?** What does the structure look like in memory? Is it one contiguous block or many small pieces connected by pointers? Who is responsible for creating that memory and for giving it back? You will be surprised how many of a structure's virtues and vices follow from the answer to this one question.

**What does it promise?** What operations does it offer, and what is always true of it between operations? Those always-true facts are called *invariants*, and they are the difference between a data structure and a heap of bytes. A binary search tree is not a picture of circles and lines; it is a promise that everything to the left of a node is smaller than it.

**What does it cost?** How does the time for each operation grow as the structure fills up? How much memory does it use? We will answer on paper, with the mathematics of growth rates, and then we will answer again with a stopwatch, because modern hardware does not always agree with the paper.

**How do you know?** How do you establish that an implementation keeps its promises and meets its costs? This is the question that older textbooks spend the least time on and the one that matters most now. You are going to spend your career surrounded by code you did not write, some of it produced by people and a growing share of it produced by machines. Being able to *check* is worth more than being able to type.

Each chapter ends with a short card that answers the four questions for the structures it introduced. If you can reproduce those cards from memory at the end of term, with reasons, you have learned the course.

## The ten weeks {#sec:ten-weeks}

Lawrence runs on ten-week terms, so this book has ten chapters, one per week. A semester-length text would have room for more structures. This one has room for fewer, and uses the space to go deeper on the ones that carry the most weight in real software.

| Week | Chapter | You will build |
|:-:|:--|:--|
| 1 | Memory Is the Material | small programs that expose addresses, copies, and lifetimes |
| 2 | What Does It Cost? | binary search; a timing harness |
| 3 | Building a Vector | your own `std::vector` |
| 4 | Links, Stacks, and Queues | a linked list, a ring-buffer queue |
| 5 | Hashing | two hash tables and a Bloom filter |
| 6 | Recursion and Trees | a binary search tree |
| 7 | Balance | an AVL tree |
| 8 | Heaps and Sorting | a binary heap; mergesort and quicksort |
| 9 | Graphs | breadth-first search, Dijkstra's algorithm, union-find |
| 10 | Data Structures in the Wild | an LRU cache and a trie |

The order is not the traditional one, and three of the departures are deliberate.

*Memory comes first.* Before any data structure, [[ch:memory]] spends a week on what C++ objects, pointers, and lifetimes actually are. Every later chapter depends on it, and students who are shaky on it stay shaky all term.

*Hash tables come before trees.* In working software the hash table is the most heavily used structure after the array. It also needs nothing but arrays and a little arithmetic, so there is no reason to make it wait behind three weeks of trees.

*Linked lists are taught as an idea more than as a tool.* You need to be fluent with pointers and nodes, because trees and graphs are made of them. But on current hardware a plain linked list loses to an array at almost everything, and [[ch:links]] shows you the measurements instead of asking you to take it on faith.

## How the book works {#sec:how-it-works}

**The code is real.** Every listing in this book is an excerpt from a file in the `code/` directory that accompanies it; the file's path is printed above the listing. Each of those files was compiled with warnings turned all the way up and run under tools that detect memory errors before the text was typeset. You should do the same: type or copy the programs, compile them, change them, and break them. There is no code editor inside the web edition, on purpose. Running a compiler on your own machine, and reading what it tells you, is one of the skills of the course.

**The figures move.** The book comes in two editions built from the same source. The web edition has interactive versions of many figures, in which you can insert into a tree, fill a hash table, or step through a sort one comparison at a time. The print edition shows one frame of each and marks it "Interactive in the web edition." If you are reading on paper, it is worth opening the web edition when you reach one of those.

**The boxes mean things.** Seven kinds of box recur.

::: goals
At the top of every chapter: what you should be able to do by the end of the week.
:::

::: check
A question to answer *before* reading on. In the web edition the answer is folded away beneath the question; in print the answers are collected at the end of the chapter. The questions are not decoration. Trying to answer, and sometimes failing, is what makes the next paragraph stick.

::: answer
Like this. If you peeked without trying, that is your business, but you have cheated only yourself.
:::
:::

::: warning
A mistake that nearly everyone makes at least once, described so that you can make it only once.
:::

::: wild
How the idea shows up in production systems: standard libraries, databases, operating systems, the software behind language models.
:::

::: note
A detail or a digression that can be skipped on a first reading.
:::

::: llm
A prompt worth giving to a language model, with a note on what to do with the answer. The next section explains how these are meant to be used.
:::

::: byhand
A task you should do with the model closed, and the reason.
:::

**The exercises are the course.** Reading about data structures is like reading about swimming. Each chapter ends with exercises in roughly increasing difficulty. Do them.

## Learning with a language model, and without one {#sec:llm}

Here is the awkward fact that any honest data structures textbook now has to open with. A large language model (an LLM: Claude, ChatGPT, Gemini, and their relatives) can write every data structure in this book. Ask for a templated AVL tree in C++20 and you will have one in seconds, usually a correct one. This book itself was drafted by such a model; the colophon at the back explains how, and what was done to check it.

So why learn any of this?

Because *producing* a data structure was never the hard part of the job. The hard parts are knowing which structure a problem calls for, knowing what can go wrong with it, and knowing how to tell whether the thing in front of you is right. An LLM does not remove those problems. It moves them all onto you. When the code arrives in seconds, the only remaining human contribution is judgment, and judgment about data structures comes from having built them, broken them, and measured them yourself. People who cannot tell a subtly broken hash table from a working one are no better off for having received it instantly.

There is also the plainer matter of how learning works. A long line of research finds that people learn most durably from effortful attempts, including failed ones, made *before* they are shown the answer [@kapur2008; @bjork2011]. The effort is the mechanism, not an unfortunate side effect. An LLM is extremely good at removing effort, and that makes it extremely good at removing learning unless you use it with some care. The early evidence from classrooms fits the theory. In one large field experiment, students given open access to a chat model during practice did noticeably better on the practice problems and then *worse* on a later exam taken without it than students who had never had the tool; a version of the tool designed to give hints instead of answers largely avoided the harm [@bastani2025]. Studies of novice programmers describe a widening gap: students who already have a solid footing use these tools to move faster, while students who are struggling can come away with an illusion of competence, feeling that they understand code they could not have written or debugged [@prather2024].

None of this is an argument for pretending the tools do not exist. They are part of how software is made now, and they can be superb tutors. It is an argument for a rule.

### Generate before you consult {#sec:generate-first .unnumbered}

Before you ask a model anything about the material, produce something of your own: a prediction, a drawing, an attempt at the code, a guess at the running time, even a wrong one. *Then* consult. The model's answer lands very differently on a mind that has already committed to a position.

It helps to think of four roles a model can play.

The examiner
:   asks you questions and withholds the answers until you respond. This is the most valuable role and the least used. "Quiz me on this, one question at a time, and do not reveal the answer until I have tried" is the best prompt in this book.

The critic
:   reviews something you have already written or drawn. "Here is my `insert`. Do not rewrite it. Tell me which inputs would break it."

The explainer
:   explains something you have already struggled with, at the level you ask for, as many times as you need, without getting tired or making you feel slow. Use it freely for compiler errors, unfamiliar library functions, and second explanations of ideas the book did not land for you.

The ghostwriter
:   writes the thing so that you do not have to. For work you already know how to do, this is a legitimate time-saver and you will use it for the rest of your career. For the thing you are currently trying to learn, it is self-sabotage with a friendly interface.

The **Ask an LLM** boxes in this book are all written for the first three roles. The **By hand first** boxes mark the places where the fourth is most tempting and most costly.

### What models are good and bad at, in this course {#sec:llm-good-bad .unnumbered}

These are tendencies, not laws, and they shift as the models change. They held when this book was written.

Models are usually *reliable* at: explaining what a compiler error means; explaining what a standard library function does; describing a well-known data structure or algorithm in general terms; generating practice questions; suggesting test cases you had not thought of; pointing out style problems.

Models are *less reliable* at: tracing pointer-heavy code step by step without slipping; counting (comparisons, swaps, loop iterations); the exact running time of an unusual loop; anything that depends on undefined behavior, where the honest answer is "this program has no defined meaning" and the tempting answer is a confident specific number; performance claims, which need to be measured on a real machine and cannot be remembered; and citations, which they can fabricate in perfect bibliographic format.

And one tendency matters more than all the rest: **a model sounds exactly as confident when it is wrong as when it is right.** A human tutor who is unsure will hedge, slow down, or look something up. A model's fluency carries no such signal. You therefore cannot use the *feel* of an answer to judge it. You need independent checks, and this course will give you four of them: compile it with warnings on; run it under a sanitizer; test it against something known to be correct; and measure it. That is the fourth question again. It applies to your own code, to the model's code, and, for that matter, to this book.

### A note on rules {#sec:llm-rules .unnumbered}

Whether and how you may use an LLM on graded work in CMSC 270 is decided by your instructor and stated in the syllabus. Nothing in this book overrides that. Where the syllabus allows these tools, the advice here is about using them so that you still learn. Where it does not, the By hand first boxes will at least tell you why.

## What you need {#sec:what-you-need}

**Background.** One prior course in programming, in any language. You should be comfortable with variables, loops, functions, and the idea of a class or object. You do *not* need to know C++; [[ch:memory]] starts it from the beginning, though it moves quickly and assumes you have programmed before. A little comfort with algebra, exponents, and logarithms will help from [[ch:cost]] onward, and [[app:math]] reviews what is needed.

**Software.** A C++ compiler that supports the C++20 standard (recent versions of GCC or Clang), a terminal, and a text editor. [[app:toolchain]] walks through setup and explains the one compile command you will use all term:

```console
$ g++ -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined prog.cpp -o prog
```

**Why C++?** Because it lets you see. Languages such as Python and Java manage memory for you, which is a kindness when you are building applications and an obstacle when you are trying to learn how data is laid out. C++ makes you say where every object lives and when it dies. That is exactly the subject of this course. It is also the language in which a great deal of the world's performance-critical infrastructure is written, including most of the software that trains and runs language models.

We use modern C++ throughout: C++20 as the baseline, with an occasional pointer toward C++23. If you have seen older C++ (or C) and remember it as a thicket of `malloc`, raw arrays, and manual bookkeeping, some of what follows will be a pleasant surprise. You will still learn the raw machinery, because you cannot understand the modern conveniences without it, but you will learn it in order to build the safe tools and then use them.

## A note for instructors {#sec:instructors .unnumbered}

Each chapter is sized for one week of three class meetings and is divided into sections that roughly match those meetings. The chapters are close to equal in length; the last, for week 10, introduces the least new machinery and can absorb review. Topics a semester course would add, and which this text deliberately leaves to a later algorithms course, include red-black tree implementation, string matching, dynamic programming, minimum spanning trees beyond a brief application of union-find, and formal amortized analysis by potential functions. The exercises marked **Lab** are sized for a week-long programming assignment. The source for the book, its figures, and its code are all in the accompanying repository and are meant to be edited.
