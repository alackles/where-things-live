"""A tiny drawing library with two backends.

Each figure is described once, with a handful of primitives (rect, circle,
line, curve, text).  `to_svg()` emits SVG whose colours are CSS classes, so
the web edition can restyle figures for light and dark themes.  `to_tikz()`
emits TikZ, so the print edition gets the book's own fonts and vector lines.

Coordinates are in "pixels" with y growing downward, as in SVG.  A figure
~640 px wide fills the text column in both editions.

Colours are semantic names: ink, muted, accent, accent2, good, bad, violet,
paper, plus "<name>-soft" for pale fills.
"""
import math
from xml.sax.saxutils import escape

TIKZ_UNIT = 0.0195  # cm per px


def _tex_escape(s):
    out = []
    for ch in s:
        if ch in "&%$#_{}":
            out.append("\\" + ch)
        elif ch == "\\":
            out.append(r"\textbackslash{}")
        elif ch == "~":
            out.append(r"\textasciitilde{}")
        elif ch == "^":
            out.append(r"\textasciicircum{}")
        elif ch == "<":
            out.append(r"\textless{}")
        elif ch == ">":
            out.append(r"\textgreater{}")
        else:
            out.append(ch)
    return "".join(out)


class Fig:
    def __init__(self, w, h, label=""):
        self.w, self.h, self.label = w, h, label
        self.items = []

    # ---- primitives -----------------------------------------------------
    def rect(self, x, y, w, h, stroke="ink", fill=None, rx=0, dashed=False, lw=1.3):
        self.items.append(("rect", x, y, w, h, stroke, fill, rx, dashed, lw))

    def circle(self, cx, cy, r, stroke="ink", fill=None, lw=1.3, dashed=False):
        self.items.append(("circle", cx, cy, r, stroke, fill, lw, dashed))

    def line(self, x1, y1, x2, y2, stroke="ink", arrow=False, dashed=False, lw=1.3):
        self.items.append(("line", x1, y1, x2, y2, stroke, arrow, dashed, lw))

    def curve(self, x1, y1, cx, cy, x2, y2, stroke="ink", arrow=False, dashed=False, lw=1.3):
        """Quadratic Bezier from (x1,y1) to (x2,y2) with control point (cx,cy)."""
        self.items.append(("curve", x1, y1, cx, cy, x2, y2, stroke, arrow, dashed, lw))

    def poly(self, pts, stroke="ink", fill=None, lw=1.3, dashed=False):
        self.items.append(("poly", list(pts), stroke, fill, lw, dashed))

    def text(self, x, y, s, color="ink", anchor="m", size=14, mono=False,
             bold=False, italic=False, tex=None):
        """anchor: 'm' middle, 's' start (left), 'e' end (right).
        `tex` overrides the LaTeX rendering (for math)."""
        self.items.append(("text", x, y, str(s), color, anchor, size, mono, bold, italic, tex))

    # ---- SVG ------------------------------------------------------------
    def to_svg(self):
        o = [f'<svg class="figsvg" viewBox="0 0 {self.w} {self.h}" width="{self.w}" '
             f'height="{self.h}" role="img" aria-label="{escape(self.label)}" '
             f'xmlns="http://www.w3.org/2000/svg">']

        def cls(stroke, fill, dashed=False):
            c = []
            c.append(f"s-{stroke}" if stroke else "s-none")
            c.append(f"f-{fill}" if fill else "f-none")
            if dashed:
                c.append("dash")
            return " ".join(c)

        def head(x1, y1, x2, y2, stroke):
            ang = math.atan2(y2 - y1, x2 - x1)
            L, W = 9, 3.6
            bx, by = x2 - L * math.cos(ang), y2 - L * math.sin(ang)
            p1 = (bx + W * math.sin(ang), by - W * math.cos(ang))
            p2 = (bx - W * math.sin(ang), by + W * math.cos(ang))
            return (f'<polygon class="s-none f-{stroke}" points="{x2:.1f},{y2:.1f} '
                    f'{p1[0]:.1f},{p1[1]:.1f} {p2[0]:.1f},{p2[1]:.1f}"/>')

        for it in self.items:
            k = it[0]
            if k == "rect":
                _, x, y, w, h, stroke, fill, rx, dashed, lw = it
                o.append(f'<rect class="{cls(stroke, fill, dashed)}" x="{x}" y="{y}" width="{w}" '
                         f'height="{h}" rx="{rx}" stroke-width="{lw}"/>')
            elif k == "circle":
                _, cx, cy, r, stroke, fill, lw, dashed = it
                o.append(f'<circle class="{cls(stroke, fill, dashed)}" cx="{cx}" cy="{cy}" r="{r}" '
                         f'stroke-width="{lw}"/>')
            elif k == "line":
                _, x1, y1, x2, y2, stroke, arrow, dashed, lw = it
                ex, ey = x2, y2
                if arrow:
                    ang = math.atan2(y2 - y1, x2 - x1)
                    ex, ey = x2 - 6 * math.cos(ang), y2 - 6 * math.sin(ang)
                o.append(f'<line class="{cls(stroke, None, dashed)}" x1="{x1:.1f}" y1="{y1:.1f}" '
                         f'x2="{ex:.1f}" y2="{ey:.1f}" stroke-width="{lw}"/>')
                if arrow:
                    o.append(head(x1, y1, x2, y2, stroke))
            elif k == "curve":
                _, x1, y1, cx, cy, x2, y2, stroke, arrow, dashed, lw = it
                o.append(f'<path class="{cls(stroke, None, dashed)}" d="M{x1:.1f},{y1:.1f} '
                         f'Q{cx:.1f},{cy:.1f} {x2:.1f},{y2:.1f}" stroke-width="{lw}"/>')
                if arrow:
                    o.append(head(cx, cy, x2, y2, stroke))
            elif k == "poly":
                _, pts, stroke, fill, lw, dashed = it
                p = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
                o.append(f'<polygon class="{cls(stroke, fill, dashed)}" points="{p}" stroke-width="{lw}"/>')
            elif k == "text":
                _, x, y, s, color, anchor, size, mono, bold, italic, tex = it
                a = {"m": "middle", "s": "start", "e": "end"}[anchor]
                c = f"t-{color}" + (" mono" if mono else "") + (" b" if bold else "") + (" i" if italic else "")
                o.append(f'<text class="{c}" x="{x:.1f}" y="{y:.1f}" text-anchor="{a}" '
                         f'dominant-baseline="central" font-size="{size}">{escape(s)}</text>')
        o.append("</svg>")
        return "\n".join(o)

    # ---- TikZ -----------------------------------------------------------
    def to_tikz(self):
        u = TIKZ_UNIT
        o = [f"\\begin{{tikzpicture}}[x={u}cm,y={u}cm,line cap=round,line join=round,"
             f">={{Stealth[length=5pt,width=4pt]}}]",
             f"\\path[use as bounding box] (0,0) rectangle ({self.w},{-self.h});"]

        def P(x, y):
            return f"({x:.1f},{-y:.1f})"

        def opts(stroke, fill, lw, dashed, arrow=False):
            a = []
            if stroke:
                a.append(f"draw=fig-{stroke}")
            if fill:
                a.append(f"fill=fig-{fill}")
            a.append(f"line width={lw * 0.55:.2f}pt")
            if dashed:
                a.append("dash pattern=on 2.5pt off 2pt")
            if arrow:
                a.append("->")
            return ",".join(a)

        for it in self.items:
            k = it[0]
            if k == "rect":
                _, x, y, w, h, stroke, fill, rx, dashed, lw = it
                r = f",rounded corners={rx * u:.3f}cm" if rx else ""
                o.append(f"\\path[{opts(stroke, fill, lw, dashed)}{r}] {P(x, y)} rectangle {P(x + w, y + h)};")
            elif k == "circle":
                _, cx, cy, r, stroke, fill, lw, dashed = it
                o.append(f"\\path[{opts(stroke, fill, lw, dashed)}] {P(cx, cy)} circle [radius={r * u:.3f}cm];")
            elif k == "line":
                _, x1, y1, x2, y2, stroke, arrow, dashed, lw = it
                o.append(f"\\path[{opts(stroke, None, lw, dashed, arrow)}] {P(x1, y1)} -- {P(x2, y2)};")
            elif k == "curve":
                _, x1, y1, cx, cy, x2, y2, stroke, arrow, dashed, lw = it
                # quadratic -> cubic control points
                c1 = (x1 + 2 / 3 * (cx - x1), y1 + 2 / 3 * (cy - y1))
                c2 = (x2 + 2 / 3 * (cx - x2), y2 + 2 / 3 * (cy - y2))
                o.append(f"\\path[{opts(stroke, None, lw, dashed, arrow)}] {P(x1, y1)} .. controls "
                         f"{P(*c1)} and {P(*c2)} .. {P(x2, y2)};")
            elif k == "poly":
                _, pts, stroke, fill, lw, dashed = it
                path = " -- ".join(P(x, y) for x, y in pts)
                o.append(f"\\path[{opts(stroke, fill, lw, dashed)}] {path} -- cycle;")
            elif k == "text":
                _, x, y, s, color, anchor, size, mono, bold, italic, tex = it
                a = {"m": "", "s": ",anchor=west", "e": ",anchor=east"}[anchor]
                fs = ("\\tiny" if size <= 9 else "\\scriptsize" if size <= 11 else
                      "\\footnotesize" if size <= 13 else "\\small" if size <= 15 else
                      "\\normalsize" if size <= 18 else "\\large")
                font = fs + ("\\ttfamily" if mono else "\\sffamily")
                if bold:
                    font += "\\bfseries"
                if italic:
                    font += "\\itshape"
                body = tex if tex is not None else _tex_escape(s)
                o.append(f"\\node[inner sep=0pt,text=fig-{color},font={font}{a}] at {P(x, y)} {{{body}}};")
        o.append("\\end{tikzpicture}")
        return "\n".join(o)


# --------------------------------------------------------------------------
# Higher-level helpers
# --------------------------------------------------------------------------
def cells(fig, x, y, values, w=44, h=36, fills=None, index=True, index_from=0,
          mono=True, size=14, stroke="ink", idx_color="muted", texts=None):
    """Draw a row of array cells.  Returns list of (cx, cy) centres.
    fills: {i: colour}; texts: {i: colour} for text colour overrides."""
    fills = fills or {}
    texts = texts or {}
    centres = []
    for i, v in enumerate(values):
        cx = x + i * w
        fig.rect(cx, y, w, h, stroke=stroke, fill=fills.get(i))
        if v is not None and v != "":
            fig.text(cx + w / 2, y + h / 2, v, mono=mono, size=size, color=texts.get(i, "ink"))
        if index:
            fig.text(cx + w / 2, y + h + 11, i + index_from, size=11, color=idx_color, mono=True)
        centres.append((cx + w / 2, y + h / 2))
    return centres


def edge_between(fig, c1, c2, r1, r2, **kw):
    """Line between two circles' boundaries (centres c1, c2; radii r1, r2)."""
    (x1, y1), (x2, y2) = c1, c2
    d = math.hypot(x2 - x1, y2 - y1) or 1
    ux, uy = (x2 - x1) / d, (y2 - y1) / d
    fig.line(x1 + ux * r1, y1 + uy * r1, x2 - ux * r2, y2 - uy * r2, **kw)


def tree_positions(spec, x0, y0, dx, dy):
    """spec: nested tuples (label, left, right) | label | None.
    Lays nodes out by in-order rank.  Returns ({path: (label, x, y)}, edges)
    where path is a string like '' (root), 'L', 'LR'."""
    nodes, edges, counter = {}, [], [0]

    def norm(s):
        if s is None:
            return None
        if isinstance(s, tuple):
            lab = s[0]
            left = s[1] if len(s) > 1 else None
            right = s[2] if len(s) > 2 else None
            return lab, left, right
        return s, None, None

    def walk(s, depth, path):
        s = norm(s)
        if s is None:
            return
        lab, left, right = s
        walk(left, depth + 1, path + "L")
        x = x0 + counter[0] * dx
        counter[0] += 1
        nodes[path] = (lab, x, y0 + depth * dy)
        walk(right, depth + 1, path + "R")
        if norm(left) is not None:
            edges.append((path, path + "L"))
        if norm(right) is not None:
            edges.append((path, path + "R"))

    walk(spec, 0, "")
    return nodes, edges


def draw_tree(fig, spec, x0, y0, dx=42, dy=58, r=17, fills=None, strokes=None,
              size=13, subs=None, edge_stroke="ink"):
    """Draw a binary tree.  fills/strokes/subs are keyed by label."""
    fills, strokes, subs = fills or {}, strokes or {}, subs or {}
    nodes, edges = tree_positions(spec, x0, y0, dx, dy)
    for a, b in edges:
        (_, x1, y1), (_, x2, y2) = nodes[a], nodes[b]
        edge_between(fig, (x1, y1), (x2, y2), r, r, stroke=edge_stroke)
    for path, (lab, x, y) in nodes.items():
        fig.circle(x, y, r, stroke=strokes.get(lab, "ink"), fill=fills.get(lab, "paper"))
        fig.text(x, y, lab, size=size, mono=True)
        if lab in subs:
            fig.text(x + r + 4, y - r + 2, subs[lab], size=10, color="accent2", anchor="s", mono=True)
    return nodes


def heap_positions(n, x_mid, y0, width, dy):
    """Positions for a complete binary tree of n nodes (index 0 = root)."""
    pos = []
    for i in range(n):
        depth = int(math.log2(i + 1))
        first = 2 ** depth - 1
        k = i - first
        slots = 2 ** depth
        x = x_mid - width / 2 + (k + 0.5) * width / slots
        pos.append((x, y0 + depth * dy))
    return pos


def list_node(fig, x, y, val, w=64, h=34, fill=None, stroke="ink", null=False):
    """A singly-linked node: [ value | next ].  Returns (out_x, out_y) for
    the pointer tail and (in_x, in_y) for incoming arrows."""
    split = w * 0.62
    fig.rect(x, y, w, h, stroke=stroke, fill=fill)
    fig.line(x + split, y, x + split, y + h, stroke=stroke)
    fig.text(x + split / 2, y + h / 2, val, mono=True)
    if null:
        fig.line(x + split + 3, y + h - 3, x + w - 3, y + 3, stroke="muted")
    else:
        fig.circle(x + (split + w) / 2, y + h / 2, 2.6, stroke="ink", fill="ink")
    return (x + (split + w) / 2, y + h / 2), (x, y + h / 2)


def brace_label(fig, x1, x2, y, text, color="muted", below=True, size=12, tex=None):
    """A flat bracket under (or over) a horizontal span, with a label."""
    d = 6 if below else -6
    fig.line(x1, y, x1, y + d, stroke=color)
    fig.line(x1, y + d, x2, y + d, stroke=color)
    fig.line(x2, y + d, x2, y, stroke=color)
    fig.text((x1 + x2) / 2, y + d + (11 if below else -11), text, color=color, size=size, tex=tex)
