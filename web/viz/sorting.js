// Chapter 8: four sorting algorithms on the same data.
Viz.register('sorting', function (root) {
  'use strict';
  var ui = Viz.shell(root, { height: 240, label: 'Sorting, as bars' });
  var stats = Viz.statsBar(ui);
  var scene = new Viz.Scene(ui.svg, { duration: 180 });
  var N = 14, X0 = 46, BW = 39, BASE = 208, data = [], seed = 3, cmp, mov;

  function draw(arr, hl) {
    hl = hl || {};
    return {
      nodes: arr.map(function (e, i) {
        var hgt = 14 + e.v * 8.5;
        return { id: 'e' + e.id, shape: 'rect', x: X0 + i * BW + BW / 2, y: BASE - hgt / 2, w: BW - 8, h: hgt, label: e.v, size: 11, cls: hl[e.id] || '' };
      }), edges: []
    };
  }
  function st(name) { return '<span><b>' + name + '</b></span><span>comparisons <b>' + cmp + '</b></span><span>element moves <b>' + mov + '</b></span><span>n = ' + N + ', so n² = ' + N * N + ' and n log₂ n ≈ ' + Math.round(N * Math.log2(N)) + '</span>'; }
  function mk(vals) { return vals.map(function (v, i) { return { id: i, v: v }; }); }
  function marks(list, cls, o) { o = o || {}; list.forEach(function (e) { if (e) o[e.id] = cls; }); return o; }

  var ALGOS = {
    insertion: {
      name: 'Insertion sort', run: function (a, F) {
        for (var i = 1; i < a.length; i++) {
          var j = i;
          F(a, marks(a.slice(0, i), 'hl', marks([a[i]], 'hl2')), 'The first ' + i + ' are in order. Slide ' + a[i].v + ' left into place among them.');
          while (j > 0) {
            cmp++;
            if (a[j - 1].v <= a[j].v) break;
            var t = a[j]; a[j] = a[j - 1]; a[j - 1] = t; mov++; j--;
            F(a, marks(a.slice(0, i + 1), 'hl', marks([a[j]], 'hl2')), a[j].v + ' is smaller than ' + a[j + 1].v + ': they swap.');
          }
        }
      }
    },
    heap: {
      name: 'Heapsort', run: function (a, F) {
        var n = a.length;
        function down(i, end) {
          while (true) {
            var l = 2 * i + 1, big = l;
            if (l >= end) return;
            if (l + 1 < end) { cmp++; if (a[l + 1].v > a[l].v) big = l + 1; }
            cmp++;
            if (a[i].v >= a[big].v) return;
            var t = a[i]; a[i] = a[big]; a[big] = t; mov++;
            F(a, marks(a.slice(end), 'ok', marks([a[i], a[big]], 'hl2')), 'Sift down: ' + a[big].v + ' sinks below its larger child ' + a[i].v + '.');
            i = big;
          }
        }
        for (var i = Math.floor(n / 2) - 1; i >= 0; i--) down(i, n);
        F(a, marks([a[0]], 'hl2'), 'The array is now a <b>max-heap</b>: the largest element, ' + a[0].v + ', is at index 0.');
        for (var end = n - 1; end > 0; end--) {
          var t = a[0]; a[0] = a[end]; a[end] = t; mov++;
          F(a, marks(a.slice(end), 'ok'), 'Swap the largest to the back, where it stays. The heap is now one shorter.');
          down(0, end);
        }
      }
    },
    merge: {
      name: 'Mergesort', run: function (a, F) {
        (function sort(lo, hi) {
          if (hi - lo < 2) return;
          var mid = lo + Math.floor((hi - lo) / 2);
          sort(lo, mid); sort(mid, hi);
          F(a, marks(a.slice(lo, mid), 'hl', marks(a.slice(mid, hi), 'hl2')), 'Two sorted runs, positions ' + lo + '–' + (mid - 1) + ' and ' + mid + '–' + (hi - 1) + '. Merge them.');
          var out = [], i = lo, j = mid;
          while (i < mid && j < hi) { cmp++; out.push(a[j].v < a[i].v ? a[j++] : a[i++]); }
          while (i < mid) out.push(a[i++]);
          while (j < hi) out.push(a[j++]);
          for (var k = 0; k < out.length; k++) a[lo + k] = out[k];
          mov += out.length;
          F(a, marks(a.slice(lo, hi), 'ok'), 'Merged: positions ' + lo + '–' + (hi - 1) + ' are now one sorted run of ' + (hi - lo) + '.');
        })(0, a.length);
      }
    },
    quick: {
      name: 'Quicksort', run: function (a, F) {
        var rnd = Viz.rng(seed + 100);
        (function sort(lo, hi) {
          if (hi - lo < 2) return;
          var p = a[lo + Math.floor(rnd() * (hi - lo))], pv = p.v, lt = lo, i = lo, gt = hi, t;
          F(a, marks(a.slice(lo, hi), 'hl', marks([p], 'hl2')), 'Partition positions ' + lo + '–' + (hi - 1) + ' around a randomly chosen pivot, <b>' + pv + '</b>.');
          while (i < gt) {
            cmp++;
            if (a[i].v < pv) { if (i !== lt) { t = a[lt]; a[lt] = a[i]; a[i] = t; mov++; } lt++; i++; }
            else if (a[i].v > pv) { gt--; if (i !== gt) { t = a[i]; a[i] = a[gt]; a[gt] = t; mov++; } }
            else i++;
          }
          F(a, marks(a.slice(lo, lt), 'hl', marks(a.slice(gt, hi), 'vio', marks(a.slice(lt, gt), 'ok'))), 'Partitioned: smaller than ' + pv + ' on the left, larger on the right. The pivot is in its <b>final position</b>.');
          sort(lo, lt); sort(gt, hi);
        })(0, a.length);
      }
    }
  };
  var INPUTS = {
    random: ['Random', function (r) { var v = []; for (var i = 0; i < N; i++) v.push(1 + Math.floor(r() * 20)); return v; }],
    sorted: ['Already sorted', function () { var v = []; for (var i = 0; i < N; i++) v.push(2 + i); return v; }],
    reversed: ['Reversed', function () { var v = []; for (var i = 0; i < N; i++) v.push(N + 2 - i); return v; }],
    few: ['Few distinct values', function (r) { var v = []; for (var i = 0; i < N; i++) v.push([4, 9, 14][Math.floor(r() * 3)]); return v; }]
  };
  var player = new Viz.Player({ bar: ui.bottom, msg: ui.msg, onFrame: function (f, i, anim) { scene.set(f.scene, anim); stats.innerHTML = f.stats; } });
  var algo = Viz.select(Object.keys(ALGOS).map(function (k) { return [k, ALGOS[k].name]; }), 'insertion', function () { show(); });
  var inp = Viz.select(Object.keys(INPUTS).map(function (k) { return [k, INPUTS[k][0]]; }), 'random', function () { fresh(); });
  function fresh() { data = mk(INPUTS[inp.value][1](Viz.rng(seed))); show(); }
  function show() { cmp = 0; mov = 0; player.load([{ scene: draw(data), msg: 'Fourteen values. Choose an algorithm and an input, and sort. Compare the counts on sorted and reversed input.', stats: st(ALGOS[algo.value].name) }], { autoplay: false }); }
  function frames() {
    var a = data.slice(), out = [], A = ALGOS[algo.value];
    cmp = 0; mov = 0;
    function F(arr, hl, msg) { out.push({ scene: draw(arr, hl), msg: msg, stats: st(A.name) }); }
    F(a, {}, A.name + ' begins.');
    A.run(a, F);
    F(a, marks(a, 'ok'), '<b>Sorted</b>, with ' + cmp + ' comparisons and ' + mov + ' moves.');
    return out;
  }
  ui.top.appendChild(algo);
  ui.top.appendChild(inp);
  ui.top.appendChild(Viz.btn('Sort', function () { player.load(frames()); }, 'primary'));
  ui.top.appendChild(Viz.sep());
  ui.top.appendChild(Viz.btn('New data', function () { seed++; fresh(); }));
  fresh();
  return { demo: function () { algo.value = 'quick'; var f = frames(); player.load(f, { testAt: 2 }); } };
});
