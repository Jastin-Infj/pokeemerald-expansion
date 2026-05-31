#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
app_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
cache_dir="$app_dir/.cache/tauri-apt-cache"
deps_dir="$app_dir/.cache/tauri-linux-deps"
archive_dir="$cache_dir/archives"
lib_dir="$deps_dir/usr/lib/x86_64-linux-gnu"

mkdir -p "$archive_dir/partial" "$deps_dir"

apt-get -y --download-only \
  -o Debug::NoLocking=true \
  -o Dir::Cache::archives="$archive_dir" \
  install \
  libgtk-3-dev \
  libjavascriptcoregtk-4.1-dev \
  libsoup-3.0-dev \
  libwebkit2gtk-4.1-dev

set -- "$archive_dir"/*.deb
if [ ! -e "$1" ]; then
  echo "No .deb files were downloaded into $archive_dir" >&2
  exit 1
fi

for deb in "$@"; do
  dpkg-deb -x "$deb" "$deps_dir"
done

# The -dev packages can contain unversioned local .so symlinks while the
# matching runtime library is already installed on the host. Complete those
# symlink chains so the linker can use this local pkg-config sysroot.
if [ -d "$lib_dir" ]; then
  for link in "$lib_dir"/*.so; do
    [ -L "$link" ] || continue
    target=$(readlink "$link")
    target_name=$(basename "$target")
    missing="$lib_dir/$target_name"
    host_target="/usr/lib/x86_64-linux-gnu/$target_name"
    if [ ! -e "$missing" ] && [ -e "$host_target" ]; then
      ln -s "$host_target" "$missing"
    fi
  done
fi

echo "Local Linux dependency sysroot:"
echo "  $deps_dir"
echo ""
echo "Build with:"
echo "  TAURI_LOCAL_DEPS=$deps_dir $app_dir/scripts/build_native.sh"
