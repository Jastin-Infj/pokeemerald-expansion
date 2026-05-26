# Map Asset Relinker GUI

Tauri + React desktop shell for the map asset relinker.

This is intentionally a desktop app, not a browser-only localhost workflow.
During development, Tauri starts the Vite dev server at `127.0.0.1:1420`.
Production builds embed the compiled frontend in the Tauri bundle, so normal
use is an executable / AppImage / platform bundle and does not require manually
opening a browser or remembering a localhost URL.

## Current Scope

- Scan a pokeemerald-expansion project root.
- Read `data/maps/map_groups.json`, `data/maps/*/map.json`,
  `data/layouts/layouts.json`, and
  `src/data/region_map/region_map_sections.json`.
- Show map, group, layout, mapsec, map type, popup, and audit warning state.
- Flag temporary-looking map names such as `test1` and mixed-case mapsec ids
  such as `MAPSEC_Jongle`.
- Let map-linked audit warnings in the right pane select the affected map.
- Show the selected map's `Map -> Group -> Layout -> Mapsec` relationship as a
  compact graph before the raw field grid.
- Filter maps and generate a reviewable CLI plan command for a selected map.
- Suggest a production map name, target map group, normalized mapsec id, and
  mapsec display name when a temporary map is anchored to a mapsec.
- Include `--rename-mapsec` / `--new-mapsec-name` in the generated dry-run
  command when the normalized mapsec id differs from the selected mapsec.
- Enable script-label rewrite by default for temporary-looking map names, so
  the GUI plan matches the CLI `plan-temp-mapsec` shortcut.
- Run Rust-core plan generation plus Rust-core `apply --dry-run` and display
  the planned moves/edits plus move/edit/review counts without changing source
  files.
- After a successful dry-run, enable `Apply With Backup`. The GUI requires an
  explicit confirmation, runs the same CLI real `apply --allow-dirty` path,
  creates the normal `.map_asset_relinker_backups/*.bak.tar` archive before
  editing files, then rescans the project and shows the backup path.
- Keep layout rename included by default for rename plans. The UI shows it as a
  locked checked option because map rename repair normally needs layout id/name
  and layout directory updates too.
- Keep the shared core / CLI as the source of truth for relink behavior; the
  GUI only orchestrates plan, dry-run, real apply, backup reporting, and rescan.
- Project scan / warning summaries and Tauri plan generation now use the shared
  Rust core crate under `tools/map_asset_relinker_core/`. Tauri dry-run also
  uses Rust core `apply --dry-run`. Real apply / backup still use the Python
  CLI until the Rust core reaches write parity.

## Development

```bash
cd tools/map_asset_relinker_gui
npm install
npm run tauri dev
```

The development command opens a native Tauri window. The localhost dev server
is an implementation detail of hot reload.

For browser-only validation, open `http://127.0.0.1:1420/` while the Vite
server is running. The dev server exposes `/api/scan`, `/api/dry-run`, and
`/api/apply` endpoints that use Node filesystem access and the Python CLI.
Production Tauri builds use the Rust commands instead.

On Linux, Tauri also needs the platform webview / GTK development packages.
For Debian/Ubuntu-like systems, install the Tauri prerequisites including
`libwebkit2gtk-4.1-dev`, `libjavascriptcoregtk-4.1-dev`,
`libsoup-3.0-dev`, `libgtk-3-dev`, `libayatana-appindicator3-dev`,
`librsvg2-dev`, `libxdo-dev`, `libssl-dev`, `build-essential`, and
`pkg-config`.

The error names from `pkg-config`, such as `gdk-3.0`, `gtk+-3.0`,
`javascriptcoregtk-4.1`, `libsoup-3.0`, and `webkit2gtk-4.1`, are not apt
package names. Use the `lib...-dev` package names above.

This repo includes a helper for Ubuntu:

```bash
tools/map_asset_relinker_gui/scripts/install_ubuntu_deps.sh
```

Equivalent apt command:

```bash
sudo apt-get update
sudo apt-get install -y build-essential curl file libayatana-appindicator3-dev libgtk-3-dev libjavascriptcoregtk-4.1-dev librsvg2-dev libsoup-3.0-dev libssl-dev libwebkit2gtk-4.1-dev libxdo-dev patchelf pkg-config wget
```

## Build

```bash
tools/map_asset_relinker_gui/scripts/build_native.sh
```

The packaged output is written under `src-tauri/target/release/bundle/`.
On Linux, the direct executable is also built under
`tools/map_asset_relinker_gui/src-tauri/target/release/`.

## Notes

- The Rust backend searches upward from the current working directory for
  `data/maps/map_groups.json` when no project root is provided.
- `npm run build` validates the React/Vite frontend only. `cargo check`,
  `npm run tauri dev`, and `npm run tauri build` also require the Tauri system
  libraries above.
- The Vite-only API endpoints exist only for local development and Playwright
  checks. They are not part of the packaged app.
- Real apply is intentionally behind dry-run success and an explicit
  confirmation dialog. A backup archive is created before source files are
  edited or moved.
- Keep the CLI as the source of truth for relink behavior; the GUI should
  orchestrate it rather than fork the rules.
- The Rust core migration now covers scan/audit data and the main GUI plan
  path. Keep any new GUI-only relink rule out of React/Tauri; promote it into
  the core or Python CLI first.
