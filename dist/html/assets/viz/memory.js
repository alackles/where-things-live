// Chapter 1: the stack and the heap, stepped through four small programs.
// The traces are scripted (this is not a C++ interpreter), but every state
// shown is what the program really does.
Viz.register('memory', function (root) {
  'use strict';
  var h = Viz.h, s = Viz.s;

  // A state: {frames: [{name, vars: [{n, v, to, ref}]}], heap: [{id, cells, st, note}]}
  function F(name, vars) { return { name: name, vars: vars }; }
  function V(n, v, to, ref) { return { n: n, v: v, to: to || null, ref: !!ref }; }
  function H(id, cells, st, note) { return { id: id, cells: cells, st: st || 'live', note: note || '' }; }

  var PROGRAMS = {
    copies: {
      title: 'Copies and references',
      code: ['void bump(int a, int& b) {', '    a += 1;', '    b += 1;', '}', '', 'int main() {', '    int x = 10;', '    int y = 20;', '    bump(x, y);', '}'],
      steps: [
        { line: 6, msg: '<code>main</code> begins. A stack frame is created for it.', frames: [F('main', [])], heap: [] },
        { line: 7, msg: '<code>x</code> is an <code>int</code> in main\'s frame.', frames: [F('main', [V('x', 10)])], heap: [] },
        { line: 8, msg: '<code>y</code> sits right beside it.', frames: [F('main', [V('x', 10), V('y', 20)])], heap: [] },
        { line: 1, msg: 'Calling <code>bump</code> pushes a new frame. <code>a</code> is passed <b>by value</b>: a fresh int holding a copy of 10. <code>b</code> is a <b>reference</b>: another name for main\'s <code>y</code>.', frames: [F('main', [V('x', 10), V('y', 20)]), F('bump', [V('a', 10), V('b', '', 'main.y', true)])], heap: [] },
        { line: 2, msg: '<code>a += 1</code> changes bump\'s own copy. main\'s <code>x</code> is untouched.', frames: [F('main', [V('x', 10), V('y', 20)]), F('bump', [V('a', 11), V('b', '', 'main.y', true)])], heap: [], hot: ['bump.a'] },
        { line: 3, msg: '<code>b += 1</code> goes through the reference and changes main\'s <code>y</code>.', frames: [F('main', [V('x', 10), V('y', 21)]), F('bump', [V('a', 11), V('b', '', 'main.y', true)])], heap: [], hot: ['main.y'] },
        { line: 9, msg: '<code>bump</code> returns. Its frame, and the copy <code>a</code>, are gone. <code>x</code> is still 10; <code>y</code> is 21.', frames: [F('main', [V('x', 10), V('y', 21)])], heap: [] }
      ]
    },
    heap: {
      title: 'An array on the heap',
      code: ['int* make_array(int n, int fill) {', '    int* block = new int[n];', '    for (int i = 0; i < n; ++i)', '        block[i] = fill;', '    return block;', '}', '', 'int main() {', '    int* many = make_array(4, 7);', '    many[0] = 9;', '    delete[] many;', '}'],
      steps: [
        { line: 9, msg: '<code>main</code> has one local: a pointer, not yet given a value.', frames: [F('main', [V('many', '?')])], heap: [] },
        { line: 1, msg: 'The call pushes a frame for <code>make_array</code> with its two parameters.', frames: [F('main', [V('many', '?')]), F('make_array', [V('n', 4), V('fill', 7)])], heap: [] },
        { line: 2, msg: '<code>new int[4]</code> claims a block of four ints on the <b>heap</b>. The pointer <code>block</code> is a local variable on the stack, holding the block\'s address.', frames: [F('main', [V('many', '?')]), F('make_array', [V('n', 4), V('fill', 7), V('block', '', 'A')])], heap: [H('A', ['?', '?', '?', '?'])] },
        { line: 4, msg: 'The loop fills the block through the pointer.', frames: [F('main', [V('many', '?')]), F('make_array', [V('n', 4), V('fill', 7), V('block', '', 'A')])], heap: [H('A', [7, 7, 7, 7])] },
        { line: 9, msg: 'The function returns the <b>address</b>. Its frame is destroyed, including the variable <code>block</code>, but the heap block lives on, now owned by <code>many</code>.', frames: [F('main', [V('many', '', 'A')])], heap: [H('A', [7, 7, 7, 7])] },
        { line: 10, msg: '<code>many[0] = 9</code> writes into the heap block.', frames: [F('main', [V('many', '', 'A')])], heap: [H('A', [9, 7, 7, 7])] },
        { line: 11, msg: '<code>delete[]</code> gives the block back. <code>many</code> still holds the old address, but must not be used again.', frames: [F('main', [V('many', '', 'A')])], heap: [H('A', [9, 7, 7, 7], 'freed', 'freed')] }
      ]
    },
    leak: {
      title: 'A leak',
      code: ['int main() {', '    int* p = new int{1};', '    p = new int{2};', '    delete p;', '}'],
      steps: [
        { line: 2, msg: '<code>p</code> points to a new int on the heap.', frames: [F('main', [V('p', '', 'A')])], heap: [H('A', [1])] },
        { line: 3, msg: '<code>p</code> is re-aimed at a second int. Nothing points to the first one any more. It can never be deleted: a <b>leak</b>.', frames: [F('main', [V('p', '', 'B')])], heap: [H('A', [1], 'leaked', 'unreachable'), H('B', [2])] },
        { line: 4, msg: 'The second int is deleted properly. The first is still allocated.', frames: [F('main', [V('p', '', 'B')])], heap: [H('A', [1], 'leaked', 'leaked'), H('B', [2], 'freed', 'freed')] },
        { line: 5, msg: 'The program ends with four bytes never returned. LeakSanitizer would report exactly this.', frames: [], heap: [H('A', [1], 'leaked', 'leaked')] }
      ]
    },
    dangling: {
      title: 'A dangling pointer',
      code: ['int main() {', '    int* p = new int{42};', '    int* q = p;', '    delete p;', '    *q = 7;      // undefined behavior', '}'],
      steps: [
        { line: 2, msg: '<code>p</code> points to a new int.', frames: [F('main', [V('p', '', 'A')])], heap: [H('A', [42])] },
        { line: 3, msg: 'Copying a pointer copies the <b>address</b>. Now two pointers refer to one int.', frames: [F('main', [V('p', '', 'A'), V('q', '', 'A')])], heap: [H('A', [42])] },
        { line: 4, msg: 'The int is deleted through <code>p</code>. Both <code>p</code> and <code>q</code> still hold its old address. They <b>dangle</b>.', frames: [F('main', [V('p', '', 'A'), V('q', '', 'A')])], heap: [H('A', [42], 'freed', 'freed')] },
        { line: 5, msg: 'Writing through <code>q</code> is a <b>use after free</b>. The memory may already belong to something else. AddressSanitizer stops the program here.', frames: [F('main', [V('p', '', 'A'), V('q', '', 'A')])], heap: [H('A', ['?'], 'bad', 'use after free')], hot: ['main.q'] }
      ]
    }
  };

  var top = h('div', { class: 'viz-bar' });
  var split = h('div', { class: 'viz-split' });
  var codeEl = h('pre', { class: 'viz-code' });
  var stage = h('div', { class: 'viz-stage' });
  var svg = s('svg', { class: 'vizsvg', viewBox: '0 0 400 300', role: 'img', 'aria-label': 'Stack frames and heap blocks' });
  stage.appendChild(svg);
  split.appendChild(codeEl); split.appendChild(stage);
  var msg = h('div', { class: 'viz-msg', 'aria-live': 'polite' });
  var bottom = h('div', { class: 'viz-bar bottom' });
  [top, split, msg, bottom].forEach(function (e) { root.appendChild(e); });
  var scene = new Viz.Scene(svg);

  function build(st) {
    var nodes = [], edges = [], y = 30, where = {};
    nodes.push(Viz.text('Lstack', 100, 12, 'Stack', 'strong'));
    nodes.push(Viz.text('Lheap', 300, 12, 'Heap', 'strong'));
    st.frames.forEach(function (fr) {
      var hgt = 24 + Math.max(1, fr.vars.length) * 30;
      nodes.push({ id: 'f:' + fr.name, shape: 'rect', x: 100, y: y + hgt / 2, w: 180, h: hgt, rx: 6, cls: 'soft' });
      nodes.push(Viz.text('ft:' + fr.name, 22, y + 11, fr.name, 'left strong'));
      fr.vars.forEach(function (v, i) {
        var id = fr.name + '.' + v.n, vy = y + 24 + i * 30 + 12;
        where[id] = true;
        nodes.push(Viz.text('n:' + id, 84, vy, v.n, 'right strong'));
        nodes.push({ id: id, shape: 'rect', x: 136, y: vy, w: 76, h: 24, label: v.to ? (v.ref ? '' : '●') : v.v, size: v.to ? 9 : null, cls: (st.hot || []).indexOf(id) >= 0 ? 'hl2' : '' });
        if (v.to) edges.push({ from: id, to: v.to.indexOf('.') > 0 ? v.to : 'h:' + v.to + ':0', directed: true, cls: v.ref ? 'hl' : '', fromSide: 'r', toSide: v.ref ? 'r' : 'l', bend: v.ref ? -34 : 0, label: v.ref ? 'ref' : '' });
      });
      y += hgt + 10;
    });
    var hy = 50;
    st.heap.forEach(function (b) {
      var cls = b.st === 'freed' ? 'dim' : b.st === 'leaked' ? 'bad' : b.st === 'bad' ? 'bad' : 'hl';
      b.cells.forEach(function (c, i) {
        nodes.push({ id: 'h:' + b.id + ':' + i, shape: 'rect', x: 270 + i * 34, y: hy + 14, w: 34, h: 28, label: c, cls: cls });
      });
      if (b.note) nodes.push(Viz.text('hn:' + b.id, 270 + b.cells.length * 17 - 17, hy + 40, b.note, b.st === 'freed' ? '' : 'acc2'));
      hy += 70;
    });
    return { nodes: nodes, edges: edges };
  }

  var player = new Viz.Player({
    bar: bottom, msg: msg,
    onFrame: function (f, i, anim) {
      scene.set(build(f), anim);
      Array.prototype.forEach.call(codeEl.children, function (ln, k) { ln.classList.toggle('on', k + 1 === f.line); });
    }
  });

  function load(key) {
    var p = PROGRAMS[key];
    codeEl.textContent = '';
    p.code.forEach(function (line) { codeEl.appendChild(h('span', { class: 'ln' }, line || ' ')); });
    player.load(p.steps, { autoplay: false });
  }
  var sel = Viz.select(Object.keys(PROGRAMS).map(function (k) { return [k, PROGRAMS[k].title]; }), 'copies', load);
  top.appendChild(h('label', null, 'Program ', sel));
  top.appendChild(h('span', { class: 'grow' }));
  top.appendChild(h('span', { style: 'color:var(--muted)' }, 'Step through with ▶'));
  load('copies');
  return { demo: function () { sel.value = 'heap'; load('heap'); player.show(3, false); } };
});
