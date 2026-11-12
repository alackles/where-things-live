// Chapter 3: a growable array under different growth policies.
Viz.register('dynarray', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 200, label: 'A dynamic array growing' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var MAXN = 32, X0 = 24, W = 18.5, YOLD = 34, YNEW = 118, H = 30;
  var POL = {
    x2: { name: 'double', next: function (c) { return c === 0 ? 1 : 2 * c; } },
    x15: { name: 'grow by half', next: function (c) { return c < 2 ? c + 1 : c + Math.floor(c / 2); } },
    p4: { name: 'grow by 4', next: function (c) { return c + 4; } },
    p1: { name: 'grow by 1', next: function (c) { return c + 1; } }
  };
  var size, cap, copies, pushes, policy = 'x2', history;

  function row(prefix, n, capacity, y, opt) {
    var vals = [];
    for (var i = 0; i < capacity; i++) vals.push(i < n ? i + 1 : '');
    return Viz.cells(prefix, vals, X0, y, W, H, {
      index: false, size: 10,
      cls: function (i) { return i < n ? (opt && opt.hot != null && i === opt.hot ? 'hl2' : (opt && opt.copied && i < opt.copied ? 'hl' : '')) : 'soft'; }
    });
  }
  function scene1(n, capacity, hot) {
    var nodes = row('b', n, capacity, YNEW, { hot: hot });
    nodes.push(Viz.text('cap', X0, YNEW - 14, 'capacity ' + capacity + ', size ' + n, 'left strong'));
    return { nodes: nodes, edges: [] };
  }
  function scene2(n, oldCap, newCap, copied) {
    var nodes = row('o', n, oldCap, YOLD).concat(row('b', copied, newCap, YNEW, { copied: copied }));
    nodes.push(Viz.text('ocap', X0, YOLD - 14, 'old block: capacity ' + oldCap + ' (full)', 'left'));
    nodes.push(Viz.text('cap', X0, YNEW - 14, 'new block: capacity ' + newCap, 'left strong'));
    return { nodes: nodes, edges: [] };
  }
  function showStats() {
    var am = pushes ? ((pushes + copies) / pushes).toFixed(2) : '0';
    stats.innerHTML = 'policy <b>' + POL[policy].name + '</b><span>appends <b>' + pushes + '</b></span><span>elements copied by growth <b>' + copies + '</b></span><span>average cost per append <b>' + am + '</b> writes</span>';
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  function statsHtml() { showStats(); return stats.innerHTML; }

  function push(frames) {
    if (size >= MAXN) { frames.push({ scene: scene1(size, cap), msg: 'This picture holds at most ' + MAXN + ' elements. Reset to go again.', stats: statsHtml() }); return false; }
    if (size === cap) {
      var nc = Math.min(Math.max(POL[policy].next(cap), 1), 64);
      frames.push({ scene: scene2(size, cap, nc, 0), msg: 'The block is full. Allocate a new block of capacity <b>' + nc + '</b>.', stats: statsHtml() });
      copies += size;
      frames.push({ scene: scene2(size, cap, nc, size), msg: 'Copy all <b>' + size + '</b> elements into it. That is the expensive part.', stats: statsHtml() });
      cap = nc;
      frames.push({ scene: scene1(size, cap), msg: 'Delete the old block.', stats: statsHtml() });
    }
    size++; pushes++;
    frames.push({ scene: scene1(size, cap, size - 1), msg: 'Write element ' + size + ' into the first spare slot.' + (size === cap ? ' The block is now full again.' : ''), stats: statsHtml() });
    return true;
  }
  function doPush(k) {
    var frames = [];
    for (var i = 0; i < k; i++) if (!push(frames)) break;
    player.load(frames);
    if (frames.length > 6) player.show(frames.length - 1, false);
  }
  function reset() {
    size = 0; cap = 0; copies = 0; pushes = 0;
    player.load([{ scene: scene1(0, 0), msg: 'An empty vector: no block yet. Append elements and watch what growth costs under each policy.', stats: statsHtml() }], { autoplay: false });
  }
  var sel = Viz.select(Object.keys(POL).map(function (k) { return [k, 'Policy: ' + POL[k].name]; }), 'x2', function (v) { policy = v; reset(); });
  ui.top.appendChild(Viz.btn('push_back', function () { doPush(1); }, 'primary'));
  ui.top.appendChild(Viz.btn('push 8', function () { doPush(8); }));
  ui.top.appendChild(sel);
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { doPush(9); } };
});
