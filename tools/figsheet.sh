#!/bin/sh
# Compile individual figures with the print fonts and tile them into one
# image, for visual review.  Usage: tools/figsheet.sh out.png name1 name2 ...
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$1"; shift
tmp="$(mktemp -d)"
imgs=""
for f in "$@"; do
  cat > "$tmp/$f.tex" <<TEX
\\documentclass[border=4pt]{standalone}
\\input{$root/latex/figpre}
\\begin{document}\\input{$root/build/figures/$f.tikz}\\end{document}
TEX
  (cd "$tmp" && pdflatex -interaction=batchmode "$f.tex" >/dev/null 2>&1 && pdftoppm -r 80 -png -singlefile "$f.pdf" "$f") || echo "FAILED: $f"
  imgs="$imgs $tmp/$f.png"
done
montage $imgs -tile 3x -geometry +8+8 -background white "$out"
rm -rf "$tmp"
echo "$out"
