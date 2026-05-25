# Map Asset Relinker Test Plan

## Docs-Only Validation

For the investigation branch:

- `rtk mdbook build docs`
- Confirm docs-only diff before any master PR.

## Future Tool Tests

| Test | Setup | Expected |
|---|---|---|
| Audit clean known map | Run `audit` on a stable existing map such as `LittlerootTown` | No missing map / layout / script include warnings for that map. |
| Detect orphan map dir | Fixture with `data/maps/Foo/` not in `map_groups.json` | Audit reports orphan directory. |
| Detect missing map dir | Fixture with `Foo` in `map_groups.json` but no `data/maps/Foo/map.json` | Audit reports missing map source. |
| Detect map name mismatch | Directory `Foo/`, `map.json` name `Bar` | Audit reports mismatch. |
| Detect missing layout id | `map.json` references `LAYOUT_FOO`, absent from `layouts.json` | Audit reports missing layout. |
| Detect stale layout binary paths | `layouts.json` path points to missing `map.bin` / `border.bin` | Audit reports missing binary file. |
| Plan map rename | `RougeCave_2` to `RougeCave_2F` fixture | Plan contains map dir move, `MAP_*` rename, `map_groups.json` update, event script include update. |
| Plan layout rename | `LAYOUT_ROUGE_CAVE_2` to `LAYOUT_ROUGE_CAVE_2F` fixture | Plan contains layout id/name/path changes and layout dir move. |
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
