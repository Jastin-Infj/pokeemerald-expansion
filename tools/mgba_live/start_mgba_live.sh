#!/usr/bin/env sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  tools/mgba_live/start_mgba_live.sh [SESSION] [FPS] [ROM]

Environment:
  MGBA_LIVE_CLI  direct mgba-live-cli path or command override
  MGBA_LIVE_UVX  uvx path used when mgba-live-cli is not installed
  MGBA_LIVE_RUNTIME_ROOT
                 session state directory, default .cache/mgba-live-runtime
  FPS_TARGET     FPS target, default 120
  ROM            ROM path, default ./pokeemerald.gba
  READY_TIMEOUT  bridge ready timeout in seconds, default 20
  VIDEO_SYNC     set to 0 to skip videoSync=1, default 1
  BATTLE_ACTION_LOG_AUTOSAVE
                  set to 0 to skip battle action log autosave, default 1
  BATTLE_ACTION_LOG_OUT
                  autosave output, default /tmp/SESSION-battle-action-log-autosave.json
  BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL
                  autosave interval in frames, default 120

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
autosave="${BATTLE_ACTION_LOG_AUTOSAVE:-1}"
autosave_script="$script_dir/battle_action_log_autosave.lua"

mgba_live_cli="$script_dir/mgba_live_cli.sh"

if [ ! -f "$rom" ]; then
  echo "ROM not found: $rom" >&2
  exit 2
fi

export DISPLAY="${DISPLAY:-:0}"

script_args=""
if [ "$autosave" != "0" ]; then
  export POKEEMERALD_ROOT="${POKEEMERALD_ROOT:-$project_root}"
  export BATTLE_ACTION_LOG_EXPORTER="${BATTLE_ACTION_LOG_EXPORTER:-$script_dir/battle_action_log_export.lua}"
  export BATTLE_ACTION_LOG_OUT="${BATTLE_ACTION_LOG_OUT:-/tmp/${session}-battle-action-log-autosave.json}"
  export BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL="${BATTLE_ACTION_LOG_AUTOSAVE_INTERVAL:-120}"
  script_args="--script $autosave_script"
fi

if [ "$video_sync" = "0" ]; then
  "$mgba_live_cli" start \
    --rom "$rom" \
    --session-id "$session" \
    --fps-target "$fps" \
    --ready-timeout "$ready_timeout" \
    $script_args
else
  "$mgba_live_cli" start \
    --rom "$rom" \
    --session-id "$session" \
    --fps-target "$fps" \
    --ready-timeout "$ready_timeout" \
    --config videoSync=1 \
    $script_args
fi
