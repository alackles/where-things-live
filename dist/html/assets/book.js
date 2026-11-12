// Page chrome for the web edition: theme toggle, mobile nav, prompt copy
// buttons, and a scroll-spy for the section list.  No dependencies.
(function () {
  'use strict';
  var root = document.documentElement;

  // ---- theme ----------------------------------------------------------
  var tbtn = document.getElementById('theme-toggle');
  function currentTheme() {
    var t = root.getAttribute('data-theme');
    if (t) return t;
    return window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  }
  if (tbtn) tbtn.addEventListener('click', function () {
    var next = currentTheme() === 'dark' ? 'light' : 'dark';
    root.setAttribute('data-theme', next);
    try { localStorage.setItem('wtl-theme', next); } catch (e) {}
  });

  // ---- mobile nav -----------------------------------------------------
  var nbtn = document.getElementById('nav-toggle');
  if (nbtn) nbtn.addEventListener('click', function () {
    var open = document.body.classList.toggle('nav-open');
    nbtn.setAttribute('aria-expanded', open ? 'true' : 'false');
  });
  document.addEventListener('click', function (e) {
    if (!document.body.classList.contains('nav-open')) return;
    var sb = document.getElementById('sidebar');
    if (sb.contains(e.target) && e.target.tagName !== 'A') return;
    if (e.target === nbtn) return;
    document.body.classList.remove('nav-open');
    if (nbtn) nbtn.setAttribute('aria-expanded', 'false');
  });

  // ---- copy buttons on LLM prompts -------------------------------------
  document.querySelectorAll('.callout.llm blockquote').forEach(function (bq) {
    if (!navigator.clipboard) return;
    var b = document.createElement('button');
    b.className = 'copy-btn';
    b.type = 'button';
    b.textContent = 'Copy';
    b.setAttribute('aria-label', 'Copy this prompt');
    b.addEventListener('click', function () {
      var clone = bq.cloneNode(true);
      var btn = clone.querySelector('.copy-btn');
      if (btn) btn.remove();
      var text = Array.prototype.map.call(clone.children, function (c) { return c.innerText.trim(); }).join('\n\n');
      navigator.clipboard.writeText(text).then(function () {
        b.textContent = 'Copied';
        setTimeout(function () { b.textContent = 'Copy'; }, 1400);
      });
    });
    bq.classList.add('has-copy');
    bq.appendChild(b);
  });

  // ---- scroll spy -----------------------------------------------------
  var links = Array.prototype.slice.call(document.querySelectorAll('.sidebar .secs a'));
  if (links.length && 'IntersectionObserver' in window) {
    var byId = {};
    links.forEach(function (a) { byId[decodeURIComponent(a.getAttribute('href').slice(1))] = a; });
    var heads = Object.keys(byId).map(function (id) { return document.getElementById(id); }).filter(Boolean);
    var active = null;
    function update() {
      var best = null, y = window.scrollY + 120;
      heads.forEach(function (h) { if (h.getBoundingClientRect().top + window.scrollY <= y) best = h; });
      var a = best ? byId[best.id] : null;
      if (a !== active) {
        if (active) active.classList.remove('active');
        if (a) a.classList.add('active');
        active = a;
      }
    }
    var ticking = false;
    window.addEventListener('scroll', function () {
      if (ticking) return;
      ticking = true;
      requestAnimationFrame(function () { ticking = false; update(); });
    }, { passive: true });
    update();
  }
  var cur = document.querySelector('.sidebar li.current');
  if (cur && cur.scrollIntoView) cur.scrollIntoView({ block: 'center' });
})();
