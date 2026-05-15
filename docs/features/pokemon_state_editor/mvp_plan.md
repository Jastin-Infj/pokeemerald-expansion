# Pokemon State Editor MVP Plan

## Entry

Open the editor from the Summary Screen while viewing the Skills page:

- Party Menu -> Summary -> Skills page -> `START`
- Box Summary remains disabled for MVP unless runtime validation proves it safe.
- Eggs and bad eggs should reject editor entry.
- The Skills page prompt uses the existing top-right Summary prompt area so it does
  not overlap the ITEM/RIBBON label area.

## Layout Defines

Put these in `include/config/summary_screen.h`:

- `P_SUMMARY_SCREEN_STATE_EDITOR`
- `P_SUMMARY_STATE_EDITOR_WINDOW_LEFT`
- `P_SUMMARY_STATE_EDITOR_WINDOW_TOP`
- `P_SUMMARY_STATE_EDITOR_WINDOW_WIDTH`
- `P_SUMMARY_STATE_EDITOR_WINDOW_HEIGHT`
- `P_SUMMARY_STATE_EDITOR_WINDOW_PALETTE`
- `P_SUMMARY_STATE_EDITOR_WINDOW_FILL`
- `P_SUMMARY_STATE_EDITOR_TEXT_X`
- `P_SUMMARY_STATE_EDITOR_TEXT_Y`
- `P_SUMMARY_STATE_EDITOR_PAGE_Y`
- `P_SUMMARY_STATE_EDITOR_TAB_X`
- `P_SUMMARY_STATE_EDITOR_TAB_Y`
- `P_SUMMARY_STATE_EDITOR_TAB_WIDTH`
- `P_SUMMARY_STATE_EDITOR_TAB_GAP`
- `P_SUMMARY_STATE_EDITOR_ROW_Y`
- `P_SUMMARY_STATE_EDITOR_ROW_HEIGHT`
- `P_SUMMARY_STATE_EDITOR_VALUE_X`
- `P_SUMMARY_STATE_EDITOR_INFO_Y`
- `P_SUMMARY_STATE_EDITOR_CONTROLS_Y`
- `P_SUMMARY_STATE_EDITOR_BAND_HEIGHT`
- `P_SUMMARY_STATE_EDITOR_SLIDE_STEP`
- `P_SUMMARY_STATE_EDITOR_COLOR_*`
- `P_SUMMARY_STATE_EDITOR_LEVEL_EDIT`
- `P_SUMMARY_STATE_EDITOR_LEVEL_CAP`

The values should tune placement without touching editor logic.

## Pages

| Page | Rows | Notes |
|---|---|---|
| EVs | HP, Atk, Def, SpA, SpD, Spe | Shows total EVs and clamps quick max to remaining total. |
| IVs | HP, Atk, Def, SpA, SpD, Spe | 0..31, no total cap. |
| Core | Level, Nature, Ability, Ball | Level respects cap policy; nature cycles 25 natures; ability cycles valid slots; ball cycles normal ball ids. |
| Dynamax/Tera | Dmax Lv, Tera, G-Max | Dynamax level clamps to max; Tera skips special/forced types; G-Max toggles on/off. |
| Gender/Friendship | Gender, Friendship | Gender rejects fixed/genderless species; friendship 0..255. |

## Controls

| Input | Behavior |
|---|---|
| Up / Down | Move row cursor. |
| Left / Right | Decrement/increment value; use repeat input for smooth adjustment. |
| L | Set selected numeric row to min; first valid value for cyclic rows. |
| R | Set selected numeric row to max; last valid value for cyclic rows. |
| A | Next editor page. |
| B | Close editor and return to Summary. |

## Commit Model

Use immediate commit per edit, then refresh Summary's cached Pokemon data when closing.
This keeps the UX fast and removes a separate confirmation screen from the MVP.
User feedback after the MVP pass noted that this can make `B` feel like an
implicit commit rather than a clean close action, so a later UX pass should revisit
value-change sounds and a visible `DONE` / `APPLY` affordance.

After each edit:

- update the backing party Pokemon;
- copy the current Pokemon back into Summary's working struct;
- recalculate stats when EV/IV/nature/personality changes;
- redraw the editor overlay.

## Non-MVP

- Move slot editing.
- Met location/origin editing.
- Box Pokemon editing.
- Custom Champions-quality art assets.
