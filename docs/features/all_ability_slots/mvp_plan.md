# All Ability Slots Runtime MVP Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-29 |
| Baseline | `master` `4e48ff993f` |
| Code status | MVP implemented on source shelf #60 and adopted into `integration/runtime-dev-20260529` |
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
| 1 | `include/config/battle.h` or local runtime rule owner | Add a guarded config such as `B_ALL_ABILITY_SLOTS_ACTIVE`. A runtime option can wrap this later, and the implementation branch can choose whether the feature PR build defaults on or off. |
| 2 | `include/pokemon.h`, `src/pokemon.c`, `include/battle_util.h`, `src/battle_util.c` | Add small ability-set structs and helpers. Use direct `GetSpeciesAbility()` slots, skip `ABILITY_NONE`, dedupe, preserve order. |
| 3 | `src/battle_util.c` | Add `BattlerHasAbility`, `IsAbilityOnField` set-aware variants, and a per-ability suppression / Mold Breaker check. Keep `GetBattlerAbility()` as a primary compatibility function. |
| 4 | `src/battle_script_commands.c`, `data/battle_scripts_1.s` | Convert `jumpifability` to set-aware behavior and implement slot-local copy / swap / overwrite commands behind the guard. |
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

Form-change contract:

- Mega Evolution uses the Mega species' three direct ability slots after the form
  change;
- base species abilities do not continue as an overlay after Mega Evolution;
- the natural active set remains three species slots before duplicate removal;
- Primal Reversion, Ultra Burst, and other form changes should follow the same
  current-form replacement rule unless a later feature deliberately changes that.

Field contract:

- field lead ability checks stay single-ability in the MVP;
- later global all-slot behavior must be a separate feature / revision.

Slot-local ability-change contract:

- ability-changing effects alter one slot, not the whole active set;
- the default operation slot is the battler's representative `abilityNum`;
- Skill Swap swaps the same slot index between attacker and target;
- Trace, Role Play, Doodle, Entrainment, Receiver, and Power of Alchemy copy one
  corresponding slot;
- Worry Seed and Simple Beam overwrite one target slot, not all three slots;
- switching out or battle cleanup follows existing volatile reset behavior.

This contract is intentionally bounded. It avoids all-set rewrites such as
turning every slot into Insomnia, while preserving the tactical value of ability
manipulation. It likely requires slot-indexed battle-only override state rather
than the existing singular `overwrittenAbility`.

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

## Integration Note 2026-05-29

The runtime integration branch adopted the MVP source after #47 / #48 / #54 /
#51 / #57. Unlike the feature shelf's final `TRUE` build, integration defaults
`B_ALL_ABILITY_SLOTS` to `FALSE` so normal ROM and full `check` behavior remain
single-ability until a later Champions facility or config explicitly enables the
mode. The focused All Ability tests still force `TRUE`, and the debug menu
`Party` -> `All Ability...` routes force the mode for manual validation.

## Open Questions

- Should Ability Capsule / Patch become display-slot selectors, hidden-slot
  progression items for non-battle systems, or simply fail in this mode?
- Should Trace ever choose a random / visible opponent slot, or should it always
  use representative-slot copying in this mode?
