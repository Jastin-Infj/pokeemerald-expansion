# Summary Tera Type Icon Test Plan

## Build / Lint

| Test | Command | Result | Notes |
|---|---|---|---|
| Diff lint | `rtk git diff --check` | Passed | 2026-05-15 docs/source diff. |
| Normal ROM | `rtk make -j16 -O all` | Passed | 2026-05-15; existing RWX LOAD segment linker warning. |
| Debug ROM | `rtk make -j16 -O debug` | Passed | 2026-05-15; existing RWX LOAD segment linker warning. |
| Full checks | `rtk make -j16 -O check` | Passed | 2026-05-15; existing RWX LOAD segment linker warning. |
| Docs book | `rtk mdbook build docs` | Passed with existing warnings | Existing missing `../CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large search index warning. |
| GitHub Actions | n/a | Not run | Local branch only; long Actions were not waited. |

## Focused Runtime Check

Use the debug ROM and mGBA Live:

1. Boot the debug ROM.
2. Reach a party Pokemon.
3. Open Party Menu -> Summary -> Info page.
4. Confirm normal species type icon(s) still render.
5. Confirm the Tera display appears immediately to the right.
6. Confirm the Tera display is the 16x16 Terastal badge only.
7. Confirm the display uses the configured `P_SUMMARY_TERA_TYPE_ICON_X/Y`
   placement.
8. Check an egg or egg-like summary path if available; the Tera sprites must not
   show stale data.
9. Stop mGBA Live and record cleanup state.

## Runtime Result

| Check | Result | Notes |
|---|---|---|
| Boot / route | Passed | Booted `pokeemerald.gba`, continued an existing save, used debug menu `Cheat start`, opened Party -> Summary -> Info. |
| Dual-type Pokemon | Passed | Magearna showed normal species `STEEL` / `FAIRY` type icons, then only the 16x16 Tera badge to their right. The removed 32x16 Tera type icon did not appear. |
| Coordinate placement | Passed | User-adjusted Tera badge anchor uses `P_SUMMARY_TERA_TYPE_ICON_X/Y` = `(205, 48)`. |
| Screenshot evidence | Passed | `/tmp/summary-tera-type-icon-mgba/summary-tera-type-icon-x205-info.png`. |
| Cleanup | Passed | mGBA Live session `summary-tera-type-x205-check` stopped cleanly. |
| Single-type Pokemon | Not run | Remaining manual check after badge-only revision. |
| Egg stale-icon path | Not run | Remaining manual check. |

## Manual Cases

| Case | Expected |
|---|---|
| Single-type Pokemon | Species type icon renders, Tera badge appears to its right. |
| Dual-type Pokemon | Both species type icons render, Tera badge appears to their right. |
| Tera type equals species type | Tera display still appears, because it describes a separate Tera value. |
| Egg | Tera badge slot is hidden. |
