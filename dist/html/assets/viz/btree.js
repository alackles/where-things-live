// Chapter 7: a B-tree whose nodes hold at most three keys.
Viz.register('btree', function (root) {
  'use strict';
  var W = 640, HT = 250, MAX = 3, CAP = 22;
  var ui = Viz.shell(root, { height: HT, label: 'A B-tree' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var tree, uid = 0;

  function node(keys, kids) { return { id: uid++, keys: keys || [], kids: kids || [] }; }
  function leaves(n) { return n.kids.length ? n.kids.reduce(function (a, k) { return a + leaves(k); }, 0) : 1; }
  function height(n) { var h = 0; while (n) { h++; n = n.kids[0]; } return h; }
  function count(n) { return n.keys.length + n.kids.reduce(function (a, k) { return a + count(k); }, 0); }

  function draw(hl) {
    hl = hl || {};
    var pos = Viz.layoutTree(tree, function (n) { return n.kids; }, { width: W, height: HT, maxStep: 120, dy: 74, padY: 34, pad: 10 });
    var nodes = [], edges = [];
    pos.forEach(function (p, n) {
      nodes.push({ id: 'b' + n.id, shape: 'rect', x: p.x, y: p.y, w: Math.max(1, n.keys.length) * 22 + 12, h: 28, rx: 4, label: n.keys.join('  '), size: 12, cls: hl[n.id] || (n.kids.length ? 'soft' : '') });
      n.kids.forEach(function (k) { edges.push({ from: 'b' + n.id, to: 'b' + k.id, toSide: 't' }); });
    });
    return { nodes: nodes, edges: edges };
  }
  function statsHtml() { return '<span>keys <b>' + count(tree) + '</b></span><span>height <b>' + height(tree) + '</b></span><span>at most <b>' + MAX + '</b> keys per node</span><span>all leaves at the same depth</span>'; }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  function fr(frames, msg, hl) { frames.push({ scene: draw(hl), msg: msg, stats: statsHtml() }); }
  function one(n, cls) { var o = {}; o[n.id] = cls; return o; }

  function insert(k, frames) {
    var path = [], n = tree, i;
    if (count(tree) >= CAP || leaves(tree) >= 8) { fr(frames, 'The picture is full. Reset to start again.'); return; }
    while (true) {
      path.push(n);
      if (n.keys.indexOf(k) >= 0) { fr(frames, k + ' is already in the tree.', one(n, 'ok')); return; }
      if (!n.kids.length) break;
      i = 0; while (i < n.keys.length && k > n.keys[i]) i++;
      fr(frames, 'In this node, ' + k + (i === 0 ? ' is less than ' + n.keys[0] : i === n.keys.length ? ' is greater than ' + n.keys[i - 1] : ' falls between ' + n.keys[i - 1] + ' and ' + n.keys[i]) + ': descend into child ' + (i + 1) + ' of ' + n.kids.length + '.', one(n, 'hl'));
      n = n.kids[i];
    }
    n.keys.push(k); n.keys.sort(function (a, b) { return a - b; });
    fr(frames, 'Reached a leaf. Insert ' + k + ' in sorted position.' + (n.keys.length > MAX ? ' The leaf now has ' + n.keys.length + ' keys, one too many.' : ' It has room, so that is all.'), one(n, n.keys.length > MAX ? 'bad' : 'ok'));
    for (var d = path.length - 1; d >= 0 && path[d].keys.length > MAX; d--) {
      var full = path[d], mid = Math.floor(full.keys.length / 2), up = full.keys[mid];
      var right = node(full.keys.slice(mid + 1), full.kids.slice(mid + 1));
      full.keys = full.keys.slice(0, mid); full.kids = full.kids.slice(0, mid + 1);
      if (!right.kids.length) full.kids = [];
      var hl = one(full, 'hl2'); hl[right.id] = 'hl2';
      if (d === 0) {
        tree = node([up], [full, right]);
        hl[tree.id] = 'ok';
        fr(frames, 'The <b>root</b> is overfull. It splits, and its middle key ' + up + ' becomes a new root above the two halves. The tree is one level taller, and every leaf is one level deeper: still all at the same depth.', hl);
      } else {
        var parent = path[d - 1], at = parent.kids.indexOf(full);
        parent.keys.splice(at, 0, up); parent.kids.splice(at + 1, 0, right);
        hl[parent.id] = parent.keys.length > MAX ? 'bad' : 'ok';
        fr(frames, '<b>Split</b>: the middle key ' + up + ' moves up into the parent, and the other keys divide into two nodes.' + (parent.keys.length > MAX ? ' Now the parent is overfull.' : ''), hl);
      }
    }
  }
  function reset() {
    uid = 0; tree = node();
    var junk = [];
    [10, 20, 30, 40, 50, 60, 70, 80, 90].forEach(function (k) { insert(k, junk); });
    var f = []; fr(f, 'A B-tree with at most three keys per node. Insert 95 to overfill the rightmost leaf; keep inserting to make the root split.');
    player.load(f, { autoplay: false });
  }
  var input = Viz.input({ value: 95, label: 'key', onenter: go });
  function go() { var k = Viz.int(input, 0, 999); if (k == null) return; var f = []; insert(k, f); player.load(f); input.value = Viz.rand(1, 99); }
  ui.top.appendChild(Viz.h('label', null, 'Key ', input));
  ui.top.appendChild(Viz.btn('insert', go, 'primary'));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { var f = []; insert(95, f); insert(96, f); insert(97, f); player.load(f, { testAt: f.length - 1 }); } };
});
