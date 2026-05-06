# mGBA Live Lua Templates

## Purpose

この文書は、`mgba-live-cli run-lua` で使う Lua script の書き方、escaping、field setup 用 template をまとめる。

原則は **短い確認だけ `--code`、複雑な処理は `/tmp` の Lua file + `--file`**。shell escaping と Lua string escaping を混ぜると壊れやすいので、長い inline script を避ける。

## Escaping Rules

`run-lua --code` は次の layer を通る。

1. shell が quote / escape を処理する。
2. `mgba-live-cli` が argument を受け取る。
3. bridge が Lua string として command file に書く。
4. mGBA process 内で `load` / `loadstring` される。

このため、問題の多くは Lua ではなく shell quote の時点で起きる。

| Pattern | Use |
|---|---|
| `--code 'return emu:currentFrame()'` | OK。短い read-only check。 |
| `--code 'return "Route 101"'` | OK。outer single quote、inner double quote。 |
| `--code 'local s = "A"; return s'` | OK。短い state read。 |
| code に `'` が入る | `--file` に逃がす。 |
| code に newline、JSON、path、`$`、backslash が多い | `--file` に逃がす。 |
| shell double quote で包む | `$VAR` / command substitution が走るため非推奨。 |

Lua の長い text は long bracket を使う。

```lua
local message = [=[
This text may contain "double quotes" and 'single quotes'.
Backslashes are not special here: C:\tmp\example
]=]
return message
```

本文に `]=]` が入る場合は、delimiter を増やす。

```lua
local message = [==[
This text can contain ]=] safely.
]==]
```

実務上の rule:

- `--code` は one-liner の read-only check に寄せる。
- input macro、memory write、SaveBlock 操作、複数行 return table は `/tmp/*.lua` に出して `--file` で実行する。
- shell escaping で詰まったら Lua を直す前に `--file` へ切り替える。
- Lua file には session 固有の absolute path、symbol address、branch 名をコメントで残す。ただし file 自体は commit しない。

## File-Based Execution

複雑な script は `/tmp` に置いて実行する。

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli run-lua \
  --session codex-mgba-runtime-check \
  --file /tmp/mgba-live/check_state.lua \
  --timeout 8
```

Lua file は tracked source にしない。検証 artifact として `/tmp` に置き、必要な template や注意点だけ docs に残す。

## Return Values

bridge response に返す値は JSON 化される。返すものは number、string、boolean、table に限定する。

避けるもの:

- function
- userdata
- coroutine
- cyclic table
- huge memory dump

大きな dump は `read-range` か小さな table に分割する。

## Common Helpers

```lua
local function hex(n)
  return string.format("0x%08X", n or 0)
end

local function u8(addr)
  return emu:read8(addr)
end

local function u16(addr)
  return emu:read16(addr)
end

local function u32(addr)
  return emu:read32(addr)
end

local function w8(addr, value)
  emu:write8(addr, value % 0x100)
end

local function has_bit(value, bit)
  return math.floor(value / (2 ^ bit)) % 2 == 1
end

local function set_bit(value, bit)
  if has_bit(value, bit) then
    return value
  end
  return value + (2 ^ bit)
end

local function clear_bit(value, bit)
  if has_bit(value, bit) then
    return value - (2 ^ bit)
  end
  return value
end

local function read_ptr(addr)
  return u32(addr)
end
```

## SaveBlock Flag Template

`SaveBlock1` flags を直接見るときは、flag id、byte offset、bit offset を分ける。

```lua
local G_SAVE_BLOCK1_PTR = 0x03005208
local SAVE_BLOCK1_FLAGS_OFFSET = 0x1270
local FLAG_NO_ENCOUNTER = 0x8E5

local save1 = read_ptr(G_SAVE_BLOCK1_PTR)
local flag_byte = save1 + SAVE_BLOCK1_FLAGS_OFFSET + math.floor(FLAG_NO_ENCOUNTER / 8)
local flag_bit = FLAG_NO_ENCOUNTER % 8
local value = u8(flag_byte)

return {
  save1 = hex(save1),
  flag_byte = hex(flag_byte),
  flag_bit = flag_bit,
  raw = value,
  enabled = has_bit(value, flag_bit),
}
```

Set / clear するとき:

```lua
local next_value = set_bit(value, flag_bit)
w8(flag_byte, next_value)
return { before = value, after = next_value }
```

```lua
local next_value = clear_bit(value, flag_bit)
w8(flag_byte, next_value)
return { before = value, after = next_value }
```

注意: offset は branch / struct 変更で変わる可能性がある。必ず対象 commit の `include/global.h`、map file、symbol、または既存調査 record で確認してから使う。

## Field State Template

field test の開始前に、最低限 map / coords / callback を見る。

```lua
local G_SAVE_BLOCK1_PTR = 0x03005208
local G_MAIN = 0x03006704
local MAIN_CALLBACK2_OFFSET = 0x04

local save1 = read_ptr(G_SAVE_BLOCK1_PTR)

return {
  save1 = hex(save1),
  callback2 = hex(read_ptr(G_MAIN + MAIN_CALLBACK2_OFFSET)),
  map_group = u8(save1 + 0x04),
  map_num = u8(save1 + 0x05),
  x = u16(save1 + 0x00),
  y = u16(save1 + 0x02),
  frame = emu:currentFrame(),
}
```

この template の offsets は project-specific。使う前に対象 branch の struct layout と既存 runtime record を確認する。

## Frame Macro Template

長く歩かせる、一定間隔で input を切り替える、といった検証は frame callback で macro を作る。

```lua
local macro_key = "codex_walk_up_down"

_G[macro_key] = {
  active = true,
  frame = 0,
  limit = 720,
}

callbacks:add("frame", function()
  local macro = _G[macro_key]
  if macro == nil or macro.active ~= true then
    return
  end

  macro.frame = macro.frame + 1

  emu:clearKey(C.GBA_KEY.UP)
  emu:clearKey(C.GBA_KEY.DOWN)

  if math.floor(macro.frame / 30) % 2 == 0 then
    emu:addKey(C.GBA_KEY.UP)
  else
    emu:addKey(C.GBA_KEY.DOWN)
  end

  if macro.frame >= macro.limit then
    emu:clearKey(C.GBA_KEY.UP)
    emu:clearKey(C.GBA_KEY.DOWN)
    macro.active = false
  end
end)

return { macro_key = macro_key, limit = _G[macro_key].limit }
```

Completion check:

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli run-lua \
  --session codex-mgba-runtime-check \
  --code 'local m = _G["codex_walk_up_down"]; return m == nil or m.active == false' \
  --timeout 8
```

cleanup:

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli input-clear \
  --session codex-mgba-runtime-check \
  --timeout 8
```

## Startup Preconditions

Lua automation cannot assume that the ROM is already in a useful field state.

Before runtime validation, record one of these:

- save file or savestate supplied by the user;
- exact debug menu route used to warp / set flags / give items;
- full new-game setup route from title screen through intro text to field control.

If the test needs grass, water, cave, battle, or a specific NPC, confirm the map and coords before starting input. A screenshot at the wrong place is not evidence.

Debug menu can usually solve map setup after field control is available. It may not bypass the earliest intro text before the player can open menus. If a test must start after intro, ask for a prepared save or explicitly spend the setup time and record it.

## Report Checklist For Lua-Assisted Validation

Lua で状態を作ると、後から「実際に画面で起きたのか」「memory を書いただけなのか」が分かりにくくなる。report では次を分ける。

| Item | Record |
|---|---|
| Setup method | Debug menu、Lua memory write、manual input、save reuse のどれか。 |
| State proof | map / coords、callback、flag / var、party species などの read result。 |
| Visual proof | screenshot path、または user が実画面で確認した画面名。 |
| Behavior proof | wild battle 発生、trainer battle 発生、menu open、sound timing など実際に起きた結果。 |
| Cleanup | `input-clear` と `status --all` の final result。 |

Debug menu や Lua は検証の準備として使ってよい。ただし feature の合否は、対象 behavior が実画面または runtime state で確認できたかで判断する。
