# Party / Status UI Overhaul Implementation

## Status

| Field | Value |
|---|---|
| Last updated | 2026-05-29 |
| Branch | `integration/runtime-dev-20260529`; source shelf `feature/party-status-ui-overhaul-20260521` |
| Scope | Runtime party menu first slice, adopted after #47 and #48 |
| Master policy | Runtime integration branch only; do not merge source / graphics into docs-only `master` |

## Implemented Slice

2026-05-29 integration note: this slice is now adopted into
`integration/runtime-dev-20260529` after the item-policy stack (#47 Battle Item
Restore and #48 Held Item Catalog). The #54 patch applied cleanly on top of
#48, and the #48 held-item helper calls in `src/party_menu.c` are still present
after the 2 x 3 party layout migration.

The first runtime slice changes the single-party menu shape from the current
`1 + 5` arrangement to a `2 columns x 3 rows` grid.

Runtime changes:

- added `PARTY_LAYOUT_GRID_2X3` in `include/constants/party_menu.h`;
- resolved existing `PARTY_LAYOUT_SINGLE` requests to `PARTY_LAYOUT_GRID_2X3`
  inside `InitPartyMenu`, and made the `PARTY_LAYOUT_SINGLE` drawing path
  grid-compatible as a fallback. Normal party, item target, move tutor,
  choose-mon, choose-half, and single-battle party entry points pick up the grid
  without rewriting every caller;
- left `PARTY_LAYOUT_DOUBLE`, `PARTY_LAYOUT_MULTI`, and
  `PARTY_LAYOUT_MULTI_SHOWCASE` structurally unchanged;
- added six row-major `14 x 5` party slot windows in
  `src/data/party_menu.h`;
- added row-major sprite coordinates for icons, held items, status icons, and
  slot Pokeballs;
- added a compact `PARTY_BOX_GRID` info rect for nickname, level, gender, HP
  text, HP bar, and choose-half description text;
- changed the grid slot from the first rough `14 x 3` cut-down wide slot to a
  `14 x 5` equal-column framed slot. The tile-number pattern and coordinate
  shape follow the Emerald Extra party menu reference, while still using local
  source-only data rather than importing PNG / BIN assets;
- widened the grid nickname rect to `POKEMON_NAME_LENGTH * 6` pixels and made
  grid nickname font fitting use that rect width, so the full configured
  nickname length is reserved instead of tuning only for current sample names
  such as `Bastiodon`;
- shifted the grid nickname origin to the right of the 32 px icon footprint and
  moved gender to the lower metadata row. This avoids species-specific icon
  overlap, which was visible on wider icons such as Bastiodon in slot 6;
- added grid-specific Confirm / Cancel button window templates with baseBlocks
  after the slot 6 tile range. The old button templates reused `0x1C7` /
  `0x1D3`, which overlapped the larger `14 x 5` slot 6 window (`0x1C1..0x206`)
  and caused Cancel/button tile data to corrupt only the bottom-right slot;
- verified the in-battle single-party entry path uses the same grid resolver:
  `OpenPartyMenuInBattle` and `ChooseMonForInBattleItem` pass
  `GetPartyLayoutFromBattleType()`, single battles return `PARTY_LAYOUT_SINGLE`,
  and `InitPartyMenu` resolves that to `PARTY_LAYOUT_GRID_2X3`;
- normalized the right-column slot coordinates back onto the same three row
  baselines as the left column. The first equal-column pass inherited a
  staggered right column from the reference, which made slot 6 collide with the
  lower message area on this screen;
- moved grid action / item / mail menus through three UX passes. The first
  compact pass still moved the window by selected slot column and row, which
  made Summary / Switch / Item feel visually unstable. The second pass kept a
  fixed right-aligned panel as a stable fallback. The current visual-skin pass
  uses a bottom command bar for short grid menus, so Summary / Switch / Item /
  Cancel and Give / Take / Move / Cancel no longer cover party slots. Longer
  or wider action lists, including field-move entries that depend on the
  existing vertical-menu cursor path, still fall back to the fixed vertical
  panel instead of cramming text into the bar;
- made grid party icons static while moving the cursor. The old single-party
  behavior nudged unselected icons and bounced the selected icon, which read as
  visual noise in a dense `2 x 3` grid. Grid selection now relies on the slot
  palette and Pokeball state while keeping icon positions stable;
- changed grid Switch to avoid the legacy slide animation and the heavier
  full-menu refresh. The old animation was designed for the `1 + 5` layout, so
  left-column grid slots slid right across occupied right-column slots and could
  erase unrelated slot tilemaps. The first refresh-based workaround avoided that
  visual path but could leak orphaned menu windows after repeated switches. Grid
  Switch now swaps the two Pokemon and their existing slot sprites directly,
  redraws only the two affected slot windows, and returns to choose-mon input;
- added grid-specific cursor traversal in `src/party_menu.c`:
  - left / right moves within each row when the paired slot exists;
  - up / down moves by row and falls back to the occupied slot in that row;
  - empty slots are skipped;
  - Confirm and Cancel are preserved for choose-half flows;
- kept the existing Summary screen implementation and return callbacks.

The old single-layout cursor traversal was removed after `PARTY_LAYOUT_SINGLE`
became grid-compatible, leaving double / multi layouts as the only legacy
party-menu traversal path.

## Asset Decision

No external PNG / BIN assets were imported in this slice.

The branch uses source-level tile-number tables for the `14 x 5` grid slot,
based on the RavePossum Emerald Extra equal-column party menu reference. That
keeps the first slice source-only while recording the lineage in
`external_references.md`.

Remaining visual polish work:

- create or import dedicated `2 x 3` slot frames;
- create a party-grid background that no longer assumes the old `1 + 5`
  composition;
- tune four-digit HP / max-HP spacing after broader save coverage;
- decide whether BW-style Summary assets should be a separate branch.

## SaveBlock / Runtime Data

This implementation does not add SaveBlock fields, saved options, generated
data, or persistent state. The UI is always-on for requests that previously
used `PARTY_LAYOUT_SINGLE`.

## Known Risk

2026-05-29 integration validation:

- `rtk git diff --cached --check`: passed.
- `rtk make -j16 -O all`: passed with the existing RWX linker warning.
- `rtk make -j16 -O debug`: passed with the existing RWX linker warning.
- `rtk make -j16 -O check`: passed with the existing RWX linker warning and
  expected existing test markers.
- mGBA Live CLI smoke:
  - session `integration-party-ui-smoke`;
  - booted `pokeemerald.gba` through `mgba-qt` with `DISPLAY=:0`;
  - captured `/tmp/integration-party-ui-smoke.png`;
  - accepted `START` input;
  - captured `/tmp/integration-party-ui-after-start.png`;
  - `mgba-live-cli stop` returned `stopped: true`;
  - `mgba-live-cli status --all` returned `[]`.

This integration pass did not replay the older field party menu visual route.
Manual UI confirmation should re-open the Party menu after the next integration
push and check grid draw, command bar placement, Switch, Summary return, and
held-item Give / Take behavior with #48 active.

This is an operational grid layout, not the final polished art pass. The most
likely follow-up is visual spacing: long nicknames, four-digit HP, and
choose-half description text may need a dedicated slot asset family rather than
the current compact reuse of the wide slot tiles.

The field party menu, cursor movement, action menu, Summary return path, and
Cancel path were confirmed with mGBA Live on 2026-05-21 after the `14 x 5`
equal-column adjustment. A follow-up right-column alignment pass was added after
slot 6 review and confirmed with mGBA Live in `party-menu-slot6-review`.
The later action-window width pass was confirmed in
`party-menu-action-column-review`. The max nickname width pass was confirmed in
`party-menu-max-name-review` and with a RAM-only 12-character nickname check in
`party-menu-max12-name-review-2`. The icon / nickname separation pass was
confirmed in `party-menu-icon-name-separation-review`. A follow-up slot 6
baseBlock overlap fix was added after identifying Confirm / Cancel button tile
data collision with the larger grid slot window, and confirmed in
`party-menu-slot6-baseblock-review`; the user also confirmed this slot 6 fix on
2026-05-21. The in-battle single-party Pokemon menu was confirmed in
`party-menu-inbattle-grid-review` by entering a debug trainer battle, opening
the battle `POKéMON` menu, and exercising a RAM-only six-slot party draw.
The grid Switch refresh path and unified grid item submenu placement were
confirmed in `party-menu-grid-switch-refresh-review` with repeated 1<->3,
3<->2, and 2<->4 style swaps plus item submenu open / return. The follow-up
direct-swap path, which removes the full refresh leak risk and shrinks the grid
menus, was confirmed in `party-menu-grid-switch-direct-stress` after 22 scripted
switches plus action / item submenu entry. The later fixed-anchor command-panel
pass removed the slot-dependent menu jump and was confirmed in
`party-menu-fixed-panel-review`. The visual-skin command-bar pass was confirmed
in `party-menu-command-bar-review`: short action and item menus now open in the
bottom message band, left / right movement selects actions, B returns cleanly,
slot 6 stays visible while its action menu is open, and grid Pokemon icons no
longer bounce or shift while the cursor moves. After adding the field-move
fallback guard, the final normal-ROM command-bar smoke was repeated in
`party-menu-command-bar-final-review`.
Choose-half Confirm / Cancel is still a manual validation target because the
active save did not provide an immediate route to that facility flow.
