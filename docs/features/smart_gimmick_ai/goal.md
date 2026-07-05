# Smart Gimmick AI Goal

## Goal

The target is not an unbeatable AI. The target is an AI that usually loses only because the player brought a better construction, found a real matchup edge, or created a line that is outside the AI's available information. It should not lose because it missed a direct KO, ignored a selected player command, wasted a limited gimmick, targeted an invalid slot, let a known setup turn through for free, or refused a realistic last-chance out when stable play was already gone.

Short version: construction should matter more than shallow tactical mistakes.

The current reset target is the no-safe-switch problem in doubles and singles: when a valuable Pokemon is pinned by confirmed pressure, has no usable safe Protect / Detect line, and no clearly safe switch-in, the AI must still rank imperfect choices instead of defaulting to a doomed attack. It should try the practical ladder in order: immunity / absorption, strong resistance, survival on acceptable damage rolls, nonfatal cushion, then deliberate sacrifice of a lower-value support or Focus Sash slot when the next board is better.

Protect / Detect only counts as safe after legality and counterplay checks. Feint, moves that ignore protection, Unseen Fist-style protection bypass, Imprisoned Protect, Taunt / Encore locks, disabled moves, insufficient PP, and similar disruption should make the AI fall back to the same imperfect-switch ladder rather than assuming Protect solves the turn.

Protect / Detect / Max Guard-style actions also need an active timer-stall role. They can be correct even when they do not directly save a pinned slot if they burn the opponent's limited board turns: Tailwind, Trick Room, weather, terrain, screens, Aurora Veil, rooms, Perish Song counters, Dynamax turns, sleep turns, or other temporary pressure. The AI should compare whether spending a protected turn creates the next board, rather than treating it as wasted tempo by default.

Recovery changes that value. Leftovers, Grassy Terrain, Poison Heal, Rain Dish, Ice Body, Aqua Ring, Leech Seed, Wish, berry thresholds, and similar end-of-turn healing can turn a protected turn from neutral into a survival line. The evaluator should compare the post-stall HP band and next-turn damage rolls, not only the current HP before the Protect.

Concrete example: if Choice Specs Kyogre is pinned by selected Electric Z pressure plus Fake Out and cannot Protect, and no Electric-immune reserve exists, switching Kyogre to Whimsicott can be correct even if Whimsicott is likely lost. The payoff is preserving Kyogre, consuming Fake Out, and allowing a pivot such as Volt Switch or a forced reserve entry to reintroduce Kyogre with pressure on the next turn.

## Current Baseline

The feature branch already has these pieces:

- Full-information read mode for non-link trainer AI that can use confirmed player commands.
- Smart timing for Mega / Ultra Burst, Z-Moves, Dynamax / Gigantamax, and Tera.
- Payoff-based Protect scoring rather than passive Protect use.
- Smart switching for selected-command KO pressure, board-control pivots, Taunt pivots, Choice-role-done pivots, and bad double positions.
- Partner tactics for redirection, ally healing, low-damage activation, guaranteed-critical `Anger Point`, `Psych Up`, Haze, and conservative ally `Clear Smog`.
- Bounded desperation comeback scoring for near-term loss boards: low-accuracy status, flinch, paralysis / freeze, crit, OHKO, Taunt, Tailwind, Trick Room, speed drops, Throat Chop, and spread sacrifice only when clean lines are gone.
- A small board snapshot / threat classifier / short-horizon helper used by the shared risk governor.
- Soundproof switch bridges for read Perish Song, lethal read sound-move pressure, and selected two-hit sound damage races when clean faster damage cannot stop the line.
- Commander targeting awareness for swallowed Tatsugiri and Dondozo slot correction.
- Battle action log v4 reason tags plus compact threat / risk / short-horizon line trace fields for non-obvious AI choices such as hax-out, desperation comeback, Perish escape, ally sacrifice, and Commander slot correction.
- Exact selected-damage roll summaries for focused switch survival across the normal one- or two-source incoming damage cases.

This is strong enough for tactical smoke testing. It is not yet a full battle-line planner.

## Remaining Design Weaknesses

1. **No unified 2-3 turn line search yet.**
   The current code has several bounded future-pressure heuristics, but it does not build and compare complete short lines such as "Protect now, switch next turn, then Tailwind" or "sacrifice support, bring reserve, then force checkmate."

2. **Move selection and switching still meet late.**
   The narrow Soundproof escape for read Perish Song exists, and lethal read sound-move pressure can now choose a Soundproof reserve. Many correct lines still require comparing a move answer against a switch answer. Examples are nonlethal sound-pressure switching, switching a board-control setter, or staying in for a high-variance stop. Today those are split across move scoring and switch scoring.

3. **The risk governor is centralized, but still shallow.**
   Accuracy risk, OHKO fishing, secondary effects, delayed attacks, partner sacrifice, second Protect, and switch survival now pass through one helper. The weakness is that the helper still answers yes / no from a compact snapshot instead of comparing fully simulated candidate lines.

4. **Ally order manipulation still needs a real order planner.**
   A narrow score-layer slice now recognizes using a Speed drop such as Scary Face on an ally under active Trick Room when it flips that ally behind a live foe and the ally can immediately cash out with a KO. Broader lines, such as intentionally slowing a partner for survival / reserve-entry sequencing or comparing the line against switches and Protect, still require action-order, survival, and next-board checks.

5. **Mechanic exception coverage is incomplete.**
   Commander targeting is implemented, and Toxic Orb / poison swallowed Tatsugiri score coverage now has a lower-level regression that avoids the high-level Commander residual turn issue. Similar future exceptions should still be validated at the smallest stable layer.

6. **Reason traces are still compact.**
   The runtime now emits one reason tag, board threat flags, and the relevant risk family per AI action in the action log. It still does not emit candidate scores, rejected alternatives, or the short-horizon facts that caused that reason to win.

7. **Pinned no-safe-Protect slots need an imperfect-switch ladder.**
   The current switch code can find a clean defensive pivot, preserve a spent Choice attacker when it is ignored, reject false Protect safety, accept a narrow 15/16 switch-in survival line under focused confirmed pressure, rank a nonfatal cushion above a doomed support sacrifice, and prefer pivot-capable switch-ins in otherwise comparable preserve / sacrifice cases. In desperation it can also fall back to a 14/16 survival threshold. The focused selected-command roll summary now uses exact one- or two-source combined roll distributions instead of adding independent 14/16 or 15/16 quantiles. It is still weak beyond that focused one-turn slice: multi-turn reserve-entry comparison, longer recovery / chip accounting, and a unified ladder are not yet implemented. A Protect move in the set should not stop this check if the selected or likely counterplay beats Protect, such as Feint, protection-bypassing moves / abilities, Imprison, Taunt, Encore, Disable, or PP failure. This must not be tested with artificial perfect immunities only; regressions should use production-like doubles positions where the correct answer may be sacrificing a support Pokemon or taking a controlled roll to preserve a win condition.

8. **Protect timer-stall is under-modeled.**
   Protect is already scored as a payoff move, but the long-term design needs to value intentional turn burning. If the opponent's Tailwind, Trick Room, Dynamax, terrain, weather, screen, or Perish clock is the real problem, a protected turn can be the board-building answer. The evaluator should also notice when the AI's own favorable timer is about to expire, because using Protect there may waste its own mode instead of improving the board.

9. **End-of-turn recovery needs to feed stall and roll thresholds.**
   Existing survival checks often treat HP as a static value. Timer-stall and imperfect-switch decisions should include expected recovery and damage-over-time before judging whether a line survives: Leftovers, Grassy Terrain, Poison Heal, Rain Dish, Ice Body, Aqua Ring, Leech Seed, Wish, berries, sand / hail / poison / burn, and other end-of-turn effects. This matters for deciding whether a 14/16 or 15/16 roll is acceptable, whether Protect restores a survival band, or whether a sacrifice is still necessary.

## Implementation Status

- Done: `AiBoardSnapshot`, `AiThreatClass`, and `AiShortHorizon` helpers.
- Done: shared `AI_RiskGovernorAllows()` for the identified risk families.
- Done: Perish Song / Soundproof switch-vs-move bridge for the narrow read-Perish escape case.
- Partial: debug reason tags plus compact threat / risk / short-horizon line trace fields in `BattleActionLog` v4 and the mGBA Live JSON exporter. Candidate scores, rejected alternatives, and full two-to-three-turn facts are not logged yet.
- Partial: short-horizon evaluation covers Perish Song, selected setup, mode-loss pressure, reserve-entry sacrifice, Protect counterplay including Feint / bypass / selected Z or Max chip, and clean damage suppression. It now records a narrow `AiCandidateLine` family for the first stable line and first fallback risk line, and high-variance / secondary-hax risk is suppressed when a clean stable line exists. It is still not a full candidate-line solver.
- Partial: one-turn end-of-turn recovery now feeds singles Protect payoff and smart switch-in survival estimates for common deterministic recovery such as Leftovers, Black Sludge, Grassy Terrain, Poison Heal, Rain Dish, Ice Body, Aqua Ring, Ingrain, Leech Seed drain recovery, and HP-restoring berry thresholds after end-turn chip. Focused Protect regressions now cover Grassy Terrain, Leftovers, Sitrus-after-chip, and Leech Seed drain recovery; switch-in recovery coverage now verifies Sitrus as 25% healing rather than a tiny fixed fraction. It is not yet a full two-to-three-turn recovery / chip simulator.
- Partial: timer-stall Protect now covers narrow singles cases where the opponent's Tailwind or Trick Room is on its final turn, the incoming move is dangerous, and the AI will gain speed control after the timer expires. It also covers narrow singles cases where final opposing Reflect, Light Screen, Aurora Veil, weather, terrain, or Dynamax is blocking a meaningful follow-up attack or making the incoming attack meaningfully more dangerous, plus the narrow case where the opponent alone will fall to an already-active Perish Song count this turn. Final weather / terrain burn checks now recalculate next-turn Speed after expiry, so expiring Swift Swim / similar field-speed boosts are not treated as still active. Final opposing Dynamax burn covers Max Guard while the AI keeps its own Dynamax afterward, and regular single-target Protect when the AI survives the 1/4 Max Move damage that leaks through protection. A narrow guard prevents spending Protect when it would burn the AI's own final Tailwind alongside final Trick Room without producing a better next board, and other guards avoid valuing Protect when both active singles battlers fall to Perish at the same count or when the AI would spend its own final Max Guard only to burn the opponent's final Dynamax. Doubles now has narrow score-layer bridges where a pinned slot can Protect to burn final opposing Tailwind, final Trick Room, final opposing screens, or final opposing terrain when its partner gains the post-expiry KO / large-hit payoff, or to outlive an opposing active Pokemon whose Perish Song count expires this turn. Doubles also has narrow self-sabotage guards that reject burning the AI side's final Tailwind with final Trick Room when neither the protected slot nor its partner gains an immediate or post-expiry KO payoff, and reject Protect when the protected slot and the target both fall to Perish this turn. Nonfinal Dynamax / Perish turns, weather in doubles, broader terrain / screen tradeoffs, and full doubles partner-board timer planning still need planner coverage.
- Done: lower-level Commander Toxic Orb / poison score coverage.
- Partial: pinned-slot switching now rejects Protect as a safe stay-in answer when confirmed Feint / protection bypass or selected Z / Max damage through Protect can still remove the slot. It can also keep an otherwise imperfect switch-in when the active high-value slot is under confirmed focused collapse and the reserve survives at the 15/16 damage-roll threshold, or at 14/16 only when the shared desperation governor says stable play is gone. Those one- or two-source selected-damage thresholds now come from an exact combined distribution of 16 damage rolls per incoming hit rather than additive per-hit quantiles. Focused collapse ranking now builds explicit safe / roll-survival / nonfatal-cushion / sacrifice candidates, gives a distinct middle tier to nonfatal cushions before deliberate support / Focus Sash sacrifice, and rejects doomed sacrifice candidates that do not consume Fake Out, carry support / Focus Sash value, or otherwise improve the next board. Pivot-capable candidates with `U-turn` / `Volt Switch` / `Flip Turn`-style hit-escape moves receive follow-up value in comparable preserve / sacrifice cases. The imperfect-switch ladder still needs multi-turn reserve-entry comparison and broader recovery / chip line comparison.
- Partial: delayed attacks now pass through the shared risk governor for both semi-invulnerable attacks and non-semi-invulnerable two-turn attacks. Semi-invulnerable moves can be a desperation out when they dodge the immediate threat. Solar Beam / Meteor Beam / Electro Shot-style moves are only allowed as delayed risks when the AI survives the charge turn and the delayed hit can create a short-horizon payoff; Power Herb or weather-accelerated versions continue to be treated as ordinary immediate attacks. Broader delayed pivot / sacrifice sequences are still planner work.
- Partial: ally-order manipulation now has a narrow score-layer bridge for Speed drops into an ally under active Trick Room. The bridge requires a real order flip, avoids White Herb / Clear Amulet / minimum-Speed dead ends, and requires immediate KO or threat payoff using on-demand damage calculation. It is not a full order planner and does not compare multi-turn switch / Protect / sacrifice lines.
- Partial: broader sound-pressure answers now cover lethal read sound-move pressure with Soundproof switch-ins, including spread sound moves, when the current battler cannot win the damage race first. They also cover selected nonlethal sound damage when the hit creates a two-hit collapse and a Soundproof reserve has immediate attack or board value. Move scoring can use `Throat Chop` as an immediate read answer to selected sound pressure, including selected Perish Song. Broader status sound moves beyond Perish Song, full Throat Chop-vs-switch comparison, lower-impact sound chip, and multi-turn sound-denial lines remain planner work.

## Redesigned Target Architecture

The next design should treat each AI action as a candidate line, not only as a move score.

1. **State Snapshot**
   Build a small immutable snapshot for the acting battler and side: active Pokemon, partner, reserves, known player commands, known switch-ins, field state, gimmick access, trapped state, perish counters, speed order, and relevant mechanics exceptions such as Commander.

2. **Threat Classifier**
   Classify the board before scoring:

   - `stable`: no immediate forced loss.
   - `damage_race`: ordinary damage can decide the position.
   - `known_ko_pressure`: selected or predicted command can remove the AI slot.
   - `setup_checkmate`: selected or existing setup makes the next board likely losing.
   - `perish_or_trap_clock`: Perish Song / trapping creates a short forced clock.
   - `mode_loss`: Tailwind, Trick Room, weather, terrain, or speed order will decide the next turns.
   - `desperation`: no clean line remains inside the short horizon.

3. **Candidate Line Generator**
   Generate move, switch, gimmick, protect, sacrifice, and comeback candidates from the same state snapshot. Each candidate should carry a one-turn result plus optional two-to-three-turn assumptions.

   Candidate families:

   - clean KO / damage race
   - Protect / stall payoff, including intentional timer burn and end-of-turn recovery
   - switch / pivot / preserve
   - board-control setup
   - direct disruption
   - ally support / ally activation
   - controlled sacrifice / reserve entry
   - high-variance comeback

   Sacrifice candidates must carry their next-board payoff. A sacrifice is not good only because it takes damage; it is good when it preserves a higher-value Pokemon, burns a one-turn disruption such as Fake Out, opens a reserve entry, enables a pivot move, restores speed or board control, or prevents a two-to-three-turn forced loss.

4. **Risk Governor**
   Apply one shared risk policy after candidate generation. Stable lines should beat hax. High-variance lines should unlock only when the classifier says `desperation` or when the payoff is large enough to justify the risk.

   The governor should own:

   - minimum hit / accuracy thresholds
   - acceptable damage roll thresholds, such as 14/16 or 15/16 survival or KO lines
   - OHKO / crit / flinch / paralysis fishing
   - low-accuracy status
   - partner sacrifice
   - risky second Protect
   - risky switch-in survival

5. **Short Horizon Evaluator**
   Compare candidates over a bounded horizon:

   - Turn 0: current legal action.
   - Turn 1: likely forced response, reserve entry, speed-order change, or perish tick.
   - Turn 2: checkmate, stabilized board, or lost board.

   This does not need to enumerate every possible move. It should enumerate likely critical lines: selected command, best damage, Protect, switch, setup, speed control, and known trap / perish moves.

6. **Reason Output**
   Every non-obvious winning candidate should keep a reason tag. The first implementation can store this only in debug builds or battle action logs.

   Required tags:

   - `clean_damage_preferred`
   - `known_command_answer`
   - `gimmick_stabilized`
   - `switch_preserve`
   - `board_control`
   - `setup_denial`
   - `perish_escape`
   - `desperation_comeback`
   - `hax_out`
   - `ally_sacrifice_board_reset`
   - `commander_slot_correction`

   The current log also stores compact `AiThreatClass` flags and the relevant `AiRiskKind` family for selected move / switch decisions. The remaining trace work is to store why that candidate beat close alternatives: scores, rejected high-variance lines, short-horizon lines available, and the clean-line suppression facts.

## Acceptance Criteria

- If clean damage can KO or win the short race, the AI should not choose a weaker hax line.
- If no stable line exists and the loss clock is within roughly three turns, the AI may choose high-variance outs.
- If Perish Song can be escaped by switching and no other board collapse exists, the AI should prefer switch / damage over flinch fishing.
- In doubles, speed-control and disruption lines should be evaluated as board answers, not only as status move bonuses.
- Ally sacrifice should require role value, opposing-side pressure, and reserve-entry payoff.
- If a high-value Pokemon is pinned, lacks a safe Protect / Detect line, and no safe switch exists, the AI should still compare imperfect switch-ins before accepting the loss. It may sacrifice a lower-value support slot when that preserves a win condition and improves the next board.
- Read-mode switch tests for this behavior must be practical battle positions, not hand-built "electric immune reserve exists" cases. The primary regression should cover preserving Choice Kyogre by switching to Whimsicott under selected Electric Z plus Fake Out pressure.
- Protect / Detect / Max Guard-style lines should be able to win by burning opponent Tailwind, Trick Room, Dynamax, weather, terrain, screen, or Perish turns, while avoiding self-sabotage when the AI's own favorable timer is the one being burned.
- Stall and survival decisions should include expected end-of-turn recovery and chip, including Leftovers and Grassy Terrain, before deciding whether a damage roll, Protect, switch, or sacrifice line is acceptable.
- Soundproof and similar ability-based answers should be compared as switch candidates, not bolted onto move scoring.
- Commander Dondozo / Tatsugiri targeting must remain a mechanics exception in target validity, switch prediction, and scoring. The AI should normally target the Dondozo slot while Tatsugiri is swallowed, should remember that spread damage still keeps spread scaling instead of becoming singles damage, and should consider attacking the Tatsugiri slot only through the Dondozo target direction when Dondozo is about to faint and Tatsugiri will remain on the board.
- Toxic Orb / Flame Orb / poison swallowed Tatsugiri lines must stay explicit mechanics coverage, not an accidental residual side effect.
- Delayed attacks, self-positioning attacks, spread moves that hit allies, and intentional support sacrifice should be allowed only when the next board is better, not merely because the current move has high damage.
- Protect safety must be tested against practical counterplay: Feint, protection-bypassing abilities or moves, Imprisoned Protect, Taunt / Encore / Disable / PP failure, and selected Z / Max chip through protection.
- High-variance moves such as flinch fishing, paralysis / freeze fishing, critical-hit lines, OHKO moves, and low-accuracy attacks or status moves should be locked behind short-horizon desperation or an unusually large board payoff.
- Every new heuristic must have either a focused `make check` regression or a documented manual runtime route in `test_plan.md`.

## Next Implementation Order

The next work should be treated as a reset of the AI planning layer, not as a collection of unrelated move-score bonuses. Priority is based on the manual failures seen in read-mode doubles: no safe switch / no safe Protect positions first, then short-horizon line comparison, then the mechanics exceptions that can make those lines legal or illegal.

1. **Pinned Slot / No Safe Switch Planner**
   Build the first candidate-line path around the practical no-safe-switch ladder. When a valuable active Pokemon is pinned and Protect is not actually safe, the AI should rank all imperfect answers before accepting the loss.

   Required behavior:

   - Check Protect / Detect legality and counterplay before trusting it.
   - Try immunity / absorption first, then strong resistance, then exact 15/16 survival, then desperation 14/16 survival, then nonfatal cushion, then deliberate lower-value support / Focus Sash sacrifice.
   - Include reserve-entry payoff: preserving a win condition, consuming Fake Out, enabling a pivot move, setting up the next attacker, or forcing a better board after a sacrifice.
   - Include "no good reserve" cases. Tests must not rely only on a perfect Electric-immune reserve. They should include realistic half-resists, bad but survivable cushions, and cases where sacrificing Whimsicott-style support is better than losing Choice Kyogre.
   - Compare singles and doubles separately. Doubles can use the partner, Fake Out consumption, redirection, and reserve entry; singles usually has fewer board-reset tools and should be more conservative.

   First slice:

   - Move the existing focused-collapse switch ranking into an explicit candidate list.
   - Add one doubles regression where the active slot has no Protect and no immunity, but a lower-value support sacrifice preserves the real win condition.
   - Add one negative regression where sacrificing the support is rejected because it does not improve the next board.

2. **Short-Horizon Line Comparator**
   Turn the current classifier into a small candidate comparator over roughly three turns. This is the layer that decides "if they take this option, the game ends" rather than only "my current HP is low."

   Required behavior:

   - Turn 0: current move / switch / Protect / sacrifice / high-variance out.
   - Turn 1: reserve entry, speed-order change, Perish tick, timer expiry, or forced response.
   - Turn 2: stabilized board, checkmate, or lost board.
   - Include selected player commands first, then obvious best damage, Protect, switch, setup, speed control, trap / Perish, and known denial moves.
   - Desperation should mean a battle-level loss clock inside this horizon, not merely one active slot being low.

   First slice:

   - Implement a narrow `AiCandidateLine` data shape that can be filled by switching, Protect, and move scoring without changing all score functions at once.
   - Start with Perish Song, selected setup, selected KO pressure, and mode-loss pressure.
   - Log the chosen line family and the rejected clean-line reason in debug / battle action logs.

3. **Protect Counterplay, Timer Stall, And Recovery Integration**
   Keep Protect valuable, but only after the AI proves it is legal and useful. Protect can be the correct move to burn an opponent's limited mode, but it can also waste the AI's own mode.

   Required behavior:

   - Reject false safety from Feint, protection-bypassing moves or abilities, Imprison, Taunt, Encore, Disable, no PP, and selected Z / Max chip through protection.
   - Value intentional turn burn for opposing Tailwind, Trick Room, Dynamax, weather, terrain, screens, Aurora Veil, Perish counters, sleep turns, and similar limited pressure.
   - Penalize Protect when it burns the AI's own last Tailwind, Trick Room, Dynamax, weather, terrain, screen, or other favorable timer without producing a better next board.
   - Feed end-of-turn recovery and chip into the survival band: Leftovers, Black Sludge, Grassy Terrain, Poison Heal, Rain Dish, Ice Body, Aqua Ring, Ingrain, Leech Seed, Wish, berries, poison, burn, sand, hail / snow, and similar effects.

   First slice:

   - Extend the existing final-turn Protect evaluator into doubles partner-board cases.
   - Add tests where Protect is correct only because Leftovers / Grassy Terrain moves the AI back into a survival roll.
   - Add tests where Protect is wrong because it burns the AI's own final speed-control mode.

4. **Desperation Risk Policy**
   Keep the "play to outs" behavior, but make the unlock condition stricter and easier to inspect. Hax should not replace clean damage; it should appear when stable play is already gone or when the payoff is large enough to justify the risk.

   Required behavior:

   - Unlock flinch, paralysis, freeze, critical-hit, OHKO, low-accuracy attack, and low-accuracy status lines only after the short-horizon comparator finds no stable line.
   - Allow 15/16 and, in deeper desperation, 14/16 survival / KO thresholds as controlled risks.
   - Let direct board stops such as Taunt, Tailwind, Trick Room, speed drops, Fake Out, Throat Chop, or Soundproof switching beat pure hax when they answer the actual loss clock.
   - Keep ordinary clean KO / damage-race moves preferred whenever they solve the board.

   First slice:

   - Route all existing desperation unlock checks through the candidate comparator result instead of ad hoc local "bad HP" checks.
   - Add one regression where an OHKO / flinch line is chosen only because every clean line loses within three turns.
   - Add one regression where the same hax line is rejected because a switch / Protect / damage answer exists.

5. **Commander / Tatsugiri Mechanics Planner**
   Keep Commander as a dedicated mechanics exception. It is too easy to break if it is treated like an ordinary target-validity quirk.

   Required behavior:

   - Do not choose swallowed Tatsugiri as the direct move target for ordinary attacks.
   - When targeting "Tatsugiri after Dondozo faints," still issue the command through the Dondozo direction because that is the legal selectable slot.
   - Preserve the official spread damage rule: spread moves remain spread damage while Commander is active; they do not become singles damage.
   - Consider residual and item state on swallowed Tatsugiri, including Toxic Orb / Flame Orb and existing poison / burn, when deciding whether KOing Dondozo creates a favorable remaining board.

   First slice:

   - Add target-selection regressions around Dondozo-at-low-HP where the AI wants the released Tatsugiri board but must still select the Dondozo slot.
   - Add a spread-damage regression that prevents accidental singles-damage assumptions.

6. **Action-Order Planner**
   Expand the narrow ally Speed-drop Trick Room bridge into a small order planner. This should cover the human-style lines around `Scary Face`, `Icy Wind`, Tailwind, Trick Room, Fake Out, and forced speed reversal.

   Required behavior:

   - Compare Speed control as board answers, not as flat status bonuses.
   - Include ally-target Speed drops under Trick Room, opponent Speed drops outside Trick Room, Tailwind setup, Trick Room setup / reversal, and priority / Fake Out interactions.
   - Check survival before the manipulated order matters. Slowing an ally is bad if the ally dies before cashing out.
   - Allow rare ally Speed manipulation, such as using `Scary Face` on a partner under Trick Room, only when it creates an immediate or near-term payoff.

   First slice:

   - Generalize the current ally Speed-drop helper into an order-delta primitive shared by move scoring and short-horizon candidates.
   - Add a doubles regression where ally speed manipulation is worse than switching or Protect, so the bridge does not overfire.

7. **Delayed Moves, Spread Friendly Fire, And Sacrifice Lines**
   Delayed attacks and spread moves that hit allies are not usually correct, but they are legitimate when they dodge the current loss, create a forced next-board payoff, or intentionally retire a role-complete support Pokemon.

   Required behavior:

   - Treat weather-accelerated or Power Herb delayed moves as ordinary immediate attacks when the charge is removed.
   - For ordinary delayed attacks, require survival through the delay plus a concrete payoff on the following board.
   - For semi-invulnerable moves, value dodging immediate lethal pressure only when the resulting turn is not still lost.
   - For spread friendly fire, require opposing-side pressure, role value of the ally being sacrificed, and reserve-entry payoff.

   First slice:

   - Move delayed-risk and spread-sacrifice checks onto the same candidate-line structure as pinned-slot switching.
   - Add one positive support-sacrifice regression and one negative "kills partner for no board gain" regression.

8. **Switch-Vs-Move Bridge Expansion**
   Broaden the existing Soundproof / Throat Chop / Perish bridges so switching and moving are compared in one place.

   Required behavior:

   - Compare Soundproof switching, Throat Chop, direct KO, Taunt, Fake Out, and Protect against selected sound or status pressure.
   - Include nonlethal sound pressure when it creates a two-hit collapse.
   - Include board-control switch-ins such as Tailwind, Trick Room, weather, terrain, status prevention, and ability bridges.

   First slice:

   - Use the candidate-line comparator for Perish Song first, then extend to selected sound damage and selected sound status.

9. **Trace And Test Harness**
   Make the new planner inspectable enough that manual mGBA losses can be converted into regressions instead of guesswork.

   Required behavior:

   - Battle action logs should show chosen candidate family, threat class, risk family, clean-line suppression, and close rejected alternatives for non-obvious decisions.
   - Manual fixtures should be production-shaped. Avoid tests that only pass because a perfect immunity or scripted dummy board exists.
   - Every planner slice should have a focused `make check` test and, for runtime-affecting changes, the standard local build plus one mGBA Live smoke or documented manual route.

   First slice:

   - Add debug-only candidate summary fields before adding broad new behavior, so later manual reviews can tell whether the AI considered the right family and merely weighted it incorrectly.
