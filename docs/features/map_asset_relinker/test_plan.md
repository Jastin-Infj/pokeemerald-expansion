# Map Asset Relinker Test Plan

## Docs-Only Validation

For the investigation branch:

- `rtk mdbook build docs`
- Confirm docs-only diff before any master PR.

## Current Tool Smoke Tests

Implemented on `feature/map-asset-relinker-20260525`:

| Test | Command | Expected |
|---|---|---|
| Help output | `python3 tools/map_asset_relinker/map_relink.py --help` | CLI lists `audit`, `plan`, `apply`, and `validate`. |
| Shell wrapper help | `tools/map_asset_relinker/map_relink.sh --help` | Wrapper delegates to the Python CLI and lists the same subcommands. |
| Master audit | `python3 tools/map_asset_relinker/map_relink.py audit` | Completes without modifying files. Existing repo issues, if any, are reported as diagnostics. |
| Shell wrapper audit | `tools/map_asset_relinker/map_relink.sh audit` | Completes with the same diagnostics as direct Python invocation. |
| Existing map dry-run plan | `python3 tools/map_asset_relinker/map_relink.py plan --map LittlerootTown:LittlerootTown_Test --out /tmp/map_relink_plan.json` | Plan infers old map id/layout and writes reviewable JSON. |
| Dry-run apply | `python3 tools/map_asset_relinker/map_relink.py apply --dry-run /tmp/map_relink_plan.json` | Prints map/layout move and edit list without changing files. |
| JSON plan validity | `python3 -m json.tool /tmp/map_relink_plan.json` | Plan is valid JSON. |
| Fixture apply | Copy `data/maps`, `data/layouts`, and `data/event_scripts.s` to `/tmp`; run real `apply --allow-dirty` there | Old map dir is removed, new map dir exists, renamed map JSON is valid, and `validate` completes with only existing warnings. |
| Committed fixture test | `tools/map_asset_relinker/test_map_relink.sh` | Copies `testdata/basic` to `/tmp`, renames `OldCave_2` to `OldCave_2F`, moves it from `gMapGroup_Temp` to `gMapGroup_RougeCave`, confirms dry-run is read-only, applies for real, validates, checks map group, layout, include, warp, connection, and preserved dialogue text, repeats against a missing target group to confirm group creation, then creates broken fixture copies and confirms audit failure plus repair apply/validate for map group, map name, map id, layout, mapsec, warp target, `--match-by name`, and `--match-by id` cases. |

Current `audit` warnings on `master` are pre-existing:

- `Route19_UnusedHouse_Frlg`, `Route23_UnusedHouse`,
  `Route6_UnusedHouse_Frlg`, and `SevenIsland_UnusedHouse` exist but are not
  listed in `map_groups.json`.
- `Route19_UnusedHouse_Frlg/scripts.inc` is not included by
  `data/event_scripts.s`.

## Future Tool Tests

| Test | Setup | Expected |
|---|---|---|
| Audit clean known map | Run `audit` on a stable existing map such as `LittlerootTown` | No missing map / layout / script include warnings for that map. |
| Detect orphan map dir | Fixture with `data/maps/Foo/` not in `map_groups.json` | Audit reports orphan directory. |
| Detect missing map dir | Fixture with `Foo` in `map_groups.json` but no `data/maps/Foo/map.json` | Audit reports missing map source. |
| Detect map name mismatch | Directory `Foo/`, `map.json` name `Bar` | Audit reports mismatch. |
| Detect missing layout id | `map.json` references `LAYOUT_FOO`, absent from `layouts.json` | Audit reports missing layout. |
| Detect missing mapsec id | `map.json` references `MAPSEC_FOO`, absent from region map sections JSON | Audit reports missing `region_map_section`. |
| Detect broken map id reference | Warp or connection points at a missing `MAP_FOO` id | Audit reports the missing target map id. |
| Detect stale layout binary paths | `layouts.json` path points to missing `map.bin` / `border.bin` | Audit reports missing binary file. |
| Plan map rename | `RougeCave_2` to `RougeCave_2F` fixture | Plan contains map dir move, `MAP_*` rename, `map_groups.json` update, event script include update. |
| Plan group move | `TempCave_2` in `gMapGroup_Temp`; run `plan --map TempCave_2:RougeCave_2F --from-group gMapGroup_Temp --to-group gMapGroup_RougeCave` | Plan records `fromGroup` / `toGroup`; apply removes the old entry from the temporary group and appends the renamed entry to the target group. |
| Missing target group | Run group move with `--to-group gMapGroup_NewArea` when that group is not present | Apply adds the group to `group_order`, creates the array, and appends the renamed map. |
| Repair group typo | Fixture map dir is correct, but `map_groups.json` lists a typoed old map name | `plan --old-group-map-name <typo>` removes the typoed entry, adds the renamed map to the target group, and `validate` passes. |
| Repair map name typo | Fixture map dir is correct, but `map.json` `name` is typoed | Default `--match-by dir` still finds the map, apply rewrites `name`, and `validate` passes. |
| Repair map id typo | Fixture map dir is correct, but `map.json` `id` is typoed while references still use the expected id | Plan includes both the current id and derived old id; apply rewrites id/references and `validate` passes. |
| Plan layout rename | `LAYOUT_ROUGE_CAVE_2` to `LAYOUT_ROUGE_CAVE_2F` fixture | Plan contains layout id/name/path changes and layout dir move. |
| Repair layout typo | Fixture map points at a typoed layout id | `plan --old-layout-id <actual-layout-id>` renames the actual layout and rewrites the map layout field. |
| Repair mapsec typo | Fixture map has a typoed `region_map_section` | `plan --new-mapsec <valid-mapsec>` rewrites the mapsec field and `validate` passes. |
| Repair warp target typo | Fixture warp points at a typoed old map id | `plan --old-map-id <typoed-map-id>` rewrites the bad target id to the new map id and `validate` passes. |
| Repair by map name anchor | Fixture directory is typoed but `map.json` `name` is correct | `plan --match-by name` selects the map, moves the actual directory, fixes script include, and `validate` passes. |
| Repair by map id anchor | Fixture directory and `map.json` `name` are typoed but map id is correct | `plan --match-by id --old-group-map-name <group-entry>` selects the map, repairs name/directory/group/include, and `validate` passes. |
| Dry-run is read-only | Run `apply --dry-run` | No file contents or paths change. |
| Apply structured edits | Run `apply` on fixture | JSON remains valid and only expected fields changed. |
| Warp rewrite | Fixture has `dest_map: MAP_ROUGE_CAVE_2` | Apply rewrites to `MAP_ROUGE_CAVE_2F`. |
| Connection rewrite | Fixture has connection map `MAP_ROUGE_CAVE_2` | Apply rewrites to `MAP_ROUGE_CAVE_2F`. |
| Avoid broad text rewrite | Script text contains old name in dialogue | Apply does not alter dialogue unless explicit script-label rewrite is requested. |

## Runtime Validation For A Real Rename

After applying to a real map branch:

1. `rtk make generated`
2. `rtk make -j16 -O debug`
3. Open Porymap and confirm renamed map / layout appears correctly.
4. Use mGBA debug warp or a script route to enter and leave the renamed map.
5. Confirm map name popup, warps, connections, and layout render.

## Remaining Manual Checks

- Region Map / Fly references if the renamed map has a new or changed
  `MAPSEC_*`.
- Wild encounters if the map has encounter data outside the map JSON.
- Trainer / object scripts if labels were renamed.
- Save compatibility if map group order or map number changes are intentional.
