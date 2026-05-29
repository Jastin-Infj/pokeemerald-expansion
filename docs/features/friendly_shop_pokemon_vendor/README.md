# Friendly Shop Pokemon Vendor

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-29 |
| Baseline | `integration/runtime-dev-20260529`; source shelf `feature/global-no-evolution-20260523` / PR #57 |
| Code status | Runtime implementation adopted in integration branch; master remains docs / Lua-only |
| Provenance | Local project overlay |

## Status

Status: First runtime slice implemented on `feature/global-no-evolution-20260523`
and adopted into `integration/runtime-dev-20260529` on 2026-05-29.

The branch now includes the global no-evolution rule, a new script-facing
Pokemon vendor, normal Pokemon purchase delivery, Egg-like sealed recruit
delivery, one-time / repeat products, gated shop rows, script-driven bond progress,
trainer victory bond reward messages, purchasable mystery sealed products, and
Summary / party / PC-visible locked sealed status.

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
- Trainer or challenge scripts should award sealed bond progress explicitly with
  per-NPC amounts such as 10, 20, 80, or 100. Field / room clear scripts can
  use `pokemonvendorawardbond amount[, showMessage]`; trainer battle setup
  scripts can use `pokemonvendorqueuebattlebond amount` so the reward is paid
  and displayed from the battle victory text flow.
- Debug `Scripts... -> Script 3` is the current normal-trainer validation
  route: it queues 20 sealed bond EXP, starts a regular trainer battle, and
  displays the reward inside the win-message sequence after the money message.
- In the runtime integration branch, debug `Scripts... -> Script 2` is reserved
  for Scout Selection pick-6, so vendor-specific checks use `Script 1` or
  `Script 3`.
- For the first runtime slice, prefer battle-win or challenge-clear based
  sealed progress while the locked recruit is in the party. Step-based progress
  is possible for literal Eggs, but it collides more directly with the existing
  Egg-cycle hatch logic.
- The sealed-product risk is primarily the occupied party slot after the
  recruit is carried. If the party is full at purchase time, sealed recruits now
  follow the normal gift-Pokemon fallback and are sent to PC when storage has
  room. They do not gain party-carried bond progress until the player moves
  them into the party.
- The word "locked" refers to the post-acquisition sealed recruit state. A shop
  row that is not buyable yet is only a gated offer; it may hide the species or
  price until its unlock flag is met, but that is not the same as a locked
  Pokemon in the party.
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
  hatch resolution. The implementation promotes the currently unused
  `PokemonSubstruct3.unused_0B` bit into
  `MON_DATA_VENDOR_SEALED_ORIGIN`.
- Mystery sealed products can hide species as `?????` while still being
  purchasable. Their actual species is selected at purchase time from the
  product's base species plus optional random-species candidates.
- Summary and party / PC icon surfaces should show the actual Pokemon identity
  for named locked products while the recruit is still locked, plus a small
  `LOCKED` label and bond progress text. Concealed `?????` products remain
  hidden after purchase until unlock, using generic Egg icon / sprite /
  nickname treatment plus `LOCKED` status. Ordinary Eggs must keep ordinary Egg
  visuals.
  A graphic lock badge can be added in a later UI pass.
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
- Sealed recruit full-party fallback to PC when storage has room.
- A policy hook for move and held-item editing entitlement.
- A gate-state UI for unavailable shop rows, e.g. "GATED" / "Not available".
- Bond / seal EXP progress while a sealed recruit is carried.
- Script macros for field / room rewards and queued trainer battle rewards.
  Field rewards can show or suppress bond EXP and unlock messages per call
  site; queued trainer rewards display from the battle victory text flow.
- Mystery sealed products with hidden display and random purchase-time species
  selection.
- A Summary-visible vendor sealed-origin marker.
- Sealed progress policy that can be driven by carried-party state.

### Out of Scope

- Merging runtime source into `master`.
- Replacing all normal Poke Marts.
- Adding actual image assets in docs-only work.
- Forcing use of Battle Frontier BP as the reward currency.
- A full Champions Challenge save-session implementation.
- Box-wide rollback or PC snapshot behavior.
- Trainer item / TM drop tables after battle. This should be a separate reward
  feature because it affects economy balance.
- Per-row Pokemon icon rendering inside the vendor list. The current runtime
  shows the selected product's real icon in the detail pane; row icons remain a
  later custom-list pass because the standard list row height is too small for
  32x32 mon icons without a layout rewrite.
- Gen 7 / Gen 8-style "add to party and choose a party member to send to PC"
  swap UI. That should be a separate gift / capture / vendor delivery feature
  because it cuts across more than this Pokemon vendor.

## Related Docs

- [Pokemon Vendor Manual](../../manuals/pokemon_vendor_manual.md)
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

- Should sealed unlock use an explicit confirmation / animation instead of the
  current immediate unlock when bond reaches threshold?
- Which non-debug map / NPC should host the first real product list?
- Should future progress be awarded from trainer wins, challenge clears, or
  product-specific script events only?
- Should the Summary origin proof become a visual badge instead of memo text?
- Should PC-stored sealed recruits ever gain bond progress, or is
  party-carried progress the intended risk?
- Which editor / relearner surfaces should call `PokemonVendor_IsEditEntitled()`
  first?
