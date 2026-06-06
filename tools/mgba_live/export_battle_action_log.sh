#!/usr/bin/env sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  tools/mgba_live/export_battle_action_log.sh SESSION [OUT_JSON]

Environment:
  MGBA_LIVE_CLI  mgba-live-cli path or command name
  TIMEOUT        run-lua timeout in seconds, default 8

Example:
  tools/mgba_live/export_battle_action_log.sh smart-gimmick-dmax-double-log-20260606 /tmp/battle-action-log.json
EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

if [ "${1:-}" = "" ]; then
  usage >&2
  exit 2
fi

session="$1"
out_json="${2:-/tmp/pokeemerald-battle-action-log.json}"
timeout="${TIMEOUT:-8}"

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
lua_script="$script_dir/battle_action_log_export.lua"

if [ -n "${MGBA_LIVE_CLI:-}" ]; then
  mgba_live_cli="$MGBA_LIVE_CLI"
elif [ -x "/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli" ]; then
  mgba_live_cli="/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli"
else
  mgba_live_cli="mgba-live-cli"
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
