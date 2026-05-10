# Pre-Battle / In-Battle Team Viewer MVP Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `7c19f56901`; `git describe` = `expansion/1.15.2-38-g7c19f56901` |
| Code status | MVP implemented; Phase 2 supersedes the two-screen selection path on `feature/prebattle-team-viewer-phase2` |
| Provenance | Local project feature docs |

## MVP

Add a guarded team viewer for normal trainer battles.

The MVP should:

- Show the player party and opponent party before battle starts.
- Let the player inspect the opponent team before choosing 3 / 4 mons.
- Let the player press `Y` on the viewer to inspect detailed strength information.
- Let the player reopen the team view during battle from the action menu.
- Reuse battle selection MVP for actual party selection.
- Ensure the opponent team shown in the viewer is the same team used in battle.
- Avoid SaveBlock changes.

## Non-Goals

- Do not build the full Champions-style integrated selector in the first slice.
- Do not support link / Frontier / Union Room / two opponents / partner battles.
- Do not show opponent moves, ability, or held item by default.
- Do not change battle UI after battle starts.
- Do not open the viewer from move selection, target selection, bag, or party switch menu in the first slice.
- Do not reveal opponent moves / ability / held item / stat allocation until official UI behavior is confirmed.
- Do not add runtime options yet.

## Implementation Steps

| Step | Files | Notes |
|---|---|---|
| 1 | `include/config/battle.h` | Add guarded configs, likely `B_PREBATTLE_TEAM_VIEWER`, `B_IN_BATTLE_TEAM_VIEWER`, and `B_TEAM_VIEWER_BUTTON`. Keep default decisions explicit in docs. |
| 2 | battle selection source files | Reapply `feature/battle-selection-mvp` source slice onto current `master` instead of merging the old branch. |
| 3 | `include/prebattle_team_viewer.h`, `src/prebattle_team_viewer.c` | Add EWRAM state, opponent cache, viewer callback, and public entrypoints. |
| 4 | `src/battle_setup.c` | Insert viewer gate before battle selection / battle start. Keep exclusion policy aligned with selection MVP. |
| 5 | `src/battle_main.c`, `include/battle_main.h` | Refactor opponent party generation so battle init can consume cached preview party. |
| 6 | `src/battle_controller_player.c` | Add action-menu shortcut to open read-only viewer in eligible trainer battles. First candidate button is `R_BUTTON`. |
| 7 | UI files / graphics helpers | Dedicated screen windows, Pokemon icons, player/opponent lists, footer controls, and `Y` detail overlay. |
| 8 | docs | Update implementation summary and test plan with build / mGBA evidence. |

## Current Contract

| Case | Behavior |
|---|---|
| config off | No-op. Existing trainer battle flow remains unchanged. |
| normal single trainer battle | Viewer shows both teams, then existing selection asks for 3 mons. |
| normal double trainer battle | Viewer shows both teams, then existing selection asks for 4 mons. |
| party too small for selection | Viewer should not appear if battle selection will not be offered. |
| opponent party count less than 6 | Show only actual opponent party slots; empty slots remain blank. |
| pool / randomized trainer | Viewer freezes the actual generated party, and battle uses the same cached party. |
| viewer cancel | MVP should not cancel the trainer encounter. `B` can be disabled or act as back within the viewer only. |
| strength view | `Y` opens detail view for selected Pokémon. Player side shows moves / ability / held item / stat allocation; opponent side shows public preview only unless verified otherwise. |
| in-battle shortcut | During player action selection in eligible trainer battles, `B_TEAM_VIEWER_BUTTON` opens read-only viewer. |
| in-battle close | `A`, `B`, or `B_TEAM_VIEWER_BUTTON` closes and returns to action selection without emitting a battle command. |
| battle end | Battle selection restore still owns player party restoration. Team viewer state clears after battle init or battle end fallback. |

## Button Contract

First implementation should prefer:

```c
#define B_IN_BATTLE_TEAM_VIEWER TRUE
#define B_TEAM_VIEWER_BUTTON R_BUTTON
#define B_TEAM_VIEWER_DETAILS_BUTTON SELECT_BUTTON
```

Reasoning:

- `L_BUTTON` is already the default move description button.
- `R_BUTTON` is last-used-ball by default, but last-used-ball cannot be thrown in trainer battles.
- `START_BUTTON` already swaps HP bar/text in action menu and toggles gimmicks in move menu.
- `SELECT_BUTTON` can be debug battle menu or move rearrangement.

The shortcut should be checked only in `HandleInputChooseAction()` for normal trainer battles.
Move menu support can be a later phase after resolving gimmick / description / rearrange conflicts.
The Champions-style `Y` strength view maps to `SELECT_BUTTON` on GBA hardware.

## Recommended UI Slice

Use a dedicated 240x160 screen instead of trying to patch the existing party menu first.
Champions is the visual target, while Colosseum / XD / Battle Revolution inform the
"team card" framing and the 3/4-from-6 selection precedent.

Suggested layout:

| Region | Contents |
|---|---|
| left 96 px | player party rows with icon, nickname, gender, held item marker, selected count note. |
| right 112 px | opponent party rows with icon, species, gender, type chips. |
| bottom 32 px | confirm prompt and required count. |

If the first UI pass runs out of tile / palette budget, drop type chips before dropping species
icons. Seeing opponent species is the core feature.

## Strength View Slice

`B_TEAM_VIEWER_DETAILS_BUTTON` should open a compact detail panel for the currently highlighted
Pokémon. This is the GBA equivalent of the Champions `Y` strength view.

First-pass player-side contents:

- species / type / gender / level,
- held item,
- ability,
- nature / stat alignment if available in this ROM data model,
- four moves with type/category and PP if layout permits,
- stat row with current values and an investment marker if the project has EV/IV data readily available.

Opponent-side contents for MVP:

- species / type / gender / level,
- no moves / ability / held item / EV/IV allocation unless official Champions selection UI is
  confirmed to reveal those details for the opponent.

This keeps the UI useful without accidentally giving perfect hidden-information scouting.

## Opponent Cache Contract

The implementation should create one source of truth for the opponent team.

Minimum cache fields:

- `active`
- `trainerA`
- `battleTypeFlags`
- `enemyParty[PARTY_SIZE]` or equivalent generated party data
- `enemyPartyCount`
- `opponentMonCanTera`
- `opponentMonCanDynamax`
- an in-battle view model or full cache that remains available after battle init

Battle init should check the cache before calling the normal trainer party generation path.
When cache is active and compatible with the current trainer / flags, copy the cached party into
`gEnemyParty` and restore gimmick capability fields onto `gBattleStruct`.
Do not discard the lightweight viewer data until battle end, because in-battle viewer needs it.

## Branch / Integration Plan

Do not use `feature/battle-selection-mvp` as the base branch.

Recommended integration sequence:

1. Start from current `master`.
2. Reapply battle selection source changes from `feature/battle-selection-mvp`.
3. Build once with selection only.
4. Add pre-battle team viewer behind config.
5. Add in-battle action-menu viewer shortcut behind config.
6. Build and mGBA validate viewer -> selection -> battle start -> in-battle viewer -> battle end restore.

This keeps newer docs and unrelated master overlays intact.

## Future Work

- Completed in Phase 2: replace existing choose-half UI with a single integrated viewer/selector.
- Completed in Phase 2: add player-side Summary entry without losing selection order.
- Future polish: apply a Battle Revolution-style "team card / battle pass" frame if it fits the GBA UI.
- Future polish: add type icons / richer opponent badges if palette / tile budget allows it.
- Future shortcut policy: allow viewer from move menu if a clean button policy exists.
- Phase 3: Champions challenge runtime can override this viewer with challenge-specific roster policy.

The integrated selector is now recorded in
[Phase 2 Integrated Selection Flow Checklist](phase2_selection_flow_checklist.md).
Further changes that touch input ownership, party restore behavior, or battle start callbacks
should still be treated as high-impact and validated through the same checklist.

## Open Questions

- Should first implementation default `B_PREBATTLE_TEAM_VIEWER` to `TRUE` for validation, then decide integration default later like selection MVP?
- Should opponent preview appear before or after trainer intro text for no-intro / continue-script modes?
- Should player held items be shown as item icons or compact text in the first pass?
- Should in-battle viewer show current HP/status/fainted overlay for opponent slots, or just the original preview team?
- What exact wording should replace Champions' `つよさ` copy on the GBA UI if text space is tight?
