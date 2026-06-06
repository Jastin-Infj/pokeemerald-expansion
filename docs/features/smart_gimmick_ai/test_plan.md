# Smart Gimmick AI Test Plan

## Required Validation

| Check | Command | Status |
| --- | --- | --- |
| Focused runtime knowledge tests | `rtk make -j16 -O check TESTS='AI runtime knowledge'` | Pass on 2026-06-05; 4 tests passed. Covers move-category, ability-category, item / hold-effect category mapping, and predicted-move immunity bridges. |
| Focused smart gimmick tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'` | Pass on 2026-06-06; 13 tests passed. Includes Z-Crystal-based all-gimmick Z-Move coverage plus Dynamax / Mega late-commit regressions. |
| Focused smart Dynamax environment tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY'` | Pass on 2026-06-06; 7 tests passed. Covers conservation, last-Pokemon use, one-reserve low-HP late commit, Max Move payoff, Fake Out disruption prevention, and phazing disruption prevention. |
| Focused smart Z tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'` | Pass on 2026-06-06; 4 tests passed. Adds one-reserve low-HP late commit. |
| Focused smart Mega tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'` | Pass on 2026-06-05; 1 test passed. |
| Focused all-gimmick environment tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_ALL'` | Pass on 2026-06-06; 4 tests passed. Covers held Z-Crystal use, doubles Tera, full-HP setup-turn Mega delay, and low-HP one-reserve Mega late commit. |
| Focused smart Tera tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'` | Pass on 2026-06-05; 4 tests passed. |
| Focused smart switching tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_SWITCHING'` | Pass on 2026-06-06. Covers double-position switching, partner-cover guard, weather / terrain reserve pivots, Tailwind / Trick Room reserve pivots, terrain seed plans, status-benefit pivots, direct and secondary status / confusion support pivots, Skill Swap bridge pivots, predicted-Taunt attacker pivots / stay-in guards, and the double-switch execution guard through the shared predicted-move immunity predicate. |
| Focused Protect scoring tests | `rtk make -j16 -O check TESTS='Protect: AI'` | Pass on 2026-06-05; 9 tests passed. Covers ignore-protection moves, Unseen Fist, passive singles Protect rejection, boosted-attacker rejection, residual payoff, and second Protect scoring in singles and doubles. |
| Debug trainer fixture generation | `rtk make tools/trainerproc/trainerproc`; `rtk make -j16 -O debug` | Pass on 2026-06-06 after adding `Battle Gimmick Single` / `Battle Gimmick Double`. |
| Debug battle EXP / EV gate | `rtk make -j16 -O check TESTS='Debug battles do not give exp or EVs'` | Pass on 2026-06-06; 1 test passed. Confirms `gIsDebugBattle` suppresses the EXP bar, EXP gain, and EV gain. |
| Trainer Party Pool regression | `rtk make -j16 -O check TESTS='Trainer Party Pool'` | Pass on 2026-06-06; 9 tests passed. Covers role-filtered pool selection and weighted selection used by the debug battle fixtures. |
| Runtime knowledge catalog tool | `rtk cargo check --manifest-path tools/runtime_knowledge/Cargo.toml`; `rtk cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog_rust --pretty`; JSON parse of `/tmp/runtime_knowledge_catalog_rust/*.json` | Pass on 2026-06-05. Generated valid catalogs for 935 moves, 319 abilities, 130 hold effects, 874 items, 4 gimmick policies, and `summary.json`. |
| Full battle / runtime checks | `rtk make -j16 -O check` | Historical pass on 2026-06-05 before the final Protect / Dmax-vs-Z debug update. Re-attempt after the final update exited 2; the visible output only showed existing test-runner / known-failing labels (`Tests resume after CRASH`, `Pokemon level up learnsets fit within MAX_LEVEL_UP_MOVES and MAX_RELEARNER_MOVES`), and both filtered checks return 0 individually. Focused checks above are the accepted evidence for this update. A `rtk make -j1 -O check` re-attempt did not progress beyond the initial link warning and was abandoned as non-evidence; stale `mgba-rom-test-hydra` / `mgba-rom-test` children were killed. |
| Broad existing Z-Move AI filter | `rtk make -j16 -O check TESTS='AI uses Z-Moves'` | Failed on 2026-06-06 in existing `AI uses Z-Moves -- Z-Detect 1/2`: expected Z-Move, got no gimmick. Focused smart-gimmick checks pass; this status-Z Protect heuristic is tracked as separate follow-up evidence. |
| Normal ROM build | `rtk make -j16 -O all` | Pass on 2026-06-06, including after the late-commit smart-gimmick checks and debug audit fixtures. |
| Docs build | `rtk mdbook build docs` | Pass on 2026-06-06 with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| mGBA Live smoke | Boot current ROM and capture one screenshot / input state | Pass on 2026-06-06. MCP startup without `DISPLAY` previously failed with Qt `xcb` display initialization. For the late-commit / audit-fixture update, sandboxed CLI startup first failed because it could not create `~/.mgba-live-mcp/runtime/sessions/smart-gimmick-late-audit-20260606`; rerunning with approval and `DISPLAY=:0` booted `pokeemerald.gba` to the title screen, exported `/tmp/smart-gimmick-late-audit-20260606.png`, `mgba-live-cli stop` returned `stopped:true`, and `status --all` returned `[]`. This was a boot smoke only; the new `Battle Gimmick Single` / `Battle Gimmick Double` menu entries still need manual progression from a debug-enabled save. |

## Manual Runtime Checks

### Debug Party Battle Fixtures

Build the debug ROM path first:

```bash
rtk make -j16 -O debug
```

In-game, open the overworld debug menu with `R + START` when `DEBUG_OVERWORLD_MENU` is enabled and `DEBUG_OVERWORLD_IN_MENU` is `FALSE`. Then use `Party`:

- `Battle 3v3 Single`: starts a level-50 3v3 singles battle. Player side is fixed to `Dragonite`, `Gholdengo`, and `Garchomp`. AI side is selected from the `Ladder` 6-Pokemon weighted pool with 3 chosen Pokemon.
- `Battle 4v4 Double`: starts a level-50 4v4 doubles battle. Player side is fixed to `Incineroar`, `Rillaboom`, `Flutter Mane`, and `Urshifu-Rapid-Strike`. AI side is selected from the `VGC Test` 7-Pokemon weighted pool with 4 chosen Pokemon.
- `Battle Dmax/Z Single`: starts a level-50 3v3 singles battle. Player side is a Z-Move test team. AI side is a Dynamax / Gigantamax / Z-Crystal pressure team with smart gimmick flags.
- `Battle Dmax/Z Double`: starts a level-50 4v4 doubles battle. Player side is a VGC-style Z-Move test team. AI side is a Dynamax / Gigantamax / Z-Crystal doubles team with Tailwind and weather pressure.
- `Battle Gimmick Single`: starts a singles availability audit. Player side is passive. AI side leads `Gengarite` Mega and has reserve `Mimikium Z`, Gigantamax `Charizard`, and Normal Tera `Dragonite`.
- `Battle Gimmick Double`: starts a doubles availability audit. Player side is passive. AI side leads Gigantamax `Charizard` plus `Electrium Z` `Tapu Koko`, with reserve `Gengarite` Mega and Normal Tera `Dragonite`.

Expected results:

- The battle starts from the Party debug menu without truncating the 4v4 player side to 3 Pokemon.
- The opposing side is generated from the `.party` pool, respecting `Party Size`, `Pool Rules`, tags, and `Pool Weight`.
- If `B_POOL_SETTING_CONSISTENT_RNG` is `FALSE`, repeated starts can produce different opposing selections. If it is `TRUE`, selection is deterministic for the same save OTID and trainer pointer.
- Fainting opposing Pokemon does not display the EXP bar and does not grant EXP or EVs.
- AI behavior should use the configured `Smart Trainer`, `Prediction`, `Smart Gimmick`, `Know Opponent Party`, and `Powerful Status` flags.
- In the Dmax/Z fixtures, the debug battle grants Z-Power Ring and Dynamax Band access through `gDebugGimmickAccessFlags`, so the player can test Z-Move pressure into AI Dynamax / Gigantamax timing without requiring those Bag key items. Z-Moves still require Z-Crystals. These fixtures are competitive timing fixtures and do not contain an AI Mega candidate.
- In the Gimmick audit fixtures, the debug battle grants all gimmick access through `gDebugGimmickAccessFlags`. Use these when the question is "can the opponent use Mega / Z / Dynamax / Tera at all?" rather than "does smart timing decide to conserve?"
- `Battle Gimmick Single` expected quick check: the opposing lead `Gengar` should be able to Mega Evolve early. Continue by fainting or forcing through reserves to inspect `Mimikyu` Z-Move, Gigantamax `Charizard`, and `Dragonite` Tera availability.
- `Battle Gimmick Double` expected quick check: the opposing lead pair exposes a Dynamax / Gigantamax candidate and a Z-Move candidate immediately. Continue through reserves to inspect `Gengar` Mega and `Dragonite` Tera availability.
- Double battles should not hit the `IsValidSwitchIn()` ASSERT when the AI tries to reposition. If both active AI Pokemon want to switch but only one legal reserve exists, or a partner already reserved the best target, the second switch request should be canceled or redirected to a legal reserve.
- Opponent gimmicks are not forced to fire immediately in smart-timing fixtures. They are expected to activate only when the smart timing checks find a concrete payoff: last-Pokemon pressure, one-reserve low-HP late commit, KO conversion, Max Move board control, disruption prevention, useful Mega / Tera form timing, or a valid Z-Move payoff.

The current mGBA Live check only reached title-screen boot for these fixtures. A progressed save or focused input route is still needed to visually confirm the new Party debug menu entries and battle intro through mGBA Live.

Use the debug trainer battle flow and Trainer 1 IDs documented in `docs/tutorials/ai_flags.md`.

- `855`: no-payoff Dynamax conserve.
- `856`: last-Pokemon Dynamax.
- `858`: Z-Move viability and smart timing.
- `859`: offensive Tera.
- `860`: Dynamax + Tera environment with separate candidates.
- `861`: all-gimmick single battle.
- `862`: all-gimmick double battle.
- `863`: Smart Switching debug double battle.

For double switching behavior, use any debug double battle with `AI_FLAG_SMART_SWITCHING` and a passive pinned AI lead pair. Expected behavior is that the AI can hard switch one or both active Pokemon if neither active slot has useful pressure and the partner cannot cover the threatened position.

For board-control switching, use an AI reserve that can change the field or speed state. Expected examples:

- `Pelipper` / `Drizzle` can be chosen over staying in when rain creates stronger reserve pressure.
- `Rillaboom` / `Grassy Surge` can be chosen when Grassy Terrain plus Grass pressure changes the board.
- A reserve with `Tailwind` can be chosen when doubled Speed flips relevant matchups.
- A reserve with `Trick Room` can be chosen when the AI side is slower and the current room state does not already cover that plan.
- A reserve terrain setter holding the matching seed can be chosen when the field activates a stat plan.
- A reserve with status-benefit ability or `Facade` / `Psycho Shift` can be chosen into a predicted status move.
- In doubles, a reserve with direct status pressure, secondary status / confusion pressure, status prevention, status cure, or Skill Swap-style ability bridge support can be chosen when the active slot is pinned and the bench role can create board pressure.

For the `863` Smart Switching debug fixture:

- Target the AI left `Zigzagoon` with `Taunt`. Expected: when prediction fires and `Zigzagoon` cannot punish in place, the AI can pivot into `Gengar` to attack the Taunt user instead of staying as disabled utility.
- Target the AI left `Zigzagoon` with `Will-O-Wisp`. Expected: when prediction fires, `Guts` `Ursaring` is an intended burn-benefit reserve.
- Pressure the AI left slot with an unfavorable Fighting or strong-damage line. Expected: the AI can pivot toward `Slowbro`, whose `Scald` / `Psybeam` pressure represents secondary-status / confusion-style board support.
- Negative guard to keep in mind while testing custom parties: the AI should not switch only because a Pokemon was Taunted. It should stay if the active Pokemon can already punish with damage, if Taunt is blocked by `Aroma Veil` / Gen 6+ `Oblivious`, or if an enabled Gen 5+ `Mental Herb` can absorb the Taunt.

For Dynamax disruption timing, use a Dynamax-capable AI active Pokemon with a reserve remaining and a selected damaging move. Expected examples:

- If the known / predicted opposing move is `Fake Out` or another relevant flinch move, smart Dynamax can spend to keep the selected attack live.
- If the known / predicted opposing move is `Roar` or `Whirlwind`, smart Dynamax can spend to avoid losing the active Pokemon to phazing.

For Protect timing, use singles and doubles battles where the AI has `Protect` plus a predicted damaging move. Expected examples:

- In singles, the AI should usually avoid passive Protect if it only gives the opponent a free turn, especially after the opponent has already boosted.
- In singles, the AI can use Protect when it gains a real turn payoff: residual target damage, incoming Wish, Poison Heal / Leftovers / Black Sludge recovery, Substitute threshold, choice-lock scouting, or a next-turn Disable / Encore line.
- A second consecutive Protect is not forbidden. It should be penalized for reduced success odds, but can remain viable when the same payoff still matters.
- In doubles, second Protect should also remain available as a risky positioning option under pressure rather than being removed from consideration.

## Accepted Risks Before Deeper Tuning

- G-Max unique effects need separate scoring if they become important to trainer fixtures.
- Air Lock / Cloud Nine preservation still needs a focused pre-weather AI unit once a clean weather fixture exists for this file.
- Smart Z-Move status tactics still depend on the existing status Z-Move checks.
- Broad existing `AI uses Z-Moves` validation currently exposes a separate `Z-Detect` status-Z timing gap. Keep the focused `AI_FLAG_GIMMICK_ENV` and `AI_FLAG_SMART_Z_MOVE` checks covered until that broader heuristic is retuned.
- The AI still evaluates one selected move / target at reconsider time; it does not perform a full turn-tree search.
- Protect scoring is payoff-based, not a full opponent turn-tree read. It recognizes common turn-gain reasons and consecutive-use risk, but it does not yet solve every PP-stall, double-target, or "Protect to bait a switch" line.
- Double switching and board-control pivoting are heuristic position checks, not a full VGC turn solver. Board-control pivots can identify weather, terrain, Tailwind, and Trick Room roles, but they do not yet search full multi-turn lines such as "switch setter now, protect partner next turn, then reposition again."
- Status and ability-bridge switching is intentionally conservative in singles. Non-immediate utility pivots are double-battle only in this slice; singles keep only immediate status-benefit pivots plus the existing bad-odds / bad-matchup gates.

## GitHub Actions

Long GitHub Actions were not re-waited for this local handoff. Local `check`, `all`, docs build, and mGBA Live smoke provide the branch evidence for this update.
