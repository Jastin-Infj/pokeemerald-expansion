# Nonconsumable Held Items Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `25731e81a0`; implementation branch `feature/held-item-catalog-current-master-20260519` |
| Code status | Catalog assignment implemented on feature branch |
| Provenance | Local source read and feature planning |

## Validation Log

2026-05-19 (`feature/held-item-catalog-current-master-20260519`, baseline
`master` `25731e81a0`):

- `rtk git diff --check`: passed.
- `rtk make -j16 -O check TESTS=test/bag.c`: passed, 10 tests. Existing linker
  warning about a LOAD segment with RWX permissions was observed.
- `rtk make -j16 -O check`: passed. Existing `EXPECTED_FAIL` /
  `KNOWN_FAILING` markers were observed and the suite exited 0.
- `rtk make -j16 -O all`: passed. Existing linker warning about a LOAD segment
  with RWX permissions was observed.
- `rtk make -j16 -O debug`: passed. Existing linker warning about a LOAD
  segment with RWX permissions was observed.
- `rtk mdbook build docs`: passed with existing warnings: missing root
  `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large
  search index.
- mGBA Live MCP boot/input smoke:
  - `mgba_live_start` launched `pokeemerald.gba` with session
    `held-item-catalog-token-smoke-20260519`.
  - `mgba_live_get_view` captured the Pokemon Emerald title screen.
  - `mgba-live-cli input-tap --key START` was accepted.
  - Follow-up `mgba_live_get_view` calls captured the intro path and continue
    menu.
  - `mgba_live_stop` returned `stopped: true`.
  - `mgba-live-cli status --all` returned `[]`.
- Feature-specific quantity behavior is covered by the headless mGBA
  `test/bag.c` route. Manual Bag / Party / Storage UI validation remains useful
  for text polish and visible icon checks.

## Build Validation

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

## Focused Tests: Catalog Assignment

| Test | Expected result |
|---|---|
| Assign one Bag-held Leftovers to two party Pokemon | Both Pokemon can hold Leftovers; Bag quantity does not decrease. Helper coverage exists; mGBA UI route remains. |
| Add duplicate catalog item copies | Bag stores one token only; existing duplicate token stacks normalize back to one when touched by catalog add / give / return helpers. |
| Add ordinary consumables like Potion | Physical quantity behavior remains unchanged because items without a hold effect are outside the catalog token policy. |
| Take catalog-assigned item | Pokemon held item clears; Bag quantity does not increase when the Bag already owns the item. |
| Take first-time held item | Pokemon held item clears and one Bag copy is added if the Bag did not already own the item. |
| Switch catalog-held item to another item | Pokemon held item changes; Bag quantities do not drift. |
| Bag Toss catalog token | Blocked; the catalog token is not physically removed. |
| Party Toss held item | Existing clear-only behavior remains for the Pokemon held slot. |
| Sell / Deposit catalog token | Blocked; the catalog token remains in Bag. |
| Buy already-owned catalog token | Shop displays / treats it as sold out. |
| Mail selected | Routed through normal Mail flow; no catalog clone. Helper coverage confirms Mail remains physical. |
| Storage item mode | Catalog-aware for give / take / close / release return paths; mGBA UI route remains. |
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
- Try Bag Toss, Sell, Deposit, and duplicate shop Buy for the same catalog
  token; confirm all routes preserve the one-token Bag entry.
- Try Storage item mode and confirm the documented MVP behavior.

## Manual Checks To Record

- Whether Bag quantity is intentionally unchanged.
- Whether the held item icon updates correctly in Party and Summary.
- Whether duplicate held items are allowed in normal mode.
- Whether duplicate held items are rejected in item-clause facilities, if tested.
- Whether link / recorded battles were skipped.

## Known Gaps

- Manual Bag / Party / Storage UI validation remains useful for visible text and
  icon polish.
- GitHub Actions should not be waited during agent handoff unless explicitly requested.
- Global vs facility-only catalog mode is currently a branch config decision.
- No final ownership decision for stolen / swapped / bestowed battle items.
