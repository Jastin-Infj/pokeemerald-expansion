# Nonconsumable Held Items MVP Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `ff4e825258`; `git describe` = `expansion/1.15.2-59-gff4e825258` |
| Code status | Docs-only plan |
| Provenance | Local source read and feature planning |

## Recommended MVP Split

Do not combine all held item policy changes in one runtime branch. The first
slice should be small and battle-focused; the second slice can own the larger
menu / catalog behavior.

| Slice | Purpose | Recommended status |
|---|---|---|
| A. Battle-End Held Item Restore | Items consumed in battle return after battle. Includes berries if enabled. | First runtime candidate. |
| B. Held Item Catalog Assignment | One owned / unlocked held item can be assigned to multiple Pokemon without Bag count friction. | Future runtime candidate after Slice A. |
| C. Item Clause Mode | Optional rule that forbids duplicate held items for a specific facility / challenge. | Keep separate; requested Champions-style default should not assume this is on. |

## Slice A: Battle-End Held Item Restore MVP

### MVP

- Use the existing battle-start `itemLost[B_SIDE_PLAYER][slot].originalItem`
  snapshot as source of truth.
- Keep battle-time item removal unchanged.
- Keep `usedHeldItem`, `canPickupItem`, `ateBerry`, `isKnockedOff`, and Unburden
  behavior unchanged during battle.
- At battle end, restore the original player party held item according to the
  selected policy.
- Include Berry pocket items if the local config / runtime policy enables the
  Champions-style restore.
- Do not alter Bag quantities.
- Do not alter Party / Bag / Storage UI.

### Non-Goals

- Do not make held items unconsumable during battle.
- Do not bypass `usedHeldItem`.
- Do not add SaveBlock state.
- Do not change item IDs, item pockets, or held item definitions.
- Do not implement catalog assignment in this slice.
- Do not change Frontier / Tent duplicate-item generation rules.

### Candidate Runtime Files

| Step | Files | Notes |
|---|---|---|
| 1 | `include/config/battle.h` | Decide whether to reuse branch-only `B_RESTORE_HELD_BATTLE_BERRIES` or introduce a clearer local policy name. |
| 2 | `src/battle_util.c` | Adjust `TryRestoreHeldItems()` through a helper so berries can be included without duplicating restore paths. |
| 3 | `src/battle_main.c` | Ensure the existing battle-end call covers the selected policy. |
| 4 | `test/battle_item_restore.c` or focused battle tests | Cover berries, non-berries, config off/on, and edge mechanics. |
| 5 | `docs/features/nonconsumable_held_items/test_plan.md` and `docs/features/battle_item_restore_policy/` | Record final policy and validation evidence. |

## Slice B: Held Item Catalog Assignment MVP

### MVP

Treat Bag ownership as permission to assign a held item, not as the exact number
of Pokemon that can hold it.

Recommended first contract:

- Exclude Mail.
- Do not alter SaveBlock in the first slice.
- If the Bag contains at least one copy of an allowed held item, the item can be
  assigned to any party Pokemon.
- Assigning the item does not remove it from Bag.
- Taking the item from a Pokemon only clears the Pokemon held item; it does not
  add another copy to Bag.
- Switching held items does not add or remove Bag quantities.
- Toss is disabled or treated as "clear held item" in catalog contexts.
- PC Storage item mode is either disabled in catalog mode or explicitly updated
  with the same no-quantity-transfer policy.

### Candidate Runtime Files

| Step | Files | Notes |
|---|---|---|
| 1 | `include/config/item.h` or local battle / challenge config | Add a future catalog enable switch only on a runtime branch. |
| 2 | `src/party_menu.c` | Own `CB2_GiveHoldItem()`, `Task_GiveHoldItem()`, `TryGiveItemOrMailToSelectedMon()`, switch paths, `TryTakeMonItem()`, and Toss behavior. |
| 3 | `src/item_menu.c` | Confirm Bag `GIVE` can enter the same catalog-aware party flow. |
| 4 | `src/pokemon_storage_system.c` | Disable or adapt item mode so Storage cannot duplicate / erase catalog-held items. |
| 5 | `src/data/party_menu.h`, `src/strings.c` | Add clear messages if needed. |
| 6 | Tests / mGBA | Verify no Bag quantity drift and no duplicate copy creation through Take / Switch / Storage. |

### Catalog Decision Table

| Case | MVP policy |
|---|---|
| Non-mail held item assigned from Bag | Set Pokemon held item; do not remove Bag item. |
| Pokemon item taken | Clear Pokemon held item; do not add Bag item. |
| Pokemon item switched | Replace Pokemon held item; do not add/remove Bag items. |
| Toss from Pokemon | Prefer disabled in MVP; if allowed, only clear held item. |
| Mail | Excluded. Existing Mail text storage is physical ownership and should not be catalog-cloned. |
| PC Storage item mode | Disable during MVP unless fully audited. |
| Battle Pyramid bag | Exclude until facility-specific rules are reviewed. |

## Slice C: Item Clause Mode

Item clause is separate from quantity. A Bag can allow infinite assignment while a
facility still rejects duplicate held items.

Existing useful checks:

- `src/party_menu.c` `CheckBattleEntriesAndGetMessage()` returns
  `PARTY_MSG_NO_SAME_HOLD_ITEMS` for duplicate selected held items.
- `src/frontier_util.c` `CheckPartyIneligibility()` rejects duplicate held items
  for Frontier-style eligibility.
- `src/trainer_pools.c` prunes duplicate held items when `rules->itemClause` is
  enabled.

For the requested Champions-like policy, default duplicate held items should be
allowed unless a challenge explicitly enables item clause.

## Future Work

- UI list that shows "owned / unlocked held items" without requiring Bag copies.
- Facility-specific catalog access from Training Dojo / Champions Challenge.
- Unlock state for held items, if story progression should gate stronger items.
- Runtime option that toggles global battle item restore.
- Explicit policy for stolen / swapped / bestowed items in wild, trainer,
  facility, and link contexts.
- A dedicated "Held Item Journal" or item assignment terminal.

## Open Questions

- Should catalog ownership use existing Bag quantity, a fixed allowed item list,
  or future unlock flags?
- Should catalog mode live in normal Bag UI, or only a dedicated team builder?
- Should battle-end restore be enabled globally before catalog mode exists?
- Should Storage item mode be disabled only while a challenge is active, or any
  time catalog mode is enabled?
