# Summary Tera Type Icon MVP Plan

## MVP

Display the Pokemon's `MON_DATA_TERA_TYPE` on the Summary Info page as a 16x16
Terastal badge.

## Implementation

| File | Change |
|---|---|
| `include/config/pokemon.h` | Set `P_SHOW_TERA_TYPE` to `GEN_9` so the existing Summary Tera display path is active by default. |
| `include/config/summary_screen.h` | Add `P_SUMMARY_TERA_TYPE_ICON_X` and `P_SUMMARY_TERA_TYPE_ICON_Y` for the Terastal badge anchor. |
| `graphics/types/tera/*.png` | Copy the credited RavePossum / Zatsu 16x16 Tera type badge set. |
| `graphics_file_rules.mk`, `src/graphics.c`, `include/graphics.h` | Build and expose the combined Tera badge sprite sheet. |
| `src/pokemon_summary_screen.c` | Draw the Terastal badge from `SetMonTypeIcons()`, then hide it for eggs / disabled Tera display. |
| `CREDITS.md` | Record RavePossum as the source branch / integration reference and Zatsu / `fakuzatsu` as the Tera type icon credit. |

## Current Contract

- On the Summary Info page, the normal species type icons render first.
- If Tera type display is enabled, the 16x16 Tera badge renders to their right.
- The current default badge anchor is `(205, 48)`.
- No scaling is attempted.
- Eggs show only the mystery type icon and hide the Tera badge slot.

## Non-Goals

- No State Editor changes.
- No Tera type value changes.
- No locally redrawn graphics assets in the MVP implementation.
- No custom scaling or affine sprite handling.
