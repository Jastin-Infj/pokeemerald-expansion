# Bag Expansion Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| SaveBlock1 layout shift | High | Existing saves can load with shifted fields or fail compatibility checks. | Decide save-breaking vs migration before source edits; update `test/save.c` intentionally. |
| SaveBlock1 capacity overflow | High | Build fails on `SaveBlock1FreeSpace`, or later feature state has no room. | Calculate slot growth first; use `FREE_*` only with explicit policy. |
| Misusing SaveBlock3 | High | SaveBlock3 has free bytes but does not back normal bag arrays. | Keep normal bag in SaveBlock1 unless a separate virtual storage design is chosen. |
| TM/HM 250 as raw slots | High | `BAG_TMHM_COUNT 250` adds about 744 bytes and exceeds current spare. | Consider virtual TM registry or split TM expansion into its own save design. |
| All current items as raw slots | High | Current catalog pressure is about +2748 bytes, exceeding the SaveBlock1 sector budget by about 2444 bytes. | Do not implement "fit every item" as a constants-only change; design compact ownership or reclaim save space first. |
| Very large TM/HM counts | High | 300 slots exceeds SaveBlock1 spare and `u8` ROM header counts; 2300 slots also exceeds `BagPocket.capacity:10` and consumes almost the whole heap in bag UI buffers. | Use a virtual TM registry / bitset or hard cap raw pocket counts below structural limits. |
| Item ID ceiling | High | `heldItem:10` and `ITEMS_COUNT < 1024` leave only about 149 new item IDs from the current catalog. | Reuse existing TM item IDs, avoid item-per-rule explosions, or plan a Pokemon save-layout migration. |
| ROM header count width | Medium | `rom_header_gf.c` uses `u8` bag count fields. Counts above 255 truncate or need redesign. | Keep MVP counts <= 255. |
| Bag menu memory growth | Medium | `MAX_POCKET_ITEMS`, item name buffers, and sort temp allocations scale with largest pocket. | Check heap use and scrolling after any large pocket target. |
| Debug fill behavior | Medium | Debug fill can add many more items and expose slow paths or full-pocket assumptions. | Add debug fill manual checks to the test plan. |
| Treating sort groups as pockets | Medium | Held items, Mega Stones, Z-Crystals, and battle items are currently `sortType` groups inside `POCKET_ITEMS`, not save-backed pockets. | Prefer Items-pocket filtering unless the feature explicitly accepts new pocket UI and save migration. |
| Downstream docs drift | Medium | Field Kit, TM, and Champions docs may keep old capacity assumptions. | Link this feature from affected docs and update owning docs when source changes. |
| Battle Pyramid confusion | Low | Pyramid bag uses separate `PYRAMID_BAG_ITEMS_COUNT` and Frontier state. | Do not change Pyramid bag as part of normal bag expansion. |

## Impact Notes

- Field Move Modernization currently uses one `ITEM_FIELD_KIT`; it does not require bag expansion today. Per-HM key item designs should depend on this feature.
- TM/HM expansion is the largest known pressure point. A full 250 TM bag-slot model is not a small constants-only change.
- Normal Items are the largest current pocket pressure point: 595 current item definitions are assigned to `POCKET_ITEMS`, while the pocket has 30 slots.
- Held-item / battle-item / Mega Stone style organization can be a UI filter over `sortType`; splitting those into true pockets is a separate save/UI expansion.
- Champions Challenge bag snapshot size will grow whenever `struct Bag` grows, so its SaveBlock1 budget must be recalculated after this feature.
- Runtime rule options and partygen seed should not compete with normal bag storage; those belong to SaveBlock2 / SaveBlock3 policy docs.

## Accepted Risks

- The kickoff docs do not run builds or mGBA checks because no source / data / config changes are made.
- Sizing examples use the current `struct ItemSlot` layout and current `test/save.c` values; implementation must verify with a build.
- Current catalog counts are static source counts from `src/data/items.h`; implementation must recheck after any config or upstream item-table change.

## Open Questions

- What is the highest acceptable Key Items count that still leaves enough SaveBlock1 spare?
- Is save compatibility required for this fork, or can feature branches be clean-save only?
- Should the TM/HM pocket remain an item list if TM ownership becomes large?
- Should held-item / Mega Stone / battle-item organization be a filter, a sort mode, or real pockets?
