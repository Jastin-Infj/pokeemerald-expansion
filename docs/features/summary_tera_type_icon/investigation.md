# Summary Tera Type Icon Investigation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `9376760f68`; `git describe` = `expansion/1.15.2-54-g9376760f68` |
| Code status | Docs-only source investigation; implementation shelf is PR #26 |
| Provenance | Local source read, asset references, and feature docs |

## Summary

Summary Tera Type Icon is implemented on `feature/summary-tera-type-badge` as
PR #26, not on `master`. Current `master` already has a disabled Gen 9 Summary
Tera type path, but it uses the normal type-icon sprite path and hard-coded
coordinates. The feature branch replaces that with a small credited Tera badge
and configurable placement.

## Source Findings

| Area | File / symbol | Finding |
|---|---|---|
| Config | `include/config/pokemon.h` `P_SHOW_TERA_TYPE` | Current `master` value is `GEN_8`, so `P_SHOW_TERA_TYPE >= GEN_9` is false. |
| Type icons | `src/pokemon_summary_screen.c` `SetMonTypeIcons` | Current logic draws species type icons at `(120,48)` / `(160,48)` and would draw Tera type at `(200,48)` when enabled. |
| Sprite setup | `CreateMoveTypeIcons`, `SetTypeSpritePosAndPal` | Summary type icons share `SPRITE_ARR_ID_TYPE` slots and use `gTypesInfo[type].palette`. |
| Egg path | `SetMonTypeIcons` | Eggs show `TYPE_MYSTERY` and hide the second type slot; a Tera badge implementation must also hide stale badge sprites. |
| Assets | `docs/features/summary_tera_type_icon/asset_references.md` | Adopted source is RavePossum BW Summary Screen Expansion icon set, credited to Zatsu / `fakuzatsu`. |

## Integration Notes

- Keep the Tera badge display-only.
- Do not add state editor controls in this slice.
- Draw normal species type icons first, then the Tera badge.
- Use configurable `x/y` defines in `include/config/summary_screen.h` on the
  implementation branch.
- If the imported asset source changes, update `asset_references.md` and
  `CREDITS.md` before importing replacement art.

## Open Questions

- Should single-type Pokemon and egg stale-icon checks be completed before PR #26
  is marked ready?
- Should the branch keep imported external PNGs, or wait for local replacement art?
- Should future passive Tera display share a layout contract with Pokemon State
  Editor's Tera editing page?

