# TM/HM Expansion 250-300 v15

## Purpose

TM item を 250-300 程度まで増やす場合の上限と、item を増やさず Move
Relearner 用の virtual TM candidate pool として 250-300 技を扱う場合の分岐を整理する。

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

## 250-300 Physical TM Arithmetic

If every TM is a physical bag item, the important number is not just "250 TMs";
it is "how many new item IDs are required."

- Current checkout already reserves `ITEM_TM51` through `ITEM_TM100`, but they are not in `FOREACH_TM`.
- Moving from 50 configured TMs to 250 configured TMs needs 200 more configured TM moves.
- 50 of those can use the existing `ITEM_TM51` through `ITEM_TM100` constants.
- The remaining 150 need new item constants if each TM is a separate bag item.
- Current `ITEMS_COUNT` is effectively 874 (`ITEM_GLIMMORANITE = 873`, then `ITEMS_COUNT`).
- Adding 150 new item IDs without deleting or repurposing anything would make `ITEMS_COUNT` 1024.
- `src/pokemon.c` requires `ITEMS_COUNT < 1024` because held items are stored in a 10-bit bitfield.
- At 300 configured TMs, the project would need 250 additional configured TM
  moves beyond the current 50. Even after using `ITEM_TM51` through
  `ITEM_TM100`, 200 more physical item IDs would be needed if every TM is a
  separate bag item.

Conclusion for physical items: **250 distinct TM item IDs are already one item ID over the current held-item limit if all existing items are kept and the new IDs are inserted normally. 300 distinct TM item IDs cannot fit the current physical item model without broader item/save changes or a virtual/mixed registry.**

## Virtual Relearner Pool Direction

The current preferred Move Relearner direction is different from physical item
expansion:

- Do not add 250-300 TM items.
- Keep the existing 50 physical TM item slots available for legacy / optional
  story use, or disable their gameplay use entirely.
- Generate a relearner-only virtual TM candidate pool from historical TM/TR data
  and compatibility rules.
- If all virtual candidates are always available, no save data is required.
- If story progression should unlock virtual TM candidates, a compact bitset for
  250-300 moves is far smaller than expanding the bag pocket, but it is still a
  save layout decision.
- Unlocks can be modeled as rank / clear-flag / group metadata, so story
  progress can unlock "this TM move" without granting an item.
- Story rewards can therefore grant access to a subset of TM-family moves in the
  Move Relearner, not a physical TM item.
- The main blocker becomes candidate count / UI: Mew must fit the chosen
  relearner storage and display policy.
- Historical TM candidates can overlap tutor / Battle Tower candidates. The
  current Move Relearner requirement is to preserve those overlaps as separate
  source entries, so the player can see that the same move is available through
  both TM and tower/tutor routes.

## Immediate Design Constraint

For 250-300 physical TMs, choose one before implementation:

| Option | Consequence |
|---|---|
| Repurpose at least one existing item ID | Keeps `ITEMS_COUNT < 1024`, avoids save layout change, but needs a deliberate unused-item policy. |
| Repurpose the 8 HM item IDs as TM IDs | Works well if HM items are removed as actual bag items, but every `ITEM_HM_*` script gift / alias needs migration. |
| Keep HM items and add only 249 total TMs | Avoids the 10-bit limit with no item repurpose, but misses the requested 250-300 range. |
| Increase held item bitfield | Allows more item IDs, but changes Pokemon save layout and is high risk. |
| Make some TMs virtual rather than items | Avoids item ID pressure, but requires new UI/shop/relearner logic outside the current TM/HM item path. |

For the Move Relearner feature, the least risky direction is a virtual registry:
keep a limited physical item set where needed, and expose broader TM
compatibility / relearner candidates through generated data instead of one bag
item per TM.

## Files That Must Change Later

This is not an implementation plan yet, but these are the local files that a future branch cannot avoid.

| File | Why |
|---|---|
| `include/constants/tms_hms.h` | Add the extra `FOREACH_TM` entries. This drives numbering, item aliases, teachability, and relearner scan count. |
| `include/constants/items.h` | Add or repurpose `ITEM_TM101+` constants. Watch `ITEMS_COUNT < 1024`. |
| `src/data/items.h` | Add item data entries for new TMs. Existing `ITEM_TM51`-`ITEM_TM100` are placeholder `sQuestionMarksDesc`. |
| `include/constants/global.h` | `BAG_TMHM_COUNT 64` is too small for 250-300 TMs if the player can hold many/all of them. |
| `include/global.h` | `struct Bag::TMsHMs[BAG_TMHM_COUNT]` is saveblock layout. Enlarging it breaks existing saves unless migration exists. |
| `include/constants/move_relearner.h` | `MAX_RELEARNER_MOVES 60` cannot hold a broad 250-300 TM list. |
| `src/move_relearner.c` | `GetRelearnerTMMoves` writes into `movesToLearn[MAX_RELEARNER_MOVES]` with no local cap in the loop. |
| `tools/learnset_helpers/make_teachables.py` | Automatically reads all `FOREACH_TM/HM` entries; output grows with TM count. |
| `data/maps/**/scripts.inc`, `data/scripts/*.inc` | Any shop/gift/hidden item placement for new TMs lives here. |
| `src/move_relearner.c` / generated virtual pool | Required if broad TM candidates are relearner-only and not physical items. |

## High-Risk Local Findings

### Relearner overflow

`src/move_relearner.c` stores candidates in `movesToLearn[MAX_RELEARNER_MOVES]` and menu rows in `menuItems[MAX_RELEARNER_MOVES + 1]`. `GetRelearnerTMMoves`, `GetRelearnerTutorMoves`, and egg/level-up helpers increment `numMoves` without checking `MAX_RELEARNER_MOVES` before writing.

With 250-300 virtual TM candidates, this is safe only if the candidate set is
guaranteed below the cap or the UI is paged/tabbed. If `P_TM_MOVES_RELEARNER`,
`P_ENABLE_MOVE_RELEARNERS`, or a new virtual TM pool is enabled later, this
needs a guard and a larger UI policy before runtime testing. Raising
`MAX_RELEARNER_MOVES` alone is a storage fix, not a UX fix.

### Bag capacity / save layout

`BAG_TMHM_COUNT` is 64. That is enough for the current 50 TM + 8 HM set, but not for 250-300 owned TMs.

Increasing it changes `struct Bag` inside `SaveBlock1`, so it is not a docs-only risk. It is a save compatibility decision. If TMs are reusable and many are sold in shops, the project needs either a save migration, a virtual TM registry, or a policy that the bag never stores all TMs as item slots.

If virtual TM relearner candidates are always available, this bag capacity issue
does not apply. If virtual TM candidates are unlockable, persistent state should
be modeled as compact unlock data rather than extra bag slots.

### Item ID pressure

Current item IDs leave 150 IDs before `ITEMS_COUNT == 1024`, but the strict assert requires less than 1024. Adding exactly 150 new item IDs fails. This is the concrete reason to repurpose at least one existing ID or avoid one physical TM item.

### UI numbering

`src/item_menu.c` already switches TM numbering to 3 digits when `NUM_TECHNICAL_MACHINES >= 100`, so TM100-TM300 numbering is anticipated. The same pocket display still needs screenshot verification because longer move names are squeezed with `PrependFontIdToFit(..., 60)`.

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
2. Count which existing non-HM item IDs are safe to repurpose if physical IDs are still required for the 250-300 TM range.
3. Decide whether relearner TM availability is always-on virtual pool, story unlock bitset, current physical registry, or mixed.
4. Before enabling broad TM relearner, define cap behavior and UX: truncate,
   paginate, source-tab, 50-60 move chunks, or enlarge `MAX_RELEARNER_MOVES`.
5. For field moves, prototype design on paper around `ScrCmd_checkfieldmove` replacement/extension and a party-slot-free field effect path.

## Open Questions

- Is the upper target closer to 250 or 300, and which subset, if any, must be physical bag items?
- If HMs stop being bag items, should `FOREACH_HM` remain only for move-forget compatibility, or should HM moves become normal moves?
- Should field move unlock state be badge-only, story flag, key item possession, or a small table combining all three?
- Should new TMs be physical item slots, virtual unlocks, or mixed?
- Should TM relearner support all 250-300 virtual candidates as always available, or should some candidates require story unlock state?
- If story unlocks exist, should they be tracked per move, per rank group, or
  per generated TM group?
- Should Mew's broad TM pool be grouped into 50-entry chunks, 60-entry chunks,
  source tabs, or a combined tab+chunk flow?
