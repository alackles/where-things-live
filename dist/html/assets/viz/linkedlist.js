// Chapter 4: a singly linked list, one pointer assignment at a time.
Viz.register('linkedlist', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 210, label: 'A singly linked list' });
  var scene = new Viz.Scene(ui.svg);
  var list = [], uid = 0, Y = 120, X0 = 110, DX = 84, MAX = 6;

  function nodeAt(n, i, cls, y) { return { id: 'n' + n.id, shape: 'rect', x: X0 + i * DX, y: y == null ? Y : y, w: 52, h: 34, label: n.v, cls: cls || '' }; }
  // order: array of nodes in display slots; next: map id -> id|null (the real links); head: id|null
  function draw(o) {
    var nodes = [], edges = [];
    o.slots.forEach(function (n, i) { if (n) nodes.push(nodeAt(n, i, (o.cls || {})[n.id], (o.ys || {})[n.id])); });
    nodes.push(Viz.text('head', 34, Y, 'head', 'strong'));
    if (o.head != null) edges.push({ from: 'head', to: 'n' + o.head, directed: true, cls: o.headCls || '' });
    Object.keys(o.next).forEach(function (id) {
      if (o.next[id] != null) edges.push({ from: 'n' + id, to: 'n' + o.next[id], directed: true, cls: (o.ecls || {})[id] || '', bend: (o.bend || {})[id] || 0 });
    });
    (o.marks || []).forEach(function (m) {
      nodes.push(Viz.text('m' + m.name, m.x, m.y, m.name, m.cls || 'acc2'));
    });
    return { nodes: nodes, edges: edges };
  }
  function links(arr) {
    var next = {};
    arr.forEach(function (n, i) { next[n.id] = i + 1 < arr.length ? arr[i + 1].id : null; });
    return next;
  }
  function plain(arr, extra) {
    var o = { slots: arr.slice(), next: links(arr), head: arr.length ? arr[0].id : null };
    if (extra) Object.keys(extra).forEach(function (k) { o[k] = extra[k]; });
    return draw(o);
  }
  function xOf(i) { return X0 + i * DX; }

  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); } });

  function pushFront(v) {
    if (list.length >= MAX) return [{ scene: plain(list), msg: 'The picture has room for ' + MAX + ' nodes. Remove one first.' }];
    var n = { id: uid++, v: v }, old = list.slice(), fr = [];
    var slots = [n].concat(old), ys = {}; ys[n.id] = 44;
    var next = links(old); next[n.id] = null;
    var c = {}; c[n.id] = 'hl2';
    fr.push({ scene: draw({ slots: slots, next: next, head: old.length ? old[0].id : null, ys: ys, cls: c }), msg: '<code>new Node{' + v + ', head_}</code>: allocate a node on the heap. Nothing points to it yet.' });
    var next2 = links(old); next2[n.id] = old.length ? old[0].id : null;
    var ec = {}; ec[n.id] = 'hl2';
    fr.push({ scene: draw({ slots: slots, next: next2, head: old.length ? old[0].id : null, ys: ys, cls: c, ecls: ec }), msg: 'Its <code>next</code> is set to the <b>current</b> head. The old first node is now reachable two ways.' });
    list.unshift(n);
    fr.push({ scene: draw({ slots: slots, next: next2, head: n.id, ys: ys, cls: c, headCls: 'hl2' }), msg: 'Only now is <code>head_</code> re-aimed at the new node. Doing these two steps in the other order would lose the whole list.' });
    fr.push({ scene: plain(list), msg: 'Done, in constant time. No existing node moved or changed.' });
    return fr;
  }

  function remove(v) {
    var fr = [], idx = -1, i;
    for (i = 0; i < list.length; i++) {
      var c = {}; c[list[i].id] = 'hl';
      var marks = [{ name: 'cur', x: xOf(i), y: Y + 40 }];
      if (i > 0) marks.push({ name: 'prev', x: xOf(i - 1), y: Y + 40, cls: 'acc' });
      if (list[i].v === v) { idx = i; c[list[i].id] = 'bad'; fr.push({ scene: plain(list, { cls: c, marks: marks }), msg: '<code>cur</code> holds ' + v + '. This is the node to remove' + (i === 0 ? '; it is the first node, so there is no <code>prev</code>.' : ', and <code>prev</code> is the node before it.') }); break; }
      fr.push({ scene: plain(list, { cls: c, marks: marks }), msg: '<code>cur</code> holds ' + list[i].v + ', not ' + v + '. Advance both pointers.' });
    }
    if (idx < 0) { fr.push({ scene: plain(list), msg: '<code>cur</code> ran off the end: ' + v + ' is not in the list. Nothing changes.' }); return fr; }
    var doomed = list[idx], next = links(list), cls = {}, ec = {}, bend = {}, o;
    cls[doomed.id] = 'bad';
    if (idx === 0) {
      o = { slots: list.slice(), next: next, head: next[doomed.id], cls: cls, headCls: 'ok' };
      fr.push({ scene: draw(o), msg: 'Removing the first node: <code>head_ = cur->next</code>. The head now skips it.' });
    } else {
      next[list[idx - 1].id] = next[doomed.id];
      ec[list[idx - 1].id] = 'ok'; bend[list[idx - 1].id] = -46;
      o = { slots: list.slice(), next: next, head: list[0].id, cls: cls, ecls: ec, bend: bend };
      fr.push({ scene: draw(o), msg: '<code>prev->next = cur->next</code>: one assignment routes the list around the node.' });
    }
    list.splice(idx, 1);
    fr.push({ scene: plain(list), msg: '<code>delete cur</code>. The node\'s memory is returned, and the rest of the list never noticed.' });
    return fr;
  }

  function reverse() {
    var fr = [], n = list.length, slots = list.slice(), next = links(list), k;
    if (n < 2) return [{ scene: plain(list), msg: 'A list of fewer than two nodes is its own reverse.' }];
    function frame(done, cur, msg, hot) {
      var marks = [], cls = {}, ec = {};
      if (done != null) marks.push({ name: 'done', x: xOf(done), y: Y - 38, cls: 'acc' });
      if (cur != null) { marks.push({ name: 'cur', x: xOf(cur), y: Y + 40 }); cls[slots[cur].id] = 'hl2'; }
      if (hot != null) ec[hot] = 'ok';
      var nx = {}; Object.keys(next).forEach(function (id) { nx[id] = next[id]; });
      var bend = {};
      Object.keys(nx).forEach(function (id) { if (nx[id] != null) { var a = slots.findIndex(function (s) { return s.id == id; }), b = slots.findIndex(function (s) { return s.id == nx[id]; }); if (b < a) bend[id] = 14; } });
      fr.push({ scene: draw({ slots: slots, next: nx, head: slots[0].id, cls: cls, ecls: ec, marks: marks, bend: bend }), msg: msg });
    }
    frame(null, 0, '<code>done</code> is null (nothing reversed yet); <code>cur</code> is the first node.');
    for (k = 0; k < n; k++) {
      next[slots[k].id] = k > 0 ? slots[k - 1].id : null;
      frame(k > 0 ? k - 1 : null, k, 'Save <code>rest = cur->next</code>, then flip: <code>cur->next = done</code>.' + (k === 0 ? ' The old first node will be the last, so its next becomes null.' : ''), slots[k].id);
      if (k + 1 < n) frame(k, k + 1, 'Advance: <code>done = cur; cur = rest</code>.');
    }
    list.reverse();
    var cls2 = {};
    fr.push({ scene: draw({ slots: slots, next: links(list), head: list[0].id, headCls: 'ok', bend: (function () { var b = {}; list.forEach(function (x) { b[x.id] = 14; }); return b; })() }), msg: '<code>cur</code> is null: every arrow has been turned around. Finally <code>head_ = done</code>.' });
    fr.push({ scene: plain(list), msg: 'The same nodes, at the same addresses, in the opposite order. Nothing was allocated or copied.' });
    return fr;
  }

  var input = Viz.input({ value: 5, label: 'value', onenter: function () { doPush(); } });
  function val() { var v = Viz.int(input, -99, 999); return v == null ? 1 : v; }
  function doPush() { player.load(pushFront(val())); input.value = Viz.rand(1, 60); }
  ui.top.appendChild(Viz.h('label', null, 'Value ', input));
  ui.top.appendChild(Viz.btn('push_front', doPush, 'primary'));
  ui.top.appendChild(Viz.btn('remove', function () { player.load(remove(val())); }));
  ui.top.appendChild(Viz.btn('reverse', function () { player.load(reverse()); }));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('Reset', reset));
  function reset() {
    list = [{ id: uid++, v: 4 }, { id: uid++, v: 1 }, { id: uid++, v: 3 }];
    player.load([{ scene: plain(list), msg: 'Three nodes. <code>head</code> points to the first; each node points to the next; the last points nowhere.' }], { autoplay: false });
  }
  reset();
  return { demo: function () { var f = remove(1); player.load(f, { testAt: f.length - 2 }); } };
});
