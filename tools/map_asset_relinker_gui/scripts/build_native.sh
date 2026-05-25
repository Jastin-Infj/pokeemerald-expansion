#!/usr/bin/env sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
app_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)

missing=""
for package in gdk-3.0 gtk+-3.0 javascriptcoregtk-4.1 libsoup-3.0 webkit2gtk-4.1; do
  if ! pkg-config --exists "$package"; then
    missing="$missing $package"
  fi
done

if [ -n "$missing" ]; then
  echo "Missing Tauri Linux pkg-config modules:$missing" >&2
  echo "" >&2
  echo "These are pkg-config module names, not apt package names." >&2
  echo "Ubuntu package mapping:" >&2
  echo "  gdk-3.0                 -> libgtk-3-dev" >&2
  echo "  gtk+-3.0                -> libgtk-3-dev" >&2
  echo "  javascriptcoregtk-4.1   -> libjavascriptcoregtk-4.1-dev" >&2
  echo "  libsoup-3.0             -> libsoup-3.0-dev" >&2
  echo "  webkit2gtk-4.1          -> libwebkit2gtk-4.1-dev" >&2
  echo "" >&2
  echo "On Ubuntu, run:" >&2
  echo "  $app_dir/scripts/install_ubuntu_deps.sh" >&2
  echo "" >&2
  echo "Equivalent apt command:" >&2
  echo "  sudo apt-get update && sudo apt-get install -y build-essential curl file libayatana-appindicator3-dev libgtk-3-dev libjavascriptcoregtk-4.1-dev librsvg2-dev libsoup-3.0-dev libssl-dev libwebkit2gtk-4.1-dev libxdo-dev patchelf pkg-config wget" >&2
  echo "" >&2
  echo "Then rerun:" >&2
  echo "  $app_dir/scripts/build_native.sh" >&2
  exit 1
fi

cd "$app_dir"
npm install
npm run tauri build

echo ""
echo "Native build output:"
find "$app_dir/src-tauri/target/release" -maxdepth 3 \
  \( -type f -o -type l \) \
  \( -perm -111 -o -name '*.deb' -o -name '*.rpm' -o -name '*.AppImage' \) \
  -print
