# Pre-Battle / In-Battle Team Viewer Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `feature/prebattle-team-viewer-phase2` |
| Code status | Phase 2 integrated selection implemented; build/check and focused mGBA route passed |
| Provenance | Local project feature docs |

## Current Validation

2026-05-10 Phase 2 implementation branch.

| Check | Result | Notes |
|---|---|---|
| Branch baseline review | Pass | Continued on `feature/prebattle-team-viewer-phase2`, originally created from current `master` lineage. |
| Previous selection branch review | Pass | Use source slice only; do not merge old branch wholesale. |
| Legacy console web reference review | Pass | Colosseum / XD / Battle Revolution notes added as secondary references. |
| Battle selection source reapply | Pass | Reapplied source slice from `feature/battle-selection-mvp`; did not merge old branch docs. |
| User manual screen check | Fail -> fixed | First viewer appeared, but footer/lower screen was corrupt. Root cause was BG charbase/mapblock overlap; fixed by moving viewer BG charbase to 0. |
| User manual icon check | Fail -> fixed | Labels appeared but Pokemon icons did not. Root cause was missing OBJ display enable, with BG priority also able to hide OBJ; viewer now sets `DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP` and lowers BG priority. |
| User manual malloc check | Fail -> fixed | Later battle path hit a malloc blue screen while allocating window memory. Root cause was entering viewer without freeing previous window buffers before `InitWindows()`; viewer now frees old window buffers first. |
| User manual layout check | Fail -> fixed | D-pad movement briefly refreshed the divider / lower bar because cursor movement redrew the full viewer and recreated icons. Cursor movement now redraws only text label areas; icons stay alive until viewer close. |
| User manual in-battle BG check | Fail -> fixed | In-battle viewer showed icons over a black upper area because battle BG0 scroll state could leak into the viewer. Viewer init now resets BG0 control coordinates and hardware scroll registers. |
| User manual in-battle tint check | Fail -> fixed | Opening the viewer immediately after action menu appeared could inherit a red battle-intro tint. Viewer init now resets palette fade / blend state before loading its palettes. |
| User manual in-battle palette check | Fail -> fixed | Red tint persisted until the viewer stopped using palette 15. Viewer windows and BG clear now use `STD_WINDOW_PALETTE_NUM`, matching `Menu_LoadStdPal()`. |
| User manual in-battle return check | Fail -> fixed | Closing viewer with `B` could leave a blank battle textbox/action menu. Viewer close now sets a redraw flag, and battle-screen reshow calls back into the player controller to reprint the action prompt/menu. |
| User manual in-battle input leak check | Fail -> fixed | `callback1` kept running behind the viewer, so D-pad / `SELECT` / `A` could advance battle action, move, or target state. In-battle viewer now pauses `gMain.callback1`, ignores D-pad / `SELECT`, and waits for all held keys to release before action-menu input resumes. |
| User manual footer / hint check | Fail -> fixed | Pre-battle footer split and prompt lines were lowered for more room. In-battle viewer footer now always says read-only, and the battle action menu shows a small 32x32 `R / TEAM / INFO` hint so the shortcut is visible. |
| User manual hint animation check | Fail -> fixed | `TEAM INFO` now mirrors the `MOVE INFO` slide model: X = -14 to X = 14 on show, then back to X = -14 before freeing. Stale active state after Bag / menu sprite resets is detected by tile-tag presence and rebuilt on action-menu return. |
| Icon-grid follow-up | Pass | Team rows use Pokemon icon sprites with compact slot-only pre-battle labels instead of name-heavy text rows; side labels moved into the top header to avoid icon overlap. |
| Legacy strength-panel follow-up | Superseded | Earlier player-side footer details were validated, then replaced by the standard Pokemon Summary shortcut. Opponent private details remain hidden. |
| Opponent public-detail cursor follow-up | Pass | Opponent-side `SELECT` detail footer remains lightweight/public-only and continues to update with cursor movement. |
| Phase 2 integrated selection | Pass | Pre-battle viewer owns selection state. `A` toggles player-side picks, `START` confirms at required count, and the existing selection compression/restore path starts the battle. |
| Phase 2 pick-order marker | Pass | Selected player slots replace the slot number with the selected order number and draw a compact low-contrast cream/orange marker. Slot 6 picked first displays `1`, not `P1` or `6`; marker/text offsets are tunable relative to the label origin, and the text printer uses transparent background / shadow so it does not paint over the marker. |
| Player Summary transition | Pass | Player-side `SELECT` opens the existing normal Summary screen on the skills/status page, then `B` returns to the integrated viewer with selection state intact. Default mode locks move reordering; `TEAM_VIEWER_SUMMARY_ALLOW_MOVE_REORDER` can opt into normal behavior. |
| In-battle viewer layout follow-up | Pass | In-battle grid labels are hidden and use the same thin white window style as pre-battle. |
| Selection back path | Pass | Team-viewer-owned choose-half selection now uses `B` to return to the cached team viewer. |
| Debug validation route | Pass | Added debug-only `Party -> Team Viewer Battle` and `Party -> Team Viewer W`, which give the player six mons with moves and start repeatable single / double trainer routes that exercise viewer -> selection -> battle. |
| `rtk git diff --check` | Pass | No whitespace errors after W debug route and MoveInfo-aligned double hint positioning. |
| `rtk make -j16 -O all` | Pass | Normal ROM built after W debug route and MoveInfo-aligned double hint positioning; existing linker RWX warning only. |
| `rtk make -j16 -O debug` | Pass | Debug ROM built after W debug route and MoveInfo-aligned double hint positioning; existing linker RWX warning only. |
| `rtk make -j16 -O check` | Pass | Test/check target passed after W debug route and MoveInfo-aligned double hint positioning; existing linker RWX warning plus existing known learnset KNOWN_FAILING line only. |
| mGBA Live focused route | Pass | Session `prebattle-team-viewer-real-route` booted the ROM, entered debug menu, selected `Party -> Team Viewer Battle`, confirmed pre-battle viewer, selected three mons, reached trainer battle, opened in-battle viewer with `R`, and returned with `B` to a visible action menu. |
| mGBA screenshots | Pass | `/tmp/prebattle-team-viewer-real-prebattle.png`, `/tmp/prebattle-team-viewer-real-action-menu.png`, `/tmp/prebattle-team-viewer-real-inbattle.png`, `/tmp/prebattle-team-viewer-real-return.png`. |
| mGBA Live read-only regression | Pass | Session `prebattle-team-viewer-readonly-route` confirmed action-menu `R` opens the viewer, D-pad / `SELECT` do not change screens, `A` close returns to action menu, and held `A` after close does not enter Fight / move selection. |
| mGBA read-only screenshots | Pass | `/tmp/prebattle-team-viewer-readonly-action-menu.png`, `/tmp/prebattle-team-viewer-readonly-inbattle.png`, `/tmp/prebattle-team-viewer-readonly-after-dpad-select.png`, `/tmp/prebattle-team-viewer-readonly-return-after-a.png`, `/tmp/prebattle-team-viewer-readonly-held-a-return.png`. |
| mGBA Live Team Info hint route | Pass | Session `prebattle-team-viewer-team-info-hint` confirmed adjusted pre-battle footer, battle action-menu `R / TEAM / INFO` hint, read-only in-battle footer prompt, ignored D-pad / `SELECT`, `B` close back to action menu, and `A` close without entering move selection. Session `prebattle-team-viewer-team-info-dark` rechecked the dark Move Info-style text palette. |
| mGBA Team Info screenshots | Pass | `/tmp/prebattle-team-viewer-team-info-prebattle.png`, `/tmp/prebattle-team-viewer-team-info-action-hint.png`, `/tmp/prebattle-team-viewer-team-info-opened.png`, `/tmp/prebattle-team-viewer-team-info-inbattle.png`, `/tmp/prebattle-team-viewer-team-info-dark-action-hint.png`. |
| mGBA Live Team Info slide route | Pass | Session `prebattle-team-viewer-team-info-slide-v4` confirmed action-menu hint display, no hint residue in Bag, hint recreation after Bag return, Fight transition leaving only `MOVE INFO`, and R viewer entry without hint residue. |
| mGBA Team Info slide screenshots | Pass | `/tmp/prebattle-team-viewer-team-info-slide-v4-action.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-bag-no-residue.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-bag-return.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-fight-moveinfo.png`, `/tmp/prebattle-team-viewer-team-info-slide-v4-r-viewer.png`. |
| mGBA Live details persistence route | Pass | Session `prebattle-team-viewer-details-persist` confirmed `SELECT` details stay open while moving right / left through opponent slots and across to player side. |
| mGBA details screenshots | Pass | `/tmp/prebattle-team-viewer-details-persist-initial.png`, `/tmp/prebattle-team-viewer-details-persist-after-right.png`, `/tmp/prebattle-team-viewer-details-persist-after-left.png`, `/tmp/prebattle-team-viewer-details-persist-player-side.png`. |
| mGBA Live Summary / Phase 2 marker route | Pass | Session `prebattle-team-viewer-summary-marker` confirmed `SELECT` opens the Summary skills/status page, `B` returns to viewer, slot 6 selected first changes to orange-brown `1` on a smaller cream marker, three selected markers display `2`, `3`, `1` by pick order, and `START` reaches trainer battle. |
| mGBA Summary / Phase 2 screenshots | Pass | `/tmp/prebattle-team-viewer-summary-marker-start.png`, `/tmp/prebattle-team-viewer-summary-skills-page.png`, `/tmp/prebattle-team-viewer-summary-marker-slot6-first.png`, `/tmp/prebattle-team-viewer-summary-marker-three-selected.png`, `/tmp/prebattle-team-viewer-summary-marker-battle-start.png`. |
| mGBA Live marker-adjust route | Pass | Session `prebattle-team-viewer-marker-adjust` confirmed the adjusted slot-6-first marker is shifted right/up enough to avoid the lower window edge, player-side `SELECT` opens the existing Summary skills/status page, and `B` returns to the viewer with pick-order state intact. |
| mGBA marker-adjust screenshots | Pass | `/tmp/prebattle-team-viewer-marker-adjust-slot6-first.png`, `/tmp/prebattle-team-viewer-marker-adjust-summary-skills.png`. |
| mGBA Live transparent-text route | Pass | Session `prebattle-team-viewer-transparent-text` confirmed slot 6 selected first keeps the cream marker while the pick-order text uses transparent background / shadow, so text rendering does not add another filled patch. |
| mGBA transparent-text screenshot | Pass | `/tmp/prebattle-team-viewer-transparent-text-slot6-first.png`. |
| mGBA Live initial Summary layout route | Pass | Session `prebattle-team-viewer-summary-initial-layout` confirmed the first player-side `SELECT` from the viewer opens the existing Summary skills/status page without layout corruption, and `B` returns to the viewer. |
| mGBA initial Summary layout screenshot | Pass | `/tmp/prebattle-team-viewer-summary-initial-layout-fixed.png`. |
| mGBA Live Summary info regression route | Pass | Session `prebattle-team-viewer-summary-info-layout-fixed` confirmed `SELECT` opens `POKEMON SKILLS`, `LEFT` returns to `POKEMON INFO` without the skills background remaining, `RIGHT` returns to `POKEMON SKILLS`, and `B` returns to the viewer. |
| mGBA Summary info regression screenshots | Pass | `/tmp/prebattle-team-viewer-summary-info-layout-skills-fixed.png`, `/tmp/prebattle-team-viewer-summary-info-layout-info-fixed.png`. |
| mGBA Live W / double debug route | Pass | Session `prebattle-team-viewer-w-debug3` used `Party -> Team Viewer W`, selected 4/4 Pokemon in the integrated viewer, reached Amy & Liv's double battle action menu, confirmed the `R / TEAM / INFO` hint, and opened the in-battle viewer with `R`. Session `prebattle-team-viewer-w-moveinfo-aligned` rechecked the action hint after restoring the double Y coordinate to match MoveInfo. |
| mGBA W / double screenshots | Pass | `/tmp/prebattle-team-viewer-w-debug-route-start.png`, `/tmp/prebattle-team-viewer-w-debug-four-selected.png`, `/tmp/prebattle-team-viewer-w-debug-battle-intro.png`, `/tmp/prebattle-team-viewer-w-debug-action-hint.png`, `/tmp/prebattle-team-viewer-w-debug-inbattle-viewer.png`, `/tmp/prebattle-team-viewer-w-debug-moveinfo-aligned-action-hint.png`. |
| mGBA cleanup | Pass | `mgba_live_stop` returned `stopped:true`; `mgba_live_status(all=true)` returned `[]`. |

## Required Build Checks After Implementation

| Command / Check | When | Expected |
|---|---|---|
| `rtk git diff --check` | every patch | no whitespace errors |
| `rtk make -j16 -O all` | source implementation | normal ROM builds |
| `rtk make -j16 -O debug` | if debug route / debug menu is used | debug ROM builds |
| `rtk make -j16 -O check` or focused `TESTS=...` | party / battle logic changes | test build passes or known failures documented |
| mGBA Live focused check | before push | boot plus viewer -> selection -> battle -> in-battle viewer evidence |

## Manual Test Matrix

| Case | Setup | Expected |
|---|---|---|
| Single trainer, player has 6 | normal trainer battle | Viewer appears, shows player 6 and opponent party, then selection asks for 3. |
| Double trainer, player has 6 | normal double trainer | Viewer appears, then selection asks for 4. |
| Player has required count only | 3 for single or 4 for double | Viewer does not appear if selection is skipped by contract. |
| Opponent static party | non-pool trainer | Viewer species match battle enemy party. |
| Opponent pool party | trainer with `Party Size` / pool | Viewer species/order match battle enemy party exactly. |
| Override trainer | trainer with `overrideTrainer` | Viewer uses the same effective party as battle. |
| Viewer pick | highlight eligible player Pokemon and press A | Toggles selected state. Selected label becomes pick order number with yellow/orange marker. |
| Viewer unpick | press A on an already selected player Pokemon | Removes that Pokemon from selected order and compacts later pick-order labels. |
| Viewer confirm too early | press START before required count | Failure beep; no battle starts. |
| Viewer confirm ready | press START after required count | Starts battle once through the existing selection compression/restore path. |
| Viewer B / cancel | press B in pre-battle viewer | Trainerbattle script does not escape. Pre-battle viewer remains the selection surface; in-battle viewer closes with `B`. |
| Viewer icon layout | open viewer | Player and opponent sides show Pokemon icons in a 3x2 grid with slot-only labels; header labels do not overlap icons; footer area is not corrupted. |
| Viewer cursor movement | move through both sides with D-pad | Divider / label area does not visibly blank, and Pokemon icons do not disappear or jump during cursor movement. |
| Player Summary view | highlight player mon and press `SELECT` | Standard Pokemon Summary opens directly on the skills/status page. `B` returns to the viewer with pick-order state preserved. |
| Opponent public detail view | highlight opponent mon and press `SELECT` | MVP shows public summary only unless official Champions behavior is verified to expose more. |
| In-battle viewer shortcut | at player action menu, press configured button | Read-only viewer opens with white thin-frame windows and shows selected player team plus cached opponent team. |
| In-battle shortcut hint | at player action menu | A small 32x32 `R / TEAM / INFO` sprite appears near the left side of the battlefield with the same background style as `MOVE INFO`, and disappears before any normal action dispatch or viewer entry. |
| In-battle shortcut hint animation | enter / leave action menu, open Bag, return from Bag, choose Fight | Hint slides in like `MOVE INFO`, slides out or is cleared before non-action-menu screens, recreates after Bag return, and does not overlap the move menu after Fight. |
| In-battle read-only input | open in-battle viewer and press D-pad / `SELECT` | Viewer remains on the same display. No battle action, move, target, debug, or summary input should advance behind it. |
| In-battle labels | open in-battle viewer | Pokemon grid labels are hidden; D-pad does not produce colored block artifacts or cursor movement. |
| In-battle viewer close | press A / B / configured button | Returns to a visible action menu with cursor and pending command unchanged; no hidden move-selection input should remain active. |
| In-battle held close input | hold A / B / R while closing viewer | Returns to action menu and waits for key release; held input must not choose Fight, Bag, Pokémon, Run, move, or target. |
| In-battle move menu | open Fight then press viewer button | MVP does not open viewer from move menu unless explicitly implemented. Existing move description / gimmick behavior remains. |
| Selection restore | choose non-leading slots | Battle end restores player party to original order with selected mons updated. |
| Whiteout / loss | lose after selecting | Team viewer state clears; selection restore still occurs before whiteout / return flow. |
| Already beaten trainer | talk after trainer flag | Viewer does not appear. |

## Exclusion Regression Tests

| Area | Expected |
|---|---|
| Wild battle | Viewer never appears. |
| Battle Frontier | Existing facility flow unchanged. |
| Cable / link / Union Room | Existing selection / link flow unchanged. |
| Wild battle | Last-used-ball shortcut behavior remains unchanged. |
| Two approaching trainers | Viewer skipped for MVP. |
| Follower partner battle | Viewer skipped for MVP. |
| Pyramid / Trainer Hill | Existing prebuilt enemy party flow unchanged. |
| Secret Base | Viewer skipped for MVP. |
| First battle tutorial | Viewer skipped. |

## Runtime Evidence To Record

Implementation handoff should record:

- local build command and result,
- mGBA Live session id / screenshot evidence,
- whether script-capable mGBA wrapper was used,
- exact trainer used for single and double checks,
- button used for in-battle viewer and any shortcut conflicts observed,
- screenshot evidence that the icon grid and footer render without corruption,
- whether integrated viewer pick / unpick / confirm works,
- whether pool / random party identity was confirmed,
- whether `Y` / detail view was validated pre-battle; in-battle detail is intentionally disabled,
- whether opponent hidden details were intentionally hidden or officially confirmed public,
- any cleanup issue from `mgba-live-cli stop`.

## Open Questions

- Add an automated test for "preview cache copied to `gEnemyParty`" once helper boundaries exist.
- Add a test-only debug assertion that cached preview species/order equals battle species/order.
- Add a test hook that opens and closes in-battle viewer without emitting a battle command,
  including held-key release gating.
- Add a screenshot check for `Y` detail overlay once the layout is stable.
- Decide whether type icon rendering needs a screenshot pixel check or manual screenshot is enough.
- Add double-battle focused runtime evidence for the 4-of-6 path.
- Add trainer-pool focused runtime evidence that preview species/order matches battle
  species/order for randomized parties.
