# Runtime Re-evaluation Spec

This spec defines how Smart Gimmick AI should re-evaluate battle knowledge before choosing a move, switching, or spending a gimmick.

The important point is order. Moves, abilities, items, field state, and gimmicks cannot be evaluated as separate flat lists because each layer can change the meaning of the next layer.

## Source Entry Points

Use Pokemon Wiki and the expansion runtime as the source pair:

- [`カテゴリ:わざ`](https://wiki.pokemonwiki.com/wiki/%E3%82%AB%E3%83%86%E3%82%B4%E3%83%AA:%E3%82%8F%E3%81%96)
- [`カテゴリ:とくせい`](https://wiki.pokemonwiki.com/wiki/%E3%82%AB%E3%83%86%E3%82%B4%E3%83%AA:%E3%81%A8%E3%81%8F%E3%81%9B%E3%81%84)
- [`もちもの`](https://wiki.pokemonwiki.com/wiki/%E3%82%82%E3%81%A1%E3%82%82%E3%81%AE)
- [`メガシンカ`](https://wiki.pokemonwiki.com/wiki/%E3%83%A1%E3%82%AC%E3%82%B7%E3%83%B3%E3%82%AB)
- [`Zワザ`](https://wiki.pokemonwiki.com/wiki/Z%E3%83%AF%E3%82%B6)
- [`ダイマックス`](https://wiki.pokemonwiki.com/wiki/%E3%83%80%E3%82%A4%E3%83%9E%E3%83%83%E3%82%AF%E3%82%B9)
- [`テラスタル`](https://wiki.pokemonwiki.com/wiki/%E3%83%86%E3%83%A9%E3%82%B9%E3%82%BF%E3%83%AB)

Pokemon Wiki should define the mechanic category. The expansion runtime should define the exact local behavior, generation gates, config gates, constants, and exceptions.

## Other Elements To Consider

Beyond move / ability / held-item categories, AI decisions need these inputs:

- Species, form, base stats, current stats, stat stages, HP ratio, max HP, typing, Tera Type, gender, level, and current form-change rules.
- Active battle format: singles, doubles, multi, inverse battle, raid-like behavior, trainer battle, wild battle, and debug fixtures.
- Current field: weather, terrain, Trick Room, Gravity, Wonder Room, Magic Room, screens, Aurora Veil, Mist, Safeguard, Tailwind, hazards, pledge effects, rooms, and side timers.
- Current battler state: non-volatile status, volatile status, Taunt, Encore, Torment, Disable, Heal Block, Leech Seed, perish count, substitute, semi-invulnerable state, protection chain, flinch risk, trapped state, chosen move, and known PP.
- Party / reserve state: alive count, last-Pokemon state, switch-in hazards, Wish / Healing Wish / Lunar Dance / Revival Blessing state, reserve ability / item / move roles, and whether a reserve can create immediate pressure.
- Knowledge state: known moves, predicted move, predicted switch, known ability, known item, illusion / transform uncertainty, and whether omniscient AI flags are active.
- Resource state: whether Mega / Z / Dynamax / Tera has already been spent, candidate-specific eligibility, item consumed state, choice locks, and setup investment already made.
- Trainer policy: AI flags, environment flags, smart-gimmick flags, scripted trainer intent, ace handling, difficulty level, and whether the branch is testing a specific mechanic.

## Information Policy

Hidden information must be explicit.

- Full opponent information may be used only when the relevant full-information / omniscient AI flag is active.
- Without that flag, the AI should use observed information plus inferred information.
- Inferred information should carry source and confidence, not be treated as confirmed truth.
- Inference sources should include local catalog JSON, generated trainer-party catalogs, Champions ranking / usage data, VGC usage and result data, species-fit heuristics, known trainer archetypes, and moves already revealed in the current battle.
- Early implementation may use broad catalog-derived assumptions, but the long-term model should prefer weighted probabilities such as "this species commonly carries Protect", "this archetype often has Fake Out", or "this Tera Type is common on this set".

Recommended knowledge tags:

- `observed`: directly revealed in the current battle.
- `omniscient`: visible only because a full-information AI flag is enabled.
- `catalog_inferred`: inferred from local catalog / JSON set data.
- `usage_inferred`: inferred from Champions / VGC usage data.
- `species_fit`: inferred from species role, stats, typing, ability, and item fit.
- `trainer_archetype`: inferred from trainer class, party style, debug fixture, or scripted battle intent.
- `low_confidence`, `medium_confidence`, `high_confidence`: confidence buckets used by prediction and risk logic.

The AI may act on inferred information, but high-risk actions should require either high confidence or an aggressive / read-oriented style profile.

## Catalog Automation Policy

Large-scale collection should be staged through generated catalogs instead of one-off AI branches.

The first local generator is:

```sh
cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog --pretty
```

This generator reads the current expansion source and emits JSON for moves, abilities, hold effects, items, gimmick resource policy, and a summary. It is the base layer for later Pokemon Wiki / VGC / Champions / trainer-party adapters.

The merge order should be:

1. local expansion catalog: constants, config-gated move fields, status Z-Move effect fields, AI knowledge flags, and item hold effects.
2. checked-in trainer-party / PartyGen catalogs: trainer-owned sets and debug fixtures.
3. external category audits: Pokemon Wiki categories and official mechanic references.
4. usage / tournament adapters: VGC, official singles events, Champions ranking / usage data, and player result data.
5. observed battle history: revealed moves, items, abilities, switches, gimmick timing, and target choices.

Every merged fact needs a source tag and confidence bucket. The catalog may suggest that a species likely has `Protect`, `Fake Out`, a common Tera Type, or a common setup move, but the runtime must still distinguish that from observed or omniscient information.

## Resource Priority Policy

Mega / Ultra Burst is not the same kind of resource as Z-Move, Dynamax, or Tera.

- Mega / Ultra Burst usually raises output and should normally be used when the post-form state is better.
- Mega / Ultra Burst can still be delayed when the pre-form ability has immediate value, such as weather denial, scouting, or a setup turn with no pressure.
- Dynamax is usually the highest-value spend because it changes HP, damage race, disruption resistance, and board effects at the same time.
- Tera is usually the next strongest spend because it changes defensive and offensive typing while preserving normal move choice.
- Z-Move is usually below Tera as a one-shot spend, but can outrank Tera when it converts a KO, breaks a trap, bypasses accuracy risk, or gives a decisive status Z effect. Status Z effect families are cataloged separately so arbitration can compare, for example, one-shot redirection, HP recovery, replacement healing, critical boost, and stat reset effects without hardcoding each source move.
- Combined environments should compare resources rather than evaluating each gimmick in isolation.

Default arbitration order when multiple spends look similarly good:

1. preserve all resources if the current board is already winning.
2. prefer Dynamax if HP / Max Move board control changes the next two or three turns.
3. prefer Tera if a type flip changes immediate survival or creates a stable sweep line.
4. prefer Z-Move if it gives immediate KO / trap-break / status payoff.
5. use Mega / Ultra Burst when post-form value is better and pre-form value is not needed.

This priority is a policy default, not an absolute rule. The reason code must explain any exception.

## Prediction Policy

Next-turn prediction is required, not optional.

The AI should predict:

- opponent move choice.
- opponent target choice.
- opponent switch.
- `Protect` / `Detect` / similar protection.
- `Fake Out` and other first-turn / priority disruption.
- `Taunt`, `Encore`, `Disable`, `Trick Room`, `Tailwind`, redirection, setup, recovery, phazing, and status pressure.
- opponent gimmick use, especially Tera and Dynamax timing.

Predictions should be weighted by information source and confidence. A likely `Protect` from a VGC-style set should not be treated the same as a confirmed `Protect` already revealed in battle.

`AI_FLAG_READ_PLAYER_MOVE` may treat already-confirmed player commands as a higher-confidence source than heuristic prediction. The opponent controller waits for all live player-side commands, then recomputes action / move choice against the confirmed command buffer. Confirmed moves override heuristic incoming-move prediction, confirmed switches override heuristic switch-in prediction, and selected player-side defensive gimmicks are included in damage calculation. If no current confirmed command is available, the AI may fall back to the current battle's action log for that battler's most recent selected move before falling back to last-used move history. This began as a debug / gauntlet audit mode, but the current branch policy is to auto-apply it to non-link NPC trainer AI at runtime. Recorded and link battles remain excluded so explicit replays and test-runner AI flag scripts stay deterministic. If this mode is used for player-style learning, the observed commands should be logged with source tags and kept separate from generic VGC / single-battle priors so the model does not overfit to one player's habits.

Runtime logging uses two in-ROM EWRAM ring buffers. `gBattleActionLog` stores 128 ×
28-byte confirmed-command / resolved-switch entries plus an 8-byte header (3,592
bytes total), including correction markers for invalid switch-in fallback and links
from AI actions to plan ID / candidate rank. `gBattleAiTraceLog` stores 32 × 32-byte
plans, 256 × 28-byte candidates, 96 × 96-byte board snapshots, and a 16-byte header
(17,424 bytes total). Trace schema version 5 is identified by magic `0xA15C`.

Persistent host export is implemented by
`tools/mgba_live/battle_action_log_export.lua`; the startup autosave script invokes
the same exporter for the latest non-empty action log. With a matching ROM / map and
trace signature, JSON schema `pokeemerald.battle_action_log.v5` includes
`ai_trace_header`, `ai_plans`, `ai_candidates`, and `ai_board_snapshots` alongside
`entries`. A missing or mismatched trace signature intentionally produces
`pokeemerald.battle_action_log.v4` with action-log data only. Normal mGBA play still
cannot write host files directly from the ROM, so manual export or autosave must run
while the battle state is available.

Recommended prediction tags:

- `predicted_attack`
- `predicted_protect`
- `predicted_switch`
- `predicted_fake_out`
- `predicted_redirection`
- `predicted_speed_control`
- `predicted_setup`
- `predicted_recovery`
- `predicted_status`
- `predicted_taunt`
- `predicted_phazing`
- `predicted_tera`
- `predicted_dynamax`

## Current Joint Board Arbitration

Central doubles arbitration is implemented for the conservative board-search
envelope. The opponent controller waits for both player commands, takes one immutable
board snapshot, generates move / switch / modeled gimmick-plus-move candidates for
both AI battlers, and chooses one side-wide action pair. Both opponent controller
callbacks then consume that shared plan, so partner choices cannot independently
select incompatible targets, switches, or once-per-trainer resources. With full
knowledge, the snapshot derives prospective Mega / Ultra profiles from real active
and reserve party data and retains future per-mon gimmick eligibility. A searched
switch can therefore expose a reserve Mega, Ultra Burst, Z-Move, or Tera candidate on
a later ply; Ultra Burst preserves its independently legal later Z action.

The runtime joint path is eligible only when all of the following are true:

- the battle is an ordinary non-link 2v2 NPC trainer double, with exactly two live,
  commandable non-Commander player battlers and exactly two live non-Commander AI
  battlers, and the trainer has no item command inventory.
- both AI battlers have `AI_FLAG_READ_PLAYER_MOVE`, `AI_FLAG_DOUBLE_BATTLE`,
  `AI_FLAG_OMNISCIENT`, and `AI_FLAG_SMART_MON_CHOICES`, and neither has
  `AI_FLAG_ATTACKS_PARTNER`.
- both player commands are confirmed move or switch actions. Trainer-item / run
  actions, forced multi-turn moves, recharge, asymmetric replacement boards, multi,
  partner, Palace, Safari, recorded playback, and other excluded formats stay on the
  legacy path.

The planner executes actions in configured runtime order. Pre-Gen-7 Mega / Ultra
rules sort the initial turn from pre-form profiles, while Gen 7+ sorts from the
activated profile. Pre-Gen-8 rules preserve that initial remaining order; Gen 8+
recalculates remaining move actions after the first move and later board changes.
Tie discovery follows the same boundary and never reintroduces an actor that already
acted. The planner enumerates all 16 supported single-target damage rolls, configured
critical branches, and only genuinely tied priority / effective-Speed order
permutations with exact integer weights. Protect,
secondary-effect, and thaw branches share that frontier. Every raw branch is applied
before equivalent post-turn boards have their weights added; transient turn-result
events are excluded from the merge key. A narrow single-hit proof may group equal
raw-damage classes before whole-turn replay only when exactly one ordinary
single-target hit exists and every other action is proven not to change its inputs.
Multiple damage actions, special damage into a Geomancy user, Fairy damage with a
switchable Fairy Aura, and other unsafe combinations retain raw enumeration. The
32-outcome bound is checked after the post-board merge. Independently, one joint turn may perform at most 4,096 exact outcome
applications; reaching that CPU-safety ceiling fails closed even if many raw
branches would merge to fewer boards. The planner includes forced
replacements and scores realized board pressure rather than only raw damage. It
completes the standard depth-three search before a selective depth-five extension and
advances in bounded per-frame slices. The transposition table has 16 entries, and the
heap runtime arena occupies 16,248 bytes, below 16 KiB. It is released after all
battler commands are committed, and allocation failure is a normal legacy-fallback
condition. Exact enumeration owns a separate 2,596-byte static EWRAM scratch under
the runtime's single-job/non-reentrant contract so controller-task stacks stay small.
Joint-turn validation owns a second 540-byte board scratch under the same contract;
final object-code local stack allocations are 240 normal / 124 debug / 244
`TESTING` bytes for `AiSim_EnumerateOutcomes()` and 56 / 28 / 56 bytes for
`AiSim_CheckJointTurn()`, with another 36 saved-register bytes in each function.

The immutable simulator fails closed rather than approximating an unmodeled line.
Unsupported snapshot knowledge, field / volatile / residual state, move effects,
damage modifiers, redirection, substitute, switch-in effects, incomplete root
generation, reaching budget before any usable depth-3 root completes, or an invalid
chosen pair discards the joint result and runs the existing evaluator. A limit hit
after a usable root completes preserves the deepest completed result. In particular:

- already-active Dynamax, or any generated newly selected Dynamax / Max Move
  candidate, makes the whole joint root set unsupported until the compact action
  carries exact base-derived Max power and effect state.
- prospective Mega / Ultra forms require a valid known transformed profile and a
  modeled passive ability. Full-knowledge profiles are derived from the copied party
  mon, target species, recalculated stats, types, and ability; unmodeled entry effects
  such as Intimidate or weather setters still fail closed.
  The unused third profile type is `TYPE_MYSTERY`; forced replacement reads the
  incoming member's persistent Mega / Ultra state, not the outgoing slot's flags.
  Test-only forced abilities are reapplied to prospective transformed profiles for
  harness parity without changing production ability resolution.
- Tera requires a valid supported Tera Type. Stellar and unknown types fail closed,
  including an already-active Stellar user and a persistently Stellar-Tera reserve
  entering through an ordinary switch or forced replacement.
- Z-Moves are admitted only when their converted move and effects are represented by
  the simulator.
- Moves flagged `cantUseTwice`, `ignoresTargetAbility`, or
  `ignoresTargetDefenseEvasionStages` fail closed, including signature Z variants,
  until repeat legality and the alternate damage path are represented.
- damaging spread moves, multi-hit damage, and pre-Generation-3 critical rules fail
  closed because their independent target / strike RNG is not represented by the
  compact per-actor outcome key. A frontier with more than 32 distinct post-turn
  boards after merging, or a turn reaching 4,096 exact outcome applications, is also
  unsupported. Encountering any such
  legal action makes the side-wide root set incomplete rather than removing only that
  action.
- targeted Prankster status effects represented by the simulator obey grounded
  Psychic Terrain and the configured Gen 7+ Dark immunity. Dark checks use the
  target's current simulated type, so Dark Tera blocks and Terastallizing away from
  Dark does not.
- a confirmed Fake Out is a composed-turn interaction rather than an atomic root
  illegality. Candidate-specific legacy checks account for prospective forms,
  abilities, typing, Speed, Sheer Force, ability suppression / Ability Shield,
  priority blocking, flinch immunity, and executable Quick Guard. The joint root
  retains Fake Out provenance while deferring the confirmed-flinch rejection so a
  paired Quick Guard or earlier ally Fake Out can answer it. With no such answer,
  exact turn application still flinches and skips the victim action.
- Tailwind uses the configured pre-Gen-5 three-turn or Gen-5+ four-turn duration.
  Selecting it again while already active is a deterministic no-op that spends PP,
  does not refresh the timer, and does not emit a new field-change event.

`Party -> Joint Trace Double` is the focused runtime acceptance fixture. Both sides
are deliberately asymmetric, and explicit `Party Size` lines are omitted so the
listed player two / AI three members keep their order. The player leads Intimidate
Arcanine with only Tailwind and Power Herb Sturdy Skarmory with only Geomancy. The
AI leads a level-7, zero-Attack-IV Scizorite Technician Scizor with only Bullet
Punch and Focus Sash Prankster Whimsicott with only Tailwind, keeps Power Herb Fairy
Aura Xerneas with only Geomancy in reserve, and has Mega Ring access. After Tailwind and Geomancy are
confirmed, a valid v5 export must contain a plan flagged `joint` and
`deepest_complete_used` (not `legacy_evaluator`), a chosen candidate flagged `joint`
and `chosen`, nonzero candidate scores, reviewable Mega and switch roots, and both
opponent action entries linked to the same plan ID and candidate rank. The trace proves
the live candidate and board path. The bulky, resisted selected target keeps this
planner acceptance deterministic enough to complete depth 3 under production
budgets; focused C regressions remain the direct evidence for raw 16-roll / critical /
Speed-tie weights because v5 does not export that frontier.

## Board Advantage Policy

Damage maximum is not the primary objective. The primary objective is board advantage.

Singles can often be evaluated like a forced endgame line. Doubles should be evaluated as board construction: pressure, pins, partner safety, Speed control, and whether the opponent is forced into bad options.

The AI should recognize positions where leaving an opposing Pokemon alive is better than taking a KO. In doubles, an opposing low-HP Pokemon with poor pressure can become a pinned slot. Ignoring that slot and attacking the other slot can create a 2-vs-1 structure while the opponent hesitates to switch or loses tempo if they do.

Recommended board tags:

- `tempo_gain`: action improves next-turn control even without maximum damage.
- `tempo_loss`: action gives the opponent a free turn, free switch, or free setup.
- `pin_created`: opponent has a Pokemon that is threatened and hard to move.
- `pin_maintained`: current action keeps an opponent pinned.
- `ignore_red_hp_slot`: low-HP opponent can be ignored because it has weak pressure.
- `two_vs_one_pressure`: one opposing slot is effectively neutralized, letting both AI slots pressure the other.
- `force_protect`: opponent is likely forced to protect or lose a key Pokemon.
- `force_switch`: opponent is likely forced to switch or lose role value.
- `stranded_target`: opponent wants to switch but is punished for doing so.
- `partner_cover`: partner can handle the immediate threat, so the active can create board value.
- `board_flip`: action changes losing board to neutral or winning board.
- `future_checkmate`: action creates a forced win line over the next one to three turns.

## Near-Term Checkmate / Comeback Policy

The AI may lower reliability thresholds only when the board is close to decided against it. This is not an HP threshold. The trigger should be a two-to-three-turn loss clock, such as trapped or no-reserve Perish Song, known same-turn KO with no reserve or visible opposing setup pressure, a selected setup move that will make the next turn unwinnable, or a board where the opponent can force the end before the AI can create stable damage.

Clean lines come first. If the AI can KO, likely KO, win the short damage race with ordinary damage, switch to a safe answer, or spend a gimmick to stabilize, it should prefer that line over hax fishing. OHKO moves, flinch chance, paralysis, freeze / frostbite, critical-hit fishing, and low-accuracy status are fallback outs for positions where stable play no longer has a credible route.

Singles and doubles should be evaluated separately:

- Singles comeback outs are mostly direct: damage, Protect payoff, switch, flinch, status, OHKO, or crit.
- Doubles comeback outs include board stops and order changes: Fake Out-style flinch, Taunt, Tailwind, Trick Room, Icy Wind-style Speed drops, Throat Chop / sound denial, and partner-assisted repositioning.
- Deliberate ally sacrifice is legal only when the ally is role-complete or the next board is meaningfully better, the move creates opposing-side pressure, and a reserve can use the opened slot.
- Ally-targeted Speed manipulation such as Scary Face under Trick Room needs a turn-order and survival check. It should not be treated as generally useful just because it changes Speed.
- Soundproof switching is a switch-layer answer to sound moves and Perish Song. It should be considered alongside switch-in punishment, trapping, and whether the current active still has a better direct out.

Recommended comeback tags:

- `desperation_comeback`
- `clean_damage_preferred`
- `hax_out`
- `ohko_fish`
- `flinch_fish`
- `crit_fish`
- `speed_order_flip`
- `sound_denial`
- `ally_sacrifice_board_reset`
- `commander_slot_correction`

## Reserve Value Policy

Reserve value must be evaluated every turn.

- A Pokemon can be sacrificed if its role in the current battle is finished.
- A Pokemon should be preserved if it is still needed to answer an opposing reserve, weather / terrain mode, Trick Room mode, priority endgame, or gimmick line.
- Cosmetic ace handling should not override battle value. Do not preserve a final "ace" only because it looks like an ace.
- Real endgame roles are valid: for example, a last-mon cleaner, Supreme Overlord payoff, Last Respects payoff, or a matchup-specific win condition.

Recommended reserve tags:

- `role_complete`
- `sackable`
- `must_preserve`
- `reserve_checks_unseen_threat`
- `reserve_enables_mode`
- `reserve_breaks_mode`
- `reserve_is_win_condition`
- `reserve_creates_board_control`
- `reserve_only_safe_switch`
- `reserve_not_worth_hazards`

## Risk Style Policy

AI style should be separated from knowledge.

Recommended style profiles:

- `stable`: prefers reliable board value, avoids low-confidence reads and unnecessary accuracy risk.
- `aggressive`: accepts damage-race and tempo risk to force progress.
- `high_variance`: accepts low-accuracy, secondary-effect, flinch, crit, or setup gambles when behind.
- `read_oriented`: acts on predicted Protect / switch / Tera / Fake Out / setup when confidence is high enough.

These profiles should adjust thresholds, not replace legality or core board evaluation.

Examples:

- `stable` may take a guaranteed 2HKO and preserve Tera.
- `aggressive` may Tera now to force a KO and deny the opponent's next move.
- `high_variance` may use a low-accuracy move if behind and no stable line exists.
- `read_oriented` may double the partner slot while leaving a red-HP pinned slot alive.

## Reason Code Policy

Every non-obvious AI action must emit a reason code.

This is mandatory because the AI is not optimizing only immediate damage. Debugging and tuning require knowing whether a move was chosen for KO conversion, board advantage, resource preservation, prediction, or reserve value.

Recommended reason tags:

- `ko_conversion`
- `damage_race`
- `low_accuracy_stabilized`
- `desperation_comeback`
- `clean_damage_preferred`
- `hax_out`
- `defensive_type_flip`
- `offensive_stab_gain`
- `max_move_board_control`
- `max_move_defensive_timing`
- `status_z_payoff`
- `preserve_dynamax`
- `preserve_tera`
- `preserve_pre_mega_ability`
- `mega_output_gain`
- `predicted_protect_read`
- `predicted_switch_read`
- `predicted_fake_out_block`
- `predicted_taunt_pivot`
- `pin_created`
- `ignore_red_hp_slot`
- `two_vs_one_pressure`
- `reserve_preserved`
- `sack_role_complete`
- `ally_sacrifice_board_reset`
- `commander_slot_correction`
- `config_rule_applied`
- `script_edge_case`

Reason tags should be reusable across singles and doubles. The display / logging layer can choose how much of this is surfaced.

## Evaluation Order

Use this order when adding deeper runtime logic:

1. **Build the raw candidate**
   - Candidate can be a move, switch, Mega / Ultra Burst, Z-Move, Dynamax, Tera, or a combined "use gimmick plus selected move" action.
   - Reject impossible candidates early: missing resource, invalid item, invalid form, disabled move, no target, no living reserve, or environment flag mismatch.

2. **Normalize known battle context**
   - Resolve format, side, partner, opposing slots, field status, weather, terrain, room effects, side statuses, hazards, and current AI flags.
   - In doubles, keep partner and both opposing slots in scope. A move or switch can be bad into one slot and correct because of partner pressure or field control.

3. **Resolve the active Pokemon state**
   - Determine effective species / form, current typing, Tera Type, current ability, held item / hold effect, item-enabled state, stats, stat stages, HP state, and status state.
   - Apply `Mold Breaker`-style ability bypass and `Ability Shield` rules when the evaluation concerns an opposing move or ability interaction.

4. **Apply gimmick transformation candidates**
   - **Mega / Ultra Burst:** re-evaluate target-form ability, typing, stats, Speed, damage, defensive race, and whether pre-Mega ability value should be preserved.
   - **Z-Move:** re-evaluate base-move category, damaging Z power, status Z effect, item resource, target legality, accuracy bypass value, and whether the Z-Move breaks a trap / KO / damage-race threshold.
   - **Dynamax / Gigantamax:** re-evaluate HP multiplier, Max Move type, Max Move power, Max Move side effect, G-Max unique effect, Fake Out / phazing immunity value, target spread, and three-turn resource timing.
   - **Tera:** re-evaluate defensive type, offensive STAB, Tera Blast behavior, Adaptability / type-changing ability interactions, weather / terrain amplification, and whether Tera should be preserved for a later matchup.
   - Combined environments must evaluate all eligible gimmicks as mutually exclusive resource candidates unless the local runtime explicitly permits a combination.

5. **Recompute move knowledge after transformation**
   - Base move shape does not always equal transformed move shape.
   - Recompute type, category, power, target, spread, contact, sound, ballistic, powder, slicing, punching, biting, pulse, dance, wind, healing, protection, Magic Coat, Snatch, ability-control, move-denial, and combo-state flags after gimmick conversion.
   - For Dynamax and Z-Moves, use the converted move's runtime behavior, not only the source move's tags.

6. **Recompute ability knowledge**
   - Read ability category flags after form / Mega / Neutralizing Gas / Gastro Acid / Skill Swap / Role Play / Entrainment / Doodle / Worry Seed / Simple Beam effects.
   - Recheck move immunity, power, damage-race, status, field-control, positioning, ability-control, stat-control, item-control, priority, and form-state categories.

7. **Recompute item knowledge**
   - Read item / hold-effect categories after item-enabled checks, Magic Room, Klutz, Embargo-like effects, item consumption, Knock Off, Trick / Switcheroo, Fling, and non-consumable local rules.
   - Recheck damage race, defensive race, stat control, Speed control, recovery, status cure, self-status, field duration, contact punishment, move-shape modifiers, ability protection, choice lock, positioning, and gimmick unlock.

8. **Evaluate field and board payoff**
   - Score weather, terrain, rooms, screens, Aurora Veil, Tailwind, Trick Room, hazards, redirection, trapping, phazing, status pressure, and partner bridge plans.
   - A reserve Pokemon can be correct if it changes the board, not only if it absorbs damage.

9. **Evaluate risk and timing**
   - Check KO race, last-Pokemon state, protect chains, priority, speed order, predicted target, predicted switch, status risk, setup risk, missed attack risk, and whether holding a gimmick creates higher expected value.
   - Do not spend Mega / Z / Dynamax / Tera just because it is available.

10. **Emit reusable reasons**
    - The AI should expose structured reasons like `ko_conversion`, `defensive_type_flip`, `max_move_field_control`, `status_z_payoff`, `preserve_pre_mega_ability`, `predicted_taunt_pivot`, `terrain_seed_entry`, or `ability_bridge`.
    - Avoid one-off comments like "use Tera now". The next tuning pass needs to know why the decision happened.

## Gimmick-Specific Re-evaluation Notes

### Mega / Ultra Burst

Mega and Ultra Burst primarily change stats, ability, sometimes typing, and immediate Speed / damage / defensive lines. The AI must evaluate both pre-form and post-form states.

Required checks:

- Prospective target-form profiles for known active and reserve party members,
  derived from the real party mon rather than injected only by simulator tests.
- Pre-form ability value: `Air Lock`, `Cloud Nine`, trapping, weather, terrain, Intimidate, or item / ability scouting value.
- Post-form ability value: trapping, weather, damage amplification, defensive race, priority, or field control.
- Speed flip and damage thresholds.
- Whether setup should happen before spending the transformation.
- Whether the target form creates a bad interaction with weather, terrain, item, or partner plan.
- Whether a searched switch exposes later Mega / Ultra eligibility, and whether Ultra
  Burst preserves a later legal Z action.

### Z-Move

Z-Move evaluation must branch by damaging Z-Move and status Z-Move.

Required checks:

- Base move type, category, target, disabled state, PP, and accuracy.
- Damaging Z power, KO conversion, low-accuracy KO stabilization, trap pressure, and damage-race improvement.
- Status Z effect, whether the status effect is useful now, and whether grounded Psychic Terrain, the target's current Dark type after Tera, Prankster, Magic Bounce, Good as Gold, Taunt, or Assault Vest-style blockers matter.
- Z-Crystal item lock and whether the item slot has opportunity cost compared with another item category.

### Dynamax / Gigantamax

Dynamax evaluation must treat the move as a temporary transformed move plus a temporary HP / disruption-resistance state.

Required checks:

- HP multiplier and whether the HP increase changes the KO race.
- Max Move type, power, target behavior, secondary effect, weather / terrain / stat side effect, and G-Max unique effect.
- Fake Out, flinch, Encore, Disable, phazing, weight-based moves, Choice lock, and other local runtime exceptions.
- Three-turn timing: immediate offense, defensive stall, field setup, or late-game cleanup.
- Whether the AI should preserve Dynamax for another candidate.

### Tera

Tera evaluation must recompute both offensive and defensive typing.

Required checks:

- Defensive type flip against known / predicted moves.
- Offensive STAB and Tera Blast behavior.
- Interaction with weather, terrain, type-changing abilities, Adaptability-style boosts, resist berries, and immunity abilities.
- Whether the current threat justifies spending the once-per-battle resource.
- Whether Tera creates a new weakness that the opponent can immediately punish.

## Required Runtime Outputs

The runtime layer and its v5 trace should be able to answer:

- What is the effective move after gimmick conversion?
- What are the effective type, category, power, target, and move-shape flags?
- What is the effective ability after form / field / suppression / bypass?
- What is the effective item knowledge after item-disabled and consumed states?
- Does this action create immediate pressure, future board value, or both?
- Does this action spend a once-per-battle resource, and is that resource better saved?
- Which opponent action is this decision reading?
- Which partner or reserve plan makes the action valuable?

## Implementation Policy

- Add data-driven predicates first; add scoring rules only after a predicate has tests.
- Prefer existing move flags, ability constants, hold effects, and config gates over hand-built parallel tables.
- Where a gimmick changes a move, re-evaluate the transformed move rather than relying on the base move.
- Where a gimmick changes type, ability, stats, or HP, evaluate both current and post-gimmick states.
- Keep singles conservative and doubles more board-aware.
- Every new scoring rule should have at least one positive and one negative test, plus one cross-gimmick or blocked-interaction test when applicable.

## Current Gap List

- Central move / switch / modeled-gimmick arbitration now runs for eligible 2v2 boards; unsupported candidates or boards deliberately use the legacy evaluator rather than a partial joint result.
- Active Dynamax, newly selected Dynamax, exact Max Move dynamic power, and most G-Max unique / residual effects remain outside the immutable board-search envelope. Reusable AI knowledge flags still support legacy tactic scoring.
- Full-knowledge active and reserve Mega / Ultra profiles and later reserve gimmick
  eligibility are now derived from real party data. Entry-event abilities and unknown
  transformed profiles still fail closed.
- Exact stochastic enumeration currently covers single-target, single-hit modern
  critical rules. A proven one-hit invariant-input case can group equal raw-damage
  classes before replay; unsafe mixed turns stay on raw enumeration. Damaging spread,
  multi-hit, pre-Generation-3 critical rules, more than 32 distinct states after
  post-board merge, and the 4,096-application safety ceiling are explicit whole-side
  fallback boundaries.
- The 16-entry transposition table keeps the heap arena at 16,248 bytes. The exact
  frontier instead uses its separate 2,596-byte static EWRAM scratch. The smaller
  cache can cause more recomputation inside the existing node / frame budgets, but
  it does not change outcome legality or probability weights.
- Status Z-Move effects are still mostly evaluated through existing Z viability logic rather than the shared runtime knowledge layer.
- Tera defensive evaluation now rejects pure defensive Tera lines that introduce a new large-hit weakness, but it still does not run a full multi-turn defensive type search.
- Combined gimmick environments are compared centrally only for candidates the simulator can represent exactly; expanding that supported envelope must preserve the same fail-closed contract.
- Ability and item categories are broad runtime knowledge flags; deeper scoring still needs targeted predicates such as "this item protects the exact line the opponent is threatening".
- Battle-script-only edge behavior still needs audit coverage for mechanics that do not surface cleanly through `GetMoveEffect()`, move flags, ability constants, or hold effects.
