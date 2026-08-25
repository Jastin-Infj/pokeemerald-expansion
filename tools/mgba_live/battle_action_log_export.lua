-- Export the in-ROM gBattleActionLog ring buffer to a host JSON file.
--
-- Usage:
--   mgba-live-cli run-lua --session SESSION --file tools/mgba_live/battle_action_log_export.lua
--
-- Optional overrides:
--   _G.BATTLE_ACTION_LOG_ROOT       project root, defaults to "." or POKEEMERALD_ROOT
--   _G.BATTLE_ACTION_LOG_MAP        map file path, defaults to ROOT/pokeemerald.map
--   _G.BATTLE_ACTION_LOG_OUT        output path, defaults to /tmp/pokeemerald-battle-action-log.json
--   _G.BATTLE_ACTION_LOG_SKIP_EMPTY skip writing when no valid entries exist
--   env POKEEMERALD_ROOT            project root fallback
--   env BATTLE_ACTION_LOG_OUT       output path fallback

local ENTRY_CAPACITY = 128
local ENTRY_SIZE = 28
local HEADER_OFFSET = ENTRY_CAPACITY * ENTRY_SIZE

local AI_PLAN_CAPACITY = 32
local AI_PLAN_SIZE = 32
local AI_CANDIDATE_CAPACITY = 256
local AI_CANDIDATE_SIZE = 28
local AI_BOARD_CAPACITY = AI_PLAN_CAPACITY * 3
local AI_BOARD_SIZE = 96
local AI_TRACE_SCHEMA_VERSION = 5
local AI_TRACE_HEADER_MAGIC = 0xA15C
local AI_PLAN_OFFSET = 0
local AI_CANDIDATE_OFFSET = AI_PLAN_CAPACITY * AI_PLAN_SIZE
local AI_BOARD_OFFSET = AI_CANDIDATE_OFFSET + AI_CANDIDATE_CAPACITY * AI_CANDIDATE_SIZE
local AI_TRACE_HEADER_OFFSET = AI_BOARD_OFFSET + AI_BOARD_CAPACITY * AI_BOARD_SIZE

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

local AI_REASON_NAMES = {
  [0] = "none",
  [1] = "clean_damage_preferred",
  [2] = "known_command_answer",
  [3] = "gimmick_stabilized",
  [4] = "switch_preserve",
  [5] = "board_control",
  [6] = "setup_denial",
  [7] = "perish_escape",
  [8] = "desperation_comeback",
  [9] = "hax_out",
  [10] = "ally_sacrifice_board_reset",
  [11] = "commander_slot_correction",
}

local AI_THREAT_NAMES = {
  { mask = 1, name = "damage_race" },
  { mask = 2, name = "known_ko_pressure" },
  { mask = 4, name = "setup_checkmate" },
  { mask = 8, name = "perish_trap_clock" },
  { mask = 16, name = "mode_loss" },
  { mask = 32, name = "desperation" },
}

local AI_RISK_NAMES = {
  [0] = "none",
  [1] = "high_variance_comeback",
  [2] = "low_accuracy_status",
  [3] = "low_accuracy_damage",
  [4] = "secondary_hax",
  [5] = "ohko_fish",
  [6] = "delayed_attack",
  [7] = "partner_sacrifice",
  [8] = "second_protect",
  [9] = "switch_survival",
}

local AI_SHORT_LINE_NAMES = {
  { mask = 1, name = "clean_damage" },
  { mask = 2, name = "switch_escape" },
  { mask = 4, name = "setup_denial" },
  { mask = 8, name = "mode_control" },
  { mask = 16, name = "reserve_entry" },
  { mask = 32, name = "high_variance" },
}

local AI_CANDIDATE_LINE_NAMES = {
  [0] = "none",
  [1] = "clean_damage",
  [2] = "switch_escape",
  [3] = "setup_denial",
  [4] = "mode_control",
  [5] = "reserve_entry",
  [6] = "high_variance",
}

local AI_TRACE_ACTION_KIND_NAMES = {
  [0] = "none",
  [1] = "move",
  [2] = "switch",
  [3] = "item",
}

local AI_TRACE_PREDICTION_SOURCE_NAMES = {
  [0] = "none",
  [1] = "normal_prediction",
  [2] = "confirmed_command",
  [3] = "action_log_fallback",
}

local AI_TRACE_TERMINATION_NAMES = {
  [0] = "complete",
  [1] = "node_budget",
  [2] = "frame_budget",
  [3] = "candidate_budget",
  [4] = "fallback",
}

local AI_TRACE_REJECTION_NAMES = {
  { mask = 1, name = "confirmed_flinch" },
  { mask = 2, name = "unapproved_ally_target" },
}

local AI_TRACE_READ_INTERACTION_NAMES = {
  { mask = 1, name = "protect" },
  { mask = 2, name = "redirection" },
  { mask = 4, name = "fake_out" },
  { mask = 8, name = "known_ko" },
  { mask = 16, name = "setup_item_sequence" },
}

local AI_TRACE_ALLY_INTERACTION_NAMES = {
  [0] = "none",
  [1] = "explicit_hostility",
  [2] = "heal_or_cure",
  [3] = "support",
  [4] = "ability_trigger",
  [5] = "item_trigger",
}

local AI_TRACE_PLAN_FLAG_NAMES = {
  { mask = 1, name = "valid" },
  { mask = 2, name = "joint" },
  { mask = 4, name = "legacy_evaluator" },
  { mask = 8, name = "components_partial" },
  { mask = 16, name = "deepest_complete_used" },
  { mask = 32, name = "node_budget_hit" },
  { mask = 64, name = "frame_budget_hit" },
  { mask = 128, name = "trace_truncated" },
}

local AI_TRACE_CANDIDATE_FLAG_NAMES = {
  { mask = 1, name = "valid" },
  { mask = 2, name = "chosen" },
  { mask = 4, name = "joint" },
  { mask = 8, name = "complete" },
  { mask = 16, name = "score_clamped" },
  { mask = 32, name = "pruned" },
  { mask = 64, name = "forced_trace_inclusion" },
  { mask = 128, name = "components_partial" },
}

local AI_TRACE_BOARD_PHASE_NAMES = {
  [0] = "none",
  [1] = "before",
  [2] = "predicted_after",
  [3] = "actual_after",
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

local function s16(addr)
  local value = u16(addr)
  if value >= 0x8000 then
    return value - 0x10000
  end
  return value
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

local function mask_name_list(value, definitions)
  local names = {}
  for _, info in ipairs(definitions) do
    if has_mask(value, info.mask) then
      names[#names + 1] = info.name
    end
  end
  return names
end

local function ai_threat_name_list(flags)
  local names = {}
  if flags == 0 then
    names[#names + 1] = "stable"
    return names
  end

  for _, info in ipairs(AI_THREAT_NAMES) do
    if has_mask(flags, info.mask) then
      names[#names + 1] = info.name
    end
  end
  return names
end

local function ai_short_line_name_list(flags)
  local names = {}
  if flags == 0 then
    names[#names + 1] = "none"
    return names
  end

  for _, info in ipairs(AI_SHORT_LINE_NAMES) do
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
  local ai_reason = u8(offset + 20)
  local flags = u8(offset + 21)
  local ai_threat_flags = u8(offset + 22)
  local ai_risk_kind = u8(offset + 23)
  local ai_line_flags = u8(offset + 24)
  local ai_stable_line_family = u8(offset + 25)
  local ai_fallback_line_family = u8(offset + 26)
  local ai_loss_clock = u8(offset + 27)

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
    ai_candidate_rank = u8(offset + 13),
    ai_plan_id = u16(offset + 14),
    ai_trace_linked = u16(offset + 14) ~= 0,
    gimmick = gimmick,
    gimmick_name = gimmick_names[gimmick] or ("GIMMICK_" .. tostring(gimmick)),
    ai_reason = ai_reason,
    ai_reason_name = AI_REASON_NAMES[ai_reason] or ("AI_REASON_" .. tostring(ai_reason)),
    ai_threat_flags = ai_threat_flags,
    ai_threat_names = ai_threat_name_list(ai_threat_flags),
    ai_risk_kind = ai_risk_kind,
    ai_risk_name = AI_RISK_NAMES[ai_risk_kind] or ("AI_RISK_" .. tostring(ai_risk_kind)),
    ai_line_flags = ai_line_flags,
    ai_line_names = ai_short_line_name_list(ai_line_flags),
    ai_stable_line_family = ai_stable_line_family,
    ai_stable_line_name = AI_CANDIDATE_LINE_NAMES[ai_stable_line_family] or ("AI_CANDIDATE_LINE_" .. tostring(ai_stable_line_family)),
    ai_fallback_line_family = ai_fallback_line_family,
    ai_fallback_line_name = AI_CANDIDATE_LINE_NAMES[ai_fallback_line_family] or ("AI_CANDIDATE_LINE_" .. tostring(ai_fallback_line_family)),
    ai_loss_clock = ai_loss_clock,
    flags = flags,
    flag_names = flag_name_list(flags),
    valid = has_mask(flags, FLAG_VALID),
    resolved_switch_in = has_mask(flags, FLAG_RESOLVED),
    corrected_switch_in = has_mask(flags, FLAG_CORRECTED),
    selected_gimmick = has_mask(flags, FLAG_GIMMICK),
  }
end

local function trace_ring_indices(capacity, cursor, count)
  local indices = {}
  local start_index = 0
  if count >= capacity then
    count = capacity
    start_index = cursor
  end
  for i = 0, count - 1 do
    indices[#indices + 1] = (start_index + i) % capacity
  end
  return indices
end

local function read_trace_action(offset, move_names, item_names, gimmick_names, battler_positions)
  local actor_meta = u8(offset + 2)
  local target_meta = u8(offset + 3)
  local valid = has_mask(actor_meta, 128)
  local battler = actor_meta % 4
  local action_kind = math.floor(actor_meta / 4) % 4
  local move_slot = math.floor(actor_meta / 16) % 8
  local target = target_meta % 8
  local gimmick = math.floor(target_meta / 8) % 8
  local prediction_source = math.floor(target_meta / 64) % 4
  local choice = u16(offset)
  local choice_name = nil

  if action_kind == 1 then
    choice_name = move_names[choice] or ("MOVE_" .. tostring(choice))
  elseif action_kind == 2 then
    choice_name = "party_index_" .. tostring(choice)
  elseif action_kind == 3 then
    choice_name = item_names[choice] or ("ITEM_" .. tostring(choice))
  end

  return {
    valid = valid,
    battler = battler,
    battler_position = battler_positions[battler + 1],
    action_kind = action_kind,
    action_kind_name = AI_TRACE_ACTION_KIND_NAMES[action_kind] or ("action_kind_" .. tostring(action_kind)),
    choice = choice,
    choice_name = choice_name,
    target = target,
    target_position = target < #battler_positions and battler_positions[target + 1] or nil,
    move_slot = move_slot,
    gimmick = gimmick,
    gimmick_name = gimmick_names[gimmick] or ("GIMMICK_" .. tostring(gimmick)),
    prediction_source = prediction_source,
    prediction_source_name = AI_TRACE_PREDICTION_SOURCE_NAMES[prediction_source] or ("prediction_source_" .. tostring(prediction_source)),
  }
end

local function read_trace_candidate(base, ring_index, move_names, item_names, gimmick_names, battler_positions)
  local offset = base + AI_CANDIDATE_OFFSET + ring_index * AI_CANDIDATE_SIZE
  local rejection0 = u8(offset + 20)
  local rejection1 = u8(offset + 21)
  local read_interaction0 = u8(offset + 22)
  local read_interaction1 = u8(offset + 23)
  local ally_interaction0 = u8(offset + 24)
  local ally_interaction1 = u8(offset + 25)
  local flags = u8(offset + 27)
  local total_score = s16(offset + 8)
  local immediate_score = s16(offset + 10)
  local future_score = s16(offset + 12)
  local risk_score = s16(offset + 14)
  local resource_score = s16(offset + 16)

  return {
    ring_index = ring_index,
    sequence = u16(offset + 18),
    actions = {
      read_trace_action(offset + 0, move_names, item_names, gimmick_names, battler_positions),
      read_trace_action(offset + 4, move_names, item_names, gimmick_names, battler_positions),
    },
    score = {
      total = total_score,
      immediate = immediate_score,
      future = future_score,
      risk = risk_score,
      resource = resource_score,
      -- Planner invariant: total = immediate + future - risk + resource.
      unattributed = total_score - immediate_score - future_score + risk_score - resource_score,
    },
    rejection_flags_by_action = { rejection0, rejection1 },
    rejection_names_by_action = {
      mask_name_list(rejection0, AI_TRACE_REJECTION_NAMES),
      mask_name_list(rejection1, AI_TRACE_REJECTION_NAMES),
    },
    read_interaction_flags_by_action = { read_interaction0, read_interaction1 },
    read_interaction_names_by_action = {
      mask_name_list(read_interaction0, AI_TRACE_READ_INTERACTION_NAMES),
      mask_name_list(read_interaction1, AI_TRACE_READ_INTERACTION_NAMES),
    },
    ally_interaction_kinds_by_action = { ally_interaction0, ally_interaction1 },
    ally_interaction_names_by_action = {
      AI_TRACE_ALLY_INTERACTION_NAMES[ally_interaction0] or ("ally_interaction_" .. tostring(ally_interaction0)),
      AI_TRACE_ALLY_INTERACTION_NAMES[ally_interaction1] or ("ally_interaction_" .. tostring(ally_interaction1)),
    },
    eligible = rejection0 == 0 and rejection1 == 0,
    completed_depth = u8(offset + 26),
    flags = flags,
    flag_names = mask_name_list(flags, AI_TRACE_CANDIDATE_FLAG_NAMES),
    valid = has_mask(flags, 1),
    chosen = has_mask(flags, 2),
  }
end

local STAT_STAGE_NAMES = {
  "hp",
  "attack",
  "defense",
  "speed",
  "sp_attack",
  "sp_defense",
  "accuracy",
  "evasion",
}

local function read_trace_battler_state(offset, species_names, item_names)
  local stages = {}
  for stat = 0, 7 do
    local packed = u8(offset + 12 + math.floor(stat / 2))
    local raw = math.floor(packed / (stat % 2 == 0 and 1 or 16)) % 16
    stages[STAT_STAGE_NAMES[stat + 1]] = {
      raw = raw,
      delta = raw - 6,
    }
  end

  local species = u16(offset + 0)
  local item = u16(offset + 6)
  return {
    species = species,
    species_name = species_names[species] or ("SPECIES_" .. tostring(species)),
    hp = u16(offset + 2),
    max_hp = u16(offset + 4),
    item = item,
    item_name = item_names[item] or ("ITEM_" .. tostring(item)),
    status1 = u32(offset + 8),
    stat_stages = stages,
  }
end

local function read_trace_board(base, ring_index, species_names, item_names)
  local offset = base + AI_BOARD_OFFSET + ring_index * AI_BOARD_SIZE
  local meta = u16(offset + 6)
  local battlers = {}
  for battler = 0, 3 do
    battlers[#battlers + 1] = read_trace_battler_state(offset + 20 + battler * 16, species_names, item_names)
  end

  return {
    ring_index = ring_index,
    sequence = u16(offset + 0),
    plan_id = u16(offset + 2),
    weather = u16(offset + 4),
    phase = meta % 4,
    phase_name = AI_TRACE_BOARD_PHASE_NAMES[meta % 4] or ("board_phase_" .. tostring(meta % 4)),
    battler_mask = math.floor(meta / 4) % 16,
    timers_clamped = has_mask(meta, 64),
    valid = has_mask(meta, 32768),
    field_statuses = u32(offset + 8),
    side_statuses = { u32(offset + 12), u32(offset + 16) },
    battlers = battlers,
    timers = {
      tailwind = { u8(offset + 84), u8(offset + 85) },
      reflect = { u8(offset + 86), u8(offset + 87) },
      light_screen = { u8(offset + 88), u8(offset + 89) },
      aurora_veil = { u8(offset + 90), u8(offset + 91) },
      trick_room = u8(offset + 92),
      terrain = u8(offset + 93),
      gravity = u8(offset + 94),
      magic_room = u8(offset + 95),
    },
  }
end

local function read_trace_plan(base, ring_index, move_names, item_names, gimmick_names, battler_positions)
  local offset = base + AI_PLAN_OFFSET + ring_index * AI_PLAN_SIZE
  local flags = u8(offset + 30)
  return {
    ring_index = ring_index,
    plan_id = u16(offset + 0),
    turn = u16(offset + 2),
    first_candidate_sequence = u16(offset + 4),
    board_sequence = u16(offset + 6),
    search = {
      nodes_visited = u16(offset + 8),
      node_budget = u16(offset + 10),
      cache_hits = u16(offset + 12),
      elapsed_frames = u16(offset + 14),
      frame_budget = u8(offset + 31),
      requested_depth = u8(offset + 27),
      completed_depth = u8(offset + 28),
      termination_reason = u8(offset + 29),
      termination_reason_name = AI_TRACE_TERMINATION_NAMES[u8(offset + 29)] or ("termination_" .. tostring(u8(offset + 29))),
    },
    predicted_player_actions = {
      read_trace_action(offset + 16, move_names, item_names, gimmick_names, battler_positions),
      read_trace_action(offset + 20, move_names, item_names, gimmick_names, battler_positions),
    },
    actor_mask = u8(offset + 24),
    candidate_count = u8(offset + 25),
    chosen_rank = u8(offset + 26),
    flags = flags,
    flag_names = mask_name_list(flags, AI_TRACE_PLAN_FLAG_NAMES),
    valid = has_mask(flags, 1),
  }
end

local function action_entry_as_trace_action(entry)
  local action_kind = 0
  local choice = 0
  if entry.action == 0 then
    action_kind = 1
    choice = entry.move
  elseif entry.action == 2 then
    action_kind = 2
    choice = entry.party_index
  elseif entry.action == 1 then
    action_kind = 3
    choice = entry.item
  end

  return {
    valid = action_kind ~= 0,
    battler = entry.battler,
    action_kind = action_kind,
    action_kind_name = AI_TRACE_ACTION_KIND_NAMES[action_kind] or ("action_kind_" .. tostring(action_kind)),
    choice = choice,
    target = entry.target,
    move_slot = entry.move_slot,
    gimmick = entry.gimmick,
    action_log_sequence = entry.sequence,
  }
end

local function compare_trace_action(predicted, actual)
  if predicted == nil or not predicted.valid then
    return nil
  end
  if actual == nil then
    return {
      predicted = predicted,
      actual = nil,
      available = false,
    }
  end

  local kind_match = predicted.action_kind == actual.action_kind
  local choice_match = predicted.choice == actual.choice
  local target_match = predicted.target == actual.target
  local move_slot_match = predicted.action_kind ~= 1 or predicted.move_slot == actual.move_slot
  local gimmick_match = predicted.action_kind ~= 1 or predicted.gimmick == actual.gimmick
  return {
    predicted = predicted,
    actual = actual,
    available = true,
    action_kind_match = kind_match,
    choice_match = choice_match,
    target_match = target_match,
    move_slot_match = move_slot_match,
    gimmick_match = gimmick_match,
    full_match = kind_match and choice_match and target_match and move_slot_match and gimmick_match,
  }
end

local root = _G.BATTLE_ACTION_LOG_ROOT or os.getenv("POKEEMERALD_ROOT") or "."
local map_path = _G.BATTLE_ACTION_LOG_MAP or join_path(root, "pokeemerald.map")
local out_path = _G.BATTLE_ACTION_LOG_OUT or os.getenv("BATTLE_ACTION_LOG_OUT") or "/tmp/pokeemerald-battle-action-log.json"

local log_addr = _G.BATTLE_ACTION_LOG_ADDR or find_symbol(map_path, "gBattleActionLog")
assert(log_addr ~= nil, "could not find gBattleActionLog in " .. map_path)

local ai_trace_symbol_addr = _G.BATTLE_AI_TRACE_LOG_ADDR or find_symbol(map_path, "gBattleAiTraceLog")
local ai_trace_addr = nil
local ai_trace_detected_version = nil
local ai_trace_detected_magic = nil
if ai_trace_symbol_addr ~= nil then
  local trace_header_addr = ai_trace_symbol_addr + AI_TRACE_HEADER_OFFSET
  ai_trace_detected_version = u8(trace_header_addr + 13)
  ai_trace_detected_magic = u16(trace_header_addr + 14)
  if ai_trace_detected_version == AI_TRACE_SCHEMA_VERSION and ai_trace_detected_magic == AI_TRACE_HEADER_MAGIC then
    ai_trace_addr = ai_trace_symbol_addr
  end
end
local battler_positions_addr = _G.BATTLER_POSITIONS_ADDR or find_symbol(map_path, "gBattlerPositions")

local move_names = load_enum_names(join_path(root, "include/constants/moves.h"), "MOVE_")
local item_names = load_enum_names(join_path(root, "include/constants/items.h"), "ITEM_")
local species_names = load_enum_names(join_path(root, "include/constants/species.h"), "SPECIES_")
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

local actual_actions_by_turn_battler = {}
for _, entry in ipairs(entries) do
  if not entry.resolved_switch_in then
    actual_actions_by_turn_battler[tostring(entry.turn) .. ":" .. tostring(entry.battler)] = action_entry_as_trace_action(entry)
  end
end

local ai_trace_header = nil
local ai_plans = {}
local ai_candidates = {}
local ai_boards = {}
if ai_trace_addr ~= nil then
  local trace_header_addr = ai_trace_addr + AI_TRACE_HEADER_OFFSET
  local plan_sequence = u16(trace_header_addr + 0)
  local candidate_sequence = u16(trace_header_addr + 2)
  local board_sequence = u16(trace_header_addr + 4)
  local candidate_count = u16(trace_header_addr + 6)
  local plan_cursor = u8(trace_header_addr + 8)
  local plan_count = u8(trace_header_addr + 9)
  local candidate_cursor = u8(trace_header_addr + 10)
  local board_cursor = u8(trace_header_addr + 11)
  local board_count = u8(trace_header_addr + 12)
  local trace_schema_version = u8(trace_header_addr + 13)
  local trace_magic = u16(trace_header_addr + 14)

  if plan_count > AI_PLAN_CAPACITY then plan_count = AI_PLAN_CAPACITY end
  if candidate_count > AI_CANDIDATE_CAPACITY then candidate_count = AI_CANDIDATE_CAPACITY end
  if board_count > AI_BOARD_CAPACITY then board_count = AI_BOARD_CAPACITY end

  ai_trace_header = {
    plan_sequence = plan_sequence,
    candidate_sequence = candidate_sequence,
    board_sequence = board_sequence,
    plan_cursor = plan_cursor,
    plan_count = plan_count,
    candidate_cursor = candidate_cursor,
    candidate_count = candidate_count,
    board_cursor = board_cursor,
    board_count = board_count,
    schema_version = trace_schema_version,
    magic = trace_magic,
  }

  local candidates_by_sequence = {}
  for _, ring_index in ipairs(trace_ring_indices(AI_CANDIDATE_CAPACITY, candidate_cursor, candidate_count)) do
    local candidate = read_trace_candidate(ai_trace_addr, ring_index, move_names, item_names, gimmick_names, battler_positions)
    if candidate.valid then
      ai_candidates[#ai_candidates + 1] = candidate
      candidates_by_sequence[candidate.sequence] = candidate
    end
  end

  local boards_by_sequence = {}
  local boards_by_plan_phase = {}
  for _, ring_index in ipairs(trace_ring_indices(AI_BOARD_CAPACITY, board_cursor, board_count)) do
    local board = read_trace_board(ai_trace_addr, ring_index, species_names, item_names)
    if board.valid then
      ai_boards[#ai_boards + 1] = board
      boards_by_sequence[board.sequence] = board
      boards_by_plan_phase[tostring(board.plan_id) .. ":" .. tostring(board.phase)] = board
    end
  end

  local function next_trace_sequence(sequence)
    sequence = (sequence + 1) % 0x10000
    if sequence == 0 then sequence = 1 end
    return sequence
  end

  for _, ring_index in ipairs(trace_ring_indices(AI_PLAN_CAPACITY, plan_cursor, plan_count)) do
    local plan = read_trace_plan(ai_trace_addr, ring_index, move_names, item_names, gimmick_names, battler_positions)
    if plan.valid then
      local expected_sequence = plan.first_candidate_sequence
      local candidates = {}
      local missing_candidate_sequences = {}
      for rank = 0, plan.candidate_count - 1 do
        local candidate = candidates_by_sequence[expected_sequence]
        if candidate ~= nil then
          candidate.trace_rank = rank
          candidate.plan_id = plan.plan_id
          candidate.root_board_sequence = plan.board_sequence
          candidates[#candidates + 1] = candidate
        else
          missing_candidate_sequences[#missing_candidate_sequences + 1] = expected_sequence
        end
        expected_sequence = next_trace_sequence(expected_sequence)
      end
      plan.candidates = candidates
      plan.candidate_trace_complete = #missing_candidate_sequences == 0
      plan.missing_candidate_sequences = missing_candidate_sequences
      plan.board_before = boards_by_sequence[plan.board_sequence]
      plan.board_predicted_after = boards_by_plan_phase[tostring(plan.plan_id) .. ":2"]
      plan.board_actual_after = boards_by_plan_phase[tostring(plan.plan_id) .. ":3"]
      plan.board_snapshot_availability = {
        before = plan.board_before ~= nil,
        predicted_after = plan.board_predicted_after ~= nil,
        actual_after = plan.board_actual_after ~= nil,
      }
      plan.selected_vs_realized_board = {
        available = plan.board_predicted_after ~= nil and plan.board_actual_after ~= nil,
        predicted_sequence = plan.board_predicted_after ~= nil and plan.board_predicted_after.sequence or nil,
        actual_sequence = plan.board_actual_after ~= nil and plan.board_actual_after.sequence or nil,
      }

      local prediction_results = {}
      for _, predicted in ipairs(plan.predicted_player_actions) do
        if predicted.valid then
          local actual = actual_actions_by_turn_battler[tostring(plan.turn) .. ":" .. tostring(predicted.battler)]
          prediction_results[#prediction_results + 1] = compare_trace_action(predicted, actual)
        end
      end
      plan.prediction_results = prediction_results

      local chosen = candidates_by_sequence[plan.first_candidate_sequence]
      if plan.chosen_rank ~= 255 then
        local chosen_sequence = plan.first_candidate_sequence
        for _ = 1, plan.chosen_rank do
          chosen_sequence = next_trace_sequence(chosen_sequence)
        end
        chosen = candidates_by_sequence[chosen_sequence]
      else
        chosen = nil
      end
      plan.chosen_candidate = chosen
      plan.chosen_actual_results = {}
      if chosen ~= nil then
        for _, selected_action in ipairs(chosen.actions) do
          if selected_action.valid then
            local actual = actual_actions_by_turn_battler[tostring(plan.turn) .. ":" .. tostring(selected_action.battler)]
            plan.chosen_actual_results[#plan.chosen_actual_results + 1] = compare_trace_action(selected_action, actual)
          end
        end
      end
      ai_plans[#ai_plans + 1] = plan
    end
  end
end

local frame = nil
if emu.currentFrame ~= nil then
  frame = emu:currentFrame()
end

local payload = {
  schema = ai_trace_addr ~= nil and "pokeemerald.battle_action_log.v5" or "pokeemerald.battle_action_log.v4",
  generated_at_utc = os.date("!%Y-%m-%dT%H:%M:%SZ"),
  frame = frame,
  source = {
    map_path = map_path,
    gBattleActionLog = hex(log_addr),
    gBattleAiTraceLog = ai_trace_addr ~= nil and hex(ai_trace_addr) or nil,
    gBattleAiTraceLog_symbol = ai_trace_symbol_addr ~= nil and hex(ai_trace_symbol_addr) or nil,
    ai_trace_detected_version = ai_trace_detected_version,
    ai_trace_detected_magic = ai_trace_detected_magic,
    ai_trace_signature_valid = ai_trace_addr ~= nil,
    gBattlerPositions = battler_positions_addr ~= nil and hex(battler_positions_addr) or nil,
    entry_capacity = ENTRY_CAPACITY,
    entry_size = ENTRY_SIZE,
    ai_plan_capacity = ai_trace_addr ~= nil and AI_PLAN_CAPACITY or nil,
    ai_plan_size = ai_trace_addr ~= nil and AI_PLAN_SIZE or nil,
    ai_candidate_capacity = ai_trace_addr ~= nil and AI_CANDIDATE_CAPACITY or nil,
    ai_candidate_size = ai_trace_addr ~= nil and AI_CANDIDATE_SIZE or nil,
    ai_board_capacity = ai_trace_addr ~= nil and AI_BOARD_CAPACITY or nil,
    ai_board_size = ai_trace_addr ~= nil and AI_BOARD_SIZE or nil,
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
  ai_trace_header = ai_trace_header,
  ai_plans = ai_plans,
  ai_candidates = ai_candidates,
  ai_board_snapshots = ai_boards,
}

if _G.BATTLE_ACTION_LOG_SKIP_EMPTY == true and #entries == 0 then
  return {
    output = out_path,
    schema = payload.schema,
    entries = #entries,
    sequence = sequence,
    last_recorded_turn = last_recorded_turn,
    cursor = cursor,
    count = count,
    skipped_invalid = skipped_invalid,
    skipped_empty = true,
    ai_plans = #ai_plans,
    ai_candidates = #ai_candidates,
    frame = frame,
  }
end

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
  ai_plans = #ai_plans,
  ai_candidates = #ai_candidates,
  frame = frame,
}
