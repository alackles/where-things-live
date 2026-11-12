// Chapter 2: binary search (and linear search) on a sorted array.
Viz.register('bsearch', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 190, label: 'Searching a sorted array' });
  var scene = new Viz.Scene(ui.svg);
  var vals = [4, 9, 13, 18, 22, 27, 31, 36, 40, 45, 51, 57, 63, 68, 74];
  var X0 = 32, W = 38, Y = 62, H = 36;
  var seed = 7;

  function draw(st) {
    var nodes = Viz.cells('c', vals, X0, Y, W, H, {
      cls: function (i) {
        if (i === st.found) return 'ok';
        if (i === st.mid) return 'hl2';
        if (st.lo != null && i >= st.lo && i < st.hi) return 'hl';
        return st.lo != null ? 'dim' : '';
      }
    });
    function mark(id, i, label, cls, above) {
      var x = X0 + i * W + W / 2;
      nodes.push(Viz.text(id, x, above ? Y - 30 : Y + H + 40, label, cls + ' strong'));
      nodes.push({ id: id + 't', shape: 'text', x: x, y: above ? Y - 4 : Y + H + 16, label: '' });
    }
    var edges = [];
    if (st.lo != null) {
      mark('lo', st.lo, 'lo', 'acc'); mark('hi', st.hi, 'hi', 'acc');
      edges.push({ from: 'lo', to: 'lot', directed: true, cls: 'hl' }, { from: 'hi', to: 'hit', directed: true, cls: 'hl' });
    }
    if (st.mid != null) { mark('mid', st.mid, st.linear ? 'i' : 'mid', 'acc2', true); edges.push({ from: 'mid', to: 'midt', directed: true, cls: 'hl2' }); }
    nodes.push({ id: 'end', shape: 'rect', x: X0 + vals.length * W + W / 2, y: Y + H / 2, w: W, h: H, cls: 'dim', sub: vals.length });
    return { nodes: nodes, edges: edges };
  }

  function binary(target) {
    var fr = [], lo = 0, hi = vals.length, cmp = 0;
    fr.push({ st: { lo: lo, hi: hi }, msg: 'Searching for <b>' + target + '</b>. Invariant: if it is here, it is in <code>[lo, hi)</code> = [' + lo + ', ' + hi + '), the shaded cells.' });
    while (lo < hi) {
      var mid = lo + Math.floor((hi - lo) / 2);
      cmp++;
      if (vals[mid] === target) {
        fr.push({ st: { lo: lo, hi: hi, mid: mid, found: mid }, msg: '<code>v[' + mid + ']</code> = ' + vals[mid] + ' is the target. <b>Found</b> after ' + cmp + ' comparison' + (cmp > 1 ? 's' : '') + '.' });
        return fr;
      }
      var less = vals[mid] < target;
      fr.push({ st: { lo: lo, hi: hi, mid: mid }, msg: 'Comparison ' + cmp + ': <code>mid</code> = ' + mid + ', and <code>v[mid]</code> = ' + vals[mid] + (less ? ' is too small. Everything up to and including mid is ruled out: <code>lo = mid + 1</code>.' : ' is too big. Mid and everything after it is ruled out: <code>hi = mid</code>.') });
      if (less) lo = mid + 1; else hi = mid;
      fr.push({ st: { lo: lo, hi: hi }, msg: 'Now <code>[lo, hi)</code> = [' + lo + ', ' + hi + '): ' + (hi - lo) + ' candidate' + (hi - lo === 1 ? '' : 's') + ' left.' });
    }
    fr.push({ st: { lo: lo, hi: hi }, msg: 'The range is empty (<code>lo == hi</code>), so <b>' + target + ' is not present</b>. That took ' + cmp + ' comparisons, for ' + vals.length + ' elements.' });
    return fr;
  }
  function linear(target) {
    var fr = [{ st: {}, msg: 'Linear search for <b>' + target + '</b>: look at every element in turn.' }];
    for (var i = 0; i < vals.length; i++) {
      if (vals[i] === target) { fr.push({ st: { mid: i, found: i, linear: true }, msg: '<code>v[' + i + ']</code> = ' + vals[i] + '. <b>Found</b> after ' + (i + 1) + ' comparisons.' }); return fr; }
      fr.push({ st: { mid: i, linear: true }, msg: 'Comparison ' + (i + 1) + ': <code>v[' + i + ']</code> = ' + vals[i] + ' is not it.' });
    }
    fr.push({ st: {}, msg: 'Reached the end: <b>not present</b>, after ' + vals.length + ' comparisons.' });
    return fr;
  }

  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(draw(f.st), anim); } });
  var input = Viz.input({ value: 57, label: 'target', onenter: go });
  var mode = Viz.select([['binary', 'Binary search'], ['linear', 'Linear search']], 'binary');
  function go() {
    var t = Viz.int(input, -999, 999);
    if (t == null) return;
    player.load(mode.value === 'binary' ? binary(t) : linear(t));
  }
  function fresh() {
    var rnd = Viz.rng(seed++), v = 2 + Math.floor(rnd() * 6);
    vals = [];
    for (var i = 0; i < 15; i++) { vals.push(v); v += 1 + Math.floor(rnd() * 8); }
    input.value = vals[Math.floor(rnd() * 15)];
    player.load([{ st: {}, msg: 'A new sorted array. Choose a target and search.' }], { autoplay: false });
  }
  ui.top.appendChild(Viz.h('label', null, 'Target ', input));
  ui.top.appendChild(mode);
  ui.top.appendChild(Viz.btn('Search', go, 'primary'));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('New array', fresh));
  player.load([{ st: {}, msg: 'Fifteen sorted values. Choose a target (try one that is absent, such as 50) and search.' }], { autoplay: false });
  return { demo: function () { input.value = 63; player.load(binary(63), { testAt: 3 }); } };
});
