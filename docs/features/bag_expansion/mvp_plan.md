# Bag Expansion MVP Plan

## MVP

Do not start by changing every pocket. The first implementation slice should choose one
capacity target and prove the save/UI path.

Recommended first slice:

1. Treat 1000 total normal bag slots as the full target, not the first source-only slice.
2. Use `include/constants/items.h` / `ITEMS_COUNT` as the canonical item catalog count; current real item count is 873 excluding `ITEM_NONE`.
3. Plan the likely target as TM/HM 350 slots plus 650 slots across the other normal pockets.
4. Decide whether the implementation accepts a save-format change. `FREE_*` alone reaches only about 891 total slots; SaveBlock3 chunk reclaim reaches about 1007 but conflicts with DexNav search levels; adding a fifth SaveBlock1 sector by consuming Hall of Fame sectors is the cleaner raw-slot path if DexNav must remain available.
5. Keep TM/HM 350 as a raw-pocket design only if the feature also widens bag UI count fields and handles ROM header counts above 255.
6. Prefer `POCKET_ITEMS` filters for held items / battle items / Mega Stones before adding true pockets.
7. Treat the change as save-breaking unless a migration path is designed before code changes.

## Non-Goals

- Do not move existing item IDs as part of pocket capacity work.
- Do not implement per-HM key items in this feature.
- Do not use SaveBlock3 as storage for normal bag item slots. Reclaiming SaveBlock3 chunk bytes is a save-format change, not normal bag storage.
- Do not change Battle Pyramid bag capacity unless a separate facility inventory feature requires it.
- Do not make Champions Challenge bag state persistent here.
- Do not add real held-item / Mega Stone / battle-item pockets unless the feature explicitly accepts the larger save/UI scope.

## Implementation Steps

| Step | Files | Notes |
|---|---|---|
| 1 | `docs/features/bag_expansion/*` | Lock target pocket counts and save compatibility policy before source edits. |
| 2 | `include/constants/global.h` | Update only the selected `BAG_*_COUNT` constants. |
| 3 | `include/global.h` | Verify `struct Bag` and `struct SaveBlock1` offsets / size after the constant change. |
| 4 | `src/item.c` | Confirm `SetBagItemsPointers`, encryption key updates, add/remove/space helpers still use the new capacities. |
| 5 | `include/item_menu.h`, `src/item_menu.c`, `src/menu_helpers.c` | Widen bag/list item count fields that are currently `u8` before any pocket exceeds 255. |
| 6 | `src/item_menu.c` | Confirm `MAX_POCKET_ITEMS`, list buffers, sort allocation, and Wally tutorial snapshots still fit and behave. |
| 7 | `src/rom_header_gf.c` | Redesign or clamp bag count fields before allowing TM/HM 350; current fields are `u8`. |
| 8 | `include/save.h`, `src/save.c` | If targeting 1000 slots, choose the save-format path: SaveBlock3 chunk reclaim, 14 -> 15 normal-save sectors, PokemonStorage shrink, or custom special-sector storage. |
| 9 | `test/save.c` | Update expected SaveBlock sizes only if the save-breaking change is accepted. If migration is required, add migration tests first. |
| 10 | `docs/upgrades/upstream_diff_checklist.md` | Add bag constants / `struct Bag` / save layout to the upstream recheck list. |

## Current Contract

- Existing `master` behavior remains unchanged.
- Normal bag slot arrays live in `struct SaveBlock1`.
- Every added normal bag slot costs approximately 4 bytes of SaveBlock1.
- `SaveBlock1` has 304 bytes of current spare capacity by `test/save.c`.
- SaveBlock1 `FREE_*` toggles can reclaim 2516 bytes; with current spare that supports about 891 total normal bag slots.
- A 1000-slot raw bag target needs about 3256 bytes. It only fits the known budget if SaveBlock3 chunk bytes are reclaimed or another 436+ bytes are found.
- If DexNav search levels are enabled, SaveBlock3 chunk reclaim should be treated as unavailable. `NUM_SPECIES` is currently 1573, so DexNav search levels plus chain state nearly fill the 1624 B SaveBlock3 budget.
- A 14 -> 15 normal-save-sector layout can give SaveBlock1 one extra sector while preserving SaveBlock3, but it consumes the Hall of Fame special sectors and is save-breaking.
- Counts at or below 255 fit the current `rom_header_gf.c` count field type and current bag UI count cache. Counts above 255 are out of contract until those are widened/redesigned.
- `BagPocket.capacity` is currently a 10-bit field, so raw pocket capacities above 1023 are out of contract.
- `ITEMS_COUNT` must stay below 1024 unless the Pokemon held-item save layout changes.
- Held item / battle item / Mega Stone groups are currently `sortType` values inside `POCKET_ITEMS`.

## Validation Targets

| Area | Required check |
|---|---|
| Save size | `test/save.c` must either remain unchanged or be intentionally updated with migration notes. |
| Build | `rtk make -j16 -O all` and `rtk make -j16 -O check`. |
| Debug | Fill each affected pocket from debug menu; no overflow, hang, or wrong pocket. |
| Runtime UI | Open bag, sort affected pocket, scroll to the end, close/reopen, save/load. |
| Regression | Wally tutorial bag, registered Key Item, Field Kit use path, item gift bag-full path. |

## Future Work

- 350 TM ownership model: large TM/HM pocket vs virtual registry vs shop/relearner policy.
- Items-pocket filter UI for held items, battle items, Mega Stones, Z-Crystals, and other `sortType` groups.
- Save migration design for old `.sav` files.
- Optional `FREE_*` save capacity policy if the desired target exceeds SaveBlock1 spare.
- Save-format policy for raw 1000 slots: SaveBlock3 chunk reclaim vs 15-sector normal save vs PokemonStorage shrink vs special-sector extension.
- Champions Challenge normal bag snapshot sizing after the final `struct Bag` size is known.

## Open Questions

- Should the first implementation go directly to 1000 total slots, or land the save-format changes first?
- If DexNav search levels are required, should this feature use a 15-sector normal save layout instead of SaveBlock3 reclaim?
- Should TM/HM 350 be raw item slots, or a virtual ownership registry rendered through the bag?
- Is this fork allowed to break existing save files for local feature branches?
- Should the first user-facing organization change be Items-pocket filters instead of new pockets?
