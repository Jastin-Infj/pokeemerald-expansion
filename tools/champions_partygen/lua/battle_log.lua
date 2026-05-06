-- Champions Partygen battle log hook (skeleton).
-- Designed to be loaded by mGBA Live MCP via mgba_live_start_with_lua.
-- Writes one line per battle event to LOG_PATH below. The line format is:
--   <iso8601>\t<event>\t<key>=<value>\t...
-- Subsequent processing by `partygen logs normalize` turns these lines into
-- JSONL rows (see docs/features/champions_challenge/partygen_player_style_logging.md).
--
-- This script does not yet wire into mGBA's battle internals. It is the
-- canonical landing pad for partygen mGBA-side hooks once memory addresses
-- for battle events are confirmed via mGBA Live reads.

local LOG_PATH = os.getenv("PARTYGEN_RAW_LOG")
    or "tools/champions_partygen/local/logs/raw/session.log"

local function ensure_dir(path)
    -- Ensure the parent directory exists. mGBA Lua does not expose mkdir -p
    -- so this script relies on the caller (partygen_collect.sh, etc.) to
    -- create the directory ahead of time.
end

local function iso_now()
    return os.date("!%Y-%m-%dT%H:%M:%SZ")
end

local function format_kv(pairs_)
    local out = {}
    for k, v in pairs(pairs_) do
        table.insert(out, tostring(k) .. "=" .. tostring(v))
    end
    table.sort(out)
    return table.concat(out, "\t")
end

local function emit(event, fields)
    ensure_dir(LOG_PATH)
    local f, err = io.open(LOG_PATH, "a")
    if not f then
        if console then console:log("partygen battle_log open failed: " .. tostring(err)) end
        return
    end
    f:write(iso_now())
    f:write("\t")
    f:write(event)
    if fields then
        f:write("\t")
        f:write(format_kv(fields))
    end
    f:write("\n")
    f:close()
end

-- Public surface used by partygen_collect helpers and integration tests.
partygen_battle_log = {
    emit = emit,
    log_path = LOG_PATH,
}

emit("session_start", {schemaVersion = 1, source = "battle_log.lua"})
