# Pre-Battle Team Viewer Manual

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Implementation branch | `feature/prebattle-team-viewer-phase2` |
| Code status | Phase 2 implemented; source branch validated |
| Primary docs | `docs/features/prebattle_team_viewer/` |

この manual は、通常 trainer battle 前後の Team Viewer を確認・調整・検証する作業順をまとめる。
設計経緯や細かい validation 証跡は feature docs を参照する。

## Current Contract

| Situation | Behavior |
|---|---|
| normal single trainer battle | Viewer shows both teams, player picks 3 from 6 inside the viewer, then battle starts. |
| normal double trainer battle | Viewer shows both teams, player picks 4 from 6 inside the viewer, then battle starts. |
| player-side `A` | Pick / unpick highlighted eligible Pokemon. |
| opponent-side `A` | Ignored / no battle command. |
| `START` | Starts battle only after the required count is selected. |
| player-side `SELECT` | Opens the standard Pokemon Summary on the skills/status page. |
| opponent-side `SELECT` | Shows lightweight public preview only. |
| battle action menu `R` | Opens read-only in-battle Team Viewer. |
| in-battle viewer `A` / `B` / `R` | Closes viewer and returns to action menu after held keys are released. |
| in-battle viewer D-pad / `SELECT` | Ignored; no battle action, move, target, party, or debug input should advance behind it. |

Opponent moves, ability, held item, and stat allocation remain hidden unless a future
design explicitly makes them public.

## Configuration

Build-time config lives in `include/config/battle.h`.

```c
#define B_TRAINER_BATTLE_SELECTION          TRUE
#define B_PREBATTLE_TEAM_VIEWER             TRUE
#define B_IN_BATTLE_TEAM_VIEWER             TRUE
#define B_TEAM_VIEWER_BUTTON                R_BUTTON
#define B_TEAM_VIEWER_DETAILS_BUTTON        SELECT_BUTTON
```

| Config | Meaning |
|---|---|
| `B_TRAINER_BATTLE_SELECTION` | Enables trainer-battle 3/4 pick selection and party restore. Required for this viewer flow. |
| `B_PREBATTLE_TEAM_VIEWER` | Enables pre-battle Team Viewer on eligible trainer battles. |
| `B_IN_BATTLE_TEAM_VIEWER` | Enables read-only viewer from battle action menu. |
| `B_TEAM_VIEWER_BUTTON` | In-battle action-menu shortcut. Current default is `R_BUTTON`. |
| `B_TEAM_VIEWER_DETAILS_BUTTON` | Pre-battle Summary / public detail button. Current default is `SELECT_BUTTON`. |

Rebuild after config changes:

```sh
rtk make -j16 -O all
rtk make -j16 -O debug
```

## Entry Flow

```text
trainerbattle script
  -> BattleSetup_StartTrainerBattle()
  -> resolve gBattleTypeFlags
  -> TrainerBattleSelection_GetRequiredCount()
  -> PreBattleTeamViewer_Begin(requiredCount, CB2_StartTrainerBattleAfterPartySelection)
      -> build and cache opponent preview party
      -> open viewer
      -> A toggles player picks
      -> START confirms when selected count == required count
      -> write gSelectedOrderFromParty
  -> CB2_StartTrainerBattleAfterPartySelection()
  -> TrainerBattleSelection_StartBattleFromSelection()
      -> compress selected player party into battle order
  -> CB2_InitBattle()
      -> PreBattleTeamViewer_LoadCachedOpponentParty()
  -> battle
  -> CB2_EndTrainerBattle()
      -> battle selection restores player party
      -> team viewer clears state
```

The viewer owns selection UI. The battle selection module still owns temporary player-party
compression and restore.

## Supported Debug Routes

Use these focused routes from the debug menu:

| Debug menu item | Trainer | Purpose |
|---|---|---|
| `Party -> Team Viewer Battle` | `TRAINER_GABRIELLE_1` | Single 3-of-6 route. |
| `Party -> Team Viewer W` | `TRAINER_AMY_AND_LIV_1` | Double 4-of-6 route. |

Both routes create the same six-mon player party with known moves, then enter normal
trainer battle setup.

Broader debug trainer-battle entries are not the acceptance route for this feature. Some
debug trainer flows can differ from normal `trainerbattle` setup and may not exercise the
viewer exactly. If they fail, record it as a debug-route mismatch and validate through the
focused routes above or a normal map trainer battle.

## UI Tuning Knobs

### Pick-Order Marker

Marker tuning is in `src/prebattle_team_viewer.c`.

```c
#define TEAM_VIEWER_SELECTED_MARKER_X_OFFSET 10
#define TEAM_VIEWER_SELECTED_MARKER_Y_OFFSET 1
#define TEAM_VIEWER_SELECTED_MARKER_WIDTH 9
#define TEAM_VIEWER_SELECTED_MARKER_HEIGHT 8
#define TEAM_VIEWER_SELECTED_TEXT_X_OFFSET 8
#define TEAM_VIEWER_SELECTED_TEXT_Y_OFFSET 0
```

The marker rectangle is drawn relative to the original slot-label origin. Keep the text
printer background and shadow transparent so the marker and text do not draw separate
filled patches.

### Summary Behavior

```c
#define TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER 0
#define TEAM_VIEWER_SUMMARY_START_PAGE PSS_PAGE_SKILLS
```

Default `0` uses `SUMMARY_MODE_LOCK_MOVES`, so player-side Summary cannot reorder moves.
Set to `1` only if this route should allow normal Summary move-order behavior.

### TeamInfo / MoveInfo Alignment

TeamInfo hint tuning is in `src/battle_controller_player.c`.

```c
#define TEAM_VIEWER_ACTION_HINT_Y_SINGLE     92
#define TEAM_VIEWER_ACTION_HINT_Y_DOUBLE     102
```

Double Y is intentionally `102` to match the existing double MoveInfo position in
`src/battle_interface.c`:

```c
#define LAST_USED_BALL_Y      ((IsDoubleBattle()) ? 78 : 68)
#define LAST_USED_WIN_Y       (LAST_USED_BALL_Y - 8)
CreateSprite(..., LAST_USED_WIN_Y + 32, ...)
```

If W / double action-menu hints need to move, retune TeamInfo and MoveInfo together.

## Validation Checklist

Run build checks after source or config changes:

```sh
rtk git diff --check
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check
```

Focused runtime checks:

| Check | Route |
|---|---|
| Single pick flow | `Party -> Team Viewer Battle`, select 3, start battle. |
| Double pick flow | `Party -> Team Viewer W`, select 4, start battle. |
| Summary first entry | In viewer, player-side `SELECT`; Summary opens on `POKEMON SKILLS` without layout corruption. |
| Summary info regression | From `POKEMON SKILLS`, press left to `POKEMON INFO`; skills background must not remain. |
| Marker order | Pick slot 6 first; label changes from `6` to `1`, not `P1`. |
| Text/marker separation | Pick-order text must not paint a second background patch over the marker. |
| In-battle viewer | At action menu, press `R`; viewer opens read-only and closes back to action menu. |
| Held input guard | Hold close key; battle action should not auto-select after viewer closes. |
| TeamInfo hint | Action menu shows `R / TEAM / INFO`; Fight transition leaves only MoveInfo. |
| W hint alignment | Double battle action menu TeamInfo Y matches MoveInfo double coordinate. |

Stop mGBA sessions after validation and record screenshots in
`docs/features/prebattle_team_viewer/test_plan.md`.

## Known Remaining Checks

| Topic | Status |
|---|---|
| Trainer Party Pools / randomized party identity | Implemented by source mechanism: preview cache is generated through the trainer pool path and battle init consumes the same cache. Optional regression should assert one concrete pool trainer. |
| Override trainer effective party | Source path uses the effective preview generator. Optional regression should assert one concrete override-trainer route. |
| Runtime option menu | Out of scope. Requires save/default migration design. |
| Full Champions detail UI | Out of scope. Opponent private data remains hidden. |

## Merge Policy

Runtime source belongs on a feature or integration branch. Do not merge this source branch
directly into `master`.

| Target | Allowed files |
|---|---|
| implementation branch | `src/`, `include/`, `graphics/`, `docs/` |
| docs-only master branch / PR | `docs/`, and `AGENTS.md` only if workflow rules changed |

Before a docs-only master merge or PR, run:

```sh
rtk git diff --name-only master..HEAD
```

Anything outside `docs/` and `AGENTS.md` means the branch is not eligible for docs-only
master integration.
