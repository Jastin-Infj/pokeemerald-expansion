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
| `plan-temp-mapsec` | CLI shortcut for temporary-map repairs anchored to the selected map's current `region_map_section`. For a map like `test1` with `MAPSEC_Jongle`, it infers `test1:Jongle`, `gMapGroup_Jongle`, `MAPSEC_JONGLE`, `JONGLE`, keeps layout rename on, rewrites script labels by default, and can immediately run `apply --dry-run`. |
| `apply --dry-run` | Prints planned directory moves, JSON edits, script include edits, and remaining textual references without modifying files. |
| `apply` | Creates a `.bak.tar` backup archive, moves map/layout directories, updates structured JSON, updates exact script include paths, and rewrites warp/connection map ids. |
| `validate` | Runs the same consistency checks as `audit` after edits. |

## Runtime Slice: Rust Core And Linux GUI Build

| Field | Value |
|---|---|
| Branch | `feature/map-asset-relinker-20260525` |
| Status | Host-dependent Linux executable build validated |
| Primary files | `tools/map_asset_relinker_core/`, `tools/map_asset_relinker_gui/`, `.github/workflows/map-asset-relinker-desktop.yml`, `tools/map_asset_relinker_gui/scripts/build_native.sh`, `tools/map_asset_relinker_gui/scripts/build_windows.ps1`, `tools/map_asset_relinker_gui/scripts/prepare_linux_deps_local.sh`, `tools/map_asset_relinker_gui/src-tauri/icons/icon.png`, `tools/map_asset_relinker_gui/src-tauri/icons/icon.ico` |
| Last updated | 2026-05-26 |

The GUI path now keeps the Rust core as the executable source of truth and uses
Tauri for the local desktop shell. `build_native.sh` builds the Rust CUI release
binary first, then builds host-dependent Linux GUI output only: the direct GUI
executable plus `.deb` and `.rpm` packages. AppImage is intentionally skipped in
this helper because it is the portable bundle path and has stricter icon/runtime
bundling requirements than the current Linux-host target.
The direct executable still depends on host runtime libraries such as
`libwebkit2gtk-4.1-0` and `libgtk-3-0`; the `.deb` bundle declares those runtime
dependencies.

The packaged desktop UX no longer assumes the executable is launched from a
repository checkout. On first launch, the Tauri dialog plugin opens a native
directory picker for the pokeemerald-expansion project root; after the user
selects a folder, the GUI stores that root and immediately scans it. The top
bar also keeps an `Open...` action beside `Scan`, so switching target projects
does not require typing a path into the root field. Browser-only development
keeps the previous automatic local scan path and uses a prompt fallback for the
open action.

When the host cannot install GTK/WebKit Tauri development packages globally,
`prepare_linux_deps_local.sh` can download the Ubuntu packages into
`tools/map_asset_relinker_gui/.cache/`, extract them as a local pkg-config
sysroot, and let `build_native.sh` consume that sysroot through
`TAURI_LOCAL_DEPS`. The generated `.cache/`, `dist/`, `src-tauri/target/`, and
Tauri `src-tauri/gen/` schema files remain ignored build output.

Validated Linux output paths:

```text
tools/map_asset_relinker_core/target/release/map-asset-relinker-core
tools/map_asset_relinker_gui/src-tauri/target/release/map-asset-relinker-gui
tools/map_asset_relinker_gui/src-tauri/target/release/bundle/deb/Map Asset Relinker_0.1.0_amd64.deb
tools/map_asset_relinker_gui/src-tauri/target/release/bundle/rpm/Map Asset Relinker-0.1.0-1.x86_64.rpm
```

Windows is the primary day-to-day GUI target. `build_windows.ps1` runs on a
Windows host, builds `map-asset-relinker-core.exe`, then asks Tauri to build
the direct GUI `.exe`, NSIS installer `.exe`, and `.msi` bundle. The
`map-asset-relinker-desktop.yml` workflow runs that same script on
`windows-latest` and uploads a `map-asset-relinker-windows` artifact so the exe
can be downloaded from GitHub Actions without manually setting up a local
Windows toolchain. The first Windows run built the direct GUI exe but failed at
MSI icon lookup; registering `icons/icon.ico` in `tauri.conf.json` fixed that.
After adding the native project-root picker, run `26481475862` uploaded
artifact id `7228721956` on the current branch head.

Expected Windows output paths:

```text
tools/map_asset_relinker_core/target/release/map-asset-relinker-core.exe
tools/map_asset_relinker_gui/src-tauri/target/release/map-asset-relinker-gui.exe
tools/map_asset_relinker_gui/src-tauri/target/release/bundle/nsis/Map Asset Relinker_0.1.0_x64-setup.exe
tools/map_asset_relinker_gui/src-tauri/target/release/bundle/msi/Map Asset Relinker_0.1.0_x64_en-US.msi
```

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
tools/map_asset_relinker/map_relink.sh plan-temp-mapsec --map test1 --dry-run --out /tmp/test1_temp_mapsec_repair.json
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
- `plan-temp-mapsec --map <map>` when a temporary map and mixed-case mapsec
  were created together and the mapsec suffix should drive the map name,
  target group, normalized mapsec id, display name, layout rename, and script
  label rewrite in one CLI command.
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

Current GUI scope:

- scan a project root from Rust without invoking Porymap;
- scan the same data from a Vite-only `/api/scan` endpoint for browser /
  Playwright validation when Tauri system libraries are not installed;
- read map groups, map JSON, layouts, and region-map sections;
- show map/group/layout/mapsec relationships and selected-map warnings;
- visualize the selected map as a compact `Map -> Group -> Layout -> Mapsec`
  graph before the detailed field grid;
- filter map rows and generate a reviewable CLI plan command for the selected
  map, including target group, mapsec rename/display-name, layout rename, and
  script-label rewrite options;
- flag temporary-looking map names such as `test1` and mixed-case mapsec ids
  such as `MAPSEC_Jongle`, then suggest mapsec-aligned repair defaults such as
  `Jongle`, `gMapGroup_Jongle`, `MAPSEC_JONGLE`, and `JONGLE`;
- keep layout rename locked on in the rename flow because the default repair
  path should update map, layout id/name, and layout directory together;
- run Rust-core JSON plan generation from the Tauri GUI, then pass that plan
  into Rust-core `apply --dry-run` and display the resulting planned
  moves/edits plus move/edit/review counts without changing source files;
- after a successful dry-run, enable `Apply With Backup`; the user must confirm
  the write, then the GUI runs the same CLI real `apply --allow-dirty` path,
  creates `.map_asset_relinker_backups/*.bak.tar` before edits, rescans the
  project, and keeps a visible backup-path notice.

The GUI now treats Rust core as the scan, plan, and dry-run authority for the
Tauri path. Python remains the write authority for real apply / backup
behavior.

## Rust Core Migration

`tools/map_asset_relinker_core/` is the long-term shared implementation target.
The current migration slice owns project scanning, map relationship summaries,
the GUI-compatible JSON plan path, and read-only dry-run apply:

- `scan_project()` reads map groups, map JSON, layouts, and region-map sections;
- it returns the same camelCase `ProjectSummary` / `MapSummary` shape that the
  React GUI already consumes;
- the Tauri command `scan_project` now delegates to this crate instead of
  carrying its own duplicate scan implementation;
- `make_plan()` emits Python-compatible version-1 JSON plans for the main GUI
  rename / group move / mapsec rename / script-label rewrite path;
- `apply_plan_dry_run()` simulates plan application in memory and prints the
  same move/edit/review style output without touching source files;
- Tauri now writes those Rust-generated plans and invokes Rust for dry-run,
  then invokes Python only for real `apply --allow-dirty`;
- the crate also exposes a small CUI entry point:
  `cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- scan --root . --pretty`
  `cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- plan --root . --map OLD:NEW --out /tmp/plan.json`,
  and `cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- apply --root . --dry-run /tmp/plan.json`.

Python remains the real `apply` / backup compatibility surface in this slice.
The remaining migration order is expanding Rust plan parity beyond the GUI
subset, then porting real apply / backup creation after fixture tests prove the
Rust path.

Validation status for this scaffold:

- `npm install` succeeds and writes a local `package-lock.json`.
- `npm run build` passes for the React/Vite frontend.
- `cargo check --manifest-path tools/map_asset_relinker_core/Cargo.toml`
  passes for the new Rust core crate.
- `cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- scan
  --root /home/jastin/dev/pokeemerald-expansion --pretty` succeeds and reports
  946 maps, 79 groups, 792 layouts, 214 mapsecs, and 5 warnings after the live
  `Jongle` repair.
- Rust core `plan` can emit Python-compatible JSON for the fixture
  `OldCave_2:OldCave_2F` rename/group move and for the mixed-case mapsec
  `OldCave_2:Jongle` repair. Passing those plans to Rust core
  `apply --dry-run` produces the expected map/layout moves, structured edits,
  script-label edit, mapsec rename edit, and review output without changing
  files. The fixture checksum check confirms the dry-run path is read-only, and
  the dry-run output matches Python `apply --dry-run` byte-for-byte for both
  fixture plans.
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
  edits. The visible dry-run counters show 2 moves, 5 edits, and 5 review
  references. The CLI reports `Dry-run complete; no files changed.`
- Audit pane warnings that resolve to a map are buttons. Clicking a warning
  such as `test1: MAPSEC_Jongle...` selects that map and shows its relationship
  graph, field details, selected-map audit, and repair controls.
- In a local worktree containing the user-created `data/maps/test1` /
  `MAPSEC_Jongle` case, Playwright filtering for `test1` shows two selected-map
  warnings: temporary map name and mixed-case mapsec id. The GUI suggests
  `test1:Jongle`, `--to-group gMapGroup_Jongle`, `--rename-mapsec
  MAPSEC_Jongle:MAPSEC_JONGLE`, `--new-mapsec-name JONGLE`, and
  `--rewrite-script-labels`. Clicking `Run Dry-Run` creates
  `/tmp/jongle_relink_*.json`, reports 2 moves, 6 edits, and 5 review
  references, and the CLI reports `Dry-run complete; no files changed.`
- Playwright also confirmed GUI real apply on a `/tmp` fixture copy with
  `MAPSEC_Jongle`. After `Run Dry-Run`, `Apply With Backup` required
  confirmation, wrote `.map_asset_relinker_backups/map_relink_20260526_132228_346167.bak.tar`,
  rescanned automatically, displayed `Applied with backup: ...`, and updated
  the selected map from `gMapGroup_Temp` / `MAPSEC_Jongle` to
  `gMapGroup_Jongle` / `MAPSEC_JONGLE` with no selected-map issues.
- A 1040x720 Playwright viewport confirms the two-column layout path with the
  audit pane moved to a full-width row and no page-level overflow.
- `cargo check --manifest-path src-tauri/Cargo.toml` currently stops before
  checking the app because this Linux environment lacks Tauri's webview / GTK
  system packages (`javascriptcoregtk-4.1`, `libsoup-3.0`, `gdk-pixbuf-2.0`,
  `cairo`, and `atk`). Install the Tauri Linux prerequisites before native
  window validation.
- `tools/map_asset_relinker_gui/scripts/install_ubuntu_deps.sh` and
  `tools/map_asset_relinker_gui/scripts/build_native.sh` were added to make the
  native executable path explicit. `build_native.sh` now builds the Rust core
  release executable first, then checks `pkg-config`, then runs `npm install`
  and `npm run tauri build` when the Linux webview packages are present. It
  prints the generated executable / bundle paths after a successful build.
- `tools/map_asset_relinker_gui/scripts/build_core_release.sh` builds the CUI
  executable directly at
  `tools/map_asset_relinker_core/target/release/map-asset-relinker-core`. This
  is the part that can be generated in the current Linux environment without
  installing Tauri webview packages.
- `tools/map_asset_relinker_gui/scripts/build_windows.ps1` is the Windows
  PowerShell path for creating `map-asset-relinker-core.exe` and then running
  the Tauri desktop build for Windows app/bundle output.
- The native build helper now distinguishes `pkg-config` module names from
  Ubuntu package names. For example `gdk-3.0` and `gtk+-3.0` come from
  `libgtk-3-dev`, while `javascriptcoregtk-4.1` comes from
  `libjavascriptcoregtk-4.1-dev`.
- Attempting to install the Ubuntu prerequisites from this sandbox stopped at
  `sudo: a terminal is required to read the password`; the helper scripts are
  ready, but GUI executable generation still needs those packages installed on
  the host.

## Validation Evidence

- `python3 tools/map_asset_relinker/map_relink.py --help` lists the expected
  subcommands.
- `tools/map_asset_relinker/map_relink.sh --help` delegates to the Python CLI.
- `tools/map_asset_relinker/map_relink.sh audit` completes with the same
  diagnostics as the Python entry point.
- With the local `test1` / `MAPSEC_Jongle` authoring test present,
  `tools/map_asset_relinker/map_relink.sh audit --target test1` reports the
  mixed-case mapsec warning plus a temporary-looking map name warning.
- `tools/map_asset_relinker/map_relink.sh plan --map test1:Jongle --to-group
  gMapGroup_Jongle --rename-mapsec MAPSEC_Jongle:MAPSEC_JONGLE
  --new-mapsec-name JONGLE --rewrite-script-labels --out
  /tmp/jongle_test1_repair.json` followed by `apply --dry-run` plans the map
  and layout directory move, group creation/move, mapsec id/display-name
  rewrite, layout JSON update, map JSON update, script include update, and
  script-label rewrite without changing source files.
- `tools/map_asset_relinker/map_relink.sh plan-temp-mapsec --map test1
  --dry-run --out /tmp/test1_temp_mapsec_auto.json` produces the same repair
  through the CLI shortcut: `test1:Jongle`, `gMapGroup_Jongle`,
  `MAPSEC_JONGLE`, `JONGLE`, default layout rename, and default script-label
  rewrite.
- `tools/map_asset_relinker/map_relink.sh plan-temp-mapsec --map test1 --out
  /tmp/test1_temp_mapsec_apply.json` followed by `apply --allow-dirty
  /tmp/test1_temp_mapsec_apply.json` applied the live `test1` /
  `MAPSEC_Jongle` repair as `Jongle`. The backup archive is
  `.map_asset_relinker_backups/map_relink_20260526_125816_385658.bak.tar`;
  follow-up `validate --target Jongle` completed with 0 errors and the 5
  existing repository warnings, and `rg` found no remaining `MAPSEC_Jongle`,
  `"test1"`, `data/maps/test1`, `MAP_TEST1`, or `LAYOUT_TEST1` references in
  `data`, `src`, or `include`.
- The latest 2026-05-26 branch validation after GUI apply support passes:
  Python compile, `tools/map_asset_relinker/test_map_relink.sh`,
  `validate --target Jongle`, GUI `npm run build`, Tauri `cargo fmt --check`,
  `rtk make generated`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`,
  `rtk make -j16 -O check`, `rtk git diff --check`, and `rtk mdbook build
  docs`. mGBA Live boot validation succeeded through the local `mgba-qt`
  wrapper and stopped cleanly; the direct raw Qt path failed without a display.
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
