# Nonconsumable Held Items MVP Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `25731e81a0`; implementation branch `feature/held-item-catalog-current-master-20260519` |
| Code status | Catalog assignment implemented on feature branch |
| Provenance | Local source read and feature planning |

## Recommended MVP Split

Do not combine all held item policy changes in one runtime branch. The first
slice should be small and battle-focused; the second slice can own the larger
menu / catalog behavior.

| Slice | Purpose | Recommended status |
|---|---|---|
| A. Battle-End Held Item Restore | Items consumed in battle return after battle. Includes berries if enabled. | Staged separately in PR #47. |
| B. Held Item Catalog Assignment | One owned / unlocked held item can be assigned to multiple Pokemon without Bag count friction. | Implemented on `feature/held-item-catalog-current-master-20260519`. |
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

Implemented first contract:

- Exclude Mail.
- Do not alter SaveBlock in the first slice.
- If the Bag contains at least one copy of an allowed held item, the item can be
  assigned to any party Pokemon.
- Assigning the item does not remove it from Bag.
- Taking the item from a Pokemon clears the Pokemon held item. If the Bag
  already has that item, no quantity is added; if the Bag does not have it, one
  copy is added so first-time held item acquisition is preserved.
- Switching held items does not add or remove Bag quantities.
- Toss keeps the existing clear-only behavior.
- PC Storage item mode is updated with the same no-quantity-transfer policy for
  give, take, close-while-holding-item, and release return paths.

### Candidate Runtime Files

| Step | Files | Notes |
|---|---|---|
| 1 | `include/config/item.h` | Added `I_HELD_ITEM_CATALOG_ASSIGNMENT`, default `TRUE` on the feature branch. |
| 2 | `src/item.c`, `include/item.h` | Added helper functions for catalog-aware held item assignment and return. |
| 3 | `src/party_menu.c` | Updated Party / Bag Give, Take, and Switch item paths. |
| 4 | `src/pokemon_storage_system.c` | Updated Storage item mode give / take / close / release return paths. |
| 5 | `src/data/party_menu.h`, `src/strings.c` | Not changed in MVP; existing messages are accepted for now. |
| 6 | `test/bag.c` / mGBA | Focused helper tests added; mGBA UI check remains required. |

### Catalog Decision Table

| Case | MVP policy |
|---|---|
| Non-mail held item assigned from Bag | Set Pokemon held item; do not remove Bag item. |
| Pokemon item taken | Clear Pokemon held item; do not add Bag item if already owned; add one copy if not owned yet. |
| Pokemon item switched | Replace Pokemon held item; do not add/remove Bag items. |
| Toss from Pokemon | Existing clear-only behavior remains. |
| Mail | Excluded. Existing Mail text storage is physical ownership and should not be catalog-cloned. |
| PC Storage item mode | Catalog-aware for item give / take / close / release return paths. |
| Battle Pyramid bag | Excluded; physical quantity behavior remains. |

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

- Should future unlock flags replace existing Bag quantity as the ownership
  source?
- Should a dedicated team builder offer a clearer catalog UI than normal Bag
  Give?
- Should battle-end restore be enabled globally before catalog mode exists?
- Should Storage item mode be disabled only while a challenge is active, or any
  time catalog mode is enabled?
