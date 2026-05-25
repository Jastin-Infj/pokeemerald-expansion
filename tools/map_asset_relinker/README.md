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
tools/map_asset_relinker/map_relink.sh plan --map TempCave_2:RougeCave_2F --from-group gMapGroup_Temp --to-group gMapGroup_RougeCave --out /tmp/rouge_cave_group_move.json
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
`audit`, `plan --from-group gMapGroup_Temp --to-group gMapGroup_RougeCave`,
`apply --dry-run`, real `apply --allow-dirty`, and `validate`, then checks the
updated map id, layout id, map group move, script include, warp/connection
references, and preserved dialogue text. It also repeats the move into a
missing target group to confirm automatic group creation.

## Notes

- Close Porymap before applying a plan.
- `plan` infers the old map id and old layout from the current `map.json`.
- `--from-group` / `--to-group` move a renamed map from a temporary map group
  into the final map group. If the target group is missing, it is added to
  `group_order` and created.
- `--group` remains a fallback for maps that are not already listed in a map
  group; use `--to-group` when you intentionally want to move groups.
- Layout rename is enabled by default. Use `--no-layout-rename` when multiple
  maps intentionally share the layout.
- Script label rename is intentionally not automatic. The tool rewrites the
  `data/event_scripts.s` include path, then reports remaining textual matches
  for manual review.
