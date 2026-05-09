# Bag Expansion Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| SaveBlock1 layout shift | High | Existing saves can load with shifted fields or fail compatibility checks. | Decide save-breaking vs migration before source edits; update `test/save.c` intentionally. |
| SaveBlock1 capacity overflow | High | Build fails on `SaveBlock1FreeSpace`, or later feature state has no room. | Calculate slot growth first; use `FREE_*` only with explicit policy. |
| Misusing SaveBlock3 | High | SaveBlock3 has free bytes but does not back normal bag arrays. | Keep normal bag in SaveBlock1 unless a separate virtual storage design is chosen. |
| TM/HM 250 as raw slots | High | `BAG_TMHM_COUNT 250` adds about 744 bytes and exceeds current spare. | Consider virtual TM registry or split TM expansion into its own save design. |
| ROM header count width | Medium | `rom_header_gf.c` uses `u8` bag count fields. Counts above 255 truncate or need redesign. | Keep MVP counts <= 255. |
| Bag menu memory growth | Medium | `MAX_POCKET_ITEMS`, item name buffers, and sort temp allocations scale with largest pocket. | Check heap use and scrolling after any large pocket target. |
| Debug fill behavior | Medium | Debug fill can add many more items and expose slow paths or full-pocket assumptions. | Add debug fill manual checks to the test plan. |
| Downstream docs drift | Medium | Field Kit, TM, and Champions docs may keep old capacity assumptions. | Link this feature from affected docs and update owning docs when source changes. |
| Battle Pyramid confusion | Low | Pyramid bag uses separate `PYRAMID_BAG_ITEMS_COUNT` and Frontier state. | Do not change Pyramid bag as part of normal bag expansion. |

## Impact Notes

- Field Move Modernization currently uses one `ITEM_FIELD_KIT`; it does not require bag expansion today. Per-HM key item designs should depend on this feature.
- TM/HM expansion is the largest known pressure point. A full 250 TM bag-slot model is not a small constants-only change.
- Champions Challenge bag snapshot size will grow whenever `struct Bag` grows, so its SaveBlock1 budget must be recalculated after this feature.
- Runtime rule options and partygen seed should not compete with normal bag storage; those belong to SaveBlock2 / SaveBlock3 policy docs.

## Accepted Risks

- The kickoff docs do not run builds or mGBA checks because no source / data / config changes are made.
- Sizing examples use the current `struct ItemSlot` layout and current `test/save.c` values; implementation must verify with a build.

## Open Questions

- What is the highest acceptable Key Items count that still leaves enough SaveBlock1 spare?
- Is save compatibility required for this fork, or can feature branches be clean-save only?
- Should the TM/HM pocket remain an item list if TM ownership becomes large?
