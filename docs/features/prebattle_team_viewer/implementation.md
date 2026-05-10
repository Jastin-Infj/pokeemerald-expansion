# Pre-Battle / In-Battle Team Viewer Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `feature/prebattle-team-viewer` from `master` `7c19f56901` |
| Code status | Implemented MVP; build/check and focused mGBA route passed |
| Provenance | Local project implementation notes |

## Summary

This branch implements the first runtime slice of the team viewer:

- reapplies the battle selection MVP source slice onto current `master`;
- adds a guarded pre-battle team viewer before the 3 / 4 mon selection menu;
- generates the opponent party once before the viewer and caches that party for battle init;
- adds a read-only in-battle viewer shortcut from the player action menu;
- renders both teams as Pokemon icon grids with compact slot labels;
- uses thin white windows, compact detail labels, and cursor-only redraws so icon sprites are not
  destroyed / recreated on every D-pad move;
- resets BG0 scroll on viewer entry so the in-battle viewer does not inherit battle BG
  offsets and render over a black top area;
- resets palette fade / blend state on viewer entry so the in-battle viewer does not inherit
  a battle-intro tint when opened immediately after the action menu appears;
- uses the standard menu palette slot for all viewer windows so battle palette 15 cannot
  tint the white team-viewer windows red;
- shows player-side held item near the nickname and all four moves in the strength panel;
- keeps the pre-battle strength panel open while the cursor moves, updating the detail
  target instead of forcing the player to press `SELECT` again;
- abbreviates detail labels (`T:`, `Ab:`, `It:`) to keep the two-column footer from
  overlapping on long type / ability names;
- moves side labels into the top header, keeps the main grid to slot-only labels before
  battle, and removes labels from the read-only in-battle grid;
- makes the in-battle viewer display-only: D-pad / `SELECT` are ignored while the viewer is
  open, so it cannot move any battle, move, or target cursor behind the overlay;
- adds a small action-menu `TEAM INFO` sprite using the same 32x32 style / palette as
  the existing `MOVE INFO` hint so the in-battle viewer shortcut is visible;
- animates the `TEAM INFO` sprite with the same slide-in / slide-out model as `MOVE INFO`,
  and clears stale hint state after Bag / menu sprite resets before recreating it;
- routes in-battle viewer close through a player-controller redraw flag that reprints the
  action prompt and action menu after `ReshowBattleScreenAfterMenu()`;
- pauses `gMain.callback1` while the in-battle viewer is open, then restores it after the
  battle screen is rebuilt and all held keys are released;
- lets `B` from the downstream party selection menu return to the team viewer;
- adds a debug-only `Team Viewer Battle` party-menu route so the full viewer -> selection
  -> battle -> in-battle viewer path can be rechecked without hunting for a save route;
- keeps opponent moves / ability / held item hidden in the detail view;
- keeps player party restoration owned by the battle selection module.

The UI is intentionally compact for GBA. It uses two icon grids instead of the full
Champions-style Switch layout. The Champions `Y`-style strength view is mapped to
`SELECT_BUTTON` by default because GBA has no `Y_BUTTON`.

2026-05-10 follow-up: user manual validation found footer / lower-screen corruption in
the first viewer. Root cause was BG tile memory overlap: the viewer used `charBaseIndex = 3`
with screenblock 31, while the footer window tile blocks extended into the same VRAM area.
The viewer now uses `charBaseIndex = 0`, which keeps text tile uploads away from the BG
map. The same follow-up replaced name-heavy team rows with Pokemon icon sprites.

Second 2026-05-10 follow-up: manual validation then showed that the icon labels rendered
but the Pokemon icons did not, and a later path hit the malloc blue screen trying to
allocate window memory. The icon issue was caused by the viewer enabling BG0 but not OBJ
rendering; the screen now sets `DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP` and lowers the viewer
BG priority so icon OBJ renders over the window background. The allocation issue was caused
by entering the viewer without freeing the previous field / battle window
buffers before `InitWindows()`, which leaked the previous window allocations. The viewer
now calls `FreeAllWindowBuffers()` before taking ownership of its own windows.

Third 2026-05-10 follow-up: manual validation showed that D-pad movement briefly refreshed
the divider / lower bar and that the UI still looked too heavy. The root cause of the
flicker was `DrawTeamViewer()` being called for every cursor move, which also destroyed
and recreated every Pokemon icon sprite. Cursor movement now refreshes only the text label
areas with `COPYWIN_GFX`; icon sprites are created only on the initial full draw. The
standard 8px window frame was replaced with a thin pixel frame, icon / label positions were
lowered and tightened, detail level text now uses `LV.`, and player-side details show item
plus all four moves.

Fourth 2026-05-10 follow-up: manual validation showed the in-battle viewer drawing icons
over a black upper area, with the actual white windows shifted down the screen. This was
consistent with battle BG0 scroll state leaking into the viewer. Viewer initialization now
resets both BG0 control coordinates and BG0 hardware scroll registers. The same polish pass
moves `YOUR TEAM` / `OPPONENT` into the top header, lowers the team/footer split by one
tile, removes level text from the main icon grid, and keeps the battle viewer grid label-free
to avoid text overlap.

Fifth 2026-05-10 follow-up: user validation still found two runtime issues. Opening the
in-battle viewer immediately after battle action selection could show a red-tinted viewer.
Palette / blend reset was necessary but not sufficient: the viewer windows used palette 15,
while `Menu_LoadStdPal()` loads the standard menu palette into palette 14. In battle,
palette 15 can contain battle UI colors, so the viewer now uses `STD_WINDOW_PALETTE_NUM`
for every window and the cleared BG tilemap. Closing the in-battle viewer with `B` also
returned to a visually blank battle textbox. `ReshowBattleScreenAfterMenu()` restores the
battle background and cursor but does not reprint the action menu text, so team-viewer close
now sets a redraw flag and `src/reshow_battle_screen.c` calls back into the player
controller after the battle screen is rebuilt.

Sixth 2026-05-10 follow-up: manual validation showed that battle input still ran behind
the in-battle viewer. The root cause was the GBA main loop calling `callback1` before
`callback2`; changing only `callback2` to the viewer did not stop `BattleMainCB1`, so
D-pad / `SELECT` / `A` could move action, move, or target state behind the overlay. The
in-battle viewer now saves and clears `gMain.callback1` while open, restores it only after
the battle screen has been rebuilt, and keeps the player controller in a release-wait guard
until all held keys are up. The in-battle viewer is now strictly read-only: `A`, `B`, and
`R` close it; D-pad and `SELECT` do nothing.

Seventh 2026-05-10 follow-up: the footer and action-menu affordance were tightened after
runtime review. The pre-battle viewer lowers the footer split, gives the prompt/footer
lines more vertical room, and keeps the in-battle footer copy read-only even when a previous
selection count is still cached. The battle action menu now shows a 32x32 `R / TEAM / INFO`
sprite near the left side of the battlefield, built from the same background / palette as
the existing `MOVE INFO` sprite. The hint is removed before opening the viewer or committing
any normal action, so it does not leak into Fight / Bag / Pokemon / Run, debug,
partner-cancel, or last-used-ball paths.

Eighth 2026-05-10 follow-up: `TEAM INFO` now uses the same motion model as `MOVE INFO`.
The sprite is created off-screen at X = -14, slides to X = 14, and when hidden slides back
to X = -14 before freeing its sprite tiles and palette. R-opening the team viewer still
destroys the hint immediately because the whole battle screen is replaced. Bag and other
menu returns can reset sprite data before the hide callback completes, so hint creation
now detects a stale active flag by checking the sprite tile tag and recreates the hint
when the action menu is restored.

Ninth 2026-05-10 follow-up: pre-battle strength view now persists across cursor movement.
Previously every D-pad move cleared `showDetails`, so the player had to press `SELECT`
again for each Pokemon. Cursor movement now preserves the details flag and redraws the
footer against the newly selected slot / side. Pressing `SELECT` still toggles the strength
panel off explicitly.

## Files Changed

| File | Change |
|---|---|
| `include/config/battle.h` | Adds `B_TRAINER_BATTLE_SELECTION`, `B_PREBATTLE_TEAM_VIEWER`, `B_IN_BATTLE_TEAM_VIEWER`, `B_TEAM_VIEWER_BUTTON`, and `B_TEAM_VIEWER_DETAILS_BUTTON`. |
| `include/trainer_battle_selection.h`, `src/trainer_battle_selection.c` | Restores the battle selection MVP source slice. |
| `include/prebattle_team_viewer.h`, `src/prebattle_team_viewer.c` | Adds viewer cache, pre-battle screen, Pokemon icon grid, persistent detail panel, in-battle read-only screen, callback1 pause/restore, OBJ display / priority setup, window-buffer cleanup on entry, BG0 scroll reset, palette/blend reset, standard menu palette use, thin white frames, cursor-only redraw, compact grid labels, and player move display. |
| `include/battle_controllers.h`, `src/battle_controller_player.c`, `src/reshow_battle_screen.c` | Adds the in-battle viewer shortcut, post-reshow action prompt/menu redraw, and held-key release guard after closing the viewer. |
| `src/battle_controller_player.c`, `graphics/battle_interface/team_info_window_l.png`, `graphics/battle_interface/team_info_window_r.png` | Adds the action-menu `R / TEAM / INFO` sprite, Move Info-style slide animation, stale sprite-state recovery after menu returns, and viewer-entry cleanup. |
| `src/battle_setup.c` | Routes eligible trainer battles through viewer -> selection -> battle, and clears viewer state at battle end. |
| `src/battle_main.c`, `include/battle_main.h` | Refactors trainer party generation so preview can build a party without `gBattleStruct`, then battle init can consume the cached party. |
| `src/battle_controller_player.c` | Opens the read-only viewer from `HandleInputChooseAction()` when `B_TEAM_VIEWER_BUTTON` is pressed. |
| `src/party_menu.c`, `include/party_menu.h` | Restores selection MVP choose-half hooks and allows team-viewer-owned selection to return to the viewer on `B`. |
| `src/debug.c` | Adds debug-only `Party -> Team Viewer Battle`, which creates a six-mon player party with moves and starts a normal trainer battle route for repeatable runtime validation. |

## Runtime Contract

Eligible battles are the same normal trainer battles accepted by the battle selection MVP.
The MVP excludes link, Frontier, multi, partner, two-opponent, Pyramid, Trainer Hill,
Secret Base, recorded, and first-battle flows.

Flow:

1. `BattleSetup_StartTrainerBattle()` decides trainer battle flags.
2. `PreBattleTeamViewer_Begin()` creates and stores the opponent party cache.
3. The viewer shows player party and opponent party.
4. `A` closes the viewer and returns to field.
5. The battle selection MVP opens and asks for 3 mons in singles or 4 mons in doubles.
   If the player presses `B` here, the menu reopens the cached team viewer instead of
   escaping the trainer battle script.
6. Battle init calls `PreBattleTeamViewer_LoadCachedOpponentParty()` and copies the cached
   party into `gEnemyParty`.
7. During battle action selection, `R_BUTTON` opens the read-only viewer.
8. While the in-battle viewer is open, battle `callback1` is paused so no action / move /
   target input can advance behind the viewer.
9. Closing the in-battle viewer calls `ReshowBattleScreenAfterMenu()` and returns to action
   selection without emitting a battle command or accepting held-key input.

## UI Notes

Pre-battle:

- left grid: player party icons and slot number;
- right grid: opponent party icons and slot number;
- `A`: continue to selection;
- `SELECT`: detail / strength panel;
- D-pad: move cursor within the 3x2 grid and switch side at the grid edge.

In-battle:

- same party data source;
- `A`, `B`, or `R`: close and return to action menu;
- D-pad and `SELECT` are ignored by design. This mode is display-only so battle action /
  move / target cursors cannot move behind the viewer.

Player-side details show nickname / species, type, level, held item, ability, and four moves.
Opponent-side details show species, type, and level, then explicitly hide private details.

## Validation

| Check | Result | Notes |
|---|---|---|
| `rtk git diff --check` | Pass | No whitespace errors after layout polish and docs updates. |
| `rtk make -j16 -O all` | Pass | Normal ROM built after return redraw / palette-slot fixes. Existing linker RWX warning only. |
| `rtk make -j16 -O debug` | Pass | Debug ROM built after adding `Team Viewer Battle`. Existing linker RWX warning only. |
| `rtk make -j16 -O check` | Pass | Test/check target passed after return redraw / palette-slot fixes. Existing linker RWX warning only. |
| mGBA Live full feature route | Pass | Session `prebattle-team-viewer-real-route` used the debug-only `Party -> Team Viewer Battle` route and confirmed viewer -> 3-mon selection -> trainer battle -> action menu -> R viewer -> B return. |
| mGBA screenshots | Pass | `/tmp/prebattle-team-viewer-real-prebattle.png`, `/tmp/prebattle-team-viewer-real-action-menu.png`, `/tmp/prebattle-team-viewer-real-inbattle.png`, `/tmp/prebattle-team-viewer-real-return.png`. |
| mGBA Live read-only regression | Pass | Session `prebattle-team-viewer-readonly-route` confirmed `R` viewer from action menu, D-pad / `SELECT` ignored, `A` close returns to action menu, and held `A` does not leak into Fight / move selection. |
| mGBA read-only screenshots | Pass | `/tmp/prebattle-team-viewer-readonly-action-menu.png`, `/tmp/prebattle-team-viewer-readonly-inbattle.png`, `/tmp/prebattle-team-viewer-readonly-after-dpad-select.png`, `/tmp/prebattle-team-viewer-readonly-return-after-a.png`, `/tmp/prebattle-team-viewer-readonly-held-a-return.png`. |
| mGBA Live Team Info hint route | Pass | Session `prebattle-team-viewer-team-info-hint` confirmed the adjusted pre-battle footer, 32x32 action-menu `R / TEAM / INFO` hint, read-only in-battle footer prompt, ignored D-pad / `SELECT`, `B` close, and `A` close without entering move selection. Session `prebattle-team-viewer-team-info-dark` rechecked the dark Move Info-style text palette. |
| mGBA Team Info screenshots | Pass | `/tmp/prebattle-team-viewer-team-info-prebattle.png`, `/tmp/prebattle-team-viewer-team-info-action-hint.png`, `/tmp/prebattle-team-viewer-team-info-opened.png`, `/tmp/prebattle-team-viewer-team-info-inbattle.png`, `/tmp/prebattle-team-viewer-team-info-dark-action-hint.png`. |
| mGBA Live Team Info slide route | Pass | Session `prebattle-team-viewer-team-info-slide-v4` confirmed action-menu hint display, no hint residue in Bag, hint recreation after Bag return, Fight transition leaving only `MOVE INFO`, and R viewer entry without hint residue. |
| mGBA Team Info slide screenshots | Pass | `/tmp/prebattle-team-viewer-team-info-slide-v4-action.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-bag-no-residue.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-bag-return.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-fight-moveinfo.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-r-viewer.png`. |
| mGBA Live details persistence route | Pass | Session `prebattle-team-viewer-details-persist` confirmed `SELECT` details stay open while moving right / left through opponent slots and across to player side. |
| mGBA details screenshots | Pass | `/tmp/prebattle-team-viewer-details-persist-initial.png`, `/tmp/prebattle-team-viewer-details-persist-after-right.png`, `/tmp/prebattle-team-viewer-details-persist-after-left.png`, `/tmp/prebattle-team-viewer-details-persist-player-side.png`. |
| mGBA cleanup | Pass | `mgba_live_stop` returned `stopped:true`; `mgba_live_status(all=true)` returned `[]`. |

## Remaining Risks

- Type chips / gender / item badges are still deferred; the current visible team read is
  species icon plus slot number before battle, and icon-only in the in-battle viewer.
- Integrated pick-order selection directly inside the team viewer remains Phase 2. The
  current branch still transitions to the existing choose-half selection screen after `A`.
- Focused mGBA validation used a single normal trainer route. Double battle viewer/selection
  and trainer pool identity checks are still manual follow-ups.
- The opponent party cache intentionally moves trainer pool RNG to encounter start before
  the viewer. Battle uses the same cached result, but this should be called out if comparing
  RNG streams against a no-viewer build.
