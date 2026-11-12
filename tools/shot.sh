#!/bin/sh
# Screenshot a built page in headless Chrome, with widgets driven to a
# representative state (#viztest).  Usage: tools/shot.sh page.html out.png [w] [h] [dark]
page="$1"; out="$2"; w="${3:-1280}"; h="${4:-2400}"
root="$(cd "$(dirname "$0")/.." && pwd)"
extra=""
[ "$5" = "dark" ] && extra="--force-dark-mode --blink-settings=preferredColorScheme=0"
google-chrome --headless=new --disable-gpu --no-sandbox --hide-scrollbars $extra \
  --virtual-time-budget=4000 --window-size="$w,$h" --screenshot="$out" \
  "file://$root/dist/html/$page#viztest" >/dev/null 2>&1
echo "$out"
