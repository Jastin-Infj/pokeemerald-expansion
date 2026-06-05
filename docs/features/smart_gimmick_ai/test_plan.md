# Smart Gimmick AI Test Plan

## Required Validation

| Check | Command | Status |
| --- | --- | --- |
| Focused runtime knowledge tests | `rtk make -j16 -O check TESTS='AI runtime knowledge'` | Pass on 2026-06-05; 4 tests passed. Covers move-category, ability-category, item / hold-effect category mapping, and predicted-move immunity bridges. |
| Focused smart gimmick tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'` | Pass on 2026-06-05; 10 tests passed. |
| Focused smart Dynamax environment tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY'` | Pass on 2026-06-05; 6 tests passed. Covers conservation, last-Pokemon use, Max Move payoff, Fake Out disruption prevention, and phazing disruption prevention. |
| Focused smart Z tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'` | Pass on 2026-06-05; 3 tests passed. |
| Focused smart Mega tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'` | Pass on 2026-06-05; 1 test passed. |
| Focused smart Tera tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'` | Pass on 2026-06-05; 4 tests passed. |
| Focused smart switching tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_SWITCHING'` | Pass on 2026-06-05. Covers double-position switching, partner-cover guard, weather / terrain reserve pivots, Tailwind / Trick Room reserve pivots, terrain seed plans, status-benefit pivots, direct and secondary status / confusion support pivots, Skill Swap bridge pivots, and predicted-Taunt attacker pivots / stay-in guards through the shared predicted-move immunity predicate. |
| Runtime knowledge catalog tool | `rtk cargo check --manifest-path tools/runtime_knowledge/Cargo.toml`; `rtk cargo run --manifest-path tools/runtime_knowledge/Cargo.toml -- --out /tmp/runtime_knowledge_catalog_rust --pretty`; JSON parse of `/tmp/runtime_knowledge_catalog_rust/*.json` | Pass on 2026-06-05. Generated valid catalogs for 935 moves, 319 abilities, 130 hold effects, 874 items, 4 gimmick policies, and `summary.json`. |
| Full battle / runtime checks | `rtk make -j16 -O check` | Pass on 2026-06-05. Existing known-failing / expected-failing test labels remained non-fatal. |
| Normal ROM build | `rtk make -j16 -O all` | Pass on 2026-06-05. |
| Docs build | `rtk mdbook build docs` | Pass on 2026-06-05 with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| mGBA Live smoke | Boot current ROM and capture one screenshot / input state | Pass on 2026-06-05. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen in session `smart-ai-ability-item-knowledge-20260605`; `mgba_live_stop` returned `stopped:true`. |

## Manual Runtime Checks

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

## Accepted Risks Before Deeper Tuning

- G-Max unique effects need separate scoring if they become important to trainer fixtures.
- Air Lock / Cloud Nine preservation still needs a focused pre-weather AI unit once a clean weather fixture exists for this file.
- Smart Z-Move status tactics still depend on the existing status Z-Move checks.
- The AI still evaluates one selected move / target at reconsider time; it does not perform a full turn-tree search.
- Double switching and board-control pivoting are heuristic position checks, not a full VGC turn solver. Board-control pivots can identify weather, terrain, Tailwind, and Trick Room roles, but they do not yet search full multi-turn lines such as "switch setter now, protect partner next turn, then reposition again."
- Status and ability-bridge switching is intentionally conservative in singles. Non-immediate utility pivots are double-battle only in this slice; singles keep only immediate status-benefit pivots plus the existing bad-odds / bad-matchup gates.

## GitHub Actions

Long GitHub Actions were not re-waited for this local handoff. Local `check`, `all`, docs build, and mGBA Live smoke provide the branch evidence for this update.
