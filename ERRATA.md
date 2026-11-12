# Errata and reading notes

Notes from the author's (Claude's) first front-to-back read of the book, on 2026-11-12, after the single-shot version was saved. **Nothing listed here has been changed in the text.** Line numbers refer to the files in `chapters/` as they stood at that read.

Scope of the read: the preface, chapters 1–10, and the colophon, in full. The three appendices were not re-read in this pass.

## 1. Errors and overclaims (fix before classroom use)

| Where | Problem | Suggested fix |
|:--|:--|:--|
| `10-wild.md:174` | "A hash table, a heap, and a trie: three chapters of this book, running every time you send a message to a model." False as written. The heap is used when a tokenizer is *trained*, not on every message, and the trie belongs to other tokenizer families (WordPiece-style longest match), not to byte-pair encoding. | End the paragraph on what is true: a hash table on every message; a heap during training; a trie in other tokenizer families. |
| `10-wild.md:132` | "tries are central to how text is fed to language models" contradicts the corrected paragraph at line 174. | "Tries also turn up in some tokenizers, as the first case study notes." |
| `00-preface.md:48` | Says *every* listing is an excerpt from a compiled file that was run under memory-error tools. The colophon now says otherwise: the `bench/` programs are built without sanitizers, and a few short fragments are typed directly into the text. | Match the colophon: every listing *with a file path above it*; note the two exceptions in a clause. |

## 2. Confusing or inconsistent (worth fixing)

| Where | Problem | Suggested fix |
|:--|:--|:--|
| `02-cost.md:326` | Section title "When the model lies" means the step-counting cost model, in a book where "model" otherwise always means a language model. It also shows in the sidebar and table of contents with no context. | Retitle, e.g. "When the step count lies". The label `sec:model-lies` can stay, so no cross-references break. |
| `02-cost.md:344`, `04-links.md:329` | Check-yourself answers that say only "See the table that follows" / "See the table." In print, answers move to the end of the chapter, where these mean nothing. | Give the actual answer in a sentence (the ratio, or who wins and by how much). |
| `06-trees.md:77` | "Write $T(n)$ for the time on an input of size $n$." The table that follows never uses $T(n)$. | Delete the sentence. |
| `10-wild.md:106` | Trie caption says "double-ringed nodes mark the ends of words". True of the print figure; the web widget marks them with shading and a tick. | "Marked nodes" or describe both. |
| `06-trees.md:187` | "We will see shortly what goes wrong" — the payoff is at the very end of the chapter. | "At the end of the chapter we will see…" |
| `10-wild.md:221` | "takes two ideas from this book and a third from [the Bloom filter section]" — that section is also in this book. | "takes two structures from this book and one idea from the Bloom filter". |

## 3. Structure and flow

- **`06-trees.md:307–333`** The chapter concludes ("That is the subject of [the next chapter]") and then continues with a further subsection, "Checking the invariant". Either move that subsection before the shape problem's conclusion, or move the concluding paragraph to the end.
- **`03-vector.md:254–260`** The warning box about moved-from objects sits between the rule-of-zero statement and the sentence "This is the rule for nearly every class you will write…", so "This" now seems to refer to the warning. Move the box below that paragraph.
- **`10-wild.md:263`** The closing seven-step checklist is in a "By hand first" box. The label does not fit; it is a keepsake, not a warning against using a model. Consider a plain note box or its own box type.
- **`07-balance.md:45`** Two parenthetical sentences back to back at the end of the rotation paragraph. Fold the second into the prose.
- **`08-heaps-sorting.md:211`** The randomized-quicksort paragraph carries a late correction (the $-2.85n$ term and the note about double comparisons) and reads like a patch. Worth rewriting as two paragraphs: what randomizing buys, then what it costs in comparisons.
- **End of `08-heaps-sorting.md` and parts of `05-hashing.md`** Several pages are mostly boxes (two "Ask an LLM" boxes in a row at `08:317–331`). Consider merging or moving one.

## 4. Where the explanation is too compressed

The prose is leisurely on easy material and terse on the hardest reasoning. These are the places a student is most likely to get lost:

- **`09-graphs.md:203`** Why Dijkstra's algorithm is correct. The argument blurs the queue key of the unsettled vertex $x$ with the length of the route through it. Needs three or four more sentences and probably a small figure.
- **`09-graphs.md:86`** Why BFS distances are shortest. One dense paragraph.
- **`07-balance.md:135`** AVL removal. "The code does not care" skips why rebalancing at every level is both necessary and sufficient.
- **`05-hashing.md:205–216`** `ProbingMap::put` and `rebuild`. The code keeps two counters (`size_` and `used_`) and decides between rebuilding in place and doubling; the text explains neither.
- **`04-links.md:122–131`** Pointers to pointers. A real jump in difficulty with one paragraph of support.

## 5. Voice (optional; leave as-is if the book is kept as an artefact)

Counts are across the preface and chapters 1–10.

- **Every paragraph resolves the same way**: setup, colon, payoff, short closing sentence. 59 paragraphs end on a sentence of five words or fewer.
- **Stock openers**: "Here is / Here are" 54 times (nine in Chapter 2 alone); "That is the / why / what…" 34 times; "Notice / Look at…" 13 times.
- **Announced importance**: 13 uses of "the most important / effective / famous…" and 7 of "one of the most / best / prettiest". The material would land without being told it matters.
- **Recurring figures of speech**: "earns its keep / place" three times; "our old friend" / "our familiar sum" for the arithmetic series.

## 6. What held up on rereading

For balance, and so a later edit does not flatten them:

- `07-balance.md:160–162`: a promise about cost is invisible to a test of results.
- `10-wild.md:89–91`: the linked list's one good job, and why the default copy of the cache is wrong.
- `08-heaps-sorting.md:229`: "That is suspicious", leading into the lower bound.
- `02-cost.md:307`: the "hold that thought" before the cache section, resolved at `02-cost.md:369`.
- The chapter openers: each check-yourself question sets up the chapter's problem before any explanation.
- The repeated lesson (trees, hash tables, quicksort): fine on random input, disastrous on a pattern, and the pattern is common.
