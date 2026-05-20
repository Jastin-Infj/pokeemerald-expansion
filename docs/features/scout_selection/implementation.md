# Scout Selection Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `e927b612b3`; implementation branch `feature/scout-selection-runtime-20260520` |
| Code status | Runtime MVP implemented on feature branch |
| Provenance | Local implementation and validation |

## Summary

Scout Selection is implemented as a new runtime module instead of extending
`starter_choose.c`. The MVP is a script-driven 12-candidate Pokemon selection
screen with 6 visible slots, scrolling, Summary preview, selected-order markers,
and gift-mon handoff.

The original hardcoded starter-like dummy pool has been replaced by a generated
demo pool sourced from the previously implemented `champions_partygen` catalog
set JSON. Generation keeps the first set for each species, skips later duplicate
species, and emits the first 12 unique candidates into an ignored runtime header.

Selection state is kept in static EWRAM, not heap allocation. Returning from the
custom screen through `CB2_ReturnToFieldContinueScript` resets the field heap
before the script resumes, so the selected generated Pokemon must survive that
transition until `GiveSelectedScoutMons` runs.

## Runtime Contract

Scripts set scratch vars, initialize the pool, open the UI, then give the chosen
Pokemon:

```asm
setvar VAR_0x8004, SCOUT_POOL_PARTYGEN_DEMO @ pool id
setvar VAR_0x8005, 12                      @ candidate count
setvar VAR_0x8006, 1                       @ pick count
special InitScoutSelection
goto_if_eq VAR_RESULT, FALSE, InitFailed
special OpenScoutSelection
waitstate
goto_if_eq VAR_RESULT, SCOUT_RESULT_CANCEL, Canceled
special GiveSelectedScoutMons
goto_if_eq VAR_RESULT, MON_CANT_GIVE, NoSpace
```

`OpenScoutSelection` returns `SCOUT_RESULT_SELECTED` only after the player
confirms the exact requested pick count. `GiveSelectedScoutMons` returns the
same party / PC result values used by scripted gift Pokemon.

## Files Changed

| File | Purpose |
|---|---|
| `src/scout_selection.c` | New CB2 screen, static EWRAM selection state, Summary transition, generated pool include, and gift handoff. |
| `include/scout_selection.h` | Public special prototypes. |
| `include/constants/scout_selection.h` | Script/C constants for pool id and scout UI result values. `SCOUT_POOL_STARTER_TEST` remains an alias for compatibility. |
| `data/specials.inc` | Registers `InitScoutSelection`, `OpenScoutSelection`, `GiveSelectedScoutMons`. |
| `data/event_scripts.s` | Includes scout constants for event scripts. |
| `data/scripts/debug.inc` | Adds a debug script route for the 12-candidate pool. |
| `src/debug.c` | Adds `Scripts... > Scout Selection` to the debug menu. |
| `Makefile` | Generates the Scout pool header from partygen catalog set JSON before ROM build. |
| `.gitignore` | Ignores `src/data/scout_selection_pools.h`, matching other generated runtime headers. |
| `tools/scout_selection/make_scout_pools.py` | Scout-specific JSON-to-C generator with species de-duplication and stat-order conversion. |
| `tools/champions_partygen/catalog/sets/hoenn_demo.json` | Demo partygen set input restored from the partygen shelf. |
| `tools/champions_partygen/catalog/sets/elite_four.json` | Larger Elite Four set input restored from the partygen shelf. |

## UI Behavior

| Input | Behavior |
|---|---|
| D-pad | Moves the cursor across the 2x3 visible grid. Moving above / below the visible window scrolls through the 12-candidate pool. |
| `A` | Selects or deselects the highlighted candidate. Selected candidates show a shaded cell and `Pick N` order marker. |
| `SELECT` | Opens standard Summary for the highlighted candidate. Returning with `B` restores the Scout UI state. |
| `START` | Confirms only when selected count equals pick count. |
| `B` | Cancels before confirm and gives no Pokemon. |

## Data And Gift Policy

- The MVP pool is deterministic generated C data sourced from partygen catalog
  JSON, not hand-authored dummy Pokemon.
- `tools/scout_selection/make_scout_pools.py` reads `hoenn_demo.json` first and
  `elite_four.json` second. The first set for a species wins; later sets with
  the same `species` are skipped. This is why `set.sidney.mightyena.disrupt` is
  not emitted after `set.mightyena.disrupt`.
- The generator emits `species`, `level`, `item`, `ball`, `nature`, `ability`,
  `moves`, `ivs`, and `evs`. partygen stat strings are HP / Atk / Def / SpA /
  SpD / Spe; generated runtime arrays are converted to MON_DATA order HP / Atk /
  Def / Spe / SpA / SpD.
- Candidates are generated as full `struct Pokemon` values before display, so
  Summary previews the exact Pokemon later given to the player.
- Candidate state intentionally survives Summary and field callback transitions
  until `GiveSelectedScoutMons` clears it.
- The current generated demo pool contains 12 level 50 candidates:
  Metang, Skarmory, Aggron, Mightyena, Wobbuffet, Geodude, Zigzagoon, Shiftry,
  Cacturne, Crawdaunt, Absol, and Sharpedo.
- `GiveSelectedScoutMons` preflights available party / PC slots before N-pick
  handoff to avoid partial grants.
- SaveBlock state is not added. One-time claim, NPC flags, and economy are still
  script-owned / future work.

## Validation

| Date | Check | Result | Notes |
|---|---|---|---|
| 2026-05-20 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning. Earlier parallel build collision hit unrelated `link_rfu_2.o`; clean rerun passed. |
| 2026-05-20 | `rtk make -j16 -O check` | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | mGBA Live `scout-selection-runtime-20260520d` | Pass | Booted debug ROM, opened `Scripts... > Scout Selection`, verified 12-candidate scroll, Summary open/return, selected Chikorita, confirmed, saw field message, and verified Chikorita in party. Screenshots exported under `/tmp/mgba-scout-selection-20260520/`. |
| 2026-05-20 | `rtk make generated` after partygen pool update | Pass | Regenerated ignored `src/data/scout_selection_pools.h` from partygen JSON. |
| 2026-05-20 | `rtk git diff --check` | Pass | No whitespace errors. |
| 2026-05-20 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning. |
| 2026-05-20 | `rtk make -j16 -O check` | Pass | Suite exits 0 with existing `EXPECTED_FAIL` / `KNOWN_FAILING` markers. |
| 2026-05-20 | `rtk mdbook build docs` | Pass | Existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, large search index. |
| 2026-05-20 | mGBA Live `scout-selection-partygen-20260520` | Pass | Booted `/tmp/mgba-scout-selection-20260520/scout-partygen.gba`, opened debug route, verified partygen-derived Metang / Skarmory screen, scrolled to Zigzagoon / Shiftry, opened Zigzagoon Summary, returned, selected Zigzagoon, confirmed gift, and verified Zigzagoon Lv.50 in party. Screenshots: `scout-partygen-open.png`, `scout-partygen-scroll.png`, `scout-partygen-summary.png`, `scout-partygen-confirm.png`, `scout-partygen-party.png`. |
| 2026-05-20 | mGBA cleanup | Pass | `mgba_live_stop` stopped session `scout-selection-partygen-20260520`; `rtk pgrep -af mgba` showed no remaining `mgba-qt` process. |
| 2026-05-20 | UI polish runtime validation | Pass | Reworked Scout screen with blue header/footer bars, white candidate cards, blue cursor border/stripe, card shadow, and green selected card fill. mGBA Live `scout-selection-ui-polish-20260520` verified open, selected state, scroll, Summary return, confirm, and clean stop. Screenshots: `scout-ui-polish-open.png`, `scout-ui-polish-selected.png`, `scout-ui-polish-scroll.png`, `scout-ui-polish-confirm.png`. |
| 2026-05-20 | UI polish local checks | Pass | `rtk git diff --check`, `rtk make -j16 -O debug`, `rtk make -j16 -O all`, `rtk make -j16 -O check`, and `rtk mdbook build docs` passed. Build warnings remain the existing RWX linker warning and mdBook's existing missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, and large search index warnings. |

## Remaining Work

- Add map/NPC authoring examples once the first runtime validation is complete.
- Add additional partygen-derived pools after deciding whether pool selection is
  file-order based, group-filter based, or manifest based.
- Add focused N-pick runtime evidence for pick count 2 / 3 before using this in
  a multi-pick facility.
- Direct skills-page Summary entry is deferred; MVP uses standard Summary entry.
