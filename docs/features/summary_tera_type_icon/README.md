# Summary Tera Type Icon

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-15 |
| Working branch | `feature/summary-tera-type-badge` / PR #26 |
| Baseline | `master` `c13184c0b1`; `git describe` = `expansion/1.15.2-45-gc13184c0b1` |
| Code status | Validated branch; open draft PR #26; not on `master` |
| Provenance | User request and local Summary UI source review |

## Goal

Show the Pokemon's current Tera type on the normal Summary Info page. This is a
display-only UI change: it does not edit Pokemon data, open the state editor, or add
new interaction.

## Current Decision

- Use the RavePossum BW Summary Screen Expansion 16x16 Tera type badge art,
  copied into `graphics/types/tera/`, as the Tera marker.
- Show only the 16x16 Terastal badge for the Tera type marker.
- Place the Tera display immediately to the right of the normal species type icons.
- Expose `x/y` placement through `P_SUMMARY_TERA_TYPE_ICON_X` and
  `P_SUMMARY_TERA_TYPE_ICON_Y`; this is the Terastal badge anchor. The current
  default anchor is `(205, 48)`.
- Do not add icon scaling in this slice.
- Draw the Tera display after the normal type icons are assigned.

## Scope

### In Scope

- Enable Summary Tera type display for the current Gen 9-style behavior.
- Add placement defines for the Summary Tera icon.
- Import credited RavePossum / Zatsu Tera type badges.
- Hide the Tera badge for eggs or when Tera display is disabled.
- Build and perform focused runtime validation on the Summary Info page.

### Out of Scope

- State Editor controls.
- Tera type editing.
- New locally redrawn icon artwork.
- Sprite scaling.
- Battle UI behavior changes.

## Docs

- [MVP Plan](mvp_plan.md)
- [Investigation](investigation.md)
- [Asset References](asset_references.md)
- [Implementation](implementation.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)

## Open Questions

- Should a later art pass redraw or replace the imported 16x16 badge set?
- If the icon source changes later, update `CREDITS.md` and
  `asset_references.md` before importing the replacement.
