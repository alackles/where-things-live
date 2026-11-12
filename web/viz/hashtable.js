// Chapter 5: a hash table with separate chaining or linear probing.
// Keys are small integers and h(k) = k mod m, so everything can be checked by hand.
Viz.register('hashtable', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 250, label: 'A hash table' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var mode = 'probing', M, chains, slots, lastProbes = 0;

  function count() {
    if (mode === 'chaining') return chains.reduce(function (a, c) { return a + c.length; }, 0);
    return slots.filter(function (s) { return s.st === 'full'; }).length;
  }
  function tombs() { return mode === 'probing' ? slots.filter(function (s) { return s.st === 'del'; }).length : 0; }
  function statsHtml() {
    var n = count();
    return '<span>h(k) = k mod <b>' + M + '</b></span><span>keys <b>' + n + '</b></span><span>load factor <b>' + (n / M).toFixed(2) + '</b></span>' +
      (mode === 'probing' ? '<span>tombstones <b>' + tombs() + '</b></span>' : '') + '<span>slots examined by the last operation <b>' + lastProbes + '</b></span>';
  }
  // hl: {bucket/slot index: cls}, ehl: {"b:j": cls} for chain entries
  function draw(hl, ehl) {
    hl = hl || {}; ehl = ehl || {};
    var nodes = [], edges = [];
    if (mode === 'chaining') {
      chains.forEach(function (chain, b) {
        var y = 26 + b * 27;
        nodes.push({ id: 'b' + b, shape: 'rect', x: 70, y: y, w: 40, h: 22, label: chain.length ? '●' : '', size: 8, sub: b, subPos: 'left', cls: hl[b] || 'soft' });
        chain.forEach(function (k, j) {
          nodes.push({ id: 'e' + k, shape: 'rect', x: 150 + j * 62, y: y, w: 44, h: 22, label: k, cls: ehl[b + ':' + j] || '' });
          edges.push({ from: j ? 'e' + chain[j - 1] : 'b' + b, to: 'e' + k, directed: true });
        });
      });
    } else {
      Viz.cells('s', slots.map(function (s) { return s.st === 'full' ? s.k : s.st === 'del' ? 'del' : ''; }), 44, 96, 46, 38, {
        cls: function (i) { return hl[i] || (slots[i].st === 'del' ? 'dim' : slots[i].st === 'full' ? '' : 'soft'); }
      }).forEach(function (n) { nodes.push(n); });
    }
    return { nodes: nodes, edges: edges };
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  function fr(frames, msg, hl, ehl) { frames.push({ scene: draw(hl, ehl), msg: msg, stats: statsHtml() }); }
  function one(a, cls) { var o = {}; o[a] = cls; return o; }

  // ---- chaining ----
  function cFind(k, frames) {
    var b = ((k % M) + M) % M, chain = chains[b];
    lastProbes = 0;
    fr(frames, 'h(' + k + ') = ' + k + ' mod ' + M + ' = <b>' + b + '</b>. If ' + k + ' is anywhere, it is in bucket ' + b + '\'s chain.', one(b, 'hl'));
    for (var j = 0; j < chain.length; j++) {
      lastProbes++;
      if (chain[j] === k) { fr(frames, 'Entry ' + (j + 1) + ' of the chain is ' + k + ': <b>found</b>.', one(b, 'hl'), one(b + ':' + j, 'ok')); return j; }
      fr(frames, 'Entry ' + (j + 1) + ' of the chain is ' + chain[j] + ', not ' + k + '.', one(b, 'hl'), one(b + ':' + j, 'hl2'));
    }
    return -1;
  }
  function cInsert(k) {
    var frames = [], b = ((k % M) + M) % M;
    if (cFind(k, frames) >= 0) { fr(frames, k + ' is already in the table. A set holds each key once.', one(b, 'hl')); return frames; }
    if (chains[b].length >= 7) { fr(frames, 'That chain is as long as this picture can show. (A real table would have grown long ago.)'); return frames; }
    chains[b].push(k);
    fr(frames, 'Not in the chain' + (chains[b].length > 1 ? ' (a <b>collision</b>: this bucket was already in use)' : '') + ', so ' + k + ' is appended to it.', one(b, 'hl'), one(b + ':' + (chains[b].length - 1), 'ok'));
    return frames;
  }
  function cErase(k) {
    var frames = [], b = ((k % M) + M) % M, j = cFind(k, frames);
    if (j < 0) { fr(frames, k + ' is not in the table; nothing to erase.', one(b, 'hl')); return frames; }
    var chain = chains[b];
    chain[j] = chain[chain.length - 1]; chain.pop();
    fr(frames, 'Erased. The last entry of the chain is moved into the gap; order within a chain does not matter.', one(b, 'hl'));
    return frames;
  }
  // ---- linear probing ----
  function pFind(k, frames) {
    var i = ((k % M) + M) % M, steps = 0;
    lastProbes = 0;
    fr(frames, 'h(' + k + ') = ' + k + ' mod ' + M + ' = <b>' + i + '</b>: the key\'s home slot.', one(i, 'hl'));
    while (slots[i].st !== 'empty' && steps < M) {
      lastProbes++; steps++;
      if (slots[i].st === 'full' && slots[i].k === k) { fr(frames, 'Slot ' + i + ' holds ' + k + ': <b>found</b> after examining ' + lastProbes + ' slot' + (lastProbes > 1 ? 's' : '') + '.', one(i, 'ok')); return i; }
      fr(frames, 'Slot ' + i + (slots[i].st === 'del' ? ' is a <b>tombstone</b>: something was here once, so the search must keep going.' : ' holds ' + slots[i].k + ', not ' + k + '. Try the next slot.'), one(i, 'hl2'));
      i = (i + 1) % M;
    }
    if (steps < M) { lastProbes++; fr(frames, 'Slot ' + i + ' is <b>empty</b>. If ' + k + ' were in the table it would have been placed here or earlier, so it is <b>not present</b>.', one(i, 'bad')); }
    return -1;
  }
  function pInsert(k) {
    var frames = [];
    if (pFind(k, frames) >= 0) { fr(frames, k + ' is already in the table.'); return frames; }
    if (count() >= M - 3) { fr(frames, 'This table is ' + count() + '/' + M + ' full. A real probing table keeps itself at most half full and would have grown before now. Erase something, or reset.'); return frames; }
    if (count() + tombs() >= M - 2) {
      var live = slots.filter(function (s) { return s.st === 'full'; }).map(function (s) { return s.k; });
      slots = []; for (var q = 0; q < M; q++) slots.push({ st: 'empty' });
      live.forEach(function (key) { var j = ((key % M) + M) % M; while (slots[j].st === 'full') j = (j + 1) % M; slots[j] = { st: 'full', k: key }; });
      fr(frames, 'Tombstones have clogged the table. <b>Rebuild</b>: re-insert every live key into a clean table, which discards them all.');
    }
    var i = ((k % M) + M) % M;
    while (slots[i].st === 'full') i = (i + 1) % M;
    var was = slots[i].st;
    slots[i] = { st: 'full', k: k };
    fr(frames, 'Not present, so insert it in the first slot at or after its home that is not full: slot <b>' + i + '</b>' + (was === 'del' ? ', reusing a tombstone.' : i !== ((k % M) + M) % M ? '. It has been displaced from its home by a <b>cluster</b>.' : '.'), one(i, 'ok'));
    return frames;
  }
  function pErase(k) {
    var frames = [], i = pFind(k, frames);
    if (i < 0) { fr(frames, k + ' is not in the table; nothing to erase.'); return frames; }
    slots[i] = { st: 'del' };
    fr(frames, 'Erased by marking the slot <b>deleted</b>, not empty. Emptying it could cut off the path to keys stored beyond it.', one(i, 'hl2'));
    return frames;
  }

  function reset(silent) {
    lastProbes = 0;
    if (mode === 'chaining') { M = 8; chains = []; for (var b = 0; b < M; b++) chains.push([]); [12, 5, 29, 21, 16, 34].forEach(function (k) { chains[k % M].push(k); }); }
    else { M = 12; slots = []; for (var i = 0; i < M; i++) slots.push({ st: 'empty' }); [24, 17, 6, 41, 22].forEach(function (k) { var j = k % M; while (slots[j].st === 'full') j = (j + 1) % M; slots[j] = { st: 'full', k: k }; }); }
    if (!silent) player.load([{ scene: draw(), msg: mode === 'chaining' ? 'Separate chaining: each bucket leads to a chain of the keys that hash there. Try inserting 13, which collides with 5, 29 and 21.' : 'Linear probing: every key lives in the array itself. Try inserting 29, whose home slot (5) is taken; then erase 17 and search for 41.', stats: statsHtml() }], { autoplay: false });
  }
  var input = Viz.input({ value: 29, label: 'key', onenter: function () { act('insert'); } });
  function act(what) {
    var k = Viz.int(input, 0, 999);
    if (k == null) return;
    var f = mode === 'chaining' ? (what === 'insert' ? cInsert(k) : what === 'erase' ? cErase(k) : (function () { var a = []; if (cFind(k, a) < 0) fr(a, 'Reached the end of the chain: ' + k + ' is <b>not present</b>.'); return a; })())
      : (what === 'insert' ? pInsert(k) : what === 'erase' ? pErase(k) : (function () { var a = []; pFind(k, a); return a; })());
    f.forEach(function (x, i) { if (i === f.length - 1) x.stats = statsHtml(); });
    player.load(f);
  }
  var sel = Viz.select([['probing', 'Linear probing'], ['chaining', 'Separate chaining']], mode, function (v) { mode = v; input.value = v === 'chaining' ? 13 : 29; reset(); });
  ui.top.appendChild(sel);
  ui.top.appendChild(Viz.h('label', null, 'Key ', input));
  ui.top.appendChild(Viz.btn('insert', function () { act('insert'); }, 'primary'));
  ui.top.appendChild(Viz.btn('find', function () { act('find'); }));
  ui.top.appendChild(Viz.btn('erase', function () { act('erase'); }));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', function () { reset(); }));
  reset();
  return { demo: function () { input.value = 29; act('insert'); } };
});
