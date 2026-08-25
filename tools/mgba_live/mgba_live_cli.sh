#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
project_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
runtime_root="${MGBA_LIVE_RUNTIME_ROOT:-$project_root/.cache/mgba-live-runtime}"
export MGBA_LIVE_RUNTIME_ROOT="$runtime_root"

resolve_command() {
  candidate="$1"
  case "$candidate" in
    */*)
      if [ -x "$candidate" ]; then
        printf '%s\n' "$candidate"
      fi
      ;;
    *)
      command -v "$candidate" 2>/dev/null || true
      ;;
  esac
}

cli_path=""
if [ -n "${MGBA_LIVE_CLI:-}" ]; then
  cli_path=$(resolve_command "$MGBA_LIVE_CLI")
fi

if [ "$cli_path" = "" ] && [ -x "/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli" ]; then
  cli_path="/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli"
fi

if [ "$cli_path" = "" ]; then
  cli_path=$(resolve_command mgba-live-cli)
fi

if [ "$cli_path" != "" ]; then
  package_root=$(CDPATH= cd -- "$(dirname -- "$cli_path")/.." && pwd)
  python_path="$package_root/bin/python"
  if [ -x "$python_path" ]; then
    exec "$python_path" "$script_dir/mgba_live_cli.py" "$@"
  fi
  exec "$cli_path" "$@"
fi

uvx_path="${MGBA_LIVE_UVX:-}"
if [ "$uvx_path" = "" ] && [ -x "/home/jastin/.local/bin/uvx" ]; then
  uvx_path="/home/jastin/.local/bin/uvx"
fi
if [ "$uvx_path" = "" ]; then
  uvx_path=$(resolve_command uvx)
fi

if [ "$uvx_path" != "" ]; then
  exec "$uvx_path" --from mgba-live-mcp python "$script_dir/mgba_live_cli.py" "$@"
fi

echo "mGBA Live CLI not found. Set MGBA_LIVE_CLI or MGBA_LIVE_UVX." >&2
exit 127
