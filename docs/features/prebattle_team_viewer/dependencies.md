# Pre-Battle / In-Battle Team Viewer Dependencies

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `feature/prebattle-team-viewer-phase2` |
| Code status | Phase 2 implemented; docs record for handoff |
| Provenance | Local source inspection, mGBA validation notes, 2026-05-18 source audit |

## Runtime Owner Map

| Area | Files | Dependency / contract |
|---|---|---|
| Build-time gates | `include/config/battle.h` | `B_TRAINER_BATTLE_SELECTION`, `B_PREBATTLE_TEAM_VIEWER`, `B_IN_BATTLE_TEAM_VIEWER`, `B_TEAM_VIEWER_BUTTON`, and `B_TEAM_VIEWER_DETAILS_BUTTON` control the feature. Rebuild is required after changes. |
| Trainer battle entry | `src/battle_setup.c` | Eligible trainer battles call `PreBattleTeamViewer_Begin(requiredCount, CB2_StartTrainerBattleAfterPartySelection)`. If viewer startup fails, the legacy trainer battle selection fallback can still start. |
| Viewer state and UI | `include/prebattle_team_viewer.h`, `src/prebattle_team_viewer.c` | Owns opponent preview cache, icon grid, integrated pick-order state, player Summary shortcut, opponent public detail footer, in-battle read-only viewer, callback1 pause/restore, window/palette/BG cleanup, and marker rendering. |
| Party compression / restore | `include/trainer_battle_selection.h`, `src/trainer_battle_selection.c` | Viewer confirmation writes `gSelectedOrderFromParty`, then `TrainerBattleSelection_StartBattleFromSelection()` compresses selected Pokemon and later restores them. The viewer does not own party restore. |
| Opponent battle party | `src/battle_main.c` | Battle init consumes the cached opponent party through `PreBattleTeamViewer_LoadCachedOpponentParty()` so preview and battle use the same team. |
| Battle action input | `src/battle_controller_player.c`, `include/battle_controllers.h`, `src/reshow_battle_screen.c` | Action menu opens read-only viewer through `B_TEAM_VIEWER_BUTTON`, pauses battle callback1 while open, redraws action prompt/menu after close, and waits for held keys to release. |
| TeamInfo / MoveInfo affordance | `src/battle_controller_player.c`, `src/battle_interface.c`, `graphics/battle_interface/team_info_window_l.png`, `graphics/battle_interface/team_info_window_r.png` | TeamInfo uses the same slide model as MoveInfo. Double battle TeamInfo Y is kept at `102`, matching MoveInfo's double position (`LAST_USED_WIN_Y + 32`). |
| Pokemon Summary entry | `include/pokemon_summary_screen.h`, `src/pokemon_summary_screen.c` | Player-side `SELECT` opens the existing Summary directly on `POKEMON SKILLS` through `ShowPokemonSummaryScreenAtPage()`. Direct skills entry must set BG order/X like a completed page scroll so `POKEMON INFO` is not corrupted. |
| Debug validation | `src/debug.c` | `Party -> Team Viewer Battle` covers single 3-of-6. `Party -> Team Viewer W` covers double 4-of-6 with Amy & Liv. Both create the same six-mon debug player party. |
| Runtime validation tools | `docs/tools/mgba_live_runtime_validation.md`, `docs/manuals/mgba_live_mcp_manual.md` | Runtime evidence should use debug ROM plus focused mGBA session screenshots/input and must stop sessions afterward. |

## State Lifetime

| State | Owner | Lifetime | Save impact |
|---|---|---|---|
| Opponent preview party | `src/prebattle_team_viewer.c` EWRAM state | Encounter start through battle init / cleanup | Not saved |
| Required pick count | viewer state from battle selection required count | Viewer open through battle start | Not saved |
| Pick order | viewer state, then `gSelectedOrderFromParty` | Viewer selection through `TrainerBattleSelection_StartBattleFromSelection()` | Not saved |
| Player party restore copy | `src/trainer_battle_selection.c` | Battle start through `CB2_EndTrainerBattle()` restore | Not saved |
| In-battle callback1 backup | `src/prebattle_team_viewer.c` | In-battle viewer open through battle screen reshow | Not saved |

No SaveBlock field, saved flag, or saved var is introduced by this feature.

## Entry / Exclusion Dependencies

Eligible route:

- normal trainer battle;
- battle type flags already resolved;
- player has more eligible Pokemon than the required battle size;
- required count is 3 for single and 4 for double.

Excluded or deferred routes:

- Battle Frontier;
- link / recorded battle;
- two-opponent trainer battle;
- multi / follower partner;
- Secret Base;
- Pyramid / Trainer Hill;
- first-battle tutorial;
- move menu, target menu, Bag, Pokemon switch menu, and Run menu viewer entry.

## Tuning Dependencies

| Need | Knob | Notes |
|---|---|---|
| Pick-order marker position | `TEAM_VIEWER_SELECTED_MARKER_*` and `TEAM_VIEWER_SELECTED_TEXT_*` in `src/prebattle_team_viewer.c` | Offsets are relative to the slot label origin. Marker fill and text printer background are separate. Text background / shadow must stay transparent. |
| Player Summary move reorder policy | `TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER` in `src/prebattle_team_viewer.c` | Default `0` uses `SUMMARY_MODE_LOCK_MOVES`. Set to `1` only if move reordering from this route is accepted. |
| Summary start page | `TEAM_VIEWER_SUMMARY_START_PAGE` in `src/prebattle_team_viewer.c` | Current value is `PSS_PAGE_SKILLS`. Direct page entry depends on `ShowPokemonSummaryScreenAtPage()`. |
| In-battle shortcut button | `B_TEAM_VIEWER_BUTTON` in `include/config/battle.h` | Current default is `R_BUTTON`. Avoid `L_BUTTON` when MoveInfo / L=A conflicts matter. |
| Detail / Summary button | `B_TEAM_VIEWER_DETAILS_BUTTON` in `include/config/battle.h` | Current default is `SELECT_BUTTON`, mapping Champions-style Y behavior to GBA. |
| TeamInfo double Y | `TEAM_VIEWER_ACTION_HINT_Y_DOUBLE` in `src/battle_controller_player.c` | Keep aligned with MoveInfo double position unless both are intentionally retuned together. |

## Interactions With Other Features

| Feature | Interaction | Current status |
|---|---|---|
| Trainer battle selection | Direct dependency. Viewer owns integrated selection UI, but selection module owns compression / restore. | Implemented and validated through single and double debug routes. |
| Trainer Party Pools / randomizer | Viewer freezes the effective opponent party before battle. Pool RNG timing becomes "encounter start before viewer." Source audit confirms preview generation uses `CreateNPCTrainerPartyForPreview()` and the same `DoTrainerPartyPool()` path as battle generation. | Implemented. Optional automated / mGBA regression can assert one concrete pool trainer before adoption. |
| Partygen | Partygen can change trainer party levels / species before runtime. Viewer displays the generated/materialized party that battle will use because battle init consumes the preview cache. | Implemented on the viewer side. Partygen CLI / catalog itself lives on `feature/trainer-partygen-catalog-expansion`. |
| Battle item restore / aftercare | These run after battle. Team viewer must clear state and leave player party restoration to battle selection before aftercare assumptions run. | No direct conflict found. |
| Pokemon Summary / Move Relearner | Player Summary can expose Summary move behavior. Default route locks move reorder; setting `TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER = 1` intentionally permits normal Summary behavior. | Default locked; docs record the toggle. |
| Options UI | Runtime option menu is out of scope. Current toggles are build-time config only. | Future runtime option work should design save/default migration separately. |
| MoveInfo | TeamInfo action hint should stay visually aligned with MoveInfo and should disappear before Fight so MoveInfo is the only hint in the move menu. | Validated with mGBA screenshots. |

## Debug Route Notes

`Party -> Team Viewer Battle` and `Party -> Team Viewer W` are the supported focused
debug routes for this feature. They create a known player party and start normal trainer
battle setup paths.

Broader debug menu trainer-battle routes are not the acceptance gate for this feature.
Some debug trainer flows can differ from normal trainerbattle scripts or skip the exact
pre-battle setup path. If those routes fail to exercise Team Viewer, record the mismatch
but validate through the normal trainer route or the focused Team Viewer debug routes.

## Remaining Dependency Checks

- Trainer Party Pool / randomized party identity: optional regression evidence should assert
  cached preview species/order matches battle species/order for a concrete pool trainer.
  The source mechanism is already implemented.
- Override trainer path: optional regression evidence should assert effective trainer data is
  previewed, not the base trainer definition.
- Future runtime optionization: if enabled, inspect save layout, option menu pages, and
  build-time fallback behavior before implementation.
