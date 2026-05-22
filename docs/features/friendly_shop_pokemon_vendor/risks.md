# Friendly Shop Pokemon Vendor Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Fake item ids for Pokemon products | High | Normal shop purchase calls `AddBagItem()`, so fake Pokemon items can leak into Bag, item description, item icon, and importance logic. | Use a dedicated Pokemon product table and delivery finalizer. |
| Per-mon origin tracking | High | Egg-origin always-editable policy needs to know that a hatched Pokemon came from a vendor Egg. The current docs did not confirm a safe persistent spare field. | Add a dedicated entitlement design before wiring editors. If needed, scope MVP to challenge-local or product-global permissions. |
| SaveBlock pressure | High | Egg progress, entitlement state, and product history can grow past saved vars / flags. | Use event flags only for one-time products; use compact dedicated state for repeatable progress. Check `docs/flows/save_data_flow_v15.md` before source work. |
| Battle-end hook side effects | High | Trainer battle end has Pyramid, Trainer Hill, follower, no-whiteout, forfeit, and trainer flag branches. A global Egg-progress hook can run in the wrong mode. | Prefer script-driven or challenge-clear progress first. If using trainer wins, add a narrow helper with explicit battle-type guards. |
| Egg cycle collision | High | Existing Eggs store hatch cycles in `MON_DATA_FRIENDSHIP`. Reusing that value for unlock progress can break hatching. | Store unlock progress separately from hatch cycles. |
| Lv.50 Egg policy ambiguity | Medium | Existing `CreateEgg()` uses `EGG_HATCH_LEVEL`; the desired challenge behavior may need Lv.50 after hatch or battle-only scaling. | Record the product level policy explicitly before implementation. |
| Party full behavior | Medium | `GiveCapturedMonToPlayer()` may send Pokemon to PC, but the intended Egg risk depends on carrying it in party. | MVP should require an empty party slot for Egg products. Normal products may allow PC delivery if desired. |
| One-time flag allocation | Medium | New one-time products can consume many event flags. | Start with a small product count and document flag ownership in the local ledger. |
| Editor integration order | Medium | Unified Move Relearner, Pokemon State Editor, and held-item catalog branches are not on `master`. | Add entitlement hook points first; wire editor UI only when the editor branch is selected for integration. |
| Economy coupling to BP | Low | Using Battle Frontier BP directly may create unwanted economy coupling. | Keep BP optional. Use an abstract progress / reward policy until balancing is decided. |

## Impact Notes

- `shop.c` is a sensitive shared UI path. The safest implementation leaves
  existing item shops behaviorally unchanged.
- `script_pokemon_util.c` is the safest creation reference, especially the
  parameterized gift path.
- Egg progress touches battle-end or challenge-clear flow only if the selected
  policy needs it. A pure vendor MVP can ship without battle hooks.
- Move and held-item edit entitlement affects Summary, party menu, relearner,
  and held-item catalog features. The first branch should expose a policy helper
  rather than directly scattering checks.

## Accepted Risks For First Slice

- Product UI may initially be text-only, with no Pokemon icon preview.
- BP-specific reward UI is deferred.
- Lv.50 normalization may be deferred if the first slice only proves purchase
  and delivery.
- Always-editable Egg-origin policy may be documented and stubbed until a safe
  per-mon origin marker is chosen.

## Open Questions

- Is a global product unlock table acceptable, or must entitlement follow the
  individual Pokemon forever?
- Are Eggs allowed to be sent to PC, or must they always occupy a party slot?
- Should bought normal Pokemon use default learnset moves or a product-specific
  move payload from the start?
