// Chapter 4: why pointer-chasing misses the cache.  A toy memory of 16
// cache lines, 8 cells each, and a cache that holds 4 lines.
Viz.register('cachesim', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 250, label: 'A simulated cache' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg, { duration: 0 });
  var LINES = 16, PER = 8, N = 16, CACHE = 4, COLS = 2;
  var W = 30, H = 20, GX = 40, X0 = 46, Y0 = 26;
  var listPos, rnd = Viz.rng(11);

  function shuffle() {
    var cells = [];
    for (var i = 0; i < LINES * PER; i++) cells.push(i);
    for (i = cells.length - 1; i > 0; i--) { var j = Math.floor(rnd() * (i + 1)), t = cells[i]; cells[i] = cells[j]; cells[j] = t; }
    listPos = cells.slice(0, N);
  }
  function xy(cell) {
    var line = Math.floor(cell / PER), k = cell % PER, col = Math.floor(line / (LINES / COLS)), rowi = line % (LINES / COLS);
    return [X0 + col * (PER * W + GX) + k * W + W / 2, Y0 + rowi * (H + 6) + H / 2];
  }
  function draw(layout, cur, cached, visited, missed) {
    var nodes = [], where = {};
    layout.forEach(function (c, i) { where[c] = i + 1; });
    for (var line = 0; line < LINES; line++) {
      var a = xy(line * PER), b = xy(line * PER + PER - 1);
      nodes.push({ id: 'L' + line, shape: 'rect', x: (a[0] + b[0]) / 2, y: a[1], w: PER * W + 6, h: H + 6, rx: 3, cls: cached.indexOf(line) >= 0 ? 'ok' : 'ghost' });
    }
    for (var c = 0; c < LINES * PER; c++) {
      var p = xy(c), has = where[c];
      nodes.push({ id: 'c' + c, shape: 'rect', x: p[0], y: p[1], w: W, h: H, label: has || '', size: 10,
        cls: c === cur ? (missed ? 'bad' : 'hl2') : has ? (visited[c] ? 'hl' : 'soft') : '' });
    }
    return { nodes: nodes, edges: [] };
  }
  function run(layout, name) {
    var cached = [], hits = 0, misses = 0, visited = {}, fr = [];
    function st() { return '<span>accesses <b>' + (hits + misses) + '</b></span><span>cache hits <b>' + hits + '</b></span><span>cache misses <b>' + misses + '</b></span><span>cache holds <b>' + CACHE + '</b> of ' + LINES + ' lines</span>'; }
    fr.push({ scene: draw(layout, -1, [], {}, false), msg: 'The ' + name + ': ' + N + ' elements, numbered in the order they will be visited. Each row segment is one cache line. The cache starts empty.', stats: st() });
    layout.forEach(function (cell, i) {
      var line = Math.floor(cell / PER), at = cached.indexOf(line), miss = at < 0;
      if (miss) { misses++; cached.push(line); if (cached.length > CACHE) cached.shift(); }
      else { hits++; cached.splice(at, 1); cached.push(line); }
      visited[cell] = true;
      fr.push({ scene: draw(layout, cell, cached.slice(), Object.assign({}, visited), miss), stats: st(),
        msg: 'Element ' + (i + 1) + ' is on line ' + line + ': ' + (miss ? '<b>miss</b>. The whole line is fetched from main memory' + (cached.length === CACHE && misses > CACHE ? ', evicting the least recently used line.' : '.') : 'a <b>hit</b>. That line is already in the cache.') });
    });
    fr.push({ scene: draw(layout, -1, cached.slice(), visited, false), stats: st(), msg: 'Done: <b>' + misses + ' misses</b> in ' + N + ' accesses. ' + (misses <= 3 ? 'Contiguous elements share cache lines, so most accesses are free.' : 'Scattered nodes each sit on their own line, so nearly every access pays full price.') });
    return fr;
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, false); stats.innerHTML = f.stats; } });
  function vec() { var v = []; for (var i = 0; i < N; i++) v.push(i); return v; }
  ui.top.appendChild(Viz.btn('Traverse a vector', function () { player.load(run(vec(), 'vector')); }, 'primary'));
  ui.top.appendChild(Viz.btn('Traverse a linked list', function () { player.load(run(listPos, 'linked list')); }, 'primary'));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Scatter the list again', function () { shuffle(); player.load(run(listPos, 'linked list'), { autoplay: false }); }));
  shuffle();
  player.load(run(listPos, 'linked list'), { autoplay: false });
  return { demo: function () { var f = run(listPos, 'linked list'); player.load(f, { testAt: 9 }); } };
});
