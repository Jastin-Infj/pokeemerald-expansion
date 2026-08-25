#!/usr/bin/env sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  tools/mgba_live/run_lua.sh SESSION LUA_FILE [TIMEOUT]

Environment:
  MGBA_LIVE_RUNTIME_ROOT  session state directory, default .cache/mgba-live-runtime
  MGBA_LIVE_CLI           direct mgba-live-cli path or command override
  MGBA_LIVE_UVX           uvx path used when mgba-live-cli is not installed
EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  usage >&2
  exit 2
fi

session="$1"
lua_file="$2"
timeout="${3:-${TIMEOUT:-20}}"

if [ ! -f "$lua_file" ]; then
  echo "Lua file not found: $lua_file" >&2
  exit 2
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
exec "$script_dir/mgba_live_cli.sh" run-lua \
  --session "$session" \
  --file "$lua_file" \
  --timeout "$timeout"
