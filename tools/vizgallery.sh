#!/bin/sh
# Build a page showing every interactive widget in its demo state and
# screenshot it, for visual review.
# Usage: tools/vizgallery.sh out.png [dark] [name ...]   (default: all widgets)
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$1"; shift
dark=""; [ "$1" = "dark" ] && { dark="--force-dark-mode --blink-settings=preferredColorScheme=0"; shift; }
[ $# -eq 0 ] && set -- $(cd "$root/web/viz" && ls *.js | sed 's/\.js$//' | grep -v '^core$')
page="$root/dist/html/_gallery.html"
{
  echo '<!doctype html><html><head><meta charset="utf-8"><link rel="stylesheet" href="assets/style.css">'
  echo '<style>body{padding:16px;max-width:720px;margin:0 auto} h3{margin:18px 0 6px;font-size:14px}</style></head><body>'
  for n in "$@"; do
    echo "<h3>$n</h3><figure class=\"fig viz\" style=\"margin:0\"><div class=\"viz-widget\" data-viz=\"$n\"></div></figure>"
    [ "$n" = "bst" ] && echo "<h3>bst (avl)</h3><figure class=\"fig viz\" style=\"margin:0\"><div class=\"viz-widget\" data-viz=\"bst\" data-mode=\"avl\"></div></figure>"
  done
  echo '<script src="assets/viz/core.js"></script>'
  for n in "$@"; do echo "<script src=\"assets/viz/$n.js\"></script>"; done
  echo '<script>Viz.mountAll();</script></body></html>'
} > "$page"
cp "$root"/web/viz/*.js "$root/dist/html/assets/viz/" 2>/dev/null
cp "$root/web/style.css" "$root/dist/html/assets/"
h=$(( 430 * $# + 200 ))
google-chrome --headless=new --disable-gpu --no-sandbox --hide-scrollbars $dark --virtual-time-budget=5000 \
  --window-size="760,$h" --screenshot="$out" "file://$page#viztest" >/dev/null 2>&1
title=$(google-chrome --headless=new --disable-gpu --no-sandbox --virtual-time-budget=5000 --dump-dom "file://$page#viztest" 2>/dev/null | grep -o 'ERROR[^<]*' | head -3)
[ -n "$title" ] && echo "JS: $title"
convert "$out" -trim +repage "$out" 2>/dev/null
echo "$out"
