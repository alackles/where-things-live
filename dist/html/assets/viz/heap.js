// Chapter 8: a binary min-heap, shown as a tree and as the array that stores it.
Viz.register('heap', function (root) {
  'use strict';
  var W = 640, HT = 270, MAXN = 15;
  var ui = Viz.shell(root, { height: HT, label: 'A binary min-heap' });
  var scene = new Viz.Scene(ui.svg);
  var a = [], uid = 0, AX = 35, AW = 38, AY = 222;

  function tpos(i) {
    var d = Math.floor(Math.log2(i + 1)), k = i - (Math.pow(2, d) - 1), slots = Math.pow(2, d);
    return [W / 2 - 280 + (k + 0.5) * 560 / slots, 30 + d * 46];
  }
  function draw(hl) {
    hl = hl || {};
    var nodes = [], edges = [];
    a.forEach(function (e, i) {
      var p = tpos(i);
      nodes.push({ id: 't' + e.id, x: p[0], y: p[1], r: 16, label: e.v, cls: hl[i] || '' });
      nodes.push({ id: 'a' + e.id, shape: 'rect', x: AX + i * AW + AW / 2, y: AY, w: AW, h: 28, label: e.v, cls: hl[i] || '' });
      if (i) edges.push({ from: 't' + a[Math.floor((i - 1) / 2)].id, to: 't' + e.id });
    });
    for (var i = 0; i < MAXN; i++) nodes.push(Viz.text('ix' + i, AX + i * AW + AW / 2, AY + 24, i, ''));
    return { nodes: nodes, edges: edges };
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); } });
  function fr(frames, msg, hl) { frames.push({ scene: draw(hl), msg: msg }); }
  function two(i, ci, j, cj) { var o = {}; o[i] = ci; if (j != null) o[j] = cj; return o; }
  function swap(i, j) { var t = a[i]; a[i] = a[j]; a[j] = t; }

  function siftUp(i, frames) {
    while (i > 0) {
      var p = Math.floor((i - 1) / 2);
      if (a[i].v >= a[p].v) { fr(frames, a[i].v + ' is not smaller than its parent ' + a[p].v + ' (index (' + i + ' - 1) / 2 = ' + p + '). The heap order holds: done.', two(i, 'ok', p, 'hl')); return; }
      fr(frames, a[i].v + ' is smaller than its parent ' + a[p].v + ' at index (' + i + ' - 1) / 2 = ' + p + '. Swap them.', two(i, 'hl2', p, 'hl'));
      swap(i, p); i = p;
    }
    fr(frames, 'It has risen all the way to the root: it is the new minimum.', two(0, 'ok'));
  }
  function siftDown(i, frames) {
    while (true) {
      var l = 2 * i + 1, r = l + 1, s = l;
      if (l >= a.length) { fr(frames, 'No children: it has reached the bottom. Done.', two(i, 'ok')); return; }
      if (r < a.length && a[r].v < a[l].v) s = r;
      if (a[s].v >= a[i].v) { fr(frames, 'Its children are no smaller. The heap order holds: done.', two(i, 'ok', s, 'hl')); return; }
      fr(frames, a[i].v + ' is larger than its smaller child, ' + a[s].v + ' at index ' + s + '. Swap with the <b>smaller</b> child.', two(i, 'hl2', s, 'hl'));
      swap(i, s); i = s;
    }
  }
  function push(v) {
    var f = [];
    if (a.length >= MAXN) { fr(f, 'The picture holds ' + MAXN + ' elements. Pop some first.'); return f; }
    a.push({ id: uid++, v: v });
    fr(f, 'push ' + v + ': it goes in the next free slot, index ' + (a.length - 1) + ', the only place that keeps the tree complete.', two(a.length - 1, 'hl2'));
    siftUp(a.length - 1, f);
    return f;
  }
  function pop() {
    var f = [];
    if (!a.length) { fr(f, 'The heap is empty.'); return f; }
    fr(f, 'The minimum, ' + a[0].v + ', is at the root. To remove it, move the <b>last</b> element, ' + a[a.length - 1].v + ', into its place.', two(0, 'bad', a.length - 1, 'hl2'));
    swap(0, a.length - 1);
    var gone = a.pop();
    if (!a.length) { fr(f, 'Removed ' + gone.v + '. The heap is now empty.'); return f; }
    fr(f, 'Removed ' + gone.v + '. The shape is right again, but ' + a[0].v + ' is probably too large to be the root. Let it sink.', two(0, 'hl2'));
    siftDown(0, f);
    return f;
  }
  function reset() {
    a = []; uid = 0;
    [1, 3, 2, 7, 4, 5, 9, 8].forEach(function (v) { a.push({ id: uid++, v: v }); });
    var f = []; fr(f, 'A min-heap of eight elements. The tree is only a way of looking at the array underneath: the children of index i are at 2i + 1 and 2i + 2. Try push 0, then pop.');
    player.load(f, { autoplay: false });
  }
  var input = Viz.input({ value: 0, label: 'value', onenter: function () { go(); } });
  function go() { var v = Viz.int(input, 0, 99); if (v == null) return; player.load(push(v)); input.value = Viz.rand(0, 30); }
  ui.top.appendChild(Viz.h('label', null, 'Value ', input));
  ui.top.appendChild(Viz.btn('push', go, 'primary'));
  ui.top.appendChild(Viz.btn('pop', function () { player.load(pop()); }, 'primary'));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { var f = push(0); player.load(f, { testAt: 2 }); } };
});
