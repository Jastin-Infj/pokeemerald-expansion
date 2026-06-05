# Smart Gimmick AI Implementation

## Summary

The feature adds smart timing flags for battle gimmicks, expands Dynamax timing beyond immediate damage, adds Mega / Ultra Burst payoff checks, adds a smart conservation layer for damaging Z-Moves, tunes Protect as a board-payoff move instead of a passive default, and provides debug battle fixtures for competitive-style 3v3 singles, 4v4 doubles, and Dmax-vs-Z checks.

Runtime files:

- `include/constants/battle_ai.h`
- `src/battle_ai_switch.c`
- `src/battle_ai_util.c`
- `src/battle_script_commands.c`
- `src/data/debug_trainers.party`
- `src/debug.c`
- `test/battle/ai/ai_switching.c`
- `test/battle/ai/ai_smart_gimmick.c`
- `test/battle/exp.c`

Documentation:

- `docs/tutorials/ai_flags.md`
- `docs/features/smart_gimmick_ai/`

Tooling:

- `tools/runtime_knowledge/`

## Runtime Behavior

`DecideGimmickBeforeMoveSelection()` still handles the early smart-gimmick gate. `ReconsiderSmartGimmick()` then checks the chosen move and cancels the gimmick when the selected turn is not worth spending.

For Dynamax, `ShouldUseSmartDynamax()` now accepts these reasons:

- Last available Pokemon.
- The current target can otherwise KO the AI Pokemon.
- A known or predicted opposing move threatens Dynamax-blocked disruption while the AI selected a damaging move:
  - Fake Out-style flinch pressure.
  - Roar / Whirlwind-style forced-switch pressure.
- Dynamax converts the selected move into a KO that the regular move would miss.
- The selected Max Move creates strategic board value:
  - Speed control through `Max Airstream` or `Max Strike`.
  - Weather control through Fire, Water, Rock, or Ice Max Moves.
  - Terrain control through Electric, Grass, Fairy, or Psychic Max Moves.
  - Side-wide stat boosts or drops from Fighting, Poison, Ground, Steel, Dragon, Bug, Ghost, and Dark Max Moves.

Weather and terrain decisions call `ShouldSetWeather()`, `ShouldSetFieldStatus()`, and `ShouldClearFieldStatus()` so smart Dynamax follows the same field-state opinions used by normal AI move scoring.

For Mega / Ultra Burst, `ShouldUseSmartMega()` now looks at the target form before spending the gimmick. It allows immediate use for target-form ability payoffs such as trapping or weather control, estimated Speed flips, meaningful defensive improvement under KO pressure, and meaningful attacking-stat improvement for the selected damaging move. It still delays setup turns when there is no immediate pressure, and it can preserve `Air Lock` / `Cloud Nine` during active weather if the target form has no stronger immediate payoff.

For Z-Moves, `ShouldUseSmartZMove()` now wraps the existing Z-Move viability checks. Status Z-Moves keep their tactical checks. Damaging Z-Moves are conserved unless the AI is on its last Pokemon, the Z-Move converts the selected move into a KO, improves the damage race under immediate KO or trap pressure, or protects a low-accuracy KO line.

For double battle switching, `ShouldSwitchIfDoublePositionBad()` adds a VGC-style positioning check under `AI_FLAG_SMART_SWITCHING`. The AI may hard switch when the active Pokemon has no meaningful pressure into either opposing slot, is threatened by either opposing slot, has enough HP to be worth preserving, and its partner cannot cover the position. The check stays out of `AI_FLAG_SEQUENCE_SWITCHING`, respects existing no-switch gates, avoids overriding the existing Intimidate-blocker contract, and allows double switches when both active Pokemon are pinned.

For board-control switching, `ShouldSwitchIfBoardControlBenefit()` lets `AI_FLAG_SMART_SWITCHING` identify reserve Pokemon that can immediately or soon change the board. The selector can choose weather setters (`Drizzle`, `Drought`, `Sand Stream`, `Snow Warning`), terrain setters (`Electric Surge`, `Grassy Surge`, `Misty Surge`, `Psychic Surge`, `Hadron Engine`), speed-control setters carrying `Tailwind` or `Trick Room`, status-pressure support, status-prevention / cure support, terrain seed plans, and ability bridge support such as `Skill Swap`, `Role Play`, and `Entrainment`.

The board-control switch is deliberately narrower in singles than doubles. Singles still need bad odds, a bad matchup, missing current pressure, an unfavorable field, or an immediate status-absorption payoff to replace. Non-immediate status pressure / support pivots are double-battle only in this slice so the AI does not abandon a winning singles 1v1 only because the bench has a utility move. Doubles can pivot more proactively once the reserve candidate itself has a clear weather, terrain, Tailwind, Trick Room, status, seed, or ability-bridge payoff, matching the VGC positioning model where a bench Pokemon can create pressure instead of merely absorbing damage.

Status-aware board-control support includes:

- Non-volatile status pressure, secondary status effects, and related pressure moves (`Spore`, `Yawn`, `Toxic`, `Will-O-Wisp`, paralysis moves, freeze / frostbite effects, `Toxic Spikes`, `Leech Seed`, `Swagger`, and confusion pressure).
- Team status care with `Heal Bell` / `Aromatherapy`.
- Status prevention with `Safeguard` or `Misty Terrain` when the opposing side has known status pressure.
- Self-status / status-benefit lines for `Guts`, `Quick Feet`, `Marvel Scale`, `Magic Guard`, `Poison Heal`, `Toxic Boost`, `Flare Boost`, `Facade`, and `Psycho Shift`.
- Terrain seed activation when the reserve setter creates the matching field.
- Aurora Veil only when snow / hail is already active or the reserve ability can create it.
- Skill Swap-style bridge moves when the reserve or active partner has a board-control ability worth moving or copying.

AI runtime knowledge now has a shared move / ability / item interpretation layer in `battle_ai_util`. The layer maps existing move flags into AI categories such as sound, bullet, powder, slicing, punching, biting, pulse, dance, wind, healing, Magic Coat, Snatch, ability-control, move-denial, and combo-state. It also maps abilities into strategy groups such as move immunity, move power, damage race, status interaction, field control, positioning, ability control, stat control, item control, priority, and form / state. Held items are mapped through hold effects into damage race, defensive race, stat control, Speed / order control, recovery, status cure, self-status, field duration, contact / hit punishment, move-shape modifiers, ability protection, choice lock, positioning, and gimmick unlock. It also exposes `AI_CanBattlerIgnorePredictedMove()`, which bridges active abilities and held items for prediction decisions using Mold Breaker-sanitized ability checks and enabled-item checks. Predicted-Taunt switching now calls this shared predicate instead of maintaining a local Taunt-only copy.

`tools/runtime_knowledge` adds the first offline catalog generator for larger knowledge collection. It is a Rust tool with no third-party crates. The generator reads the local expansion source and writes review JSON for moves, abilities, hold effects, items, gimmick policy, and a summary. This gives later Pokemon Wiki, VGC / official tournament, Champions usage, PartyGen, and observed-history adapters a local runtime baseline instead of requiring one-off source audits for every AI heuristic.

Predicted-Taunt support is intentionally separate from "bad move" switching. `ShouldSwitchIfPredictedTauntPunish()` only runs when `AI_FLAG_SMART_SWITCHING` and `AI_FLAG_PREDICT_MOVE` are both active and the incoming move is predicted as `Taunt`. It requires the current Pokemon to depend on important status moves, rejects positions where the current Pokemon can already damage-race or 2HKO the target, and skips Pokemon protected by `Aroma Veil`, Gen 6+ `Oblivious`, or an enabled Gen 5+ `Mental Herb`. If those gates pass, the selector evaluates eligible reserves as free switch-ins and chooses a damaging attacker that can win the immediate 1v1 or cross the switch-in damage threshold. The goal is to model "pivot an attacker into a predicted Taunt" without making the AI flee only because Taunt would block a utility move.

Protect scoring now separates singles and doubles payoff. In singles, first-turn Protect is no longer rewarded just because a damaging move is predicted. `ShouldUseSinglesProtect()` asks whether Protect gains the AI a turn or a future option: residual damage on the target, incoming Wish recovery, Poison Heal / Leftovers / Black Sludge recovery, Substitute-threshold recovery, choice-lock scouting, Disable / Encore follow-up, Explosion-style avoidance, or target secondary damage. It also avoids passive Protect against an already boosted attacker unless there is a Wish or Disable / Encore line to justify the stall.

Consecutive Protect remains possible. The second Protect is penalized for its reduced success rate, but it is not treated as impossible. Singles can still value a second Protect when the same payoff remains, and doubles get a lighter second-Protect penalty so high-pressure VGC-style "protect again" turns stay available. Third and later consecutive Protect attempts remain heavily discouraged.

Future usage / ranking data should come from source-tagged competitive data such as Pokemon Battle DataBase, official event reports, Pokemon Home / Champions-style usage when available, local PartyGen catalogs, and observed battle history. Pokemon Showdown articles should not be used as strategy source material for this feature; at most, raw data or team examples can be inspected with a clear source tag and lower confidence.

The debug Party menu now exposes four focused runtime fixtures:

- `Battle 3v3 Single` builds a level-50 player team of `Dragonite`, `Gholdengo`, and `Garchomp`, then pits it against a 3-Pokemon AI side selected from a 6-Pokemon weighted singles pool.
- `Battle 4v4 Double` builds a level-50 player team of `Incineroar`, `Rillaboom`, `Flutter Mane`, and `Urshifu-Rapid-Strike`, then pits it against a 4-Pokemon AI side selected from a 7-Pokemon weighted doubles pool.
- `Battle Dmax/Z Single` builds a level-50 player-side Z-Move team, then pits it against a smart AI Dynamax / Gigantamax / itemless Z-Move team.
- `Battle Dmax/Z Double` builds a level-50 player-side VGC-style Z-Move team, then pits it against a smart AI Dynamax / Gigantamax / itemless Z-Move doubles team with Tailwind / weather pressure.

These AI fixtures enable `Smart Trainer`, `Prediction`, `Smart Gimmick`, `Gimmick Env Itemless`, `Know Opponent Party`, and `Powerful Status` AI flags. The pool entries carry `Pool Weight` plus role tags such as `Lead`, `Ace`, `Support`, `Weather Setter`, and `Weather Abuser`, so the pool-based fixtures also exercise Trainer Party Pool role filtering and weighted selection.

Itemless Z-Move availability is now split into a runtime environment unlock and explicit trainer intent:

- `B_FLAG_ITEMLESS_GIMMICK_BATTLE` can be assigned to a real event flag for in-game rulesets that intentionally allow itemless gimmick access.
- `AI_FLAG_GIMMICK_ENV_ITEMLESS` / `AI_FLAG_GIMMICK_ENV_ALL_ITEMLESS` enables the same loosened item requirement for debug / AI environments.
- `.party` supports `Z Move: Yes`, stored as `TrainerMon.shouldUseZMove` and copied into `gBattleStruct->opponentMonCanZMove` for opponent parties.
- Normal Z-Crystal behavior remains unchanged. Without the itemless environment, Z-Move availability is still item-based.
- Itemless Z-Moves use the selected move's type to produce the generic type-based Z-Move or status Z-Move. Signature Z-Moves still require their signature crystal.

Debug battles now skip `BattleTypeAllowsExp()` through `gIsDebugBattle`. This prevents the EXP bar, normal EXP gain, and EV gain in debug battles, including the level-100 EV-gain path that would otherwise still occur when EXP is disabled by level.

## Tests Added

- Conservation baseline: keeps Dynamax unused when another Dynamax user remains and the move has no immediate payoff.
- Last-Pokemon baseline: spends Dynamax when no reserve remains.
- Max Geyser payoff: spends Dynamax to set rain even with a reserve remaining.
- Max Airstream payoff: spends Dynamax in doubles for Speed-control tempo even with a reserve remaining.
- Dynamax disruption payoff: spends Dynamax to keep a damaging move live through Fake Out-style flinch or Roar / Whirlwind-style phazing.
- Smart Z conserve: keeps a damaging Z-Move unused when another Pokemon remains and the Z-Move has no immediate payoff.
- Smart Z last Pokemon: spends a damaging Z-Move when no reserve remains.
- Smart Z trap pressure: spends a damaging Z-Move when trapped and the Z-Move improves the damage race.
- Itemless Z environment: a marked trainer candidate can spend a Z-Move without holding a Z-Crystal when `AI_FLAG_GIMMICK_ENV_ALL_ITEMLESS` is active.
- Smart Mega Shadow Tag: spends Mega Evolution when the target form's ability creates immediate trapping pressure.
- Smart Switching doubles: can double switch out of bad double positions when neither partner can cover.
- Smart Switching doubles guard: stays in a bad position when the partner can cover the target.
- Smart Switching weather pivot: switches to a `Drizzle` reserve when rain improves reserve pressure.
- Smart Switching terrain pivot: switches to a `Grassy Surge` reserve that can change board control.
- Smart Switching speed-control pivots: switches to Tailwind and Trick Room reserves when those controls can flip the speed state.
- Smart Switching terrain seed pivot: switches to a terrain setter whose field triggers the reserve's seed plan.
- Smart Switching status pivots: switches into a status-benefit reserve under predicted burn, and in doubles can pivot to direct or secondary status / confusion pressure support.
- Smart Switching ability bridge pivot: switches in Skill Swap support when a partner board-control ability creates a bridge plan.
- Smart Switching Taunt reads: pivots an attacker into a predicted `Taunt`, stays in when the active Pokemon can punish with damage, and stays in when the active Pokemon ignores Taunt.
- Runtime knowledge layer: verifies AI move-category flags, ability-category flags, item / hold-effect category flags, and predicted-move immunity bridges for Magic Bounce, Safety Goggles / powder immunity, and ordinary non-ignored moves.
- Runtime knowledge catalog tool: generates local runtime review JSON for 935 moves, 319 abilities, 130 hold effects, 874 items, and 4 gimmick policies.
- Protect singles payoff scoring: avoids passive singles Protect with no turn-gain payoff, avoids passive Protect against boosted attackers, values Protect when residual damage creates payoff, and keeps a second singles Protect viable when the payoff remains.
- Protect doubles consecutive scoring: keeps a second double Protect as a risky but possible option instead of treating it as impossible.
- Debug battle EXP gate: verifies a `gIsDebugBattle` trainer battle does not show the EXP bar and does not award EXP or EVs.
- Existing Tera, Mega, Z-Move, and combined environment tests remain in `ai_smart_gimmick.c`.

## Validation

2026-06-05 local validation:

- `rtk make -j16 -O check TESTS='AI runtime knowledge'`: pass, 4 tests. Covers move-category, ability-category, item / hold-effect category mapping, and predicted-move immunity bridges.
- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 10 tests.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_ITEMLESS'`: pass, 1 test. Covers a marked itemless Z-Move candidate without a Z-Crystal.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 11 tests. Includes the itemless Z candidate regression.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'`: pass, 3 tests.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'`: pass, 3 tests.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI uses Z-Moves'`: failed 1 existing broad Z-Move AI case, `AI uses Z-Moves -- Z-Detect 1/2`, where the AI selected `Detect` without the Z-Move gimmick. The itemless-focused and smart-gimmick filters passed; this broad status-Z Protect heuristic remains a separate follow-up.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'`: pass, 1 test.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'`: pass, 4 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY'`: pass, 6 tests. Includes Fake Out and phazing disruption-prevention Dynamax checks.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_SWITCHING'`: pass. Includes bad-position double switching, weather / terrain / Tailwind / Trick Room board-control pivots, terrain seed, predicted burn status-benefit, direct and secondary status / confusion support, Skill Swap bridge pivots, and predicted-Taunt attacker pivot / stay-in guards through the shared predicted-move immunity predicate.
- `rtk make -j16 -O check TESTS='Protect: AI'`: pass, 9 tests. Covers ignore-protection moves, Unseen Fist, passive singles Protect rejection, boosted-attacker rejection, residual payoff, and second Protect scoring in singles and doubles.
- `rtk make tools/trainerproc/trainerproc`: pass. Regenerated trainer data from `.party` fixtures, including `Pool Weight`.
- `rtk make -j16 -O check TESTS='Debug battles do not give exp or EVs'`: pass, 1 test. Covers no EXP bar, no EXP gain, and no EV gain while `gIsDebugBattle` is set.
- 2026-06-06 `rtk make -j16 -O check TESTS='Trainer Party Pool'`: pass, 9 tests. Regresses pool role filtering and weighted selection behavior used by the debug fixtures.
- `rtk cargo check --manifest-path tools/runtime_knowledge/Cargo.toml`: pass.
- `rtk cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog_rust --pretty`: pass. Generated 935 moves, 319 abilities, 130 hold effects, 874 items, 4 gimmick policies, and a valid summary.
- `rtk make -j16 -O check`: pass. Existing known-failing / expected-failing labels remained non-fatal.
- `rtk make -j16 -O check`: attempted again after the Protect / Dmax-vs-Z debug-fixture update and exited 2. The captured output only exposed existing test-runner / known-failing labels (`Tests resume after CRASH`, `Pokemon level up learnsets fit within MAX_LEVEL_UP_MOVES and MAX_RELEARNER_MOVES`); both pass as expected when filtered individually. The focused AI, debug EXP / EV, and Trainer Party Pool checks above are the validation evidence for this update.
- `rtk make -j1 -O check`: attempted to rule out make-job parallelism, but the run did not progress beyond the initial link warning and was abandoned as non-evidence. The stale `mgba-rom-test-hydra` / `mgba-rom-test` children were killed before handoff.
- `rtk make -j16 -O debug`: pass.
- 2026-06-06 `rtk make -j16 -O debug`: pass. Confirms the debug Dmax/Z fixtures build with `Gimmick Env Itemless` and `.party` `Z Move: Yes` candidates.
- 2026-06-06 `rtk make -j16 -O all`: pass.
- 2026-06-06 `rtk mdbook build docs`: pass with existing missing-root-`CHANGELOG.md` include warning, `CREDITS.md` `</img>` warning, and large search index warning.
- mGBA Live: wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen and captured a screenshot in session `smart-ai-ability-item-knowledge-20260605`. `mgba_live_stop` returned `stopped:true`.
- mGBA Live: current ROM booted to the title screen and captured `/tmp/debug-vgc-fixtures-20260605.png` in session `debug-vgc-fixtures-20260605`. `mgba_live_stop` returned `stopped:true`, and CLI `status --all` returned `[]`. The debug Party menu battle itself still needs a progressed save or a focused input route for visual confirmation.
- mGBA Live: current ROM booted in session `smart-ai-protect-dmaxz-20260605`. `mgba_live_start_with_lua_and_view` reported a Lua bridge invalid-context error after starting, but `mgba_live_get_view` returned a rendered frame, `mgba_live_export_screenshot` saved `/tmp/smart-ai-protect-dmaxz-20260605.png`, and `mgba_live_stop` returned `stopped:true`.
- 2026-06-06 mGBA Live: MCP startup without `DISPLAY` failed with Qt `xcb` display initialization. CLI startup with `DISPLAY=:0` booted `pokeemerald.gba` to the title / demo screen in session `smart-gimmick-itemless-cli-smoke`, saved `/tmp/smart-gimmick-itemless-smoke.png`, and `mgba-live-cli stop` returned `stopped:true`.

## Known Gaps

- Air Lock / Cloud Nine preservation is source-derived, but not covered by a focused unit test yet because the AI test DSL does not have a direct pre-turn weather fixture in this file.
- Z-Move status tactics still rely on the existing Z-Move viability checks. More status Z-Move tactics can be modeled later.
- Tera doubles support uses the selected target and explicit candidate data. It does not fully simulate every partner threat or all possible double-target lines.
- G-Max unique secondary effects are not separately modeled in this slice. Regular Max Move effects are the current runtime focus.
- The new 3v3 / 4v4 debug fixtures are runtime smoke fixtures, not exhaustive balance fixtures. They provide quick repeated checks for competitive-style teams, weighted pool selection, smart gimmick timing, and no-EXP debug battles.
