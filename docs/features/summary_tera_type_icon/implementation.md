# Summary Tera Type Icon Implementation

## Branch State

| Field | Value |
|---|---|
| Branch | `feature/summary-tera-type-badge` / PR #26 |
| Baseline | `master` `c13184c0b1`; `git describe` = `expansion/1.15.2-45-gc13184c0b1` |
| Status | Validated branch; open draft PR #26; not on `master` |
| Date | 2026-05-15 |

## Source Changes

- `include/config/pokemon.h` enables Summary Tera type display by setting
  `P_SHOW_TERA_TYPE` to `GEN_9`.
- `include/config/summary_screen.h` defines `P_SUMMARY_TERA_TYPE_ICON_X` and
  `P_SUMMARY_TERA_TYPE_ICON_Y` for the Summary Info page Terastal badge
  anchor.
- `graphics/types/tera/*.png` contains the copied RavePossum / Zatsu 16x16 Tera
  type badge set.
- `graphics_file_rules.mk`, `src/graphics.c`, and `include/graphics.h` build and
  expose `graphics/types/tera/tera_types.4bpp` as the Summary Tera badge sprite
  sheet.
- `src/pokemon_summary_screen.c` uses those coordinates in `SetMonTypeIcons()`,
  draws the Terastal badge only, and hides it for eggs or when Tera display is
  disabled.
- `CREDITS.md` records RavePossum as the source branch / integration reference
  and Zatsu / `fakuzatsu` as the Tera type icon credit.

## Runtime Behavior

The Summary Info page still draws the normal species type icon path first. When
`P_SHOW_TERA_TYPE >= GEN_9`, the Tera display uses `summary->teraType` to select
the 16x16 Terastal badge from `graphics/types/tera/*.png`.
`P_SUMMARY_TERA_TYPE_ICON_X/Y` anchors the badge. The current default anchor is
`(205, 48)`.

The implementation is display-only. It does not change Tera type data, add Tera
type editing, or touch the Pokemon State Editor.

## Validation

- `rtk git diff --check`: passed.
- `rtk make -j16 -O all`: passed with the existing RWX LOAD segment linker
  warning.
- `rtk make -j16 -O debug`: passed with the existing RWX LOAD segment linker
  warning.
- `rtk make -j16 -O check`: passed with the existing RWX LOAD segment linker
  warning.
- mGBA Live focused route: booted the ROM, continued an existing save, used the
  debug menu `Cheat start`, opened Party -> Summary -> Info for Magearna, and
  confirmed the normal `STEEL` / `FAIRY` species type icons still render while
  only the 16x16 Tera badge appears to their right at the user-adjusted
  `(205, 48)` anchor. Screenshot:
  `/tmp/summary-tera-type-icon-mgba/summary-tera-type-icon-x205-info.png`.

The mGBA Live validation session `summary-tera-type-x205-check` stopped
cleanly.

## Remaining Checks

- A single-type Pokemon should still be checked manually after the badge-only
  revision.
- An egg Summary page should still be checked manually to confirm the Tera sprites
  do not retain stale data.
- GitHub Actions were not run or waited for this local branch.
