# Pokemon State Editor Investigation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `9376760f68`; `git describe` = `expansion/1.15.2-54-g9376760f68` |
| Code status | Docs-only source investigation; implementation shelf is PR #23 |
| Provenance | Local source read and feature docs |

## Summary

Pokemon State Editor is implemented on `feature/pokemon-state-editor-expansion`
as PR #23, not on `master`. Current `master` has the Summary Screen hooks and
Pokemon data APIs needed to host the editor, but not the editor itself.

## Source Findings

| Area | File / symbol | Finding |
|---|---|---|
| Summary input | `src/pokemon_summary_screen.c` `Task_HandleInput` | Owns D-pad page/mon switching, `A`, `B`, `START`, `L/R`, and debug `SELECT`. Any editor entry must be page-gated. |
| Skills prompt | `ShowUtilityPrompt`, `ShouldShowIvEvPrompt` | Skills page already owns A-button IV/EV prompt when enabled. State editor must not overlap this prompt area. |
| Summary state | `sMonSummaryScreen`, `CopyMonToSummaryStruct`, `ExtractMonDataToSummaryStruct` | Editor must refresh cached Summary data after writes. |
| Config home | `include/config/summary_screen.h` | Best owner for editor enable, layout, palette, slide, level cap, and value-position defines. |
| Pokemon fields | `include/pokemon.h` `MON_DATA_*` | EV/IV/nature/ability/ball/Tera/Gmax/friendship/gender/level edits use existing Pokemon data fields. |
| Stat recalculation | `src/pokemon.c` `CalculateMonStats` | Stat-affecting edits require recalculation before returning to Summary. |

## Integration Notes

- `START EDIT` should be owned by the Skills page only.
- `START RELEARN` remains owned by Battle / Contest Move pages.
- Box Summary remains follow-up unless storage return path is explicitly validated.
- Gender edits touch personality-derived data; keep helper ownership and shiny /
  nature preservation documented in implementation handoff.
- Tera type editing and passive Tera display share Summary real estate but are
  separate features.

## Open Questions

- Should final adoption keep PR #23 non-draft, or convert it to a draft shelf until
  merge-state and box policy are rechecked?
- Should legality locks for G-Max / forced Tera species be tightened before merge?
- Should stale mGBA Live status entries from earlier UI validation be cleaned by
  tooling or just recorded as accepted cleanup risk?

