"""Every figure in the book, described once.  See figlib.py.

Add a function decorated with @figure("name"); refer to it in a chapter as

    ::: {.figure #fig:name}
    Caption text.
    :::
"""
import math
from pathlib import Path

from figlib import (Fig, brace_label, cells, draw_tree, edge_between,
                    heap_positions, list_node, tree_positions)

FIGS = {}


def figure(name):
    def deco(fn):
        FIGS[name] = fn
        return fn
    return deco


def build_all(outdir):
    outdir = Path(outdir)
    for name, fn in FIGS.items():
        f = fn()
        (outdir / f"{name}.svg").write_text(f.to_svg(), encoding="utf-8")
        (outdir / f"{name}.tikz").write_text(f.to_tikz(), encoding="utf-8")
    return list(FIGS)


# ==========================================================================
# Chapter 1: memory
# ==========================================================================
@figure("bytes")
def _bytes():
    f = Fig(640, 150, "A row of numbered byte cells; four of them hold the int 270.")
    vals = ["", "", "0E", "01", "00", "00", "", "41", "", "", "", ""]
    fills = {2: "accent-soft", 3: "accent-soft", 4: "accent-soft", 5: "accent-soft", 7: "accent2-soft"}
    x0, w = 44, 46
    cells(f, x0, 50, vals, w=w, h=38, fills=fills, index=False)
    for i in range(len(vals)):
        f.text(x0 + i * w + w / 2, 100, f"{0x7ffc10 + i:x}"[-4:], size=10, color="muted", mono=True)
    f.text(x0 - 8, 100, "…", size=11, color="muted", anchor="e")
    brace_label(f, x0 + 2 * w + 2, x0 + 6 * w - 2, 44, "int n = 270;  (4 bytes)", color="accent", below=False,
                tex=r"\texttt{int n = 270;}\ \ (4 bytes)")
    brace_label(f, x0 + 7 * w + 2, x0 + 8 * w - 2, 44, "char c = 'A';", color="accent2", below=False,
                tex=r"\texttt{char c = 'A';}")
    f.text(320, 132, "addresses (last four hex digits) increase to the right", size=12, color="muted")
    return f


@figure("copy-alias")
def _copy_alias():
    f = Fig(640, 200, "Left: two vectors with separate storage. Right: one int with two names.")
    # left: value semantics
    f.text(30, 18, "std::vector<int> b = a;  b.push_back(4);", mono=True, size=12, anchor="s", color="muted")
    for name, y, vals, hl in (("a", 48, ["1", "2", "3"], {}), ("b", 124, ["1", "2", "3", "4"], {3: "accent2-soft"})):
        f.text(46, y + 17, name, mono=True, bold=True, anchor="e")
        f.rect(56, y, 70, 34, fill="muted-soft")
        f.circle(91, y + 17, 2.8, fill="ink")
        f.line(91, y + 17, 176, y + 17, arrow=True)
        cells(f, 180, y - 1, vals, w=40, h=36, fills=hl, index=False)
    f.text(91, 96, "on the stack", size=10, color="muted")
    f.text(260, 96, "on the heap", size=10, color="muted")
    f.line(400, 12, 400, 188, stroke="muted", dashed=True)
    # right: reference
    f.text(430, 18, "int& alias = score;", mono=True, size=12, anchor="s", color="muted")
    f.rect(500, 86, 70, 36, fill="accent-soft", stroke="accent")
    f.text(535, 104, "11", mono=True)
    f.text(450, 70, "score", mono=True, bold=True, anchor="s")
    f.line(478, 80, 500, 94, arrow=True)
    f.text(450, 140, "alias", mono=True, bold=True, anchor="s", color="accent")
    f.line(478, 130, 500, 114, arrow=True, stroke="accent")
    f.text(535, 150, "one object,", size=10, color="muted")
    f.text(535, 164, "two names", size=10, color="muted")
    return f


@figure("array-arith")
def _array_arith():
    f = Fig(640, 170, "An array of five ints with the address of each element.")
    x0, w = 110, 76
    vals = ["10", "20", "30", "40", "50"]
    cells(f, x0, 46, vals, w=w, h=40, fills={3: "accent-soft"}, index=False)
    f.rect(x0 + 5 * w, 46, w, 40, stroke="muted", dashed=True)
    for i in range(6):
        if i < 5:
            f.text(x0 + i * w + w / 2, 30, f"a[{i}]", mono=True, size=12)
        f.text(x0 + i * w + w / 2, 100, str(1000 + 4 * i), mono=True, size=11, color="muted")
    f.text(x0 - 12, 100, "address", size=11, color="muted", anchor="e")
    f.text(x0 + 5 * w + w / 2, 30, "one past the end", size=10, color="muted")
    cx = x0 + 3 * w + w / 2
    f.line(cx, 136, cx, 112, stroke="accent", arrow=True)
    f.text(cx, 150, "a + 3  =  1000 + 3 × 4  =  1012", mono=True, size=12, color="accent",
           tex=r"\texttt{a + 3}\ \ =\ \ $1000 + 3 \times 4 = 1012$")
    return f


@figure("stack-heap")
def _stack_heap():
    f = Fig(640, 250, "Stack frames for main and make_array, and a heap block of four ints.")
    f.text(160, 18, "Stack", bold=True)
    f.text(490, 18, "Heap", bold=True)
    f.line(330, 8, 330, 242, stroke="muted", dashed=True)
    # main frame
    f.rect(40, 34, 240, 54, fill="muted-soft", stroke="muted")
    f.text(50, 46, "main", size=11, anchor="s", bold=True, color="muted")
    f.text(128, 68, "many", mono=True, anchor="e")
    f.rect(138, 54, 96, 28)
    f.text(186, 68, "?", mono=True, color="muted")
    # make_array frame
    f.rect(40, 98, 240, 136, fill="accent-soft", stroke="accent")
    f.text(50, 110, "make_array", size=11, anchor="s", bold=True, color="accent")
    for k, (name, val) in enumerate((("n", "4"), ("fill", "7"), ("block", None))):
        y = 124 + k * 34
        f.text(128, y + 14, name, mono=True, anchor="e")
        f.rect(138, y, 96, 28, fill="paper")
        if val:
            f.text(186, y + 14, val, mono=True)
    f.circle(186, 206, 2.8, fill="ink")
    f.curve(186, 206, 300, 220, 396, 170, arrow=True)
    cells(f, 400, 140, ["7", "7", "7", "7"], w=44, h=36, fills={i: "accent2-soft" for i in range(4)})
    f.text(488, 118, "new int[4]", mono=True, size=12, color="accent2")
    f.text(488, 212, "lives until someone calls delete[]", size=10, color="muted")
    return f


# ==========================================================================
# Chapter 2: cost
# ==========================================================================
def _plot(f, fn, x0, x1, X, Y, ymax, stroke, steps=80, lw=1.8):
    """Draw y = fn(x) for x in [x0, x1], clipped at ymax.  X, Y map data to px."""
    prev = None
    for k in range(steps + 1):
        x = x0 + (x1 - x0) * k / steps
        y = fn(x)
        if y > ymax:
            if prev:
                # draw up to the clip line
                px, py = prev
                t = (ymax - py) / (y - py)
                f.line(X(px), Y(py), X(px + (x - px) * t), Y(ymax), stroke=stroke, lw=lw)
            return prev
        if prev:
            f.line(X(prev[0]), Y(prev[1]), X(x), Y(y), stroke=stroke, lw=lw)
        prev = (x, y)
    return prev


@figure("growth")
def _growth():
    f = Fig(640, 300, "Plot of log n, n, n log n, n squared, and 2 to the n for n up to 40.")
    L, R, T, B = 56, 560, 24, 262
    xmax, ymax = 40, 400
    X = lambda x: L + (R - L) * x / xmax
    Y = lambda y: B - (B - T) * y / ymax
    f.line(L, B, R + 10, B, stroke="muted")
    f.line(L, B, L, T - 8, stroke="muted")
    for x in (10, 20, 30, 40):
        f.line(X(x), B, X(x), B + 4, stroke="muted")
        f.text(X(x), B + 14, x, size=11, color="muted")
    for y in (100, 200, 300, 400):
        f.line(L - 4, Y(y), L, Y(y), stroke="muted")
        f.text(L - 8, Y(y), y, size=11, color="muted", anchor="e")
    f.text((L + R) / 2, B + 30, "input size n", size=12, color="muted", tex=r"input size $n$")
    f.text(L - 4, T - 14, "steps", size=12, color="muted", anchor="s")
    lg = lambda x: math.log2(x)
    _plot(f, lambda x: 2 ** x, 0, xmax, X, Y, ymax, "bad")
    _plot(f, lambda x: x * x, 0, xmax, X, Y, ymax, "accent2")
    _plot(f, lambda x: x * lg(x) if x > 1 else 0, 1, xmax, X, Y, ymax, "violet")
    _plot(f, lambda x: x, 0, xmax, X, Y, ymax, "accent")
    _plot(f, lambda x: lg(x) if x >= 1 else 0, 1, xmax, X, Y, ymax, "good")
    f.text(X(8.9) + 6, T + 6, "2ⁿ", color="bad", anchor="s", bold=True, tex=r"$2^n$")
    f.text(X(20) + 8, T + 6, "n²", color="accent2", anchor="s", bold=True, tex=r"$n^2$")
    f.text(R + 8, Y(40 * lg(40)), "n log n", color="violet", anchor="s", bold=True, tex=r"$n\log_2 n$")
    f.text(R + 8, Y(40) - 4, "n", color="accent", anchor="s", bold=True, tex=r"$n$")
    f.text(R + 8, Y(lg(40)) + 2, "log n", color="good", anchor="s", bold=True, tex=r"$\log_2 n$")
    return f


@figure("bigo")
def _bigo():
    f = Fig(640, 240, "f(n) = 3n + 40 lies below 5n for all n past 20.")
    L, R, T, B = 70, 520, 20, 200
    xmax, ymax = 50, 260
    X = lambda x: L + (R - L) * x / xmax
    Y = lambda y: B - (B - T) * y / ymax
    f.line(L, B, R + 10, B, stroke="muted")
    f.line(L, B, L, T - 6, stroke="muted")
    f.line(X(20), B, X(20), Y(250), stroke="muted", dashed=True)
    f.rect(L, T, X(20) - L, B - T, stroke=None, fill="muted-soft")
    f.line(X(0), Y(0), X(50), Y(250), stroke="accent2", lw=1.8)
    f.line(X(0), Y(40), X(50), Y(190), stroke="accent", lw=1.8)
    f.circle(X(20), Y(100), 3.2, stroke="ink", fill="ink")
    f.text(X(50) + 8, Y(250), "c · g(n) = 5n", color="accent2", anchor="s", bold=True, tex=r"$c\cdot g(n) = 5n$")
    f.text(X(50) + 8, Y(190), "f(n) = 3n + 40", color="accent", anchor="s", bold=True, tex=r"$f(n) = 3n + 40$")
    f.text(X(20), B + 14, "n₀ = 20", size=12, tex=r"$n_0 = 20$")
    f.text(X(10), T + 14, "ignored", size=11, color="muted", italic=True)
    f.text(X(35), B + 14, "f(n) ≤ c · g(n) from here on", size=11, color="muted",
           tex=r"$f(n) \le c\cdot g(n)$ from here on")
    f.text(L - 8, T, "steps", size=11, color="muted", anchor="e")
    f.text(R + 16, B, "n", size=11, color="muted", anchor="s", tex="$n$")
    return f


BS_VALS = [4, 9, 13, 18, 22, 27, 31, 36, 40, 45, 51, 57, 63, 68, 74]


@figure("bsearch")
def _bsearch():
    f = Fig(640, 170, "Binary search state: the first eight cells ruled out; lo at 8, mid at 11, hi at 15.")
    x0, w, y = 32, 38, 60
    fills = {i: ("accent-soft" if i >= 8 else None) for i in range(15)}
    fills[11] = "accent2-soft"
    texts = {i: "muted" for i in range(8)}
    cells(f, x0, y, BS_VALS, w=w, h=36, fills=fills, texts=texts)
    f.rect(x0 + 15 * w, y, w, 36, stroke="muted", dashed=True)
    f.text(x0 + 15 * w + w / 2, y + 47, 15, size=11, color="muted", mono=True)

    def marker(i, label, color, above=False):
        cx = x0 + i * w + w / 2
        if above:
            f.line(cx, y - 24, cx, y - 4, stroke=color, arrow=True)
            f.text(cx, y - 34, label, color=color, mono=True, bold=True, size=13)
        else:
            f.line(cx, y + 80, cx, y + 58, stroke=color, arrow=True)
            f.text(cx, y + 90, label, color=color, mono=True, bold=True, size=13)

    marker(8, "lo", "accent")
    marker(15, "hi", "accent")
    marker(11, "mid", "accent2", above=True)
    f.text(x0 + 4 * w, y - 22, "ruled out: all less than 57", size=11, color="muted", italic=True)
    return f


@figure("rowcol")
def _rowcol():
    f = Fig(640, 290, "A 3 by 8 grid laid out in memory, visited in row order and in column order.")
    x0, w, rows, cols = 32, 24, 3, 8
    n = rows * cols

    def strip(y, order, title):
        f.text(x0, y - 44, title, anchor="s", bold=True, size=13)
        for i in range(n):
            fill = "muted-soft" if (i // 4) % 2 == 0 else None
            f.rect(x0 + i * w, y, w, 26, fill=fill)
            f.text(x0 + i * w + w / 2, y + 13, order[i], size=10, mono=True)
        for r in range(rows):
            brace_label(f, x0 + r * cols * w + 2, x0 + (r + 1) * cols * w - 2, y + 30, f"row {r}", size=11)

    def hop(y, i, j, color):
        xa, xb = x0 + i * w + w / 2, x0 + j * w + w / 2
        span = abs(xb - xa)
        f.curve(xa, y - 2, (xa + xb) / 2, y - 6 - min(34, span * 0.22), xb, y - 2, stroke=color, arrow=True, lw=1.2)

    y1, y2 = 60, 204
    strip(y1, [i + 1 for i in range(n)], "Row order: consecutive addresses")
    for i in range(9):
        hop(y1, i, i + 1, "good")
    order = [0] * n
    k = 1
    for c in range(cols):
        for r in range(rows):
            order[r * cols + c] = k
            k += 1
    strip(y2, order, "Column order: a jump of one full row on every access")
    seq = [r * cols + c for c in range(3) for r in range(rows)]
    for a, b in zip(seq, seq[1:7]):
        hop(y2, a, b, "bad")
    f.text(x0 + n * w, y1 - 44, "numbers show the order of visits; shaded bands are cache lines",
           anchor="e", size=10, color="muted")
    return f


# ==========================================================================
# Chapter 3: vector
# ==========================================================================
def _obj_fields(f, x, y, title, rows, w=150, color="muted"):
    """A small 'object' box with labelled fields.  rows: [(name, value|None)].
    Returns {name: (cx, cy)} of the value boxes (for pointer tails)."""
    h = 22 + 30 * len(rows)
    f.rect(x, y, w, h, fill=f"{color}-soft", stroke=color)
    f.text(x + 8, y + 12, title, size=11, anchor="s", bold=True, color=color)
    out = {}
    for k, (name, val) in enumerate(rows):
        yy = y + 22 + 30 * k
        f.text(x + 74, yy + 12, name, mono=True, anchor="e", size=13)
        f.rect(x + 82, yy, w - 92, 24, fill="paper")
        cx, cy = x + 82 + (w - 92) / 2, yy + 12
        if val is None:
            f.circle(cx, cy, 2.8, fill="ink")
        else:
            f.text(cx, cy, val, mono=True, size=13)
        out[name] = (cx, cy)
    return out


@figure("vec-layout")
def _vec_layout():
    f = Fig(640, 200, "A vector object with data, size and capacity fields, and its heap block.")
    pos = _obj_fields(f, 24, 44, "the vector object", [("data_", None), ("size_", "5"), ("capacity_", "8")], w=170)
    x0, w, y = 270, 42, 70
    vals = ["10", "20", "30", "40", "50", "", "", ""]
    cells(f, x0, y, vals, w=w, h=36, fills={i: ("accent-soft" if i < 5 else "muted-soft") for i in range(8)})
    f.curve(pos["data_"][0], pos["data_"][1], 220, 60, x0 - 4, y + 14, arrow=True)
    brace_label(f, x0 + 2, x0 + 5 * w - 2, y + 54, "size = 5 (in use)", color="accent")
    brace_label(f, x0 + 2, x0 + 8 * w - 2, y - 8, "capacity = 8", color="muted", below=False)
    f.text(x0 + 6.5 * w, y + 68, "spare", size=11, color="muted", italic=True)
    f.text(x0 + 4 * w, 178, "one contiguous heap block", size=11, color="muted")
    return f


@figure("dynarray")
def _dynarray():
    f = Fig(640, 250, "A vector growing from capacity 4 to 8 to 16 as elements are appended.")
    x0, w = 150, 29

    def row(y, n_used, cap, copied, new, l1, l2):
        fills = {}
        for i in range(cap):
            fills[i] = ("accent2-soft" if i < copied else "accent-soft" if i == new else
                        "paper" if i < n_used else "muted-soft")
        vals = [str(i + 1) if i < n_used else "" for i in range(cap)]
        cells(f, x0, y, vals, w=w, h=30, fills=fills, index=False, size=12)
        f.text(x0 - 12, y + 8, l1, anchor="e", size=12, bold=True)
        f.text(x0 - 12, y + 23, l2, anchor="e", size=10, color="muted")

    row(34, 4, 4, 0, -1, "4 appends", "capacity 4, full")
    row(104, 5, 8, 4, 4, "5th append", "copy 4, then write")
    row(174, 9, 16, 8, 8, "9th append", "copy 8, then write")
    f.line(x0 + 2 * w, 68, x0 + 2 * w, 98, stroke="accent2", arrow=True)
    f.line(x0 + 4 * w, 138, x0 + 4 * w, 168, stroke="accent2", arrow=True)
    f.rect(x0, 222, 14, 14, fill="accent2-soft")
    f.text(x0 + 20, 229, "copied from the old block", size=11, anchor="s", color="muted")
    f.rect(x0 + 190, 222, 14, 14, fill="accent-soft")
    f.text(x0 + 210, 229, "newly appended", size=11, anchor="s", color="muted")
    f.rect(x0 + 320, 222, 14, 14, fill="muted-soft")
    f.text(x0 + 340, 229, "spare capacity", size=11, anchor="s", color="muted")
    return f


@figure("shallow-deep")
def _shallow_deep():
    f = Fig(640, 190, "Shallow copy: two objects share one block. Deep copy: each has its own.")

    def obj(x, y, name):
        f.text(x - 8, y + 14, name, mono=True, bold=True, anchor="e")
        f.rect(x, y, 56, 28, fill="muted-soft")
        f.circle(x + 28, y + 14, 2.8, fill="ink")
        return x + 28, y + 14

    # shallow
    f.text(150, 16, "member-by-member copy", bold=True, size=13)
    a = obj(50, 50, "a")
    b = obj(50, 120, "b")
    cells(f, 190, 82, ["1", "2"], w=40, h=34, fills={0: "bad-soft", 1: "bad-soft"}, index=False, stroke="bad")
    f.line(a[0], a[1], 186, 92, arrow=True)
    f.line(b[0], b[1], 186, 106, arrow=True)
    f.text(230, 134, "one block,", size=11, color="bad")
    f.text(230, 148, "two owners", size=11, color="bad")
    f.line(320, 10, 320, 180, stroke="muted", dashed=True)
    # deep
    f.text(480, 16, "copy constructor", bold=True, size=13)
    a = obj(380, 50, "a")
    b = obj(380, 120, "b")
    cells(f, 510, 47, ["1", "2"], w=40, h=34, fills={0: "good-soft", 1: "good-soft"}, index=False, stroke="good")
    cells(f, 510, 117, ["1", "2"], w=40, h=34, fills={0: "good-soft", 1: "good-soft"}, index=False, stroke="good")
    f.line(a[0], a[1], 506, 64, arrow=True)
    f.line(b[0], b[1], 506, 134, arrow=True)
    f.text(550, 168, "each owns its own block", size=11, color="good")
    return f


@figure("insert-shift")
def _insert_shift():
    f = Fig(640, 200, "Inserting 25 at index 2 shifts the later elements right by one.")
    x0, w = 150, 46
    f.text(x0 - 14, 66, "before", anchor="e", bold=True, size=12)
    cells(f, x0, 48, ["10", "20", "30", "40", "50", "", "", ""], w=w, h=36,
          fills={2: "accent2-soft", 3: "accent2-soft", 4: "accent2-soft", 5: "muted-soft", 6: "muted-soft", 7: "muted-soft"})
    for k, i in enumerate((4, 3, 2)):
        xa, xb = x0 + i * w + w / 2, x0 + (i + 1) * w + w / 2
        f.curve(xa, 46, (xa + xb) / 2, 18, xb, 46, stroke="accent2", arrow=True)
        f.text((xa + xb) / 2, 14, ("1st", "2nd", "3rd")[k], size=10, color="accent2")
    f.text(x0 - 14, 152, "after", anchor="e", bold=True, size=12)
    cells(f, x0, 134, ["10", "20", "25", "30", "40", "50", "", ""], w=w, h=36,
          fills={2: "accent-soft", 6: "muted-soft", 7: "muted-soft"})
    f.text(x0 + 2 * w + w / 2, 118, "insert(2, 25)", mono=True, size=12, color="accent")
    return f


# ==========================================================================
# Chapter 4: links
# ==========================================================================
def _head_box(f, x, y, label="head"):
    f.text(x + 22, y - 12, label, mono=True, bold=True, size=13)
    f.rect(x, y, 44, 34, fill="muted-soft")
    f.circle(x + 22, y + 17, 2.8, fill="ink")
    return x + 22, y + 17


@figure("linked-list")
def _linked_list():
    f = Fig(640, 120, "A head pointer and three list nodes holding 4, 1 and 3.")
    hx, hy = _head_box(f, 40, 50)
    xs = [150, 300, 450]
    prev = (hx, hy)
    for i, (x, v) in enumerate(zip(xs, ["4", "1", "3"])):
        out, inn = list_node(f, x, 50, v, w=84, h=34, null=(i == 2), fill="accent-soft")
        f.line(prev[0], prev[1], inn[0] - 2, inn[1], arrow=True)
        prev = out
    f.text(xs[0] + 26, 98, "value", size=10, color="muted")
    f.text(xs[0] + 68, 98, "next", size=10, color="muted")
    f.text(xs[2] + 68, 98, "nullptr", size=10, color="muted", mono=True)
    return f


@figure("list-remove")
def _list_remove():
    f = Fig(640, 250, "Removing a node by pointing its predecessor at its successor.")
    xs = [120, 250, 380, 510]
    vals = ["10", "20", "30", "40"]

    def row(y, removed):
        hx, hy = _head_box(f, 30, y)
        prev = (hx, hy)
        outs = []
        for i, (x, v) in enumerate(zip(xs, vals)):
            dead = removed and i == 1
            out, inn = list_node(f, x, y, v, w=84, h=34, null=(i == 3),
                                 fill=("bad-soft" if dead else "accent-soft"),
                                 stroke=("bad" if dead else "ink"))
            outs.append((out, inn))
            if removed and i == 1:
                pass
            elif removed and i == 2:
                # routed around node 1
                o = outs[0][0]
                f.curve(o[0], o[1], (o[0] + inn[0]) / 2, y - 36, inn[0] + 6, y - 2, stroke="good", arrow=True, lw=1.8)
            else:
                f.line(prev[0], prev[1], inn[0] - 2, inn[1], arrow=True)
            prev = out
        if removed:
            o = outs[1][0]
            f.line(o[0], o[1], outs[2][1][0] - 2, outs[2][1][1], stroke="muted", dashed=True, arrow=True)
        return outs

    f.text(30, 18, "after the search", size=12, bold=True, anchor="s")
    row(48, False)
    for i, name in ((0, "prev"), (1, "cur")):
        cx = xs[i] + 26
        f.line(cx, 112, cx, 88, stroke="accent2", arrow=True)
        f.text(cx, 122, name, mono=True, bold=True, color="accent2", size=13)
    f.text(30, 150, "after  prev->next = cur->next;  delete cur;", size=12, bold=True, anchor="s",
           tex=r"after\ \ \texttt{prev->next = cur->next; delete cur;}")
    row(190, True)
    f.text(xs[1] + 42, 240, "deleted", size=11, color="bad", italic=True)
    return f


@figure("dlist")
def _dlist():
    f = Fig(640, 190, "A circular doubly linked list with a sentinel node.")
    xs = [70, 220, 370, 520]
    labels = ["", "A", "B", "C"]
    y, w, h = 78, 70, 36
    for i, (x, lab) in enumerate(zip(xs, labels)):
        f.rect(x, y, w, h, fill=("muted-soft" if i == 0 else "accent-soft"), stroke=("muted" if i == 0 else "ink"))
        if lab:
            f.text(x + w / 2, y + h / 2, lab, mono=True)
    f.text(xs[0] + w / 2, y + h / 2, "sentinel", size=11, color="muted", italic=True)
    for a, b in zip(xs, xs[1:]):
        f.line(a + w, y + 11, b - 1, y + 11, arrow=True)                    # next
        f.line(b, y + 25, a + w + 1, y + 25, arrow=True, stroke="accent2")  # prev
    # wrap-around links
    f.curve(xs[3] + w / 2, y, 320, y - 86, xs[0] + w / 2, y - 2, arrow=True)
    f.curve(xs[0] + w / 2, y + h, 320, y + h + 86, xs[3] + w / 2, y + h + 2, arrow=True, stroke="accent2")
    f.text(320, 22, "next", size=11, mono=True)
    f.text(320, 170, "prev", size=11, mono=True, color="accent2")
    f.text(xs[1] + w / 2, y + h + 14, "front", size=10, color="muted")
    f.text(xs[3] + w / 2, y - 12, "back", size=10, color="muted")
    return f


@figure("ring")
def _ring():
    f = Fig(640, 190, "A ring buffer of capacity 8: elements at indices 5, 6, 7, 0 and 1.")
    x0, w, y = 136, 46, 70
    vals = ["60", "70", "", "", "", "30", "40", "50"]
    fills = {i: ("accent-soft" if vals[i] else "muted-soft") for i in range(8)}
    fills[5] = "accent2-soft"
    cells(f, x0, y, vals, w=w, h=38, fills=fills)
    cx = x0 + 5 * w + w / 2
    f.line(cx, y - 26, cx, y - 4, stroke="accent2", arrow=True)
    f.text(cx - 14, y - 36, "head = 5 (front)", color="accent2", bold=True, size=12, anchor="s",
           tex=r"\texttt{head = 5} (front)")
    cx2 = x0 + 2 * w + w / 2
    f.line(cx2, y - 26, cx2, y - 4, stroke="accent", arrow=True)
    f.text(cx2 + 14, y - 36, "next push: (5 + 5) % 8 = 2", color="accent", size=12, anchor="e",
           tex=r"next push: \texttt{(5 + 5) \% 8 = 2}")
    f.curve(x0 + 8 * w - w / 2, y + 62, x0 + 4 * w, y + 122, x0 + w / 2, y + 62, stroke="muted", arrow=True, dashed=True)
    f.text(x0 + 4 * w, y + 104, "wraps around", size=11, color="muted", italic=True)
    return f


@figure("cache-sim")
def _cache_sim():
    f = Fig(640, 250, "A vector fills consecutive cache lines; list nodes are scattered across them.")
    x0, w, n, per = 48, 17, 32, 8

    def strip(y, filled, title, note, color, ty=16):
        f.text(x0, y - ty, title, anchor="s", bold=True, size=13)
        f.text(x0 + n * w, y - ty, note, anchor="e", size=11, color="muted")
        for i in range(n):
            f.rect(x0 + i * w, y, w, 26, fill=(f"{color}-soft" if i in filled else None), stroke="muted", lw=0.8)
            if i in filled:
                f.text(x0 + i * w + w / 2, y + 13, filled[i], size=9, mono=True)
        for k in range(n // per):
            f.rect(x0 + k * per * w, y, per * w, 26, lw=1.6)
            f.text(x0 + k * per * w + per * w / 2, y + 38, f"cache line {k}", size=10, color="muted")

    strip(40, {i: i + 1 for i in range(12)}, "vector", "12 elements on 2 cache lines", "good")
    pos = [3, 17, 9, 28, 0, 22, 13, 6, 30, 19, 11, 25]
    strip(176, {p: k + 1 for k, p in enumerate(pos)}, "linked list",
          "12 nodes; nearly every hop lands on a different line", "bad", ty=62)
    for a, b in zip(pos, pos[1:6]):
        xa, xb = x0 + a * w + w / 2, x0 + b * w + w / 2
        f.curve(xa, 174, (xa + xb) / 2, 174 - 12 - min(34, abs(xb - xa) * 0.12), xb, 174, stroke="bad", arrow=True, lw=1.1)
    return f


# ==========================================================================
# Chapter 5: hashing
# ==========================================================================
@figure("chaining")
def _chaining():
    f = Fig(640, 236, "A bucket array of six buckets, each leading to a chain of entries.")
    bx, by, bw, bh, gap = 250, 22, 44, 28, 34
    chains = {0: [("dog", "3")], 2: [("fox", "7"), ("cat", "1")], 3: [("owl", "4")], 5: [("bee", "9")]}
    for i in range(6):
        y = by + i * gap
        f.text(bx - 10, y + bh / 2, i, mono=True, size=11, color="muted", anchor="e")
        f.rect(bx, y, bw, bh, fill=("accent-soft" if i == 2 else "muted-soft"))
        if i in chains:
            f.circle(bx + bw / 2, y + bh / 2, 2.8, fill="ink")
            f.line(bx + bw / 2, y + bh / 2, bx + bw + 30, y + bh / 2, arrow=True)
            for j, (k, v) in enumerate(chains[i]):
                x = bx + bw + 34 + j * 96
                hit = (k == "cat")
                f.rect(x, y, 96, bh, fill=("accent2-soft" if hit else "paper"), stroke=("accent2" if hit else "ink"))
                f.line(x + 58, y, x + 58, y + bh)
                f.text(x + 29, y + bh / 2, k, mono=True, size=13)
                f.text(x + 77, y + bh / 2, v, mono=True, size=13)
    f.text(bx + bw / 2, by + 6 * gap + 6, "buckets", size=11, color="muted")
    f.text(bx + bw + 34 + 96, by + 6 * gap + 6, "chains of (key, value) entries", size=11, color="muted")
    # the lookup
    f.text(24, by + 2 * gap - 18, 'find("cat")', mono=True, size=13, anchor="s", bold=True)
    f.text(24, by + 2 * gap + 2, 'hash("cat") % 6', mono=True, size=12, anchor="s", color="accent")
    f.text(24, by + 2 * gap + 20, "= 2", mono=True, size=12, anchor="s", color="accent")
    f.line(150, by + 2 * gap + bh / 2, bx - 26, by + 2 * gap + bh / 2, stroke="accent", arrow=True)
    return f


@figure("probing")
def _probing():
    f = Fig(640, 176, "A linear probing table of 12 slots; key 29 probes slots 5, 6, 7 and lands in 8.")
    x0, w, y = 44, 46, 84
    vals = ["24", "", "del", "", "", "17", "6", "41", "29", "", "22", ""]
    fills = {0: "paper", 5: "accent-soft", 6: "accent-soft", 7: "accent-soft", 8: "accent2-soft", 10: "paper", 2: "muted-soft"}
    texts = {2: "muted"}
    cells(f, x0, y, vals, w=w, h=38, fills=fills, texts=texts)
    cx = lambda i: x0 + i * w + w / 2
    f.text(cx(5), 20, "29 % 12 = 5", mono=True, size=13, color="accent", bold=True)
    f.line(cx(5), 30, cx(5), y - 4, stroke="accent", arrow=True)
    for i in (5, 6, 7):
        f.curve(cx(i) + 6, y - 6, cx(i) + w / 2, y - 30, cx(i + 1) - 6, y - 6, stroke="accent2", arrow=True)
    f.text(cx(8) + 50, y - 26, "first free slot", size=11, color="accent2", anchor="s")
    f.text(cx(2), y + 62, "tombstone", size=10, color="muted", italic=True)
    f.text(cx(6), y + 62, "a cluster", size=10, color="accent", italic=True)
    f.text(x0 + 12 * w, y + 78, "h(k) = k mod 12", size=11, color="muted", anchor="e", tex=r"$h(k) = k \bmod 12$")
    return f


@figure("tombstone")
def _tombstone():
    f = Fig(640, 270, "Erasing from a probing table with and without a tombstone.")
    x0, w = 250, 50

    def row(y, vals, fills, title, sub, texts=None):
        f.text(x0 - 16, y + 12, title, anchor="e", bold=True, size=12)
        f.text(x0 - 16, y + 28, sub, anchor="e", size=10, color="muted")
        cells(f, x0, y, vals, w=w, h=36, fills=fills, index_from=2, texts=texts or {})

    cx = lambda i: x0 + i * w + w / 2
    row(26, ["", "A", "B", "C", "", ""], {1: "accent-soft", 2: "accent-soft", 3: "accent-soft"},
        "A, B, C inserted", "all three have home slot 3")
    row(112, ["", "A", "", "C", "", ""], {1: "accent-soft", 2: "bad-soft", 3: "accent-soft"},
        "B's slot emptied", "find(C) is cut short")
    f.curve(cx(1), 110, (cx(1) + cx(2)) / 2, 92, cx(2), 110, stroke="bad", arrow=True)
    f.text(cx(2) + 8, 96, "empty: stop.  \"C is not here\"", size=11, color="bad", anchor="s")
    row(198, ["", "A", "del", "C", "", ""], {1: "accent-soft", 2: "muted-soft", 3: "good-soft"},
        "B's slot marked deleted", "find(C) passes over it", texts={2: "muted"})
    f.curve(cx(1), 196, (cx(1) + cx(2)) / 2, 178, cx(2), 196, stroke="good", arrow=True)
    f.curve(cx(2), 196, (cx(2) + cx(3)) / 2, 178, cx(3), 196, stroke="good", arrow=True)
    f.text(cx(3) + 14, 182, "found", size=11, color="good", anchor="s")
    return f


@figure("bloom")
def _bloom():
    f = Fig(640, 216, "A 32-bit Bloom filter with two keys added and a false positive on a third.")
    x0, w, y = 48, 17, 92
    cat, dog, emu = [3, 14, 27], [9, 14, 21], [3, 9, 21]
    ones = set(cat) | set(dog)
    for i in range(32):
        f.rect(x0 + i * w, y, w, 26, fill=("accent-soft" if i in ones else None))
        f.text(x0 + i * w + w / 2, y + 13, "1" if i in ones else "0", size=10, mono=True,
               color=("ink" if i in ones else "muted"))
    cx = lambda i: x0 + i * w + w / 2
    for name, pos, x, color in (("add(\"cat\")", cat, 170, "accent"), ("add(\"dog\")", dog, 420, "accent2")):
        f.text(x, 20, name, mono=True, bold=True, size=13, color=color)
        for p in pos:
            f.line(x, 32, cx(p), y - 3, stroke=color, arrow=True, lw=1.1)
    f.text(250, 196, "might_contain(\"emu\")", mono=True, bold=True, size=13, color="bad")
    for p in emu:
        f.line(250, 184, cx(p), y + 29, stroke="bad", arrow=True, lw=1.1, dashed=True)
    f.text(360, 196, "all three bits are 1: a false positive", size=11, color="bad", anchor="s")
    f.text(x0, y + 40, "0", size=9, color="muted", mono=True)
    f.text(cx(31), y + 40, "31", size=9, color="muted", mono=True)
    return f


# ==========================================================================
# Chapter 6: trees
# ==========================================================================
BST8 = (50, (30, 20, (40, 35)), (70, 60, 80))


@figure("tree-anatomy")
def _tree_anatomy():
    f = Fig(640, 250, "A binary tree with root, leaves, depths and a subtree labelled.")
    x0, y0, dx, dy = 150, 40, 48, 56
    nodes, _ = tree_positions(BST8, x0, y0, dx, dy)
    # shade the left subtree of the root
    f.rect(x0 - 28, y0 + dy - 26, 3 * dx + 56, 2 * dy + 52, fill="accent-soft", stroke=None, rx=12)
    for d in range(4):
        y = y0 + d * dy
        f.line(x0 + 7 * dx + 36, y, 560, y, stroke="muted", dashed=True, lw=0.8)
        f.text(566, y, f"depth {d}", size=11, color="muted", anchor="s")
    draw_tree(f, BST8, x0, y0, dx, dy)
    rx, ry = nodes[""][1], nodes[""][2]
    f.text(rx + 70, ry - 20, "root", size=12, bold=True, anchor="s")
    f.line(rx + 66, ry - 18, rx + 19, ry - 6, arrow=True)
    lx, ly = nodes["LL"][1], nodes["LL"][2]
    f.text(lx - 70, ly + 34, "a leaf", size=12, bold=True)
    f.line(lx - 52, ly + 26, lx - 16, ly + 10, arrow=True)
    f.text(x0 + 2 * dx - 44, y0 + 3 * dy + 40, "the left subtree of the root", size=11, color="accent")
    px, py = nodes["R"][1], nodes["R"][2]
    f.text(px + 62, py - 26, "parent of 60 and 80", size=11, color="muted", anchor="s")
    f.line(px + 58, py - 22, px + 17, py - 9, stroke="muted", arrow=True)
    return f


@figure("bst")
def _bst():
    f = Fig(640, 250, "A binary search tree of eight keys with the search path to 35 highlighted.")
    hl = {50: "accent-soft", 30: "accent-soft", 40: "accent-soft", 35: "accent2-soft"}
    st = {50: "accent", 30: "accent", 40: "accent", 35: "accent2"}
    draw_tree(f, BST8, 90, 36, 48, 58, fills=hl, strokes=st)
    f.text(480, 50, "search for 35", bold=True, size=13, anchor="s")
    for k, line in enumerate(("35 < 50: go left", "35 > 30: go right", "35 < 40: go left", "35 = 35: found")):
        f.text(480, 76 + 22 * k, line, mono=True, size=12, anchor="s",
               color=("accent2" if k == 3 else "ink"))
    return f


@figure("bst-erase")
def _bst_erase():
    f = Fig(640, 250, "Removing a node with two children by replacing its key with its successor's.")
    draw_tree(f, BST8, 84, 50, 32, 56, fills={30: "bad-soft", 35: "accent2-soft"},
              strokes={30: "bad", 35: "accent2"}, r=15)
    f.text(84 + 3.5 * 32, 16, "before: erase 30", bold=True, size=13)
    f.text(92, 100, "two", size=10, color="bad", anchor="e")
    f.text(92, 112, "children", size=10, color="bad", anchor="e")
    f.text(84 + 2 * 32, 244, "successor: leftmost in right subtree", size=10, color="accent2")
    f.line(322, 120, 356, 120, arrow=True, lw=1.8)
    after = (50, (35, 20, 40), (70, 60, 80))
    draw_tree(f, after, 384, 50, 32, 56, fills={35: "accent2-soft"}, strokes={35: "accent2"}, r=15)
    f.text(384 + 3 * 32, 16, "after", bold=True, size=13)
    f.text(384 + 3 * 32, 244, "35 moved up; its old node is gone", size=10, color="muted")
    return f


@figure("shapes")
def _shapes():
    f = Fig(640, 262, "A balanced tree and a degenerate chain holding the same seven keys.")
    draw_tree(f, (4, (2, 1, 3), (6, 5, 7)), 40, 60, 38, 56, r=16)
    f.text(40 + 3 * 38, 20, "inserted as 4, 2, 6, 1, 3, 5, 7", size=12, bold=True)
    f.text(40 + 3 * 38, 200, "height 3", size=12, color="good")
    chain = None
    for k in range(7, 0, -1):
        chain = (k, None, chain)
    draw_tree(f, chain, 370, 40, 34, 31, r=13, size=11)
    f.text(430, 20, "inserted as 1, 2, 3, 4, 5, 6, 7", size=12, bold=True)
    f.text(430, 200, "height 7", size=12, color="bad")
    f.line(320, 10, 320, 250, stroke="muted", dashed=True)
    return f


# ==========================================================================
# Chapter 7: balance
# ==========================================================================
def _tri(f, cx, cy, label, fill="muted-soft"):
    f.poly([(cx, cy - 16), (cx - 24, cy + 26), (cx + 24, cy + 26)], fill=fill)
    f.text(cx, cy + 12, label, italic=True, size=13, tex=f"${label}$")
    return cx, cy - 16


def _vnode(f, x, y, label, fill="paper", stroke="ink", r=17):
    f.circle(x, y, r, fill=fill, stroke=stroke)
    f.text(x, y, label, italic=True, size=14, tex=f"${label}$")


@figure("rotation")
def _rotation():
    f = Fig(640, 220, "A right rotation and its inverse, a left rotation.")

    def link(a, b, r1=17, r2=0):
        edge_between(f, a, b, r1, r2)

    # left tree: y on top
    y, x = (170, 40), (110, 100)
    C, A, B = (232, 110), (68, 170), (152, 170)
    link(y, x, 17, 17); link(y, (C[0], C[1] - 16)); link(x, (A[0], A[1] - 16)); link(x, (B[0], B[1] - 16))
    _vnode(f, *y, "y", fill="accent2-soft", stroke="accent2")
    _vnode(f, *x, "x", fill="accent-soft", stroke="accent")
    _tri(f, *C, "C"); _tri(f, *A, "A"); _tri(f, *B, "B", fill="good-soft")
    # right tree: x on top
    x2, y2 = (450, 40), (510, 100)
    A2, B2, C2 = (388, 110), (468, 170), (552, 170)
    link(x2, y2, 17, 17); link(x2, (A2[0], A2[1] - 16)); link(y2, (B2[0], B2[1] - 16)); link(y2, (C2[0], C2[1] - 16))
    _vnode(f, *x2, "x", fill="accent-soft", stroke="accent")
    _vnode(f, *y2, "y", fill="accent2-soft", stroke="accent2")
    _tri(f, *A2, "A"); _tri(f, *B2, "B", fill="good-soft"); _tri(f, *C2, "C")
    f.line(276, 78, 348, 78, arrow=True, lw=1.8)
    f.text(312, 62, "rotate right at y", size=12, tex=r"rotate right at $y$")
    f.line(348, 112, 276, 112, arrow=True, lw=1.8)
    f.text(312, 128, "rotate left at x", size=12, tex=r"rotate left at $x$")
    f.text(320, 206, "B is the only subtree that changes parent", size=11, color="good",
           tex=r"$B$ is the only subtree that changes parent")
    return f


@figure("avl-cases")
def _avl_cases():
    f = Fig(640, 300, "The outside and inside cases of AVL rebalancing.")
    kw = dict(dx=34, dy=42, r=14, size=12)
    hot = {"fills": {30: "bad-soft"}, "strokes": {30: "bad"}}
    f.text(20, 22, "Outside case (left-left): one rotation", bold=True, size=13, anchor="s")
    draw_tree(f, (30, (20, 10)), 130, 56, subs={30: "+2", 20: "+1"}, **hot, **kw)
    f.line(250, 96, 330, 96, arrow=True, lw=1.6)
    f.text(290, 82, "right at 30", size=11)
    draw_tree(f, (20, 10, 30), 370, 70, subs={20: "0"}, fills={20: "good-soft"}, strokes={20: "good"}, **kw)

    f.text(20, 166, "Inside case (left-right): two rotations", bold=True, size=13, anchor="s")
    draw_tree(f, (30, (10, None, 20)), 60, 196, subs={30: "+2", 10: "-1"}, **hot, **kw)
    f.line(178, 240, 246, 240, arrow=True, lw=1.6)
    f.text(212, 226, "left at 10", size=11)
    draw_tree(f, (30, (20, 10)), 270, 196, subs={30: "+2", 20: "+1"}, **hot, **kw)
    f.line(388, 240, 456, 240, arrow=True, lw=1.6)
    f.text(422, 226, "right at 30", size=11)
    draw_tree(f, (20, 10, 30), 490, 212, subs={20: "0"}, fills={20: "good-soft"}, strokes={20: "good"}, **kw)
    return f


@figure("avl")
def _avl():
    f = Fig(640, 210, "An AVL tree with balance factors after a double rotation.")
    spec = (30, (20, 10, 25), (40, None, 50))
    draw_tree(f, spec, 170, 40, 56, 60,
              subs={30: "0", 20: "0", 40: "-1", 10: "0", 25: "0", 50: "0"},
              fills={30: "accent-soft", 20: "accent-soft", 40: "accent-soft", 25: "accent2-soft"},
              strokes={25: "accent2"})
    f.text(500, 60, "inserted last: 25", size=12, color="accent2", anchor="s")
    f.text(500, 84, "balance factor =", size=11, color="muted", anchor="s")
    f.text(500, 100, "left height - right height", size=11, color="muted", anchor="s")
    return f


def _bnode(f, cx, y, keys, w=32, h=30, fill="paper", stroke="ink"):
    x = cx - len(keys) * w / 2
    cells(f, x, y, [str(k) for k in keys], w=w, h=h, fills={i: fill for i in range(len(keys))},
          index=False, size=13, stroke=stroke)
    gaps = [(x + i * w, y + h) for i in range(len(keys) + 1)]
    return gaps, (cx, y)


@figure("btree")
def _btree():
    f = Fig(640, 190, "A two-level B-tree: a root with two keys and three leaf children.")
    gaps, _ = _bnode(f, 320, 36, [30, 60], w=40, fill="accent-soft")
    for i, (cx, keys) in enumerate(((130, [10, 20]), (320, [40, 50]), (520, [70, 80, 90]))):
        _, top = _bnode(f, cx, 124, keys, w=40)
        f.line(gaps[i][0], gaps[i][1], top[0], top[1] - 2, arrow=True)
    f.text(130, 172, "keys < 30", size=11, color="muted", tex=r"keys $< 30$")
    f.text(320, 172, "30 < keys < 60", size=11, color="muted", tex=r"$30 <$ keys $< 60$")
    f.text(520, 172, "keys > 60", size=11, color="muted", tex=r"keys $> 60$")
    return f


@figure("btree-split")
def _btree_split():
    f = Fig(640, 200, "A leaf with four keys splits; its middle key moves up into the parent.")
    w = 28
    f.text(150, 16, "after inserting 95: overfull", bold=True, size=12)
    gaps, _ = _bnode(f, 140, 40, [30, 60], w=w, fill="accent-soft")
    for i, (cx, keys, fill) in enumerate(((45, [10, 20], "paper"), (120, [40, 50], "paper"),
                                          (225, [70, 80, 90, 95], "bad-soft"))):
        _, top = _bnode(f, cx, 120, keys, w=w, fill=fill, stroke=("bad" if i == 2 else "ink"))
        f.line(gaps[i][0], gaps[i][1], top[0], top[1] - 2, arrow=True)
    f.text(225 + 14, 166, "middle key", size=10, color="bad")
    f.line(298, 96, 330, 96, arrow=True, lw=1.8)
    f.text(480, 16, "after the split", bold=True, size=12)
    gaps, _ = _bnode(f, 480, 40, [30, 60, 90], w=w, fill="accent-soft")
    f.rect(480 + w / 2, 40, w, 30, fill="accent2-soft", stroke="accent2")
    f.text(480 + w, 55, "90", mono=True, size=13)
    for i, (cx, keys) in enumerate(((370, [10, 20]), (445, [40, 50]), (520, [70, 80]), (592, [95]))):
        fill = "good-soft" if i >= 2 else "paper"
        _, top = _bnode(f, cx, 120, keys, w=w, fill=fill)
        f.line(gaps[i][0], gaps[i][1], top[0], top[1] - 2, arrow=True)
    f.text(556, 166, "two half-full leaves", size=10, color="good")
    return f


# ==========================================================================
# Chapter 8: heaps and sorting
# ==========================================================================
@figure("heap")
def _heap():
    f = Fig(640, 270, "A min-heap of eight elements drawn as a tree and as an array.")
    vals = [1, 3, 2, 7, 4, 5, 9, 8]
    pos = heap_positions(len(vals), 320, 30, 440, 48)
    hot = {1: "accent2-soft", 3: "accent-soft", 4: "accent-soft"}
    for i in range(1, len(vals)):
        edge_between(f, pos[(i - 1) // 2], pos[i], 16, 16)
    for i, (x, y) in enumerate(pos):
        f.circle(x, y, 16, fill=hot.get(i, "paper"),
                 stroke=("accent2" if i == 1 else "accent" if i in (3, 4) else "ink"))
        f.text(x, y, vals[i], mono=True, size=13)
        f.text(x + 22, y - 12, i, mono=True, size=10, color="muted")
    x0, w, y = 144, 44, 204
    cells(f, x0, y, [str(v) for v in vals], w=w, h=34, fills=hot)
    cx = lambda i: x0 + i * w + w / 2
    f.curve(cx(1), y - 2, (cx(1) + cx(3)) / 2, y - 26, cx(3), y - 2, stroke="accent", arrow=True)
    f.curve(cx(1), y - 2, (cx(1) + cx(4)) / 2, y - 36, cx(4), y - 2, stroke="accent", arrow=True)
    f.text(x0 + 8 * w + 14, y + 10, "children of index 1:", size=11, color="muted", anchor="s")
    f.text(x0 + 8 * w + 14, y + 26, "2·1+1 = 3,  2·1+2 = 4", size=11, color="accent", anchor="s", mono=True,
           tex=r"$2\cdot 1+1 = 3,\ \ 2\cdot 1+2 = 4$")
    f.text(x0 - 12, y + 17, "array", size=11, color="muted", anchor="e")
    return f


@figure("merge")
def _merge():
    f = Fig(640, 220, "Two sorted runs being merged into one output array.")
    w = 44
    lx, rx, oy = 60, 360, 150
    cells(f, lx, 44, ["2", "5", "9", "14"], w=w, h=34, index=False,
          fills={0: "muted-soft", 1: "muted-soft", 2: "accent2-soft"}, texts={0: "muted", 1: "muted"})
    cells(f, rx, 44, ["1", "7", "12", "20"], w=w, h=34, index=False,
          fills={0: "muted-soft", 1: "muted-soft", 2: "accent2-soft"}, texts={0: "muted", 1: "muted"})
    f.text(lx + 2 * w, 24, "left run (sorted)", size=12, bold=True)
    f.text(rx + 2 * w, 24, "right run (sorted)", size=12, bold=True)
    ox = 144
    cells(f, ox, oy, ["1", "2", "5", "7", "", "", "", ""], w=w, h=34, index=False,
          fills={0: "good-soft", 1: "good-soft", 2: "good-soft", 3: "good-soft", 4: "paper"})
    f.text(ox + 4 * w, oy + 52, "output", size=12, bold=True)
    f.text(lx + 2.5 * w, 96, "i", mono=True, bold=True, color="accent2")
    f.text(rx + 2.5 * w, 96, "j", mono=True, bold=True, color="accent2")
    f.text(320, 62, "9 < 12 ?", mono=True, size=13, color="accent2", tex=r"\texttt{9 < 12}\,?")
    f.curve(lx + 2.5 * w, 80, lx + 3 * w + 30, 120, ox + 4.5 * w, oy - 3, stroke="good", arrow=True, dashed=True)
    return f


@figure("mergesort-tree")
def _mergesort_tree():
    f = Fig(640, 262, "The recursion tree of mergesort: n work on each of log n levels.")
    L, W, h = 110, 384, 26
    levels = [(1, "n"), (2, "n/2"), (4, "n/4"), (8, "n/8")]
    for d, (count, lab) in enumerate(levels):
        y = 24 + d * 44
        bw = W / count
        for k in range(count):
            f.rect(L + k * bw + 3, y, bw - 6, h, fill="accent-soft", rx=3)
            f.text(L + k * bw + bw / 2, y + h / 2, lab, size=(13 if count < 8 else 11),
                   tex=f"${lab}$")
        f.text(L + W + 20, y + h / 2, "n work", size=12, color="accent2", anchor="s", tex="$n$ work")
        if d > 0:
            pw = W / (count // 2)
            for k in range(count):
                px = L + (k // 2) * pw + pw / 2
                f.line(px, y - 18, L + k * bw + bw / 2, y, stroke="muted", lw=0.9)
    y = 24 + 4 * 44
    f.text(L + W / 2, y + 2, "· · ·", size=16, color="muted", tex=r"$\cdots$")
    y += 22
    for k in range(32):
        f.rect(L + k * 12 + 1.5, y, 9, 16, fill="accent-soft", lw=0.8)
    f.text(L + W + 20, y + 8, "n work", size=12, color="accent2", anchor="s", tex="$n$ work")
    f.text(L + W / 2, y + 28, "pieces of size 1", size=11, color="muted")
    f.line(60, 24, 60, y + 16, stroke="muted")
    f.line(56, 24, 64, 24, stroke="muted")
    f.line(56, y + 16, 64, y + 16, stroke="muted")
    f.text(50, 125, "log n", size=12, color="muted", anchor="e", tex=r"$\log_2 n$")
    f.text(50, 141, "levels", size=12, color="muted", anchor="e")
    return f


@figure("partition")
def _partition():
    f = Fig(640, 150, "The four regions of the three-way partition and their boundary indices.")
    x0, y, h = 60, 46, 40
    parts = [(110, "< pivot", "accent-soft", r"$<$ pivot"), (90, "= pivot", "good-soft", r"$=$ pivot"),
             (190, "not yet examined", "paper", None), (130, "> pivot", "accent2-soft", r"$>$ pivot")]
    x = x0
    bounds = [x0]
    for wd, lab, fill, tex in parts:
        f.rect(x, y, wd, h, fill=fill)
        f.text(x + wd / 2, y + h / 2, lab, size=13, tex=tex, italic=(tex is None), color=("muted" if tex is None else "ink"))
        x += wd
        bounds.append(x)
    for bx, name in zip(bounds, ["lo", "lt", "i", "gt", "hi"]):
        f.line(bx, y + h + 26, bx, y + h + 4, stroke="accent2" if name in ("lt", "i", "gt") else "muted", arrow=True)
        f.text(bx, y + h + 38, name, mono=True, bold=True, size=13,
               color="accent2" if name in ("lt", "i", "gt") else "muted")
    f.text(bounds[2] + 12, y - 14, "a[i] is examined next", size=11, color="muted", anchor="s",
           tex=r"\texttt{a[i]} is examined next")
    f.line(bounds[2] + 8, y - 8, bounds[2] + 8, y - 1, stroke="muted")
    return f


@figure("sorting")
def _sorting():
    f = Fig(640, 220, "Bars of different heights; the first four are in increasing order.")
    vals = [3, 5, 8, 9, 4, 7, 2, 6, 1, 10]
    x0, w, base = 110, 42, 180
    for i, v in enumerate(vals):
        fill = "accent-soft" if i < 4 else "accent2-soft" if i == 4 else "paper"
        stroke = "accent" if i < 4 else "accent2" if i == 4 else "ink"
        f.rect(x0 + i * w + 4, base - v * 15, w - 8, v * 15, fill=fill, stroke=stroke)
        f.text(x0 + i * w + w / 2, base + 12, v, mono=True, size=12)
    brace_label(f, x0 + 4, x0 + 4 * w - 4, base + 24, "sorted so far", color="accent")
    f.text(x0 + 4 * w + w / 2, base - 4 * 15 - 12, "next", size=11, color="accent2", bold=True)
    f.curve(x0 + 4.5 * w, base - 4 * 15 - 4, x0 + 3 * w, base - 9 * 15 - 30, x0 + 1.5 * w, base - 5 * 15 - 6,
            stroke="accent2", arrow=True, dashed=True)
    return f


# ==========================================================================
# Chapter 9: graphs
# ==========================================================================
EX_EDGES = [(0, 1), (0, 2), (1, 3), (2, 3), (3, 4), (4, 5)]


def _draw_graph(f, pos, edges, r=15, fills=None, strokes=None, heavy=(), labels=None, size=13,
                directed=False, weights=None):
    fills, strokes = fills or {}, strokes or {}
    for k, (a, b) in enumerate(edges):
        hv = (a, b) in heavy or (b, a) in heavy
        edge_between(f, pos[a], pos[b], r, r, stroke=("accent" if hv else "ink"),
                     lw=(2.6 if hv else 1.2), arrow=directed)
        if weights:
            (x1, y1), (x2, y2) = pos[a], pos[b]
            d = math.hypot(x2 - x1, y2 - y1)
            nx, ny = -(y2 - y1) / d, (x2 - x1) / d
            f.text((x1 + x2) / 2 + nx * 12, (y1 + y2) / 2 + ny * 12, weights[k], size=12, mono=True,
                   color="accent2", bold=True)
    for v, (x, y) in pos.items():
        f.circle(x, y, r, fill=fills.get(v, "paper"), stroke=strokes.get(v, "ink"))
        f.text(x, y, labels[v] if labels else v, mono=True, size=size)


@figure("graph-rep")
def _graph_rep():
    f = Fig(640, 200, "A six-vertex graph with its adjacency lists and adjacency matrix.")
    pos = {0: (36, 100), 1: (92, 54), 2: (92, 146), 3: (150, 100), 4: (204, 100), 5: (258, 100)}
    _draw_graph(f, pos, EX_EDGES, r=13, size=12)
    f.text(340, 20, "adjacency lists", bold=True, size=12)
    adj = {v: [] for v in range(6)}
    for a, b in EX_EDGES:
        adj[a].append(b)
        adj[b].append(a)
    for v in range(6):
        y = 46 + v * 24
        f.rect(300, y - 10, 22, 20, fill="muted-soft")
        f.text(311, y, v, mono=True, size=12)
        f.line(322, y, 338, y, arrow=True, lw=1)
        for k, nb in enumerate(sorted(adj[v])):
            f.rect(340 + k * 22, y - 10, 22, 20)
            f.text(351 + k * 22, y, nb, mono=True, size=12)
    f.text(540, 20, "adjacency matrix", bold=True, size=12)
    mx, my, c = 478, 46, 22
    for v in range(6):
        f.text(mx + v * c + c / 2, my - 10, v, mono=True, size=10, color="muted")
        f.text(mx - 10, my + v * c + c / 2, v, mono=True, size=10, color="muted")
    for u in range(6):
        for v in range(6):
            on = v in adj[u]
            f.rect(mx + v * c, my + u * c, c, c, fill=("accent-soft" if on else None), stroke="muted", lw=0.8)
            f.text(mx + v * c + c / 2, my + u * c + c / 2, 1 if on else 0, mono=True, size=11,
                   color=("ink" if on else "muted"))
    return f


@figure("bfs-layers")
def _bfs_layers():
    f = Fig(640, 210, "The example graph with each vertex labelled by its BFS distance from vertex 0.")
    pos = {0: (70, 105), 1: (180, 50), 2: (180, 160), 3: (300, 105), 4: (420, 105), 5: (540, 105)}
    dist = {0: 0, 1: 1, 2: 1, 3: 2, 4: 3, 5: 4}
    tints = ["accent2-soft", "accent-soft", "good-soft", "violet-soft", "muted-soft"]
    _draw_graph(f, pos, EX_EDGES, r=17, fills={v: tints[d] for v, d in dist.items()},
                heavy={(0, 1), (0, 2), (1, 3), (3, 4), (4, 5)})
    for v, (x, y) in pos.items():
        f.text(x, y + 30, f"dist {dist[v]}", size=11, color="muted")
    f.text(70, 60, "source", size=11, color="accent2", bold=True)
    for d, x in enumerate((70, 180, 300, 420, 540)):
        f.text(x, 16, f"wave {d}", size=11, color="muted", italic=True)
    return f


@figure("topo")
def _topo():
    f = Fig(640, 170, "Six courses in a row, in an order where every prerequisite arrow points right.")
    names = ["Intro", "Discrete Math", "Systems", "Data Structures", "Operating Systems", "Algorithms"]
    short = ["Intro", "Discrete", "Systems", "Data", "Operating", "Algorithms"]
    short2 = ["", "Math", "", "Structures", "Systems", ""]
    xs = [50, 150, 250, 350, 462, 578]
    y, w, h = 86, 84, 40
    for x, a, b in zip(xs, short, short2):
        f.rect(x - w / 2, y - h / 2, w, h, fill="accent-soft", rx=6)
        if b:
            f.text(x, y - 8, a, size=12)
            f.text(x, y + 8, b, size=12)
        else:
            f.text(x, y, a, size=12)
    # edges by position index: Intro(0)->Systems(2), Intro(0)->Data(3), Systems(2)->OS(4),
    # Data(3)->OS(4), Data(3)->Algorithms(5), Discrete(1)->Algorithms(5)
    for a, b, up in ((0, 2, True), (0, 3, False), (2, 4, True), (3, 4, None), (3, 5, False), (1, 5, True)):
        xa, xb = xs[a], xs[b]
        if up is None:
            f.line(xa + w / 2, y, xb - w / 2 - 1, y, arrow=True)
        elif up:
            f.curve(xa + 10, y - h / 2, (xa + xb) / 2, y - h / 2 - 26 - (xb - xa) * 0.16, xb - 10, y - h / 2 - 1, arrow=True)
        else:
            f.curve(xa + 10, y + h / 2, (xa + xb) / 2, y + h / 2 + 26 + (xb - xa) * 0.12, xb - 10, y + h / 2 + 1, arrow=True)
    return f


@figure("graph")
def _graph():
    f = Fig(640, 230, "A weighted graph with shortest-path distances from vertex 0.")
    pos = {0: (80, 115), 1: (240, 46), 2: (220, 186), 3: (410, 115), 4: (560, 115)}
    edges = [(0, 1), (0, 2), (2, 1), (1, 3), (2, 3), (3, 4)]
    weights = [4, 1, 2, 5, 8, 3]
    dist = {0: 0, 1: 3, 2: 1, 3: 8, 4: 11}
    _draw_graph(f, pos, edges, r=17, weights=weights, heavy={(0, 2), (2, 1), (1, 3), (3, 4)},
                fills={v: "accent-soft" for v in pos}, strokes={0: "accent2"})
    for v, (x, y) in pos.items():
        dy = -30 if v in (1,) else 32
        f.text(x, y + dy, f"dist {dist[v]}", size=12, color="accent", bold=True)
    f.text(80, 76, "source", size=11, color="accent2", bold=True)
    return f


@figure("unionfind")
def _unionfind():
    f = Fig(640, 260, "A union-find forest of three trees and the parent array that represents it.")
    pos = {0: (150, 36), 1: (96, 100), 2: (204, 100), 3: (204, 164), 4: (370, 36), 5: (320, 100),
           6: (420, 100), 7: (530, 36)}
    parent = [0, 0, 0, 2, 4, 4, 4, 7]
    for v, p in enumerate(parent):
        if p != v:
            edge_between(f, pos[v], pos[p], 16, 16, arrow=True)
    for v, (x, y) in pos.items():
        root = parent[v] == v
        f.circle(x, y, 16, fill=("accent2-soft" if root else "paper"), stroke=("accent2" if root else "ink"))
        f.text(x, y, v, mono=True, size=13)
    f.text(530, 70, "a set of one", size=10, color="muted")
    f.text(150, 10, "root", size=10, color="accent2")
    x0, w, y = 144, 44, 208
    cells(f, x0, y, [str(p) for p in parent], w=w, h=32,
          fills={i: ("accent2-soft" if parent[i] == i else None) for i in range(8)})
    f.text(x0 - 12, y + 16, "parent", size=12, mono=True, anchor="e")
    return f


# ==========================================================================
# Chapter 10: in the wild
# ==========================================================================
@figure("lru")
def _lru():
    f = Fig(640, 210, "A hash table whose values are positions in a doubly linked list.")
    order = [("B", "2"), ("D", "4"), ("A", "1"), ("C", "3")]
    xs = [70, 210, 350, 490]
    y, w, h = 136, 96, 34
    node_x = {}
    for (k, v), x in zip(order, xs):
        f.rect(x, y, w, h, fill="accent-soft")
        f.line(x + 44, y, x + 44, y + h)
        f.text(x + 22, y + h / 2, k, mono=True)
        f.text(x + 70, y + h / 2, v, mono=True)
        node_x[k] = x + w / 2
    for a, b in zip(xs, xs[1:]):
        f.line(a + w, y + 11, b - 1, y + 11, arrow=True)
        f.line(b, y + 24, a + w + 1, y + 24, arrow=True, stroke="muted")
    f.text(xs[0] + w / 2, y + h + 16, "most recently used", size=11, color="good")
    f.text(xs[3] + w / 2, y + h + 16, "least recently used:", size=11, color="bad")
    f.text(xs[3] + w / 2, y + h + 30, "evicted next", size=11, color="bad")
    f.text(40, y + h / 2, "list", size=12, bold=True, anchor="e")
    hx, hy, c = 220, 24, 50
    f.text(hx - 14, hy + 16, "hash table", size=12, bold=True, anchor="e")
    for i, k in enumerate("ABCD"):
        f.rect(hx + i * c, hy, c, 32, fill="muted-soft")
        f.text(hx + i * c + 14, hy + 16, k, mono=True)
        f.circle(hx + i * c + 34, hy + 16, 2.6, fill="ink")
        f.line(hx + i * c + 34, hy + 16, node_x[k], y - 2, arrow=True, stroke="accent2", lw=1.1)
    f.text(hx + 4 * c + 12, hy + 16, "key  ->  node", size=11, color="muted", anchor="s",
           tex=r"key $\rightarrow$ node")
    return f


@figure("trie")
def _trie():
    f = Fig(640, 290, "A trie containing tree, trie, try, trial, heap, hash and he.")
    # (id, letter, x, y, is_word, parent)
    N = [("root", "", 330, 24, False, None),
         ("t", "t", 210, 70, False, "root"), ("tr", "r", 210, 116, False, "t"),
         ("tre", "e", 120, 162, False, "tr"), ("tree", "e", 120, 208, True, "tre"),
         ("tri", "i", 220, 162, False, "tr"), ("tria", "a", 180, 208, False, "tri"),
         ("trial", "l", 180, 254, True, "tria"), ("trie", "e", 260, 208, True, "tri"),
         ("try", "y", 310, 162, True, "tr"),
         ("h", "h", 470, 70, False, "root"), ("he", "e", 420, 116, True, "h"),
         ("hea", "a", 420, 162, False, "he"), ("heap", "p", 420, 208, True, "hea"),
         ("ha", "a", 530, 116, False, "h"), ("has", "s", 530, 162, False, "ha"),
         ("hash", "h", 530, 208, True, "has")]
    pos = {i: (x, y) for i, _, x, y, _, _ in N}
    for i, _, x, y, _, p in N:
        if p:
            edge_between(f, pos[p], (x, y), 13, 13)
    for i, ch, x, y, word, _ in N:
        if word:
            f.circle(x, y, 16.5, stroke="accent2", fill="accent2-soft")
        f.circle(x, y, 13, fill=("paper" if not word else "accent2-soft"),
                 stroke=("accent2" if word else "ink"))
        if ch:
            f.text(x, y, ch, mono=True, size=13)
    f.text(330, 6, "root", size=10, color="muted")
    for i, label in (("tree", "tree"), ("trial", "trial"), ("trie", "trie"), ("try", "try"),
                     ("he", "he"), ("heap", "heap"), ("hash", "hash")):
        x, y = pos[i]
        if i in ("try", "he"):
            f.text(x + 22, y, label, size=11, color="accent2", anchor="s", italic=True)
        else:
            f.text(x, y + 26, label, size=11, color="accent2", italic=True)
    return f


@figure("git")
def _git():
    f = Fig(640, 236, "Two commits whose trees share an unchanged blob.")

    def box(cx, cy, label, fill, w=128, h=30, stroke="ink"):
        f.rect(cx - w / 2, cy - h / 2, w, h, fill=fill, rx=5, stroke=stroke)
        f.text(cx, cy, label, size=12, mono=True)
        return cx, cy

    c1 = box(150, 30, "commit 1", "muted-soft")
    c2 = box(480, 30, "commit 2", "muted-soft")
    t1 = box(150, 100, "tree", "accent-soft")
    t2 = box(480, 100, "tree", "accent-soft")
    b1 = box(80, 190, "blob (old)", "paper", w=120)
    br = box(315, 190, "blob", "good-soft", w=110, stroke="good")
    b2 = box(560, 190, "blob (new)", "accent2-soft", w=120, stroke="accent2")
    f.line(c2[0] - 64, 30, c1[0] + 66, 30, arrow=True)
    f.text(315, 18, "parent", size=11, color="muted")
    f.line(c1[0], 45, t1[0], 84, arrow=True)
    f.line(c2[0], 45, t2[0], 84, arrow=True)
    f.line(t1[0] - 30, 115, b1[0], 174, arrow=True)
    f.line(t1[0] + 30, 115, br[0] - 20, 174, arrow=True)
    f.line(t2[0] - 30, 115, br[0] + 20, 174, arrow=True)
    f.line(t2[0] + 30, 115, b2[0], 174, arrow=True)
    f.text(84, 140, "main.cpp", size=11, mono=True, color="muted")
    f.text(262, 140, "README", size=11, mono=True, color="muted")
    f.text(376, 140, "README", size=11, mono=True, color="muted")
    f.text(552, 140, "main.cpp", size=11, mono=True, color="muted")
    f.text(315, 218, "unchanged content: one object, shared", size=11, color="good")
    return f


@figure("hnsw")
def _hnsw():
    f = Fig(640, 250, "Three layers of a proximity graph, with a search path descending through them.")
    xs = [24 + 43 * i for i in range(14)]
    jit = [0, 8, -6, 4, -8, 6, 0, -5, 7, -3, 5, -7, 3, 0]
    ys = {0: 196, 1: 116, 2: 40}
    layers = {0: list(range(14)), 1: [0, 3, 5, 8, 11, 13], 2: [0, 5, 11]}
    path = {2: [0, 5, 11], 1: [11, 8], 0: [8, 9]}
    for L in (2, 1, 0):
        y = ys[L]
        f.text(604, y - 4, f"layer {L}", size=11, color="muted", anchor="s")
        pts = layers[L]
        for a, b in zip(pts, pts[1:]):
            f.line(xs[a], y + jit[a] / (1 if L == 0 else 2), xs[b], y + jit[b] / (1 if L == 0 else 2), stroke="muted", lw=1)
        if L == 0:
            for a in range(0, 12):
                f.curve(xs[a], y + jit[a], (xs[a] + xs[a + 2]) / 2, y + 26, xs[a + 2], y + jit[a + 2], stroke="muted", lw=0.7)
    for i in layers[1]:
        f.line(xs[i], ys[1] + jit[i] / 2, xs[i], ys[0] + jit[i], stroke="muted", dashed=True, lw=0.8)
    for i in layers[2]:
        f.line(xs[i], ys[2] + jit[i] / 2, xs[i], ys[1] + jit[i] / 2, stroke="muted", dashed=True, lw=0.8)
    P = lambda L, i: (xs[i], ys[L] + (jit[i] if L == 0 else jit[i] / 2))
    for L, seq in path.items():
        for a, b in zip(seq, seq[1:]):
            edge_between(f, P(L, a), P(L, b), 6, 6, stroke="accent2", lw=2.4, arrow=True)
    for (La, a), (Lb, b) in (((2, 11), (1, 11)), ((1, 8), (0, 8))):
        edge_between(f, P(La, a), P(Lb, b), 6, 6, stroke="accent2", lw=2.4, arrow=True)
    for L in (2, 1, 0):
        for i in layers[L]:
            x, y = P(L, i)
            on = i in path[L]
            f.circle(x, y, 5.5, fill=("accent2" if on else "accent-soft"), stroke=("accent2" if on else "accent"))
    qx, qy = xs[9] + 12, ys[0] + 36
    f.text(qx, qy, "×", size=18, color="bad", bold=True, tex=r"$\boldsymbol{\times}$")
    f.text(qx + 12, qy + 2, "query", size=11, color="bad", anchor="s")
    f.text(xs[0] + 12, ys[2] - 16, "start", size=11, color="accent2", bold=True, anchor="s")
    return f
