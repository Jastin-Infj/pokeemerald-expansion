# Bag Expansion Test Plan

## Build / Lint

| Test | Command | Expected |
|---|---|---|
| Normal build | `rtk make -j16 -O all` | Build passes. |
| Focused check | `rtk make -j16 -O check TESTS=save` | Save compatibility test either passes unchanged or fails only with intentional expected-size update. |
| Full check | `rtk make -j16 -O check` | Item / save related checks pass. |
| Debug build | `rtk make -j16 -O debug` | Required if debug fill or Field Kit debug routes are used for validation. |

## Focused Tests

| Test | Steps | Expected |
|---|---|---|
| SaveBlock size | Compare `sizeof(struct SaveBlock1)` against `test/save.c`. | Any size change is intentional and documented. |
| Save sector format | If `SAVE_BLOCK_3_CHUNK_SIZE` / `SECTOR_DATA_SIZE` changes, save a file and reload it across reset. | SaveBlock1, SaveBlock2, PokemonStorage, and relocated/removed SaveBlock3 data remain valid. |
| Save slot sector count | If `NUM_SECTORS_PER_SLOT` or sector IDs change, save/load across multiple save rotations and power cycles. | Both save slots are valid; old special sectors do not corrupt normal save data. |
| Special-sector policy | If Hall of Fame, Trainer Hill, or Recorded Battle sectors are consumed, exercise the disabled/replaced feature path. | The feature is either unavailable by policy or uses a new storage path without corrupting normal saves. |
| DexNav SaveBlock3 pressure | If DexNav search levels are enabled, build with all planned SaveBlock3 fields. | `SaveBlock3FreeSpace` passes and save/load preserves DexNav state. |
| Affected pocket fill | Add enough distinct items to fill the expanded pocket. | `CheckBagHasSpace` returns false only when the new capacity is reached. |
| TM/HM 350 fill | Fill TM/HM to 350 entries plus Cancel and scroll/sort the full list. | No `u8` wrap, list truncation, cursor corruption, or wrong item selection. |
| Sort affected pocket | Fill and sort the expanded pocket by each available sort mode. | No crash, lost item, duplicate item, or cursor corruption. |
| Save/load bag | Fill affected pocket, save, reload. | Item IDs, quantities, cursor positions, and registered item state remain valid. |

## Manual Checks

| Check | Steps | Expected |
|---|---|---|
| Bag UI scroll | Open the expanded pocket and scroll from first to last item. | Smooth scrolling; Cancel row appears correctly. |
| Debug fill | Use debug PC/Bag fill for affected pockets. | Fill completes; bag remains usable. |
| Full catalog stress | In a debug build, fill the largest affected pocket to its target capacity and sort it. | No allocation failure, hang, cursor corruption, lost item, or duplicated item. |
| Items `sortType` filter | If held-item / Mega Stone / battle-item filters are added, switch each filter with a mixed Items pocket. | Filtered view is correct; item use/give/toss actions still target the underlying slot. |
| Field Kit | Register and use `ITEM_FIELD_KIT` from Key Items. | Field Kit menu still opens and returns cleanly. |
| Bag full script | Trigger an item gift when the target pocket is full. | Existing bag full message appears and no flag is set early. |
| Wally tutorial | Run Wally tutorial bag route. | Temporary item / Poke Ball pockets restore correctly. |
| Battle Pyramid | Enter / exit Pyramid-style inventory paths if touched indirectly. | Pyramid bag behavior unchanged. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-09 | Docs-only kickoff | Not run | No source / data / config changes. Runtime validation will be required once pocket constants change. |

## Feature Complete Gate

- Target pocket counts and save compatibility policy are documented.
- Any pocket target above 255 has explicit ROM header and bag UI count-width changes; any target above 1023 has an explicit `BagPocket.capacity` redesign.
- A 1000-slot raw bag target has an explicit SaveBlock1 `FREE_*`, SaveBlock3 chunk reclaim, 15-sector normal save, PokemonStorage shrink, or equivalent save-space plan.
- Any large item-ID increase has an explicit Pokemon `heldItem:10` / `ITEMS_COUNT < 1024` plan.
- `test/save.c` either remains unchanged or has an intentional expected-size update.
- `rtk make -j16 -O all`, `rtk make -j16 -O check`, and required focused save tests pass.
- mGBA Live or manual runtime confirms bag open, fill, scroll, save/load, and affected downstream feature path.
- `docs/features/feature_registry.md`, affected feature docs, and `docs/upgrades/upstream_diff_checklist.md` link the final policy.

## Open Questions

- Which `.sav` files should be used for migration regression if save compatibility is required?
- Does the final target need mGBA validation for both normal and debug ROMs?
