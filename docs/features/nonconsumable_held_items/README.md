# Nonconsumable Held Items

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `25731e81a0`; implementation branch `feature/held-item-catalog-current-master-20260519` |
| Code status | Catalog assignment implemented on feature branch; not present in `master` source |
| Provenance | Local source read and feature planning |

## Status

Status: Integration candidate

This feature now has runtime PR #48 for the catalog / unique-token held item
assignment slice. Runtime source is still not changed on `master`.

## Goal

Make held items easier to use in competitive / Champions-style play:

- Battle-consumed held items should not be permanently lost after battle.
- A player should not need multiple physical copies just to assign the same
  held item to multiple Pokemon in a team-building flow.
- Duplicate held item rules should be explicit feature / facility policy, not an
  accidental consequence of Bag quantity.

## Current Decision

This is possible, but it is not one switch. The existing code separates three
different policies:

| Policy axis | Current code reality | Feature direction |
|---|---|---|
| Battle-time consumption | Items are still removed during battle so `Recycle`, `Pickup`, `Harvest`, `Cud Chew`, `Unburden`, and related mechanics work. | Do not make items unconsumable during battle. Restore at battle end instead. |
| Battle-end restore | `B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9` restores non-berry held items, but berries are excluded in `TryRestoreHeldItems()`. | First runtime slice should restore original player held items after battle, berries included, using the existing battle-start `originalItem` snapshot. |
| Bag / party ownership | Party and Bag UI currently move one physical item: Bag count decreases on Give and increases on Take. | A Champions-style catalog / infinite assignment mode needs separate UI and ownership rules. Runtime PR #48 now treats held-effect catalog items as one Bag token, blocks Bag Toss / Sell / Deposit, and leaves ordinary no-hold-effect items physical. |
| Duplicate held items | Frontier / selection / party pool code can enforce item clause, while normal gameplay does not globally ban duplicates. | Duplicate allowance and item clause should be mode-specific. The requested Champions-like policy should allow duplicate assignment unless a challenge explicitly enables item clause. |

Implementation order:

1. Battle-end held item restore is staged separately in Battle Item Restore
   Policy PR #47.
2. Held Item Catalog assignment mode is implemented on
   `feature/held-item-catalog-current-master-20260519`.
3. Keep item clause as a challenge rule, not a default ownership restriction.

## Scope

### In Scope

- Document the existing held item consumption and restore flow.
- Document party / bag / storage item ownership paths.
- Separate battle-end restoration from inventory quantity / duplicate item
  assignment.
- Implement and document a Champions-style held item catalog MVP.
- Cross-link the existing Battle Item Restore Policy as the first likely runtime
  slice.

### Out of Scope

- SaveBlock changes.
- New held item IDs or held item effects.
- Battle hook rewrites.
- Link battle / recorded battle policy.
- Trainer Card, PokéNav, or full Champions facility UI.
- Physical Bag expansion.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
- [Battle Item Restore Policy](../battle_item_restore_policy/README.md)
- [Roguelike Party Policy Impact](../../overview/roguelike_party_policy_impact_v15.md)
- [Scout Selection and Battlefield Status](../../overview/scout_selection_and_battlefield_status_v15.md)

## Open Questions

- Should battle-end restore be global for the player party, or active only in a
  Champions / competitive ruleset?
- Should berries restore by default when this feature is adopted, or only under a
  new config / runtime rule?
- Should the catalog be available in normal gameplay, debug only, a training
  facility, or Champions Challenge only?
- In catalog mode, should `TAKE` return an item to Bag, or only clear the held
  item from the Pokemon?
- Should Mail be excluded entirely from catalog mode? Current code treats Mail as
  a special owned object with saved text.
- What is the final ownership policy for `Thief`, `Covet`, `Trick`,
  `Switcheroo`, `Bestow`, `Symbiosis`, `Pickpocket`, and `Magician`?
