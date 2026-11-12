#!/usr/bin/env python3
"""Build both editions of the book from one Markdown source.

    python3 tools/build.py html     -> dist/html/
    python3 tools/build.py pdf      -> dist/<pdf_name>
    python3 tools/build.py all      -> both
    python3 tools/build.py check    -> report unresolved refs / long code lines only

Pipeline
  1. scan     read every chapter, assign chapter/section/figure numbers
  2. figures  figures/figs.py emits each figure as themed SVG and as TikZ
  3. expand   per target: resolve [[refs]], pull code listings out of code/,
              number headings (HTML only; LaTeX numbers its own)
  4. pandoc   Markdown -> HTML fragment / LaTeX fragment (tools/book.lua
              turns the ::: blocks into callouts, figures, widgets)
  5. wrap     HTML: page template + nav.  LaTeX: book.tex + pdflatex/bibtex
"""
import html
import re
import shutil
import subprocess
import sys
import textwrap
from pathlib import Path

import yaml

problems = []
ROOT = Path(__file__).resolve().parent.parent
CH = ROOT / "chapters"
CODE = ROOT / "code"
BUILD = ROOT / "build"
DIST = ROOT / "dist"
WEB = ROOT / "web"
BIB = ROOT / "references.bib"
LUA = ROOT / "tools" / "book.lua"

MAX_CODE_COLS = 78

sys.path.insert(0, str(ROOT / "figures"))


# --------------------------------------------------------------------------
# 1. scan
# --------------------------------------------------------------------------
class Part:
    def __init__(self, spec):
        self.file = spec["file"]
        self.kind = spec["kind"]
        self.blurb = spec.get("blurb", "")
        self.week = spec.get("week")
        self.stem = Path(self.file).stem
        self.html = self.stem + ".html"
        self.text = (CH / self.file).read_text(encoding="utf-8")
        self.num = None  # "3" or "A" or None
        self.title = ""
        self.id = ""
        self.sections = []  # (num, title, id)
        self.vizzes = []

    @property
    def label(self):
        if self.kind == "chapter":
            return f"Chapter {self.num}"
        if self.kind == "appendix":
            return f"Appendix {self.num}"
        return ""


H1 = re.compile(r"^# (.+?)\s*\{#([\w:-]+)([^}]*)\}\s*$")
H2 = re.compile(r"^## (.+?)\s*\{#([\w:-]+)([^}]*)\}\s*$")
FIGDIV = re.compile(r"^(:::+)\s*\{\.(figure|viz)\b([^}]*)\}\s*$")
FENCE = re.compile(r"^(```+|~~~+)")
REF = re.compile(r"\[\[((?:ch|app|sec|fig):[\w-]+)\]\]")


def load_book():
    meta = yaml.safe_load((ROOT / "book.yaml").read_text())
    parts = [Part(p) for p in meta["parts"] if (CH / p["file"]).exists()]
    labels = {}
    seen_ids = {}
    nch, napp = 0, 0
    for p in parts:
        if p.kind == "chapter":
            nch += 1
            p.num = str(nch)
        elif p.kind == "appendix":
            p.num = chr(ord("A") + napp)
            napp += 1
        nsec = nfig = 0
        infence = False
        for line in p.text.splitlines():
            if FENCE.match(line):
                infence = not infence
                continue
            if infence:
                continue
            m = H1.match(line)
            if m and not p.title:
                p.title, p.id = m.group(1), m.group(2)
                text = p.label or f"“{p.title}”"
                labels[p.id] = dict(text=text, part=p, anchor="")
                continue
            m = H2.match(line)
            if m:
                title, sid, rest = m.groups()
                if ".unnumbered" in rest or p.num is None:
                    p.sections.append((None, title, sid))
                    labels[sid] = dict(text=f"“{title}”", part=p, anchor=sid)
                else:
                    nsec += 1
                    n = f"{p.num}.{nsec}"
                    p.sections.append((n, title, sid))
                    labels[sid] = dict(text=f"Section {n}", part=p, anchor=sid)
                continue
            m = FIGDIV.match(line)
            if m:
                attrs = m.group(3)
                idm = re.search(r"#(fig:[\w-]+)", attrs)
                if not idm:
                    sys.exit(f"{p.file}: figure/viz without #fig: id: {line}")
                nfig += 1
                n = f"{p.num}.{nfig}" if p.num else str(nfig)
                labels[idm.group(1)] = dict(
                    text=f"Figure {n}", part=p, anchor=idm.group(1), num=n
                )
                if m.group(2) == "viz":
                    nm = re.search(r'name="([\w-]+)"', attrs)
                    if nm and nm.group(1) not in p.vizzes:
                        p.vizzes.append(nm.group(1))
        if not p.title:
            sys.exit(f"{p.file}: no '# Title {{#id}}' heading found")
        for hid in re.findall(r"^#{1,4} .*\{#([\w:-]+)", p.text, flags=re.M):
            if hid in seen_ids:
                problems.append(f"{p.file}: heading id '{hid}' is also used in {seen_ids[hid]}")
            seen_ids[hid] = p.file
    return meta, parts, labels


# --------------------------------------------------------------------------
# 3. expand
# --------------------------------------------------------------------------
MARK = re.compile(r"^\s*//\s*\[(/?)([\w-]+)\]\s*$")


def read_listing(path, region):
    src = (CODE / path).read_text(encoding="utf-8").splitlines()
    out, on, found = [], region is None, False
    for line in src:
        m = MARK.match(line)
        if m:
            if region and m.group(2) == region:
                on = not m.group(1)
                found = True
            continue
        if on:
            out.append(line)
    if region and not found:
        sys.exit(f"listing region [{region}] not found in code/{path}")
    body = textwrap.dedent("\n".join(out)).strip("\n")
    for i, line in enumerate(body.splitlines(), 1):
        if len(line) > MAX_CODE_COLS:
            print(f"  warning: code/{path} [{region}] line {i} is {len(line)} cols")
    return body


def expand(p, labels, target):
    out = []
    lines = p.text.splitlines()
    i = 0
    infence = False
    while i < len(lines):
        line = lines[i]
        fm = FENCE.match(line)
        if fm and not infence:
            inc = re.search(r'include="([^"]+)"', line)
            if inc:
                reg = re.search(r'region="([^"]+)"', line)
                lang = re.search(r"\{\.([\w+-]+)", line)
                body = read_listing(inc.group(1), reg.group(1) if reg else None)
                # skip to closing fence
                i += 1
                while i < len(lines) and not FENCE.match(lines[i]):
                    i += 1
                i += 1
                out.append(f'::: {{.listing file="{inc.group(1)}"}}')
                out.append(f"```{lang.group(1) if lang else 'cpp'}")
                out.append(body)
                out.append("```")
                out.append(":::")
                continue
            infence = True
            out.append(line)
            i += 1
            continue
        if fm and infence:
            infence = False
            out.append(line)
            i += 1
            continue
        if infence:
            if len(line) > MAX_CODE_COLS:
                print(f"  warning: {p.file}:{i+1} code line is {len(line)} cols")
            out.append(line)
            i += 1
            continue

        m = H1.match(line)
        if m and m.group(2) == p.id:
            title, cid, rest = m.groups()
            if target == "html":
                kicker = p.label
                if p.week:
                    kicker += f" · Week {p.week}"
                k = f'<p class="kicker">{kicker}</p>' if kicker else ""
                out.append(
                    f'<header class="chapter-head" id="{cid}">{k}'
                    f"<h1>{html.escape(title)}</h1></header>"
                )
            else:
                if p.num is None:
                    out.append(f"# {title} {{#{cid} .unnumbered}}")
                    out.append("")
                    out.append(f"`\\markboth{{{title}}}{{{title}}}`{{=latex}}")
                else:
                    out.append(f"# {title} {{#{cid}}}")
                    if p.week:
                        out.append("")
                        out.append(f"`\\weekline{{{p.week}}}`{{=latex}}")
            i += 1
            continue

        m = H2.match(line)
        if m and target == "html":
            title, sid, rest = m.groups()
            num = next((n for n, t, s in p.sections if s == sid), None)
            if num:
                line = f"## [{num}]{{.secnum}} {title} {{#{sid}{rest}}}"

        m = FIGDIV.match(line)
        if m:
            colons, kind, attrs = m.groups()
            fid = re.search(r"#(fig:[\w-]+)", attrs).group(1)
            line = f'{colons} {{.{kind}{attrs} num="{labels[fid]["num"]}"}}'

        def sub(mm):
            lab = mm.group(1)
            if lab not in labels:
                problems.append(f"{p.file}:{i+1}: unresolved reference [[{lab}]]")
                return f"**??{lab}??**"
            L = labels[lab]
            if target == "html":
                href = "" if L["part"] is p else L["part"].html
                href += f"#{L['anchor']}" if L["anchor"] else ("#" if not href else "")
                return f"[{L['text']}]({href})"
            kind = lab.split(":")[0]
            if kind in ("ch", "app"):
                if L["part"].num is None:
                    return L["text"]
                word = "Chapter" if kind == "ch" else "Appendix"
                return f"`{word}~\\ref{{{lab}}}`{{=latex}}"
            if kind == "sec":
                if L["text"].startswith("Section"):
                    return f"`Section~\\ref{{{lab}}}`{{=latex}}"
                return L["text"]
            return f"`Figure~\\ref{{{lab}}}`{{=latex}}"

        out.append(REF.sub(sub, line))
        i += 1
    return "\n".join(out) + "\n"


# --------------------------------------------------------------------------
# pandoc helpers
# --------------------------------------------------------------------------
def pandoc(src_text, args, cwd=ROOT):
    r = subprocess.run(
        ["pandoc", "-f", "markdown+smart-implicit_figures", *args],
        input=src_text, capture_output=True, text=True, cwd=cwd,
    )
    if r.stderr.strip():
        for ln in r.stderr.strip().splitlines():
            print("  pandoc:", ln)
    if r.returncode != 0:
        sys.exit("pandoc failed")
    return r.stdout


def fix_math(body):
    """texmath marks every ( ) pair as a stretchy fence, which browsers render
    tall and widely spaced.  Plain parentheses read better for O(n)-style math."""
    return re.sub(r'<mo stretchy="true" form="(prefix|postfix)">',
                  r'<mo stretchy="false" form="\1" lspace="0" rspace="0">', body)


def build_figures():
    import figs  # figures/figs.py
    (BUILD / "figures").mkdir(parents=True, exist_ok=True)
    names = figs.build_all(BUILD / "figures")
    print(f"figures: {len(names)} built")
    return set(names)


# --------------------------------------------------------------------------
# HTML edition
# --------------------------------------------------------------------------
PAGE = """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{pagetitle}</title>
<script>
(function(){{try{{var t=localStorage.getItem('wtl-theme');if(t)document.documentElement.setAttribute('data-theme',t);}}catch(e){{}}}})();
</script>
<link rel="stylesheet" href="assets/style.css">
</head>
<body class="{bodyclass}">
<a class="skip" href="#main">Skip to content</a>
<header class="topbar">
  <button id="nav-toggle" aria-label="Toggle table of contents" aria-expanded="false">☰</button>
  <a class="brand" href="index.html">{title}</a>
  <span class="here">{here}</span>
  <button id="theme-toggle" aria-label="Switch between light and dark theme" title="Light / dark">◐</button>
</header>
<nav class="sidebar" id="sidebar" aria-label="Table of contents">
{nav}
</nav>
<main id="main">
<article>
{body}
</article>
<footer class="pagenav">
{prevnext}
</footer>
<footer class="fine">
<p><em>{title}: {subtitle}</em> · {edition} · <a href="z1-colophon.html">How this book was made</a></p>
</footer>
</main>
<script src="assets/book.js"></script>
{scripts}
</body>
</html>
"""


def nav_html(parts, current):
    out = ['<ol class="toc">']
    out.append(
        f'<li class="{"current" if current is None else ""}"><a href="index.html">Contents</a></li>'
    )
    for p in parts:
        cur = p is current
        num = f'<span class="n">{p.num}</span> ' if p.num else '<span class="n"></span> '
        out.append(f'<li class="{"current" if cur else ""}"><a href="{p.html}">{num}{html.escape(p.title)}</a>')
        if cur and p.sections:
            out.append('<ol class="secs">')
            for n, t, sid in p.sections:
                nn = f'<span class="n">{n}</span> ' if n else ""
                out.append(f'<li><a href="#{sid}">{nn}{html.escape(strip_md(t))}</a></li>')
            out.append("</ol>")
        out.append("</li>")
    out.append(
        f'<li class="{"current" if current == "bib" else ""}"><a href="bibliography.html"><span class="n"></span> Bibliography</a></li>'
    )
    out.append("</ol>")
    return "\n".join(out)


def strip_md(s):
    return re.sub(r"[`*_]", "", s)


def build_html(meta, parts, labels):
    out = DIST / "html"
    if out.exists():
        shutil.rmtree(out)
    (out / "assets").mkdir(parents=True)
    shutil.copy(WEB / "style.css", out / "assets")
    shutil.copy(WEB / "book.js", out / "assets")
    shutil.copytree(WEB / "viz", out / "assets" / "viz")
    shutil.copytree(WEB / "fonts", out / "assets" / "fonts")
    tmp = BUILD / "html"
    tmp.mkdir(parents=True, exist_ok=True)

    common = [
        "-t", "html5", "--mathml", "--citeproc", f"--bibliography={BIB}",
        "-M", "link-citations=true", "-M", "reference-section-title=References",
        "--lua-filter", str(LUA), "--no-highlight" if False else "--highlight-style=pygments",
        "--section-divs", "--wrap=none",
        "-M", f"figdir={BUILD / 'figures'}",
    ]
    pages = parts
    for idx, p in enumerate(pages):
        md = expand(p, labels, "html")
        (tmp / (p.stem + ".md")).write_text(md)
        body = fix_math(pandoc(md, common + ["-M", f"chapnum={p.num or ''}"]))
        prev = pages[idx - 1] if idx > 0 else None
        nxt = pages[idx + 1] if idx + 1 < len(pages) else None
        pn = []
        if prev:
            pn.append(f'<a class="prev" href="{prev.html}"><span>← Previous</span>{html.escape(prev.title)}</a>')
        else:
            pn.append('<a class="prev" href="index.html"><span>← Back</span>Contents</a>')
        if nxt:
            pn.append(f'<a class="next" href="{nxt.html}"><span>Next →</span>{html.escape(nxt.title)}</a>')
        else:
            pn.append('<a class="next" href="bibliography.html"><span>Next →</span>Bibliography</a>')
        scripts = ""
        if p.vizzes:
            scripts = '<script src="assets/viz/core.js"></script>\n' + "\n".join(
                f'<script src="assets/viz/{v}.js"></script>' for v in p.vizzes
            ) + "\n<script>Viz.mountAll();</script>"
            for v in p.vizzes:
                if not (WEB / "viz" / f"{v}.js").exists():
                    problems.append(f"{p.file}: widget '{v}' has no web/viz/{v}.js")
        page = PAGE.format(
            pagetitle=f"{p.title} · {meta['title']}",
            bodyclass=f"kind-{p.kind}",
            title=meta["title"], subtitle=meta["subtitle"], edition=meta["edition"],
            here=html.escape((p.label + ": " if p.label else "") + p.title),
            nav=nav_html(parts, p), body=body, prevnext="\n".join(pn), scripts=scripts,
        )
        (out / p.html).write_text(page, encoding="utf-8")
        print(f"html: {p.html}")

    # bibliography page
    bibmd = "---\nnocite: '@*'\n---\n" + \
        '<header class="chapter-head"><h1>Bibliography</h1></header>\n\n' + \
        "Every work cited in the book, in one place. Each chapter also ends with its own list.\n\n" + \
        "::: {#refs}\n:::\n"
    body = pandoc(bibmd, common)
    page = PAGE.format(
        pagetitle=f"Bibliography · {meta['title']}", bodyclass="kind-back",
        title=meta["title"], subtitle=meta["subtitle"], edition=meta["edition"],
        here="Bibliography", nav=nav_html(parts, "bib"), body=body,
        prevnext=f'<a class="prev" href="{parts[-1].html}"><span>← Previous</span>{html.escape(parts[-1].title)}</a>',
        scripts="",
    )
    (out / "bibliography.html").write_text(page, encoding="utf-8")

    # index / cover
    rows = []
    for p in parts:
        num = p.label or ""
        wk = f'<span class="wk">Week {p.week}</span>' if p.week else ""
        rows.append(
            f'<li><a href="{p.html}"><span class="k">{num}{wk}</span>'
            f'<span class="t">{html.escape(p.title)}</span>'
            f'<span class="b">{html.escape(p.blurb)}</span></a></li>'
        )
    rows.append(
        '<li><a href="bibliography.html"><span class="k"></span><span class="t">Bibliography</span>'
        '<span class="b">Every source cited, in one list.</span></a></li>'
    )
    cover = f"""
<header class="cover">
  <p class="kicker">{meta['tagline']}</p>
  <h1>{meta['title']}</h1>
  <p class="sub">{meta['subtitle']}</p>
  <p class="ed">{meta['edition']}</p>
  <div class="fourq-strip" aria-label="The four questions this book asks of every data structure">
    <div><b>Where does it live?</b><span>layout and ownership in memory</span></div>
    <div><b>What does it promise?</b><span>interface and invariants</span></div>
    <div><b>What does it cost?</b><span>on paper and on real hardware</span></div>
    <div><b>How do you know?</b><span>tests, tools, and measurement</span></div>
  </div>
  <p class="links"><a class="btn" href="{parts[0].html}">Start reading</a>
  <a class="btn ghost" href="{meta['pdf_name']}">PDF edition</a></p>
</header>
<ol class="cover-toc">
{chr(10).join(rows)}
</ol>
"""
    page = PAGE.format(
        pagetitle=f"{meta['title']}: {meta['subtitle']}", bodyclass="kind-cover",
        title=meta["title"], subtitle=meta["subtitle"], edition=meta["edition"],
        here=meta["subtitle"], nav=nav_html(parts, None), body=cover,
        prevnext=f'<a class="next" href="{parts[0].html}"><span>Next →</span>{html.escape(parts[0].title)}</a>',
        scripts="",
    )
    (out / "index.html").write_text(page, encoding="utf-8")
    pdf = DIST / meta["pdf_name"]
    if pdf.exists():
        shutil.copy(pdf, out / meta["pdf_name"])
    print(f"html: done -> {out}/index.html")


# --------------------------------------------------------------------------
# PDF edition
# --------------------------------------------------------------------------
BIBTEX = ("\\cleardoublepage\\phantomsection\\addcontentsline{toc}{chapter}{Bibliography}\n"
          "\\markboth{Bibliography}{Bibliography}\n"
          "\\bibliographystyle{plainnat}\n\\bibliography{references}")


def build_pdf(meta, parts, labels):
    tex = BUILD / "latex"
    (tex / "chapters").mkdir(parents=True, exist_ok=True)
    common = [
        "-t", "latex", "--top-level-division=chapter", "--natbib",
        f"--bibliography={BIB}", "--lua-filter", str(LUA),
        "--highlight-style", str(ROOT / "latex" / "print.theme"), "--wrap=preserve",
        "-M", f"figdir={BUILD / 'figures'}",
    ]
    # highlighting macros (pandoc only emits them in standalone mode)
    hl_tpl = tex / "hl.tpl"
    hl_tpl.write_text("$highlighting-macros$\n")
    hl = pandoc("```cpp\nint x;\n```\n", common + ["--template", str(hl_tpl)])
    (tex / "highlighting.tex").write_text(hl)

    body = []
    state = "front"
    for p in parts:
        md = expand(p, labels, "latex")
        frag = pandoc(md, common + ["-M", f"chapnum={p.num or ''}"])
        (tex / "chapters" / (p.stem + ".tex")).write_text(frag)
        if p.kind == "chapter" and state == "front":
            body.append("\\mainmatter\\setcounter{secnumdepth}{1}")
            state = "main"
        if p.kind == "appendix" and state != "app":
            body.append("\\appendix")
            state = "app"
        if p.kind == "back" and state != "back":
            body.append("\\backmatter\\setcounter{secnumdepth}{-1}")
            body.append(BIBTEX)
            state = "back"
        body.append(f"\\input{{chapters/{p.stem}}}")
        print(f"latex: {p.stem}.tex")

    if state != "back":
        body.append("\\backmatter")
        body.append(BIBTEX)
    main = (ROOT / "latex" / "book.tex").read_text()
    main = (main.replace("@@TITLE@@", meta["title"])
                .replace("@@SUBTITLE@@", meta["subtitle"])
                .replace("@@TAGLINE@@", meta["tagline"])
                .replace("@@EDITION@@", meta["edition"])
                .replace("@@BODY@@", "\n".join(body)))
    (tex / "book.tex").write_text(main)
    shutil.copy(ROOT / "latex" / "preamble.tex", tex / "preamble.tex")
    shutil.copy(BIB, tex / "references.bib")

    def run(cmd):
        r = subprocess.run(cmd, cwd=tex, capture_output=True, text=True, errors="replace")
        return r

    for n in (1, 2, 3):
        r = run(["pdflatex", "-interaction=nonstopmode", "-halt-on-error", "book.tex"])
        if r.returncode != 0:
            tail = "\n".join(r.stdout.splitlines()[-40:])
            sys.exit(f"pdflatex failed (pass {n}):\n{tail}")
        if n == 1:
            b = run(["bibtex", "book"])
            for ln in b.stdout.splitlines():
                if "Warning" in ln or "Error" in ln or "error" in ln:
                    print("  bibtex:", ln)
    log = (tex / "book.log").read_text(errors="replace")
    over = re.findall(r"Overfull \\hbox \(([\d.]+)pt too wide\)[^\n]*lines (\d+)--(\d+)", log)
    big = [o for o in over if float(o[0]) > 3]
    undef = re.findall(r"LaTeX Warning: (?:Reference|Citation) `([^']+)'[^\n]*undefined", log)
    pages = re.search(r"Output written on book.pdf \((\d+) pages", log)
    print(f"pdf: {pages.group(1) if pages else '?'} pages; "
          f"{len(big)} overfull hboxes >3pt; {len(set(undef))} undefined refs/cites")
    for u in sorted(set(undef)):
        print("  undefined:", u)
    DIST.mkdir(exist_ok=True)
    shutil.copy(tex / "book.pdf", DIST / meta["pdf_name"])
    print(f"pdf: done -> {DIST / meta['pdf_name']}")


def main():
    what = sys.argv[1] if len(sys.argv) > 1 else "all"
    meta, parts, labels = load_book()
    only = sys.argv[2:]
    if what == "check":
        for p in parts:
            expand(p, labels, "html")
    else:
        build_figures()
        if what in ("pdf", "all"):
            build_pdf(meta, parts, labels)
        if what in ("html", "all"):
            build_html(meta, parts, labels)
    if problems:
        print("\nPROBLEMS:")
        for pr in sorted(set(problems)):
            print("  " + pr)
        sys.exit(1)


if __name__ == "__main__":
    main()
