#!/usr/bin/env sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  tools/mgba_live/export_battle_action_log.sh [SESSION] [OUT_JSON]

Environment:
  MGBA_LIVE_CLI  direct mgba-live-cli path or command override
  MGBA_LIVE_UVX  uvx path used when mgba-live-cli is not installed
  MGBA_LIVE_RUNTIME_ROOT
                 session state directory, default .cache/mgba-live-runtime
  TIMEOUT        run-lua timeout in seconds, default 8

Examples:
  tools/mgba_live/export_battle_action_log.sh
  tools/mgba_live/export_battle_action_log.sh smart-gimmick-dmax-double-log-20260606 /tmp/battle-action-log.json
EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

out_json="${2:-/tmp/pokeemerald-battle-action-log.json}"
timeout="${TIMEOUT:-8}"

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
lua_script="$script_dir/battle_action_log_export.lua"
runtime_root="${MGBA_LIVE_RUNTIME_ROOT:-$project_root/.cache/mgba-live-runtime}"

mgba_live_cli="$script_dir/mgba_live_cli.sh"

status_sessions() {
  "$mgba_live_cli" status --all 2>/dev/null || printf '[]'
}

json_first_session() {
  python3 -c 'import json,sys
try:
    data=json.load(sys.stdin)
except Exception:
    data=[]
print(data[0].get("session_id","") if data else "")' 2>/dev/null || true
}

json_has_session() {
  wanted="$1"
  python3 -c 'import json,sys
wanted=sys.argv[1]
try:
    data=json.load(sys.stdin)
except Exception:
    data=[]
print("yes" if any(item.get("session_id") == wanted for item in data) else "no")' "$wanted" 2>/dev/null || printf 'no'
}

active_session_file="$runtime_root/active_session"
if [ "${1:-}" = "" ]; then
  status_json=$(status_sessions)
  session=""
  if [ -r "$active_session_file" ]; then
    candidate=$(sed -n '1p' "$active_session_file")
    if [ "$candidate" != "" ] && [ "$(printf '%s' "$status_json" | json_has_session "$candidate")" = "yes" ]; then
      session="$candidate"
    fi
  fi
  if [ "$session" = "" ]; then
    session=$(printf '%s' "$status_json" | json_first_session)
  fi
else
  session="$1"
fi

if [ "$session" = "" ]; then
  echo "No live mGBA Live session found. Start one first:" >&2
  echo "  tools/mgba_live/start_mgba_live.sh manual-ai-log 120" >&2
  exit 2
fi

lua_escape() {
  printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

project_root_lua=$(lua_escape "$project_root")
out_json_lua=$(lua_escape "$out_json")
lua_script_lua=$(lua_escape "$lua_script")

code="_G.BATTLE_ACTION_LOG_ROOT=\"$project_root_lua\"; _G.BATTLE_ACTION_LOG_OUT=\"$out_json_lua\"; return dofile(\"$lua_script_lua\")"

"$mgba_live_cli" run-lua \
  --session "$session" \
  --code "$code" \
  --timeout "$timeout"
