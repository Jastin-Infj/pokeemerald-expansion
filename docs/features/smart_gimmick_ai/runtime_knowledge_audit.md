# AI Runtime Knowledge Audit

This document tracks the plan for turning move, ability, and item specifications into reusable AI runtime knowledge.

The goal is not to copy wiki text into the ROM. The goal is to read external specifications, verify them against the expansion runtime, and expose the result as small AI predicates that switching, move scoring, gimmick timing, and prediction code can share.

## Source Policy

- Use Pokemon Wiki and adjacent competitive / official references as source material for mechanics.
- Do not paste source prose into code or docs beyond short references.
- Prefer the expansion's existing data and config gates when they already encode the mechanic.
- Record any unresolved generation / config difference before changing runtime behavior.

Useful Pokemon Wiki entry points:

- [`カテゴリ:わざ`](https://wiki.pokemonwiki.com/wiki/%E3%82%AB%E3%83%86%E3%82%B4%E3%83%AA:%E3%82%8F%E3%81%96): lists move categories such as sound moves, dance moves, wind moves, biting moves, slicing moves, powder / spore moves, bullet moves, direct-contact moves, pulse moves, status moves, move-effect categories, combo moves, and ability categories that boost / reduce / nullify moves.
- [`わざの一覧`](https://wiki.pokemonwiki.com/wiki/%E3%82%8F%E3%81%96%E3%81%AE%E4%B8%80%E8%A6%A7): generation-specific move lists.
- [`へんかわざ`](https://wiki.pokemonwiki.com/wiki/%E3%81%B8%E3%82%93%E3%81%8B%E3%82%8F%E3%81%96): status move inventory.
- Individual move / ability / item pages for edge cases such as `よこどり`, `まるくなる`, `ころがる`, `いかり`, `ふういん`, `へんしん`, `おまじない`, `スキルスワップ`, and ability-changing moves.

## Existing Runtime Knowledge

The expansion already has a substantial move flag layer in `include/move.h` and `src/data/moves_info.h`.

Existing move predicates include:

- Contact / protection / reflection / stealing style flags:
  - `MoveMakesContact`
  - `MoveIgnoresProtect`
  - `MoveCanBeBouncedBack`
  - `MoveCanBeSnatched`
  - `MoveIgnoresSubstitute`
- Category tags used by abilities and items:
  - `IsPunchingMove`
  - `IsBitingMove`
  - `IsPulseMove`
  - `IsSoundMove`
  - `IsBallisticMove`
  - `IsPowderMove`
  - `IsDanceMove`
  - `IsWindMove`
  - `IsSlicingMove`
  - `IsHealingMove`
- Special target / state flags:
  - `MoveIncreasesPowerToMinimizedTargets`
  - `MoveIgnoresTargetAbility`
  - `MoveIgnoresDefenseEvasionStages`
  - `MoveDamagesUnderground`
  - `MoveDamagesUnderWater`
  - `MoveDamagesAirborne`
  - `MoveDamagesAirborneDoubleDamage`
  - `MoveThawsUser`
  - `MoveForcesPressure`
  - weather accuracy helpers for rain / sun.

This means the first pass should not invent a parallel "all move tag" table. The AI layer should reuse these flags and add only higher-level strategy predicates.

## Gap

The missing piece is not raw move tagging. The missing piece is a shared AI interpretation layer.

Right now, move, ability, and item mechanics are distributed across:

- `include/move.h` and `src/data/moves_info.h` for move flags.
- `src/battle_ai_main.c` for move scoring and bad-move checks.
- `src/battle_ai_util.c` for damage, status, and matchup predicates.
- `src/battle_ai_switch.c` for switch-out and switch-in logic.
- `src/battle_util.c`, `src/battle_move_resolution.c`, `src/battle_script_commands.c`, and `data/battle_scripts_1.s` for actual battle behavior.
- `src/battle_hold_effects.c` and item hold-effect data for item behavior.

Because of that distribution, a new runtime check should usually be a shared predicate such as "can this Pokemon ignore Taunt", "can this reserve punish a predicted utility move", or "does this move create board control", not a one-off hardcoded branch in every AI file.

## Runtime Knowledge Layers

Use these layers when adding knowledge.

### Move Shape

These are mostly already encoded by move flags:

- contact
- sound
- bullet / ballistic
- slicing
- punching
- biting
- pulse
- powder / spore
- dance
- wind
- healing
- explosion
- multi-hit
- charge / semi-invulnerable
- switch-out / pivot
- phazing
- priority
- protection / substitute / Magic Coat / Snatch / Mirror Move / Copycat / Sleep Talk / Encore bans

### Move Outcome

These need AI-friendly grouping by `effect`, additional effects, and battle-script behavior:

- direct damage
- fixed damage
- drain / recoil / crash recoil
- stat raise / stat drop / stat swap / stat copy
- non-volatile status
- volatile status
- confusion / infatuation / nightmare / curse / perish
- trapping / bind / block
- redirection
- field / weather / terrain / room / screen / veil / hazard
- ability change / ability suppression / ability copy / ability swap
- item change / item removal / item suppression
- type change / form change / transform
- move disable / Taunt / Torment / Encore / Imprison / Heal Block
- move call / copy / metronome-like behavior
- combo state, such as `Defense Curl` enabling stronger `Rollout` / `Ice Ball`.

### Ability Shape

Ability behavior needs a separate AI taxonomy because many abilities are not simple boosts.

High-value groups:

- move immunity: `Soundproof`, `Bulletproof`, powder immunity, status immunity, priority blocking, phazing blocking, redirection immunity.
- move boost: `Sharpness`, `Iron Fist`, `Strong Jaw`, `Mega Launcher`, `Punk Rock`, `Reckless`, `Technician`, `Sheer Force`, weather / terrain / type conversion boosts.
- damage / type immunity and absorption: water / electric / ground / fire absorb lines, `Flash Fire`, `Sap Sipper`.
- status interaction: `Guts`, `Quick Feet`, `Marvel Scale`, `Poison Heal`, `Magic Guard`, `Purifying Salt`, `Comatose`, `Good as Gold`, `Aroma Veil`, `Oblivious`.
- field control: weather setters, terrain setters, `Air Lock`, `Cloud Nine`, weather / terrain speed abilities.
- switch / positioning: trapping, escape, `Regenerator`, Intimidate-style entry pressure, `Emergency Exit`, `Wimp Out`, `Commander`.
- ability mutability: cannot be overwritten, cannot be suppressed, `Ability Shield`, `Mold Breaker`-style bypass, `Gastro Acid`, `Worry Seed`, `Skill Swap`, `Role Play`, `Entrainment`, `Simple Beam`, `Doodle`.

### Item Shape

Item behavior should be tagged around when it matters to AI:

- damage race: Focus Sash, Life Orb, Choice items, gems, type-boost items, metronome-style boosts.
- status prevention / cure / self-status: Mental Herb, Lum-like cures, Flame Orb, Toxic Orb.
- field / seed activation: terrain seeds, Booster Energy, Room Service, Air Balloon.
- contact / hit punishment: Rocky Helmet, Eject Button, Eject Pack, Red Card, Weakness Policy, berries.
- move-shape modifiers: Punching Glove, Covert Cloak, Utility Umbrella, Protective Pads.
- ability / item suppression edge cases: Klutz, Magic Room, Embargo, Ability Shield.

## AI Predicate Direction

Prefer predicates named around strategy, not around one source mechanic.

Examples:

- `AI_CanIgnorePredictedTaunt(battler)`
- `AI_MoveCreatesSpeedControl(move, battler, target)`
- `AI_MoveCreatesFieldControl(move, battler, target)`
- `AI_MoveBlocksOpponentPlan(move, battler, target)`
- `AI_AbilityCreatesBoardControl(ability, battler)`
- `AI_ItemCreatesStatusPlan(item, battler)`
- `AI_CanPunishPredictedUtilityMove(battler, target, incomingMove)`
- `AI_ReserveCreatesImmediatePressure(battler, partyIndex, target)`

This keeps runtime behavior explainable. The AI should not switch because "Taunt exists"; it should switch because a predicted Taunt creates a clean attacker entry and the active Pokemon cannot punish or ignore it.

## Priority Queue

1. **Already started:** predicted `Taunt` punishment and stay-in guards.
2. **Move denial cluster:** `Encore`, `Torment`, `Disable`, `Heal Block`, `Imprison`, `Throat Chop`, `Taunt` follow-up behavior.
3. **Reflection / stealing cluster:** `Magic Coat`, `Magic Bounce`, `Snatch`, `Substitute`, `Protect`, `Good as Gold`.
4. **Ability mutability cluster:** `Skill Swap`, `Role Play`, `Entrainment`, `Worry Seed`, `Gastro Acid`, `Simple Beam`, `Doodle`, `Mummy`, `Lingering Aroma`, `Ability Shield`, "cannot overwrite / suppress" abilities.
5. **Combo / latent state cluster:** `Defense Curl` + `Rollout` / `Ice Ball`, `Rage`, `Fury Cutter`, `Echoed Voice`, `Stockpile`, `Charge`, `Focus Energy`, `Lucky Chant`, `Laser Focus`.
6. **Board-control cluster:** weather, terrain, room, screens, veil, hazards, trapping, phazing, redirection, Follow Me / Rage Powder.
7. **Move-shape ability cluster:** `Soundproof`, `Bulletproof`, `Sharpness`, `Iron Fist`, `Strong Jaw`, `Mega Launcher`, `Punk Rock`, `Wind Rider`, `Wind Power`, `Dancer`.
8. **Status and item cluster:** cures, self-status, status-benefit abilities, Mental Herb, Covert Cloak, Protective Pads, Flame / Toxic Orb, terrain seeds.

## Implementation Rules

- Reuse `include/move.h` predicates before adding new flags.
- If a mechanic depends on config, read the same config used by battle resolution.
- If a mechanic can be bypassed by Mold Breaker-style effects, use the same sanitized ability path used by damage / move-resolution logic.
- If an item can be disabled by Klutz / Magic Room / Embargo, call an item-enabled predicate before treating the item as active.
- For doubles, evaluate the partner and both opposing slots before pivoting.
- For singles, stay conservative unless the active Pokemon is losing, pinned, or cannot convert pressure.
- Every new runtime heuristic needs at least one positive and one negative unit test.

## Open Audit Questions

- Which existing move flags are incomplete against the current Pokemon Wiki category lists when `B_UPDATED_MOVE_FLAGS` and `B_EXTRAPOLATED_MOVE_FLAGS` are enabled?
- Which status moves have battle-script-only special behavior that AI cannot currently see from `GetMoveEffect()` alone?
- Which abilities are currently modeled only in damage resolution but not in switching / prediction?
- Which item effects are currently modeled only after activation and not during AI planning?
- Which generation differences should be accepted as configured expansion behavior rather than copied from the latest official games?
