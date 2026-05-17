# Nonconsumable Held Items Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `ff4e825258`; `git describe` = `expansion/1.15.2-59-gff4e825258` |
| Code status | Docs-only test plan |
| Provenance | Local source read and feature planning |

## Docs-Only Validation

- `rtk mdbook build docs`

## Future Build Validation

For any runtime branch:

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future Focused Tests: Battle-End Restore

| Test | Expected result |
|---|---|
| Player Sitrus / Oran Berry consumed in battle | Berry is gone during battle and restored to the original party slot after battle. |
| Player non-berry single-use item consumed | Item is restored after battle, preserving existing Gen9-style behavior. |
| Config / policy disabled | Existing master behavior remains; berries are not restored if disabled. |
| `Recycle` after berry consumption | Recycle still works during battle and battle end does not corrupt final held item state. |
| `Pickup` / `Harvest` / `Cud Chew` smoke | Battle-time `usedHeldItem` remains usable. |
| `Fling` / `Natural Gift` | Battle-time removal happens normally; battle-end policy is explicit. |
| `Knock Off` / `Thief` / `Covet` | Final item ownership matches the documented policy for wild / trainer battles. |
| Air Balloon / Corrosive Gas | Exceptions are either preserved or intentionally restored with explicit tests. |
| Caught wild Pokemon with held berry | Behavior is documented; restore-or-not is not accidental. |

## Future Focused Tests: Catalog Assignment

| Test | Expected result |
|---|---|
| Assign one Bag-held Leftovers to two party Pokemon | Both Pokemon can hold Leftovers; Bag quantity does not decrease. |
| Take catalog-assigned item | Pokemon held item clears; Bag quantity does not increase. |
| Switch catalog-held item to another item | Pokemon held item changes; Bag quantities do not drift. |
| Toss catalog-held item | Disabled, or clears only the Pokemon held item as documented. |
| Mail selected | Rejected or routed through normal Mail flow; no catalog clone. |
| Storage item mode | Disabled or catalog-aware; no copy creation through PC Storage. |
| Frontier item clause challenge | Duplicate held items are still rejected if that facility rule is active. |
| Normal gameplay with catalog enabled | Duplicate held items are allowed unless a separate item clause is active. |

## Future mGBA Checks

### Battle-End Restore

- Boot normal and debug ROM.
- Give a party Pokemon a berry and trigger consumption in a simple battle.
- Confirm the berry is unavailable during battle after consumption.
- End battle and confirm the same Pokemon has the original berry again.
- Repeat with a non-berry single-use item if a safe test item is available.
- Confirm no lockup on victory, faint, run, or capture paths selected for MVP.

### Catalog Assignment

- Open Bag / Party Give flow.
- Assign the same held item to at least two party Pokemon from one Bag copy.
- Confirm Bag quantity does not decrease.
- Take the item from one Pokemon and confirm Bag quantity does not increase.
- Switch the held item and confirm no Bag quantity drift.
- Try Storage item mode and confirm the documented MVP behavior.

## Manual Checks To Record

- Whether Bag quantity is intentionally unchanged.
- Whether the held item icon updates correctly in Party and Summary.
- Whether duplicate held items are allowed in normal mode.
- Whether duplicate held items are rejected in item-clause facilities, if tested.
- Whether link / recorded battles were skipped.

## Known Gaps For This Docs-Only Branch

- No runtime implementation.
- No source builds required.
- No mGBA validation required.
- No decision yet on global vs facility-only catalog mode.
- No final ownership decision for stolen / swapped / bestowed battle items.
