# Bag Expansion MVP Plan

## MVP

Do not start by changing every pocket. The first implementation slice should choose one
capacity target and prove the save/UI path.

Recommended first slice:

1. Expand only a small, selected pocket target first, such as Key Items 30 -> 64, if the project wants room for more progression key items.
2. Do not try to make every current item definition fit in the bag in the MVP; that requires roughly +2748 bytes and exceeds SaveBlock1.
3. Keep TM/HM expansion as a separate decision unless the user explicitly chooses a raw TM/HM bag-slot model and accepts the save/UI redesign.
4. Prefer `POCKET_ITEMS` filters for held items / battle items / Mega Stones before adding true pockets.
5. Treat the change as save-breaking unless a migration path is designed before code changes.

## Non-Goals

- Do not move existing item IDs as part of pocket capacity work.
- Do not implement per-HM key items in this feature.
- Do not use SaveBlock3 as storage for normal bag item slots.
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
| 5 | `src/item_menu.c` | Confirm `MAX_POCKET_ITEMS`, list buffers, sort allocation, and Wally tutorial snapshots still fit and behave. |
| 6 | `src/rom_header_gf.c` | Confirm bag count fields remain valid `u8` values. Do not exceed 255 without redesign. |
| 7 | `test/save.c` | Update expected SaveBlock sizes only if the save-breaking change is accepted. If migration is required, add migration tests first. |
| 8 | `docs/upgrades/upstream_diff_checklist.md` | Add bag constants / `struct Bag` / save layout to the upstream recheck list. |

## Current Contract

- Existing `master` behavior remains unchanged.
- Normal bag slot arrays live in `struct SaveBlock1`.
- Every added normal bag slot costs approximately 4 bytes of SaveBlock1.
- `SaveBlock1` has 304 bytes of current spare capacity by `test/save.c`.
- Counts at or below 255 fit the current `rom_header_gf.c` count field type. Counts above 255 are out of contract.
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

- 250 TM ownership model: large TM/HM pocket vs virtual registry vs shop/relearner policy.
- Items-pocket filter UI for held items, battle items, Mega Stones, Z-Crystals, and other `sortType` groups.
- Save migration design for old `.sav` files.
- Optional `FREE_*` save capacity policy if the desired target exceeds SaveBlock1 spare.
- Champions Challenge normal bag snapshot sizing after the final `struct Bag` size is known.

## Open Questions

- Should Key Items 64 be the first target, or is a smaller target enough?
- Should TM/HM 250 be blocked until virtual TM ownership is designed?
- Is this fork allowed to break existing save files for local feature branches?
- Should the first user-facing organization change be Items-pocket filters instead of new pockets?
