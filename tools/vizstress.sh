#!/bin/sh
# Mount every widget, then press every button and try every dropdown option
# several times, and report any JavaScript error.  A crude but effective
# smoke test for the paths the demo states do not reach.
root="$(cd "$(dirname "$0")/.." && pwd)"
page="$root/dist/html/_stress.html"
names=$(cd "$root/web/viz" && ls *.js | sed 's/\.js$//' | grep -v '^core$')
{
  echo '<!doctype html><html><head><meta charset="utf-8"><title>stress</title><link rel="stylesheet" href="assets/style.css"></head><body>'
  for n in $names; do echo "<div class=\"viz-widget\" data-viz=\"$n\"></div>"; done
  echo '<div class="viz-widget" data-viz="bst" data-mode="avl"></div>'
  echo '<script src="assets/viz/core.js"></script>'
  for n in $names; do echo "<script src=\"assets/viz/$n.js\"></script>"; done
  cat <<'JS'
<script>
var errors = [];
window.addEventListener('error', function (e) { errors.push(e.message + ' @' + (e.filename || '').split('/').pop() + ':' + e.lineno); });
Viz.mountAll();
var clicks = 0;
document.querySelectorAll('.viz-widget').forEach(function (w) {
  var name = w.dataset.viz + (w.dataset.mode ? ':' + w.dataset.mode : '');
  function press() {
    w.querySelectorAll('button').forEach(function (b) {
      if (b.disabled) return;
      try { b.click(); clicks++; } catch (e) { errors.push(name + ' [' + b.textContent + ']: ' + e.message); }
    });
  }
  var selects = Array.prototype.slice.call(w.querySelectorAll('.viz-bar:not(.bottom) select'));
  for (var round = 0; round < 4; round++) {
    press();
    w.querySelectorAll('input[type=text]').forEach(function (inp, i) { if (inp.inputMode === 'numeric') inp.value = [7, 0, 99, 29, 13][(round + i) % 5]; else inp.value = ['tr', 'heat', 'zebra', 'a'][round % 4]; });
    selects.forEach(function (s) {
      for (var k = 0; k < s.options.length; k++) {
        s.selectedIndex = k;
        try { s.dispatchEvent(new Event('change')); press(); } catch (e) { errors.push(name + ' select: ' + e.message); }
      }
    });
    w.querySelectorAll('input[type=checkbox]').forEach(function (c) { c.click(); });
    w.querySelectorAll('input[type=range]').forEach(function (r) { r.value = round * 33; r.dispatchEvent(new Event('input')); });
  }
});
setTimeout(function () { document.title = (errors.length ? 'STRESSFAIL ' + errors.slice(0, 6).join(' | ') : 'STRESSOK ' + clicks + ' clicks'); }, 3000);
</script></body></html>
JS
} > "$page"
cp "$root"/web/viz/*.js "$root/dist/html/assets/viz/"
google-chrome --headless=new --disable-gpu --no-sandbox --virtual-time-budget=20000 --dump-dom "file://$page" 2>/dev/null | grep -o '<title>[^<]*</title>'
rm -f "$page"
