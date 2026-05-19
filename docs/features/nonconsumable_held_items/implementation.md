# Nonconsumable Held Items Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `25731e81a0`; implementation branch `feature/held-item-catalog-current-master-20260519` |
| Code status | Implemented and locally validated on feature branch; not present in `master` source |
| Provenance | Runtime feature handoff |

## Status

Status: Implemented on `feature/held-item-catalog-current-master-20260519`;
not present in `master` source. Current runtime PR: #48.

This branch implements the catalog / unique ownership token assignment slice.
It does not merge runtime source into `master`, and it does not include the
battle-end berry restore source from PR #47. Full competitive-style held item
behavior should adopt this branch together with the Battle Item Restore Policy
branch or a future integration branch.

## Implemented Behavior

`I_HELD_ITEM_CATALOG_ASSIGNMENT` is enabled in `include/config/item.h`.

When the catalog policy applies:

- A non-mail, non-Key Item with an actual hold effect
  (`GetItemHoldEffect(item) != HOLD_EFFECT_NONE`) acts as a unique Bag
  ownership token.
- Normal consumables and utility items with no hold effect remain physical Bag
  quantities.
- `AddBagItem` stores only one catalog token. Existing duplicate catalog stacks
  are normalized to one token when touched by catalog add / give / return paths.
- Giving a held item from Bag / Party / PC Storage does not decrement Bag
  quantity.
- Taking or switching a held item does not create another Bag copy if the Bag
  already has that item.
- Taking a held item that is not yet in the Bag still adds one copy, preserving
  first-time held item acquisition from caught Pokemon, gifts, or other
  non-catalog sources.
- Bag Toss, shop Sell, and PC Deposit are blocked for catalog tokens.
- Shop Buy treats already-owned catalog tokens as sold out, matching the
  one-token purchase model.
- Mail remains physical and continues through the existing Mail flow.
- Battle Pyramid bag paths remain physical and are excluded from catalog mode.
- Item clause rules are not changed. Facilities that reject duplicate held
  items still own those checks separately from Bag quantity.

## Files Changed

| File | Change |
|---|---|
| `include/config/item.h` | Added `I_HELD_ITEM_CATALOG_ASSIGNMENT`, default `TRUE` on this feature branch. |
| `include/item.h`, `src/item.c` | Added catalog-aware helper functions for held item assignment / return, plus unique-token add and duplicate normalization. |
| `src/item_menu.c` | Blocks Bag Toss, shop Sell, and PC Deposit for catalog tokens. |
| `src/shop.c` | Treats already-owned catalog tokens as sold out and buys unowned tokens as a single item. |
| `src/party_menu.c` | Routed Party / Bag Give, Take, and Switch item paths through catalog-aware helpers. |
| `src/pokemon_storage_system.c` | Routed PC Storage item give / take / close / release paths through catalog-aware helpers. |
| `test/bag.c` | Added focused quantity drift tests for catalog give / take / first-copy preservation / Mail exclusion / duplicate normalization / ordinary consumable exclusion. |

## Validation

Confirmed on 2026-05-19:

```sh
rtk git diff --check
rtk make -j16 -O check TESTS=test/bag.c
rtk make -j16 -O check
rtk make -j16 -O all
rtk make -j16 -O debug
rtk mdbook build docs
```

Results:

- `rtk git diff --check`: passed.
- `rtk make -j16 -O check TESTS=test/bag.c`: passed, 10 tests.
- `rtk make -j16 -O check`: passed; existing `EXPECTED_FAIL` /
  `KNOWN_FAILING` markers remain expected and the suite exits 0.
- `rtk make -j16 -O all`: passed with the existing RWX linker warning.
- `rtk make -j16 -O debug`: passed with the existing RWX linker warning.
- `rtk mdbook build docs`: passed with existing warnings: missing root
  `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large
  search index.
- mGBA Live MCP boot/input smoke used session
  `held-item-catalog-token-smoke-20260519`, reached the title screen, accepted
  `START`, reached the intro / continue path, stopped cleanly, and
  `mgba-live-cli status --all` returned `[]`.

Feature-specific quantity behavior is covered by the headless mGBA
`test/bag.c` route. A manual Bag / Party / Storage UI route remains useful for
visual text polish, but was not required to validate the underlying item
quantity policy.

## Merge Handoff

This branch contains runtime source changes. Do not merge it into `master` under
the docs-only master policy. Use a runtime PR as a staging shelf, then create a
docs-only handoff separately if the feature is user-confirmed.

Current runtime PR: #48
`feature/held-item-catalog-current-master-20260519` -> `master`.

Before any docs-only master merge, confirm:

```sh
rtk git diff --name-only master..HEAD
```

Only Markdown docs, `AGENTS.md`, or approved Lua scripts may be present for a
docs-only master lane.

## Remaining Risks

- UI text still uses the existing Give / Take / Bag wording. Behavior is
  correct, but a future polish pass can add catalog-specific messages.
- Battle-time item consumption is still separate. Use PR #47 / Battle Item
  Restore Policy for consumed berries and single-use held item restoration.
- Link / recorded battle ownership and stolen / swapped item ownership are not
  changed in this slice.
