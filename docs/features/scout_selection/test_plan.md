# Scout Selection Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `e927b612b3`; `feature/scout-selection-runtime-20260520` |
| Code status | Runtime MVP implemented; focused mGBA validation passed |
| Provenance | Project runtime validation policy and related feature test plans |

## Build / Lint

| Test | Command | Expected |
|---|---|---|
| Whitespace / patch sanity | `rtk git diff --check` | No errors. |
| Scout generated pool | `rtk make generated` | `src/data/scout_selection_pools.h` regenerates from partygen JSON, de-duplicates species, and remains ignored. |
| Normal ROM | `rtk make -j16 -O all` | Build passes. Existing RWX linker warning is acceptable if unchanged. |
| Debug ROM | `rtk make -j16 -O debug` | Build passes. Required for debug scout route validation. |
| Full checks | `rtk make -j16 -O check` | Suite exits 0; existing expected / known failing markers are recorded if present. |
| Focused Pokemon/gift tests | `rtk make -j16 -O check TESTS=test/pokemon.c` or focused equivalent | Gift / candidate helper tests pass if added. |
| Docs | `rtk mdbook build docs` | Docs build passes; existing warnings are recorded separately from new warnings. |

## Focused Runtime Checks

| Check | Steps | Expected |
|---|---|---|
| Open UI | Trigger debug/test NPC or object. | Scout screen opens from field, shows candidates, and does not softlock. |
| Icon display | Inspect all visible candidates. | Each visible Pokemon has an icon and label; no blank OBJ / palette corruption. |
| Generated species source | Inspect the first visible candidates. | The screen shows partygen-derived candidates such as Metang / Skarmory, not the old starter dummy pool. |
| Cursor movement | Move through all visible rows and across scroll boundary. | Cursor updates without shifting layout or losing selected markers. |
| Select 1 | Pick one candidate and confirm. | Candidate is given to party / PC and script receives non-cancel result. |
| Select N | Configure pick count 2 or 3. | `START` fails before required count, succeeds at exact count, gives in selected order. |
| Summary | Open Summary on a candidate, move pages, press `B`. | Returns to Scout UI with same cursor, scroll, and selected markers. |
| Summary repeat | Open Summary twice for different candidates. | No window buffer leak, stale sprite, or corrupted Summary page. |
| Cancel | Press `B` before confirm. | UI closes, no Pokemon is given, script can branch on cancel. |
| Party full / PC fallback | Fill party and leave PC space. | Pokemon is sent to PC or existing gift result behavior is preserved. |
| No space | Fill party / PC if feasible. | `MON_CANT_GIVE` path is reached without partial N-pick grant. |
| Reopen route | Open, cancel, open again. | Icons and windows initialize cleanly both times. |

## mGBA Live Plan

Use the script-capable mGBA build required by `AGENTS.md`.

Minimum evidence before push on a runtime branch:

1. Boot debug ROM.
2. Trigger the debug/test scout route.
3. Capture screenshot of candidate screen with icons.
4. Open Summary for one candidate and return.
5. Select and confirm one Pokemon.
6. Verify party / PC result through party menu, script message, or Lua memory read.
7. Stop mGBA Live and record cleanup state.

If mGBA Live cannot validate the target behavior, record the exact failure and manual
gap here before handoff.

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-19 | Docs-only investigation | Not run | No runtime source exists on this docs branch. |
| 2026-05-20 | `rtk make -j16 -O debug` | Pass | Debug ROM builds with existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning. Earlier parallel build collision hit unrelated `link_rfu_2.o`; clean rerun passed. |
| 2026-05-20 | `rtk make -j16 -O check` | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | mGBA Live `scout-selection-runtime-20260520d` | Pass | Booted `/tmp/mgba-scout-selection-20260520/scout.gba`, continued saved game, opened debug route, verified visible icons and 12-candidate scroll. |
| 2026-05-20 | Summary runtime | Pass | Pressed `SELECT` on Chikorita, Summary opened, `B` returned to Scout UI with cursor and scroll preserved. Screenshot: `/tmp/mgba-scout-selection-20260520/scout-summary.png`. |
| 2026-05-20 | Confirm / gift runtime | Pass | Selected Chikorita, `START` returned to field, message `Scout Pokemon received.` displayed, and party menu showed Chikorita Lv.15 in slot 2. Screenshots: `scout-confirm.png`, `scout-party.png`. |
| 2026-05-20 | mGBA cleanup | Pass | `mgba_live_stop` stopped session `scout-selection-runtime-20260520d`; `rtk pgrep -af mgba` showed no remaining `mgba-qt` process. |
| 2026-05-20 | GitHub Actions for PR #51 | Pending / not waited | `rtk gh pr checks 51` showed build/docs/test/release jobs pending. Per branch runtime policy, local make + mGBA evidence are recorded and long Actions were not re-waited. |
| 2026-05-20 | `rtk make generated` after partygen pool update | Pass | Regenerated ignored `src/data/scout_selection_pools.h` from `hoenn_demo.json` and `elite_four.json`; first 12 unique species are Metang, Skarmory, Aggron, Mightyena, Wobbuffet, Geodude, Zigzagoon, Shiftry, Cacturne, Crawdaunt, Absol, and Sharpedo. |
| 2026-05-20 | `rtk git diff --check` | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O debug` after partygen pool update | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` after partygen pool update | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` after partygen pool update | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| 2026-05-20 | mGBA Live `scout-selection-partygen-20260520` | Pass | Opened debug route, verified partygen-derived Metang / Skarmory screen, scrolled to Zigzagoon / Shiftry, opened Zigzagoon Summary, returned, selected Zigzagoon, confirmed, saw `Scout Pokemon received.`, and verified Zigzagoon Lv.50 in party. Screenshots are under `/tmp/mgba-scout-selection-20260520/`. |
| 2026-05-20 | mGBA cleanup after partygen pool update | Pass | `mgba_live_stop` stopped session `scout-selection-partygen-20260520`; `rtk pgrep -af mgba` showed no remaining `mgba-qt` process. |
| 2026-05-20 | `rtk make -j16 -O debug` after UI polish | Pass | Existing RWX linker warning. |
| 2026-05-20 | mGBA Live `scout-selection-ui-polish-20260520` | Pass | Opened debug route, verified blue header/footer bars, white card layout, cursor stripe, selected green card, scroll, Summary return, and confirm message. Screenshots: `scout-ui-polish-open.png`, `scout-ui-polish-selected.png`, `scout-ui-polish-scroll.png`, `scout-ui-polish-confirm.png`. |
| 2026-05-20 | mGBA cleanup after UI polish | Pass | `mgba_live_stop` stopped session `scout-selection-ui-polish-20260520`; `rtk pgrep -af mgba` showed no remaining `mgba-qt` process. |
| 2026-05-20 | `rtk git diff --check` after UI polish docs | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O all` after UI polish | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` after UI polish | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` after UI polish | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| 2026-05-20 | Debug route configured for 6-pick test | Pass | `Scripts... > Script 2` sets `SCOUT_POOL_PARTYGEN_DEMO`, candidate count 12, and pick count 6. |
| 2026-05-20 | mGBA Live `scout-selection-pick6-20260520` | Pass | Opened `Script 2`, verified `0/6`, selected five candidates and confirmed START was blocked with `Pick the requested number first.`, selected the sixth candidate, confirmed, saw `Scout Pokemon received.`, and verified the party filled with the first five selected Pokemon after Mew. The sixth selected Pokemon used the existing gift path PC fallback after party filled. Screenshots: `scout-pick6-open.png`, `scout-pick6-five-selected.png`, `scout-pick6-start-blocked.png`, `scout-pick6-six-selected.png`, `scout-pick6-confirm.png`, `scout-pick6-party.png`. |
| 2026-05-20 | mGBA cleanup after 6-pick test | Pass | `mgba_live_stop` stopped session `scout-selection-pick6-20260520`; `rtk pgrep -af mgba` showed no remaining `mgba-qt` process. |
| 2026-05-20 | `rtk git diff --check` after 6-pick route | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O debug` after 6-pick route | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` after 6-pick route | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` after 6-pick route | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` after 6-pick route | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| 2026-05-20 | Cursor redraw optimization | Pass | Cursor movement and A selection no longer recreate icons unless scrolling changes the visible candidate set. mGBA Live `scout-selection-redraw-opt-20260520` ran at 60 fps target and verified cursor movement, selected markers, scroll-boundary icon refresh, Summary open/return, and clean stop. Screenshots: `scout-redraw-opt-open.png`, `scout-redraw-opt-scroll.png`, `scout-redraw-opt-summary-return.png`. |
| 2026-05-20 | `rtk git diff --check` after cursor redraw optimization | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O debug` after cursor redraw optimization | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` after cursor redraw optimization | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` after cursor redraw optimization | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` after cursor redraw optimization | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| 2026-05-20 | Cursor responsiveness pass | Pass | Replaced Scout D-pad movement with `JOY_REPEAT`, applied Scout-local 10-frame initial / 3-frame continued repeat, and changed same-scroll cursor movement to redraw only previous/current card rows. mGBA Live `scout-selection-responsive-20260520b` ran at 60 fps target, opened `Scripts... > Scout Selection`, held DOWN for repeat movement and scroll, selected Metang, opened/returned from Summary, and stopped cleanly. Screenshots: `scout-responsive-hold-scroll.png`, `scout-responsive-return.png`. |
| 2026-05-20 | mGBA fixed-binary retry note | Pass | First attempt `scout-selection-responsive-20260520` exited before boot with Qt xcb / DISPLAY unset when the script-capable binary was passed directly. Retried through the project wrapper as `scout-selection-responsive-20260520b`; validation passed and `mgba-live-cli status --all` returned `[]`. |
| 2026-05-20 | `rtk git diff --check` after cursor responsiveness pass | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O debug` after cursor responsiveness pass | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` after cursor responsiveness pass | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` after cursor responsiveness pass | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` after cursor responsiveness pass | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| 2026-05-20 | Cursor repeat tuning | Pass | Scout-local repeat changed from 10-frame initial / 3-frame continued to 16-frame initial / 5-frame continued to reduce accidental two-step movement while preserving hold-to-scroll. mGBA Live `scout-selection-repeat-tune-20260520` ran at 60 fps target, verified a 12-frame DOWN tap moved one row, a 12-frame UP tap moved one row, a 45-frame hold still repeated through scroll, and stopped cleanly with empty CLI `status --all`. Screenshots: `scout-repeat-tune-single-step.png`, `scout-repeat-tune-hold-scroll.png`. |
| 2026-05-20 | `rtk git diff --check` after cursor repeat tuning | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O debug` after cursor repeat tuning | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` after cursor repeat tuning | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` after cursor repeat tuning | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` after cursor repeat tuning | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |

## Feature Complete Gate

- Runtime branch builds normal and debug ROMs.
- Full `check` or justified focused checks pass.
- mGBA Live covers open -> scroll -> Summary -> return -> confirm.
- `test_plan.md` records exact commands, screenshots / session names, and any skipped
  long GitHub Actions waits.
- `implementation.md` is added after source work lands.

## Open Questions

- Which debug map/NPC should own the first non-debug-menu route?
- Should we add a Lua helper for deterministic candidate/party setup?
