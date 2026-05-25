# Map Asset Relinker Implementation

## Runtime Slice: Python CLI MVP

| Field | Value |
|---|---|
| Branch | `feature/map-asset-relinker-20260525` |
| Status | Initial tool implementation |
| Primary files | `tools/map_asset_relinker/map_relink.py`, `tools/map_asset_relinker/map_relink.sh`, `tools/map_asset_relinker/test_map_relink.sh`, `tools/map_asset_relinker/testdata/basic/`, `tools/map_asset_relinker/README.md` |
| Last updated | 2026-05-25 |

## Implemented Commands

| Command | Status |
|---|---|
| `audit` | Scans map directories, `map_groups.json`, `layouts.json`, layout binary paths, `region_map_section`, map id references, and `data/event_scripts.s` includes. |
| `plan` | Creates a JSON rename/relink plan from `--map OLD:NEW`. It defaults to `--match-by dir` so the map directory can be the source of truth, infers `MAP_*` and `LAYOUT_*` names from current source, and accepts repair hints for typoed group names, map ids, layout ids, and mapsec. `--from-group` / `--to-group` can move the map from a temporary group into the final group. |
| `apply --dry-run` | Prints planned directory moves, JSON edits, script include edits, and remaining textual references without modifying files. |
| `apply` | Moves map/layout directories, updates structured JSON, updates exact script include paths, and rewrites warp/connection map ids. |
| `validate` | Runs the same consistency checks as `audit` after edits. |

## Wrapper

`tools/map_asset_relinker/map_relink.sh` is a thin POSIX shell wrapper. It
resolves the repository root from its own location and runs the Python entry
point with `--root <repo>`, so normal usage can stay short:

```bash
tools/map_asset_relinker/map_relink.sh audit
tools/map_asset_relinker/map_relink.sh plan --map RougeCave_2:RougeCave_2F --out /tmp/rouge_cave_rename.json
tools/map_asset_relinker/map_relink.sh plan --map TempCave_2:RougeCave_2F --from-group gMapGroup_Temp --to-group gMapGroup_RougeCave --out /tmp/rouge_cave_group_move.json
tools/map_asset_relinker/map_relink.sh plan --map TempCave_2:RougeCave_2F --old-group-map-name TempCaveTypo --old-layout-id LAYOUT_TEMP_CAVE_2 --old-map-id MAP_TEMP_CAVE_TYPO --new-mapsec MAPSEC_NONE --out /tmp/rouge_cave_repair.json
```

Use the Python entry point directly for fixture tests that need a custom
`--root`.

## Fixture Test Data

`tools/map_asset_relinker/testdata/basic/` is a minimal Porymap-shaped source
tree with two maps, two map groups, and two layouts:

- `OldCave_2` is the rename target.
- `OldCave_2` starts in `gMapGroup_Temp`.
- `gMapGroup_RougeCave` is the final target group.
- `OldCave_Exit` points at `MAP_OLD_CAVE_2` through both a warp and a
  connection.
- `layouts.json` points at `data/layouts/OldCave_2/map.bin` and
  `border.bin`.
- `data/event_scripts.s` includes `data/maps/OldCave_2/scripts.inc`.
- `OldCave_2/scripts.inc` keeps old-name dialogue text so the test can prove
  the tool is not doing broad script text rewrites.

`tools/map_asset_relinker/test_map_relink.sh` copies this fixture to `/tmp`,
renames `OldCave_2` to `OldCave_2F`, and moves it from `gMapGroup_Temp` to
`gMapGroup_RougeCave`. It checks that the generated identifiers are
`MAP_OLD_CAVE_2F` and `LAYOUT_OLD_CAVE_2F`, applies the plan for real in the
temporary copy, runs `validate`, and confirms map group, map JSON, layout JSON,
script include, warp, and connection updates. The same script also repeats the
group move into a missing target group to confirm the tool adds the new group
to `group_order` and creates its map list.

The fixture script also creates broken temporary copies to prove the audit path
fails on the high-risk authoring mistakes this tool is meant to catch:

- map listed under the wrong name in `map_groups.json`;
- `map.json` `name` mismatch against its directory;
- typoed map id;
- typoed layout id;
- typoed `region_map_section`;
- typoed warp target map id.

For each broken copy, the script also creates a repair plan, applies it, and
runs `validate` again. The repair paths cover these anchors / hints:

- default `--match-by dir` when the directory name is the trusted source;
- `--old-group-map-name` for typoed map group entries;
- derived old map id plus `--old-map-id` for map id and warp target repairs;
- `--old-layout-id` when `map.json` points at the wrong layout;
- `--new-mapsec` when `region_map_section` was typoed.
- `--match-by name` when the directory is wrong but `map.json` `name` is
  trusted.
- `--match-by id` when both the directory and `map.json` `name` are wrong but
  the map id is trusted.

## Current Contract

- The tool does not modify Porymap or generated files.
- Structured JSON updates are used for map groups, map JSON, and layouts JSON.
- Text rewriting is constrained to exact `data/event_scripts.s` include paths.
- Script labels and dialogue are not broadly renamed. Remaining matches are
  reported for manual review.
- Layout rename is enabled by default, but `--no-layout-rename` allows shared
  layout cases.
- Group move is optional. Without `--to-group`, map groups are updated in place
  for backward-compatible rename plans. With `--to-group`, the old map is
  removed from `--from-group` or every current group, then the new map is
  appended to the target group.
- `--group` remains a fallback for maps that are not already listed in any map
  group; it does not move an already grouped map unless `--to-group` is also
  used.
- `--match-by dir` is the default repair anchor because Porymap-created map
  directories are the most stable source after a typo. `--match-by name` and
  `--match-by id` remain available when those fields are the intentional source
  of truth.
- Repair hints do not guess arbitrary typos. The user/agent must provide the
  typoed group map name or old map id when references contain a wrong token the
  tool cannot infer from the directory.
- `audit` / `validate` treat group shape errors, duplicate map names / ids,
  map name mismatch, missing layout ids, missing mapsec ids, and broken
  warp/connection map ids as errors because those can make Porymap or generated
  map constants unreliable.
- The tool refuses real apply when target files are dirty unless
  `--allow-dirty` is passed.

## Example

```bash
python3 tools/map_asset_relinker/map_relink.py plan \
  --map RougeCave_2:RougeCave_2F \
  --out /tmp/rouge_cave_rename.json
python3 tools/map_asset_relinker/map_relink.py apply --dry-run /tmp/rouge_cave_rename.json
```

## Validation Notes

Run after a real apply:

```bash
rtk make generated
rtk make -j16 -O debug
```

The implementation branch should also run the script-level smoke tests recorded
in `test_plan.md`.

## Validation Evidence

- `python3 tools/map_asset_relinker/map_relink.py --help` lists the expected
  subcommands.
- `tools/map_asset_relinker/map_relink.sh --help` delegates to the Python CLI.
- `tools/map_asset_relinker/map_relink.sh audit` completes with the same
  diagnostics as the Python entry point.
- `python3 -m py_compile tools/map_asset_relinker/map_relink.py` passes.
- `python3 tools/map_asset_relinker/map_relink.py audit` completes with 0
  errors and 5 existing warnings for unused map directories / one unused script
  include.
- `python3 tools/map_asset_relinker/map_relink.py plan --map
  LittlerootTown:LittlerootTown_Test --out /tmp/map_relink_plan.json` writes
  valid reviewable JSON.
- `python3 tools/map_asset_relinker/map_relink.py apply --dry-run
  /tmp/map_relink_plan.json` lists map/layout moves and structured edits
  without changing the worktree.
- A `/tmp` fixture copy of `data/maps`, `data/layouts`, and
  `data/event_scripts.s` successfully ran real `apply --allow-dirty`, then
  `validate`, and confirmed the old map directory was removed, the new
  directory existed, and the renamed `map.json` remained valid JSON.
- `tools/map_asset_relinker/test_map_relink.sh` passes against the committed
  `testdata/basic` fixture and leaves the temporary result path in the output
  for inspection. It covers successful rename/group move, missing target group
  creation, and negative audit checks for map group, map name, layout, mapsec,
  and warp target typos.
