# Friendly Shop Pokemon Vendor

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `master` `33932f1c30`; upstream `expansion/1.15.2-86-g2bb16c85b3` |
| Code status | Runtime implementation started on feature branch; Pokemon vendor menu is not implemented yet |
| Provenance | Local project overlay |

## Status

Status: Implementation started.

The first runtime slice is the global no-evolution rule required by the fixed
species product model. The actual Friendly Shop Pokemon vendor, sealed recruit
delivery, and bond / seal EXP progress systems remain future work on this
feature branch.

This feature adds a shop-like runtime that sells Pokemon products from a
Friendly Shop / Poke Mart style NPC. Products may be normal Pokemon, literal
Eggs, or Egg-like sealed recruits, and each product may be repeatable or
one-time only.

## Goal

- Let map scripts offer Pokemon through a familiar shop flow instead of a
  one-off `givemon` NPC.
- Support both repeat purchases and one-time purchases.
- Support Egg-like sealed products as a high-risk / high-reward option,
  including final evolutions and legendary Pokemon when the product table
  explicitly allows them.
- Allow product data to configure species, level, price, held item, and future
  edit-entitlement policy.
- Keep normal purchased Pokemon and sealed-origin Pokemon distinct for move and
  held-item editing rules.
- Make vendor sealed-origin Pokemon visibly identifiable from Summary.
- Allow only Pokemon unlocked from vendor sealed products to use the Status
  Editor anywhere.

## Current Decision

- Do not overload the current `pokemart` item list with fake item ids for
  Pokemon. The current shop path is item-centric and ends in `AddBagItem()`.
- Add a new Pokemon-vendor entry point that can reuse Poke Mart window style,
  money checks, list-menu behavior, and script blocking, but has its own
  product table and purchase finalizer.
- Treat BP as optional flavor only. The core design should use an abstract
  sealed progress / unlock counter; whether that maps to Battle Frontier BP,
  challenge-only points, or another reward value is a later balancing choice.
- For the first runtime slice, prefer battle-win or challenge-clear based
  sealed progress while the locked recruit is in the party. Step-based progress
  is possible for literal Eggs, but it collides more directly with the existing
  Egg-cycle hatch logic.
- The sealed-product risk is primarily the occupied party slot. While carried
  in locked state, the player effectively has one fewer battle-capable Pokemon.
- A sealed product is not limited to breedable species. Product data may point
  at a final evolution, restricted species, or legendary; the lock is a
  gameplay contract, not biological breeding compatibility.
- This runtime does not need Pokemon evolution. Treat every species as a fixed
  playable product. If a different evolution stage or form should be available,
  it should be listed as a separate product.
- The catalog is expected to lean toward final-stage Pokemon. Exceptions such
  as Clefairy-style middle / stone-evolution species are still fixed products;
  evolution stones and other evolution triggers should not evolve them.
- Vendor sealed products need a persistent origin marker that survives unlock /
  hatch resolution. The
  preferred implementation candidate is to promote the currently unused
  `PokemonSubstruct3.unused_0B` bit into a named
  `MON_DATA_VENDOR_SEALED_ORIGIN`
  field, if the implementation branch confirms it is unused in this fork.
- Summary should show a small "LOCKED" label while the recruit is still sealed,
  then a compact "Sealed Origin" / "Vendor Origin" style label or badge after
  unlock. This is player-facing proof of why the Pokemon receives
  always-available Status Editor access.
- The tone should be close to a Shadow Pokemon purification / bond-deepening
  flow: the Pokemon is present but not yet usable, then becomes available when
  enough bond / seal EXP has accumulated.
- The UI may call the meter "EXP" if that reads best in-game, but the first
  implementation should keep it separate from normal `MON_DATA_EXP` unless a
  later balancing pass intentionally wants level EXP side effects.
- Bond / seal EXP threshold should not be based on usage rate alone. Usage can
  raise the lock value for proven high-use Pokemon, but missing usage data must
  fall back to a neutral species tier instead of making absent Pokemon
  impossible or accidentally free.
- Experience tables are a last-resort reference only. Prefer product overrides
  and manual species tiers. If EXP tables are used, use only a small curve
  multiplier; do not copy raw cumulative EXP totals into bond thresholds.
  Values such as 600,000 are far outside the intended unlock scale.

## Scope

### In Scope

- A new script-facing vendor for Pokemon / sealed recruit products.
- Product data for repeatable vs one-time purchase.
- Money check, party / PC destination behavior, and no-charge-on-failure rules.
- Normal Pokemon purchase path based on existing scripted gift Pokemon helpers.
- Sealed recruit purchase path based on either existing `CreateEgg()` /
  `ScriptGiveEgg()` mechanics or a custom locked-mon creator, depending on the
  chosen UI language.
- A policy hook for move and held-item editing entitlement.
- A lock-state UI for unavailable shop rows, e.g. "LOCKED" / "Still locked".
- Bond / seal EXP progress while a sealed recruit is carried.
- A Summary-visible vendor sealed-origin marker.
- Sealed progress policy that can be driven by carried-party state.

### Out of Scope

- Merging runtime source into `master`.
- Replacing all normal Poke Marts.
- Adding actual image assets in docs-only work.
- Forcing use of Battle Frontier BP as the reward currency.
- A full Champions Challenge save-session implementation.
- Box-wide rollback or PC snapshot behavior.

## Related Docs

- [Investigation](investigation.md)
- [Implementation](implementation.md)
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
- Should sealed products resolve through vanilla Egg hatch flow, or through a
  custom unlock animation / message that better fits final evolutions and
  legendary Pokemon?
- Should repeat products be allowed to send Pokemon to PC, or should the vendor
  require an empty party slot for all purchases?
- Should sealed progress be tied to trainer wins only, any battle win, challenge
  room clear, or a product-specific script event?
- Should bond / seal EXP use a visible numeric meter, a small segmented gauge,
  or only text such as "The bond is deepening"?
- What exact Summary wording / badge should represent vendor sealed origin
  without being confused with ordinary daycare Eggs?
