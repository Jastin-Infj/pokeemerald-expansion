# Friendly Shop Pokemon Vendor MVP Plan

## MVP

Implement a new shop-like Pokemon vendor on a feature branch from current
`master`. The first slice should prove that a script NPC can sell normal
Pokemon and Eggs without abusing Bag item delivery.

## First Runtime Slice

- Add a Pokemon-product table type.
- Add a script-facing entry point for a Pokemon vendor.
- Show a shop-like list with product name, price, and a cancel row.
- Support product kind: normal Pokemon or Egg.
- Support purchase mode: repeatable or one-time.
- Check money before purchase.
- Check destination capacity before purchase.
- Do not subtract money if delivery fails.
- Subtract money and increment shop stat after delivery succeeds.
- For one-time products, set a product flag after successful delivery.
- For one-time products already bought, either hide the row or show it as sold
  out. Hiding is simpler for a first slice.

## Product Shape

The exact C layout can change during implementation, but the product needs these
fields:

| Field | Notes |
|---|---|
| Species | Required. |
| Level | Normal Pokemon level. Egg hatch / challenge level policy is separate. |
| Price | Money cost. |
| Product kind | Normal Pokemon or Egg. |
| Purchase mode | Repeatable or one-time. |
| One-time flag | Optional event flag set after successful purchase. |
| Held item | Optional normal Pokemon held item. Egg held item policy should be explicit if supported. |
| Ball | Optional. Default Poke Ball is acceptable. |
| Move payload | Optional. MVP may use default moves, but product data should not block future custom moves. |
| Edit policy | Normal area-gated, Egg-origin always-editable, or none. |
| Origin marker policy | Whether an Egg product sets the vendor Egg-origin marker. |
| Progress policy | None, trainer wins while carried, challenge clears while carried, steps, or script-driven. |

## Recommended Implementation Shape

| Step | Files | Notes |
|---|---|---|
| 1 | `include/shop.h`, `src/shop.c` or new `src/pokemon_vendor.c` | Add a sibling vendor flow instead of modifying normal `pokemart` purchase finalization in place. |
| 2 | `src/scrcmd.c`, `data/script_cmd_table.inc`, `asm/macros/event.inc` | Add a script command / macro such as `pokemonvendor products` or `pokemartmon products`. |
| 3 | New product data file | Define fixed debug products first: repeat normal Pokemon, one-time normal Pokemon, repeat Egg, one-time Egg. |
| 4 | `src/script_pokemon_util.c` or a new helper | Reuse `ScriptGiveMonParameterized`-style creation for normal Pokemon and `CreateEgg` for Eggs. |
| 5 | Event flags / local ledger | Reserve one-time purchase flags only in the feature branch, then document them in the local config / flag ledger. |
| 6 | `include/pokemon.h`, `src/pokemon.c`, `src/egg_hatch.c` | Add a persistent vendor Egg-origin mon-data bit, set it on vendor Eggs, and preserve it when the Egg hatches. |
| 7 | `src/pokemon_summary_screen.c` | Show a compact Summary label / badge for vendor Egg-origin Pokemon so the edit entitlement is visible to the player. |
| 8 | Egg progress helper | Add no-op state hooks first, then gate the first battle-win / script-driven progress source behind a feature config. |
| 9 | Editor entitlement helper | Add a read-only policy function before wiring Summary / relearner / held-item UI. |

## Egg Progress Policy

Do not hardcode BP in the first slice. Use a feature-local concept:

- Egg must be in the player's party.
- Egg occupies a party slot and cannot battle.
- Progress source is configurable per product.
- MVP preferred source: trainer victory while the Egg is carried.
- Alternative source: script event after a challenge room clear.
- Step-based source remains possible, but should not reuse the existing Egg
  cycle counter.

This allows the reward to later become BP, a separate point counter, an unlock
flag, a product-specific hatch permission, or another challenge reward.

## Edit Entitlement Policy

Normal purchased Pokemon:

- Can be edited only in approved areas or through approved NPCs.
- Should use the same future move / item editor surfaces as the rest of the
  project.

Egg-origin Pokemon:

- Should unlock Status Editor access anywhere after hatch.
- Should be identifiable from Summary through a small label / badge.
- The entitlement must survive Summary transitions, party switching, box
  storage, save/load, and ordinary party menu use.
- Eggs themselves should still reject Status Editor entry until they hatch.
- Persistent per-mon tagging is required for feature complete.

Recommended helper contract:

- `IsVendorEggOriginMon(mon)` returns true for a non-Egg Pokemon hatched from a
  vendor Egg.
- `ShouldShowVendorEggOriginSummaryMark(mon)` returns true for marked vendor
  Eggs and/or their hatched Pokemon, depending on the final UI choice.
- `CanUseStateEditorAnywhere(mon)` returns true only for non-Egg Pokemon with
  the vendor Egg-origin marker.

## Non-Goals

- Do not change existing `pokemart` item behavior.
- Do not add image assets in a docs-only or master branch.
- Do not connect to all Battle Frontier BP UI in the first slice.
- Do not implement a full PC / Box rollback system.
- Do not assume the Unified Move Relearner or Pokemon State Editor runtime code
  is already on `master`.

## Current Contract

- Input: a script-provided product table.
- Output: either a delivered Pokemon / Egg and money subtraction, or a failed
  purchase with no money subtraction.
- Repeatability: controlled per product.
- One-time state: set only after successful delivery.
- Egg risk: party slot pressure, not a hidden stat penalty.
- Egg-origin identity: stored on the Pokemon, not inferred from species or
  ordinary hatch memo text.
- Status Editor access: always available only for non-Egg Pokemon with the
  vendor Egg-origin marker; ordinary purchased Pokemon remain area-gated.
- Reward currency: abstract until the implementation branch selects a concrete
  source.

## Future Work

- Product pools generated from partygen JSON.
- Product-specific species de-duplication.
- Product preview with Pokemon icon or Summary preview.
- Custom moves, IVs, EVs, ability, nature, and held item payload parity with
  the script `givemon` macro.
- Challenge-only Lv.50 normalization.
- Separate reward UI for Egg progress.
- Integration with Summary-first move/item editor.

## Open Questions

- Should first MVP require an empty party slot, or allow PC delivery for normal
  Pokemon and disallow PC delivery for Eggs?
- Should one-time products hide after purchase, show sold out, or remain visible
  but disabled?
- Should battle-win progress count only trainer battles, or also wild battles?
