// Chapter 2: growth rates.  Drag the scale and watch the curves separate.
Viz.register('growth', function (root) {
  'use strict';
  var h = Viz.h, s = Viz.s;
  var W = 640, H = 300, L = 56, R = 540, T = 20, B = 262;
  var top = h('div', { class: 'viz-bar' });
  var stage = h('div', { class: 'viz-stage' });
  var svg = s('svg', { class: 'vizsvg figsvg', viewBox: '0 0 ' + W + ' ' + H, role: 'img', 'aria-label': 'Plot of five growth rates' });
  stage.appendChild(svg);
  var table = h('div', { class: 'viz-msg' });
  [top, stage, table].forEach(function (e) { root.appendChild(e); });

  var FNS = [
    { name: 'log₂ n', cls: 'good', f: function (n) { return n >= 1 ? Math.log2(n) : 0; } },
    { name: 'n', cls: 'accent', f: function (n) { return n; } },
    { name: 'n log₂ n', cls: 'violet', f: function (n) { return n > 1 ? n * Math.log2(n) : 0; } },
    { name: 'n²', cls: 'accent2', f: function (n) { return n * n; } },
    { name: '2ⁿ', cls: 'bad', f: function (n) { return Math.pow(2, n); } }
  ];
  var slider = h('input', { type: 'range', min: '0', max: '100', value: '28', 'aria-label': 'Largest n shown', style: 'width:14rem' });
  var readout = h('b', null, '');
  top.appendChild(h('label', null, 'Largest n shown ', slider));
  top.appendChild(readout);
  top.appendChild(h('span', { class: 'grow' }));
  top.appendChild(h('span', { style: 'color:var(--muted)' }, 'vertical axis always runs to n²'));

  function fmtCount(v) {
    if (!isFinite(v) || v > 1e300) return 'more than 10³⁰⁰';
    if (v < 1000) return String(Math.round(v * 10) / 10);
    if (v < 1e15) return Math.round(v).toLocaleString('en-US');
    var e = Math.floor(Math.log10(v));
    return (v / Math.pow(10, e)).toFixed(1) + ' × 10^' + e;
  }
  function fmtTime(steps) {
    var sec = steps / 1e9;
    if (!isFinite(sec) || sec > 1e290) return 'longer than you have';
    if (sec < 1e-6) return Math.max(1, Math.round(sec * 1e9)) + ' ns';
    if (sec < 1e-3) return (sec * 1e6).toFixed(1) + ' μs';
    if (sec < 1) return (sec * 1e3).toFixed(1) + ' ms';
    if (sec < 120) return sec.toFixed(1) + ' s';
    if (sec < 7200) return (sec / 60).toFixed(1) + ' minutes';
    if (sec < 172800) return (sec / 3600).toFixed(1) + ' hours';
    if (sec < 3.156e7 * 2) return (sec / 86400).toFixed(1) + ' days';
    var yrs = sec / 3.156e7;
    if (yrs < 1e6) return Math.round(yrs).toLocaleString('en-US') + ' years';
    return fmtCount(yrs) + ' years';
  }

  function draw() {
    // slider 0..100 maps to n from 4 to 1,000,000 on a log scale
    var n = Math.round(4 * Math.pow(250000, slider.value / 100));
    var ymax = n * n;
    readout.textContent = 'n = ' + n.toLocaleString('en-US');
    while (svg.firstChild) svg.removeChild(svg.firstChild);
    function X(x) { return L + (R - L) * x / n; }
    function Y(y) { return B - (B - T) * Math.min(y, ymax) / ymax; }
    svg.appendChild(s('line', { class: 's-muted', x1: L, y1: B, x2: R + 8, y2: B }));
    svg.appendChild(s('line', { class: 's-muted', x1: L, y1: B, x2: L, y2: T - 6 }));
    [0.25, 0.5, 0.75, 1].forEach(function (t) {
      svg.appendChild(s('text', { class: 't-muted', x: X(n * t), y: B + 14, 'text-anchor': 'middle', 'font-size': 11 }, Math.round(n * t).toLocaleString('en-US')));
      svg.appendChild(s('text', { class: 't-muted', x: L - 6, y: Y(ymax * t) + 4, 'text-anchor': 'end', 'font-size': 10 }, fmtCount(ymax * t)));
    });
    svg.appendChild(s('text', { class: 't-muted', x: (L + R) / 2, y: B + 30, 'text-anchor': 'middle', 'font-size': 12 }, 'input size n'));
    var labels = [];
    FNS.forEach(function (fn) {
      var d = '', steps = 160, started = false, lastY = B, clipped = false;
      for (var k = 0; k <= steps; k++) {
        var x = n * k / steps, y = fn.f(x);
        if (y > ymax * 1.0000001) {
          if (started) d += ' L' + X(x).toFixed(1) + ',' + T;
          clipped = true;
          break;
        }
        d += (started ? ' L' : 'M') + X(x).toFixed(1) + ',' + Y(y).toFixed(1);
        started = true; lastY = Y(y);
      }
      svg.appendChild(s('path', { class: 's-' + fn.cls + ' f-none', d: d, 'stroke-width': 2.2 }));
      if (!clipped) labels.push({ fn: fn, y: lastY + 4 });
    });
    // keep the end-of-curve labels from piling on top of each other
    labels.sort(function (a, b) { return b.y - a.y; });
    var floor = B + 2;
    labels.forEach(function (lb) {
      lb.y = Math.min(lb.y, floor);
      floor = lb.y - 15;
      svg.appendChild(s('text', { class: 't-' + lb.fn.cls + ' b', x: R + 10, y: Math.max(T + 10, lb.y), 'font-size': 13 }, lb.fn.name));
    });
    svg.appendChild(s('text', { class: 't-bad b', x: L + 10, y: T + 10, 'font-size': 13 }, '2ⁿ'));
    var rows = FNS.map(function (fn) {
      var v = fn.f(n);
      return '<tr><td><b>' + fn.name + '</b></td><td style="text-align:right">' + fmtCount(v) + ' steps</td><td style="text-align:right">' + fmtTime(v) + '</td></tr>';
    }).join('');
    table.innerHTML = '<table style="margin:0;font-size:0.82rem"><thead><tr><th>at n = ' + n.toLocaleString('en-US') + '</th><th style="text-align:right">steps</th><th style="text-align:right">at a billion steps per second</th></tr></thead><tbody>' + rows + '</tbody></table>';
  }
  slider.addEventListener('input', draw);
  draw();
  return { demo: function () { slider.value = 60; draw(); } };
});
