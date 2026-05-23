# Friendly Shop Pokemon Vendor Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Fake item ids for Pokemon products | High | Normal shop purchase calls `AddBagItem()`, so fake Pokemon items can leak into Bag, item description, item icon, and importance logic. | Use a dedicated Pokemon product table and delivery finalizer. |
| Per-mon origin tracking | High | Sealed-origin always-editable policy needs to know that an unlocked Pokemon came from a vendor sealed product. | Prefer a named mon-data bit backed by `PokemonSubstruct3.unused_0B` after confirming it is truly unused in this fork. Preserve it through hatch / unlock. |
| Summary marker confusion | Medium | Ordinary daycare Eggs also show hatch-related memo text, while this feature may unlock final evolutions or legendaries. | Add dedicated `LOCKED` and sealed-origin Summary labels / badges. Do not infer from normal hatch memo text. |
| Misusing event flags as per-mon identity | High | A global product flag proves the shop product was bought, not that a specific Pokemon came from that sealed recruit. It can misgrant editor access to the wrong Pokemon. | Store identity on the Pokemon when possible; use SaveBlock side tables only for product progress / counters. |
| SaveBlock pressure | High | Gated row state, sealed progress, entitlement state, and product history can grow past saved vars / flags. | Use event flags only for one-time / simple shop gates; use compact dedicated state for repeatable progress. Check `docs/flows/save_data_flow_v15.md` before source work. |
| Battle-end hook side effects | High | Trainer battle end has Pyramid, Trainer Hill, follower, no-whiteout, forfeit, and trainer flag branches. A global sealed-progress hook can run in the wrong mode. | Prefer script-driven or challenge-clear progress first. If using trainer wins, add a narrow helper with explicit battle-type guards. |
| Egg cycle collision | High | Existing Eggs store hatch cycles in `MON_DATA_FRIENDSHIP`. Reusing that value for sealed unlock progress can break hatching. | Store unlock progress separately from hatch cycles. If using custom sealed recruits, avoid Egg-cycle fields entirely. |
| Normal EXP collision | High | Using `MON_DATA_EXP` for lock release can alter level, level caps, and accidentally pass through vanilla evolution checks if the global lock is incomplete. | Use feature-owned bond / seal EXP by default. If normal EXP-equivalent gain is desired, convert it into bond EXP rather than writing directly to `MON_DATA_EXP`. |
| Raw EXP threshold inflation | High | Copying cumulative EXP table values into bond thresholds can create extreme requirements such as 600,000 points. | Treat EXP tables as last-resort references only. Use product overrides / manual tiers first; if needed, use EXP curves only as a clamped multiplier. |
| Usage-rate threshold bias | Medium | If usage rate is the main input, Pokemon absent from current pools have bad or missing data and can become unfairly expensive or too cheap. | MVP uses explicit product thresholds. Future auto-generation uses species tier as base, smoothed usage only as a modifier, and neutral fallback for missing data. |
| Formula overconfidence | Medium | A neat formula will still fail many hand-authored products, favorites, story rewards, and weird species. | Formula output is draft-only. Product-level absolute threshold override is mandatory. |
| Shadow bit collision | Medium | `PokemonSubstruct3.isShadow` exists and fits the flavor, but may later imply real Shadow Pokemon mechanics, purification, ribbons, or battle behavior. | Treat Shadow Pokemon as inspiration only. Use dedicated sealed helpers and marker bits unless intentionally adopting a full Shadow ruleset. |
| Species legality confusion | Medium | Final evolutions and legendaries are intentionally allowed even though the feature may use Egg-like language. | Treat eligibility as product-table allowlist data, not breeding compatibility. Use lock/seal UI text instead of daycare Egg text. |
| Global evolution leak | High | This game does not need evolution. If any Pokemon can evolve through level-up, item, trade, friendship, script, or special conditions, product identity and balance break. | Add a global no-evolution guard through the central evolution resolver and test every trigger class. Sell alternate stages/forms as separate products. |
| Evolution item UX | Medium | A player may try to use an evolution stone on a fixed-species Pokemon such as Clefairy. A generic failure message can feel like a bug. | Add or reuse a clear "This Pokemon's form is fixed" style rejection path for evolution items. |
| Lv.50 sealed policy ambiguity | Medium | Existing `CreateEgg()` uses `EGG_HATCH_LEVEL`; the desired challenge behavior may need Lv.50 after unlock or battle-only scaling. | Record the product level policy explicitly before implementation. |
| Party full behavior | Medium | `GiveCapturedMonToPlayer()` may send Pokemon to PC, but the intended sealed risk depends on carrying it in party. | Sealed products now follow PC fallback when the party is full and storage has room. Bond progress remains party-carried, so PC delivery pauses progress until the player carries the recruit. A party swap prompt is deferred to a broader delivery UX feature. |
| One-time flag allocation | Medium | New one-time products can consume many event flags. | Start with a small product count and document flag ownership in the local ledger. |
| Editor integration order | Medium | Unified Move Relearner, Pokemon State Editor, and held-item catalog branches are not on `master`. | Add entitlement hook points first; wire editor UI only when the editor branch is selected for integration. |
| Economy coupling to BP | Low | Using Battle Frontier BP directly may create unwanted economy coupling. | Keep BP optional. Use an abstract progress / reward policy until balancing is decided. |

## Impact Notes

- `shop.c` is a sensitive shared UI path. The safest implementation leaves
  existing item shops behaviorally unchanged.
- `script_pokemon_util.c` is the safest creation reference, especially the
  parameterized gift path.
- Bond / sealed progress touches battle-end or challenge-clear flow only if the selected
  policy needs it. A pure vendor MVP can ship without battle hooks.
- Move and held-item edit entitlement affects Summary, party menu, relearner,
  and held-item catalog features. The first branch should expose a policy helper
  rather than directly scattering checks.
- Summary marker work should stay text/badge based in the first slice. New
  graphic assets are implementation artifacts and should not enter docs-only
  `master` work.

## Accepted Risks For First Slice

- Product UI may initially be text-only, with no Pokemon icon preview.
- BP-specific reward UI is deferred.
- Lv.50 normalization may be deferred if the first slice only proves purchase
  and delivery.
- Always-editable sealed-origin policy may be documented and stubbed until the
  vendor-origin mon-data bit is implemented and validated through hatch.

## Open Questions

- Is a global product unlock table acceptable, or must entitlement follow the
  individual Pokemon forever?
- Should the future gift / capture / vendor delivery UI offer a Gen 7 /
  Gen 8-style party swap prompt when the party is full?
- Should bought normal Pokemon use default learnset moves or a product-specific
  move payload from the start?
- Should vendor sealed-origin be tradeable/transferable as a persistent marker, or
  should editor entitlement apply only while the Pokemon belongs to this save?
- Do gated shop rows reveal species like legendary names, or keep them hidden
  as `????` until unlocked?
- Is bond EXP purely party-carried progress, or should it also persist while the
  sealed recruit is stored in PC?
- Should high-usage Pokemon always have higher thresholds, or should manual
  product overrides keep event / story rewards fast when needed?
