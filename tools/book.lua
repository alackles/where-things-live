-- Pandoc Lua filter: turns the book's ::: blocks into the right thing for
-- each edition.  HTML gets <aside>/<figure>/<details>; LaTeX gets tcolorbox
-- callouts, TikZ figures, and end-of-chapter answers.

local is_html = FORMAT:match("html") ~= nil
local is_latex = FORMAT:match("latex") ~= nil

local chap = ""
local figdir = "build/figures"
local ncheck = 0
local answers = {}

local LABELS = {
  llm = "Ask an LLM",
  byhand = "By hand first",
  note = "Aside",
  warning = "Pitfall",
  wild = "In the wild",
  goals = "This week",
  fourq = "The four questions",
  check = "Check yourself",
}

local function esc_html(s)
  return (s:gsub("&", "&amp;"):gsub("<", "&lt;"):gsub(">", "&gt;"):gsub('"', "&quot;"))
end

local function esc_tex(s)
  return (s:gsub("([%%&#_$])", "\\%1"))
end

local function read_file(path)
  local f = io.open(path, "r")
  if not f then
    io.stderr:write("book.lua: missing figure file " .. path .. "\n")
    return nil
  end
  local s = f:read("a")
  f:close()
  return s
end

local function has(el, cls)
  return el.classes:includes(cls)
end

local function caption_inlines(blocks)
  local out = pandoc.List()
  for i, b in ipairs(blocks) do
    if b.t == "Para" or b.t == "Plain" then
      if #out > 0 then out:insert(pandoc.Space()) end
      out:extend(b.content)
    end
  end
  return out
end

local function figure(el, kind)
  local id = el.identifier
  local base = id:gsub("^fig:", "")
  local name = el.attributes["fig"] or base
  local num = el.attributes["num"] or "?"
  local cap = caption_inlines(el.content)
  if is_html then
    local svg = read_file(figdir .. "/" .. name .. ".svg") or "<p>[missing figure " .. name .. "]</p>"
    local open
    if kind == "viz" then
      local data = ""
      for k, v in pairs(el.attributes) do
        if k ~= "fig" and k ~= "num" and k ~= "name" then
          data = data .. " data-" .. k .. '="' .. esc_html(v) .. '"'
        end
      end
      open = '<figure class="fig viz" id="' .. id .. '">'
        .. '<div class="viz-widget" data-viz="' .. esc_html(el.attributes["name"] or "") .. '"' .. data .. ">"
        .. '<div class="viz-fallback">' .. svg .. "</div></div><figcaption>"
    else
      open = '<figure class="fig" id="' .. id .. '">' .. svg .. "<figcaption>"
    end
    local inl = pandoc.List({ pandoc.RawInline("html", '<span class="fignum">Figure ' .. num .. "</span> ") })
    inl:extend(cap)
    return { pandoc.RawBlock("html", open), pandoc.Plain(inl), pandoc.RawBlock("html", "</figcaption></figure>") }
  elseif is_latex then
    local inl = pandoc.List({ pandoc.RawInline("latex", "\\caption{") })
    inl:extend(cap)
    if kind == "viz" then inl:insert(pandoc.RawInline("latex", "\\vizmark")) end
    inl:insert(pandoc.RawInline("latex", "}\\label{" .. id .. "}"))
    return {
      pandoc.RawBlock("latex", "\\begin{figure}[!htbp]\\centering\n\\input{" .. figdir .. "/" .. name .. ".tikz}"),
      pandoc.Plain(inl),
      pandoc.RawBlock("latex", "\\end{figure}"),
    }
  end
end

local function callout(el, kind)
  local label = LABELS[kind]
  local title = el.attributes["title"]
  local blocks = pandoc.List()
  if is_html then
    local t = '<p class="callout-title"><span class="lab">' .. label .. "</span>"
    if title then t = t .. ' <span class="ttl">' .. esc_html(title) .. "</span>" end
    blocks:insert(pandoc.RawBlock("html", '<aside class="callout ' .. kind .. '">' .. t .. "</p>"))
    blocks:extend(el.content)
    blocks:insert(pandoc.RawBlock("html", "</aside>"))
  elseif is_latex then
    blocks:insert(pandoc.RawBlock("latex",
      "\\begin{callout}{" .. kind .. "}{" .. label .. "}{" .. esc_tex(title or "") .. "}"))
    blocks:extend(el.content)
    blocks:insert(pandoc.RawBlock("latex", "\\end{callout}"))
  end
  return blocks
end

local function check(el)
  ncheck = ncheck + 1
  local num = (chap ~= "" and (chap .. ".") or "") .. ncheck
  local q, a = pandoc.List(), pandoc.List()
  for _, b in ipairs(el.content) do
    if b.t == "Div" and has(b, "answer") then a:extend(b.content) else q:insert(b) end
  end
  local blocks = pandoc.List()
  if is_html then
    blocks:insert(pandoc.RawBlock("html",
      '<aside class="callout check"><p class="callout-title"><span class="lab">Check yourself ' .. num .. "</span></p>"))
    blocks:extend(q)
    if #a > 0 then
      blocks:insert(pandoc.RawBlock("html", '<details class="answer"><summary>Answer</summary>'))
      blocks:extend(a)
      blocks:insert(pandoc.RawBlock("html", "</details>"))
    end
    blocks:insert(pandoc.RawBlock("html", "</aside>"))
  elseif is_latex then
    blocks:insert(pandoc.RawBlock("latex", "\\begin{callout}{check}{Check yourself " .. num .. "}{}"))
    blocks:extend(q)
    blocks:insert(pandoc.RawBlock("latex", "\\end{callout}"))
    if #a > 0 then answers[#answers + 1] = { num = num, blocks = a } end
  end
  return blocks
end

local function listing(el)
  local file = el.attributes["file"] or ""
  local blocks = pandoc.List()
  if is_html then
    blocks:insert(pandoc.RawBlock("html",
      '<div class="listing"><div class="listing-file">code/' .. esc_html(file) .. "</div>"))
    blocks:extend(el.content)
    blocks:insert(pandoc.RawBlock("html", "</div>"))
  elseif is_latex then
    blocks:insert(pandoc.RawBlock("latex", "\\listingfile{code/" .. file .. "}"))
    blocks:extend(el.content)
  end
  return blocks
end

local function Div(el)
  if has(el, "figure") then return figure(el, "figure") end
  if has(el, "viz") then return figure(el, "viz") end
  if has(el, "listing") then return listing(el) end
  if has(el, "check") then return check(el) end
  if has(el, "answer") then return nil end
  for kind, _ in pairs(LABELS) do
    if has(el, kind) then return callout(el, kind) end
  end
  return nil
end

-- LaTeX: code blocks pandoc does not highlight (console sessions, plain
-- text) would come out as bare verbatim.  Give them the same shaded box.
local HIGHLIGHTED = { cpp = true, python = true, c = true, makefile = true, bash = true }
local function CodeBlock(el)
  if not is_latex then return nil end
  local lang = el.classes[1]
  if lang and HIGHLIGHTED[lang] then return nil end
  local t = el.text:gsub("\\", "\1"):gsub("([{}])", "\\%1"):gsub("\1", "\\textbackslash{}")
  return pandoc.RawBlock("latex",
    "\\begin{Shaded}\n\\begin{Highlighting}[]\n" .. t .. "\n\\end{Highlighting}\n\\end{Shaded}")
end

local function get_meta(m)
  if m.chapnum then chap = pandoc.utils.stringify(m.chapnum) end
  if m.figdir then figdir = pandoc.utils.stringify(m.figdir) end
end

local function finish(doc)
  if is_latex and #answers > 0 then
    doc.blocks:insert(pandoc.RawBlock("latex", "\\begin{answers}"))
    for _, ans in ipairs(answers) do
      local first = ans.blocks[1]
      local lead = pandoc.RawInline("latex", "\\answeritem{" .. ans.num .. "}")
      if first and (first.t == "Para" or first.t == "Plain") then
        first.content:insert(1, lead)
      else
        ans.blocks:insert(1, pandoc.Para({ lead }))
      end
      doc.blocks:extend(ans.blocks)
    end
    doc.blocks:insert(pandoc.RawBlock("latex", "\\end{answers}"))
  end
  return doc
end

-- Tables: in LaTeX, keep them from floating away and make them a bit smaller.
return {
  { Meta = get_meta },
  { Div = Div },
  { CodeBlock = CodeBlock },
  { Pandoc = finish },
}
