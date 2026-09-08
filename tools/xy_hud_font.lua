-- Shared Aseprite font adapter for the XY status HUD design.
-- Uses the repository's actual Latin glyph pixels AND advance tables.
-- No font rescaling, guessed letter widths, or replacement typeface.
local M = {}
local root = assert(app.params.root, "root script parameter is required")
local function read(path)
  local f = assert(io.open(root .. "/" .. path, "r"))
  local s = f:read("*a")
  f:close()
  return s
end
local definitions = read("src/fonts.c")
local charmap = {}
for ch, hex in read("charmap.txt"):gmatch("'([^']+)'%s*=%s*([%x]+)") do
  if #ch == 1 and not charmap[ch] then charmap[ch] = tonumber(hex, 16) end
end
local cache = {}
local names = {
  small = {"latin_small", "Small"},
  narrow = {"latin_small_narrow", "SmallNarrow"},
  narrower = {"latin_small_narrower", "SmallNarrower"},
}
local function font(kind)
  if cache[kind] then return cache[kind] end
  local spec = assert(names[kind], "unknown font")
  local body = assert(definitions:match("gFont" .. spec[2]
    .. "LatinGlyphWidths%[%]%s*=%s*{(.-)}"), "missing glyph width table")
  local widths = {}
  for n in body:gmatch("%d+") do widths[#widths + 1] = tonumber(n) end
  local spr = assert(app.open(root .. "/graphics/fonts/" .. spec[1] .. ".png"))
  assert(spr.colorMode == ColorMode.INDEXED)
  cache[kind] = {image = Image(spr.cels[1].image), widths = widths}
  spr:close()
  return cache[kind]
end
function M.width(text, kind)
  local f, width = font(kind or "small"), 0
  for ch in text:gmatch(".") do
    local id = assert(charmap[ch], "unmapped character: " .. ch)
    width = width + assert(f.widths[id + 1])
  end
  return width
end
function M.fit(text, available)
  for _, kind in ipairs({"small", "narrow", "narrower"}) do
    if M.width(text, kind) <= available then return kind end
  end
  error("text does not fit reserved width: " .. text)
end
function M.draw(image, x, y, text, foreground, shadow, kind)
  local f = font(kind or "small")
  for ch in text:gmatch(".") do
    local id = assert(charmap[ch])
    local advance = assert(f.widths[id + 1])
    local sx, sy = (id % 16) * 16, math.floor(id / 16) * 16
    for yy = 0, 11 do
      for xx = 0, advance - 1 do
        local index = f.image:getPixel(sx + xx, sy + yy)
        if index == 1 or (index == 2 and shadow) then
          assert(x + xx >= 0 and x + xx < image.width
            and y + yy >= 0 and y + yy < image.height, "glyph clipped")
          image:drawPixel(x + xx, y + yy, index == 1 and foreground or shadow)
        end
      end
    end
    x = x + advance
  end
  return x
end
return M
