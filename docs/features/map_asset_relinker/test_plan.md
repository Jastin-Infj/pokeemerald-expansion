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
| Committed fixture test | `tools/map_asset_relinker/test_map_relink.sh` | Copies `testdata/basic` to `/tmp`, renames `OldCave_2` to `OldCave_2F`, moves it from `gMapGroup_Temp` to `gMapGroup_RougeCave`, confirms dry-run is read-only, applies for real, confirms `.map_asset_relinker_backups/*.bak.tar` was created, validates, checks map group, layout, include, warp, connection, and preserved dialogue text, repeats against a missing target group to confirm group creation, then creates broken fixture copies and confirms audit failure plus repair apply/validate for map group, map name, map id, layout, mapsec, mapsec rename with map name as anchor, empty bad group drop, layout tileset field repair, layout label repair, duplicate mapsec repair, mapsec display-name repair, map type, map-name popup flag, mapsec bounds, region-map cell, transition visit flag, claimed unused flag, Fly destination icon row, Fly mapsec type, Fly mapsec-to-map warp row, Fly icon style transitions (`palette-blink` / `blue-blink`, `stock`, `red-outline`), warp target, valid-but-wrong layout assignment, duplicate script include, opt-in script label prefix rewrite, `--match-by name`, and `--match-by id` cases. |
| Existing layout reassignment dry-run | `tools/map_asset_relinker/map_relink.sh plan --map AncientTomb:AncientTomb --no-layout-rename --set-layout-id LAYOUT_ANCIENT_TOMB --out /tmp/ancient_tomb_layout_repair.json`; then `tools/map_asset_relinker/map_relink.sh apply --dry-run /tmp/ancient_tomb_layout_repair.json` | Plan succeeds. After the current worktree has already restored the layout, dry-run reports no file changes. |
| Live Sample group repair | `tools/map_asset_relinker/map_relink.sh plan --map Temper1:Sample1 --to-group gMapGroup_Sample --rename-mapsec MAPSEC_Sampler:MAPSEC_SAMPLE --new-mapsec-name SAMPLE --rewrite-script-labels --out /tmp/sample1_repair.json`; then `apply --allow-dirty` | Repairs the real test case where the group name was correct but map name, map id, layout id/name/path, mapsec id/name, script include, and script labels were created from the wrong temporary name. Follow-up audit finds only the existing UnusedHouse warnings, and `rg` finds no remaining `Temper1`, `Temper2`, `Templar`, or `Sampler` references in map/layout/region-map/event-script data. |
| Live route rename with backup | `tools/map_asset_relinker/map_relink.sh plan --map Route201:Route301 --rename-mapsec MAPSEC_ROUTE_201:MAPSEC_ROUTE_301 --new-mapsec-name 'ROUTE 301' --rewrite-script-labels --out /tmp/route301_relink.json`; then `apply --allow-dirty` | Creates `.map_asset_relinker_backups/map_relink_20260525_222949_637106.bak.tar`, moves `data/maps/Route201` and `data/layouts/Route201` to `Route301`, rewrites map id, layout id/name/path, mapsec id/name, script include, group entry, and script labels. Follow-up `rg` finds no remaining `Route201` / `MAP_ROUTE201` / `LAYOUT_ROUTE201` / `MAPSEC_ROUTE_201` references in map/layout/region-map/event-script data. |
| Live Route301 world-map/Fly setup | `tools/map_asset_relinker/map_relink.sh plan --map Route301:Route301 --no-layout-rename --set-map-type MAP_TYPE_ROUTE --set-show-map-name true --set-transition-setflag FLAG_VISITED_ROUTE301 --set-mapsec-bounds MAPSEC_ROUTE_301:9:0:1:1 --set-region-map-cell hoenn:9:0:MAPSEC_ROUTE_301 --ensure-mapsec-map MAPSEC_ROUTE_301:MAP_ROUTE301:HEAL_LOCATION_NONE --ensure-fly-location hoenn:MAPSEC_ROUTE_301:FLAG_VISITED_ROUTE301 --ensure-fly-mapsec-type MAPSEC_ROUTE_301:FLAG_VISITED_ROUTE301 --set-fly-icon-style MAPSEC_ROUTE_301:palette-blink --claim-unused-flag FLAG_UNUSED_0x881:FLAG_VISITED_ROUTE301 --out /tmp/route301_worldmap.json`; then `apply --allow-dirty` | Creates `.map_asset_relinker_backups/map_relink_20260525_225213_796018.bak.tar` and `.map_asset_relinker_backups/map_relink_20260525_225438_483794.bak.tar`. `data/maps/Route301/map.json` is `MAP_TYPE_ROUTE`; `Route301_OnTransition` sets `FLAG_VISITED_ROUTE301`; `MAPSEC_ROUTE_301` has bounds and Hoenn cell `(9, 0)`; `sMapHealLocations`, `sFlyLocations`, `GetMapsecType()`, and `sPaletteBlinkFlyDestinations` all contain `MAPSEC_ROUTE_301`. |
| Live Route301 debug access | `Scripts -> Route301 Fly`, `Utilities -> Fly to map...`, and optional `Scripts -> Warp Route301` in the debug menu | `Route301 Fly` sets `FLAG_VISITED_ROUTE301` and badge 6 for compatibility, so the Route301 Fly icon should be selectable. `Warp Route301` directly warps to `MAP_ROUTE301` at `(7, 7)` to confirm the map exists and its transition script sets the same visit flag. The existing `Flags/Vars -> Toggle Locations` and `Cheat start` paths also include `FLAG_VISITED_ROUTE301`. |
| Live Route301 Fly icon blink | `Scripts -> Route301 Fly`; open `Utilities -> Fly to map...`; move the cursor to `ROUTE 301` | Route301 is listed in `sPaletteBlinkFlyDestinations`, so its selected Fly icon should stay visible and blink by palette swap over a small underlay instead of disappearing over route artwork. Existing stock cities still use the original hide/show callback path. |
| Live Route301 Fly icon style no-op | `tools/map_asset_relinker/map_relink.sh plan --map Route301:Route301 --no-layout-rename --set-fly-icon-style MAPSEC_ROUTE_301:palette-blink --out /tmp/route301_fly_icon_style.json`; then `apply --dry-run` | Dry-run reports no file changes, confirming the live Route301 source already matches the toolized `palette-blink` style. |
| GUI scaffold source check | `rtk cargo fmt --manifest-path tools/map_asset_relinker_gui/src-tauri/Cargo.toml --check`; `cd tools/map_asset_relinker_gui && npm install && npm run build`; `cargo check --manifest-path src-tauri/Cargo.toml` | Rust source is formatted. `npm install` and `npm run build` pass. `cargo check` downloads Rust deps but is blocked on this Linux environment by missing Tauri system packages: `javascriptcoregtk-4.1`, `libsoup-3.0`, `gdk-pixbuf-2.0`, `cairo`, and `atk`; install the Tauri Linux prerequisites before native-window validation. |
| Rust core scan check | `rtk cargo check --manifest-path tools/map_asset_relinker_core/Cargo.toml`; `rtk cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- scan --root /home/jastin/dev/pokeemerald-expansion --pretty` | The core crate compiles and the CUI scan path returns the GUI summary JSON. After the live `Jongle` repair, scan reports 946 maps, 79 groups, 792 layouts, 214 mapsecs, and 5 warnings. |
| Rust core plan compatibility | On fixture copies, run `cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- plan --root <tmp> --map OldCave_2:OldCave_2F --from-group gMapGroup_Temp --to-group gMapGroup_RougeCave --out <tmp>/rust-plan.json`; then Python `apply --dry-run`; repeat with `--map OldCave_2:Jongle --to-group gMapGroup_Jongle --rename-mapsec MAPSEC_Jongle:MAPSEC_JONGLE --new-mapsec-name JONGLE --rewrite-script-labels` | Rust emits valid version-1 JSON plans accepted by the existing Python apply path. Dry-run reports the expected map/layout moves, map group edit, layout edit, map JSON edit, event script edit, mapsec edit for the Jongle case, script-label edit when requested, and no file changes. |
| GUI native build helper | `tools/map_asset_relinker_gui/scripts/build_native.sh` | Checks required Tauri Linux `pkg-config` modules before building. In the current environment it exits early because `gdk-3.0`, `gtk+-3.0`, `javascriptcoregtk-4.1`, `libsoup-3.0`, and `webkit2gtk-4.1` are not installed, then prints the correct Ubuntu package names: `libgtk-3-dev`, `libjavascriptcoregtk-4.1-dev`, `libsoup-3.0-dev`, and `libwebkit2gtk-4.1-dev`. |
| GUI Playwright scan smoke | Start `npm run dev -- --host 127.0.0.1`, open `http://127.0.0.1:1420/`, wait for `Loaded 945 maps` | Dev-only `/api/scan` returns real repo data. Metrics show 945 maps, 78 groups, 791 layouts, 213 mapsecs, and 5 warnings. |
| GUI Playwright plan preview | In the browser, filter `Route301`, select it, set new map name `Route401`, enable `Rewrite script labels` | Plan preview prints `tools/map_asset_relinker/map_relink.sh plan --map Route301:Route401 --to-group gMapGroup_TownsAndRoutes --rewrite-script-labels --out /tmp/route401_relink.json` followed by `apply --dry-run /tmp/route401_relink.json`. |
| GUI layout rename default | Open the Route301 plan panel | `Rename layout with map` is checked and locked. The generated plan does not include `--no-layout-rename`, so layout id/name/path remain part of the default rename plan. |
| GUI Playwright dry-run execution | With the same Route301 setup, click `Run Dry-Run` | The GUI status becomes `Dry-run complete`, displays counters for 2 moves, 5 edits, and 5 review references, displays a `/tmp/route401_relink_*.json` plan path, and shows the existing CLI dry-run output: layout/map moves, structured edits, review references, and `Dry-run complete; no files changed.` |
| GUI Playwright audit warning navigation | In a local worktree containing `data/maps/test1`, click the right-side `test1: MAPSEC_Jongle...` audit warning | The detail pane switches to `test1`, showing its relationship graph, selected-map audit, and repair controls. |
| GUI Playwright `test1` / `MAPSEC_Jongle` repair | In a local worktree containing `data/maps/test1` with `region_map_section: MAPSEC_Jongle`, click the audit warning or filter `test1`, then click `Run Dry-Run` | The selected map shows temporary-name and mixed-case mapsec warnings. The plan defaults to `--map test1:Jongle --to-group gMapGroup_Jongle --rename-mapsec MAPSEC_Jongle:MAPSEC_JONGLE --new-mapsec-name JONGLE --rewrite-script-labels`, then the dev API dry-run reports 2 moves, 6 edits, 5 review references, and `Dry-run complete; no files changed.` |
| GUI Playwright apply with backup | In a `/tmp` fixture project containing `MAPSEC_Jongle`, click `Run Dry-Run`, then `Apply With Backup`, and accept the confirmation dialog | The GUI uses the same CLI write path, creates `.map_asset_relinker_backups/map_relink_20260526_132228_346167.bak.tar`, shows `Applied with backup: ...`, rescans automatically, and updates the selected map to `gMapGroup_Jongle` / `MAPSEC_JONGLE` with no selected-map issues. Follow-up CLI `validate --target MAPSEC_JONGLE` reports 0 errors and one fixture-only empty-group warning. |
| CLI `test1` / `MAPSEC_Jongle` audit and dry-run | `tools/map_asset_relinker/map_relink.sh audit --target test1`; then `tools/map_asset_relinker/map_relink.sh plan --map test1:Jongle --to-group gMapGroup_Jongle --rename-mapsec MAPSEC_Jongle:MAPSEC_JONGLE --new-mapsec-name JONGLE --rewrite-script-labels --out /tmp/jongle_test1_repair.json` and `apply --dry-run` | Audit warns on the mixed-case mapsec id and temporary-looking map name. Dry-run plans the map/layout move, group repair, mapsec rename/display-name repair, layout JSON edit, map JSON edit, script include edit, and script-label rewrite without changing source files. |
| CLI `plan-temp-mapsec` shortcut | `tools/map_asset_relinker/map_relink.sh plan-temp-mapsec --map test1 --dry-run --out /tmp/test1_temp_mapsec_auto.json` | Infers the same mapsec-derived repair from source data without GUI-only logic: `test1:Jongle`, `gMapGroup_Jongle`, `MAPSEC_JONGLE`, `JONGLE`, default layout rename, default script-label rewrite, and dry-run without changing source files. |
| CLI `plan-temp-mapsec` real apply | `tools/map_asset_relinker/map_relink.sh plan-temp-mapsec --map test1 --out /tmp/test1_temp_mapsec_apply.json`; then `tools/map_asset_relinker/map_relink.sh apply --allow-dirty /tmp/test1_temp_mapsec_apply.json` | Applies the live `test1` / `MAPSEC_Jongle` repair as `Jongle`, creates `.map_asset_relinker_backups/map_relink_20260526_125816_385658.bak.tar`, moves `data/maps/test1` and `data/layouts/test1` to `Jongle`, rewrites map group, layout JSON, map JSON, event script include, script label, and region-map section data, and follow-up `validate --target Jongle` completes with 0 errors plus the 5 existing repo warnings. |
| GUI Playwright layout overflow | Check 1320x860 and 1040x720 viewports | No page-level horizontal or vertical overflow. The 1320x860 view keeps map/detail/audit panes in three columns; the 1040x720 view moves the audit pane to a full-width second row. |
| Python compile | `python3 -m py_compile tools/map_asset_relinker/map_relink.py` | Passes after adding Fly/mapsec-map/flag plan options. |
| Diff whitespace | `rtk git diff --check` | Passes. |
| Generated source refresh | `rtk make generated` | Passes and regenerates map / mapsec / heal-location outputs from source JSON. |
| Normal ROM build | `rtk make -j16 -O all` | Passes with existing RWX linker warning. |
| Debug ROM build | `rtk make -j16 -O debug` | Passes with existing RWX linker warning. |
| Test ROM build/check | `rtk make -j16 -O check` | Passes with existing RWX linker warning. |
| Docs build | `rtk mdbook build docs` | Passes with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large search index. |
| mGBA Live boot smoke | `mgba-live-cli start --rom pokeemerald.gba --session-id map-relink-route301-blink --mgba-path mgba-qt`; screenshot; stop | Boots into the intro flow, captures a nonblank frame, and stops cleanly. This confirms the built ROM starts after the Route301 blink change; Route301 Fly selection / blink still needs a manual in-game check with `Scripts -> Route301 Fly` followed by `Utilities -> Fly to map...`. |

Current `audit` warnings on `master` are pre-existing:

- `Route19_UnusedHouse_Frlg`, `Route23_UnusedHouse`,
  `Route6_UnusedHouse_Frlg`, and `SevenIsland_UnusedHouse` exist but are not
  listed in `map_groups.json`.
- `Route19_UnusedHouse_Frlg/scripts.inc` is not included by
  `data/event_scripts.s`.

## Latest Branch Validation (2026-05-26)

After applying the live `test1` / `MAPSEC_Jongle` repair as `Jongle` and adding
GUI real apply:

- `rtk python3 -m py_compile tools/map_asset_relinker/map_relink.py` passes.
- `rtk tools/map_asset_relinker/test_map_relink.sh` passes.
- `rtk tools/map_asset_relinker/map_relink.sh validate --target Jongle` passes
  with 0 errors and the 5 existing unused-map / script-include warnings.
- `rtk bash -lc 'cd tools/map_asset_relinker_gui && npm run build'` passes.
- `rtk cargo fmt --manifest-path tools/map_asset_relinker_gui/src-tauri/Cargo.toml --check`
  passes.
- `rtk cargo check --manifest-path tools/map_asset_relinker_core/Cargo.toml`
  passes.
- `rtk cargo run --manifest-path tools/map_asset_relinker_core/Cargo.toml -- scan
  --root /home/jastin/dev/pokeemerald-expansion --pretty` succeeds and reports
  946 maps, 79 groups, 792 layouts, 214 mapsecs, and 5 warnings.
- Rust core `plan` fixture checks pass for the basic `OldCave_2:OldCave_2F`
  rename/group move and the mixed-case mapsec `OldCave_2:Jongle` repair; the
  generated plans are valid JSON and Python `apply --dry-run` accepts them.
- `rtk cargo check --manifest-path tools/map_asset_relinker_gui/src-tauri/Cargo.toml`
  still stops on missing Linux Tauri system packages (`atk`, `pango`, `cairo`,
  `gdk-3.0`, `gdk-pixbuf-2.0`, `javascriptcoregtk-4.1`, and `libsoup-3.0`);
  the new core crate itself checked successfully before those system package
  failures.
- Playwright verified GUI dry-run plus `Apply With Backup` on
  `/tmp/map-relink-gui-apply-root-4`: the GUI confirmed the write, created
  `.map_asset_relinker_backups/map_relink_20260526_132228_346167.bak.tar`,
  displayed the backup notice, rescanned, and showed `gMapGroup_Jongle` /
  `MAPSEC_JONGLE` with no selected-map issues. CLI validation of that fixture
  reported 0 errors and one fixture-only empty-group warning.
- `rtk make generated` passes.
- `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and
  `rtk make -j16 -O check` pass with the existing RWX linker warning.
- `rtk git diff --check` passes.
- `rtk mdbook build docs` passes with existing warnings: missing root
  `CHANGELOG.md`, existing `CREDITS.md` `</img>`, and large search index.
- mGBA Live boot smoke: starting with the raw script-capable Qt path failed
  because no display was available, but starting with the local `mgba-qt`
  wrapper succeeded, `mgba_live_get_view` captured the Emerald title screen,
  and `mgba_live_stop` stopped `map-relink-jongle-boot-wrapper` cleanly.
- GitHub Actions were not re-waited; local build, check, GUI, Playwright, and
  mGBA evidence are the handoff validation for this slice.

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
| Repair wrong existing layout | Fixture map points at another existing layout id | `plan --no-layout-rename --set-layout-id <correct-layout-id>` rewrites only the selected map's `layout` field and leaves layout entries untouched. |
| Repair mapsec typo | Fixture map has a typoed `region_map_section` | `plan --new-mapsec <valid-mapsec>` rewrites the mapsec field and `validate` passes. |
| Repair wrong existing mapsec and empty bad group | Fixture map name/layout are correct, mapsec id is wrong-but-valid, and an empty accidental group exists | `plan --no-layout-rename --to-group <correct-group> --rename-mapsec OLD:NEW --new-mapsec-name <name> --drop-group <bad-group>` moves the map to the correct group, renames the mapsec id/name, preserves the layout, and removes the empty bad group. |
| Repair layout tileset field | Fixture layout id/name/path are correct but `primary_tileset` is wrong | `plan --no-layout-rename --set-primary-tileset <tileset>` edits only the selected layout's tileset field and preserves layout identity. |
| Repair layout label only | Fixture layout id/path are correct but the layout `name` duplicates another layout | `plan --no-layout-rename --set-layout-name <layout-label>` edits only the selected layout label and `validate` passes. |
| Repair duplicate mapsec id | Fixture region map sections contain the same `MAPSEC_*` twice | Audit fails, then `plan --no-layout-rename --rename-mapsec OLD:NEW --new-mapsec-name <name>` renames one section and rewrites map references. |
| Repair mapsec display name only | Fixture mapsec id is correct but display `name` is wrong | `plan --no-layout-rename --set-mapsec-name MAPSEC_ID:<name>` edits only the mapsec display name. |
| Repair map popup / Town Map / Fly metadata | Fixture has a valid mapsec but lacks post-creation world-map setup | `plan --no-layout-rename --new-mapsec <id> --set-map-type MAP_TYPE_ROUTE --set-show-map-name false --set-transition-setflag <flag> --set-mapsec-bounds <id>:x:y:w:h --set-region-map-cell hoenn:x:y:<id> --ensure-mapsec-map <id>:<map>:HEAL_LOCATION_NONE --ensure-fly-location hoenn:<id>:<flag> --ensure-fly-mapsec-type <id>:<flag> --set-fly-icon-style <id>:palette-blink --claim-unused-flag <unused>:<flag>` updates map JSON, map scripts, mapsec bounds JSON, region map layout cell, `sMapHealLocations`, `sFlyLocations`, `GetMapsecType()`, Fly icon style membership, and flags. |
| Switch Fly icon style | Fixture Fly destination has a valid `sFlyLocations` row | `--set-fly-icon-style <id>:palette-blink` and `--set-fly-icon-style <id>:blue-blink` both insert the mapsec into `sPaletteBlinkFlyDestinations`; `--set-fly-icon-style <id>:stock` removes it; `--set-fly-icon-style <id>:red-outline` removes palette blink and adds a `sRedOutlineFlyDestinations` entry using the Fly location flag. |
| Repair warp target typo | Fixture warp points at a typoed old map id | `plan --old-map-id <typoed-map-id>` rewrites the bad target id to the new map id and `validate` passes. |
| Repair by map name anchor | Fixture directory is typoed but `map.json` `name` is correct | `plan --match-by name` selects the map, moves the actual directory, fixes script include, and `validate` passes. |
| Repair by map id anchor | Fixture directory and `map.json` `name` are typoed but map id is correct | `plan --match-by id --old-group-map-name <group-entry>` selects the map, repairs name/directory/group/include, and `validate` passes. |
| Deduplicate script include | Fixture `data/event_scripts.s` contains two exact includes for the same map | Applying a plan for that map keeps the first include and removes later duplicates. |
| Repair script label prefix | Fixture map is renamed from `OldCave_2` to `OldCave_2F` with `--rewrite-script-labels` | The selected map's `scripts.inc` changes labels/references to `OldCave_2F_*`, while literal `.string` text containing `OldCave_2` is preserved. |
| Dry-run is read-only | Run `apply --dry-run` | No file contents or paths change. |
| Real apply backup | Run real `apply` on fixture | A `.bak.tar` archive is created under `.map_asset_relinker_backups` before source files are edited or moved. |
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
  `MAPSEC_*`: mapsec bounds, region-map cell, `sMapHealLocations`,
  `sFlyLocations`, `GetMapsecType()`, and a real unlock flag path all need to
  agree.
- Wild encounters if the map has encounter data outside the map JSON.
- Trainer / object scripts if labels were renamed.
- Save compatibility if map group order or map number changes are intentional.
