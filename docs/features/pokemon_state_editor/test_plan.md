# Pokemon State Editor Test Plan

## Local Builds

Run after implementation:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

If a focused Pokemon/stat test target exists during implementation, prefer adding it
or running it in addition to the full check build.

## mGBA Live Runtime

Required focused runtime path:

1. Boot the debug ROM with mGBA Live.
2. Reach or create a party Pokemon.
3. Open Party Menu -> Summary.
4. Navigate to the Skills page.
5. Confirm the top-right `START EDIT` prompt is clean and does not overlap the
   ITEM/RIBBON labels.
6. Press `START` to open the Status Editor.
7. Confirm the editor panel opens in the right-side Summary pane and does not cover
   the Pokemon sprite area on the left.
8. Confirm the editor panel renders without the standard white window frame or a
   single flat dark rectangle: header/footer bands, body rows, selected-row
   highlight, and readable light text should be visible.
9. Hold RIGHT on an EV row until it reaches the cap. The value and total should
   update immediately, but the panel should not flash, blank, or show a partial
   full-window redraw after the cap is reached.
10. Move the cursor across rows and confirm only the row highlight changes; the
    rest of the editor should remain stable.
11. Change at least one EV with quick max, one IV, level, ball, nature, ability,
   Dynamax level, Tera type, Gigantamax factor, friendship, and
   gender if the selected species supports both male and female.
12. Close the editor and confirm Summary returns without a softlock.

## Manual Cases

| Case | Expected result |
|---|---|
| EV quick max | Selected EV reaches `MAX_PER_STAT_EVS` unless total EV cap leaves less room. |
| EV total cap | Total EVs never exceed `MAX_TOTAL_EVS`. |
| IV max | Selected IV reaches `MAX_PER_STAT_IVS`. |
| Level max | Selected level reaches the current level cap, or 100 when no cap is active. |
| Level lock config | If `P_SUMMARY_STATE_EDITOR_LEVEL_EDIT` is false, level displays locked and does not change. |
| Nature cycle | Summary nature/stat coloring reflects the hidden-nature edit. |
| Ability cycle | Ability name changes only among valid species ability slots. |
| Pokeball cycle | Ball name changes among displayable ball ids. |
| Dynamax max | Dynamax level reaches `MAX_DYNAMAX_LEVEL` and HP/stat display remains valid. |
| Tera cycle | Tera type changes among editable non-special types. |
| Forced Tera species | Forced Tera species display locked and do not apply invalid changes. |
| Gigantamax toggle | G-Max row toggles Off/On. |
| Friendship min/max | Value clamps to 0 and `MAX_FRIENDSHIP`. |
| Gender fixed species | Fixed male, fixed female, and genderless species do not apply invalid changes. |
| Gender variable species | Male/female species can switch both directions. |
| Right-pane layout | Editor panel occupies the right Summary pane and does not cover the Pokemon sprite. |
| Held input redraw | Holding D-pad at an already capped value does not visibly flash or redraw the whole panel. |
| Cursor redraw | Moving selection redraws the old/new row only, without blanking unrelated rows. |
| EV total shrink redraw | Raising an EV to a multi-digit value and lowering it again clears all old digits without stray hyphen/underscore glyphs or black total-line flashes. |
| Future slide animation | When implemented, the editor should enter from the right edge and return to the right edge on cancel, with a faster move-info-like timing rather than a fade/delayed draw. |
| Future vertical anchor | When adjusted, the panel should align below the `START EDIT`/green header area and avoid covering the `POKEMON SKILLS` title area. |
| Close editor | B returns to Summary input; B again exits Summary normally. |

## Handoff Fields

Final results for `feature/pokemon-state-editor-expansion`:

- `rtk git diff --check`: passed
- `rtk make -j16 -O all`: passed after UI polish
- `rtk make -j16 -O debug`: passed after UI polish
- `rtk make -j16 -O check`: passed after UI redraw follow-up
  - First redraw-follow-up attempt failed before test execution with stale
    `/tmp/mgba-rom-test-hydra-*` files causing `open tmpfd failed: File exists`.
    Removed those stale temp ROMs and reran `rtk make -j16 -O check`; the rerun
    passed.
- mGBA Live: passed
  - Session: `pokemon-state-editor-runtime-fixed`
  - Route: clean boot -> truck -> debug menu with `R+START` -> Utilities -> Cheat start -> Party -> Treecko Summary -> Skills page -> `START EDIT`.
  - Confirmed overlay pages rendered and accepted input.
  - Confirmed direct party data after edits with Lua: HP EV `252`, HP IV `31`, hidden nature `24` (`Quirky`), ability slot `2`, gender `female`, friendship `255`.
  - Confirmed closing the editor returned to Summary without corruption or softlock.
  - Screenshot evidence: `/tmp/pokemon-state-editor-mgba/final-summary-after-editor.png`.
  - Cleanup: `mgba_live_stop` returned `stopped: true`; `mgba-live-cli status --all` returned `[]`.
- mGBA Live UI follow-up: passed
  - Session: `pokemon-state-editor-ui-check`
  - Route: clean boot -> truck -> debug menu with `R+START` -> Utilities -> Cheat start -> Party -> Treecko Summary -> Skills page -> `START EDIT`.
  - Confirmed `START EDIT` moved to the top-right prompt area and no longer leaves ITEM/RIBBON text behind it.
  - Confirmed the Status Editor panel uses a darker Summary-palette fill rather than the standard white window frame.
  - Edited level `100`, ball `Great Ball`, Dynamax level `10`, Tera type `Electric`, and G-Max `On`.
  - Confirmed direct party data after edits with Lua: `level=100`, `ball=2`, `dmax=10`, `tera=14`, `gmax=1`.
  - Confirmed closing the editor returned to Summary and Treecko displayed as level 100.
  - Screenshot evidence: `/tmp/pokemon-state-editor-mgba/status-editor-core-page.png`, `/tmp/pokemon-state-editor-mgba/status-editor-gimmick-page.png`, `/tmp/pokemon-state-editor-mgba/status-editor-summary-after-fields.png`.
  - Cleanup: `mgba_live_stop` returned `stopped: true`; `mgba-live-cli status --all` returned `[]`.
- mGBA Live UI polish follow-up: passed
  - Session: `pokemon-state-editor-ui-polish-2`
  - Route: clean boot -> truck -> debug menu with `R+START` -> Utilities -> Cheat start -> Party -> Treecko Summary -> Skills page -> `START EDIT`.
  - Confirmed the prompt remains in the top-right Summary prompt area.
  - Confirmed the editor renders as a menu panel with dedicated palette, yellow title, readable light row text, body row striping, selected-row highlight, and footer command band.
  - Confirmed EVs, Core, and Dynamax/Tera pages fit without obvious overlap.
  - Screenshot evidence: `/tmp/pokemon-state-editor-mgba/status-editor-ui-polish-evs.png`, `/tmp/pokemon-state-editor-mgba/status-editor-ui-polish-core.png`, `/tmp/pokemon-state-editor-mgba/status-editor-ui-polish-gimmick.png`.
  - Cleanup: `mgba_live_stop` returned `stopped: true`; `mgba-live-cli status --all` returned `[]`.
- mGBA Live UI redraw/layout follow-up: passed with stale cleanup note
  - Session: `pokemon-state-editor-ui-redraw-2`
  - Route: boot debug ROM -> New Game -> truck -> debug menu with `R+START` -> Utilities -> Cheat start -> Party -> Treecko Summary -> Skills page -> `START EDIT`.
  - Confirmed the editor renders in the right-side Summary pane and does not cover the Treecko sprite.
  - Held RIGHT on HP EV until it reached `252`; captured two capped-input screenshots while RIGHT was still held and did not observe the previous half-redrawn/blank panel.
  - Moved the cursor down to SpA and confirmed the row highlight changed without blanking unrelated rows.
  - Screenshot evidence: `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-open.png`, `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-held-right-1.png`, `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-held-right-2.png`, `/tmp/pokemon-state-editor-mgba/status-editor-redraw-partial-row-move.png`.
  - Cleanup: `mgba_live_stop` returned `stopped: false`, `alive_after: true`; `ps -p 79933` showed no matching PID. `mgba-live-cli status --all` still reported a stale `pokemon-state-editor-ui-redraw-2` entry.
- GitHub Actions: not waited unless explicitly requested
