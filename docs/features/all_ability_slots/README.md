# All Ability Slots Runtime

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-24 |
| Baseline | `master` `0407f6daf7` |
| Code status | Runtime implementation validated locally on feature branch |
| Provenance | Local project overlay, source read on current `master` |

## Status

Status: Implementing on `feature/all-ability-slots-runtime-20260523`.

This feature is the proposed runtime rule where every legal species ability slot
is active at once: slot 0, slot 1, and the hidden slot. The current engine stores
and evaluates a single active ability, so this is a cross-cutting battle and UI
refactor rather than a data-only change.

## Goal

- Make Pokemon behave as if all three species ability slots are active
  simultaneously in battle.
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
- Scope the first implementation to battle runtime behavior. Field lead ability
  behavior should remain the upstream single-ability rule until a later feature
  explicitly opts into global all-slot behavior.
- Hidden ability slot 2 is part of the battle active set when the mode is
  enabled. `abilityNum` remains a representative / primary slot only.
- Runtime validation currently exposes Pattern A-T debug battles under
  `Party` -> `All Ability...`, including Chlorophyll, Trace, Conkeldurr
  offensive stacking, Durant Hustle, and Lightning Rod / Storm Drain
  redirection routes, plus modifier-stack / defensive-modifier / partner-
  modifier routes.
- Enumerate species slots with `GetSpeciesAbility()`, not repeated
  `GetAbilityBySpecies()`, because `GetAbilityBySpecies()` intentionally falls
  back from empty slots to another valid ability.
- Dedupe identical abilities before evaluation. For example, if a custom species
  table is effectively `Intimidate / Intimidate / Defiant`, Intimidate should
  trigger once and Defiant should still be active.
- Treat Ability Capsule and Ability Patch as impacted UI / item policy. In an
  all-active world they no longer change battle behavior unless repurposed.
- Ability-changing battle effects should be slot-local, not whole-set
  replacements. Skill Swap, Trace, Role Play, Entrainment, Worry Seed, Simple
  Beam, Receiver, and similar effects should change only one corresponding slot
  in the active set.
- Treat Mega Evolution and other form changes as current-form replacement for
  ability purposes. After Mega Evolution, the active set is the Mega species'
  three direct slots, not base slots plus Mega slots.
- Keep balance tuning separate from the mechanics branch. Ability buffs /
  nerfs, species-slot table edits, and gym / trainer balance should be tracked
  by a later balance feature.
- Add balance-facing config switches for the two most disruptive categories:
  `B_ALL_ABILITY_SLOTS_MOLD_BREAKER` controls whether non-representative
  Mold Breaker / Teravolt / Turboblaze / Mycelium Might bypass target
  abilities, and `B_ALL_ABILITY_SLOTS_NEUTRALIZING_GAS` controls whether
  non-representative Neutralizing Gas suppresses other abilities.
- Summary stays in normal single-ability name / description mode while
  `B_ALL_ABILITY_SLOTS` is disabled. When the config is enabled, Summary shows
  direct slots `1`, `2`, and `H`, with the representative `abilityNum` slot
  marked by a right arrow.
- The current implementation covers a broad battle-modifier pass: base-power,
  Attack / Defense, final damage, STAB / Tera STAB, accuracy, priority,
  multi-hit, contact, powder-block, and partner modifier helpers now use
  all-slot-aware predicates for the major ability families. Obscure
  animation / form / item-edge paths remain on the manual regression list.

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
- Field lead ability all-slot behavior such as encounter modifiers, fishing,
  Cut, overworld poison, or Match Call.
- Adding more than three species ability slots.
- Replacing species ability data or importing external ability sets.
- Reworking Party / Status UI assets; this feature only records the required
  display changes and should coordinate with
  [Party / Status UI Overhaul](../party_status_ui_overhaul/README.md).

## Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Ability Audit](ability_audit.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)

## Open Questions

- Should `B_ALL_ABILITY_SLOTS` stay as a compile-time config only, or should a
  later Champions runtime rule toggle it per facility?
- Should Ability Capsule / Patch stay disabled in all-slot mode, or should a
  later UI pass repurpose them to choose the representative display slot?
- For Trace, should the default copied slot always be the tracer's representative
  slot, or should battle script support an explicit random / chosen slot later?
- Should Summary gain a dedicated per-ability description browser, or are compact
  active names enough for the first runtime branch?
