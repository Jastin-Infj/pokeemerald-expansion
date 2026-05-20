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

Selection state is kept in static EWRAM, not heap allocation. Returning from the
custom screen through `CB2_ReturnToFieldContinueScript` resets the field heap
before the script resumes, so the selected generated Pokemon must survive that
transition until `GiveSelectedScoutMons` runs.

## Runtime Contract

Scripts set scratch vars, initialize the pool, open the UI, then give the chosen
Pokemon:

```asm
setvar VAR_0x8004, SCOUT_POOL_STARTER_TEST @ pool id
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
| `src/scout_selection.c` | New CB2 screen, 12-candidate test pool, static EWRAM selection state, Summary transition, and gift handoff. |
| `include/scout_selection.h` | Public special prototypes. |
| `include/constants/scout_selection.h` | Script/C constants for pool id and scout UI result values. |
| `data/specials.inc` | Registers `InitScoutSelection`, `OpenScoutSelection`, `GiveSelectedScoutMons`. |
| `data/event_scripts.s` | Includes scout constants for event scripts. |
| `data/scripts/debug.inc` | Adds a debug script route for the 12-candidate pool. |
| `src/debug.c` | Adds `Scripts... > Scout Selection` to the debug menu. |

## UI Behavior

| Input | Behavior |
|---|---|
| D-pad | Moves the cursor across the 2x3 visible grid. Moving above / below the visible window scrolls through the 12-candidate pool. |
| `A` | Selects or deselects the highlighted candidate. Selected candidates show a shaded cell and `Pick N` order marker. |
| `SELECT` | Opens standard Summary for the highlighted candidate. Returning with `B` restores the Scout UI state. |
| `START` | Confirms only when selected count equals pick count. |
| `B` | Cancels before confirm and gives no Pokemon. |

## Data And Gift Policy

- The MVP pool is C-owned and deterministic.
- Candidates are generated as full `struct Pokemon` values before display, so
  Summary previews the exact Pokemon later given to the player.
- Candidate state intentionally survives Summary and field callback transitions
  until `GiveSelectedScoutMons` clears it.
- The first pool contains 12 level 15 candidates:
  Treecko, Torchic, Mudkip, Bulbasaur, Charmander, Squirtle, Chikorita,
  Cyndaquil, Totodile, Pikachu, Eevee, and Riolu.
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

## Remaining Work

- Add map/NPC authoring examples once the first runtime validation is complete.
- Add additional pools or generated pool data after table shape is accepted.
- Add focused N-pick runtime evidence for pick count 2 / 3 before using this in
  a multi-pick facility.
- Direct skills-page Summary entry is deferred; MVP uses standard Summary entry.
