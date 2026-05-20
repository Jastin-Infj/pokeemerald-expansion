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
