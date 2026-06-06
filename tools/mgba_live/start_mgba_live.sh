#!/usr/bin/env sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  tools/mgba_live/start_mgba_live.sh [SESSION] [FPS] [ROM]

Environment:
  MGBA_LIVE_CLI  mgba-live-cli path or command name
  FPS_TARGET     FPS target, default 120
  ROM            ROM path, default ./pokeemerald.gba
  READY_TIMEOUT  bridge ready timeout in seconds, default 20
  VIDEO_SYNC     set to 0 to skip videoSync=1, default 1

Examples:
  tools/mgba_live/start_mgba_live.sh
  tools/mgba_live/start_mgba_live.sh manual-ai-log 120
  FPS_TARGET=180 tools/mgba_live/start_mgba_live.sh manual-ai-log
EOF
}

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)

session="${1:-manual-mgba-$(date +%Y%m%d-%H%M%S)}"
fps="${2:-${FPS_TARGET:-120}}"
rom="${3:-${ROM:-$project_root/pokeemerald.gba}}"
ready_timeout="${READY_TIMEOUT:-20}"
video_sync="${VIDEO_SYNC:-1}"

if [ -n "${MGBA_LIVE_CLI:-}" ]; then
  mgba_live_cli="$MGBA_LIVE_CLI"
elif [ -x "/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli" ]; then
  mgba_live_cli="/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli"
else
  mgba_live_cli="mgba-live-cli"
fi

if [ ! -f "$rom" ]; then
  echo "ROM not found: $rom" >&2
  exit 2
fi

export DISPLAY="${DISPLAY:-:0}"

if [ "$video_sync" = "0" ]; then
  "$mgba_live_cli" start \
    --rom "$rom" \
    --session-id "$session" \
    --fps-target "$fps" \
    --ready-timeout "$ready_timeout"
else
  "$mgba_live_cli" start \
    --rom "$rom" \
    --session-id "$session" \
    --fps-target "$fps" \
    --ready-timeout "$ready_timeout" \
    --config videoSync=1
fi
