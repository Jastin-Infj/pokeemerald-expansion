# XY status HUD: scope, runtime constraints, and acceptance

Status: reference reviewed; Aseprite design and ROM integration locally validated.
This document records the agreed scope and observed runtime contracts.

## Authorized outcome

Create the status-area design in real Aseprite, implement it in the ROM, validate
at a 120fps target with video synchronization, then commit and push the completed
feature branch. The user's latest instruction includes ROM implementation.

Keep opponent panels upper-left and player panels lower-right. Redesign the HP
label, fill strip, nickname/level/HP numbers, status badge, experience strip and
surrounding ornaments as a coherent XY-inspired status area. Keep current command
and message areas in the design board to check fit and color compatibility.

Reference selected by the user:
https://www.gameuidatabase.com/gameData.php?id=545

The web page returned an access check. The user then provided local reference
images in `D:/engram/20_projects/engram-mcp/resources/reference/incoming`.
Viewed `Pokmon-X-and-Y05292021-105437-53912.jpg`, `105438-64555.jpg`,
`105438-73127.jpg`, and `105438-93209.jpg` (the latter three share the same
`Pokmon-X-and-Y05292021-` filename prefix). These are reference-only and are not
redistributed in this branch. Pokemon X/Y and its interface are Nintendo/Game
Freak/The Pokemon Company material; new HUD pixels are drawn by the Lua generator.

Observed design: floating light nickname/level and HP numerals with dark shadows,
a narrow dark HP capsule with lime HP label, and a separate thin blue EXP strip.
The source has enemy upper-right/player lower-left; the user's selected GBA side
placement remains upper-left/lower-right. The user explicitly requested first
trying XY style, then inspecting incompatibilities, then balancing. Initial
boards are preserved as `graphics/battle_interface/xy_initial_*.png`.

The initial overlay exposed crowding between status and HP. Status capsules are
now 16px wide, preserving the existing three-tile copy with a transparent third
tile. The status-present path restores the HP prefix even after numbers/bar
mode toggling. Large opaque panels are removed, white text retains dark shadows,
and HP-label colors use a separate palette so party balls keep their old colors.
The final design boards use each Pokemon's actual `normal.pal`; the initial board
also records the palette mismatch that was found and corrected during review.

## Branch and preservation

- Work branch: `feature/xy-status-hud-20260908`.
- Starting revision: `036e535bfea3b89aa6c46bdf15bc007167da468a`, fetched
  `origin/master` at branch creation.
- Previous work remains on `codex/aseprite-gba-sprite-script-20260908` and the
  named stash `pre-xy-status-hud-runtime-20260908` (including untracked sources).
- New image assets and consuming runtime changes belong on this feature branch.
  This is not a docs/Lua-only master integration. No master merge is authorized.
- Publish only after the acceptance checklist below passes. Push the feature
  branch explicitly; its initial tracking configuration points to origin/master.

## Observed rendering contracts

Source paths below are relative to the repository root.

| Component | Current contract | Consequence for the design |
|---|---|---|
| Canvas | 240x160 | Inspect native-size output; enlarged previews use nearest-neighbor scaling. |
| Panel positions | `src/battle_interface.c`, `sBattlerHealthboxCoords`: single player (158,88), opponent (44,30); doubles players (159,76)/(171,101), opponents (44,19)/(32,44) | These are sprite anchors, not image top-left coordinates. Preserve side placement; derive visible bounds from sprite/subsprite layout. |
| Panel sheets | Singles player 128x64; singles opponent and doubles 128x32 | Sheets span two OAM sprites. Art beyond visible sections is not automatically available on screen. |
| Text | `UpdateNickInHealthbox` uses `GetFontIdToFit` with a 55px nickname region; level is right-aligned separately | Use actual glyph pixels and advance widths; long names need the same narrow-font fallback. |
| HP numbers | `HP_MAX_DIGITS` is 4; `PrintHpOnHealthbox` splits long strings across the two panel sprites | Required 3-digit examples must not regress existing 4-digit support. Keep the background clearing regions aligned with the design. |
| HP fill | `MoveBattleBarGraphically` copies six 8x8 tiles, total 48px | A different fill width requires renderer and calculation changes, not only a new PNG. |
| Experience | Same function copies eight 8x8 tiles, total 64px; MAX_LEVEL clears them | Design both partial experience and Lv100 empty experience states. |
| Status | `UpdateStatusIconInHealthbox` copies 3 tiles, total 24x8 | Badge art must remain readable at 24x8 or its allocation/copy logic must change together. |
| HP/status interaction | Opponent and double layouts clear the HP label when status is present | If the reference requires simultaneous HP text and status, allocate separate regions and modify the update/clear paths. A new HP texture alone will disappear. |
| Caught marker | `TryAddPokeballIconToHealthbox` clears the caught marker while status is shown | Include caught/status transitions in regression checks if their positions change. |
| Healthbox palette | `src/battle_gfx_sfx_util.c` loads `ball_status_bar.png` for `TAG_HEALTHBOX_PAL` | Palette changes also affect the party-summary bar and existing panel/text consumers. |
| HP palette | Same file loads `ball_display.png` for `TAG_HEALTHBAR_PAL` | HP-label colors share a bank with party-ball graphics and HP fill states. |
| Dynamic colors | Status path writes palette entry `12 + battler` | Healthbox palette entries 12..15 cannot be used as stable ornament colors. Test four simultaneous statuses. |
| Tile ordering | `src/graphics.c`, `gHealthboxElementsGfxTable`, concatenates HP/EXP/status/misc/animation/status2..4 sheets | Preserve tile counts and ordering unless all enum offsets and consumers are updated. All four status sheets are required. |

## Aseprite font adapter

`tools/xy_hud_font.lua` reads the actual `charmap.txt`, Latin small-font PNGs,
and `gFont*LatinGlyphWidths` arrays from `src/fonts.c`. It provides measurement,
small/narrow/narrower fit selection, and glyph rendering without resizing.
Current adapter scope is single-byte mapped text; it must reject unsupported
characters instead of substituting a guessed glyph. Gender/Lv special glyphs
need explicit mapping if used by the final board.

Real Aseprite batch checks passed for the width of `A`, a ten-character string,
`999/999`, selected-font fit, and the final draw cursor. Local evidence:
`C:/Users/jastin/AppData/Local/Temp/battle_ui_xy_variant_20260908/xy-font-check.png`
and `.aseprite`; test driver `/tmp/xy-hud-font-check.lua`.

## Execution and acceptance checklist

- [x] Preserve previous work and create a feature branch from fetched master.
- [x] Verify real game font import in Aseprite.
- [x] Inspect runtime layout, tile, palette and status-update contracts.
- [x] View the user-selected reference and record concrete visual decisions.
- [x] Produce layered editable single/double Aseprite boards, native PNGs and
  nearest-neighbor enlarged comparison images using real Pokemon assets.
- [x] Include long names, Lv100, three-digit HP, green/yellow/red/empty bars,
  status badges and experience states in the boards.
- [x] Export and implement the selected assets/layout, including all update and
  clearing paths affected by position changes.
- [x] Run normal ROM build (`rtk make -j16 -O all`) and debug build
  (`rtk make -j16 -O debug`); run focused checks if C logic changes.
- [x] At 120fps, inspect a single battle and a full 2v2 battle, all HP states,
  status apply/cure, long-name and level alignment, EXP updates, command/message
  transitions and sprite overlap. Record any memory-injected fixture separately
  from ordinary battle progression. Confirm MCP availability before choosing CLI.
- [x] Stop validation sessions and verify no stale session remains.
- [x] Record implementation, runtime evidence, build results and any accepted
  limitations. Do not wait for long GitHub Actions runs.
Publication gate: review the final diff, commit and push the feature branch,
then compare its remote hash with local HEAD. Do not merge into master.
Publication evidence is the final commit and remote branch, not the build result.
