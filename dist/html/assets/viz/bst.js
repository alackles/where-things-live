// Chapters 6 and 7: a binary search tree; with data-mode="avl", one that
// rebalances itself by rotations after every insertion and removal.
Viz.register('bst', function (root, opts) {
  'use strict';
  var AVL = opts.mode === 'avl';
  var W = 640, HT = 300;
  var ui = Viz.shell(root, { height: HT, label: AVL ? 'An AVL tree' : 'A binary search tree' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var tree = null, MAXN = 15, MAXH = 7;

  function ht(n) { return n ? n.h : 0; }
  function upd(n) { n.h = 1 + Math.max(ht(n.left), ht(n.right)); }
  function bf(n) { return ht(n.left) - ht(n.right); }
  function fix(n) { if (!n) return 0; fix(n.left); fix(n.right); upd(n); return n.h; }
  function size(n) { return n ? 1 + size(n.left) + size(n.right) : 0; }

  function draw(hl, ehl) {
    hl = hl || {};
    var order = [], maxd = 0, nodes = [], edges = [];
    (function walk(n, d) { if (!n) return; walk(n.left, d + 1); order.push([n, d]); if (d > maxd) maxd = d; walk(n.right, d + 1); })(tree, 0);
    var step = Math.min(46, (W - 60) / Math.max(order.length, 1)), dy = Math.min(50, maxd ? (HT - 70) / maxd : 50);
    var left = (W - step * (order.length - 1)) / 2;
    order.forEach(function (p, i) {
      var n = p[0];
      nodes.push({ id: 'k' + n.key, x: left + i * step, y: 30 + p[1] * dy, r: 16, label: n.key, cls: hl[n.key] || '', sub: AVL ? (bf(n) > 0 ? '+' : '') + bf(n) : null, subPos: 'ne' });
      if (n.left) edges.push({ from: 'k' + n.key, to: 'k' + n.left.key });
      if (n.right) edges.push({ from: 'k' + n.key, to: 'k' + n.right.key });
    });
    return { nodes: nodes, edges: edges };
  }
  function statsHtml() {
    var n = size(tree), ideal = Math.ceil(Math.log2(n + 1));
    return '<span>keys <b>' + n + '</b></span><span>height <b>' + ht(tree) + '</b></span><span>smallest possible height for ' + n + ' keys <b>' + ideal + '</b></span>';
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  function fr(frames, msg, hl) { fix(tree); frames.push({ scene: draw(hl), msg: msg, stats: statsHtml() }); }
  function marks(path, cls, last, lastCls) { var o = {}; path.forEach(function (n) { o[n.key] = cls; }); if (last) o[last.key] = lastCls; return o; }

  function attach(parent, old, neu) {
    if (!parent) tree = neu; else if (parent.left === old) parent.left = neu; else parent.right = neu;
  }
  function rotR(z) { var y = z.left; z.left = y.right; y.right = z; upd(z); upd(y); return y; }
  function rotL(z) { var y = z.right; z.right = y.left; y.left = z; upd(z); upd(y); return y; }

  // Walk back up `path` (root first), restoring the AVL invariant.
  function rebalance(path, frames) {
    for (var i = path.length - 1; i >= 0; i--) {
      var z = path[i], parent = i ? path[i - 1] : null;
      fix(tree);
      var b = bf(z);
      if (b > 1 || b < -1) {
        var leftHeavy = b > 1, child = leftHeavy ? z.left : z.right, cb = bf(child);
        var inside = leftHeavy ? cb < 0 : cb > 0;
        var o = {}; o[z.key] = 'bad'; o[child.key] = 'hl2';
        fr(frames, 'Node <b>' + z.key + '</b> has balance factor ' + (b > 0 ? '+' : '') + b + ': its ' + (leftHeavy ? 'left' : 'right') + ' subtree is two levels taller. The extra height is on the <b>' + (inside ? 'inside' : 'outside') + '</b> (' + (leftHeavy ? 'left' : 'right') + '-' + (inside ? (leftHeavy ? 'right' : 'left') : (leftHeavy ? 'left' : 'right')) + ' case).', o);
        if (inside) {
          var c2 = leftHeavy ? rotL(child) : rotR(child);
          if (leftHeavy) z.left = c2; else z.right = c2;
          var o2 = {}; o2[z.key] = 'bad'; o2[c2.key] = 'hl2';
          fr(frames, 'First rotate <b>' + (leftHeavy ? 'left' : 'right') + ' at ' + child.key + '</b>, to straighten the zig-zag into the outside case.', o2);
        }
        var top = leftHeavy ? rotR(z) : rotL(z);
        attach(parent, z, top);
        var o3 = {}; o3[top.key] = 'ok';
        fr(frames, 'Rotate <b>' + (leftHeavy ? 'right' : 'left') + ' at ' + z.key + '</b>. ' + top.key + ' rises, ' + z.key + ' goes down, and the subtree is balanced again.', o3);
      }
    }
  }

  function insert(k, frames) {
    var path = [], n = tree;
    if (size(tree) >= MAXN) { fr(frames, 'The picture holds at most ' + MAXN + ' keys. Remove one, or clear the tree.'); return; }
    if (!tree) { tree = { key: k, left: null, right: null, h: 1 }; fr(frames, 'The tree is empty, so ' + k + ' becomes the root.', marks([], '', tree, 'ok')); return; }
    while (n) {
      path.push(n);
      if (k === n.key) { fr(frames, k + ' is already in the tree. A set holds each key once.', marks(path, 'hl', n, 'ok')); return; }
      var goLeft = k < n.key, next = goLeft ? n.left : n.right;
      if (!next) {
        if (path.length >= MAXH) { fr(frames, 'That would make the tree ' + (MAXH + 1) + ' levels deep, taller than this picture can show. ' + (AVL ? '' : 'Notice how unbalanced it has become.'), marks(path, 'hl')); return; }
        fr(frames, k + (goLeft ? ' < ' : ' > ') + n.key + ', and there is no ' + (goLeft ? 'left' : 'right') + ' child: this empty spot is where ' + k + ' belongs.', marks(path, 'hl', n, 'hl2'));
        var leaf = { key: k, left: null, right: null, h: 1 };
        if (goLeft) n.left = leaf; else n.right = leaf;
        fr(frames, 'Inserted ' + k + ' as a new leaf.' + (AVL ? ' Now walk back up the path, checking balance factors.' : ''), marks(path, 'hl', leaf, 'ok'));
        if (AVL) { var before = frames.length; rebalance(path, frames); if (frames.length === before) fr(frames, 'Every node on the path still has balance factor -1, 0 or +1. No rotation needed.', marks([], '', leaf, 'ok')); }
        return;
      }
      fr(frames, k + (goLeft ? ' < ' : ' > ') + n.key + ': go ' + (goLeft ? 'left' : 'right') + '.', marks(path, 'hl', n, 'hl2'));
      n = next;
    }
  }
  function find(k, frames) {
    var path = [], n = tree;
    while (n) {
      path.push(n);
      if (k === n.key) { fr(frames, '<b>Found</b> ' + k + ' after ' + path.length + ' comparison' + (path.length > 1 ? 's' : '') + '.', marks(path, 'hl', n, 'ok')); return path; }
      var goLeft = k < n.key;
      fr(frames, k + (goLeft ? ' < ' : ' > ') + n.key + ': go ' + (goLeft ? 'left' : 'right') + '. The whole ' + (goLeft ? 'right' : 'left') + ' subtree of ' + n.key + ' is ruled out.', marks(path, 'hl', n, 'hl2'));
      n = goLeft ? n.left : n.right;
    }
    fr(frames, 'Fell off the tree: ' + k + ' is <b>not present</b>.', marks(path, 'hl'));
    return null;
  }
  function erase(k, frames) {
    var path = find(k, frames);
    if (!path) return;
    var n = path[path.length - 1], parent = path.length > 1 ? path[path.length - 2] : null, o = {};
    if (!n.left || !n.right) {
      var child = n.left || n.right;
      o[n.key] = 'bad'; if (child) o[child.key] = 'hl2';
      fr(frames, n.key + (child ? ' has one child. Its child ' + child.key + ' will take its place.' : ' is a leaf. It can simply be removed.'), o);
      attach(parent, n, child);
      path.pop();
      fr(frames, 'Removed ' + k + '.', child ? marks([], '', child, 'ok') : {});
    } else {
      var sp = [n], s = n.right;
      while (s.left) { sp.push(s); s = s.left; }
      o[n.key] = 'bad'; o[s.key] = 'hl2';
      fr(frames, n.key + ' has two children. Its in-order <b>successor</b> is ' + s.key + ': one step right, then left as far as possible.', o);
      var sparent = sp[sp.length - 1];
      if (sparent === n) n.right = s.right; else sparent.left = s.right;
      n.key = s.key;
      path = path.concat(sp.slice(1));
      fr(frames, 'The successor\'s key moves up to replace ' + k + ', and the successor\'s old node (which had no left child) is unlinked.', marks([], '', n, 'ok'));
    }
    if (AVL && tree) { var before = frames.length; rebalance(path, frames); if (frames.length === before) fr(frames, 'Every node on the path back to the root is still balanced.'); }
  }
  function inorder(frames) {
    var out = [], seen = {};
    (function walk(n) {
      if (!n) return;
      walk(n.left);
      out.push(n.key); seen[n.key] = 'hl';
      var o = Object.assign({}, seen); o[n.key] = 'hl2';
      fr(frames, 'In-order: left subtree, then the node, then the right subtree. Output so far: <b>' + out.join(', ') + '</b>', o);
      walk(n.right);
    })(tree);
    fr(frames, 'The keys came out in <b>sorted order</b>: ' + out.join(', ') + '.', seen);
  }

  function build(keys) { tree = null; var junk = []; keys.forEach(function (k) { insert(k, junk); }); }
  function reset() {
    build(AVL ? [10, 20, 30, 40, 50, 25] : [50, 30, 70, 20, 40, 60, 80, 35]);
    var f = []; fr(f, AVL ? 'An AVL tree. The small number beside each node is its balance factor: left height minus right height. Insert a key and watch for rotations.' : 'A binary search tree. Everything left of a node is smaller; everything right is larger. Try find 35, insert 45, or erase 30.');
    player.load(f, { autoplay: false });
  }
  var input = Viz.input({ value: AVL ? 22 : 45, label: 'key', onenter: function () { run(insert); } });
  function run(op) { var k = Viz.int(input, 0, 99); if (k == null) return; var f = []; op(k, f); player.load(f); }
  ui.top.appendChild(Viz.h('label', null, 'Key ', input));
  ui.top.appendChild(Viz.btn('insert', function () { run(insert); }, 'primary'));
  ui.top.appendChild(Viz.btn('find', function () { run(function (k, f) { find(k, f); }); }));
  ui.top.appendChild(Viz.btn('erase', function () { run(erase); }));
  ui.top.appendChild(Viz.btn('in-order', function () { var f = []; inorder(f); player.load(f); }));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('insert 1–7 in order', function () {
    tree = null; var f = [];
    for (var k = 1; k <= 7; k++) insert(k, f);
    fr(f, AVL ? 'Seven keys inserted in sorted order, and the tree is perfectly balanced: height 3.' : 'Seven keys inserted in sorted order: every node has only a right child. The "tree" is a linked list of height 7.');
    player.load(f);
  }));
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { var f = []; if (AVL) { tree = null; for (var k = 1; k <= 6; k++) insert(k, f); player.load(f, { testAt: f.length - 2 }); } else { find(35, f); player.load(f); } } };
});
