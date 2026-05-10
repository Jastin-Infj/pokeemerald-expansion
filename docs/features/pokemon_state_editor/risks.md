# Pokemon State Editor Risks

## Runtime Risks

| Risk | Severity | Mitigation |
|---|---|---|
| Summary input conflict | Medium | Use `START` only on the Skills page and keep existing page navigation intact. |
| Summary cached data becomes stale | Medium | Refresh `currentMon` and extracted summary data after edits and before returning to input. |
| EV total overflow | Medium | Compute other-stat EV total before applying the selected stat value. |
| Invalid ability slot | Medium | Cycle only slots where `GetSpeciesAbility` is not `ABILITY_NONE`. |
| Level/EXP mismatch | Medium | Set target-level EXP together with `MON_DATA_LEVEL`, then recalculate stats. |
| Level cap bypass | Medium | Clamp the level editor to `GetCurrentLevelCap` when `P_SUMMARY_STATE_EDITOR_LEVEL_CAP` is true. |
| Forced Tera type | Low | Lock the row when species data defines a forced Tera type. |
| Invalid special Tera type | Low | Skip special-case types from `gTypesInfo`. |
| Gigantamax species legality | Low | Current MVP toggles the raw factor; species-specific legality is documented as follow-up. |
| Fixed/genderless species | Low | Display the current gender but reject incompatible changes. |
| Personality side effects | Medium | Only change the low byte for gender through `SetMonPersonality`; preserve visible nature with `MON_DATA_HIDDEN_NATURE` and shiny state with `MON_DATA_IS_SHINY`. |
| Personality-derived visuals/forms | Medium | Spinda spot layout, Unown letter, or similar personality-derived presentation can still change. Keep as a documented follow-up until species-specific policy is decided. |
| Box Pokemon corruption | Medium | Keep MVP party-only. |
| UI overlap on GBA window | Medium | Keep coordinates in defines, place the editor in the right Summary pane, use the Summary prompt area for `START EDIT`, and verify with mGBA screenshots. |
| Held-input redraw flicker | Medium | Avoid full window/tilemap copies on every input frame; redraw only changed rows and skip redraws when capped input does not change data. |
| EV total stale glyphs | Medium | Next pass should verify that shrinking multi-digit EV totals clears the previous glyphs and that the EV total line copy area does not flash or leave underscore/hyphen-like artifacts. |
| mGBA Live stale cleanup | Low | If `mgba_live_stop` reports an alive session but the PID no longer exists, record the stale session state in the test plan instead of treating cleanup as fully clean. |

## Design Risks

The user prefers a Pokemon Champions-like training editor. A first GBA overlay will be
more utilitarian than that target. The MVP should still honor the useful parts of that
direction: page structure, fast value changes, a non-debug entry point from Summary,
readable contrast, and avoiding both the plain white standard window look and a
single flat dark panel.

## Accepted Deferrals

- No move editor.
- No met location/origin editor.
- No custom art pass.
- No true right-edge slide-in/out transition animation for the editor panel yet.
- No final vertical anchor pass against the Skills page title/header/`NEXT LV.` bands.
- No PC box editor until storage Summary is reviewed.
- No species-specific block/warning for personality-derived visual side effects.
- No species-specific lock for Gigantamax factor.
