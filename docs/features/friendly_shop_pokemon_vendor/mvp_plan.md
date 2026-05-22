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
| Progress policy | None, trainer wins while carried, challenge clears while carried, steps, or script-driven. |

## Recommended Implementation Shape

| Step | Files | Notes |
|---|---|---|
| 1 | `include/shop.h`, `src/shop.c` or new `src/pokemon_vendor.c` | Add a sibling vendor flow instead of modifying normal `pokemart` purchase finalization in place. |
| 2 | `src/scrcmd.c`, `data/script_cmd_table.inc`, `asm/macros/event.inc` | Add a script command / macro such as `pokemonvendor products` or `pokemartmon products`. |
| 3 | New product data file | Define fixed debug products first: repeat normal Pokemon, one-time normal Pokemon, repeat Egg, one-time Egg. |
| 4 | `src/script_pokemon_util.c` or a new helper | Reuse `ScriptGiveMonParameterized`-style creation for normal Pokemon and `CreateEgg` for Eggs. |
| 5 | Event flags / local ledger | Reserve one-time purchase flags only in the feature branch, then document them in the local config / flag ledger. |
| 6 | Egg progress helper | Add no-op state hooks first, then gate the first battle-win / script-driven progress source behind a feature config. |
| 7 | Editor entitlement helper | Add a read-only policy function before wiring Summary / relearner / held-item UI. |

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

- Should unlock broader editing after hatch.
- The entitlement must survive Summary transitions and ordinary party menu use.
- Persistent per-mon tagging is the hard part and must be solved before this is
  treated as feature complete.

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
