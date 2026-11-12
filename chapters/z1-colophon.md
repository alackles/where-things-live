# How This Book Was Made {#ch:colophon}

This book asks, of every data structure, *how do you know?* It would be poor form not to answer the same question about itself.

## Who wrote it {#sec:colophon-who}

The text, the code, the figures, and the interactive widgets were drafted by Claude, a large language model made by Anthropic (the Fable 5.2 model), working in a single extended session in November 2026 at the request of the instructor of CMSC 270 at Lawrence University. The instructor's brief asked for a ten-week data structures text in C++ in two editions, one for the browser and one typeset with LaTeX, and left the content, the order, and the presentation to the model's judgment. The decisions described in the preface, such as putting memory first and hashing before trees, the four recurring questions, and the stance on language models, are the model's own.

That is an unusual way for a textbook to come about, and you are entitled to ask how far to trust the result. The answer has two halves.

## What was checked, and how {#sec:colophon-checked}

**The programs were compiled and run.** Every C++ listing with a file path printed above it is an excerpt, extracted mechanically at build time, from that file in the `code/` directory. Running `make test` there compiles each program with `-std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror` and both AddressSanitizer and UndefinedBehaviorSanitizer, and executes it; all of them pass. The timing programs in the `bench/` directories are the exception: they are compiled with optimization and without sanitizers, as timing requires, and were run by hand to produce the tables. A few short fragments in check-yourself questions, warnings, and exercises have no file path above them. Those were typed directly into the text and were not compiled; several are wrong on purpose.

**The data structures were tested against independent references.** The vector, lists, queue, hash tables, search trees, AVL tree, B-tree, heap, sorts, LRU cache, and trie are each put through thousands of randomized operations and compared with a standard-library container or a deliberately simple second implementation. Where a structure has an invariant that can be checked from inside (the vector, the lists, the hash tables, all the trees, the heap), an executable check runs alongside. The shortest-path, spanning-tree, topological-sort, and union-find code is checked on hundreds of small random graphs against slower independent algorithms. A few functions (depth-first visiting order, path reconstruction, the maze search) are tested only on hand-built examples.

**The tests found real mistakes.** This is worth recording, because it is the book's argument in miniature. While the book was being written, the tests caught, among other things: an expected value for a maze's shortest path that the author had worked out by hand and gotten wrong (the code was right); an example in Appendix C that held an iterator across a call to `sort` and then expected it to still refer to the largest element; and a demonstration of quicksort's worst case that did not exhibit the worst case at all, because the particular partitioning scheme happened to scramble sorted input. Each of these would have gone to print as a confident, plausible, wrong statement. None of them was caught by rereading.

**Every timing was measured.** The numbers in the performance tables are the output of the programs in the `bench/` directories, run on one laptop (a 2023 Intel Core i9-13900H with 64 GB of memory, Ubuntu Linux, GCC 13.3 at `-O2`). They are single runs, reported as observed, noise included. Your machine will give different numbers and, with luck, the same patterns. Where a measurement contradicted what the author expected, the measurement stayed and the text was changed.

**The figures are generated.** Each static figure is described once, in a short Python program, and rendered from that single description both as vector graphics for the web edition and as TikZ for print. The interactive figures compute their states by running the actual algorithm in the browser; they are not hand-drawn animations. (The one exception is the memory explorer in Chapter 1, which replays a scripted trace of four small programs.)

## What was not checked {#sec:colophon-not-checked}

**The prose, by a person.** The explanations, historical remarks, and descriptions of how real systems work were written from the model's training. After drafting, three separate instances of the model, given the source cold and told only to find errors, reviewed the chapters; they reported about forty problems, among them an exercise that overfilled its own hash table, three passages that overstated what the measurements beside them showed, two real bugs in the code (both in copying or moving objects, which the tests had never exercised), and several claims in an earlier draft of this very colophon that the repository did not support. All were corrected. That is a useful second pass, and it is not the same as review by a human expert, which the book has not had. The preface says that a language model sounds exactly as confident when it is wrong as when it is right. That applies to this book.

Claims of the kind most likely to be wrong are: dates and attributions in the chapter notes; statements about what particular libraries and products do internally, which change over time; and the summaries of research on learning in the preface, which compress nuanced findings into a sentence or two. If something here contradicts your instructor, a primary source, or a measurement you have made yourself, believe them and not the book.

**The bibliography.** A language model can produce a citation that is perfectly formatted and entirely fictitious. The references here were restricted to well-known works, and each was then checked against publisher records, library catalogs, and primary copies on the open web by a separate automated pass. All sixty were found to exist as described; two needed corrections (a link, and a title), and a handful gained missing page numbers. One entry was added afterward and has not been through that check: Dijkstra's *A Discipline of Programming*. That is a weaker check than a librarian's. Before you cite any of these works in your own writing, look it up.

**Pedagogy.** No student has yet learned from this book. Whether the order works, whether the exercises are the right difficulty, and whether a ten-week term can hold all of it are empirical questions, and the people best placed to answer them are the instructor and you.

## How to report a problem {#sec:colophon-errata}

Tell your instructor. The source of the book is a directory of Markdown files, C++ programs, and a build script, meant to be edited. A correction to the text is a one-line change; a correction to a program is checked by the same tests as everything else.

## Tools {#sec:colophon-tools}

The source is written in Pandoc Markdown. A Python script assigns numbers, pulls the listings out of the code files, and runs Pandoc with a Lua filter to produce HTML for the web edition and LaTeX for print. The print edition is typeset by pdfLaTeX in XCharter, with Fira Sans for headings and Inconsolata for code; the web edition uses the same typefaces, which are distributed under open licenses. The web edition makes no network requests and runs from a local folder.
