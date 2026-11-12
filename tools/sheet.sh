#!/bin/sh
# Contact sheet of PDF pages for visual review.
# Usage: tools/sheet.sh FIRST LAST out.png [dpi] [columns]
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
pdftoppm -r "${4:-75}" -png -f "$1" -l "$2" "$root/dist/where-things-live.pdf" "$tmp/p"
montage "$tmp"/p-*.png -tile "${5:-3}x" -geometry +6+6 -background '#888' "$3"
rm -rf "$tmp"
echo "$3"
