# Summary Tera Type Icon Implementation

## Branch State

| Field | Value |
|---|---|
| Branch | `feature/summary-tera-type-badge` / completed shelf #26 |
| Baseline | `master` `c13184c0b1`; `git describe` = `expansion/1.15.2-45-gc13184c0b1` |
| Status | Validated branch; completed shelf #26 closed 2026-05-17 after CI success; adopted into `integration/runtime-dev-20260529` on 2026-05-30; not on `master` |
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

Integration note: on `integration/runtime-dev-20260529`, the badge is layered on
top of the already-staged State Editor and All Ability Summary changes. The
integration keeps the dedicated 16x16 Tera badge sprite separate from the normal
type icon sprite array so the Skills-page State Editor prompt remains unchanged.

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

Integration adoption on `integration/runtime-dev-20260529` passed:

- Applied source/graphics changes from `feature/summary-tera-type-badge` commit
  `bee3f54025`; the branch was not merged wholesale because it would rewind the
  newer runtime-dev docs/source stack.
- Conflict resolution retained `P_SUMMARY_SCREEN_ALL_ABILITY_SLOT_SWITCH`,
  `P_SUMMARY_STATE_EDITOR_*`, and the new `P_SUMMARY_TERA_TYPE_ICON_X/Y`
  defines together.
- `rtk make -j16 -O debug`: passed with the existing RWX linker warning.
- `rtk make -j16 -O all`: passed with the existing RWX linker warning.
- `rtk make -j16 -O check`: passed with the existing RWX linker warning.
- mGBA Live session `integration-summary-tera-smoke` reused an existing save,
  opened Party -> Wobbuffet Summary -> Info, confirmed the Tera badge appears
  to the right of the normal type icon area, then moved to Skills and confirmed
  the State Editor `START EDIT` prompt still renders.
- Integration screenshots:
  `/tmp/integration-summary-tera-info.png`,
  `/tmp/integration-summary-tera-skills.png`.
- Cleanup: `mgba_live_stop` returned `stopped: true`; `mgba-live-cli status --all`
  returned `[]`.

## Remaining Checks

- A single-type Pokemon should still be checked manually after the badge-only
  revision.
- An egg Summary page should still be checked manually to confirm the Tera sprites
  do not retain stale data.
- GitHub Actions were not run or waited for this local branch.
