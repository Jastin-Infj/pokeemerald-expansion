#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
core_dir="$repo_root/tools/map_asset_relinker_core"

cargo build --release --manifest-path "$core_dir/Cargo.toml"

binary="$core_dir/target/release/map-asset-relinker-core"
if [ ! -x "$binary" ]; then
  echo "Expected executable was not created: $binary" >&2
  exit 1
fi

echo "Core executable:"
echo "  $binary"
