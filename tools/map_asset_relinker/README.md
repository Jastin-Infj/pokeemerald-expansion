# Map Asset Relinker

Repo-side helper for auditing and relinking Porymap-style map/layout assets.

The tool edits source files only:

- `data/maps/map_groups.json`
- `data/maps/<MapName>/map.json`
- `data/layouts/layouts.json`
- exact map script includes in `data/event_scripts.s`
- `warp_events[].dest_map` and `connections[].map` references in map JSON

It does not edit Porymap internals or generated outputs.

## Commands

```bash
tools/map_asset_relinker/map_relink.sh audit
tools/map_asset_relinker/map_relink.sh plan --map RougeCave_2:RougeCave_2F --out /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh apply --dry-run /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh apply /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh validate
```

The `.sh` wrapper resolves the repository root from its own location, so it can
be run from the repo root or another working directory. Use the Python entry
point directly when you need to pass a custom `--root` for fixture testing.

After a real apply, run:

```bash
rtk make generated
rtk make -j16 -O debug
```

## Fixture Test

The repository includes a small fake map/layout tree that exercises a real
rename from `OldCave_2` to `OldCave_2F` without touching the project data:

```bash
tools/map_asset_relinker/test_map_relink.sh
```

The test copies `tools/map_asset_relinker/testdata/basic` to `/tmp`, runs
`audit`, `plan`, `apply --dry-run`, real `apply --allow-dirty`, and `validate`,
then checks the updated map id, layout id, map group entry, script include,
warp/connection references, and preserved dialogue text.

## Notes

- Close Porymap before applying a plan.
- `plan` infers the old map id and old layout from the current `map.json`.
- Layout rename is enabled by default. Use `--no-layout-rename` when multiple
  maps intentionally share the layout.
- Script label rename is intentionally not automatic. The tool rewrites the
  `data/event_scripts.s` include path, then reports remaining textual matches
  for manual review.
