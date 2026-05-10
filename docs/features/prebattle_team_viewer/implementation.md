# Pre-Battle / In-Battle Team Viewer Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `feature/prebattle-team-viewer-phase2` from current `master` lineage |
| Code status | Phase 2 implemented; build/check and focused mGBA routes passed |
| Provenance | Local project implementation notes |

## Summary

This branch implements the Phase 2 runtime slice of the team viewer:

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
- opens the standard Pokemon Summary for player-side `SELECT`, starting on the skills/status page;
- keeps opponent-side `SELECT` as a lightweight public-only footer preview;
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
  -> battle -> in-battle viewer path can be rechecked without hunting for a save route,
  plus a `Team Viewer W` route for 4-of-6 double battle validation;
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

Ninth 2026-05-10 follow-up: the pre-battle lightweight detail footer was made persistent
across cursor movement. Previously every D-pad move cleared `showDetails`, so the player
had to press `SELECT` again for each Pokemon. Phase 2 later replaced the player-side
footer with the standard Pokemon Summary shortcut; the persistent footer behavior remains
for opponent-side public preview.

Tenth 2026-05-10 follow-up: Phase 2 integrates party selection into the pre-battle
team viewer. The viewer now owns the pick-order state: `A` toggles eligible player
Pokemon, `START` confirms only after the required 3 / 4 picks, and confirmation writes
`gSelectedOrderFromParty` before returning to the existing battle-start callback. Selected
player slots replace the slot number with the pick order number only, and draw a compact
low-contrast cream/orange marker behind the number so choosing slot 6 first visibly changes
`6` to `1`. The marker draw rectangle and text render offsets are named `#define`s in
`src/prebattle_team_viewer.c`; they are relative to the slot label origin, and the current
tuning shifts the marker slightly right/up to avoid the lower window edge. The existing
`TrainerBattleSelection_StartBattleFromSelection()` path remains the
party compression / restore source of truth after the viewer confirms.

Eleventh 2026-05-10 follow-up: `SELECT` on the player side now opens the normal Pokemon
Summary screen directly on the `POKEMON SKILLS` / status page, then returns to the same
team viewer state on `B`. This uses the existing Summary implementation rather than a
separate viewer screen. `TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER` keeps the default path in
`SUMMARY_MODE_LOCK_MOVES`, but can be set to `1` to use normal Summary move-order behavior.
Opponent-side `SELECT` keeps the lightweight public-only detail footer because the regular
Summary screen would expose private opponent data.

Twelfth 2026-05-10 follow-up: selected marker tuning moved the cream marker and pick-order
text slightly right/up relative to the slot label origin. The marker fill and text printer
background are separate: the rectangle keeps the cream marker, while the pick-order text
uses transparent background / transparent shadow so text rendering does not add another
filled patch. The player Summary mode was also made explicit with
`TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER`, defaulting to locked moves while allowing normal
Summary move-order behavior if set to `1`.

Thirteenth 2026-05-10 follow-up: fixed first-open Summary layout when the viewer opens
directly on `POKEMON SKILLS`. The standard Summary screen normally reaches that page via
the horizontal page-scroll path, which displays the right half of a 512px BG tilemap and
leaves the left half empty so returning to `POKEMON INFO` reveals the info background
behind it. Direct page entry now sets the initial BG order and X coordinate to the same
state as a completed scroll to skills instead of copying the skills tilemap into the left
half. This keeps first-open `POKEMON SKILLS` intact and prevents the skills background from
covering `POKEMON INFO` when the player presses left.

Fourteenth 2026-05-10 follow-up: added the W / double debug route and aligned the
double action-menu hint position with MoveInfo. `Party -> Team Viewer W` reuses the same
six-mon debug player party as the single route but starts Amy & Liv's normal double trainer
battle, so the integrated viewer must select 4/4 before battle start. The `TEAM INFO`
action-menu sprite keeps the same slide animation and uses
`TEAM_VIEWER_ACTION_HINT_Y_DOUBLE = 102`, matching the existing double `MOVE INFO` Y
coordinate while leaving the single-battle Y at 92.

## Files Changed

| File | Change |
|---|---|
| `include/config/battle.h` | Adds `B_TRAINER_BATTLE_SELECTION`, `B_PREBATTLE_TEAM_VIEWER`, `B_IN_BATTLE_TEAM_VIEWER`, `B_TEAM_VIEWER_BUTTON`, and `B_TEAM_VIEWER_DETAILS_BUTTON`. |
| `include/trainer_battle_selection.h`, `src/trainer_battle_selection.c` | Restores the battle selection MVP source slice. |
| `include/prebattle_team_viewer.h`, `src/prebattle_team_viewer.c` | Adds viewer cache, pre-battle screen, Pokemon icon grid, integrated pick-order selection, selected-order label markers, player Summary shortcut, opponent public detail footer, in-battle read-only screen, callback1 pause/restore, OBJ display / priority setup, window-buffer cleanup on entry, BG0 scroll reset, palette/blend reset, standard menu palette use, thin white frames, cursor-only redraw, and compact grid labels. |
| `include/pokemon_summary_screen.h`, `src/pokemon_summary_screen.c` | Adds `ShowPokemonSummaryScreenAtPage()` so the viewer can open the standard Summary directly on the skills/status page. |
| `include/battle_controllers.h`, `src/battle_controller_player.c`, `src/reshow_battle_screen.c` | Adds the in-battle viewer shortcut, post-reshow action prompt/menu redraw, held-key release guard after closing the viewer, and MoveInfo-aligned double `TEAM INFO` hint positioning. |
| `src/battle_controller_player.c`, `graphics/battle_interface/team_info_window_l.png`, `graphics/battle_interface/team_info_window_r.png` | Adds the action-menu `R / TEAM / INFO` sprite, Move Info-style slide animation, stale sprite-state recovery after menu returns, and viewer-entry cleanup. |
| `src/battle_setup.c` | Routes eligible trainer battles through integrated viewer selection -> battle, falls back to the legacy battle selection menu if the viewer cannot start, and clears viewer state at battle end. |
| `src/battle_main.c`, `include/battle_main.h` | Refactors trainer party generation so preview can build a party without `gBattleStruct`, then battle init can consume the cached party. |
| `src/battle_controller_player.c` | Opens the read-only viewer from `HandleInputChooseAction()` when `B_TEAM_VIEWER_BUTTON` is pressed. |
| `src/party_menu.c`, `include/party_menu.h` | Restores selection MVP choose-half hooks for fallback / non-viewer paths. Phase 2 viewer-owned selection no longer enters the party menu before battle. |
| `src/debug.c` | Adds debug-only `Party -> Team Viewer Battle` and `Party -> Team Viewer W`, which create a six-mon player party with moves and start normal single / double trainer battle routes for repeatable runtime validation. |

## Runtime Contract

Eligible battles are the same normal trainer battles accepted by the battle selection MVP.
The MVP excludes link, Frontier, multi, partner, two-opponent, Pyramid, Trainer Hill,
Secret Base, recorded, and first-battle flows.

Flow:

1. `BattleSetup_StartTrainerBattle()` decides trainer battle flags.
2. `PreBattleTeamViewer_Begin()` receives the required pick count and creates the opponent party cache.
3. The viewer shows player party and opponent party.
4. Player-side `A` toggles selected Pokemon. Opponent-side `A` is ignored.
5. Selected player slots show the pick order number with a compact yellow/orange marker.
6. `START` confirms only when the required count is selected, writes `gSelectedOrderFromParty`,
   and returns through `CB2_StartTrainerBattleAfterPartySelection`.
7. `TrainerBattleSelection_StartBattleFromSelection()` compresses the selected Pokemon into
   battle order, preserving the existing restore semantics.
8. Battle init calls `PreBattleTeamViewer_LoadCachedOpponentParty()` and copies the cached
   party into `gEnemyParty`.
9. During battle action selection, `R_BUTTON` opens the read-only viewer.
10. While the in-battle viewer is open, battle `callback1` is paused so no action / move /
   target input can advance behind the viewer.
11. Closing the in-battle viewer calls `ReshowBattleScreenAfterMenu()` and returns to action
   selection without emitting a battle command or accepting held-key input.

## UI Notes

Pre-battle:

- left grid: player party icons and slot number;
- right grid: opponent party icons and slot number;
- player-side `A`: pick / unpick that Pokemon;
- `START`: start battle after exactly the required number of picks;
- player-side `SELECT`: normal Summary skills/status page;
- opponent-side `SELECT`: public-only detail footer;
- D-pad: move cursor within the 3x2 grid and switch side at the grid edge.
- selected player labels replace the slot number with pick order (`1`, `2`, `3`, `4`)
  and use a compact yellow/orange marker.

In-battle:

- same party data source;
- `A`, `B`, or `R`: close and return to action menu;
- D-pad and `SELECT` are ignored by design. This mode is display-only so battle action /
  move / target cursors cannot move behind the viewer.

Player-side details use the standard Summary skills/status page. Opponent-side details show
species, type, and level, then explicitly hide private details.

## Validation

| Check | Result | Notes |
|---|---|---|
| `rtk git diff --check` | Pass | No whitespace errors after W debug route and MoveInfo-aligned double hint positioning. |
| `rtk make -j16 -O all` | Pass | Normal ROM built after W debug route and MoveInfo-aligned double hint positioning. Existing linker RWX warning only. |
| `rtk make -j16 -O debug` | Pass | Debug ROM built after W debug route and MoveInfo-aligned double hint positioning. Existing linker RWX warning only. |
| `rtk make -j16 -O check` | Pass | Test/check target passed after W debug route and MoveInfo-aligned double hint positioning. Existing linker RWX warning plus existing known learnset KNOWN_FAILING line only. |
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
| mGBA Live Summary / Phase 2 marker route | Pass | Session `prebattle-team-viewer-summary-marker` used `Party -> Team Viewer Battle`, opened player Summary on the skills/status page via `SELECT`, returned to the viewer, selected slot 6 first, confirmed its label changed from `6` to orange-brown `1` on a low-contrast cream marker, selected three Pokemon, and reached trainer battle. |
| mGBA Summary / Phase 2 screenshots | Pass | `/tmp/prebattle-team-viewer-summary-marker-start.png`, `/tmp/prebattle-team-viewer-summary-skills-page.png`, `/tmp/prebattle-team-viewer-summary-marker-slot6-first.png`, `/tmp/prebattle-team-viewer-summary-marker-three-selected.png`, `/tmp/prebattle-team-viewer-summary-marker-battle-start.png`. |
| mGBA Live marker-adjust route | Pass | Session `prebattle-team-viewer-marker-adjust` confirmed the tuned slot-6-first marker avoids the lower window edge, player-side `SELECT` opens the existing Summary skills/status page, and `B` returns to the integrated viewer with selection state intact. |
| mGBA marker-adjust screenshots | Pass | `/tmp/prebattle-team-viewer-marker-adjust-slot6-first.png`, `/tmp/prebattle-team-viewer-marker-adjust-summary-skills.png`. |
| mGBA Live transparent-text route | Pass | Session `prebattle-team-viewer-transparent-text` confirmed slot 6 selected first still shows the cream marker but the pick-order text does not paint its own background / shadow patch. |
| mGBA transparent-text screenshot | Pass | `/tmp/prebattle-team-viewer-transparent-text-slot6-first.png`. |
| mGBA Live initial Summary layout route | Pass | Session `prebattle-team-viewer-summary-initial-layout` confirmed the first player-side `SELECT` from the viewer opens `POKEMON SKILLS` without layout corruption, and `B` returns to the integrated viewer. |
| mGBA initial Summary layout screenshot | Pass | `/tmp/prebattle-team-viewer-summary-initial-layout-fixed.png`. |
| mGBA Live Summary info regression route | Pass | Session `prebattle-team-viewer-summary-info-layout-fixed` confirmed `SELECT` opens `POKEMON SKILLS`, `LEFT` returns to `POKEMON INFO` without the skills background remaining, `RIGHT` returns to `POKEMON SKILLS`, and `B` returns to the integrated viewer. |
| mGBA Summary info regression screenshots | Pass | `/tmp/prebattle-team-viewer-summary-info-layout-skills-fixed.png`, `/tmp/prebattle-team-viewer-summary-info-layout-info-fixed.png`. |
| mGBA Live W / double debug route | Pass | Session `prebattle-team-viewer-w-debug3` used `Party -> Team Viewer W`, selected 4/4 Pokemon, reached Amy & Liv's double battle action menu, confirmed the `R / TEAM / INFO` hint, and opened the in-battle viewer with `R`. Session `prebattle-team-viewer-w-moveinfo-aligned` rechecked the action hint after restoring the double Y coordinate to match MoveInfo. |
| mGBA W / double screenshots | Pass | `/tmp/prebattle-team-viewer-w-debug-route-start.png`, `/tmp/prebattle-team-viewer-w-debug-four-selected.png`, `/tmp/prebattle-team-viewer-w-debug-battle-intro.png`, `/tmp/prebattle-team-viewer-w-debug-action-hint.png`, `/tmp/prebattle-team-viewer-w-debug-inbattle-viewer.png`, `/tmp/prebattle-team-viewer-w-debug-moveinfo-aligned-action-hint.png`. |
| mGBA cleanup | Pass | `mgba_live_stop` returned `stopped:true`; `mgba_live_status(all=true)` returned `[]`. |

## Remaining Risks

- Type chips / gender / item badges are still deferred; the current visible team read is
  species icon plus slot number before battle, and icon-only in the in-battle viewer.
- Focused mGBA validation covered the single debug route and W / double debug route.
  Trainer Party Pool / randomized party identity checks are still manual follow-ups.
- The opponent party cache intentionally moves trainer pool RNG to encounter start before
  the viewer. Battle uses the same cached result, but this should be called out if comparing
  RNG streams against a no-viewer build.
