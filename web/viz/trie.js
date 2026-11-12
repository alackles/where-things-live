// Chapter 10: a trie of lowercase words, with prefix completion.
Viz.register('trie', function (root) {
  'use strict';
  var W = 640, HT = 300;
  var ui = Viz.shell(root, { height: HT, label: 'A trie' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var trie, uid;

  function node(ch) { return { id: uid++, ch: ch, kids: {}, word: false }; }
  function kidsOf(n) { return Object.keys(n.kids).sort().map(function (c) { return n.kids[c]; }); }
  function countNodes(n) { return 1 + kidsOf(n).reduce(function (a, k) { return a + countNodes(k); }, 0); }
  function countLeaves(n) { var k = kidsOf(n); return k.length ? k.reduce(function (a, c) { return a + countLeaves(c); }, 0) : 1; }
  function words(n, prefix, out) { if (n.word) out.push(prefix); kidsOf(n).forEach(function (k) { words(k, prefix + k.ch, out); }); return out; }

  function draw(hl) {
    hl = hl || {};
    var pos = Viz.layoutTree(trie, kidsOf, { width: W, height: HT, dy: 44, padY: 24, maxStep: 54, pad: 12 });
    var nodes = [], edges = [];
    pos.forEach(function (p, n) {
      nodes.push({ id: 'n' + n.id, x: p.x, y: p.y, r: 13, label: n.ch, cls: hl[n.id] || (n.word ? 'vio' : ''), sub: n.word ? '✓' : null, subPos: 'ne' });
      kidsOf(n).forEach(function (k) { edges.push({ from: 'n' + n.id, to: 'n' + k.id }); });
    });
    return { nodes: nodes, edges: edges };
  }
  function statsHtml() { return '<span>words <b>' + words(trie, '', []).length + '</b></span><span>nodes <b>' + countNodes(trie) + '</b></span><span>shaded nodes with ✓ end a word</span>'; }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  function fr(frames, msg, hl) { frames.push({ scene: draw(hl), msg: msg, stats: statsHtml() }); }

  function insert(word, frames) {
    var n = trie, hl = {}, made = 0;
    if (countLeaves(trie) >= 11 && !walk(word)) { if (frames) fr(frames, 'The picture is full. Reset to start again.'); return; }
    for (var i = 0; i < word.length; i++) {
      var c = word[i], fresh = !n.kids[c];
      if (fresh) { n.kids[c] = node(c); made++; }
      n = n.kids[c]; hl[n.id] = fresh ? 'ok' : 'hl';
      if (frames) fr(frames, '"' + word.slice(0, i + 1) + '": ' + (fresh ? 'no edge for <b>' + c + '</b> here, so create a node.' : 'follow the existing edge for <b>' + c + '</b>. This prefix is shared with another word.'), Object.assign({}, hl));
    }
    var was = n.word; n.word = true;
    if (frames) fr(frames, was ? '"' + word + '" was already in the trie.' : 'Mark the last node as the end of a word. "' + word + '" needed ' + made + ' new node' + (made === 1 ? '' : 's') + ' for its ' + word.length + ' letters.', hl);
  }
  function walk(s) { var n = trie; for (var i = 0; i < s.length && n; i++) n = n.kids[s[i]]; return n; }
  function complete(prefix, frames) {
    var n = trie, hl = {};
    for (var i = 0; i < prefix.length; i++) {
      n = n.kids[prefix[i]];
      if (!n) { fr(frames, 'There is no path for "' + prefix + '": no word in the trie begins with it.', hl); return; }
      hl[n.id] = 'hl';
    }
    fr(frames, 'Walk the path for the prefix "' + prefix + '": ' + prefix.length + ' step' + (prefix.length === 1 ? '' : 's') + ', however many words the trie holds.', Object.assign({}, hl));
    (function mark(m) { hl[m.id] = hl[m.id] || 'ok'; kidsOf(m).forEach(mark); })(n);
    var out = words(n, prefix, []);
    fr(frames, 'Every word with that prefix lives in the subtree below: <b>' + out.join(', ') + '</b>.', hl);
  }
  function reset() {
    uid = 0; trie = node('');
    ['tree', 'trie', 'try', 'trial', 'heap', 'hash', 'he'].forEach(function (w) { insert(w, null); });
    var f = []; fr(f, 'Seven words. Words that share a prefix share the path for it. Insert "heat" or "trip", or complete the prefix "tr".');
    player.load(f, { autoplay: false });
  }
  var input = Viz.input({ value: 'heat', numeric: false, cls: 'wide', label: 'word or prefix', onenter: function () { go(insert); } });
  function text() { return String(input.value).toLowerCase().replace(/[^a-z]/g, '').slice(0, 6); }
  function go(op) { var w = text(); if (!w && op === insert) return; var f = []; op(w, f); player.load(f); }
  ui.top.appendChild(Viz.h('label', null, 'Letters ', input));
  ui.top.appendChild(Viz.btn('insert word', function () { go(insert); }, 'primary'));
  ui.top.appendChild(Viz.btn('complete prefix', function () { go(complete); }));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { var f = []; complete('tr', f); player.load(f); } };
});
