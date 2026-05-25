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
- Show the selected map's `Map -> Group -> Layout -> Mapsec` relationship as a
  compact graph before the raw field grid.
- Filter maps and generate a reviewable CLI plan command for a selected map.
- Run the existing CLI `plan` plus `apply --dry-run` flow and display the
  planned moves/edits plus move/edit/review counts without changing source
  files.
- Keep the GUI read-only while the CLI remains the write path.

Future slices can add guarded real apply. That should keep the existing backup
archive behavior and require an explicit confirmation step after dry-run review.

## Development

```bash
cd tools/map_asset_relinker_gui
npm install
npm run tauri dev
```

The development command opens a native Tauri window. The localhost dev server
is an implementation detail of hot reload.

For browser-only validation, open `http://127.0.0.1:1420/` while the Vite
server is running. The dev server exposes a read-only `/api/scan` endpoint that
uses Node filesystem access to scan the current repo. Production Tauri builds
use the Rust command instead.

On Linux, Tauri also needs the platform webview / GTK development packages.
For Debian/Ubuntu-like systems, install the Tauri prerequisites including
`libwebkit2gtk-4.1-dev`, `libsoup-3.0-dev`, `libgtk-3-dev`,
`libayatana-appindicator3-dev`, `librsvg2-dev`, `libxdo-dev`, `libssl-dev`,
`build-essential`, and `pkg-config`.

This repo includes a helper for Ubuntu:

```bash
tools/map_asset_relinker_gui/scripts/install_ubuntu_deps.sh
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
- The Vite-only `/api/scan` endpoint exists only for local development and
  Playwright checks. It is not part of the packaged app.
- The first GUI slice avoids write commands on purpose. Plan/apply integration
  should preserve the existing CLI backup behavior and dry-run review step.
- Keep the CLI as the source of truth for relink behavior; the GUI should
  orchestrate it rather than fork the rules.
