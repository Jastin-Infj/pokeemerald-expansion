# Nonconsumable Held Items Risks

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `ff4e825258`; `git describe` = `expansion/1.15.2-59-gff4e825258` |
| Code status | Docs-only risk register |
| Provenance | Local source read and feature planning |

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Clearing battle consumption instead of restoring at battle end | High | Breaks `Recycle`, `Pickup`, `Harvest`, `Cud Chew`, `Unburden`, Belch, and AI assumptions. | Keep battle-time item loss intact; restore from `originalItem` after battle. |
| Berry restore changes too broad | Medium | Wild / trainer / facility battles may all inherit the new restore rule if not gated. | Decide global vs mode-specific policy before runtime implementation. |
| Item duplication through `TAKE` | High | Catalog assignment can create infinite physical Bag copies if Take still calls `AddBagItem`. | In catalog mode, Take clears the held item without adding to Bag. |
| Item deletion through `GIVE` | High | If Give still removes from Bag while multiple mons are allowed, the feature does not solve quantity friction. | In catalog mode, Give sets held item without `RemoveBagItem`. |
| Switch path quantity drift | High | Switch can remove the incoming item and return the old item, causing copy gain/loss. | Audit both party-origin and bag-origin switch handlers. |
| PC Storage bypass | High | Storage item mode can move held items outside the party menu callbacks. | Disable Storage item mode in catalog MVP or add catalog-aware storage helpers. |
| Mail cloning | High | Mail includes saved message data, so catalog cloning can corrupt or duplicate mail state. | Exclude Mail from catalog MVP. |
| Toss ambiguity | Medium | Toss can become "delete catalog item" or just "clear held item" depending on UI wording. | Disable Toss in catalog mode or rename behavior clearly. |
| Stolen / swapped final ownership | High | `Thief`, `Covet`, `Trick`, `Switcheroo`, `Bestow`, `Symbiosis`, `Pickpocket`, and `Magician` can move items during battle. | Keep battle-time behavior, but define battle-end ownership per battle type before broad restore. |
| Air Balloon / Corrosive Gas exceptions | Medium | Current code avoids saving these to `usedHeldItem`; battle-end original restore may intentionally or accidentally bypass that. | Add explicit tests and policy notes before restoring all originals. |
| Frontier / Tent rule conflict | Medium | Existing facilities enforce duplicate held item rules and may restore held items through their own backup paths. | Treat facilities as separate rule owners; do not globally bypass item clause. |
| Link / recorded battle desync | High | Battle-end item restore may affect reproducibility and sync assumptions. | Exclude link / recorded battle until explicitly investigated. |
| Save compatibility creep | Medium | Unlock tracking for a catalog can tempt new SaveBlock fields. | MVP should use existing Bag presence or fixed debug / facility list; add save state only with a separate save-flow decision. |
| UI mismatch | Medium | Bag quantity staying constant can confuse players if text still says an item was given / taken normally. | Add clear catalog-specific text when runtime implementation reaches UI. |
| Item importance / Key Item confusion | Low | Not every item should be assignable as a held item. | Use existing holdable item checks and exclude non-held categories. |

## Accepted Risk For First Runtime Slice

The first battle-end restore slice does not solve Bag quantity or duplicate held
item assignment. That is acceptable because it targets only permanent loss after
battle and avoids Party / Bag / Storage UI churn.

## Blockers Before Catalog Runtime

- Decide whether catalog mode is global, facility-only, or debug-only.
- Decide whether Storage item mode is disabled or supported.
- Decide Mail policy.
- Decide Take / Toss wording and behavior.
- Decide whether owned Bag copy, unlock flags, or fixed allowed list controls
  catalog availability.

## Future Risk Controls

- Add helper functions for catalog-aware Bag mutation instead of sprinkling
  direct `RemoveBagItem` / `AddBagItem` exceptions through UI code.
- Add focused tests for item quantity drift before any mGBA UI validation.
- Keep item clause validation separate from catalog ownership.
- Re-check upstream restore changes when upgrading past the current baseline.
