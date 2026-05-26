# Map Asset Relinker Core

Rust core migration target for the map asset relinker.

The current slice owns project scanning and relationship summaries used by the
Tauri GUI. The Python CLI remains the full plan / apply implementation until
the Rust core reaches feature parity.

## Commands

```bash
cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- scan --root . --pretty
cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- audit --root .
```

`scan` prints the same camelCase summary shape consumed by the React GUI:

- map / group / layout / mapsec counts
- per-map group, layout, mapsec, map type, popup metadata
- selected-map issues and global warning strings

## Migration Plan

1. Keep the Python CLI as the compatibility surface for `plan`, `apply`, and
   backup behavior.
2. Move read-only scan / audit logic into this crate and have Tauri call it
   directly.
3. Port plan generation into Rust while keeping JSON plan compatibility with
   the Python CLI.
4. Port real apply and backup creation last, after fixture parity tests cover
   the Rust path.
5. When parity is proven, make both CUI and GUI call this crate as the source
   of truth and leave Python as a temporary shim or remove it.
