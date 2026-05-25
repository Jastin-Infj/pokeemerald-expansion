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
| `plan` | Creates a JSON rename/relink plan from `--map OLD:NEW`. It defaults to `--match-by dir` so the map directory can be the source of truth, infers `MAP_*` and `LAYOUT_*` names from current source, and accepts repair hints for typoed group names, map ids, layout ids, mapsec, valid-but-wrong map layout assignments, stale script label prefixes, mapsec/group repairs, map-name popup metadata, Town Map cell/bounds, map type, transition visit flags, Fly destination metadata, Fly icon style, Fly mapsec type, mapsec-to-map Fly warp rows, and unused flag claims. `--from-group` / `--to-group` can move the map from a temporary group into the final group. |
| `apply --dry-run` | Prints planned directory moves, JSON edits, script include edits, and remaining textual references without modifying files. |
| `apply` | Creates a `.bak.tar` backup archive, moves map/layout directories, updates structured JSON, updates exact script include paths, and rewrites warp/connection map ids. |
| `validate` | Runs the same consistency checks as `audit` after edits. |

## Wrapper

`tools/map_asset_relinker/map_relink.sh` is a thin POSIX shell wrapper. It
resolves the repository root from its own location and runs the Python entry
point with `--root <repo>`, so normal usage can stay short:

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
```

Use the Python entry point directly for fixture tests that need a custom
`--root`.

Real `apply` snapshots existing target files and directories before it edits or
moves source data. The default backup location is
`.map_asset_relinker_backups/map_relink_*.bak.tar`, kept outside `data/maps` so
Porymap will not see backup copies as real maps. `--backup-root` can redirect
the archive when a test or handoff needs a different location.

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
- valid but wrong layout assignment where a map points at another existing
  layout id;
- typoed `region_map_section`;
- wrong-but-valid mapsec id and empty bad group while the map name/layout are
  already correct;
- typoed warp target map id.
- duplicate `data/event_scripts.s` include lines for the same map.
- stale script label prefix inside the selected map's `scripts.inc`, for
  example `Test_3F_MapScripts::` after the map has become `Tester_3F`.
- correct layout identity with a build-incompatible tileset field.
- duplicate layout labels, duplicate region map section ids, and typoed mapsec
  display names.
- missing or post-creation world-map metadata that Porymap does not reliably
  maintain: `map_type`, `show_map_name`, mapsec bounds, region-map cells, Fly
  destination rows, Fly icon style, Fly mapsec type, mapsec-to-map Fly warp
  rows, transition visit flags, and claimed unused flags.

For each broken copy, the script also creates a repair plan, applies it, and
runs `validate` again. The repair paths cover these anchors / hints:

- default `--match-by dir` when the directory name is the trusted source;
- `--old-group-map-name` for typoed map group entries;
- derived old map id plus `--old-map-id` for map id and warp target repairs;
- `--old-layout-id` when `map.json` points at the wrong layout;
- `--set-layout-id` with `--no-layout-rename` when the map itself needs to be
  reattached to an existing layout and the layout entry must not be edited;
- `--rewrite-script-labels` for safe symbol-prefix rewrites inside the selected
  `scripts.inc`; `.string` dialogue lines are skipped.
- `--old-script-prefix` when the map is already renamed and only old script
  labels remain.
- `--set-primary-tileset` / `--set-secondary-tileset` when `layouts.json`
  should keep the same layout id/name/path but change the selected tileset.
- `--set-layout-name` when layout id/path are correct but the layout label is
  duplicated or typoed.
- `--new-mapsec` when `region_map_section` was typoed.
- `--rename-mapsec OLD:NEW` plus `--new-mapsec-name` when an existing mapsec id
  was created with the wrong normalized name.
- `--set-mapsec-name MAPSEC_ID:NAME` when the mapsec id is correct but the
  display name is wrong.
- `--set-show-map-name`, `--set-mapsec-bounds`,
  `--set-region-map-cell`, `--set-map-type`, `--set-transition-setflag`,
  `--ensure-mapsec-map`, `--ensure-fly-location`,
  `--ensure-fly-mapsec-type`, `--set-fly-icon-style`, and
  `--claim-unused-flag` for map-name popup, Town Map coordinate/cursor, Fly
  unlock, Fly icon animation, and Fly warp metadata.
- `--drop-group` to remove an empty accidental group after moving the map into
  the intended `--to-group`.
- `--match-by name` when the directory is wrong but `map.json` `name` is
  trusted.
- `--match-by id` when both the directory and `map.json` `name` are wrong but
  the map id is trusted.

## Current Contract

- The tool does not modify Porymap or generated files.
- Structured JSON updates are used for map groups, map JSON, and layouts JSON.
- Text rewriting is constrained to exact `data/event_scripts.s` include paths.
- Applying a plan for a map also deduplicates later exact `data/event_scripts.s`
  includes for that map.
- Applying a plan creates a `.bak.tar` archive first. The archive contains the
  pre-apply source files/directories and `MANIFEST.json` with the plan.
- Script label prefix rewrite is opt-in. When enabled, only the selected map's
  `scripts.inc` is touched and `.string` lines are skipped.
- Script labels and dialogue are not broadly renamed. Remaining matches are
  reported for manual review.
- Layout rename is enabled by default, but `--no-layout-rename` allows shared
  layout cases.
- `--set-layout-id` sets only `map.json`'s `layout` field and is the repair path
  for cases like `AncientTomb` accidentally pointing at a Tester layout.
- `--rename-mapsec` edits `src/data/region_map/region_map_sections.json` and
  rewrites `region_map_section` references from the old id to the new id.
- `--drop-group` refuses non-empty groups, so deleting a bad group remains an
  explicit cleanup step rather than a lossy merge operation.
- `--rewrite-script-labels` is the repair path for map renames where Porymap
  left `OldName_MapScripts::` or local script labels behind. Use
  `--old-script-prefix` for post-rename cleanup.
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
  duplicate layout ids/names, duplicate mapsec ids, duplicate script includes,
  map name mismatch, missing layout ids, missing mapsec ids, and broken
  warp/connection map ids as errors because those can make Porymap or generated
  map constants unreliable.
- `audit` / `validate` warn on suspicious map group and mapsec naming, such as
  lowercase `gMapGroup_dslite` or mixed-case `MAPSEC_Route201`. These are
  warnings because author intent cannot always be inferred from naming alone.
- Fly destination editing is explicit. The complete Route301-style path is:
  claim or provide a `FLAG_*`, set it from the map transition script, add the
  `sFlyLocations` icon row, add the `GetMapsecType()` flag-gated can-fly case,
  and add the `sMapHealLocations` mapsec-to-map row used by the actual Fly
  warp.
- Fly icon animation is sprite-driven, not data-table animation. `LoadFlyDestIcons()`
  loads `graphics/pokenav/region_map/fly_target_icons.png`, `CreateFlyDestIcons()`
  creates one sprite per `sFlyLocations` row at the mapsec bounds, and
  `SpriteCB_FlyDestIcon()` flickers the selected visited icon every 16 frames.
  Stock city destinations flicker by hiding the Fly sprite so the city dot in
  the static region-map artwork shows underneath. Route301 has no city dot in
  the artwork, so it is listed in `sPaletteBlinkFlyDestinations`; that path adds
  a small underlay sprite and swaps between the normal and custom palette instead
  of hiding the sprite. This keeps Route301 visually active when its name window
  is shown and avoids the "icon just disappears over route art" look.
- The tool can now set the source-side Fly icon style with
  `--set-fly-icon-style MAPSEC_ID:stock`, `MAPSEC_ID:palette-blink`,
  `MAPSEC_ID:blue-blink`, or `MAPSEC_ID:red-outline[:FLAG]`. `blue-blink` and
  `palette-blink` are the same custom blue palette path from the older
  `feature/new-map` experiment. This edits the style membership arrays while
  leaving the actual icon frame shape to the mapsec bounds: `1x1` becomes 8x8,
  `2x1` becomes the 16x8 horizontal oval, and `1x2` becomes the 8x16 vertical
  oval.
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

## GUI Scaffold

The same feature branch now includes a first Tauri + React desktop shell under
`tools/map_asset_relinker_gui/`.

This is a native desktop packaging path rather than a permanent browser-only
localhost workflow. `npm run tauri dev` uses a local Vite server for hot reload
while developing, but `npm run tauri build` embeds the frontend into the Tauri
bundle so normal use can be an exe / AppImage / platform bundle.

Current GUI scope is read-only:

- scan a project root from Rust without invoking Porymap;
- scan the same data from a Vite-only `/api/scan` endpoint for browser /
  Playwright validation when Tauri system libraries are not installed;
- read map groups, map JSON, layouts, and region-map sections;
- show map/group/layout/mapsec relationships and selected-map warnings;
- filter map rows and generate a reviewable CLI plan command for the selected
  map, including target group, layout rename, and script-label rewrite options;
- run the existing Python CLI `plan` and `apply --dry-run` flow from the GUI
  and display the resulting planned moves/edits without changing source files;
- keep real apply disabled until backup review and explicit confirmation
  controls are implemented.

The intended next GUI slice is to have Rust orchestrate
`tools/map_asset_relinker/map_relink.py plan` and `apply --dry-run`, then show
the generated plan before enabling any real apply action.

Validation status for this scaffold:

- `npm install` succeeds and writes a local `package-lock.json`.
- `npm run build` passes for the React/Vite frontend.
- `cargo fmt --check` passes for the Tauri Rust source.
- Playwright browser validation against `http://127.0.0.1:1420/` loads the real
  repo data through the dev-only scan endpoint and reports 945 maps, 78 groups,
  791 layouts, 213 mapsecs, and 5 warnings. At 1320x860 there is no page-level
  overflow and the map/detail panes scroll internally. Filtering `Route301`
  selects the live test map and generates the expected dry-run command for a
  `Route301:Route401` plan with `--to-group gMapGroup_TownsAndRoutes` and
  `--rewrite-script-labels`.
- The Playwright Route301 dry-run button calls the existing CLI via the dev API,
  creates `/tmp/route401_relink_*.json`, and displays the planned layout/map
  moves plus map group, layout, map JSON, script include, and script label
  edits. The CLI reports `Dry-run complete; no files changed.`
- A 1040x720 Playwright viewport confirms the two-column layout path with the
  audit pane moved to a full-width row and no page-level overflow.
- `cargo check --manifest-path src-tauri/Cargo.toml` currently stops before
  checking the app because this Linux environment lacks Tauri's webview / GTK
  system packages (`javascriptcoregtk-4.1`, `libsoup-3.0`, `gdk-pixbuf-2.0`,
  `cairo`, and `atk`). Install the Tauri Linux prerequisites before native
  window validation.

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
  warp target typos, wrong-but-existing layout assignment repair, and duplicate
  script include cleanup. It also verifies metadata-only repairs for layout
  label typos, duplicate mapsec ids, and mapsec display names.
- The fixture test also covers `--rewrite-script-labels`; it rewrites
  `OldCave_2_*` labels/references to `OldCave_2F_*` while preserving the
  literal dialogue string `OldCave_2 should remain in dialogue`.
- Real fixture apply creates `.map_asset_relinker_backups/map_relink_*.bak.tar`
  before editing the temporary source tree.
- Route301 post-creation world-map/Fly setup was applied through the tool with
  backup archives:
  `.map_asset_relinker_backups/map_relink_20260525_225213_796018.bak.tar` and
  `.map_asset_relinker_backups/map_relink_20260525_225438_483794.bak.tar`.
  The applied plan sets `MAP_TYPE_ROUTE`, `show_map_name`, mapsec bounds
  `(9, 0, 1, 1)`, Hoenn region-map cell `(9, 0)`, `FLAG_VISITED_ROUTE301`,
  `Route301_OnTransition`, `sMapHealLocations`, `sFlyLocations`, and
  `GetMapsecType()` for `MAPSEC_ROUTE_301`.
- Debug menu support now exposes `Scripts -> Route301 Fly` for setting
  `FLAG_VISITED_ROUTE301` and `Scripts -> Warp Route301` for direct warp
  validation. The debug `Cheat start` path and `Flags/Vars -> Toggle Locations`
  include `FLAG_VISITED_ROUTE301`, so normal debug location unlocks also cover
  the new route.
- Route301's Fly icon now uses the custom blue / palette-blink path derived
  from the older `feature/new-map` experiment (`a721e70605`, "fly regsion map
  to townmap animetion added") instead of the stock hide/show blink used by map
  art that already has a city dot.
- `tools/map_asset_relinker/map_relink.sh plan --map Route301:Route301
  --no-layout-rename --set-fly-icon-style MAPSEC_ROUTE_301:palette-blink --out
  /tmp/route301_fly_icon_style.json` followed by dry-run apply reports no file
  changes, confirming the live Route301 source matches the toolized style.
- `rtk make generated` passes after the Route301 world-map/Fly source edits.
- `rtk make -j16 -O debug`, `rtk make -j16 -O all`, and
  `rtk make -j16 -O check` pass with the existing RWX linker warning.
- `rtk mdbook build docs` passes with existing warnings for missing root
  `CHANGELOG.md`, `CREDITS.md` `</img>`, and the large search index.
- mGBA Live boot smoke on `pokeemerald.gba` reaches the boot / intro flow,
  captures a nonblank frame after the Route301 blink change, and stops cleanly.
  A manual in-game Fly selection / blink check remains: run
  `Scripts -> Route301 Fly`, then `Utilities -> Fly to map...`, select
  `ROUTE 301`, and confirm the player lands on `MAP_ROUTE301`.
- `tools/map_asset_relinker/map_relink.sh plan --map
  AncientTomb:AncientTomb --no-layout-rename --set-layout-id
  LAYOUT_ANCIENT_TOMB --out /tmp/ancient_tomb_layout_repair.json` succeeds, and
  `apply --dry-run /tmp/ancient_tomb_layout_repair.json` reports no changes
  after the current worktree's AncientTomb layout assignment has already been
  restored.
