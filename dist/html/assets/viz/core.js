// Shared engine for the interactive figures.
//
//   Viz.register(name, (root, opts) => {...})   one call per widget file
//   Viz.shell(root, {width, height})            standard layout: controls / stage / message / player
//   new Viz.Scene(svg)                          keyed nodes + edges, tweened between frames
//   new Viz.Player({bar, msg, onFrame})         first / back / play / step / last over a list of frames
//
// A widget runs its algorithm to completion, recording a "frame" (a scene
// description plus a sentence of explanation) at each interesting moment.
// The player then lets the reader step through those frames in either
// direction.  Nothing here talks to the network.
(function () {
  'use strict';
  var SVGNS = 'http://www.w3.org/2000/svg';
  var TEST = /viztest/.test(location.hash);
  var reduce = TEST || (window.matchMedia && matchMedia('(prefers-reduced-motion: reduce)').matches);

  function setAttrs(e, attrs) {
    if (!attrs) return;
    Object.keys(attrs).forEach(function (k) {
      var v = attrs[k];
      if (v == null || v === false) return;
      if (k === 'text') e.textContent = v;
      else if (k === 'html') e.innerHTML = v;
      else if (k.slice(0, 2) === 'on') e.addEventListener(k.slice(2), v);
      else e.setAttribute(k, v === true ? '' : v);
    });
  }
  function addKids(e, kids) {
    kids.forEach(function (c) {
      if (c == null || c === false) return;
      if (Array.isArray(c)) addKids(e, c);
      else e.appendChild(typeof c === 'object' ? c : document.createTextNode(String(c)));
    });
  }
  function h(tag, attrs) {
    var e = document.createElement(tag);
    setAttrs(e, attrs);
    addKids(e, Array.prototype.slice.call(arguments, 2));
    return e;
  }
  function s(tag, attrs) {
    var e = document.createElementNS(SVGNS, tag);
    setAttrs(e, attrs);
    addKids(e, Array.prototype.slice.call(arguments, 2));
    return e;
  }

  // ---- Scene ------------------------------------------------------------
  // spec = { nodes: [{id, x, y, shape: 'circle'|'rect'|'text', r, w, h, rx,
  //                   label, sub, subPos: 'below'|'above'|'right'|'left', cls}],
  //          edges: [{from, to, cls, directed, label, bend, fromSide, toSide}] }
  function Scene(svg, opts) {
    opts = opts || {};
    this.svg = svg;
    this.dur = reduce ? 0 : (opts.duration == null ? 320 : opts.duration);
    this.gE = s('g');
    this.gN = s('g');
    svg.appendChild(this.gE);
    svg.appendChild(this.gN);
    this.nodes = new Map();
    this.edges = [];
    this.raf = 0;
  }
  Scene.prototype.set = function (spec, animate) {
    var self = this;
    if (animate === undefined) animate = true;
    var want = new Map();
    (spec.nodes || []).forEach(function (n) { want.set(String(n.id), n); });
    this.nodes.forEach(function (rec, id) {
      if (!want.has(id)) { rec.el.remove(); self.nodes.delete(id); }
    });
    want.forEach(function (n, id) {
      var rec = self.nodes.get(id), fresh = false;
      if (!rec) {
        rec = { el: s('g'), x: n.x, y: n.y };
        self.gN.appendChild(rec.el);
        self.nodes.set(id, rec);
        fresh = true;
      }
      rec.fx = rec.x; rec.fy = rec.y; rec.tx = n.x; rec.ty = n.y; rec.spec = n;
      self.paint(rec, fresh && animate && self.dur > 0);
    });
    this.edges = spec.edges || [];
    cancelAnimationFrame(this.raf);
    var dur = animate ? this.dur : 0;
    if (!dur) {
      this.nodes.forEach(function (r) { r.x = r.tx; r.y = r.ty; });
      this.place();
      return;
    }
    var t0 = performance.now();
    function tick(now) {
      var t = Math.min(1, (now - t0) / dur);
      var e = t < 0.5 ? 2 * t * t : 1 - Math.pow(-2 * t + 2, 2) / 2;
      self.nodes.forEach(function (r) { r.x = r.fx + (r.tx - r.fx) * e; r.y = r.fy + (r.ty - r.fy) * e; });
      self.place();
      if (t < 1) self.raf = requestAnimationFrame(tick);
    }
    this.raf = requestAnimationFrame(tick);
  };
  Scene.prototype.paint = function (rec, entering) {
    var n = rec.spec, g = rec.el;
    var shape = n.shape || 'circle';
    g.setAttribute('class', 'vg ' + (shape === 'text' ? 'lbl ' : '') + (n.cls || '') + (entering ? ' enter' : ''));
    while (g.firstChild) g.removeChild(g.firstChild);
    var hw = 0, hh = 0;
    if (shape === 'circle') {
      var r = n.r || 17;
      hw = hh = r;
      g.appendChild(s('circle', { class: 'vn', r: r }));
    } else if (shape === 'rect') {
      hw = (n.w || 44) / 2; hh = (n.h || 34) / 2;
      g.appendChild(s('rect', { class: 'vn', x: -hw, y: -hh, width: hw * 2, height: hh * 2, rx: n.rx || 0 }));
    }
    if (shape === 'text') { hw = String(n.label == null ? '' : n.label).length * 3.6 + 5; hh = 9; }
    rec.hw = hw; rec.hh = hh; rec.shape = shape;
    if (n.label != null && n.label !== '') {
      var t = s('text', { class: 'vt', text: String(n.label) });
      if (n.size) t.setAttribute('style', 'font-size:' + n.size + 'px');
      g.appendChild(t);
    }
    if (n.sub != null && n.sub !== '') {
      var pos = n.subPos || 'below', sx = 0, sy = 0, anchor = 'middle';
      if (pos === 'below') sy = hh + 11;
      else if (pos === 'above') sy = -hh - 10;
      else if (pos === 'right') { sx = hw + 5; anchor = 'start'; }
      else if (pos === 'left') { sx = -hw - 5; anchor = 'end'; }
      else if (pos === 'ne') { sx = hw * 0.8 + 3; sy = -hh * 0.8 - 5; anchor = 'start'; }
      g.appendChild(s('text', { class: 'vs', x: sx, y: sy, style: 'text-anchor:' + anchor, text: String(n.sub) }));
    }
  };
  function clip(rec, tx, ty, side) {
    var x = rec.x, y = rec.y;
    if (side) {
      if (side === 'l') return [x - rec.hw, y];
      if (side === 'r') return [x + rec.hw, y];
      if (side === 't') return [x, y - rec.hh];
      if (side === 'b') return [x, y + rec.hh];
      return [x, y];
    }
    var dx = tx - x, dy = ty - y, d = Math.sqrt(dx * dx + dy * dy) || 1;
    if (rec.shape === 'circle') return [x + dx / d * rec.hw, y + dy / d * rec.hw];
    if (rec.shape === 'rect' || rec.shape === 'text') {
      var kx = rec.hw / Math.abs(dx || 1e-9), ky = rec.hh / Math.abs(dy || 1e-9), k = Math.min(kx, ky);
      return [x + dx * k, y + dy * k];
    }
    return [x, y];
  }
  Scene.prototype.place = function () {
    var self = this;
    this.nodes.forEach(function (r) { r.el.setAttribute('transform', 'translate(' + r.x.toFixed(1) + ',' + r.y.toFixed(1) + ')'); });
    var gE = this.gE;
    while (gE.firstChild) gE.removeChild(gE.firstChild);
    this.edges.forEach(function (e) {
      var a = self.nodes.get(String(e.from)), b = self.nodes.get(String(e.to));
      if (!a || !b) return;
      var p1 = clip(a, b.x, b.y, e.fromSide), p2 = clip(b, a.x, a.y, e.toSide);
      var mx = (p1[0] + p2[0]) / 2, my = (p1[1] + p2[1]) / 2;
      var dx = p2[0] - p1[0], dy = p2[1] - p1[1], d = Math.sqrt(dx * dx + dy * dy) || 1;
      var cx = mx, cy = my, bent = !!e.bend;
      if (bent) { cx = mx - dy / d * e.bend; cy = my + dx / d * e.bend; }
      // direction at the end, for the arrowhead
      var ex = p2[0] - (bent ? cx : p1[0]), ey = p2[1] - (bent ? cy : p1[1]), ed = Math.sqrt(ex * ex + ey * ey) || 1;
      ex /= ed; ey /= ed;
      var end = p2;
      if (e.directed) end = [p2[0] - ex * 6, p2[1] - ey * 6];
      var dstr = 'M' + p1[0].toFixed(1) + ',' + p1[1].toFixed(1) +
        (bent ? ' Q' + cx.toFixed(1) + ',' + cy.toFixed(1) + ' ' : ' L') + end[0].toFixed(1) + ',' + end[1].toFixed(1);
      gE.appendChild(s('path', { class: 've ' + (e.cls || ''), d: dstr }));
      if (e.directed) {
        var L = 9, W = 3.8, bx = p2[0] - ex * L, by = p2[1] - ey * L;
        gE.appendChild(s('polygon', {
          class: 'vah ' + (e.cls || ''),
          points: p2[0].toFixed(1) + ',' + p2[1].toFixed(1) + ' ' + (bx - ey * W).toFixed(1) + ',' + (by + ex * W).toFixed(1) +
            ' ' + (bx + ey * W).toFixed(1) + ',' + (by - ex * W).toFixed(1)
        }));
      }
      if (e.label != null && e.label !== '') {
        var lx = bent ? (mx + cx) / 2 : mx, ly = bent ? (my + cy) / 2 : my;
        gE.appendChild(s('text', { class: 'vel', x: lx, y: ly, text: String(e.label) }));
      }
    });
  };

  // ---- Player -----------------------------------------------------------
  function Player(o) {
    var self = this;
    this.onFrame = o.onFrame;
    this.msg = o.msg;
    this.frames = [];
    this.i = 0;
    this.timer = 0;
    this.playing = false;
    this.speed = 1;
    this.bFirst = h('button', { type: 'button', title: 'First step', 'aria-label': 'First step', onclick: function () { self.pause(); self.show(0, false); } }, '⏮');
    this.bBack = h('button', { type: 'button', title: 'Step back', 'aria-label': 'Step back', onclick: function () { self.pause(); self.show(self.i - 1, false); } }, '◀');
    this.bPlay = h('button', { type: 'button', class: 'primary', onclick: function () { self.playing ? self.pause() : self.play(); } }, 'Play');
    this.bStep = h('button', { type: 'button', title: 'Step forward', 'aria-label': 'Step forward', onclick: function () { self.pause(); self.show(self.i + 1, true); } }, '▶');
    this.bLast = h('button', { type: 'button', title: 'Last step', 'aria-label': 'Last step', onclick: function () { self.pause(); self.show(self.frames.length - 1, false); } }, '⏭');
    this.count = h('span', { class: 'viz-count' }, '');
    this.sel = h('select', { 'aria-label': 'Playback speed', onchange: function () { self.speed = parseFloat(self.sel.value); } },
      h('option', { value: '0.5' }, '0.5×'), h('option', { value: '1', selected: true }, '1×'),
      h('option', { value: '2' }, '2×'), h('option', { value: '4' }, '4×'));
    addKids(o.bar, [this.bFirst, this.bBack, this.bPlay, this.bStep, this.bLast, this.count, h('span', { class: 'grow' }), h('label', null, 'Speed ', this.sel)]);
    this.update();
  }
  Player.prototype.load = function (frames, opts) {
    opts = opts || {};
    this.pause();
    this.frames = frames;
    if (!frames.length) { this.update(); return; }
    if (TEST) { this.show(opts.testAt != null ? opts.testAt : frames.length - 1, false); return; }
    if (opts.autoplay === false || reduce) { this.show(opts.startAt || 0, false); return; }
    this.show(0, false);
    if (frames.length > 1) this.play();
  };
  Player.prototype.show = function (i, animate) {
    if (!this.frames.length) return;
    this.i = Math.max(0, Math.min(this.frames.length - 1, i));
    var f = this.frames[this.i];
    this.onFrame(f, this.i, animate);
    if (this.msg) this.msg.innerHTML = f.msg || '';
    this.update();
  };
  Player.prototype.update = function () {
    var n = this.frames.length;
    this.bFirst.disabled = this.bBack.disabled = n === 0 || this.i === 0;
    this.bStep.disabled = this.bLast.disabled = n === 0 || this.i >= n - 1;
    this.bPlay.disabled = n < 2 || (this.i >= n - 1 && !this.playing);
    this.bPlay.textContent = this.playing ? 'Pause' : 'Play';
    this.count.textContent = n ? (this.i + 1) + ' / ' + n : '';
  };
  Player.prototype.play = function () {
    var self = this;
    if (this.i >= this.frames.length - 1) return;
    this.playing = true;
    this.update();
    (function next() {
      self.timer = setTimeout(function () {
        self.show(self.i + 1, true);
        if (self.i >= self.frames.length - 1) { self.playing = false; self.update(); }
        else if (self.playing) next();
      }, 950 / self.speed);
    })();
  };
  Player.prototype.pause = function () {
    clearTimeout(this.timer);
    this.playing = false;
    this.update();
  };

  // ---- shell + helpers ----------------------------------------------------
  var Viz = {
    registry: {}, h: h, s: s, Scene: Scene, Player: Player, TEST: TEST, reduce: reduce,
    register: function (name, fn) { this.registry[name] = fn; },
    shell: function (root, o) {
      o = o || {};
      var top = h('div', { class: 'viz-bar' });
      var stage = h('div', { class: 'viz-stage' });
      var svg = s('svg', { class: 'vizsvg', viewBox: '0 0 ' + (o.width || 640) + ' ' + (o.height || 300), role: 'img', 'aria-label': o.label || 'Interactive figure' });
      stage.appendChild(svg);
      var msg = h('div', { class: 'viz-msg', 'aria-live': 'polite' });
      var bottom = h('div', { class: 'viz-bar bottom' });
      addKids(root, [top, stage, msg, bottom]);
      return { top: top, stage: stage, svg: svg, msg: msg, bottom: bottom };
    },
    btn: function (label, fn, cls) { return h('button', { type: 'button', class: cls || '', onclick: fn }, label); },
    input: function (o) {
      o = o || {};
      var e = h('input', { type: 'text', inputmode: o.numeric === false ? null : 'numeric', placeholder: o.placeholder || '', value: o.value == null ? '' : o.value, class: o.cls || '', 'aria-label': o.label || o.placeholder || 'value' });
      if (o.onenter) e.addEventListener('keydown', function (ev) { if (ev.key === 'Enter') o.onenter(); });
      return e;
    },
    select: function (pairs, value, onchange) {
      var e = h('select', { onchange: function () { onchange && onchange(e.value); } });
      pairs.forEach(function (p) { e.appendChild(h('option', { value: p[0], selected: p[0] === value }, p[1])); });
      return e;
    },
    sep: function () { return h('span', { class: 'sep' }); },
    int: function (input, lo, hi) {
      var v = parseInt(String(input.value).trim(), 10);
      if (isNaN(v)) return null;
      if (lo != null && v < lo) v = lo;
      if (hi != null && v > hi) v = hi;
      return v;
    },
    rand: function (lo, hi) { return lo + Math.floor(Math.random() * (hi - lo + 1)); },
    // deterministic generator so the "random" buttons can be replayed in tests
    rng: function (seed) {
      var a = seed >>> 0;
      return function () {
        a = (a + 0x6D2B79F5) >>> 0;
        var t = Math.imul(a ^ (a >>> 15), 1 | a);
        t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
        return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
      };
    },
    // A row of array cells as scene nodes.  o.cls(i, v) -> class; o.index: false to hide indices.
    cells: function (prefix, values, x0, y, w, hgt, o) {
      o = o || {};
      return values.map(function (v, i) {
        return {
          id: prefix + i, shape: 'rect', x: x0 + i * w + w / 2, y: y + hgt / 2, w: w, h: hgt,
          label: v == null ? '' : v, sub: o.index === false ? null : i + (o.indexFrom || 0),
          cls: o.cls ? (o.cls(i, v) || '') : '', size: o.size
        };
      });
    },
    text: function (id, x, y, label, cls) { return { id: id, shape: 'text', x: x, y: y, label: label, cls: cls || '' }; },
    statsBar: function (ui) {
      var d = h('div', { class: 'viz-stats' });
      ui.bottom.parentNode.insertBefore(d, ui.bottom);
      return d;
    },
    // Tidy layout for a rooted tree with any number of children per node.
    // kids(node) -> array of children.  Leaves take consecutive slots; a
    // parent sits midway between its first and last child.
    layoutTree: function (root, kids, o) {
      var pos = new Map(), next = 0, maxDepth = 0;
      (function walk(n, depth) {
        var ks = kids(n) || [], xs;
        if (depth > maxDepth) maxDepth = depth;
        if (!ks.length) { pos.set(n, { slot: next++, depth: depth }); return; }
        ks.forEach(function (k) { walk(k, depth + 1); });
        xs = ks.map(function (k) { return pos.get(k).slot; });
        pos.set(n, { slot: (xs[0] + xs[xs.length - 1]) / 2, depth: depth });
      })(root, 0);
      var slots = Math.max(1, next), step = Math.min(o.maxStep || 60, (o.width - 2 * (o.pad || 24)) / slots);
      var left = (o.x0 || 0) + (o.width - step * slots) / 2 + step / 2;
      var dy = Math.min(o.dy || 56, maxDepth ? (o.height - 2 * (o.padY || 28)) / maxDepth : (o.dy || 56));
      pos.forEach(function (p) { p.x = left + p.slot * step; p.y = (o.y0 || 0) + (o.padY || 28) + p.depth * dy; });
      return pos;
    },
    esc: function (str) { return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); },
    mountAll: function () {
      var reg = this.registry;
      document.querySelectorAll('.viz-widget').forEach(function (root) {
        if (root.dataset.mounted) return;
        var fn = reg[root.dataset.viz];
        if (!fn) return;
        var fb = root.firstElementChild;
        try {
          if (fb) fb.remove();
          root.classList.add('mounted');
          var api = fn(root, root.dataset) || {};
          root.dataset.mounted = '1';
          if (TEST && api.demo) api.demo();
        } catch (err) {
          console.error(err);
          root.textContent = '';
          root.classList.remove('mounted');
          if (fb) root.appendChild(fb);
          root.appendChild(h('div', { class: 'viz-error' }, 'This interactive figure failed to start (' + err.message + '). The static version is shown instead.'));
          document.title = 'VIZERROR ' + root.dataset.viz + ': ' + err.message;
        }
      });
    }
  };
  window.addEventListener('error', function (e) { document.title = 'JSERROR ' + e.message + ' @' + e.lineno; });
  window.Viz = Viz;
})();
