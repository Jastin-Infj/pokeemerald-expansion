# Battle Item Restore Policy Implementation

## Status

Status: Shipped on `feature/trainer-battle-aftercare-heal`

The berry-inclusive battle-end restore path is implemented, locally tested,
verified through mGBA headless battle tests, and user-confirmed in game.

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

## Files Changed By The Implementation

| File | Change |
|---|---|
| `include/config/battle.h` | Added `B_RESTORE_HELD_BATTLE_BERRIES`, defaulting to `TRUE` for this branch. |
| `src/battle_util.c` | Added the restore policy helper and allowed berry restore when the new config is enabled. |
| `src/battle_main.c` | Calls `TryRestoreHeldItems()` when non-berry restore, knock-off restore, or berry restore policy requires it. |
| `test/battle_item_restore.c` | Direct tests for berry restore and existing non-berry restore. |
| `test/battle/hold_effect/battle_item_restore.c` | Full battle test that consumes an Oran Berry and checks party held item restoration after battle end. |

## Validation

Confirmed commands:

```sh
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check TESTS=test/battle_item_restore.c
rtk make -j16 -O check TESTS=test/battle/hold_effect/battle_item_restore.c
```

Results:

- `test/battle_item_restore.c`: passed, 2 tests.
- `test/battle/hold_effect/battle_item_restore.c`: passed.
- mGBA Live MCP boot/input smoke check reached title screen and continue menu,
  then cleaned up with `status --all == []`.
- User confirmed the real in-game behavior after push: the related battle flow
  and berry restoration both worked, including the berry returning after battle.

GitHub Actions were not re-waited during the agent handoff because the long
jobs can take roughly 20-30 minutes. The branch handoff used local make,
focused mGBA tests, mGBA Live MCP evidence, and user manual confirmation.

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

This implementation branch contains source and test changes. If updating
`master` under the local docs-only policy, do not merge the entire branch into
`master`. Create a docs-only branch from `master` or cherry-pick only the docs
commits that are meant to land there.

Before a docs-only merge, run:

```sh
rtk git diff --name-only master..HEAD
```

Only docs changes should be present for that merge path. Runtime code, generated
output, ROMs, saves, screenshots, and cache artifacts must not be included.

## Future Work

- Add equivalent full battle coverage for Sitrus Berry if needed.
- Re-check facility-specific held item policy when Champions Challenge or
  battle selection runtime state is implemented.
- Keep party menu / summary icon checks as manual or mGBA Live checks when UI
  display changes are involved.
