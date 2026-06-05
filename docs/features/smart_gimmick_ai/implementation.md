# Smart Gimmick AI Implementation

## Summary

The feature adds smart timing flags for battle gimmicks, expands Dynamax timing beyond immediate damage, adds Mega / Ultra Burst payoff checks, adds a smart conservation layer for damaging Z-Moves, and provides debug battle fixtures for competitive-style 3v3 singles and 4v4 doubles checks.

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

The debug Party menu now exposes two focused runtime fixtures:

- `Battle 3v3 Single` builds a level-50 player team of `Dragonite`, `Gholdengo`, and `Garchomp`, then pits it against a 3-Pokemon AI side selected from a 6-Pokemon weighted singles pool.
- `Battle 4v4 Double` builds a level-50 player team of `Incineroar`, `Rillaboom`, `Flutter Mane`, and `Urshifu-Rapid-Strike`, then pits it against a 4-Pokemon AI side selected from a 7-Pokemon weighted doubles pool.

Both AI fixtures enable `Smart Trainer`, `Prediction`, `Smart Gimmick`, `Know Opponent Party`, and `Powerful Status` AI flags. The pool entries carry `Pool Weight` plus role tags such as `Lead`, `Ace`, `Support`, `Weather Setter`, and `Weather Abuser`, so these fixtures also exercise Trainer Party Pool role filtering and weighted selection.

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
- Debug battle EXP gate: verifies a `gIsDebugBattle` trainer battle does not show the EXP bar and does not award EXP or EVs.
- Existing Tera, Mega, Z-Move, and combined environment tests remain in `ai_smart_gimmick.c`.

## Validation

2026-06-05 local validation:

- `rtk make -j16 -O check TESTS='AI runtime knowledge'`: pass, 4 tests. Covers move-category, ability-category, item / hold-effect category mapping, and predicted-move immunity bridges.
- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 10 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'`: pass, 3 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'`: pass, 1 test.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'`: pass, 4 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY'`: pass, 6 tests. Includes Fake Out and phazing disruption-prevention Dynamax checks.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_SWITCHING'`: pass. Includes bad-position double switching, weather / terrain / Tailwind / Trick Room board-control pivots, terrain seed, predicted burn status-benefit, direct and secondary status / confusion support, Skill Swap bridge pivots, and predicted-Taunt attacker pivot / stay-in guards through the shared predicted-move immunity predicate.
- `rtk make tools/trainerproc/trainerproc`: pass. Regenerated trainer data from `.party` fixtures, including `Pool Weight`.
- `rtk make -j16 -O check TESTS='Debug battles do not give exp'`: pass, 1 test. Covers no EXP bar, no EXP gain, and no EV gain while `gIsDebugBattle` is set.
- `rtk make -j16 -O check TESTS='Trainer Party Pool'`: pass, 9 tests. Regresses pool role filtering and weighted selection behavior used by the debug fixtures.
- `rtk cargo check --manifest-path tools/runtime_knowledge/Cargo.toml`: pass.
- `rtk cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog_rust --pretty`: pass. Generated 935 moves, 319 abilities, 130 hold effects, 874 items, 4 gimmick policies, and a valid summary.
- `rtk make -j16 -O check`: pass. Existing known-failing / expected-failing labels remained non-fatal.
- `rtk make -j16 -O debug`: pass.
- `rtk make -j16 -O all`: pass.
- `rtk mdbook build docs`: pass with existing warnings for missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, and large search index.
- mGBA Live: wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen and captured a screenshot in session `smart-ai-ability-item-knowledge-20260605`. `mgba_live_stop` returned `stopped:true`.
- mGBA Live: current ROM booted to the title screen and captured `/tmp/debug-vgc-fixtures-20260605.png` in session `debug-vgc-fixtures-20260605`. `mgba_live_stop` returned `stopped:true`, and CLI `status --all` returned `[]`. The debug Party menu battle itself still needs a progressed save or a focused input route for visual confirmation.

## Known Gaps

- Air Lock / Cloud Nine preservation is source-derived, but not covered by a focused unit test yet because the AI test DSL does not have a direct pre-turn weather fixture in this file.
- Z-Move status tactics still rely on the existing Z-Move viability checks. More status Z-Move tactics can be modeled later.
- Tera doubles support uses the selected target and explicit candidate data. It does not fully simulate every partner threat or all possible double-target lines.
- G-Max unique secondary effects are not separately modeled in this slice. Regular Max Move effects are the current runtime focus.
- The new 3v3 / 4v4 debug fixtures are runtime smoke fixtures, not exhaustive balance fixtures. They provide quick repeated checks for competitive-style teams, weighted pool selection, smart gimmick timing, and no-EXP debug battles.
