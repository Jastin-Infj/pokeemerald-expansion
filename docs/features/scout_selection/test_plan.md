# Scout Selection Test Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `8bb44a15f4`; `git describe` = `expansion/1.15.2-77-g8bb44a15f4` |
| Code status | Planned runtime validation |
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

## Feature Complete Gate

- Runtime branch builds normal and debug ROMs.
- Full `check` or justified focused checks pass.
- mGBA Live covers open -> Summary -> return -> confirm.
- `test_plan.md` records exact commands, screenshots / session names, and any skipped
  long GitHub Actions waits.
- `implementation.md` is added after source work lands.

## Open Questions

- Which debug map/NPC should own the first route?
- Which candidate pool should be used for first manual validation?
- Should we add a Lua helper for deterministic candidate/party setup?
