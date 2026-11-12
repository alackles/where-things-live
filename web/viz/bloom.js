// Chapter 5: a Bloom filter of 32 bits with 3 hash functions.
Viz.register('bloom', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 170, label: 'A Bloom filter' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg, { duration: 0 });
  var M = 32, K = 3, bits, added, tests, falsePos;
  var WORDS = ['heap', 'stack', 'queue', 'tree', 'graph', 'hash', 'trie', 'list', 'array', 'vector', 'node', 'edge', 'sort', 'merge', 'pivot', 'cache'];

  function fnv(str) {
    var hsh = 2166136261;
    for (var i = 0; i < str.length; i++) { hsh ^= str.charCodeAt(i); hsh = Math.imul(hsh, 16777619); }
    return hsh >>> 0;
  }
  function mix(x) { x ^= x >>> 16; x = Math.imul(x, 0x45d9f3b); x ^= x >>> 16; return x >>> 0; }
  function positions(word) {
    var h1 = fnv(word), h2 = mix(h1) | 1, out = [];
    for (var i = 0; i < K; i++) out.push(((h1 + Math.imul(i, h2)) >>> 0) % M);
    return out;
  }
  function draw(hot, cls) {
    hot = hot || [];
    return {
      nodes: Viz.cells('b', bits.map(function (b) { return b ? 1 : 0; }), 48, 62, 17, 28, {
        size: 11, index: false,
        cls: function (i) { return hot.indexOf(i) >= 0 ? (bits[i] ? cls : 'bad') : bits[i] ? 'hl' : ''; }
      }).concat([Viz.text('i0', 56, 104, '0', ''), Viz.text('i31', 48 + 31 * 17 + 8, 104, '31', '')]).concat(
        hot.map(function (p, j) { return Viz.text('p' + j, 48 + p * 17 + 8.5, 44, '▼', cls === 'ok' || cls === 'hl2' ? 'acc2' : ''); })),
      edges: []
    };
  }
  function statsHtml() {
    var set = bits.filter(Boolean).length;
    return '<span>keys added <b>' + added.size + '</b></span><span>bits set <b>' + set + ' / ' + M + '</b></span><span>chance a new key tests positive ≈ <b>' + (100 * Math.pow(set / M, K)).toFixed(1) + '%</b></span><span>false positives seen <b>' + falsePos + '</b> of ' + tests + ' tests</span>';
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f) { scene.set(f.scene, false); stats.innerHTML = f.stats; } });
  function show(scn, msg) { player.load([{ scene: scn, msg: msg, stats: statsHtml() }], { autoplay: false }); }

  function add(word) {
    var pos = positions(word);
    pos.forEach(function (p) { bits[p] = true; });
    added.add(word);
    show(draw(pos, 'hl2'), 'add("' + Viz.esc(word) + '"): its three hashes select bits <b>' + pos.join(', ') + '</b>, which are set to 1. The word itself is not stored anywhere.');
  }
  function test(word) {
    var pos = positions(word), zero = pos.filter(function (p) { return !bits[p]; });
    tests++;
    if (zero.length) { show(draw(pos, 'ok'), 'might_contain("' + Viz.esc(word) + '"): bit ' + zero[0] + ' is 0. Adding this word would have set it, so the word was <b>definitely never added</b>.'); return; }
    if (added.has(word)) show(draw(pos, 'ok'), 'might_contain("' + Viz.esc(word) + '"): bits ' + pos.join(', ') + ' are all 1, so the answer is <b>probably yes</b>. (And it is right: this word was added.)');
    else { falsePos++; show(draw(pos, 'bad'), 'might_contain("' + Viz.esc(word) + '"): bits ' + pos.join(', ') + ' are all 1, so the filter says <b>probably yes</b>. But this word was never added; other words set those bits. A <b>false positive</b>.'); }
  }
  function reset() {
    bits = []; for (var i = 0; i < M; i++) bits.push(false);
    added = new Set(); tests = 0; falsePos = 0;
    show(draw(), 'An empty filter: 32 bits, all zero. Add some words, then test words you did <i>not</i> add. As the filter fills, false positives appear.');
  }
  var input = Viz.input({ value: 'heap', numeric: false, cls: 'wide', label: 'word', onenter: function () { add(word()); } });
  function word() { return String(input.value).trim().toLowerCase() || 'heap'; }
  ui.top.appendChild(Viz.h('label', null, 'Word ', input));
  ui.top.appendChild(Viz.btn('add', function () { add(word()); }, 'primary'));
  ui.top.appendChild(Viz.btn('test', function () { test(word()); }));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('add 4 more words', function () {
    var n = 0;
    WORDS.forEach(function (w) { if (n < 4 && !added.has(w)) { positions(w).forEach(function (p) { bits[p] = true; }); added.add(w); n++; } });
    show(draw(), 'Added. The filter now represents ' + added.size + ' words in ' + M + ' bits. Try testing words that are not among them, such as "zebra", "apple", or "c++".');
  }));
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { ['heap', 'stack', 'queue', 'tree', 'graph', 'hash'].forEach(add); var w = ['zebra', 'apple', 'mango', 'delta', 'kiwi', 'plum', 'fig', 'lime', 'pear', 'oak', 'elm', 'ash']; for (var i = 0; i < w.length; i++) { test(w[i]); if (falsePos) break; } } };
});
