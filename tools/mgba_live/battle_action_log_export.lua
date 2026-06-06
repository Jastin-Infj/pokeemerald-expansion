-- Export the in-ROM gBattleActionLog ring buffer to a host JSON file.
--
-- Usage:
--   mgba-live-cli run-lua --session SESSION --file tools/mgba_live/battle_action_log_export.lua
--
-- Optional overrides:
--   _G.BATTLE_ACTION_LOG_ROOT       project root, defaults to "." or POKEEMERALD_ROOT
--   _G.BATTLE_ACTION_LOG_MAP        map file path, defaults to ROOT/pokeemerald.map
--   _G.BATTLE_ACTION_LOG_OUT        output path, defaults to /tmp/pokeemerald-battle-action-log.json
--   env POKEEMERALD_ROOT            project root fallback
--   env BATTLE_ACTION_LOG_OUT       output path fallback

local ENTRY_CAPACITY = 128
local ENTRY_SIZE = 24
local HEADER_OFFSET = ENTRY_CAPACITY * ENTRY_SIZE

local FLAG_VALID = 1
local FLAG_RESOLVED = 2
local FLAG_CORRECTED = 4
local FLAG_GIMMICK = 8

local ACTION_NAMES = {
  [0] = "B_ACTION_USE_MOVE",
  [1] = "B_ACTION_USE_ITEM",
  [2] = "B_ACTION_SWITCH",
  [3] = "B_ACTION_RUN",
  [4] = "B_ACTION_SAFARI_WATCH_CAREFULLY",
  [5] = "B_ACTION_SAFARI_BALL",
  [6] = "B_ACTION_SAFARI_POKEBLOCK",
  [7] = "B_ACTION_SAFARI_GO_NEAR",
  [8] = "B_ACTION_SAFARI_RUN",
  [9] = "B_ACTION_WALLY_THROW",
  [10] = "B_ACTION_EXEC_SCRIPT",
  [11] = "B_ACTION_TRY_FINISH",
  [12] = "B_ACTION_FINISHED_OR_CANCEL_PARTNER",
  [13] = "B_ACTION_NOTHING_FAINTED",
  [14] = "B_ACTION_UNK_14",
  [15] = "B_ACTION_UNK_15",
  [20] = "B_ACTION_DEBUG",
  [21] = "B_ACTION_THROW_BALL",
  [255] = "B_ACTION_NONE",
}

local POSITION_NAMES = {
  [0] = "B_POSITION_PLAYER_LEFT",
  [1] = "B_POSITION_OPPONENT_LEFT",
  [2] = "B_POSITION_PLAYER_RIGHT",
  [3] = "B_POSITION_OPPONENT_RIGHT",
  [255] = "B_POSITION_ABSENT",
}

local FLAG_NAMES = {
  { mask = FLAG_VALID, name = "valid" },
  { mask = FLAG_RESOLVED, name = "resolved_switch_in" },
  { mask = FLAG_CORRECTED, name = "corrected_switch_in" },
  { mask = FLAG_GIMMICK, name = "selected_gimmick" },
}

local function join_path(root, leaf)
  if root == "" or root == "." then
    return "./" .. leaf
  end
  if root:sub(-1) == "/" then
    return root .. leaf
  end
  return root .. "/" .. leaf
end

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

local function has_mask(value, mask)
  return math.floor(value / mask) % 2 == 1
end

local function json_escape(value)
  value = tostring(value)
  value = value:gsub("\\", "\\\\")
  value = value:gsub("\"", "\\\"")
  value = value:gsub("\b", "\\b")
  value = value:gsub("\f", "\\f")
  value = value:gsub("\n", "\\n")
  value = value:gsub("\r", "\\r")
  value = value:gsub("\t", "\\t")
  return value
end

local function is_array(value)
  local count = 0
  local max_index = 0
  for key, _ in pairs(value) do
    if type(key) ~= "number" or key < 1 or key ~= math.floor(key) then
      return false
    end
    count = count + 1
    if key > max_index then
      max_index = key
    end
  end
  return max_index == count
end

local function sorted_keys(value)
  local keys = {}
  for key, _ in pairs(value) do
    keys[#keys + 1] = key
  end
  table.sort(keys, function(a, b)
    return tostring(a) < tostring(b)
  end)
  return keys
end

local function json_encode(value)
  local value_type = type(value)
  if value_type == "nil" then
    return "null"
  elseif value_type == "boolean" then
    return value and "true" or "false"
  elseif value_type == "number" then
    return tostring(value)
  elseif value_type == "string" then
    return "\"" .. json_escape(value) .. "\""
  elseif value_type == "table" then
    if is_array(value) then
      local parts = {}
      for i = 1, #value do
        parts[#parts + 1] = json_encode(value[i])
      end
      return "[" .. table.concat(parts, ",") .. "]"
    end

    local parts = {}
    for _, key in ipairs(sorted_keys(value)) do
      parts[#parts + 1] = json_encode(tostring(key)) .. ":" .. json_encode(value[key])
    end
    return "{" .. table.concat(parts, ",") .. "}"
  end

  error("cannot JSON encode " .. value_type)
end

local function read_file_lines(path)
  local file = io.open(path, "r")
  if file == nil then
    return nil
  end

  local lines = {}
  for line in file:lines() do
    lines[#lines + 1] = line
  end
  file:close()
  return lines
end

local function read_number(text)
  if text == nil then
    return nil
  end

  local hex_value = text:match("^0x([0-9A-Fa-f]+)$")
  if hex_value ~= nil then
    return tonumber(hex_value, 16)
  end

  if text:match("^%d+$") then
    return tonumber(text, 10)
  end

  return nil
end

local function load_enum_names(path, wanted_prefix)
  local names = {}
  local values_by_name = {}
  local lines = read_file_lines(path)
  if lines == nil then
    return names
  end

  local next_value = 0
  for _, line in ipairs(lines) do
    local direct_name, value_text = line:match("^%s*([A-Z0-9_]+)%s*=%s*([^,%s/]+)")
    local auto_name = line:match("^%s*([A-Z0-9_]+)%s*,")
    local name = direct_name or auto_name

    if name ~= nil then
      local value = read_number(value_text)
      if value == nil and value_text ~= nil then
        value = values_by_name[value_text]
      end
      if value == nil then
        value = next_value
      end

      values_by_name[name] = value
      if name:sub(1, #wanted_prefix) == wanted_prefix and names[value] == nil then
        names[value] = name
      end
      next_value = value + 1
    end
  end

  return names
end

local function load_auto_enum_names(path, wanted_prefix)
  local names = {}
  local lines = read_file_lines(path)
  if lines == nil then
    return names
  end

  local next_value = 0
  for _, line in ipairs(lines) do
    local direct_name, value_text = line:match("^%s*([A-Z0-9_]+)%s*=%s*([^,%s/]+)")
    local auto_name = line:match("^%s*([A-Z0-9_]+)%s*,")
    local name = direct_name or auto_name

    if name ~= nil and name:sub(1, #wanted_prefix) == wanted_prefix then
      local value = read_number(value_text)
      if value == nil then
        value = next_value
      end
      if names[value] == nil then
        names[value] = name
      end
      next_value = value + 1
    end
  end

  return names
end

local function find_symbol(map_path, symbol_name)
  local file = assert(io.open(map_path, "r"), "could not open map file: " .. map_path)
  for line in file:lines() do
    local addr = line:match("0x([0-9A-Fa-f]+)%s+" .. symbol_name .. "%s*$")
    if addr ~= nil then
      file:close()
      return tonumber(addr, 16)
    end
  end
  file:close()
  return nil
end

local function position_side_name(position)
  if position == 255 then
    return "absent"
  end
  if position % 2 == 0 then
    return "player"
  end
  return "opponent"
end

local function position_slot_name(position)
  if position == 255 then
    return "absent"
  end
  if position < 2 then
    return "left"
  end
  return "right"
end

local function flag_name_list(flags)
  local names = {}
  for _, info in ipairs(FLAG_NAMES) do
    if has_mask(flags, info.mask) then
      names[#names + 1] = info.name
    end
  end
  return names
end

local function read_entry(base, ring_index, move_names, item_names, gimmick_names, battler_positions)
  local offset = base + ring_index * ENTRY_SIZE
  local battler = u8(offset + 8)
  local target = u8(offset + 10)
  local position = battler_positions[battler + 1]
  local target_position = nil

  if target < #battler_positions then
    target_position = battler_positions[target + 1]
  end

  local move = u16(offset + 4)
  local item = u16(offset + 6)
  local action = u8(offset + 9)
  local gimmick = u32(offset + 16)
  local flags = u8(offset + 20)

  return {
    ring_index = ring_index,
    sequence = u16(offset + 0),
    turn = u16(offset + 2),
    battler = battler,
    battler_position = position,
    battler_position_name = POSITION_NAMES[position] or ("POSITION_" .. tostring(position)),
    battler_side = position_side_name(position),
    battler_slot = position_slot_name(position),
    action = action,
    action_name = ACTION_NAMES[action] or ("B_ACTION_" .. tostring(action)),
    move = move,
    move_name = move_names[move] or ("MOVE_" .. tostring(move)),
    item = item,
    item_name = item_names[item] or ("ITEM_" .. tostring(item)),
    target = target,
    target_position = target_position,
    target_position_name = target_position ~= nil and (POSITION_NAMES[target_position] or ("POSITION_" .. tostring(target_position))) or nil,
    move_slot = u8(offset + 11),
    party_index = u8(offset + 12),
    gimmick = gimmick,
    gimmick_name = gimmick_names[gimmick] or ("GIMMICK_" .. tostring(gimmick)),
    flags = flags,
    flag_names = flag_name_list(flags),
    valid = has_mask(flags, FLAG_VALID),
    resolved_switch_in = has_mask(flags, FLAG_RESOLVED),
    corrected_switch_in = has_mask(flags, FLAG_CORRECTED),
    selected_gimmick = has_mask(flags, FLAG_GIMMICK),
  }
end

local root = _G.BATTLE_ACTION_LOG_ROOT or os.getenv("POKEEMERALD_ROOT") or "."
local map_path = _G.BATTLE_ACTION_LOG_MAP or join_path(root, "pokeemerald.map")
local out_path = _G.BATTLE_ACTION_LOG_OUT or os.getenv("BATTLE_ACTION_LOG_OUT") or "/tmp/pokeemerald-battle-action-log.json"

local log_addr = _G.BATTLE_ACTION_LOG_ADDR or find_symbol(map_path, "gBattleActionLog")
assert(log_addr ~= nil, "could not find gBattleActionLog in " .. map_path)

local battler_positions_addr = _G.BATTLER_POSITIONS_ADDR or find_symbol(map_path, "gBattlerPositions")

local move_names = load_enum_names(join_path(root, "include/constants/moves.h"), "MOVE_")
local item_names = load_enum_names(join_path(root, "include/constants/items.h"), "ITEM_")
local gimmick_names = load_auto_enum_names(join_path(root, "include/battle_gimmick.h"), "GIMMICK_")

local header_addr = log_addr + HEADER_OFFSET
local sequence = u16(header_addr + 0)
local last_recorded_turn = u16(header_addr + 2)
local cursor = u8(header_addr + 4)
local count = u8(header_addr + 5)

if count > ENTRY_CAPACITY then
  count = ENTRY_CAPACITY
end

local battler_positions = {}
for i = 0, 3 do
  if battler_positions_addr ~= nil then
    battler_positions[#battler_positions + 1] = u8(battler_positions_addr + i)
  else
    battler_positions[#battler_positions + 1] = i
  end
end

local start_index = 0
if count == ENTRY_CAPACITY then
  start_index = cursor
end

local entries = {}
local skipped_invalid = 0
for i = 0, count - 1 do
  local ring_index = (start_index + i) % ENTRY_CAPACITY
  local entry = read_entry(log_addr, ring_index, move_names, item_names, gimmick_names, battler_positions)
  if entry.valid then
    entries[#entries + 1] = entry
  else
    skipped_invalid = skipped_invalid + 1
  end
end

local frame = nil
if emu.currentFrame ~= nil then
  frame = emu:currentFrame()
end

local payload = {
  schema = "pokeemerald.battle_action_log.v1",
  generated_at_utc = os.date("!%Y-%m-%dT%H:%M:%SZ"),
  frame = frame,
  source = {
    map_path = map_path,
    gBattleActionLog = hex(log_addr),
    gBattlerPositions = battler_positions_addr ~= nil and hex(battler_positions_addr) or nil,
    entry_capacity = ENTRY_CAPACITY,
    entry_size = ENTRY_SIZE,
  },
  header = {
    sequence = sequence,
    last_recorded_turn = last_recorded_turn,
    cursor = cursor,
    count = count,
    skipped_invalid = skipped_invalid,
  },
  battler_positions = battler_positions,
  entries = entries,
}

local out_file = assert(io.open(out_path, "w"), "could not open output file: " .. out_path)
out_file:write(json_encode(payload))
out_file:write("\n")
out_file:close()

return {
  output = out_path,
  schema = payload.schema,
  entries = #entries,
  sequence = sequence,
  last_recorded_turn = last_recorded_turn,
  cursor = cursor,
  count = count,
  skipped_invalid = skipped_invalid,
  frame = frame,
}
