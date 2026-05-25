# Map Asset Relinker Investigation

## Problem

新規 map を作る時、Porymap 上で仮名を入れて作業すると、後から正式名に直す
範囲が広い。

例:

- map directory: `data/maps/RougeCave_2/`
- map JSON: `id`, `name`, `layout`
- map group: `data/maps/map_groups.json`
- layout JSON: `id`, `name`, `border_filepath`, `blockdata_filepath`
- layout directory: `data/layouts/RougeCave_2/`
- scripts include: `data/event_scripts.s`
- map-to-map references: warps / connections / scripts using `MAP_*`

手作業でやると、Porymap からは見えやすい部分だけ直って、build や runtime で
初めて漏れが出る。

## Local Source Findings

| Area | Source of truth | Generated output / user impact |
|---|---|---|
| Map directory | `data/maps/<MapName>/` | Porymap map list and script include paths. |
| Map identity | `data/maps/<MapName>/map.json` `id`, `name` | `MAP_*`, map header label, generated event constants. |
| Map group | `data/maps/map_groups.json` | `include/constants/map_groups.h`, `data/maps/groups.inc`, `headers.inc`, `events.inc`, `connections.inc`, `src/data/map_group_count.h`. |
| Layout registry | `data/layouts/layouts.json` | `include/constants/layouts.h`, `data/layouts/layouts.inc`, `layouts_table.inc`. |
| Layout binary paths | `border_filepath`, `blockdata_filepath` | `mapjson layouts` emits `.incbin` labels from the JSON paths. |
| Script body | `data/maps/<MapName>/scripts.inc` | Not generated; labels must keep matching object scripts and map script labels. |
| Script include | `data/event_scripts.s` | Must include the renamed `scripts.inc`; otherwise scripts are not assembled. |
| Warp references | `map.json` `warp_events[].dest_map` | Usually `MAP_*`; must follow renamed map IDs. |
| Connections | `map.json` `connections[].map` | Usually `MAP_*`; must follow renamed map IDs. |
| Region / Fly | `region_map_section`, region map JSON / C tables | Should be audited, not auto-created in MVP. |

`tools/mapjson` already validates several downstream edges:

- `map` mode fails if `map.json` references a missing layout id.
- `groups` mode emits map constants from `map_groups.json`.
- `layouts` mode emits layout constants only for layouts whose binary files
  exist.

However, `mapjson` is a generator, not a refactor tool. It does not rename
directories, update `data/event_scripts.s`, or rewrite cross-map references.

## New-Map Branch Audit

`feature/new-map-test-v15` shows the failure mode clearly:

- `RougeCave_2` was added as a map and layout name.
- `RougeCave_B1F` also exists, but `map_groups.json` in the inspected branch
  only listed `RougeCave` and `RougeCave_2` under `gMapGroup_RougeCave`.
- Layout entries include mixed naming such as `LAYOUT_ROUGE_CAVE_LAYOUT`,
  `RougeCave_Layout`, `LAYOUT_ROUGE_CAVE_2`, and `RougeCave_2_Layout`.
- There are extra layout directories from iteration:
  `LAYOUT_ROUGE_CAVE_B2F_Layout/`, `RougeCave_B1F_Layout/`,
  `RougeCave_B2F_Layout/`, `RougeCave_2/`.

This is exactly the type of drift a relinker should surface before the user
tries to repair it by hand.

## Proposed Tool Shape

CLI name candidate:

```text
tools/map_asset_relinker/map_relink.py
```

Commands:

| Command | Purpose |
|---|---|
| `audit` | Build an index of maps, layouts, files, generated references, and suspicious mismatches. |
| `plan` | Produce a JSON plan from old/new map and layout names without modifying files. |
| `apply --dry-run` | Show file moves and JSON/text edits. |
| `apply` | Execute file moves and structured JSON updates. |
| `validate` | Run cheap consistency checks after edits. |

Initial invocation sketch:

```bash
python3 tools/map_asset_relinker/map_relink.py audit
python3 tools/map_asset_relinker/map_relink.py plan \
  --map RougeCave_2:RougeCave_2F \
  --layout LAYOUT_ROUGE_CAVE_2:LAYOUT_ROUGE_CAVE_2F \
  --layout-dir RougeCave_2:RougeCave_2F \
  --out /tmp/rouge_cave_rename.json
python3 tools/map_asset_relinker/map_relink.py apply --dry-run /tmp/rouge_cave_rename.json
```

## Structured vs Text Updates

Use structured JSON for:

- `data/maps/map_groups.json`
- `data/maps/<MapName>/map.json`
- `data/layouts/layouts.json`

Use constrained text updates for:

- `data/event_scripts.s` `.include "data/maps/<MapName>/scripts.inc"`
- script labels inside `data/maps/<MapName>/scripts.inc`, only when explicitly
  requested by the plan.

Do not broad-replace every matching token by default. Map names often appear in
comments, strings, story text, and docs. The MVP should rewrite known structural
edges first, then report remaining textual matches for human review.

## Validation Hooks

Minimum validation after apply:

- `python3 -m json.tool` for changed JSON files.
- `rtk make generated` or the equivalent `mapjson` targets.
- `rtk make -j16 -O debug` when script includes or debug warp routes changed.
- Optional mGBA check when the rename affects an entered map.

## Recommended First Implementation

Start with Python. The repo already uses Python for data generation, and this
tool needs filesystem / JSON orchestration more than compile-time performance.
Rust can be reconsidered if the tool grows into a long-lived Porymap-adjacent
workflow with richer schema validation.
