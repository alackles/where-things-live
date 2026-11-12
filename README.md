# Where Things Live: Data Structures in Modern C++

A ten-week textbook for CMSC 270 (Lawrence University), in two editions built from one source:

- **Web edition**: `dist/html/index.html`. Open it straight from disk; no server and no network needed. Sixteen interactive figures, light and dark themes.
- **Print edition**: `dist/where-things-live.pdf`, typeset with LaTeX (7 × 10 in).

Drafted by Claude (Anthropic, Fable 5.2) in November 2026 at the instructor's request. Read the colophon (`chapters/z1-colophon.md`) for what was and was not verified. Short version: all code is compiled, sanitized and differentially tested; all timings are measured; the prose has had automated review only and should get a human expert's eye before students rely on it.

## Layout

```
book.yaml            the order of the book: one entry per chapter file
chapters/*.md        the text, in Pandoc Markdown (one file per chapter)
references.bib       the bibliography (BibTeX)
code/chNN/           every C++ program in the book; `make test` runs them all
code/chNN/bench/     timing programs (built with -O2, no sanitizers)
code/chNN/bugs/      deliberately broken programs, for the sanitizer demos
figures/figlib.py    tiny drawing library with two backends: SVG and TikZ
figures/figs.py      every static figure, described once
web/style.css        web edition stylesheet;  web/book.js  page behaviour
web/viz/core.js      engine for the interactive figures (scene + step player)
web/viz/<name>.js    one file per interactive figure
latex/               preamble, title page, print syntax theme
tools/build.py       the build: numbering, listings, pandoc, page assembly
tools/book.lua       pandoc filter: callouts, figures, widgets, answers
tools/*.sh           review helpers (screenshots, contact sheets, JS check)
dist/                the built editions
```

## Building

```
make            # both editions
make pdf        # just the PDF
make html       # just the web edition
make test       # compile + run all C++ under ASan/UBSan
make check      # all of the above, then load every page in headless Chrome
```

Requirements: Python 3 with PyYAML, pandoc 3+, TeX Live (pdflatex, bibtex, and the packages XCharter, newtx, FiraSans, inconsolata, tcolorbox, titlesec, fvextra, natbib), and g++ 11+ or clang++ 14+. `make check` and the screenshot tools also want Google Chrome and ImageMagick.

## Editing the text

Chapters are ordinary Markdown with a few conventions. All of them are handled by `tools/build.py` and `tools/book.lua`.

**Cross-references.** Give headings ids, `## Title {#sec:name}`, and refer to them as `[[sec:name]]`, `[[ch:name]]`, `[[app:name]]`, or `[[fig:name]]`. The build numbers everything and reports any reference it cannot resolve.

**Code listings** are never typed into the text. Put the code in `code/`, mark a region with comments, and include it:

````
// [push_back]                      (in code/ch03/vec.hpp)
void push_back(T value) { ... }
// [/push_back]

```{.cpp include="ch03/vec.hpp" region="push_back"}
```
````

Omit `region` to include the whole file. The build warns about lines too long for the printed page (78 columns).

**Callouts** are fenced divs: `::: goals`, `::: check` (with a nested `::: answer`), `::: warning`, `::: wild`, `::: note`, `::: llm` (block quotes inside become copyable prompts), `::: byhand`, and `::: {.fourq title="..."}` for the four-questions card. Any of them takes an optional `title="..."`.

**Static figures.** Add a function to `figures/figs.py` decorated with `@figure("name")`, then:

```
::: {.figure #fig:name}
Caption text.
:::
```

The same description renders as theme-aware SVG on the web and as TikZ in print. `tools/figsheet.sh out.png name1 name2 ...` previews figures with the print fonts.

**Interactive figures.** Add `web/viz/<name>.js` that calls `Viz.register('<name>', fn)` (see any existing widget, and the header of `core.js`), then:

```
::: {.viz #fig:some-id name="<name>" fig="static-fallback"}
Caption text.
:::
```

`fig` names the static figure shown in print and when JavaScript is off; it defaults to the id without `fig:`. Extra attributes become `data-*` options (the AVL figure is the BST widget with `mode="avl"`). `tools/vizgallery.sh out.png [dark] [names...]` screenshots widgets in a demo state; `tools/vizcheck.sh` loads every page headlessly and reports script errors.

**Citations** use Pandoc syntax, `[@knuth1998]` or `@knuth1998`, with keys from `references.bib`. The web edition lists references per chapter and on a bibliography page; the PDF uses natbib.

## Things left for the instructor

- **Read it critically.** Especially the chapter notes (history, attributions), the "In the wild" boxes (claims about real systems drift), and the preface's summary of research on LLMs and learning.
- **The LLM stance** in the preface ("generate before you consult") is the book's, not a course policy. The preface defers to the syllabus; adjust to taste.
- **Title, attribution, license.** The title page names the course, not an author. No license has been chosen for the text or code.
- **Fit to the term.** Each chapter is sized for one week of three meetings; weeks 1, 3 and 8 are the heaviest. Exercises marked **Lab** are sized as week-long assignments. No solutions manual exists.
- **Timings** come from one laptop (see the colophon). Rerun `code/chNN/bench/*` on the lab machines if you want local numbers; the tables in the text are typed by hand from that output.
- **Git.** This directory is not yet a repository. `.gitignore` is set up to keep `dist/` and ignore `build/`.
