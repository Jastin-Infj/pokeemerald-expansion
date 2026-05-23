# All Ability Slots Runtime

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `master` `de310ef9eb`; upstream `expansion/1.15.2-96-gde310ef9eb` |
| Code status | Docs-only investigation / no runtime source changes |
| Provenance | Local project overlay, source read on current `master` |

## Status

Status: Investigating.

This feature is the proposed runtime rule where every legal species ability slot
is active at once: slot 0, slot 1, and the hidden slot. The current engine stores
and evaluates a single active ability, so this is a cross-cutting battle and UI
refactor rather than a data-only change.

## Goal

- Make Pokemon behave as if all three species ability slots are active
  simultaneously.
- Keep the existing save layout compatible by leaving `abilityNum` as a stored
  representative slot, not by widening per-Pokemon save data.
- Audit the battle engine, Ability Capsule / Ability Patch, Summary and party UI,
  AI, form change, Mega / Primal / Ultra Burst, and ability-copying moves before
  implementation.
- Provide an implementation path that can be guarded by a config flag or
  Champions runtime rule while upstream behavior remains available.

## Current Decision

- Do not modify `PokemonSubstruct3::abilityNum` or `BattlePokemon.abilityNum` in
  the first implementation. Both are 2-bit fields and already cover the current
  three slots.
- Do not make `GetBattlerAbility()` return "all abilities"; its return type is a
  single `enum Ability` and roughly 290 call sites currently assume one value.
- Add an ability-set helper layer first. New callers should ask questions such
  as "does this battler have ability X?" or "iterate active abilities for this
  trigger".
- Enumerate species slots with `GetSpeciesAbility()`, not repeated
  `GetAbilityBySpecies()`, because `GetAbilityBySpecies()` intentionally falls
  back from empty slots to another valid ability.
- Treat Ability Capsule and Ability Patch as impacted UI / item policy. In an
  all-active world they no longer change battle behavior unless repurposed.
- Treat Mega / form changes as a major audit point. Form checks currently pass a
  single ability into `GetBattleFormChangeTargetSpecies()` /
  `TryBattleFormChange()`, and the active ability cache is singular.

## Scope

### In Scope

- A source dependency map for current single-ability assumptions.
- A staged plan for battle helpers, AI, UI, and ability-changing move semantics.
- Policy notes for Ability Capsule / Ability Patch and current ability display.
- Risk notes for suppression, Mold Breaker, Neutralizing Gas, Ability Shield,
  Trace, Role Play, Skill Swap, Entrainment, Receiver, and form changes.

### Out of Scope

- Runtime source implementation on `master`.
- Balance tuning of every Pokemon after all slots become active.
- Adding more than three species ability slots.
- Replacing species ability data or importing external ability sets.
- Reworking Party / Status UI assets; this feature only records the required
  display changes and should coordinate with
  [Party / Status UI Overhaul](../party_status_ui_overhaul/README.md).

## Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)

## Open Questions

- Should all-active mode be a compile-time config, a runtime rule option, or a
  Champions-only facility flag?
- Should Ability Capsule / Patch be disabled, repurposed to choose a primary
  display slot, or used as a hidden-slot unlock item?
- For Trace / Role Play / Skill Swap / Receiver, should copied abilities replace
  the whole active set temporarily, or add to the natural three-slot set?
- Should UI show all active abilities everywhere, or show one primary slot with a
  Summary-only full list?
