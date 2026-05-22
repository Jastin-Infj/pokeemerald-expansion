# Friendly Shop Pokemon Vendor

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-22 |
| Baseline | `master` `2bb16c85b311`; upstream `expansion/1.15.2-86-g2bb16c85b3` |
| Code status | Docs-only investigation; no runtime code on `master` |
| Provenance | Local project overlay |

## Status

Status: Planned.

This feature adds a shop-like runtime that sells Pokemon products from a
Friendly Shop / Poke Mart style NPC. Products may be normal Pokemon or Eggs,
and each product may be repeatable or one-time only.

## Goal

- Let map scripts offer Pokemon through a familiar shop flow instead of a
  one-off `givemon` NPC.
- Support both repeat purchases and one-time purchases.
- Support Egg products as a high-risk / high-reward option.
- Allow product data to configure species, level, price, held item, and future
  edit-entitlement policy.
- Keep normal purchased Pokemon and Egg-origin Pokemon distinct for move and
  held-item editing rules.
- Make vendor Egg-origin Pokemon visibly identifiable from Summary.
- Allow only Pokemon hatched from vendor Eggs to use the Status Editor anywhere.

## Current Decision

- Do not overload the current `pokemart` item list with fake item ids for
  Pokemon. The current shop path is item-centric and ends in `AddBagItem()`.
- Add a new Pokemon-vendor entry point that can reuse Poke Mart window style,
  money checks, list-menu behavior, and script blocking, but has its own
  product table and purchase finalizer.
- Treat BP as optional flavor only. The core design should use an abstract
  Egg progress / unlock counter; whether that maps to Battle Frontier BP,
  challenge-only points, or another reward value is a later balancing choice.
- For the first runtime slice, prefer battle-win based Egg progress while the
  Egg is in the party. Step-based progress is possible, but it collides more
  directly with the existing Egg-cycle hatch logic.
- The Egg risk is primarily the occupied party slot. While carried, the player
  effectively has one fewer battle-capable Pokemon because Eggs cannot battle.
- Vendor Eggs need a persistent origin marker that survives hatching. The
  preferred implementation candidate is to promote the currently unused
  `PokemonSubstruct3.unused_0B` bit into a named `MON_DATA_VENDOR_EGG_ORIGIN`
  field, if the implementation branch confirms it is unused in this fork.
- Summary should show a small "Egg-Origin" / "Vendor Egg" style label or badge
  for marked Pokemon. This is player-facing proof of why the Pokemon receives
  always-available Status Editor access.

## Scope

### In Scope

- A new script-facing vendor for Pokemon / Egg products.
- Product data for repeatable vs one-time purchase.
- Money check, party / PC destination behavior, and no-charge-on-failure rules.
- Normal Pokemon purchase path based on existing scripted gift Pokemon helpers.
- Egg purchase path based on existing `CreateEgg()` / `ScriptGiveEgg()` flow.
- A policy hook for move and held-item editing entitlement.
- A Summary-visible vendor Egg-origin marker.
- Egg progress policy that can be driven by carried-party state.

### Out of Scope

- Merging runtime source into `master`.
- Replacing all normal Poke Marts.
- Adding actual image assets in docs-only work.
- Forcing use of Battle Frontier BP as the reward currency.
- A full Champions Challenge save-session implementation.
- Box-wide rollback or PC snapshot behavior.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Unified Move Relearner](../unified_move_relearner/README.md)
- [Pokemon State Editor](../pokemon_state_editor/README.md)
- [Nonconsumable Held Items](../nonconsumable_held_items/README.md)
- [Champions Challenge Facility](../champions_challenge/README.md)

## Open Questions

- How should a purchased Pokemon's origin / edit entitlement be stored
  persistently without exhausting Pokemon struct spare bits?
- Should Egg products hatch at the normal hatch level first and then be scaled
  to Lv.50 for a challenge, or should the product create a Lv.50 Pokemon when
  the Egg resolves?
- Should repeat products be allowed to send Pokemon to PC, or should the vendor
  require an empty party slot for all purchases?
- Should Egg progress be tied to trainer wins only, any battle win, challenge
  room clear, or a product-specific script event?
- What exact Summary wording / badge should represent vendor Egg origin without
  being confused with ordinary daycare Eggs?
