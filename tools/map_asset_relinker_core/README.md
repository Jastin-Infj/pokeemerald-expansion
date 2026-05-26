# Map Asset Relinker Core

Rust core migration target for the map asset relinker.

The current slice owns project scanning, relationship summaries, and the
GUI-compatible JSON plan path. The Python CLI remains the full apply / backup
implementation until the Rust core reaches feature parity.

## Commands

```bash
cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- scan --root . --pretty
cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- audit --root .
cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- plan --root . --map OldCave_2:OldCave_2F --to-group gMapGroup_RougeCave --out /tmp/plan.json
```

`scan` prints the same camelCase summary shape consumed by the React GUI:

- map / group / layout / mapsec counts
- per-map group, layout, mapsec, map type, popup metadata
- selected-map issues and global warning strings

`plan` writes Python-compatible version-1 JSON for the main GUI path:

- map rename / directory rename
- optional group move
- default layout rename
- optional mapsec rename / display-name update
- optional script-label prefix rewrite

## Migration Plan

1. Keep the Python CLI as the compatibility surface for `apply` and backup
   behavior.
2. Move read-only scan / audit logic into this crate and have Tauri call it
   directly. Done for the Tauri path.
3. Port plan generation into Rust while keeping JSON plan compatibility with
   the Python CLI. Started for the GUI rename / mapsec path.
4. Port real apply and backup creation last, after fixture parity tests cover
   the Rust path.
5. When parity is proven, make both CUI and GUI call this crate as the source
   of truth and leave Python as a temporary shim or remove it.
