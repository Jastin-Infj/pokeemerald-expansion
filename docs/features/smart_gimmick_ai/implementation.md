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

- Last available Pokemon, or one-reserve late-commit pressure when the active Pokemon is already low on HP or under immediate KO threat.
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

The Dynamax policy is intentionally not "use the strongest attack immediately." The current runtime model treats Dynamax as a board-presence resource: HP scaling can keep an active Pokemon on the field, Max Move effects can change Speed, weather, terrain, Defense, Special Defense, Attack, Special Attack, or opponent-side pressure, and Dynamax can deny disruption such as flinching or forced switching. This matches the Pokemon Wiki mechanics summary for Dynamax's HP increase, 3-turn duration, flinch / forced-switch immunity, Max Guard, and Max Move side effects, the official Sword / Shield Max Move explanation, and Victory Road VGC reports where Dynamax is discussed through Fake Out denial, Max Airstream tempo, Max Steelspike / Max Quake defensive value, and status / positioning pressure.

For Mega / Ultra Burst, `ShouldUseSmartMega()` now looks at the target form before spending the gimmick. It allows immediate use for target-form ability payoffs such as trapping or weather control, estimated Speed flips, meaningful defensive improvement under KO pressure, meaningful attacking-stat improvement for the selected damaging move, and one-reserve low-HP late-commit pressure. It still delays setup turns when there is no immediate pressure, and it can preserve `Air Lock` / `Cloud Nine` during active weather if the target form has no stronger immediate payoff.

For Z-Moves, `ShouldUseSmartZMove()` now wraps the existing Z-Move viability checks. Status Z-Moves keep their tactical checks. Damaging Z-Moves are conserved unless the AI is on its last Pokemon, has one reserve left and the active Pokemon is low on HP or under KO threat, the Z-Move converts the selected move into a KO, improves the damage race under immediate KO or trap pressure, or protects a low-accuracy KO line.

For double battle switching, `ShouldSwitchIfDoublePositionBad()` adds a VGC-style positioning check under `AI_FLAG_SMART_SWITCHING`. The AI may hard switch when the active Pokemon has no meaningful pressure into either opposing slot, is threatened by either opposing slot, has enough HP to be worth preserving, and its partner cannot cover the position. The check stays out of `AI_FLAG_SEQUENCE_SWITCHING`, respects existing no-switch gates, avoids overriding the existing Intimidate-blocker contract, and allows double switches when both active Pokemon are pinned.

Double-battle switch execution validates the final party index before emitting `B_ACTION_SWITCH` or returning a post-KO `ChoosePokemon` result. This fixes a debug ASSERT path where the smart-switch fallback could finish with no valid candidate (`-1`, `PARTY_SIZE`, a fainted mon, an active partner's mon, or a partner-reserved mon) and then pass that invalid index into `Cmd_getswitchedmondata()` / `IsValidSwitchIn()`. If an optional mid-turn switch has no valid target after validation, the AI clears the switch request and falls back to a normal move action instead of crashing the battle script. Post-KO / forced controller selection now rechecks the advanced result and falls back to the first valid reserve.

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

The debug Party menu exposes focused runtime fixtures and weighted gauntlet battles:

- `Battle 3v3 Single` builds a level-50 player team of `Dragonite`, `Gholdengo`, and `Garchomp`, then pits it against a 3-Pokemon AI side selected from a 6-Pokemon weighted singles pool.
- `Battle 4v4 Double` builds a level-50 player team of `Incineroar`, `Rillaboom`, `Flutter Mane`, and `Urshifu-Rapid-Strike`, then pits it against a 4-Pokemon AI side selected from a 7-Pokemon weighted doubles pool.
- `Battle Dmax/Z Single` builds a level-50 player-side Z-Move team, then pits it against a smart AI Dynamax / Gigantamax / Z-Crystal team.
- `Battle Dmax/Z Double` builds a level-50 player-side VGC-style Z-Move team, then pits it against a smart AI Dynamax / Gigantamax / Z-Crystal doubles team with Tailwind / weather pressure.
- `Battle Gimmick Single` builds a passive player side and an opponent audit party with lead `Gengarite` Mega, reserve `Mimikium Z`, reserve Gigantamax `Charizard`, and reserve Normal Tera `Dragonite`. It grants all debug gimmick access to prove the opponent's source data and runtime gates can expose every gimmick.
- `Battle Gimmick Double` builds a passive double player side and an opponent audit party with active Gigantamax `Charizard` plus `Electrium Z` `Tapu Koko`, then reserve `Gengarite` Mega and Normal Tera `Dragonite`. It is an availability audit, separate from the smarter Dmax/Z timing fixtures.
- `Gauntlet Mega S` / `Gauntlet Mega D` build level-50 3v3 singles and 4v4 doubles battles with Mega-only access.
- `Gauntlet MegaZ S` / `Gauntlet MegaZ D` build level-50 3v3 singles and 4v4 doubles battles with Gen 7-style Mega + Z-Move access.
- `Gauntlet Dmax S` / `Gauntlet Dmax D` build level-50 3v3 singles and 4v4 doubles battles with Dynamax-only access.
- `Gauntlet Tera S` / `Gauntlet Tera D` build level-50 3v3 singles and 4v4 doubles battles with Tera-only access.

These AI fixtures enable `Smart Trainer`, `Prediction`, `Smart Gimmick`, `Know Opponent Party`, and `Powerful Status` AI flags. The gauntlet AI fixtures also include `Smart Switching`, `Smart Mon Choices`, and explicit `Omniscient` so the AI can read with full player moves / items / abilities while also knowing the player party species. The pool entries carry `Pool Weight` plus role tags such as `Lead`, `Ace`, and `Support`, so the pool-based fixtures also exercise Trainer Party Pool role filtering and weighted selection.

The gauntlet battles are intentionally unfairer than the earlier audit fixtures. Player and AI sides are both generated from weighted local pools on each start, but the AI pools are biased toward restricted legends, mythicals, strong support, and high-value gimmick anchors. With `B_POOL_SETTING_CONSISTENT_RNG == FALSE`, repeated debug starts can produce different player and opponent teams. Mega and Mega+Z pools use `Lead` / `Ace` tags so the selected party reliably contains the intended Z or Mega slot while still changing the supporting Pokemon.

Gimmick access is split from AI timing:

- `B_FLAG_GIMMICK_ACCESS_ALL` grants Mega Ring, Z-Power Ring, Dynamax Band, and charged Tera Orb access for a special ruleset.
- `B_FLAG_GIMMICK_ACCESS_MEGA_RING`, `B_FLAG_GIMMICK_ACCESS_Z_POWER_RING`, `B_FLAG_GIMMICK_ACCESS_DYNAMAX_BAND`, and `B_FLAG_GIMMICK_ACCESS_TERA_ORB` allow individual formats such as Mega-only, Tera-only, or Z + Dynamax.
- `HasGimmickAccess()` centralizes Bag item checks and runtime/debug access overrides. Mega, Z, Ultra Burst, Dynamax, and Tera now read the same access layer.
- Debug Party Dmax/Z fixtures pass `GIMMICK_ACCESS_Z_POWER_RING | GIMMICK_ACCESS_DYNAMAX_BAND` through `gDebugGimmickAccessFlags` instead of using an AI flag as a ruleset unlock.
- `.party` supports `Z Move: Yes`, stored as `TrainerMon.shouldUseZMove` and copied into `gBattleStruct->opponentMonCanZMove` for opponent parties. Ordinary Z-Move availability still requires a matching Z-Crystal and Z-Power access.
- The earlier itemless Z-Move unlock was removed so AI flags remain tactical timing / ruleset-intent hints, not replacement access items.

Opponent gimmick use is not disabled. AI-side Mega / Z-Move / Dynamax / Tera availability still comes from trainer party data, held items, and the battle gimmick gates, and the focused AI tests confirm that the opponent can spend those gimmicks. The earlier `Battle Dmax/Z Single` and `Battle Dmax/Z Double` fixtures did not include an AI Mega candidate, and their Z-Move users were not both lead-visible, so they were poor "is the opponent allowed to use every gimmick?" audits. The new `Battle Gimmick Single` and `Battle Gimmick Double` fixtures are explicit availability audits. Under `AI_FLAG_SMART_GIMMICK_TIMING`, trainer gimmick data remains permission rather than an immediate command: Dynamax, Z-Move, Mega, and Tera can be conserved until the selected turn has KO, board-control, defensive, setup, late-commit, or last-Pokemon value.

Debug battles now skip `BattleTypeAllowsExp()` through `gIsDebugBattle`. This prevents the EXP bar, normal EXP gain, and EV gain in debug battles, including the level-100 EV-gain path that would otherwise still occur when EXP is disabled by level.

## Tests Added

- Conservation baseline: keeps Dynamax unused when another Dynamax user remains and the move has no immediate payoff.
- Last-Pokemon baseline: spends Dynamax when no reserve remains.
- One-reserve late-commit baseline: spends Dynamax when the active Pokemon is low on HP and only one reserve remains, even if the opponent is passive.
- Max Geyser payoff: spends Dynamax to set rain even with a reserve remaining.
- Max Airstream payoff: spends Dynamax in doubles for Speed-control tempo even with a reserve remaining.
- Dynamax disruption payoff: spends Dynamax to keep a damaging move live through Fake Out-style flinch or Roar / Whirlwind-style phazing.
- Smart Z conserve: keeps a damaging Z-Move unused when another Pokemon remains and the Z-Move has no immediate payoff.
- Smart Z last Pokemon: spends a damaging Z-Move when no reserve remains.
- Smart Z one-reserve low-HP: spends a damaging Z-Move before it is stranded by over-conservation.
- Smart Z trap pressure: spends a damaging Z-Move when trapped and the Z-Move improves the damage race.
- Gimmick access separation: `AI_FLAG_GIMMICK_ENV_ALL` keeps Z-Move timing tactical while Z-Move availability remains tied to Z-Crystals and Z-Power access.
- Smart Mega Shadow Tag: spends Mega Evolution when the target form's ability creates immediate trapping pressure.
- Smart Mega low-HP late-commit: spends Mega Evolution with one reserve remaining, while a separate setup-delay test still verifies full-HP no-pressure conservation.
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

Local validation highlights:

- `rtk make -j16 -O check TESTS='AI runtime knowledge'`: pass, 4 tests. Covers move-category, ability-category, item / hold-effect category mapping, and predicted-move immunity bridges.
- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 10 tests.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 11 tests. Includes the Z-Crystal-based all-gimmick Z-Move regression after access flags were split from AI flags.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 13 tests. Includes Dynamax / Z / Mega late-commit regressions and the all-gimmick environment checks.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY'`: pass, 7 tests. Adds the one-reserve low-HP Dynamax late-commit regression.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_ALL'`: pass, 4 tests. Adds the one-reserve low-HP Mega late-commit regression while keeping full-HP setup-turn delay.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'`: pass, 3 tests.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'`: pass, 4 tests. Adds the one-reserve low-HP Z-Move late-commit regression.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI uses Z-Moves'`: failed 1 existing broad Z-Move AI case, `AI uses Z-Moves -- Z-Detect 1/2`, where the AI selected `Detect` without the Z-Move gimmick. The smart-gimmick filters passed; this broad status-Z Protect heuristic remains a separate follow-up.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'`: pass, 1 test.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'`: pass, 4 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY'`: historical pass, 6 tests. Includes Fake Out and phazing disruption-prevention Dynamax checks before the late-commit regression was added.
- 2026-06-06 `rtk make -j16 -O check TESTS='AI_FLAG_SMART_SWITCHING'`: pass. Includes bad-position double switching, weather / terrain / Tailwind / Trick Room board-control pivots, terrain seed, predicted burn status-benefit, direct and secondary status / confusion support, Skill Swap bridge pivots, predicted-Taunt attacker pivot / stay-in guards, and the double-switch execution guard through the shared predicted-move immunity predicate.
- `rtk make -j16 -O check TESTS='Protect: AI'`: pass, 9 tests. Covers ignore-protection moves, Unseen Fist, passive singles Protect rejection, boosted-attacker rejection, residual payoff, and second Protect scoring in singles and doubles.
- `rtk make tools/trainerproc/trainerproc`: pass. Regenerated trainer data from `.party` fixtures, including `Pool Weight`.
- 2026-06-06 `rtk make tools/trainerproc/trainerproc`: pass. Regenerated debug trainer data for `Battle Gimmick Single` / `Battle Gimmick Double`.
- 2026-06-06 `rtk make tools/trainerproc/trainerproc`: pass after adding the 8 weighted gauntlet debug battles.
- 2026-06-06 `rtk make -j16 -O check TESTS='Debug battles do not give exp or EVs'`: pass, 1 test. Covers no EXP bar, no EXP gain, and no EV gain while `gIsDebugBattle` is set.
- 2026-06-06 `rtk make -j16 -O check TESTS='Trainer Party Pool'`: pass, 9 tests. Regresses pool role filtering and weighted selection behavior used by the debug fixtures.
- 2026-06-06 `rtk make -j16 -O check TESTS='Trainer Party Pool'`: pass, 9 tests after adding the gauntlet pools. Covers weighted eligible candidates, tag constraints, runtime RNG variation, custom rules, and fallback.
- `rtk cargo check --manifest-path tools/runtime_knowledge/Cargo.toml`: pass.
- `rtk cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog_rust --pretty`: pass. Generated 935 moves, 319 abilities, 130 hold effects, 874 items, 4 gimmick policies, and a valid summary.
- `rtk make -j16 -O check`: pass. Existing known-failing / expected-failing labels remained non-fatal.
- `rtk make -j16 -O check`: attempted again after the Protect / Dmax-vs-Z debug-fixture update and exited 2. The captured output only exposed existing test-runner / known-failing labels (`Tests resume after CRASH`, `Pokemon level up learnsets fit within MAX_LEVEL_UP_MOVES and MAX_RELEARNER_MOVES`); both pass as expected when filtered individually. The focused AI, debug EXP / EV, and Trainer Party Pool checks above are the validation evidence for this update.
- `rtk make -j1 -O check`: attempted to rule out make-job parallelism, but the run did not progress beyond the initial link warning and was abandoned as non-evidence. The stale `mgba-rom-test-hydra` / `mgba-rom-test` children were killed before handoff.
- `rtk make -j16 -O debug`: pass.
- 2026-06-06 `rtk make -j16 -O debug`: pass after adding `Battle Gimmick Single` / `Battle Gimmick Double`.
- 2026-06-06 `rtk make -j16 -O debug`: pass after adding the 8 weighted gauntlet debug battles.
- 2026-06-06 `rtk make -j16 -O debug`: pass. Confirms the debug Dmax/Z fixtures build with debug Z-Power / Dynamax access and `.party` `Z Move: Yes` candidates.
- 2026-06-06 `rtk make -j16 -O debug`: pass after the double-switch execution guard.
- 2026-06-06 `rtk make -j16 -O all`: pass.
- 2026-06-06 `rtk make -j16 -O all`: pass after adding late-commit smart-gimmick checks and debug audit fixtures.
- 2026-06-06 `rtk make -j16 -O all`: pass after the double-switch execution guard.
- 2026-06-06 `rtk make -j16 -O all`: pass after adding the 8 weighted gauntlet debug battles.
- 2026-06-06 `rtk mdbook build docs`: pass with existing missing-root-`CHANGELOG.md` include warning, `CREDITS.md` `</img>` warning, and large search index warning.
- 2026-06-06 `rtk mdbook build docs`: pass after the double-switch execution guard, with the same existing warnings.
- 2026-06-06 `rtk mdbook build docs`: pass after the late-commit and debug audit update, with the same existing warnings.
- 2026-06-06 `rtk mdbook build docs`: pass after documenting the gauntlet debug battles, with the same existing warnings.
- mGBA Live: wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen and captured a screenshot in session `smart-ai-ability-item-knowledge-20260605`. `mgba_live_stop` returned `stopped:true`.
- mGBA Live: current ROM booted to the title screen and captured `/tmp/debug-vgc-fixtures-20260605.png` in session `debug-vgc-fixtures-20260605`. `mgba_live_stop` returned `stopped:true`, and CLI `status --all` returned `[]`. The debug Party menu battle itself still needs a progressed save or a focused input route for visual confirmation.
- mGBA Live: current ROM booted in session `smart-ai-protect-dmaxz-20260605`. `mgba_live_start_with_lua_and_view` reported a Lua bridge invalid-context error after starting, but `mgba_live_get_view` returned a rendered frame, `mgba_live_export_screenshot` saved `/tmp/smart-ai-protect-dmaxz-20260605.png`, and `mgba_live_stop` returned `stopped:true`.
- 2026-06-06 mGBA Live: MCP startup without `DISPLAY` failed with Qt `xcb` display initialization. CLI startup with `DISPLAY=:0` booted `pokeemerald.gba` to the title / demo screen in session `smart-gimmick-access-cli-smoke`, saved `/tmp/smart-gimmick-access-smoke.png`, and `mgba-live-cli stop` returned `stopped:true`.
- 2026-06-06 mGBA Live: sandboxed CLI startup first failed because the tool could not create `~/.mgba-live-mcp/runtime/sessions/...`. Rerunning with approval and `DISPLAY=:0` booted `pokeemerald.gba` in session `smart-gimmick-switch-guard-20260606`, saved `/tmp/smart-gimmick-switch-guard-20260606.png`, and `mgba-live-cli stop` returned `stopped:true`. `status --all` returned `[]`. This was a boot smoke only; the exact debug double-battle ASSERT path still needs manual progression to the Party debug fixture.
- 2026-06-06 mGBA Live: sandboxed CLI startup first failed because it could not create `~/.mgba-live-mcp/runtime/sessions/smart-gimmick-late-audit-20260606`. Rerunning with approval and `DISPLAY=:0` booted `pokeemerald.gba` to the title screen, saved `/tmp/smart-gimmick-late-audit-20260606.png`, and `mgba-live-cli stop` returned `stopped:true`. `status --all` returned `[]`. This was a boot smoke only; the new `Battle Gimmick Single` / `Battle Gimmick Double` menu entries still need manual progression from a debug-enabled save.
- 2026-06-06 mGBA Live: sandboxed CLI startup first failed because it could not create `~/.mgba-live-mcp/runtime/sessions/smart-gimmick-gauntlet-20260606`. Rerunning with approval and `DISPLAY=:0` booted `pokeemerald.gba` to the title screen, saved `/tmp/smart-gimmick-gauntlet-20260606.png`, and `mgba-live-cli stop` returned `stopped:true`. `status --all` returned `[]`. This was a boot smoke only; the 8 gauntlet menu entries still need manual progression from a debug-enabled save.

## Strategy Sources

The runtime policy above was cross-checked against:

- [Pokemon Wiki / Pokemon battle consideration wiki Dynamax mechanics](https://poke-wiki.net/%E3%83%80%E3%82%A4%E3%83%9E%E3%83%83%E3%82%AF%E3%82%B9): 3-turn duration, switch cancellation, HP scaling by Dynamax level, flinch and forced-switch immunity, Max Guard, Max Move side effects, and double-battle side effects.
- [Official Pokemon Sword / Shield Dynamax and Max Moves page](https://swordshield.pokemon.com/en-us/gameplay/dynamaxing-max-moves/): Max Moves are powerful but also carry additional effects; status moves become Max Guard; held item freedom means the Dynamax choice depends on the battle state.
- Victory Road VGC writing: [Incineroar / Fake Out discussion](https://victoryroad.pro/2020/03/02/incineroar-vgc-column/) for Dynamax flinch immunity; [Durant team report](https://victoryroad.pro/2020/01/31/bingjie-dallas-report-finalist/) and [Series 10 report](https://victoryroad.pro/2021/07/20/series-10-introduction/) for Max Steelspike / Max Quake defensive value; [Corviknight / Max Airstream report](https://victoryroad.pro/2020/03/14/zach-kelly-ocic20-report/) for speed and positioning value; team reports as examples that Dynamax is frequently a board-control and survival resource, not only a damage button.
- Pokemon Battle DataBase / official usage-style data remains the preferred future source for species, item, and move priors. Pokemon Showdown articles remain excluded as strategy source material per project policy.

## Known Gaps

- Air Lock / Cloud Nine preservation is source-derived, but not covered by a focused unit test yet because the AI test DSL does not have a direct pre-turn weather fixture in this file.
- Z-Move status tactics still rely on the existing Z-Move viability checks. More status Z-Move tactics can be modeled later.
- Tera doubles support uses the selected target and explicit candidate data. It does not fully simulate every partner threat or all possible double-target lines.
- G-Max unique secondary effects are not separately modeled in this slice. Regular Max Move effects are the current runtime focus.
- The new 3v3 / 4v4 debug fixtures are runtime smoke fixtures, not exhaustive balance fixtures. They provide quick repeated checks for competitive-style teams, weighted pool selection, smart gimmick timing, and no-EXP debug battles.
