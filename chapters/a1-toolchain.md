# The Toolchain {#app:toolchain}

This appendix is a reference for the mechanics of working in C++: getting a compiler, building programs, and reading what the tools tell you. Skim it in the first week and come back when something breaks.

## Getting a compiler {#sec:tc-install}

You need a compiler that supports C++20. Either of the two major open-source compilers will do: GCC (whose C++ compiler is the command `g++`), version 11 or later, or Clang (`clang++`), version 14 or later. The book's code was tested with GCC 13.

**Linux.** Install your distribution's compiler package. On Debian or Ubuntu: `sudo apt install g++ make`.

**macOS.** Run `xcode-select --install` in a terminal. This installs Apple's version of Clang, which also answers to the name `g++`. It handles everything in this book, with one caveat about leak detection noted below.

**Windows.** The smoothest route is the Windows Subsystem for Linux (WSL), which gives you a real Linux environment; then follow the Linux instructions.

**Lab machines.** Your instructor will tell you what is installed and how to reach it.

Check your installation:

```console
$ g++ --version
g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
```

## Building programs {#sec:tc-compile}

The one command you need:

```console
$ g++ -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined prog.cpp -o prog
$ ./prog
```

| Flag | What it does | Why |
|:----------------|:-------------|:-------------|
| `-std=c++20` | selects the language version | older defaults reject some of the book's code |
| `-Wall -Wextra` | enables most warnings | warnings are free bug reports; fix them all |
| `-g` | records line numbers in the program | so that sanitizers and debuggers can name source lines |
| `-fsanitize=address,undefined` | adds run-time error checking | catches memory errors and undefined behavior as they happen |
| `-o prog` | names the output | otherwise it is called `a.out` |

For *timing* a program, use a different set of flags: optimization on, sanitizers off.

```console
$ g++ -std=c++20 -O2 prog.cpp -o prog
```

Never time a program built with sanitizers, and never trust the correctness of a program you have only run without them.

**Programs in several files.** List all the `.cpp` files on the command line. Header files (`.hpp`) are not listed; they are pulled in by `#include`. To tell the compiler where to look for headers, use `-I`:

```console
$ g++ -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined -I. main.cpp util.cpp -o main
```

**The book's code.** The `code/` directory has a `Makefile` that builds everything.

| Command | Effect |
|:--|:--|
| `make test` | compiles every program with warnings as errors and both sanitizers, then runs each one |
| `make bench` | compiles the timing programs with `-O2` |
| `make bugs` | compiles the deliberately broken programs, so that you can watch them fail |
| `make clean` | removes everything that was built |

Typing long commands repeatedly is tedious and error-prone. For your own assignments, a three-line `Makefile` is enough to start with (the last line, the command, must begin with a tab character, not spaces):

```makefile
CXXFLAGS = -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined
prog: prog.cpp
	g++ $(CXXFLAGS) prog.cpp -o prog
```

## Reading compiler errors {#sec:tc-errors}

C++ compilers are thorough and not concise. Three habits make their output manageable.

**Fix the first error first.** One mistake often produces a cascade of consequent errors. Scroll to the top, fix the first, and recompile. Many of the rest will vanish.

**Find the line that is yours.** An error involving a template can run to dozens of lines, most of them naming files deep inside the standard library. Look for the line that mentions *your* file. That is where the mistake is.

**Read the "note:" lines.** They often say what the compiler expected.

| Message contains | Usually means |
|:--|:--|
| `expected ';' before ...` | a missing semicolon, on the line *before* the one named |
| `'x' was not declared in this scope` | a misspelling, a missing `#include`, or a missing `std::` |
| `no matching function for call to ...` | wrong number or types of arguments; the notes list the candidates |
| `invalid conversion from 'const char*' to 'int'` | a value of the wrong type |
| `passing 'const X' as 'this' argument discards qualifiers` | calling a non-`const` member function on a `const` object; the function probably should be marked `const` |
| `use of deleted function` | an attempt to copy something that cannot be copied, such as a `std::unique_ptr`; use `std::move`, or pass by reference |
| `undefined reference to ...` | this is from the *linker*: a function was declared but its definition was never compiled in; a `.cpp` file is missing from the command line |
| `comparison of integer expressions of different signedness` | (warning) comparing an `int` with a `std::size_t`; make the loop variable a `std::size_t` |
| `unused variable` / `unused parameter` | (warning) often a sign of a typo or unfinished logic |
| `control reaches end of non-void function` | (warning) some path through the function forgets to `return` a value; that path is undefined behavior |

## Reading sanitizer reports {#sec:tc-sanitizers}

A report from AddressSanitizer has the same parts every time. Here is one from [[ch:memory]], abbreviated.

```console
==46472==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x50200000001c
READ of size 4 at 0x50200000001c thread T0
    #0 0x62a34ecde731 in main ch01/bugs/oob.cpp:8
    #1 ...
0x50200000001c is located 0 bytes after 12-byte region
                                     [0x502000000010,0x50200000001c)
allocated by thread T0 here:
    #0 ... in operator new(unsigned long)
    ...
    #7 0x62a34ecde5b3 in main ch01/bugs/oob.cpp:5
```

1. **The first line names the kind of error.** See the table below.
2. **The second says what the program was doing**: reading or writing, and how many bytes.
3. **The first stack trace says where.** Each line is a function call, innermost first. Read down until you reach a line in *your* code: here, `oob.cpp`, line 8.
4. **The next line describes the memory involved**: here, just past the end of a 12-byte block.
5. **Further stack traces say where that memory was allocated**, and for a use-after-free, where it was freed.

| Report | Meaning | Typical cause |
|:--|:--|:--|
| `heap-buffer-overflow` | access outside a heap block | index out of range; off-by-one in a loop |
| `stack-buffer-overflow` | access outside a local array | the same, on a local array |
| `heap-use-after-free` | access to memory already deleted | dangling pointer, reference, or iterator; an invalidated vector iterator |
| `attempting double-free` | the same block deleted twice | a missing copy constructor (shallow copy); deleting through two pointers |
| `stack-use-after-scope`, `stack-use-after-return` | access to a local variable after it is gone | keeping a pointer to a variable from an inner block; returning a pointer to a local (as Clang reports it) |
| `SEGV on unknown address 0x000000000000` | dereferenced a null pointer | forgetting to check for `nullptr`; following `next` off the end of a list; with GCC, using a returned reference to a local (GCC turns the address into null) |
| `detected memory leaks` | (at exit) blocks never deleted | a missing `delete`, or a destructor that does not free everything |
| `alloc-dealloc-mismatch` | `new[]` paired with `delete`, or the reverse | use `delete[]` for arrays |

UndefinedBehaviorSanitizer prints one line per problem, beginning with the file and line and the words `runtime error`: `signed integer overflow`, `index out of bounds`, `load of null pointer`, `division by zero`.

::: note
Leak detection is on by default on Linux. With Apple's compiler on macOS it is not available; the other AddressSanitizer checks work normally. If you develop on a Mac, run your tests on a Linux machine as well before you hand them in.
:::

## When the program is simply wrong {#sec:tc-debug}

The sanitizers catch errors of *memory*. Errors of *logic*, where the program runs cleanly and gives the wrong answer, need other methods.

**Make it small.** Find the smallest input on which the program misbehaves. For data structures the bug almost always appears with three or four elements. A differential test with small random inputs ([[sec:vec-testing]]) will often find that input for you; print the sequence of operations that led to the failure.

**Assert what you believe.** Put `assert` statements wherever you can state something that must be true: a precondition at the top of a function, the class invariant at the end of every member function that modifies the object. The first assertion to fail is close to the bug.

**Print.** A few well-placed output statements, showing the state of the structure after each operation, are often the fastest route to understanding. Write a `print` or `dump` function for each structure you build. Send debugging output to `std::cerr`, which is not buffered, so that it appears even if the program crashes on the next line.

**Use a debugger.** A debugger (`gdb` on Linux, `lldb` on macOS) runs your program under your control: you can stop at any line, examine any variable, and step forward one statement at a time. Half a dozen commands cover most needs.

| `gdb` command | Effect |
|:--|:--|
| `break file.cpp:42` | stop when execution reaches that line |
| `run` | start the program |
| `next` / `step` | execute one line, stepping over / into function calls |
| `print expr` | show the value of an expression |
| `backtrace` | show the chain of function calls that led here |
| `continue` | resume until the next breakpoint |

**Explain it.** Describe, line by line and out loud, what the code is supposed to do: to a classmate, to a rubber duck, or in writing. The act of explaining forces you to look at what the code says and not at what you meant. A language model can play the listener, and is good at it, particularly for compiler errors and sanitizer reports. Ask it what the message means, and do the fixing yourself.
