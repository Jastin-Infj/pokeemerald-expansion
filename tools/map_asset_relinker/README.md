# Map Asset Relinker

Repo-side helper for auditing and relinking Porymap-style map/layout assets.

The tool edits source files only:

- `data/maps/map_groups.json`
- `data/maps/<MapName>/map.json`
- `data/layouts/layouts.json`
- exact map script includes in `data/event_scripts.s`
- `warp_events[].dest_map` and `connections[].map` references in map JSON
- map-name popup / map type metadata in `data/maps/<MapName>/map.json`
- Town Map mapsec names/bounds in `src/data/region_map/region_map_sections.json`
- Town Map cursor cells in `src/data/region_map/region_map_layout*.h`
- Fly destination, Fly mapsec type, and mapsec-to-map destination rows in
  `src/region_map.c`
- claimed unused flags in `include/constants/flags.h`

It does not edit Porymap internals or generated outputs.

## Commands

```bash
tools/map_asset_relinker/map_relink.sh audit
tools/map_asset_relinker/map_relink.sh plan --map RougeCave_2:RougeCave_2F --out /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh plan --map Test_3F:Tester_3F --to-group gMapGroup_Tester --rewrite-script-labels --out /tmp/tester_3f_repair.json
tools/map_asset_relinker/map_relink.sh plan --map DS_LITE_1F:DS_LITE_1F --no-layout-rename --to-group gMapGroup_DSLite --rename-mapsec MAPSEC_DSLITE:MAPSEC_DS_LITE --new-mapsec-name DS-LITE --drop-group gMapGroup_dslite --out /tmp/ds_lite_repair.json
tools/map_asset_relinker/map_relink.sh plan --map Route201:Route201 --no-layout-rename --to-group gMapGroup_TownsAndRoutes --rename-mapsec MAPSEC_Route201:MAPSEC_ROUTE_201 --new-mapsec-name 'ROUTE 201' --set-primary-tileset gTileset_General --out /tmp/route201_repair.json
tools/map_asset_relinker/map_relink.sh plan --map Route201:Route201 --no-layout-rename --set-layout-name Route201_Layout --set-mapsec-name MAPSEC_ROUTE_201:'ROUTE 201' --out /tmp/route201_metadata_repair.json
tools/map_asset_relinker/map_relink.sh plan --map Route301:Route301 --no-layout-rename --set-map-type MAP_TYPE_ROUTE --set-show-map-name true --set-transition-setflag FLAG_VISITED_ROUTE301 --set-mapsec-bounds MAPSEC_ROUTE_301:9:0:1:1 --set-region-map-cell hoenn:9:0:MAPSEC_ROUTE_301 --ensure-mapsec-map MAPSEC_ROUTE_301:MAP_ROUTE301:HEAL_LOCATION_NONE --ensure-fly-location hoenn:MAPSEC_ROUTE_301:FLAG_VISITED_ROUTE301 --ensure-fly-mapsec-type MAPSEC_ROUTE_301:FLAG_VISITED_ROUTE301 --set-fly-icon-style MAPSEC_ROUTE_301:palette-blink --claim-unused-flag FLAG_UNUSED_0x881:FLAG_VISITED_ROUTE301 --out /tmp/route301_worldmap.json
tools/map_asset_relinker/map_relink.sh plan --map TempCave_2:RougeCave_2F --from-group gMapGroup_Temp --to-group gMapGroup_RougeCave --out /tmp/rouge_cave_group_move.json
tools/map_asset_relinker/map_relink.sh plan --map TempCave_2:RougeCave_2F --old-group-map-name TempCaveTypo --old-layout-id LAYOUT_TEMP_CAVE_2 --old-map-id MAP_TEMP_CAVE_TYPO --new-mapsec MAPSEC_NONE --out /tmp/rouge_cave_repair.json
tools/map_asset_relinker/map_relink.sh plan --map AncientTomb:AncientTomb --no-layout-rename --set-layout-id LAYOUT_ANCIENT_TOMB --out /tmp/ancient_tomb_layout_repair.json
tools/map_asset_relinker/map_relink.sh plan-temp-mapsec --map test1 --dry-run --out /tmp/test1_temp_mapsec_repair.json
tools/map_asset_relinker/map_relink.sh apply --dry-run /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh apply /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh validate
```

The `.sh` wrapper resolves the repository root from its own location, so it can
be run from the repo root or another working directory. Use the Python entry
point directly when you need to pass a custom `--root` for fixture testing.

Real `apply` creates a backup archive before moving, deleting, or editing source
data. By default the archive is written under
`.map_asset_relinker_backups/map_relink_*.bak.tar`; use `--backup-root` to
place the archive somewhere else. Dry-run does not create a backup.

After a real apply, run:

```bash
rtk make generated
rtk make -j16 -O debug
```

## Desktop GUI Scaffold

`tools/map_asset_relinker_gui/` contains an early Tauri + React desktop shell.
It scans map groups, maps, layouts, and mapsecs, shows relationship warnings,
runs CLI dry-runs, and can run the same real `apply --allow-dirty` path after
an explicit confirmation. GUI apply creates the normal
`.map_asset_relinker_backups/*.bak.tar` archive before source edits, then
rescans the project and reports the backup path.

The long-term migration target is `tools/map_asset_relinker_core/`, a Rust
crate shared by CUI and GUI. Scan / warning summaries and the main GUI JSON
plan path now live there. Python remains the compatibility surface for
`apply` / backup behavior until the Rust core reaches fixture parity.

Development uses a local Vite server:

```bash
cd tools/map_asset_relinker_gui
npm install
npm run tauri dev
```

Production bundles embed the frontend in the Tauri executable, so normal use
does not require opening a browser or manually connecting to localhost.

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

`plan-temp-mapsec` is the CLI shortcut for the common Porymap mistake where a
temporary map name, layout, group placement, and mixed-case `MAPSEC_*` were
created together. Given a map such as `test1` with `region_map_section:
MAPSEC_Jongle`, it infers `test1:Jongle`, `gMapGroup_Jongle`,
`MAPSEC_JONGLE`, `JONGLE`, keeps layout rename on, rewrites script label
prefixes by default, and can immediately run `apply --dry-run`.

The fixture test also creates broken copies and confirms they can be repaired
by generating a plan from a chosen anchor:

- group entry typo: `--old-group-map-name`;
- map directory is correct but `map.json` `name` is wrong: default
  `--match-by dir`;
- map id typo: derived old id plus `--old-map-id`;
- map layout typo: `--old-layout-id`;
- valid but wrong map layout assignment: `--set-layout-id` with
  `--no-layout-rename`;
- mapsec typo: `--new-mapsec`;
- map name as the trusted anchor while mapsec/group are wrong:
  `--rename-mapsec` rewrites the section id and map references,
  `--drop-group` removes an empty bad group, and `--to-group` creates or fills
  the correct group without touching layout assets when paired with
  `--no-layout-rename`;
- warp / connection target typo: `--old-map-id`.
- directory typo while `map.json` `name` is correct: `--match-by name`;
- directory and `map.json` `name` typo while map id is correct:
  `--match-by id`.
- duplicate `data/event_scripts.s` map includes: applying a plan for that map
  keeps one exact include and removes later duplicates.
- script label prefixes in the selected map's `scripts.inc`:
  `--rewrite-script-labels` rewrites symbol prefixes such as
  `Test_3F_MapScripts` to `Tester_3F_MapScripts` while leaving literal
  `.string` dialogue lines alone.
- selected layout tileset fields: `--set-primary-tileset` and
  `--set-secondary-tileset` adjust `layouts.json` without renaming the layout
  id/name/path.
- selected layout / mapsec display names: `--set-layout-name` and
  `--set-mapsec-name MAPSEC_ID:NAME` repair metadata without direct JSON edits.
- map-name popup and world-map / Fly metadata:
  `--set-show-map-name`, `--set-mapsec-bounds`,
  `--set-region-map-cell`, `--ensure-mapsec-map`,
  `--ensure-fly-location`, `--ensure-fly-mapsec-type`,
  `--set-transition-setflag`, and `--claim-unused-flag` let a plan cover the
  source files Porymap does not reliably update after the initial map creation.

## Audit Coverage

`audit` / `validate` are intended to catch common Porymap-breaking mistakes
before opening the editor:

- malformed `map_groups.json` shape, missing group arrays, empty groups, group
  names omitted from `group_order`, and duplicate map entries;
- map directory / `map.json` name mismatch and duplicate map `name` / `id`;
- missing layout ids, duplicate layout ids/names, bad layout binary paths, and
  empty layout file fields;
- missing or duplicate `region_map_section` ids from
  `src/data/region_map/region_map_sections.json`;
- duplicate `data/event_scripts.s` map script includes;
- suspicious map group / mapsec naming such as lowercase
  `gMapGroup_dslite` or mixed-case `MAPSEC_Route201`;
- `connections[].map` and `warp_events[].dest_map` values that do not point at
  a known map id.

## Notes

- Close Porymap before applying a plan.
- `plan` defaults to `--match-by dir`, treating the `data/maps/<MapName>/`
  directory as the source of truth. Use `--match-by name` or `--match-by id`
  when the directory is not the trustworthy anchor.
- `plan` infers the old map id and old layout from the selected `map.json`,
  but `--old-map-id` and `--old-layout-id` can add repair hints when those
  fields were typoed.
- `--from-group` / `--to-group` move a renamed map from a temporary map group
  into the final map group. If the target group is missing, it is added to
  `group_order` and created.
- `--old-group-map-name` removes a typoed old map entry from `map_groups.json`
  during the move/rename.
- `--group` remains a fallback for maps that are not already listed in a map
  group; use `--to-group` when you intentionally want to move groups.
- `--new-mapsec` sets `region_map_section` on the renamed map when mapsec was
  typoed or intentionally changed.
- `--rename-mapsec OLD:NEW` renames an existing region map section id and
  rewrites map `region_map_section` references from OLD to NEW.
- `--new-mapsec-name` updates the display name while using `--rename-mapsec`.
- `--drop-group` removes an empty bad group from `map_groups.json`; it refuses
  to drop non-empty groups.
- `--set-layout-id` sets only the selected map's `layout` field. Pair it with
  `--no-layout-rename` when a map accidentally borrowed another map's layout
  and the layout entry itself should not be edited.
- `--set-layout-name` sets only the selected layout label. Pair it with
  `--no-layout-rename` when the layout id/path are correct but the label is
  duplicated or typoed.
- `--set-mapsec-name MAPSEC_ID:NAME` sets an existing region map section
  display name without changing the id or map references.
- `--set-show-map-name true|false` updates the selected map's
  `show_map_name` field, which controls the overworld map-name popup together
  with the map's `region_map_section`.
- `--set-map-type MAP_TYPE_*` updates the selected map's `map_type`, useful
  when a temporary map was created as `MAP_TYPE_NONE` but should behave as a
  route, town, city, cave, or other real map type.
- `--set-transition-setflag FLAG_*` ensures the selected map's
  `MAP_SCRIPT_ON_TRANSITION` sets a visit/unlock flag. For Fly unlocks this is
  the usual route that turns a newly visited destination from locked to usable.
- `--set-mapsec-bounds MAPSEC_ID:X:Y:WIDTH:HEIGHT` writes the Town Map
  position and dimensions in `src/data/region_map/region_map_sections.json`.
- `--set-region-map-cell REGION:X:Y:MAPSEC_ID` writes a cursor/cell mapping in
  one of the region map layout headers. REGION is `hoenn`, `kanto`,
  `sevii123`, `sevii45`, or `sevii67`.
- `--ensure-fly-location REGION:MAPSEC_ID:FLAG` adds or updates a Fly
  destination icon row in `src/region_map.c`.
- `--ensure-fly-mapsec-type MAPSEC_ID:FLAG` updates `GetMapsecType()` so the
  cursor treats the mapsec as a flag-gated Fly destination.
- `--ensure-mapsec-map MAPSEC_ID:MAP_ID[:HEAL_LOCATION_ID]` updates
  `sMapHealLocations`, which is the table used after selecting Fly to resolve
  the destination map or heal location. If no heal location is provided, it
  uses `HEAL_LOCATION_NONE` and warps to the map itself.
- `--set-fly-icon-style MAPSEC_ID:STYLE[:FLAG]` updates source-side Fly icon
  animation membership. `stock` / `default` uses the vanilla hide/show blink,
  `palette-blink` / `blue-blink` / `blue` keeps the icon visible and swaps to
  the blue custom palette over a small underlay for route-style destinations
  without map art, and `red-outline` adds a Battle Frontier-style red outline
  overlay. `red-outline` can infer the flag from `sFlyLocations`; pass
  `:FLAG_*` when inference is ambiguous.
  Fly icon shape is still automatic: `width:height` from `--set-mapsec-bounds`
  selects 8x8, 16x8, or 8x16 icon frames in `fly_target_icons.png`.
- `--claim-unused-flag OLD_FLAG:NEW_FLAG` renames an existing unused flag
  define, for example `FLAG_UNUSED_0x881:FLAG_VISITED_ROUTE301`.
- `--rewrite-script-labels` is opt-in because script text can contain old map
  names intentionally. Use `--old-script-prefix` when the map has already been
  renamed and only the stale script symbols remain.
- `--set-primary-tileset` and `--set-secondary-tileset` are useful when the
  layout identity is correct but Porymap selected a build-incompatible tileset.
- Real `apply` backs up existing target files and moved directories before any
  source edit. The backup root is ignored by git and stays outside `data/maps`
  so Porymap will not treat backup folders as maps.
- Layout rename is enabled by default. Use `--no-layout-rename` when multiple
  maps intentionally share the layout.
- Script label rename is intentionally not automatic. The tool rewrites the
  `data/event_scripts.s` include path, then reports remaining textual matches
  for manual review.
