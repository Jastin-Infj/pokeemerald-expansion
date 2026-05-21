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

Not confirmed in this slice:

- choose-half Confirm / Cancel behavior; the active save did not provide an
  immediate route to a choose-half facility. This remains a manual validation
  target before treating battle selection UX as fully signed off.

## Accepted First-Slice Risk

The first implementation reuses existing party slot tilemaps instead of
importing new graphics. It should prove layout and interaction first. Dedicated
party grid assets are still expected for the final visual pass.
