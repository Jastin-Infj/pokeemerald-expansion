# All Ability Slots Runtime MVP Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `master` `de310ef9eb`; upstream `expansion/1.15.2-96-gde310ef9eb` |
| Code status | Docs-only MVP plan |
| Provenance | Local project overlay |

## MVP

The first runtime branch should prove the rule without changing save layout:

- Add a guarded all-active ability mode.
- Build direct species-slot ability sets for battlers and party Pokemon.
- Apply the mode only during battle for the first branch.
- Convert a focused but representative subset of battle checks from singular
  ability equality to set predicates.
- Convert switch-in / end-turn trigger dispatch to iterate active abilities for
  selected test abilities.
- Disable or no-op Ability Capsule / Patch under all-active mode until a product
  decision is made.
- Show all active abilities somewhere reliable, preferably Summary first.

## Non-Goals

- Do not widen `abilityNum` or change the boxed Pokemon save layout.
- Do not implement more than three ability slots.
- Do not rebalance species ability tables in the MVP branch.
- Do not change field lead ability behavior in the MVP branch.
- Do not fully redesign party / PC / team viewer layouts in the first battle
  proof. Summary can be the first complete UI surface.
- Do not merge runtime source into `master`; use a fresh `feature/*` branch.

## Recommended Implementation Steps

| Step | Files | Notes |
|---|---|---|
| 1 | `include/config/battle.h` or local runtime rule owner | Add a default-off guard such as `B_ALL_ABILITY_SLOTS_ACTIVE`. A runtime option can wrap this later. |
| 2 | `include/pokemon.h`, `src/pokemon.c`, `include/battle_util.h`, `src/battle_util.c` | Add small ability-set structs and helpers. Use direct `GetSpeciesAbility()` slots, skip `ABILITY_NONE`, dedupe, preserve order. |
| 3 | `src/battle_util.c` | Add `BattlerHasAbility`, `IsAbilityOnField` set-aware variants, and a per-ability suppression / Mold Breaker check. Keep `GetBattlerAbility()` as a primary compatibility function. |
| 4 | `src/battle_script_commands.c`, `data/battle_scripts_1.s` | Convert `jumpifability` to set-aware behavior and audit copy / swap / overwrite commands behind the guard. |
| 5 | `src/battle_switch_in.c`, `src/battle_end_turn.c`, `src/battle_move_resolution.c` | Convert selected trigger families to iterate active abilities in stable slot order. Start with switch-in weather / Intimidate-style tests and Magic Guard / Soundproof predicate tests. |
| 6 | `src/party_menu.c`, `src/item_use.c` | Under all-active mode, make Ability Capsule / Patch fail cleanly or select display-only primary slot if that policy is chosen. |
| 7 | `src/pokemon_summary_screen.c` | Add a Summary display mode that lists active abilities and descriptions. This is the minimum UI evidence before deeper party / PC work. |
| 8 | `src/battle_ai_main.c`, `src/battle_ai_util.c`, `src/battle_ai_switch.c` | Add ability-set-aware AI helpers. In the MVP, at least immunities, priority, speed, trapping, and obvious switch-in threats must not stay singular. |
| 9 | `test/battle/ability/`, `test/pokemon.c`, focused existing ability tests | Add tests for set enumeration, multiple predicates, multiple switch-in triggers, suppression, Ability Shield, and one ability-changing move. |

## Current Contract

When all-active mode is enabled:

- a non-Egg Pokemon's natural active ability set is all non-`ABILITY_NONE`
  direct ability slots on its current species;
- hidden ability slot 2 is included in the battle active set;
- duplicate abilities count once;
- slot order is deterministic: slot 0, slot 1, hidden slot;
- `abilityNum` remains stored and can still provide a primary display /
  compatibility slot;
- singular legacy APIs may continue returning the primary ability until their
  call sites are migrated;
- a suppressed or bypassed ability should be removed from the effective set for
  that check, not by changing the saved mon.

Mega Evolution contract:

- Mega Evolution overlays the Mega target species ability slots on top of the
  base species ability slots;
- the active set can therefore reach six entries before duplicate removal;
- duplicate abilities across base and Mega species still count once;
- Mega overlay behavior is part of this mode's identity, not a bug-compatible
  replacement of the base species active set.

Field contract:

- field lead ability checks stay single-ability in the MVP;
- later global all-slot behavior must be a separate feature / revision.

Recommended temporary override contract for the first implementation:

- `overwrittenAbility` creates a one-ability active set while it is present;
- Trace / Role Play / Entrainment / Worry Seed / Simple Beam / Receiver copy or
  apply one ability, not all natural abilities;
- Skill Swap swaps the current primary / override ability and then both battlers
  use one-ability override sets;
- switching out or battle cleanup follows existing volatile reset behavior.

This override contract is intentionally conservative. It avoids needing a saved
or volatile array of copied abilities in the first branch, while still making the
natural three-slot rule work for ordinary battlers.

## UI Contract

Minimum UI:

- Summary must show the full active set, not only `abilityNum`.
- If space is tight, show the three names as a compact list and use cursor /
  page input to show one description at a time.
- Battle ability popups should still show one ability per trigger. If multiple
  switch-in abilities trigger, they should appear in slot order and avoid
  duplicate popups for duplicate slots.

Deferred UI:

- Party menu quick cards.
- Pokemon Storage detail pane.
- Pre-Battle / In-Battle Team Viewer.
- Debug mon editor ability selector.

## Future Work

- Balance pass for species with strong ability combinations.
- Dedicated ability / trainer balance feature for this alternate battle ruleset.
- Optional species allowlist / denylist for all-active mode.
- Runtime option or Champions facility-only enablement.
- Better AI scoring for three-ability combinations.
- Ability Capsule / Patch redesign.
- Multi-ability popup grouping if sequential popups become too noisy.

## Open Questions

- Should Primal Reversion, Ultra Burst, and non-Mega form changes use the Mega
  overlay rule or current-species-only ability sets?
- Should Ability Capsule / Patch become display-slot selectors, hidden-slot
  progression items for non-battle systems, or simply fail in this mode?
