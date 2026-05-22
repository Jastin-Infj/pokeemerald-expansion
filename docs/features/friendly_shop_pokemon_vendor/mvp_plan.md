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
- Show locked shop rows for products that are not available yet.
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
| Shop unlock flag / rule | Optional condition for locked rows before purchase. |
| Held item | Optional normal Pokemon held item. Sealed held item policy should be explicit if supported. |
| Ball | Optional. Default Poke Ball is acceptable. |
| Move payload | Optional. MVP may use default moves, but product data should not block future custom moves. |
| Edit policy | Normal area-gated, sealed-origin always-editable, or none. |
| Origin marker policy | Whether a sealed product sets the vendor sealed-origin marker. |
| Progress policy | None, trainer wins while carried, challenge clears while carried, steps, or script-driven. |
| Bond EXP threshold | Feature-owned progress required to release the locked recruit. |
| Bond EXP yield | Amount awarded per trainer win, challenge clear, script event, or EXP-equivalent event. |
| Reveal policy | Whether locked rows show the target species or display `????`. |

## Recommended Implementation Shape

| Step | Files | Notes |
|---|---|---|
| 1 | `include/shop.h`, `src/shop.c` or new `src/pokemon_vendor.c` | Add a sibling vendor flow instead of modifying normal `pokemart` purchase finalization in place. |
| 2 | `src/scrcmd.c`, `data/script_cmd_table.inc`, `asm/macros/event.inc` | Add a script command / macro such as `pokemonvendor products` or `pokemartmon products`. |
| 3 | New product data file | Define fixed debug products first: repeat normal Pokemon, one-time normal Pokemon, repeat sealed recruit, one-time sealed recruit, and one locked row. |
| 4 | `src/script_pokemon_util.c` or a new helper | Reuse `ScriptGiveMonParameterized`-style creation for normal Pokemon. For sealed recruits, choose either `CreateEgg`-based internal delivery or a custom locked-mon creator. |
| 5 | Event flags / local ledger | Reserve one-time purchase flags only in the feature branch, then document them in the local config / flag ledger. |
| 6 | `include/pokemon.h`, `src/pokemon.c`, `src/egg_hatch.c` or custom unlock file | Add a persistent vendor sealed-origin mon-data bit, set it on sealed products, and preserve it when the recruit unlocks. |
| 7 | `src/pokemon_summary_screen.c` | Show `LOCKED` while sealed and a compact sealed-origin label / badge after unlock so the edit entitlement is visible to the player. |
| 8 | Sealed progress helper | Add feature-owned bond / seal EXP state, then gate the first battle-win / script-driven progress source behind a feature config. |
| 9 | Editor entitlement helper | Add a read-only policy function before wiring Summary / relearner / held-item UI. |

## Sealed Progress Policy

Do not hardcode BP in the first slice. Use a feature-local concept:

- Sealed recruit must be in the player's party.
- Sealed recruit occupies a party slot and cannot battle until unlocked.
- Progress source is configurable per product.
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

- Each product should define a bond EXP threshold.
- When progress reaches threshold, Summary / party UI should show a ready state.
- Actual release can happen immediately or through an explicit confirm message.
  Explicit confirm is safer for UX because it gives the player feedback that the
  bond has deepened and the lock is gone.

This allows the reward to later become BP, a separate point counter, an unlock
flag, a product-specific release permission, or another challenge reward.

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
- Locked UI: shop rows can show `LOCKED` / `Still locked` and refuse purchase
  before the unlock condition is met.
- Bond EXP: feature-owned release progress accumulated while the sealed recruit
  is carried; not the same as normal level EXP by default.
- Status Editor access: always available only for non-locked Pokemon with the
  vendor sealed-origin marker; ordinary purchased Pokemon remain area-gated.
- Reward currency: abstract until the implementation branch selects a concrete
  source.

## Future Work

- Product pools generated from partygen JSON.
- Product-specific species de-duplication.
- Product preview with Pokemon icon or Summary preview.
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
- Should locked rows reveal the target legendary / final evolution, or hide the
  species until the row unlocks?
- Should release happen automatically at full bond EXP, or require a Summary /
  NPC confirmation step?
