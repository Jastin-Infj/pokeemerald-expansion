# Smart Gimmick AI Test Plan

## Required Validation

| Check | Command | Status |
| --- | --- | --- |
| Focused smart gimmick tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'` | Pass on 2026-06-05; 8 tests passed. |
| Focused smart Z tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'` | Pass on 2026-06-05; 3 tests passed. |
| Focused smart Mega tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'` | Pass on 2026-06-05; 1 test passed. |
| Focused smart Tera tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'` | Pass on 2026-06-05; 4 tests passed. |
| Focused smart switching tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_SWITCHING'` | Pass on 2026-06-05. Covers double-position switching, partner-cover guard, weather / terrain reserve pivots, and Tailwind / Trick Room reserve pivots. |
| Full battle / runtime checks | `rtk make -j16 -O check` | Pass on 2026-06-05. Existing known-failing / expected-failing test labels remained non-fatal. |
| Normal ROM build | `rtk make -j16 -O all` | Pass on 2026-06-05. |
| Docs build | `rtk mdbook build docs` | Pass on 2026-06-05 with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| mGBA Live smoke | Boot current ROM and capture one screenshot / input state | Pass on 2026-06-05. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen and captured a screenshot. `mgba_live_stop` returned `stopped:true`; CLI `status --all` returned `[]`. |

## Manual Runtime Checks

Use the debug trainer battle flow and Trainer 1 IDs documented in `docs/tutorials/ai_flags.md`.

- `855`: no-payoff Dynamax conserve.
- `856`: last-Pokemon Dynamax.
- `858`: Z-Move viability and smart timing.
- `859`: offensive Tera.
- `860`: Dynamax + Tera environment with separate candidates.
- `861`: all-gimmick single battle.
- `862`: all-gimmick double battle.

For double switching behavior, use any debug double battle with `AI_FLAG_SMART_SWITCHING` and a passive pinned AI lead pair. Expected behavior is that the AI can hard switch one or both active Pokemon if neither active slot has useful pressure and the partner cannot cover the threatened position.

For board-control switching, use an AI reserve that can change the field or speed state. Expected examples:

- `Pelipper` / `Drizzle` can be chosen over staying in when rain creates stronger reserve pressure.
- `Rillaboom` / `Grassy Surge` can be chosen when Grassy Terrain plus Grass pressure changes the board.
- A reserve with `Tailwind` can be chosen when doubled Speed flips relevant matchups.
- A reserve with `Trick Room` can be chosen when the AI side is slower and the current room state does not already cover that plan.

## Accepted Risks Before Deeper Tuning

- G-Max unique effects need separate scoring if they become important to trainer fixtures.
- Air Lock / Cloud Nine preservation still needs a focused pre-weather AI unit once a clean weather fixture exists for this file.
- Smart Z-Move status tactics still depend on the existing status Z-Move checks.
- The AI still evaluates one selected move / target at reconsider time; it does not perform a full turn-tree search.
- Double switching and board-control pivoting are heuristic position checks, not a full VGC turn solver. Board-control pivots can identify weather, terrain, Tailwind, and Trick Room roles, but they do not yet search full multi-turn lines such as "switch setter now, protect partner next turn, then reposition again."

## GitHub Actions

Long GitHub Actions were not re-waited for this local handoff. Local `check`, `all`, docs build, and mGBA Live smoke provide the branch evidence for this update.
