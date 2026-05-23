# Friendly Shop Pokemon Vendor MVP Plan

## MVP

Implement a new shop-like Pokemon vendor on a feature branch from current
`master`. The first slice should prove that a script NPC can sell normal
Pokemon and Egg-like sealed recruits without abusing Bag item delivery.

## First Runtime Slice

- Add a Pokemon-product table type.
- Add a script-facing entry point for a Pokemon vendor.
- Show a shop-like list with product name, price, and a cancel row.
- Support product kind: normal Pokemon or sealed recruit.
- Allow sealed recruit products to target any explicitly listed species,
  including final evolutions and legendary Pokemon.
- Show gated shop rows for products that are not available yet.
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
| Level | Normal Pokemon level. Sealed unlock / challenge level policy is separate. |
| Price | Money cost. |
| Product kind | Normal Pokemon, literal Egg, or sealed recruit. |
| Purchase mode | Repeatable or one-time. |
| One-time flag | Optional event flag set after successful purchase. |
| Shop unlock flag / rule | Optional condition for gated rows before purchase. |
| Held item | Optional normal Pokemon held item. Sealed held item policy should be explicit if supported. |
| Ball | Optional. Default Poke Ball is acceptable. |
| Move payload | Optional. MVP may use default moves, but product data should not block future custom moves. |
| Edit policy | Normal area-gated, sealed-origin always-editable, or none. |
| Origin marker policy | Whether a sealed product sets the vendor sealed-origin marker. |
| Progress policy | None, trainer wins while carried, challenge clears while carried, steps, or script-driven. |
| Bond EXP threshold | Feature-owned progress required to release the locked recruit. |
| Bond EXP yield | Amount awarded per trainer win, challenge clear, script event, or EXP-equivalent event. |
| Reveal policy | Whether gated rows show the target species, and whether purchasable mystery rows display `????`. |
| Random species pool | Optional candidate species for mystery products; actual species is selected when purchased. |
| Evolution policy | Runtime species are fixed; global evolution lock is allowed and preferred. |

## Recommended Implementation Shape

| Step | Files | Notes |
|---|---|---|
| 1 | `include/shop.h`, `src/shop.c` or new `src/pokemon_vendor.c` | Add a sibling vendor flow instead of modifying normal `pokemart` purchase finalization in place. |
| 2 | `src/scrcmd.c`, `data/script_cmd_table.inc`, `asm/macros/event.inc` | Add a script command / macro such as `pokemonvendor products` or `pokemartmon products`. |
| 3 | New product data file | Define fixed debug products first: repeat normal Pokemon, one-time normal Pokemon, repeat sealed recruit, one-time sealed recruit, and one gated row. |
| 4 | `src/script_pokemon_util.c` or a new helper | Reuse `ScriptGiveMonParameterized`-style creation for normal Pokemon. For sealed recruits, choose either `CreateEgg`-based internal delivery or a custom locked-mon creator. |
| 5 | Event flags / local ledger | Reserve one-time purchase flags only in the feature branch, then document them in the local config / flag ledger. |
| 6 | `include/pokemon.h`, `src/pokemon.c`, `src/egg_hatch.c` or custom unlock file | Add a persistent vendor sealed-origin mon-data bit, set it on sealed products, and preserve it when the recruit unlocks. |
| 7 | `src/pokemon_summary_screen.c` | Show `LOCKED` while sealed and a compact sealed-origin label / badge after unlock so the edit entitlement is visible to the player. |
| 8 | Sealed progress helper | Add feature-owned bond / seal EXP state, then gate the first battle-win / script-driven progress source behind a feature config. |
| 9 | Global evolution-block helper | Block all evolution triggers for the runtime, not just sealed-origin Pokemon. |
| 10 | Editor entitlement helper | Add a read-only policy function before wiring Summary / relearner / held-item UI. |

## Sealed Progress Policy

Do not hardcode BP in the first slice. Use a feature-local concept:

- Sealed recruit must be in the player's party.
- Sealed recruit occupies a party slot and cannot battle until unlocked.
- Progress source is configurable per product.
- Trainer / NPC scripts can call `pokemonvendorawardbond amount` after battle
  to award per-NPC progress, e.g. 10 or 20 for regular trainers, 80 for Elite
  Four, and 100 for a Champion-class clear.
- The reward macro has a `showMessage` parameter so noisy repeat rewards can be
  hidden while boss clears can show bond EXP and unlock messages.
- The in-game feel is "bond deepened" / Shadow Pokemon style release.
- Store progress as feature-owned bond / seal EXP, not normal
  `MON_DATA_EXP`, unless a later branch intentionally wants level EXP side
  effects.
- MVP preferred source: trainer victory or challenge clear while the sealed
  recruit is carried.
- Alternative source: script event after a challenge room clear.
- Optional later source: award bond EXP proportional to normal battle EXP
  earned by the party, without directly modifying the locked recruit's level
  EXP.
- Step-based source remains possible, but should not reuse the existing Egg
  cycle counter if the implementation uses literal Egg internals.

Unlock threshold:

- Each product should define a bond EXP threshold in the MVP.
- When progress reaches threshold, Summary / party UI should show a ready state.
- Actual release can happen immediately or through an explicit confirm message.
  Explicit confirm is safer for UX because it gives the player feedback that the
  bond has deepened and the lock is gone.
- Later automatic thresholds may use usage rate, but usage must be a modifier
  rather than the base. Missing usage data should use a neutral species-tier
  prior so Pokemon absent from current pools are not unfairly punished.

This allows the reward to later become BP, a separate point counter, an unlock
flag, a product-specific release permission, or another challenge reward.

## Bond EXP Threshold Policy

First implementation:

- Product table owns `bondExpThreshold`.
- Product table owns `bondExpYieldPolicy`.
- No runtime usage-rate calculation is required.
- A product with no explicit threshold should fail validation or use a small
  debug-only default.

Future generator / balancing pass:

- Start with a species tier base value.
- Use manual species tier / product overrides before any EXP table reference.
- If using species EXP tables as a last resort, convert the table to a small
  clamped multiplier; never use raw cumulative EXP values as bond EXP
  thresholds.
- Apply a usage multiplier only when usage data is reliable.
- Clamp thresholds into product-friendly bands.
- Allow product overrides to replace any generated value.
- Prefer additive band deltas over large multiplier stacks. The formula should
  produce a reviewable draft value, not the final truth.

Example bands for tuning, not final numbers:

| Band | Product examples | Threshold feel |
|---|---|---|
| Low | weak or intentionally underused Pokemon | 1-2 small clears. |
| Standard | ordinary final forms | several battles / one short route. |
| High | strong final forms, strong role compression | one meaningful challenge segment. |
| Restricted | legendary / mythical / special prize | explicit hand-tuned threshold. |

This avoids the "Pokemon with no usage data is impossible to release" problem.
It also avoids the "600,000 EXP to unlock" problem: bond EXP thresholds should
live in a compact feature-owned scale that can be cleared through meaningful
challenge progress, not long-form level grinding.

Suggested formula shape:

```text
threshold = override
         or clamp(roundTo25(tierBase + usageDelta + curveDelta + productDelta),
                  minThreshold,
                  maxThreshold)
```

Rules:

- `override` is the preferred path for known exceptions.
- `tierBase` is the main value.
- `usageDelta` is small and becomes `0` when data is missing.
- `curveDelta` is tiny and only used if EXP table reference is enabled.
- `productDelta` is a human-authored adjustment for local context.
- Generated thresholds must be easy to inspect and hand-edit.

## Fixed Species Policy

This game does not need Pokemon evolution. Every Pokemon should be treated as
the intended fixed product species, even if it is normally an unevolved species,
final evolution, legendary, mythical, or special form.

Contract:

- `CanPokemonEvolveInThisRuntime(mon)` returns false for every Pokemon while
  the no-evolution runtime rule is enabled.
- Level-up evolution is blocked.
- Item evolution is blocked.
- Stone evolution is blocked even when the required stone is available, e.g.
  Clefairy remains Clefairy under the fixed-species policy.
- Trade / link evolution is blocked.
- Friendship / move / map / time / form-condition evolution is blocked.
- Status Editor changes must not re-enable evolution.

Design implication: threshold balancing should treat the product as a fixed
species, not as part of a family path. A rare base form can still have a high
threshold if the product intends it as a special fixed-form reward.

## Locked / Release UX

Minimum player-facing states:

| State | Suggested text | Behavior |
|---|---|---|
| Locked | `LOCKED` / `The bond is still faint.` | Cannot battle or use Status Editor. |
| Progressing | `Bond EXP 40/100` or a small gauge | Still locked, but the player can see progress. |
| Ready | `READY` / `The bond is deep enough.` | Release can trigger or be confirmed. |
| Released | `Sealed Origin` / `Bonded` | Can battle and can use Status Editor anywhere. |

## Edit Entitlement Policy

Normal purchased Pokemon:

- Can be edited only in approved areas or through approved NPCs.
- Should use the same future move / item editor surfaces as the rest of the
  project.

Sealed-origin Pokemon:

- Should unlock Status Editor access anywhere after unlock / hatch resolution.
- Should be identifiable from Summary through a small label / badge.
- The entitlement must survive Summary transitions, party switching, box
  storage, save/load, and ordinary party menu use.
- Locked recruits themselves should still reject Status Editor entry until they
  unlock.
- Persistent per-mon tagging is required for feature complete.

Recommended helper contract:

- `IsVendorSealedOriginMon(mon)` returns true for a non-locked Pokemon resolved
  from a vendor sealed product.
- `ShouldShowVendorLockedSummaryMark(mon)` returns true while the recruit is
  still sealed.
- `GetVendorSealedBondProgress(mon)` returns current and required bond EXP for
  Summary / party UI.
- `CanReleaseVendorSealedRecruit(mon)` returns true once bond EXP reaches the
  product threshold.
- `ShouldShowVendorSealedOriginSummaryMark(mon)` returns true for unlocked
  sealed-origin Pokemon.
- `CanUseStateEditorAnywhere(mon)` returns true only for non-locked Pokemon with
  the vendor sealed-origin marker.

## Non-Goals

- Do not change existing `pokemart` item behavior.
- Do not add image assets in a docs-only or master branch.
- Do not connect to all Battle Frontier BP UI in the first slice.
- Do not implement a full PC / Box rollback system.
- Do not assume the Unified Move Relearner or Pokemon State Editor runtime code
  is already on `master`.

## Current Contract

- Input: a script-provided product table.
- Output: either a delivered Pokemon / sealed recruit and money subtraction, or
  a failed purchase with no money subtraction.
- Repeatability: controlled per product.
- One-time state: set only after successful delivery.
- Sealed risk: party slot pressure, not a hidden stat penalty.
- Sealed-origin identity: stored on the Pokemon, not inferred from species or
  ordinary hatch memo text.
- Gated UI: shop rows can show `GATED` / unavailable text and refuse purchase
  before the availability condition is met. This is separate from the locked
  sealed-recruit state after purchase.
- Bond EXP: feature-owned release progress accumulated while the sealed recruit
  is carried; not the same as normal level EXP by default.
- Status Editor access: always available only for non-locked Pokemon with the
  vendor sealed-origin marker; ordinary purchased Pokemon remain area-gated.
- Evolution: all Pokemon remain fixed species under this runtime; sell separate
  products for separate forms / stages.
- Reward currency: abstract until the implementation branch selects a concrete
  source.

## Future Work

- Product pools generated from partygen JSON.
- Product-specific species de-duplication.
- Product preview with Pokemon icon or Summary preview.
- Trainer / challenge item-drop tables, including TM / held-item drops, as a
  separate economy feature.
- Custom moves, IVs, EVs, ability, nature, and held item payload parity with
  the script `givemon` macro.
- Challenge-only Lv.50 normalization.
- Separate reward UI for sealed progress.
- Integration with Summary-first move/item editor.

## Open Questions

- Should first MVP require an empty party slot, or allow PC delivery for normal
  Pokemon and disallow PC delivery for sealed recruits?
- Should one-time products hide after purchase, show sold out, or remain visible
  but disabled?
- Should battle-win progress count only trainer battles, or also wild battles?
- Should gated rows reveal the target legendary / final evolution, or hide the
  species until the row unlocks?
- Should release happen automatically at full bond EXP, or require a Summary /
  NPC confirmation step?
- Should automatic threshold generation use partygen catalog usage, trainer
  pool frequency, manual tier files, or a mix?
