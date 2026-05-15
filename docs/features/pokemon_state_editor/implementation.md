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
Pokemon sprite area on the Skills page. The panel is anchored slightly lower so the
`POKEMON SKILLS` title remains visible, and the header/footer bands now share the
same configured height. The Skills-page prompt uses the existing top-right Summary
prompt area to avoid the ITEM/RIBBON label overlap.

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
| `include/config/summary_screen.h` | Added feature flag and layout defines for editor window, palette/fill, header/footer band height, slide step, row, text, value positions, and level edit/cap policy. |
| `src/pokemon_summary_screen.c` | Added Summary prompt, dedicated editor palette/panel rendering, right-pane page rendering, selected-row styling, row input, clamped EV/IV edits, level/nature/ability/ball edits, Dynamax/Tera/G-Max edits, gender/friendship edits, partial row redraws, right-edge slide-in/out, and Summary refresh on close. |
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

- Opening the editor copies window graphics once, then reveals the tilemap from the
  right edge over several frames.
- Page changes redraw the window graphics without re-putting the tilemap.
- Cursor moves redraw only the old row and new row.
- Value changes redraw only the selected row; EV edits also redraw the total line.
- No-op value input at min/max plays the failure sound when appropriate but does not
  redraw.

This keeps the existing immediate-apply behavior while avoiding repeated full-panel
VRAM copies during held input.

## Animation Policy

The editor uses the same tilemap-column idea as the Summary move-info panels:

- `START EDIT` draws the editor graphics offscreen in window memory, then reveals
  rightmost columns until the full right pane is visible.
- `B` restores the underlying Summary text and hides the editor columns back toward
  the right edge before returning to normal Summary input.
- `P_SUMMARY_STATE_EDITOR_SLIDE_STEP` controls the column speed. It is currently
  tuned to a short, move-info-like transition instead of a slow fade.

## Follow-Up Notes

- Text coordinates were nudged away from the left accent strip and band edges after
  the slide/layout pass. The page indicator now uses numbered header tabs instead
  of embedding `1/5` text in the body page label. If further clipping appears,
  prefer adjusting the `P_SUMMARY_STATE_EDITOR_*_Y`,
  `P_SUMMARY_STATE_EDITOR_TEXT_X`, and `P_SUMMARY_STATE_EDITOR_TAB_*` defines
  before changing draw code.
- State editor palette RGB values are exposed as
  `P_SUMMARY_STATE_EDITOR_COLOR_*` defines in `include/config/summary_screen.h`.
  The palette table in `src/pokemon_summary_screen.c` references those defines so
  later color tuning does not require hunting through rendering logic.
- A later Terastal Summary feature should display the current Tera type/status near
  the existing type UI, preferably to the right of the type labels with a small icon
  or compact overlay. Keep that as a separate branch from this editor implementation.
- If the editor expands into a full training hub, prioritize EV spread presets,
  slider/range-bar style EV allocation, direct numeric entry, Hyper Training toggles,
  Pokerus test controls, and PP Up / PP Max controls after move editing exists.
  These have existing Pokemon data hooks or clear ownership and remove the most
  repetitive setup work.
- Interaction feedback needs a later pass. The editor currently applies changes
  immediately and uses selection-style sounds for open/close/change. Consider a
  distinct value-change SE and a clearer `DONE` / `APPLY` presentation so B/close
  does not feel like the only confirmation affordance.

## Validation

Local commands passed:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

mGBA Live passed on sessions `pokemon-state-editor-runtime-fixed`,
`pokemon-state-editor-ui-check`, `pokemon-state-editor-ui-polish-2`,
`pokemon-state-editor-ui-redraw-2`, `pokemon-state-editor-slide-check`, and
`pokemon-state-editor-final-visual-check`:

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
- Slide follow-up pass confirmed the panel opens lower in the right pane, keeps the
  `POKEMON SKILLS` title visible, uses matching header/footer band heights, and
  returns to the Skills page through the B-button close path.
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
- Current slide validation screenshots:
  `/tmp/pokemon-state-editor-mgba/status-editor-slide-open.png`,
  `/tmp/pokemon-state-editor-mgba/status-editor-slide-close.png`.
- The slide validation session also closed visually, but `mgba-live-cli stop`
  returned `alive_after: true` while `ps` showed no matching PID for the reported
  process. Treat this as an mGBA Live stale session entry, not as a clean stop.
- Current tab/color validation screenshot:
  `/tmp/pokemon-state-editor-mgba/status-editor-final-tabs-color-open.png`.
- The tab/color validation confirmed numbered header tabs, a visible `EVs` page
  label above the first row, and a lighter green palette closer to the Skills page.
  Its `mgba-live-cli stop` cleanup also returned a stale alive entry while `ps`
  showed no matching PID.

GitHub Actions were not waited; local make and mGBA Live are the handoff evidence.

## Remaining Risks

- Box Summary is intentionally disabled until storage return paths are reviewed.
- Personality-derived presentation such as Spinda spots or Unown letter can change
  when gender changes personality low byte.
- Gigantamax factor is a raw toggle and is not locked to species that have a
  Gigantamax form.
- The UI is structured and readable, but not a custom Pokemon Champions-quality art
  pass.
- EV total redraw should be rechecked for stale glyphs when values shrink from
  multi-digit totals; the reported symptom is a hyphen/underscore-like artifact and
  occasional black flash on the total line until the editor is reopened.
