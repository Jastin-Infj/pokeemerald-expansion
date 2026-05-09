# Battle Item Restore Policy Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `f5a3b7b6c2`; implementation branch `feature/battle-item-restore-policy` |
| Code status | Implemented and locally validated on feature branch; not present in `master` source |
| Provenance | Feature handoff |

## Status

Status: Implemented on `feature/battle-item-restore-policy`; not yet present in
`master` source as of 2026-05-09 (`master` `f5a3b7b6c2`)

The berry-inclusive battle-end restore path is implemented, locally tested, and
verified through mGBA headless battle tests plus an mGBA Live MCP boot/input
smoke check.

## Implemented Behavior

The branch keeps battle-time held item consumption intact, then restores the
original party held item at battle end when the new config is enabled.

Key contract:

- Berries are still consumed during battle.
- `usedHeldItem` and related battle runtime state remain available to
  `Recycle`, `Pickup`, `Harvest`, `Cud Chew`, and similar mechanics.
- Battle-end restore uses the original item captured in
  `gBattleStruct->itemLost[B_SIDE_PLAYER][slot].originalItem`.
- When `B_RESTORE_HELD_BATTLE_BERRIES == TRUE`, berry pocket items can be
  restored to the player party at battle end.
- Existing non-berry restore behavior under `B_RESTORE_HELD_BATTLE_ITEMS`
  remains in place.

The broader item-mechanics impact matrix is maintained in `impact_scope.md`.
Natural Gift, Fling, Recycle, Pickup, Harvest, Cud Chew, G-Max Replenish,
Belch, Bug Bite / Pluck, Knock Off, Thief / Covet, Trick / Switcheroo, Bestow,
Symbiosis, Corrosive Gas, Air Balloon, and Unburden are treated as follow-up
coverage areas, not as battle-time behavior changes in this slice.

## Files Changed By The Implementation

| File | Change |
|---|---|
| `include/config/battle.h` | Added `B_RESTORE_HELD_BATTLE_BERRIES`, defaulting to `TRUE` for this feature branch. |
| `src/battle_util.c` | Added the restore policy helper and allowed berry restore when the new config is enabled. |
| `src/battle_main.c` | Calls `TryRestoreHeldItems()` when non-berry restore, knock-off restore, or berry restore policy requires it. |
| `test/battle_item_restore.c` | Direct tests for berry restore and existing non-berry restore. |
| `test/battle/hold_effect/battle_item_restore.c` | Full battle test that consumes an Oran Berry and checks party held item restoration after battle end. |

## Validation

Confirmed commands:

```sh
rtk git diff --check
rtk make -j16 -O check TESTS=test/battle_item_restore.c
rtk make -j16 -O check TESTS=test/battle/hold_effect/battle_item_restore.c
rtk make -j16 -O all
rtk make -j16 -O debug
```

Results:

- `rtk git diff --check`: passed.
- `test/battle_item_restore.c`: passed, 2 tests.
- `test/battle/hold_effect/battle_item_restore.c`: passed.
- `rtk make -j16 -O all`: passed.
- `rtk make -j16 -O debug`: passed.
- mGBA Live MCP boot/input smoke check reached title screen, accepted `A`,
  reached the continue menu, exported `/tmp/mgba-battle-item-restore-smoke-continue.png`,
  and `mgba_live_stop` reported `stopped: true`.
- Follow-up focused mGBA Live MCP check ran `pokeemerald-test.elf` filtered to
  `test/battle/hold_effect/battle_item_restore.c`. Lua memory read of
  `gTestRunnerState` reported `runner_state = STATE_EXIT`, `exit_code = 0`,
  `result = TEST_RESULT_PASS`, `argv = test/battle/hold_effect/battle_item_restore.c`.
  This is the feature-specific MCP evidence for the Oran Berry consume and
  battle-end restore path. The earlier normal ROM title / continue check should
  be treated only as an MCP boot/input smoke check.

GitHub Actions were not waited during the agent handoff because the long jobs
can take roughly 20-30 minutes. The branch handoff uses local make, focused
mGBA tests, and mGBA Live MCP evidence.

## mGBA Live Setup Used

The working MCP path uses the script-capable mGBA cache build:

```text
/home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt
```

The default wrapper is:

```text
/home/jastin/.local/bin/mgba-qt
```

The wrapper sets `DISPLAY="${DISPLAY:-:0}"` and execs the script-capable build.
This avoids the system `/usr/games/mgba-qt`, which does not support `--script`
in the confirmed environment.

## Merge Handoff Notes

This implementation branch contains source and test changes. Do not merge it
into `master` as part of the docs-only upstream intake lane. When this feature
is intentionally adopted, review and merge the feature PR explicitly.

Draft PR: #14 `feature/battle-item-restore-policy` -> `master`.

If updating `master` under the local docs-only policy, do not merge the entire
branch into `master`. Create a docs-only branch from `master` or cherry-pick
only the docs commits that are meant to land there.

Before a docs-only merge, run:

```sh
rtk git diff --name-only master..HEAD
```

Only `docs/` changes and, if needed, agent-facing `AGENTS.md` workflow updates
should be present for that merge path. Runtime code, generated output, ROMs,
saves, screenshots, and cache artifacts must not be included.

## Future Work

- Add equivalent full battle coverage for Sitrus Berry if needed.
- Re-check facility-specific held item policy when Champions Challenge or
  battle selection runtime state is implemented.
- Keep party menu / summary icon checks as manual or mGBA Live checks when UI
  display changes are involved.
