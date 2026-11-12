// Chapter 9: union-find, with union by size and path compression that can
// each be switched off to see what they are for.
Viz.register('unionfind', function (root) {
  'use strict';
  var W = 640, HT = 270, N = 10;
  var ui = Viz.shell(root, { height: HT, label: 'Union-find' });
  var scene = new Viz.Scene(ui.svg);
  var parent, sz, bySize = true, compress = true;

  function draw(hl) {
    hl = hl || {};
    var kids = {}, roots = [], x;
    for (x = 0; x < N; x++) kids[x] = [];
    for (x = 0; x < N; x++) { if (parent[x] === x) roots.push(x); else kids[parent[x]].push(x); }
    var pos = Viz.layoutTree(-1, function (n) { return n === -1 ? roots : kids[n]; }, { width: W, height: 230, y0: -52, dy: 46, padY: 28, maxStep: 58 });
    var nodes = [], edges = [];
    for (x = 0; x < N; x++) {
      var p = pos.get(x);
      nodes.push({ id: 'u' + x, x: p.x, y: p.y, r: 15, label: x, cls: hl[x] || (parent[x] === x ? 'soft' : '') });
      if (parent[x] !== x) edges.push({ from: 'u' + x, to: 'u' + parent[x], directed: true });
    }
    Viz.cells('p', parent, 150, 214, 34, 26, { size: 12, cls: function (i) { return hl[i] || (parent[i] === i ? 'soft' : ''); } }).forEach(function (n) { nodes.push(n); });
    nodes.push(Viz.text('pl', 138, 227, 'parent', 'right strong'));
    return { nodes: nodes, edges: edges };
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); } });
  function fr(frames, msg, hl) { frames.push({ scene: draw(hl), msg: msg }); }

  function find(x, frames, quiet) {
    var path = [x], r = x, hl = {};
    while (parent[r] !== r) { r = parent[r]; path.push(r); }
    path.forEach(function (v) { hl[v] = 'hl'; }); hl[r] = 'ok';
    if (!quiet) fr(frames, 'find(' + x + '): climb parent links ' + path.join(' → ') + '. The root, <b>' + r + '</b>, is the set\'s representative. That took ' + (path.length - 1) + ' step' + (path.length === 2 ? '' : 's') + '.', hl);
    if (compress && path.length > 2) {
      path.slice(0, -1).forEach(function (v) { parent[v] = r; });
      if (!quiet) fr(frames, '<b>Path compression</b>: every node on that path now points straight at the root. The next find for any of them takes one step.', hl);
    }
    return r;
  }
  function unite(a, b) {
    var f = [], ra = find(a, f), rb = find(b, f), hl = {};
    if (ra === rb) { fr(f, a + ' and ' + b + ' have the same root: they are <b>already in the same set</b>.', (hl[ra] = 'ok', hl)); return f; }
    if (bySize && sz[ra] < sz[rb]) { var t = ra; ra = rb; rb = t; }
    parent[rb] = ra; sz[ra] += sz[rb];
    hl[ra] = 'ok'; hl[rb] = 'hl2';
    fr(f, 'unite(' + a + ', ' + b + '): the roots are ' + ra + ' and ' + rb + '. ' + (bySize ? 'The <b>smaller</b> tree (rooted at ' + rb + ') is hung beneath the larger, so nobody gets much deeper.' : 'Root ' + rb + ' is hung beneath ' + ra + ', whatever their sizes.'), hl);
    return f;
  }
  function reset() {
    parent = []; sz = [];
    for (var i = 0; i < N; i++) { parent.push(i); sz.push(1); }
    var f = []; fr(f, 'Ten elements, each in a set of its own: every element is its own parent. Unite some, then switch off union by size and watch the trees grow tall.');
    player.load(f, { autoplay: false });
  }
  var ia = Viz.input({ value: 0, label: 'first element' }), ib = Viz.input({ value: 1, label: 'second element' });
  function chk(label, on, set) {
    var c = Viz.h('input', { type: 'checkbox', checked: on, onchange: function () { set(c.checked); } });
    return Viz.h('label', null, c, label);
  }
  ui.top.appendChild(ia); ui.top.appendChild(ib);
  ui.top.appendChild(Viz.btn('unite', function () { var a = Viz.int(ia, 0, N - 1), b = Viz.int(ib, 0, N - 1); if (a == null || b == null) return; player.load(unite(a, b)); ia.value = Viz.rand(0, N - 1); ib.value = Viz.rand(0, N - 1); }, 'primary'));
  ui.top.appendChild(Viz.btn('find first', function () { var a = Viz.int(ia, 0, N - 1); if (a == null) return; var f = []; find(a, f); player.load(f); }));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(chk('union by size', true, function (v) { bySize = v; }));
  ui.top.appendChild(chk('path compression', true, function (v) { compress = v; }));
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return {
    demo: function () {
      compress = false;
      [[0, 1], [2, 3], [0, 2], [4, 5], [6, 4], [7, 8], [4, 0]].forEach(function (p) { unite(p[0], p[1]); });
      compress = true;
      var f = []; find(5, f); player.load(f, { testAt: 0 });
    }
  };
});
