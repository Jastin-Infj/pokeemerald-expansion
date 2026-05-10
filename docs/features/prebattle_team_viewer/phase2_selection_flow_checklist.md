# Phase 2 Integrated Selection Flow Checklist

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `feature/prebattle-team-viewer` |
| Code status | Future-work checklist; not implemented in the MVP |
| Provenance | User feedback and local implementation handoff notes |

## Purpose

The MVP intentionally uses a two-screen flow:

1. Team viewer shows both teams.
2. Existing choose-half selection screen selects 3 or 4 Pokemon.

The desired Phase 2 is an integrated selector: select 3 for singles or 4 for
doubles directly inside the team viewer while still inspecting both teams.

This is a larger flow change than the current viewer polish. Treat it as a
separate feature or integration branch unless the user explicitly asks to fold it
into the current implementation branch.

## Branch Policy

- Do not merge the implementation branch into `master` for a docs-only update.
- If only docs should land on `master`, create a docs-only branch from current
  `master` and cherry-pick or reapply only `docs/` and approved workflow files.
- Before a docs-only master PR or merge, run:
  `rtk git diff --name-only master..HEAD`.
- Any file outside `docs/` and `AGENTS.md` means the branch is not eligible for
  docs-only master merge.
- Keep the runtime implementation PR as evidence / staging until the user
  decides the source integration order.

## Impact Surface

| Area | Why it matters |
|---|---|
| `src/prebattle_team_viewer.c` | Owns the current viewer state machine, cursor movement, details panel, icon lifetime, and footer redraw. Integrated selection would add pick-order state here. |
| `src/trainer_battle_selection.c` | Currently owns choose-half selection and player party compression. Phase 2 may bypass or reuse parts of this module. |
| `src/party_menu.c` | Current MVP still enters the party menu for selection. Removing that step changes `B` return behavior and party menu hooks. |
| `src/battle_setup.c` | Routes trainer battles through viewer -> selection -> battle. Integrated flow changes the callback path into battle start. |
| `src/battle_main.c` | Opponent party cache must still match the final battle party exactly. |
| `src/battle_controller_player.c` | In-battle viewer should remain read-only and should not inherit pre-battle selection controls. |
| `include/config/battle.h` | Button assignments may need new config for select, unselect, details, confirm, and cancel. |
| UI graphics / palettes | Pick-order markers, colored labels, or overlays add tile / OBJ / palette pressure. |
| Debug route | `Party -> Team Viewer Battle` should exercise the integrated flow after the change. |
| Docs / test plan | Manual validation must cover selection order, cancel/back, and battle party restore. |

## Design Decisions To Make First

- Keep `A` as select / unselect, or use `A` for details and another button for
  selection.
- Decide the confirm button once the required count is reached: `START`, `A` on
  a footer command, or automatic confirm prompt.
- Decide whether `SELECT` continues to toggle the strength panel while selection
  markers stay visible.
- Decide how to display pick order:
  - replace slot number with red / orange `1`, `2`, `3`, `4`;
  - add a small colored badge near the icon;
  - use icon tint / frame color if palette budget allows it.
- Decide whether selecting an already-picked Pokemon removes it or changes order.
- Decide whether opponent-side cursor is allowed while selecting player Pokemon.
- Decide whether B from integrated flow returns to field, returns to viewer
  preview mode, or is disabled for trainerbattle script safety.
- Decide how doubles show 4 picks while singles show 3 without changing layout.

## Implementation Checklist

- Add explicit selection state to the viewer:
  - selected party slots,
  - selected order,
  - required count,
  - current mode: preview, selecting, details, confirming.
- Keep details mode persistent while moving the cursor, as implemented in the MVP.
- Keep opponent details public-only unless official Champions behavior confirms
  more information should be shown.
- Keep icon sprites alive during cursor movement; redraw only text / marker areas.
- Add selection markers without recreating all icons on every input.
- Disable confirm until exactly the required count is selected.
- Preserve battle selection restore semantics:
  - selected mons appear in battle order,
  - unselected mons remain in the party,
  - battle end restores original party order with selected mons updated.
- Preserve opponent cache semantics:
  - preview party is generated once,
  - battle consumes the same cached party,
  - trainer pool / override trainer behavior remains consistent.
- Keep in-battle viewer display-only:
  - no D-pad cursor movement,
  - no `SELECT` details,
  - no selection state mutation.
- Confirm all exit paths clean up windows, sprites, palettes, and callbacks.
- Keep unsupported battle types excluded unless the feature explicitly expands
  support.

## Validation Checklist

Minimum local checks:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

Focused mGBA route:

- Boot debug ROM.
- Use `Party -> Team Viewer Battle`.
- Select 3 from the integrated viewer in a single battle.
- Verify selected order in battle lead / party order.
- Return from battle and verify player party restore.
- Reopen pre-battle details with `SELECT`, move cursor, and confirm details remain
  open.
- Try selecting / unselecting the same slot.
- Try confirming with too few and too many selections.
- Try B / cancel behavior from preview, details, and selection modes.
- Open Bag during the later battle action menu and verify `TEAM INFO` hint
  returns correctly.
- Open in-battle viewer with `R` and verify it is still read-only.

Additional manual cases:

- Double battle path selects 4.
- Trainer Party Pool path shows the same generated opponent team in preview and
  battle.
- Override trainer path uses the effective trainer party.
- Player with only the required number of valid Pokemon still follows the
  intended skip / selection policy.
- Whiteout / loss restores party order and clears viewer state.

## Merge Handoff Notes

When Phase 2 is implemented, update:

- `docs/features/prebattle_team_viewer/implementation.md`
- `docs/features/prebattle_team_viewer/test_plan.md`
- `docs/features/prebattle_team_viewer/risks.md`
- this checklist with resolved decisions and new evidence.

If the user wants docs-only master intake before source integration, use a
docs-only branch from `master`; do not merge a branch containing `src/`,
`include/`, `data/`, `graphics/`, or generated runtime changes into `master`.
