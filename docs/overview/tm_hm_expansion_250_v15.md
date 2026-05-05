# TM/HM Expansion 250 v15

## Purpose

TM を 250 前後まで増やす前提で、現 checkout の local source だけから、先に詰めるべき上限・生成物・field move 分離ポイントを整理する。

調査日: 2026-05-03。source 改造はしておらず、`docs/` への記録のみ。

## Related Docs

| Doc | Focus |
|---|---|
| [pokemon_learnset_flow_v15.md](../flows/pokemon_learnset_flow_v15.md) | learnset / teachable / relearner の既存 flow。 |
| [field_move_hm_flow_v15.md](../flows/field_move_hm_flow_v15.md) | HM field move、badge unlock、party slot 前提。 |
| [move_relearner_flow_v15.md](../flows/move_relearner_flow_v15.md) | `MAX_RELEARNER_MOVES` と TM relearner の詳細。 |
| [pokemon_data_map_v15.md](pokemon_data_map_v15.md) | item ID / held item bitfield / save layout 上限。 |

## Current Facts

| Area | Current value / behavior | Source |
|---|---|---|
| TM moves in use | `FOREACH_TM` は 50 entries。 | `include/constants/tms_hms.h` |
| HM moves in use | `FOREACH_HM` は 8 entries。 | `include/constants/tms_hms.h` |
| Reserved TM item constants | `ITEM_TM01 = 582` through `ITEM_TM100 = 681`。 | `include/constants/items.h` |
| HM item constants | `ITEM_HM01 = 682` through `ITEM_HM08 = 689`。 | `include/constants/items.h` |
| Current last real item | `ITEM_GLIMMORANITE = 873`、次が `ITEMS_COUNT`。 | `include/constants/items.h` |
| Held item storage limit | `ITEMS_COUNT < (1 << 10)` static assert。 | `src/pokemon.c` |
| Bag TM/HM pocket size | `BAG_TMHM_COUNT 64`。 | `include/constants/global.h`, `include/global.h` |
| Relearner move list size | `MAX_RELEARNER_MOVES 60`。 | `include/constants/move_relearner.h` |
| TM/HM numbering | `GetItemTMHMIndex`, `GetTMHMMoveId`, `GetTMHMItemId` are generated from `FOREACH_TMHM` into `gTMHMItemMoveIds`. | `include/item.h`, `src/item.c` |
| Teachability source | `make_teachables.py` reads every `F(...)` in `include/constants/tms_hms.h`. | `tools/learnset_helpers/make_teachables.py` |

## 250 TM Arithmetic

The important number is not just "250 TMs"; it is "how many new item IDs are required."

- Current checkout already reserves `ITEM_TM51` through `ITEM_TM100`, but they are not in `FOREACH_TM`.
- Moving from 50 configured TMs to 250 configured TMs needs 200 more configured TM moves.
- 50 of those can use the existing `ITEM_TM51` through `ITEM_TM100` constants.
- The remaining 150 need new item constants if each TM is a separate bag item.
- Current `ITEMS_COUNT` is effectively 874 (`ITEM_GLIMMORANITE = 873`, then `ITEMS_COUNT`).
- Adding 150 new item IDs without deleting or repurposing anything would make `ITEMS_COUNT` 1024.
- `src/pokemon.c` requires `ITEMS_COUNT < 1024` because held items are stored in a 10-bit bitfield.

Conclusion: **250 distinct TM item IDs are one item ID over the current held-item limit if all existing items are kept and the new IDs are inserted normally.**

## Immediate Design Constraint

For 250 TMs, choose one before implementation:

| Option | Consequence |
|---|---|
| Repurpose at least one existing item ID | Keeps `ITEMS_COUNT < 1024`, avoids save layout change, but needs a deliberate unused-item policy. |
| Repurpose the 8 HM item IDs as TM IDs | Works well if HM items are removed as actual bag items, but every `ITEM_HM_*` script gift / alias needs migration. |
| Keep HM items and add only 249 total TMs | Avoids the 10-bit limit with no item repurpose, but misses the requested 250. |
| Increase held item bitfield | Allows more item IDs, but changes Pokemon save layout and is high risk. |
| Make some TMs virtual rather than items | Avoids item ID pressure, but requires new UI/shop/relearner logic outside the current TM/HM item path. |

The least invasive path for "about 250" is likely: use `ITEM_TM51`-`ITEM_TM100`, repurpose HM item IDs if HMs become key-item/flag unlocks, then add only the remaining new TM item constants needed after that.

## Files That Must Change Later

This is not an implementation plan yet, but these are the local files that a future branch cannot avoid.

| File | Why |
|---|---|
| `include/constants/tms_hms.h` | Add the extra `FOREACH_TM` entries. This drives numbering, item aliases, teachability, and relearner scan count. |
| `include/constants/items.h` | Add or repurpose `ITEM_TM101+` constants. Watch `ITEMS_COUNT < 1024`. |
| `src/data/items.h` | Add item data entries for new TMs. Existing `ITEM_TM51`-`ITEM_TM100` are placeholder `sQuestionMarksDesc`. |
| `include/constants/global.h` | `BAG_TMHM_COUNT 64` is too small for 250 TMs if the player can hold many/all of them. |
| `include/global.h` | `struct Bag::TMsHMs[BAG_TMHM_COUNT]` is saveblock layout. Enlarging it breaks existing saves unless migration exists. |
| `include/constants/move_relearner.h` | `MAX_RELEARNER_MOVES 60` cannot hold a broad TM list. |
| `src/move_relearner.c` | `GetRelearnerTMMoves` writes into `movesToLearn[MAX_RELEARNER_MOVES]` with no local cap in the loop. |
| `tools/learnset_helpers/make_teachables.py` | Automatically reads all `FOREACH_TM/HM` entries; output grows with TM count. |
| `data/maps/**/scripts.inc`, `data/scripts/*.inc` | Any shop/gift/hidden item placement for new TMs lives here. |

## High-Risk Local Findings

### Relearner overflow

`src/move_relearner.c` stores candidates in `movesToLearn[MAX_RELEARNER_MOVES]` and menu rows in `menuItems[MAX_RELEARNER_MOVES + 1]`. `GetRelearnerTMMoves`, `GetRelearnerTutorMoves`, and egg/level-up helpers increment `numMoves` without checking `MAX_RELEARNER_MOVES` before writing.

With 250 TMs, this is safe only if TM relearner is disabled or the candidate set is guaranteed below the cap. If `P_TM_MOVES_RELEARNER`, `P_ENABLE_MOVE_RELEARNERS`, or `P_ENABLE_ALL_TM_MOVES` is enabled later, this needs a guard and a larger UI policy before runtime testing.

### Bag capacity / save layout

`BAG_TMHM_COUNT` is 64. That is enough for the current 50 TM + 8 HM set, but not for 250 owned TMs.

Increasing it changes `struct Bag` inside `SaveBlock1`, so it is not a docs-only risk. It is a save compatibility decision. If TMs are reusable and many are sold in shops, the project needs either a save migration, a virtual TM registry, or a policy that the bag never stores all TMs as item slots.

### Item ID pressure

Current item IDs leave 150 IDs before `ITEMS_COUNT == 1024`, but the strict assert requires less than 1024. Adding exactly 150 new item IDs fails. This is the concrete reason to repurpose at least one existing ID or avoid one physical TM item.

### UI numbering

`src/item_menu.c` already switches TM numbering to 3 digits when `NUM_TECHNICAL_MACHINES >= 100`, so TM100-TM250 numbering is anticipated. The same pocket display still needs screenshot verification because longer move names are squeezed with `PrependFontIdToFit(..., 60)`.

### Item icons / palettes

`src/item_icon.c` derives the TM/HM icon and palette from `GetItemTMHMIndex` and `GetTMHMMoveId`. New TM moves need valid move info and type data; otherwise TM icon palette can fall back through invalid move/type behavior.

## Field HM Decoupling Notes

Badge unlock is already separate from move ownership:

| Field move | Emerald unlock | FRLG unlock |
|---|---|---|
| Cut | `FLAG_BADGE01_GET` | `FLAG_BADGE02_GET` |
| Flash | `FLAG_BADGE02_GET` | `FLAG_BADGE01_GET` |
| Rock Smash | `FLAG_BADGE03_GET` | `FLAG_BADGE06_GET` |
| Strength | `FLAG_BADGE04_GET` | `FLAG_BADGE04_GET` |
| Surf | `FLAG_BADGE05_GET` | `FLAG_BADGE05_GET` |
| Fly | `FLAG_BADGE06_GET` | `FLAG_BADGE03_GET` |
| Dive | `FLAG_BADGE07_GET` | `FLAG_BADGE07_GET` |
| Waterfall | `FLAG_BADGE08_GET` | `FLAG_BADGE07_GET` |

The remaining coupling is move ownership / party slot:

- `ScrCmd_checkfieldmove` returns `PARTY_SIZE` on failure and otherwise returns the party slot of a Pokemon knowing the required move.
- field move scripts use that party slot for `bufferpartymonnick` and `setfieldeffectargument 0`.
- `FldEff_FieldMoveShowMonInit` reads `gPlayerParty[slot]`.
- party menu field moves are added only when the selected Pokemon knows a `FieldMove_GetMoveId(j)` move.
- Surf interaction has an extra C-side `PartyHasMonWithSurf()` gate before `EventScript_UseSurf`.

For key-item/flag unlock, do not only change `IsFieldMoveUnlocked`. That would leave script and animation paths still requiring a Pokemon that knows the move.

## Suggested Investigation Queue

1. Decide whether HM item IDs may be repurposed if field moves become badge/key-item/flag unlocks.
2. Count which existing non-HM item IDs are safe to repurpose if one more slot is needed for exactly 250 TMs.
3. Decide whether TM ownership is bag-slot based, virtual registry based, or shop-only/reusable with limited storage.
4. Before enabling TM relearner, define cap behavior: truncate, paginate, or enlarge `MAX_RELEARNER_MOVES`.
5. For field moves, prototype design on paper around `ScrCmd_checkfieldmove` replacement/extension and a party-slot-free field effect path.

## Open Questions

- Is "250" exact, or is 249 acceptable if all existing items and HMs remain physical items?
- If HMs stop being bag items, should `FOREACH_HM` remain only for move-forget compatibility, or should HM moves become normal moves?
- Should field move unlock state be badge-only, story flag, key item possession, or a small table combining all three?
- Should new TMs be physical item slots, virtual unlocks, or mixed?
- Should TM relearner support all 250 moves, or stay disabled / bag-limited?
