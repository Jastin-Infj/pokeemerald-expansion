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

1. local expansion catalog: constants, config-gated move fields, AI knowledge flags, and item hold effects.
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
- Z-Move is usually below Tera as a one-shot spend, but can outrank Tera when it converts a KO, breaks a trap, bypasses accuracy risk, or gives a decisive status Z effect.
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

- Pre-form ability value: `Air Lock`, `Cloud Nine`, trapping, weather, terrain, Intimidate, or item / ability scouting value.
- Post-form ability value: trapping, weather, damage amplification, defensive race, priority, or field control.
- Speed flip and damage thresholds.
- Whether setup should happen before spending the transformation.
- Whether the target form creates a bad interaction with weather, terrain, item, or partner plan.

### Z-Move

Z-Move evaluation must branch by damaging Z-Move and status Z-Move.

Required checks:

- Base move type, category, target, disabled state, PP, and accuracy.
- Damaging Z power, KO conversion, low-accuracy KO stabilization, trap pressure, and damage-race improvement.
- Status Z effect, whether the status effect is useful now, and whether Dark-type / Prankster / Magic Bounce / Good as Gold / Taunt / Assault Vest style blockers matter.
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

The long-term runtime layer should be able to answer:

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

- Max Move / G-Max unique effects are not yet fully exposed as reusable AI knowledge flags.
- Status Z-Move effects are still mostly evaluated through existing Z viability logic rather than the shared runtime knowledge layer.
- Tera defensive evaluation still needs a richer "new weakness introduced" check.
- Combined gimmick environments need a central resource arbitration pass so Mega / Z / Dynamax / Tera decisions are compared consistently.
- Ability and item categories are broad runtime knowledge flags; deeper scoring still needs targeted predicates such as "this item protects the exact line the opponent is threatening".
- Battle-script-only edge behavior still needs audit coverage for mechanics that do not surface cleanly through `GetMoveEffect()`, move flags, ability constants, or hold effects.
