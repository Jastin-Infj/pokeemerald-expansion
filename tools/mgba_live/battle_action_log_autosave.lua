-- Periodically export the in-ROM gBattleActionLog while mGBA Live is running.
--
-- This is intended as a startup script passed with mgba-live-cli start --script.
-- It keeps the latest non-empty battle action log on the host so a session
-- close does not lose the last useful EWRAM snapshot.

local function join_path(root, leaf)
  if root == "" or root == "." then
    return "./" .. leaf
  end
  if root:sub(-1) == "/" then
    return root .. leaf
  end
  return root .. "/" .. leaf
end

local root = os.getenv("POKEEMERALD_ROOT") or "."
local exporter = os.getenv("BATTLE_ACTION_LOG_EXPORTER") or join_path(root, "tools/mgba_live/battle_action_log_export.lua")
local out_path = os.getenv("BATTLE_ACTION_LOG_OUT") or "/tmp/pokeemerald-battle-action-log-autosave.json"
local interval = tonumber(os.getenv("BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL") or "120") or 120

if interval < 1 then
  interval = 120
end

local autosave = {
  frame = 0,
  last_sequence = nil,
  last_error = nil,
}

callbacks:add("frame", function()
  autosave.frame = autosave.frame + 1
  if autosave.frame % interval ~= 0 then
    return
  end

  _G.BATTLE_ACTION_LOG_ROOT = root
  _G.BATTLE_ACTION_LOG_OUT = out_path
  _G.BATTLE_ACTION_LOG_SKIP_EMPTY = true

  local ok, result = pcall(dofile, exporter)
  if not ok then
    autosave.last_error = tostring(result)
    return
  end

  if result ~= nil and result.entries ~= nil and result.entries > 0 then
    autosave.last_sequence = result.sequence
    autosave.last_error = nil
  end
end)

_G.BATTLE_ACTION_LOG_AUTOSAVE = autosave
