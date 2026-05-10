# Pokemon State Editor Implementation

## Summary

Branch: `feature/pokemon-state-editor-expansion`

The MVP adds a party Summary overlay editor on the Skills page. `START EDIT` opens
the editor, `A` changes editor pages, `B` closes it, D-pad changes values, and
`L`/`R` apply min/max or first/last valid values.

The visible label is `STATUS EDITOR`. The editor uses a dedicated Summary/interface-
style BG palette with a header band, body rows, selected-row highlight, and footer
control band instead of the standard white window frame or a single dark rectangle.
The editor window now occupies the right-side Summary pane so it does not cover the
Pokemon sprite area on the Skills page. The Skills-page prompt uses the existing
top-right Summary prompt area to avoid the ITEM/RIBBON label overlap.

Implemented pages:

| Page | Fields |
|---|---|
| EVs | HP, Atk, Def, SpA, SpD, Spe, plus total display |
| IVs | HP, Atk, Def, SpA, SpD, Spe |
| Core | Level, hidden nature, valid species ability slot, caught ball |
| Dynamax/Tera | Dynamax level, Tera type, Gigantamax factor |
| Gender/Friendship | Male/female when supported, and friendship |

Moves, origin/met data, box Summary, and custom art remain follow-up work.

## Code Changes

| File | Change |
|---|---|
| `include/config/summary_screen.h` | Added feature flag and layout defines for editor window, palette/fill, row, text, value positions, and level edit/cap policy. |
| `src/pokemon_summary_screen.c` | Added Summary prompt, dedicated editor palette/panel rendering, right-pane page rendering, selected-row styling, row input, clamped EV/IV edits, level/nature/ability/ball edits, Dynamax/Tera/G-Max edits, gender/friendship edits, partial row redraws, and Summary refresh on close. |
| `include/pokemon.h` | Added `SUBSTRUCT_TYPE_COUNT` and public personality helper prototypes. |
| `src/pokemon.c` | Added `SetMonPersonality` / `SetBoxMonPersonality` to change personality while preserving encrypted Pokemon substruct ordering and checksum integrity. |

## Data Policy

- EVs clamp per stat to `MAX_PER_STAT_EVS` and across all stats to `MAX_TOTAL_EVS`.
- IVs clamp to `MAX_PER_STAT_IVS`.
- Nature edits `MON_DATA_HIDDEN_NATURE`, so raw personality does not need to change
  for nature-only changes.
- Ability cycles only slots where `GetSpeciesAbility(species, slot) != ABILITY_NONE`.
- Level writes target-level EXP and `MON_DATA_LEVEL`, clamps to the configured level
  cap policy, and recalculates stats.
- Pokeball cycles normal caught-ball ids and displays the corresponding item name.
- Dynamax level clamps 0..`MAX_DYNAMAX_LEVEL` and recalculates stats.
- Tera type cycles editable non-special types and locks species with forced Tera type.
- Gigantamax factor toggles the raw on/off flag.
- Gender changes the personality low byte through `SetMonPersonality`, then restores
  shiny and hidden nature state via `MON_DATA_IS_SHINY` and `MON_DATA_HIDDEN_NATURE`.
- Fixed male, fixed female, and genderless species display as locked for gender edits.

## Redraw Policy

The flicker reported during repeated parameter changes came from redrawing and
copying the full editor window on every input frame. Holding a direction at the cap
could also keep requesting redraws even when the value no longer changed, making a
partially transferred panel visible for a frame.

The editor now separates initial/page rendering from value-row updates:

- Opening the editor does one full window/tilemap copy.
- Page changes redraw the window graphics without re-putting the tilemap.
- Cursor moves redraw only the old row and new row.
- Value changes redraw only the selected row; EV edits also redraw the total line.
- No-op value input at min/max plays the failure sound when appropriate but does not
  redraw.

This keeps the existing immediate-apply behavior while avoiding repeated full-panel
VRAM copies during held input.

## Validation

Local commands passed:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

mGBA Live passed on sessions `pokemon-state-editor-runtime-fixed`,
`pokemon-state-editor-ui-check`, `pokemon-state-editor-ui-polish-2`, and
`pokemon-state-editor-ui-redraw-2`:

- Booted debug ROM from a clean save path.
- Used truck debug access (`R+START`) and Utilities -> Cheat start.
- Opened Party -> Treecko Summary -> Skills page -> `START EDIT`.
- Earlier functional pass edited and verified HP EV `252`, HP IV `31`, hidden nature
  `24` (`Quirky`), ability slot `2`, gender `female`, and friendship `255`.
- UI follow-up pass confirmed the top-right `START EDIT` prompt no longer leaves
  ITEM/RIBBON text behind it.
- UI polish pass confirmed EVs, Core, and Dynamax/Tera pages render with readable
  light text, header/footer bands, row striping, and selected-row highlight.
- Redraw follow-up pass confirmed the editor opens on the right pane without covering
  Treecko's sprite area, continuous RIGHT held on HP EV reaches `252`, and repeated
  capped input does not expose a half-redrawn/blank editor panel.
- UI follow-up pass edited level `100`, ball `Great Ball`, Dynamax level `10`,
  Tera type `Electric`, and G-Max `On`; Lua readback reported `level=100`,
  `ball=2`, `dmax=10`, `tera=14`, and `gmax=1`.
- Closed the editor and confirmed Summary resumed with Treecko shown at level 100.
- Current redraw validation screenshots:
  `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-open.png`,
  `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-held-right-1.png`,
  `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-held-right-2.png`,
  `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-row-move.png`.
- The redraw validation session closed visually, but `mgba-live-cli stop` returned
  `alive_after: true` while `ps` showed no matching PID for the reported process.
  Treat this as an mGBA Live stale session entry, not as a clean stop.

GitHub Actions were not waited; local make and mGBA Live are the handoff evidence.

## Remaining Risks

- Box Summary is intentionally disabled until storage return paths are reviewed.
- Personality-derived presentation such as Spinda spots or Unown letter can change
  when gender changes personality low byte.
- Gigantamax factor is a raw toggle and is not locked to species that have a
  Gigantamax form.
- The UI is structured and readable, but not a custom Pokemon Champions-quality art
  pass.
- The next UI pass should replace the current abrupt/delayed-looking editor reveal
  with a true right-edge slide-in and matching slide-out on cancel.
- The right-pane layout is accepted for now, but the vertical anchor should be
  revisited so the panel sits below the Skills page title/header area and aligns
  more naturally with the existing Summary bands.
- EV total redraw should be rechecked for stale glyphs when values shrink from
  multi-digit totals; the reported symptom is a hyphen/underscore-like artifact and
  occasional black flash on the total line until the editor is reopened.
