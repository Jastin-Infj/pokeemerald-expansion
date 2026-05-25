# Map Asset Relinker MVP Plan

## MVP Goal

Build a dry-run-first CLI that can safely rename or relink one map plus its
layout assets without touching Porymap internals.

The MVP should handle the common case:

- one map directory rename;
- one map `id` / `name` rename;
- one layout id / label / directory rename;
- map group membership update, including temporary group to final group moves;
- script include path update;
- warp / connection references to the renamed map.

## Phase 1: Audit-Only Tool

Input: current checkout.

Output:

- list of map directories not present in `map_groups.json`;
- maps in `map_groups.json` whose directory or `map.json` is missing;
- `map.json` entries where `name` does not match the directory;
- `map.json` layout ids that do not exist in `layouts.json`;
- layout entries whose `border_filepath` / `blockdata_filepath` does not exist;
- layout entry names that look like generated placeholders
  (`LAYOUT_*_Layout`, duplicate `_Layout`, numeric suffix without floor label);
- `data/event_scripts.s` includes that point to missing map script files;
- `scripts.inc` files not included by `data/event_scripts.s`;
- remaining textual references to a target old map name or old layout id.

This phase has no write path and is safe to run on `master`.

## Phase 2: Plan File

Use a JSON plan so edits are reviewable before they run.

Example:

```json
{
  "maps": [
    {
      "oldName": "RougeCave_2",
      "newName": "RougeCave_2F",
      "oldId": "MAP_ROUGE_CAVE_2",
      "newId": "MAP_ROUGE_CAVE_2F",
      "group": "gMapGroup_RougeCave",
      "fromGroup": "gMapGroup_Temp",
      "toGroup": "gMapGroup_RougeCave"
    }
  ],
  "layouts": [
    {
      "oldId": "LAYOUT_ROUGE_CAVE_2",
      "newId": "LAYOUT_ROUGE_CAVE_2F",
      "oldName": "RougeCave_2_Layout",
      "newName": "RougeCave_2F_Layout",
      "oldDir": "data/layouts/RougeCave_2",
      "newDir": "data/layouts/RougeCave_2F"
    }
  ]
}
```

The tool may generate this plan from CLI flags, but the user should be able to
edit it before apply.

## Phase 3: Dry-Run Apply

Dry-run output should be explicit:

- files to move;
- JSON fields to update;
- text include lines to update;
- references intentionally not changed;
- warnings that block apply.

Blocking warnings:

- destination file / directory already exists unless `--overwrite` is passed;
- old map / layout appears in multiple ambiguous roles;
- JSON parse error;
- map group would remove the only reference to a map without adding the new one;
- layout binary files would be orphaned.

## Phase 4: Apply

Apply order:

1. Load and validate all source files.
2. Compute all file moves and edits.
3. Refuse to continue if the working tree has unstaged changes in target files
   unless `--allow-dirty` is passed.
4. Move directories / files.
5. Write JSON with stable indentation.
6. Write constrained text edits.
7. Run internal validation.

The tool should not run `git add` or commit. Git remains user / agent owned.

## Phase 5: Validation Integration

Recommended command after apply:

```bash
rtk make generated
rtk make -j16 -O debug
```

If only `audit` ran, no build is required.

## Initial Files

| File | Role |
|---|---|
| `tools/map_asset_relinker/map_relink.py` | CLI entry point. |
| `tools/map_asset_relinker/README.md` | Tool-specific usage examples. |
| `test/map_asset_relinker/` or focused fixture dir | Tiny JSON fixture tests if the repo test harness permits non-ROM Python tests. |

## Acceptance Criteria

- Can audit current `master` without modifying files.
- Can produce a plan for a `RougeCave_2` to `RougeCave_2F` rename.
- Can move a renamed map from a temporary map group into a final map group.
- Dry-run lists all map/layout/script include changes.
- Apply updates only known structural files.
- Generated map/layout constants can be regenerated after apply.
