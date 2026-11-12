// Chapter 9: breadth-first search, depth-first search and Dijkstra's
// algorithm on small graphs, with the frontier structure shown at each step.
Viz.register('graph', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 260, label: 'Graph search' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var GRAPHS = {
    roads: { name: 'Road network (weighted)', pos: [[80, 130], [250, 50], [230, 210], [420, 130], [570, 130]], edges: [[0, 1, 4], [0, 2, 1], [2, 1, 2], [1, 3, 5], [2, 3, 8], [3, 4, 3]] },
    example: { name: 'The chapter\'s example (unweighted)', pos: [[70, 130], [190, 60], [190, 200], [320, 130], [450, 130], [570, 130]], edges: [[0, 1, 1], [0, 2, 1], [1, 3, 1], [2, 3, 1], [3, 4, 1], [4, 5, 1]] },
    town: { name: 'A small town (weighted)', pos: [[60, 130], [180, 46], [180, 214], [310, 130], [440, 46], [440, 214], [570, 130]], edges: [[0, 1, 7], [0, 2, 2], [1, 3, 3], [2, 3, 6], [1, 4, 6], [2, 5, 9], [3, 4, 2], [3, 5, 4], [4, 6, 8], [5, 6, 1]] }
  };
  var G, adj, key = 'roads';

  function load(k) {
    key = k; G = GRAPHS[k]; adj = G.pos.map(function () { return []; });
    G.edges.forEach(function (e, i) { adj[e[0]].push({ to: e[1], w: e[2], i: i }); adj[e[1]].push({ to: e[0], w: e[2], i: i }); });
    adj.forEach(function (l) { l.sort(function (a, b) { return a.to - b.to; }); });
  }
  function weighted() { return G.edges.some(function (e) { return e[2] !== 1; }); }
  // vcls: per-vertex class; ecls: per-edge class; sub: per-vertex small label
  function draw(vcls, ecls, sub) {
    vcls = vcls || {}; ecls = ecls || {}; sub = sub || {};
    return {
      nodes: G.pos.map(function (p, v) { return { id: 'v' + v, x: p[0], y: p[1], r: 18, label: v, cls: vcls[v] || '', sub: sub[v], subPos: 'below' }; }),
      edges: G.edges.map(function (e, i) { return { from: 'v' + e[0], to: 'v' + e[1], cls: ecls[i] || '', label: weighted() ? e[2] : '' }; })
    };
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });

  function bfs() {
    var n = G.pos.length, dist = [], q = [0], vc = {}, ec = {}, out = [], sub = {};
    for (var i = 0; i < n; i++) dist.push(-1);
    dist[0] = 0; vc[0] = 'hl2'; sub[0] = 'dist 0';
    function F(msg) { out.push({ scene: draw(Object.assign({}, vc), Object.assign({}, ec), Object.assign({}, sub)), msg: msg, stats: '<span>queue (front first): <b>[' + q.join(', ') + ']</b></span>' }); }
    F('Breadth-first search from vertex 0. The source is discovered, at distance 0, and joins the queue.');
    while (q.length) {
      var u = q.shift(), found = [];
      vc[u] = 'hl';
      adj[u].forEach(function (e) {
        if (dist[e.to] === -1) { dist[e.to] = dist[u] + 1; sub[e.to] = 'dist ' + dist[e.to]; vc[e.to] = 'hl2'; ec[e.i] = 'hl'; q.push(e.to); found.push(e.to); }
      });
      F('Take <b>' + u + '</b> from the front of the queue and look at its neighbours. ' + (found.length ? 'Newly discovered: <b>' + found.join(', ') + '</b>, each one edge further than ' + u + ' (distance ' + (dist[u] + 1) + '). They join the back of the queue.' : 'All of them have been discovered already.'));
      vc[u] = 'ok';
    }
    F('The queue is empty. Every reachable vertex has its distance in edges, and the highlighted edges form a tree of shortest paths.');
    return out;
  }
  function dfs() {
    var n = G.pos.length, seen = [], vc = {}, ec = {}, out = [], stack = [], order = [];
    function F(msg) { out.push({ scene: draw(Object.assign({}, vc), Object.assign({}, ec)), msg: msg, stats: '<span>call stack: <b>' + (stack.length ? stack.join(' → ') : '(empty)') + '</b></span><span>visit order: <b>' + order.join(', ') + '</b></span>' }); }
    (function visit(u, via) {
      seen[u] = true; stack.push(u); order.push(u); vc[u] = 'hl2';
      if (via != null) ec[via] = 'hl';
      F('Visit <b>' + u + '</b> and mark it. Now try each of its neighbours in turn, going as deep as possible before coming back.');
      adj[u].forEach(function (e) {
        if (!seen[e.to]) { vc[u] = 'hl'; visit(e.to, e.i); vc[u] = 'hl2'; }
      });
      vc[u] = 'ok'; stack.pop();
      F('Every neighbour of <b>' + u + '</b> has been visited. ' + (stack.length ? 'Return (backtrack) to ' + stack[stack.length - 1] + '.' : 'The search is complete.'));
    })(0, null);
    return out;
  }
  function dijkstra() {
    var n = G.pos.length, dist = [], pq = [[0, 0]], vc = {}, ec = {}, sub = {}, out = [], via = [];
    for (var i = 0; i < n; i++) { dist.push(Infinity); sub[i] = 'dist ∞'; }
    dist[0] = 0; sub[0] = 'dist 0'; vc[0] = 'hl2';
    function F(msg) { out.push({ scene: draw(Object.assign({}, vc), Object.assign({}, ec), Object.assign({}, sub)), msg: msg, stats: '<span>priority queue (distance, vertex): <b>' + (pq.length ? pq.map(function (p) { return '(' + p[0] + ', ' + p[1] + ')'; }).join(' ') : '(empty)') + '</b></span>' }); }
    F('Dijkstra\'s algorithm from vertex 0. Each vertex shows the best distance found so far. Only the source is known.');
    while (pq.length) {
      pq.sort(function (a, b) { return a[0] - b[0] || a[1] - b[1]; });
      var top = pq.shift(), d = top[0], u = top[1];
      if (d > dist[u]) { F('Pop (' + d + ', ' + u + '): a <b>stale</b> entry. Vertex ' + u + ' was already settled at distance ' + dist[u] + '. Skip it.'); continue; }
      vc[u] = 'hl';
      var notes = [];
      adj[u].forEach(function (e) {
        var c = d + e.w;
        if (c < dist[e.to]) {
          notes.push(e.to + ' to ' + c + (dist[e.to] === Infinity ? '' : ' (was ' + dist[e.to] + ')'));
          if (via[e.to] != null) delete ec[via[e.to]];
          dist[e.to] = c; sub[e.to] = 'dist ' + c; via[e.to] = e.i; ec[e.i] = 'hl'; if (vc[e.to] !== 'ok') vc[e.to] = 'hl2';
          pq.push([c, e.to]);
        }
      });
      F('Pop (' + d + ', ' + u + '): the closest unsettled vertex. Its distance, <b>' + d + '</b>, is final. ' + (notes.length ? 'Relaxing its edges improves ' + notes.join('; ') + '.' : 'None of its edges improves anything.'));
      vc[u] = 'ok';
    }
    F('The priority queue is empty. Every distance is final, and the highlighted edges are the shortest-path tree.');
    return out;
  }
  var ALG = { bfs: ['Breadth-first search', bfs], dfs: ['Depth-first search', dfs], dijkstra: ['Dijkstra', dijkstra] };
  var gsel = Viz.select(Object.keys(GRAPHS).map(function (k) { return [k, GRAPHS[k].name]; }), key, function (v) { load(v); idle(); });
  var asel = Viz.select(Object.keys(ALG).map(function (k) { return [k, ALG[k][0]]; }), 'dijkstra', function () { idle(); });
  function idle() { player.load([{ scene: draw(), msg: 'Choose a graph and an algorithm, then run it from vertex 0. Step through and watch the queue, stack, or priority queue.', stats: '<span>source: vertex <b>0</b></span>' }], { autoplay: false }); }
  ui.top.appendChild(gsel);
  ui.top.appendChild(asel);
  ui.top.appendChild(Viz.btn('Run from vertex 0', function () { player.load(ALG[asel.value][1]()); }, 'primary'));
  load(key); idle();
  return { demo: function () { player.load(dijkstra()); } };
});
