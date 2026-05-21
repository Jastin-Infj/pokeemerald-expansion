# Party / Status UI Overhaul Implementation

## Status

| Field | Value |
|---|---|
| Last updated | 2026-05-21 |
| Branch | `feature/party-status-ui-overhaul-20260521` |
| Scope | Runtime party menu first slice |
| Master policy | Implementation branch only; do not merge source / graphics into docs-only `master` |

## Implemented Slice

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
- added six row-major `14 x 3` party slot windows in
  `src/data/party_menu.h`;
- added row-major sprite coordinates for icons, held items, status icons, and
  slot Pokeballs;
- added a compact `PARTY_BOX_GRID` info rect for nickname, level, gender, HP
  text, HP bar, and choose-half description text;
- changed the grid slot from the first rough `14 x 3` cut-down wide slot to a
  `14 x 5` equal-column framed slot. The tile-number pattern and coordinate
  shape follow the Emerald Extra party menu reference, while still using local
  source-only data rather than importing PNG / BIN assets;
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
- tune icon and HP text positions after mGBA screenshot review;
- decide whether BW-style Summary assets should be a separate branch.

## SaveBlock / Runtime Data

This implementation does not add SaveBlock fields, saved options, generated
data, or persistent state. The UI is always-on for requests that previously
used `PARTY_LAYOUT_SINGLE`.

## Known Risk

This is an operational grid layout, not the final polished art pass. The most
likely follow-up is visual spacing: long nicknames, four-digit HP, and
choose-half description text may need a dedicated slot asset family rather than
the current compact reuse of the wide slot tiles.

The field party menu, cursor movement, action menu, Summary return path, and
Cancel path were confirmed with mGBA Live on 2026-05-21 after the `14 x 5`
equal-column adjustment. Choose-half Confirm / Cancel is still a manual
validation target because the active save did not provide an immediate route to
that facility flow.
