#!/bin/sh
# Load each chapter page in headless Chrome with widgets driven to a demo
# state, and report any JavaScript error (the engine writes errors into the
# page title).  Usage: tools/vizcheck.sh [page.html ...]
root="$(cd "$(dirname "$0")/.." && pwd)"
[ $# -eq 0 ] && set -- $(cd "$root/dist/html" && ls [0-9a-z]*.html)
for page in "$@"; do
  title=$(google-chrome --headless=new --disable-gpu --no-sandbox --virtual-time-budget=5000 \
      --dump-dom "file://$root/dist/html/$page#viztest" 2>/dev/null | grep -o '<title>[^<]*</title>' | head -1)
  mounted=$(google-chrome --headless=new --disable-gpu --no-sandbox --virtual-time-budget=5000 \
      --dump-dom "file://$root/dist/html/$page#viztest" 2>/dev/null | grep -o 'data-mounted="1"' | wc -l)
  case "$title" in *ERROR*) echo "FAIL  $page  $title";; *) echo "ok    $page  ($mounted widgets)";; esac
done
