# Party / Status UI Overhaul Test Plan

## Status

| Field | Value |
|---|---|
| Last updated | 2026-05-21 |
| Branch | `feature/party-status-ui-overhaul-20260521` |
| Scope | `2 x 3` party menu first slice |

## Local Build Validation

| Check | Result | Notes |
|---|---|---|
| `rtk git diff --check` | Pass | No whitespace errors. |
| `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only. |
| `rtk make -j16 -O check` | Pass | Suite exits 0; existing RWX linker warning only. |
| `rtk mdbook build docs` | Pass | Existing warnings: missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, large search index. |

## mGBA Live / Manual Runtime Targets

Confirmed on 2026-05-21 with mGBA Live session `party-menu-equal-review`:

- booted the normal ROM from `pokeemerald.gba`;
- continued the local save;
- opened the field party menu from the overworld;
- confirmed six `14 x 5` framed slots draw in `2 / 2 / 2` order;
- moved cursor across rows and columns with `RIGHT`, `DOWN`, and `LEFT`;
- opened the action menu without corrupting the selected slot;
- opened Summary for Arceus from the party action menu;
- returned from Summary to the same party selection state;
- cancelled out of the party menu back to the overworld start menu;
- stopped the mGBA Live session; `mgba_live_status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session `party-menu-slot6-review`:

- confirmed slot 6 is aligned to the same bottom-row baseline as slot 5;
- confirmed the lower message frame no longer cuts into slot 6;
- moved the cursor to slot 6, opened the action menu, and confirmed the menu
  opens on the opposite side while slot 6 remains visible;
- exported evidence screenshot to `/tmp/party-menu-slot6-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session `party-menu-action-column-review`:

- selected slot 6, opened the action menu, and confirmed the window covers the
  opposite column cleanly without leaving partial party-slot fragments behind it;
- returned to the party menu, selected top-row slot 2, opened the action menu,
  and confirmed the window moves to the lower opposite column without cutting
  into the message frame;
- exported evidence screenshot to `/tmp/party-menu-action-column-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session `party-menu-max-name-review`:

- confirmed `Bastiodon` renders cleanly in the bottom-right slot after the
  nickname rect was widened to `POKEMON_NAME_LENGTH * 6`;
- exported evidence screenshot to `/tmp/party-menu-max-name-review.png`;

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-max12-name-review-2`:

- used a RAM-only Lua validation helper to set slot 6's nickname to a
  12-character string (`ABCDEFGHIJKL`) through the encrypted BoxPokemon
  nickname fields;
- confirmed the max-length nickname renders with the fitted font in the
  bottom-right slot without colliding with the Pokemon icon or gender symbol;
- exported evidence screenshot to `/tmp/party-menu-max12-name-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-icon-name-separation-review`:

- confirmed Bastiodon's icon no longer touches or obscures the first nickname
  glyph in slot 6 after shifting the grid nickname origin right of the 32 px
  icon footprint;
- confirmed gender moved to the lower metadata row does not collide with HP
  text;
- used the RAM-only 12-character nickname helper again and confirmed
  `ABCDEFGHIJKL` still fits in slot 6 after the icon / nickname separation pass;
- exported evidence screenshots to
  `/tmp/party-menu-icon-name-separation-review.png` and
  `/tmp/party-menu-icon-name-separation-max12-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-slot6-baseblock-review`:

- confirmed slot 6 no longer shows Cancel / Confirm button glyphs, stale text,
  or corrupted frame tiles behind the nickname / HP / Pokeball area;
- confirmed the field party Cancel button still draws correctly after moving
  grid button tile data to `0x207..0x21E`;
- used the RAM-only 12-character nickname helper again and confirmed slot 6
  remains clean with `ABCDEFGHIJKL`;
- exported evidence screenshots to `/tmp/party-menu-slot6-baseblock-review.png`
  and `/tmp/party-menu-slot6-baseblock-max12-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.
- user confirmed the slot 6 correction after this pass on 2026-05-21.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-inbattle-grid-review`:

- source route confirmed: `OpenPartyMenuInBattle()` and
  `ChooseMonForInBattleItem()` call `InitPartyMenu()` with
  `GetPartyLayoutFromBattleType()`, single battles return `PARTY_LAYOUT_SINGLE`,
  and `InitPartyMenu()` resolves that to `PARTY_LAYOUT_GRID_2X3`;
- booted the normal ROM, continued the local save, opened
  `Debug Menu > Party... > Start Debug Battle`, and reached the battle command
  menu;
- opened the battle `POKéMON` menu and confirmed the in-battle party screen uses
  the same `2 / 2 / 2` grid layout;
- copied the active debug-battle Pokemon into party slots 2-6 with a RAM-only Lua
  helper to exercise the full six-slot in-battle draw without changing source or
  save data;
- confirmed the six-slot battle party draw keeps the same grid slot sizing,
  cursor state, HP text placement, bottom message frame, and Cancel button
  placement;
- exported evidence screenshot to `/tmp/party-menu-inbattle-grid-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-grid-switch-refresh-review`:

- booted the rebuilt normal ROM and opened the field party menu from the local
  save;
- validated the grid Switch path after replacing the legacy slide animation with
  an in-place refresh / full slot sprite rebuild;
- performed the high-risk swap patterns around `1 <-> 3`, `3 <-> 2`, and
  `2 <-> 4`, which previously let left-column slots slide across occupied
  right-column tilemaps;
- confirmed the affected right-column slots, Pokemon icons, held-item/status
  sprites, HP text, slot frames, bottom message frame, and Cancel button stayed
  intact after each swap;
- opened the Item submenu after switching and confirmed it now shares the same
  grid opposite-column placement as Summary / Switch / Cancel instead of using
  the old fixed bottom-right submenu position;
- exported evidence screenshot to `/tmp/party-menu-grid-switch-refresh-review.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-grid-switch-direct-stress`:

- replaced the refresh-based grid Switch workaround with a direct two-slot swap
  that reuses existing slot sprites and redraws only the two affected windows;
- reduced the grid action / item / mail submenu footprint from the earlier full
  `14`-tile column window to a `12 x 8` minimum opposite-column window;
- ran a scripted 22-switch stress sequence through the field party menu,
  including `1 <-> 3`, `3 <-> 2`, and repeated `2 <-> 4` / `4 <-> 2` patterns;
- confirmed no top-left stale window tiles, nickname corruption, forced Summary
  entry, or input lock after the stress sequence;
- confirmed pressing A still opened the normal action menu and entering Item
  still opened the smaller grid-aligned item submenu;
- exported evidence screenshot to `/tmp/party-menu-grid-switch-direct-stress.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-fixed-panel-review`:

- grid action / item / mail menus now use a fixed right-aligned,
  bottom-anchored command-panel position instead of moving to the selected
  slot's opposite column;
- confirmed the action panel opens from the same anchor for slot 1, slot 2,
  slot 4, and slot 6;
- confirmed the Item submenu uses the same fixed anchor from slot 6;
- exported evidence screenshots to
  `/tmp/party-menu-fixed-panel-review-slot1-action.png`,
  `/tmp/party-menu-fixed-panel-review-slot2-action.png`,
  `/tmp/party-menu-fixed-panel-review-slot4-action.png`,
  `/tmp/party-menu-fixed-panel-review-slot6-action.png`, and
  `/tmp/party-menu-fixed-panel-review-slot6-item.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-command-bar-review`:

- replaced the fixed vertical panel for short grid action / item / mail menus
  with a bottom command bar that leaves all six party slots visible;
- confirmed slot 1 opens `SUMMARY / SWITCH / ITEM / CANCEL` in the bottom
  command bar;
- confirmed left / right input moves the command-bar cursor from Summary to
  Item and A opens the Item submenu;
- confirmed the Item submenu opens as `GIVE / TAKE / MOVE / CANCEL` in the same
  bottom command-bar position;
- confirmed B from the Item submenu returns to the main action bar, and B from
  the action bar returns to normal party selection;
- confirmed slot 6 opens the same bottom command bar without covering the
  bottom-right Pokemon slot;
- confirmed grid Pokemon icons stay position-stable while the cursor moves,
  with no selected-icon bounce or nonselected-icon side nudge;
- exported evidence screenshots to
  `/tmp/party-menu-command-bar-action-summary.png`,
  `/tmp/party-menu-command-bar-action-item-selected.png`,
  `/tmp/party-menu-command-bar-item-give.png`,
  `/tmp/party-menu-command-bar-back-to-action.png`,
  `/tmp/party-menu-command-bar-cancel-back.png`, and
  `/tmp/party-menu-command-bar-slot6-action.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Confirmed on 2026-05-21 with mGBA Live session
`party-menu-command-bar-final-review`:

- reran the normal-ROM command-bar smoke after adding the guard that keeps field
  moves on the vertical menu path;
- confirmed the main action bar opens in the bottom command bar;
- confirmed right / right / A opens the Item command bar;
- exported evidence screenshots to
  `/tmp/party-menu-command-bar-final-action.png` and
  `/tmp/party-menu-command-bar-final-item.png`;
- stopped the mGBA Live session; `mgba-live-cli status --all` returned `[]`.

Not confirmed in this slice:

- choose-half Confirm / Cancel behavior; the active save did not provide an
  immediate route to a choose-half facility. The grid-specific Confirm / Cancel
  baseBlocks are implemented, but that route remains a manual validation target
  before treating choose-half UX as fully signed off.

## Accepted First-Slice Risk

The first implementation reuses existing party slot tilemaps instead of
importing new graphics. It should prove layout and interaction first. Dedicated
party grid assets are still expected for the final visual pass.
