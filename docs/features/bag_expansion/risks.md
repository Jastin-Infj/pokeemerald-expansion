# Bag Expansion Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| SaveBlock1 layout shift | High | Existing saves can load with shifted fields or fail compatibility checks. | Decide save-breaking vs migration before source edits; update `test/save.c` intentionally. |
| SaveBlock1 capacity overflow | High | Build fails on `SaveBlock1FreeSpace`, or later feature state has no room. | Calculate slot growth first; use `FREE_*` only with explicit policy. |
| Misusing SaveBlock3 | High | SaveBlock3 has free bytes but does not back normal bag arrays. | Keep normal bag in SaveBlock1 unless a separate virtual storage design is chosen. |
| TM/HM 250+ as raw slots | High | Even `BAG_TMHM_COUNT 250` adds about 744 bytes and exceeds current spare; the current target is closer to 350. | Consider virtual TM registry or split TM expansion into its own save design. |
| All current items as raw slots | High | Current catalog pressure is about +2748 bytes, exceeding the SaveBlock1 sector budget by about 2444 bytes. | Do not implement "fit every item" as a constants-only change; design compact ownership or reclaim save space first. |
| 1000 total raw slots | High | 1000 total slots need about +3256 bytes. Current spare + all SaveBlock1 `FREE_*` reaches only about 891 total slots. | Either reclaim SaveBlock3 chunk bytes / find another 436+ bytes, or lower the raw-slot target. |
| TM/HM 350 as raw slots | High | 350 is within `BagPocket.capacity:10`, but exceeds current SaveBlock1 spare, `u8` ROM header counts, and bag UI `u8` count caches. | Widen UI counts, redesign/clamp ROM header counts, and pair with a save-space plan. |
| Very large TM/HM counts | High | 2300 slots exceeds `BagPocket.capacity:10` and consumes almost the whole heap in bag UI buffers. | Use a virtual TM registry / bitset or hard cap raw pocket counts below structural limits. |
| DexNav vs SaveBlock3 reclaim | High | DexNav search levels need `NUM_SPECIES` bytes; current `NUM_SPECIES` is 1573, nearly all of SaveBlock3. | If DexNav search levels are required, keep SaveBlock3 and use another save-capacity path. |
| Save slot sector expansion | High | Growing the normal save slot from 14 to 15 sectors can solve 1000 raw slots, but consumes Hall of Fame sectors and changes save rotation layout. | Treat as a major save-format migration with explicit HOF/special-sector policy. |
| Nonstandard emulator-only flash size | High | The code currently targets `FLASH1M_V103` / 128 KiB / 32 sectors; larger mGBA-only saves need save driver, layout, and emulator configuration changes. | Prefer staying inside 128 KiB and repurposing special sectors before attempting a custom flash target. |
| Item ID ceiling | High | `heldItem:10` and `ITEMS_COUNT < 1024` leave only about 149 new item IDs from the current catalog. | Reuse existing TM item IDs, avoid item-per-rule explosions, or plan a Pokemon save-layout migration. |
| ROM header count width | Medium | `rom_header_gf.c` uses `u8` bag count fields. Counts above 255 truncate or need redesign. | Keep MVP counts <= 255. |
| Bag menu memory growth | Medium | `MAX_POCKET_ITEMS`, item name buffers, and sort temp allocations scale with largest pocket. | Check heap use and scrolling after any large pocket target. |
| Bag UI count width | Medium | `BagMenu.numItemStacks`, `numShownItems`, and `SetItemListPerPageCount()` use `u8` counts. | Widen count types before any pocket can exceed 255 entries including Cancel. |
| Debug fill behavior | Medium | Debug fill can add many more items and expose slow paths or full-pocket assumptions. | Add debug fill manual checks to the test plan. |
| SaveBlock3 chunk reclaim | Medium | Reclaiming the 116-byte per-sector SaveBlock3 chunk changes the save sector format and current SaveBlock3 handling. | Treat as a save-format migration; relocate or remove current SaveBlock3 data before changing `SAVE_BLOCK_3_CHUNK_SIZE`. |
| Special-sector custom storage | Medium | Trainer Hill / Recorded Battle / Hall of Fame sectors are outside the normal double-slot save path. | Only use with custom checksum/mirroring/load policy and after disabling the feature that owns the sector. |
| Treating sort groups as pockets | Medium | Held items, Mega Stones, Z-Crystals, and battle items are currently `sortType` groups inside `POCKET_ITEMS`, not save-backed pockets. | Prefer Items-pocket filtering unless the feature explicitly accepts new pocket UI and save migration. |
| Downstream docs drift | Medium | Field Kit, TM, and Champions docs may keep old capacity assumptions. | Link this feature from affected docs and update owning docs when source changes. |
| Battle Pyramid confusion | Low | Pyramid bag uses separate `PYRAMID_BAG_ITEMS_COUNT` and Frontier state. | Do not change Pyramid bag as part of normal bag expansion. |

## Impact Notes

- Field Move Modernization currently uses one `ITEM_FIELD_KIT`; it does not require bag expansion today. Per-HM key item designs should depend on this feature.
- TM/HM expansion is the largest known pressure point. A 350-slot TM/HM raw-pocket model is not a small constants-only change.
- Normal Items are the largest current pocket pressure point: 595 current item definitions are assigned to `POCKET_ITEMS`, while the pocket has 30 slots.
- Held-item / battle-item / Mega Stone style organization can be a UI filter over `sortType`; splitting those into true pockets is a separate save/UI expansion.
- Champions Challenge bag snapshot size will grow whenever `struct Bag` grows, so its SaveBlock1 budget must be recalculated after this feature.
- Runtime rule options and partygen seed should not compete with normal bag storage; those belong to SaveBlock2 / SaveBlock3 policy docs.
- `u8` -> `u16` helps list counts and ROM header representation, but it does not create more save sectors. Save capacity requires `FREE_*`, compact storage, sector reallocation, or a custom external storage path.
- mGBA-only removes real-cartridge compatibility as a product constraint, but it does not remove the in-ROM save driver and sector layout constraints. The first practical mGBA-only expansion path is still a save-layout change inside the 128 KiB flash image, usually by consuming Hall of Fame / special sectors.

## Accepted Risks

- The kickoff docs do not run builds or mGBA checks because no source / data / config changes are made.
- Sizing examples use the current `struct ItemSlot` layout and current `test/save.c` values; implementation must verify with a build.
- Current item catalog count comes from `include/constants/items.h` / `ITEMS_COUNT`; pocket pressure still comes from `src/data/items.h` `.pocket` assignments.

## Open Questions

- What is the highest acceptable Key Items count that still leaves enough SaveBlock1 spare?
- Is save compatibility required for this fork, or can feature branches be clean-save only?
- Should the TM/HM pocket remain an item list at 350 entries, or should ownership become virtual storage?
- Should the 1000-slot target use SaveBlock3 chunk reclaim, or should the target be capped around 891 slots with only SaveBlock1 `FREE_*`?
- If DexNav search levels are required, should bag expansion consume Hall of Fame sectors through a 15-sector normal save layout?
- For an mGBA-only build, is a nonstandard flash driver worth the extra risk, or should the feature stay on the 128 KiB `FLASH1M` path and repurpose existing sectors?
- Should held-item / Mega Stone / battle-item organization be a filter, a sort mode, or real pockets?
