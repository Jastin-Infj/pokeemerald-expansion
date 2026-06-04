# Smart Gimmick AI Test Plan

## Required Validation

| Check | Command | Status |
| --- | --- | --- |
| Focused smart gimmick tests | `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'` | Pass on 2026-06-05; 8 tests passed. |
| Focused smart Z tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'` | Pass on 2026-06-05; 2 tests passed. |
| Focused smart Tera tests | `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'` | Pass on 2026-06-05; 4 tests passed. |
| Full battle / runtime checks | `rtk make -j16 -O check` | Pass on 2026-06-05. Existing known-failing / expected-failing test labels remained non-fatal. |
| Normal ROM build | `rtk make -j16 -O all` | Pass on 2026-06-05. |
| Docs build | `rtk mdbook build docs` | Pass on 2026-06-05 with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| mGBA Live smoke | Boot current ROM and capture one screenshot / input state | Pass on 2026-06-05. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` and captured an intro-frame screenshot. `mgba_live_stop` returned `stopped:true`; CLI `status --all` returned `[]`. |

## Manual Runtime Checks

Use the debug trainer battle flow and Trainer 1 IDs documented in `docs/tutorials/ai_flags.md`.

- `855`: no-payoff Dynamax conserve.
- `856`: last-Pokemon Dynamax.
- `858`: Z-Move viability and smart timing.
- `859`: offensive Tera.
- `860`: Dynamax + Tera environment with separate candidates.
- `861`: all-gimmick single battle.
- `862`: all-gimmick double battle.

## Accepted Risks Before Deeper Tuning

- G-Max unique effects need separate scoring if they become important to trainer fixtures.
- Mega weather-control examples such as weather re-control from Mega abilities are not fully modeled yet.
- Smart Z-Move status tactics still depend on the existing status Z-Move checks.
- The AI still evaluates one selected move / target at reconsider time; it does not perform a full turn-tree search.

## GitHub Actions

Long GitHub Actions were not re-waited for this local handoff. Local `check`, `all`, docs build, and mGBA Live smoke provide the branch evidence for this update.
