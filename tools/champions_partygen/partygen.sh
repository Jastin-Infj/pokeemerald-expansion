#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
if [ "$#" -eq 0 ]; then
    exec cargo run --manifest-path "$script_dir/Cargo.toml" -- --help
fi
cmd=$1
shift
case "$cmd" in
    audit|logs|profile)
        if [ "$#" -eq 0 ]; then
            exec cargo run --manifest-path "$script_dir/Cargo.toml" -- "$cmd" --rom-repo "$repo_root"
        fi
        sub=$1
        shift
        exec cargo run --manifest-path "$script_dir/Cargo.toml" -- "$cmd" "$sub" --rom-repo "$repo_root" "$@"
        ;;
    *)
        exec cargo run --manifest-path "$script_dir/Cargo.toml" -- "$cmd" --rom-repo "$repo_root" "$@"
        ;;
esac
