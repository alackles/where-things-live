// Chapter 4: a queue in a circular buffer.
Viz.register('ring', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 250, label: 'A ring buffer queue' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg);
  var CAP = 8, buf, head, size, counter;
  var CX = 200, CY = 124, RAD = 82;

  function slotPos(i, r) {
    var a = -Math.PI / 2 + (i + 0.5) * 2 * Math.PI / CAP;
    return [CX + r * Math.cos(a), CY + r * Math.sin(a)];
  }
  function draw(hot) {
    var nodes = [], edges = [], i, p;
    for (i = 0; i < CAP; i++) {
      p = slotPos(i, RAD);
      var used = ((i - head + CAP) % CAP) < size;
      nodes.push({ id: 's' + i, shape: 'circle', r: 24, x: p[0], y: p[1], label: used ? buf[i] : '', cls: i === hot ? 'hl2' : used ? 'hl' : 'soft' });
      p = slotPos(i, RAD + 38);
      nodes.push(Viz.text('i' + i, p[0], p[1], i, ''));
    }
    p = slotPos(head, RAD - 46);
    nodes.push(Viz.text('hd', p[0], p[1], 'head', 'acc2'));
    if (size < CAP) { p = slotPos((head + size) % CAP, RAD - 46); nodes.push(Viz.text('bk', p[0], p[1], size ? 'back' : '', 'acc')); }
    // the same queue, drawn straight, in the block
    var vals = [];
    for (i = 0; i < CAP; i++) vals.push(((i - head + CAP) % CAP) < size ? buf[i] : '');
    Viz.cells('a', vals, 376, 96, 30, 30, { size: 12, cls: function (k) { return k === hot ? 'hl2' : vals[k] !== '' ? 'hl' : 'soft'; } }).forEach(function (n) { nodes.push(n); });
    nodes.push(Viz.text('lbl', 496, 78, 'the block in memory', ''));
    nodes.push(Viz.text('q', 496, 176, 'queue order: ' + (size ? order().join('  ') : '(empty)'), 'strong'));
    return { nodes: nodes, edges: edges };
  }
  function order() { var o = []; for (var k = 0; k < size; k++) o.push(buf[(head + k) % CAP]); return o; }
  function showStats() {
    stats.innerHTML = '<span>head <b>' + head + '</b></span><span>size <b>' + size + '</b></span><span>next push goes to (head + size) % ' + CAP + ' = <b>' + ((head + size) % CAP) + '</b></span>';
  }
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  function snap(msg, hot) { showStats(); return { scene: draw(hot), msg: msg, stats: stats.innerHTML }; }

  function push() {
    if (size === CAP) { player.load([snap('The buffer is <b>full</b>: size equals capacity. A real queue would now move everything into a block twice as large, starting from <code>head</code> so that the queue is unwrapped.')]); return; }
    var at = (head + size) % CAP, v = counter++;
    buf[at] = v; size++;
    player.load([snap('push ' + v + ': written at index <b>' + at + '</b>' + (at < head ? ', which has <b>wrapped around</b> past the end of the block.' : '.'), at)]);
  }
  function pop() {
    if (size === 0) { player.load([snap('The queue is empty.')]); return; }
    var v = buf[head], old = head;
    head = (head + 1) % CAP; size--;
    player.load([snap('pop: removed ' + v + ' from the front. Nothing shifts; <code>head</code> simply advances from ' + old + ' to <b>' + head + '</b>' + (head === 0 ? ', wrapping around.' : '.'))]);
  }
  function reset() {
    buf = new Array(CAP); head = 0; size = 0; counter = 10;
    player.load([snap('An empty queue in a block of ' + CAP + ' slots, drawn as a ring. Push a few values, pop a few, and keep going until the contents wrap around.')], { autoplay: false });
  }
  ui.top.appendChild(Viz.btn('push', push, 'primary'));
  ui.top.appendChild(Viz.btn('pop', pop));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', reset));
  reset();
  return { demo: function () { for (var i = 0; i < 8; i++) push(); for (i = 0; i < 5; i++) pop(); push(); push(); } };
});
