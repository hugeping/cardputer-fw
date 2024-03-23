local bit = require "bit32"
local cp
local mode = 'cp'

local function printf(fmt, ...)
  io.stdout:write(string.format(fmt, ...))
end

local function parse_line(l)
  local r = 0
  for i=l:len(),1, -1 do
    local c = string.byte(l, i)
    if c == string.byte '-' or c == string.byte ' ' then
      r = r*2
    else
      r = r*2 + 1
    end
  end
  return r
end

local dict = {}
local cur = 0
local max = 15
local h = 1
print "const uint8_t FONT_data[] PROGMEM ={"
for l in io.lines() do
  if mode == 'cp' then
    if tonumber(l) then
      local nr = tonumber(l)
      if not cp or nr ~= cp + 1 then
--        print(l)
        table.insert(dict, { off = cur, start = nr, range = 1 })
      else
        local t = dict[#dict]
        t.range = t.range + 1
      end
      cp = nr
      printf("\t")
      mode = 'sym'
    end
  elseif mode == 'sym' then
    if l == "" then
      mode = 'cp'
      h = 1
      print()
    else
      if h <= max then
        printf("0x%02x, ", parse_line(l))
        cur = cur + 1
      end
      h = h + 1
    end
  end
end
print ""
print "};"
print "const uint16_t FONT_map[] = {"
for _, v in ipairs(dict) do
  printf("\t%d, 0x%04x, %d,\n", v.off, v.start, v.range)
end
print("\t0, 0, 0,")
print "};"

print [[

struct font8 {
	const uint8_t *data;
	const uint16_t *map;
	const int w;
	const int h;
};
#define FONT_W 8
#define FONT_H 15
const struct font8 FONT = {
	.data = FONT_data,
	.map = FONT_map,
	.w = FONT_W,
	.h = FONT_H,
};
]]
