# Phase 2 Integrated Selection Flow Checklist

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `feature/prebattle-team-viewer-phase2` |
| Code status | Implemented on Phase 2 branch; W / double debug route validated; pool cache path source-audited 2026-05-18 |
| Provenance | User feedback and local implementation handoff notes |

## Purpose

The original MVP used a two-screen flow: team viewer first, then the existing
choose-half selection screen. Phase 2 replaces that with an integrated selector:
select 3 for singles or 4 for doubles directly inside the team viewer while
still inspecting both teams.

This checklist now records the implemented Phase 2 decisions and the remaining
validation follow-ups.

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
| Debug route | `Party -> Team Viewer Battle` exercises single selection; `Party -> Team Viewer W` exercises 4-of-6 double selection. |
| Docs / test plan | Manual validation must cover selection order, cancel/back, and battle party restore. |

## Design Decisions To Make First

- Keep `A` as select / unselect, or use `A` for details and another button for
  selection.
- Decide the confirm button once the required count is reached: `START`, `A` on
  a footer command, or automatic confirm prompt.
- Player-side `SELECT` opens the existing Pokemon Summary skills/status page while
  selection markers stay visible after returning. Opponent-side `SELECT` keeps the
  public-only footer preview.
- Pick order display uses the existing slot label position: selected player slots
  replace the slot number with orange-brown `1`, `2`, `3`, or `4` on a compact
  low-contrast cream marker. Marker rectangle and text render offsets are named
  `#define`s in `src/prebattle_team_viewer.c` and are relative to the slot label
  origin for manual tuning. Text printer background / shadow stays transparent so
  marker fill and text rendering remain separate.
- Selecting an already-picked Pokemon removes it and compacts later pick order.
- Decide whether opponent-side cursor is allowed while selecting player Pokemon.
- Decide whether B from integrated flow returns to field, returns to viewer
  preview mode, or is disabled for trainerbattle script safety.
- Doubles use the same layout but require 4 picks; `Party -> Team Viewer W` covers
  the 4-of-6 runtime path.

## Implementation Checklist

- Add explicit selection state to the viewer:
  - selected party slots,
  - selected order,
  - required count,
  - current mode: preview, selecting, details, confirming.
- Implemented: viewer tracks selected slots and required count directly.
- Player-side `SELECT` opens the regular Summary skills/status page and returns to
  the integrated viewer.
- Opponent-side `SELECT` keeps the lightweight public-only detail footer.
- Keep opponent details public-only unless official Champions behavior confirms
  more information should be shown.
- Keep icon sprites alive during cursor movement; redraw only text / marker areas.
- Add selection markers without recreating all icons on every input.
- Disable confirm until exactly the required count is selected.
- Preserve battle selection restore semantics:
  - selected mons appear in battle order,
  - unselected mons remain in the party,
  - battle end restores original party order with selected mons updated.
- Implemented: confirmation writes `gSelectedOrderFromParty`, then the existing
  `TrainerBattleSelection_StartBattleFromSelection()` path starts the battle.
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
- Use `Party -> Team Viewer W`.
- Select 4 from the integrated viewer in a double battle.
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

Completed Phase 2 evidence:

- `prebattle-team-viewer-summary-marker`
- `/tmp/prebattle-team-viewer-summary-marker-start.png`
- `/tmp/prebattle-team-viewer-summary-skills-page.png`
- `/tmp/prebattle-team-viewer-summary-marker-slot6-first.png`
- `/tmp/prebattle-team-viewer-summary-marker-three-selected.png`
- `/tmp/prebattle-team-viewer-summary-marker-battle-start.png`
- `prebattle-team-viewer-marker-adjust`
- `/tmp/prebattle-team-viewer-marker-adjust-slot6-first.png`
- `/tmp/prebattle-team-viewer-marker-adjust-summary-skills.png`
- `prebattle-team-viewer-transparent-text`
- `/tmp/prebattle-team-viewer-transparent-text-slot6-first.png`
- `prebattle-team-viewer-summary-initial-layout`
- `/tmp/prebattle-team-viewer-summary-initial-layout-fixed.png`
- `prebattle-team-viewer-summary-info-layout-fixed`
- `/tmp/prebattle-team-viewer-summary-info-layout-skills-fixed.png`
- `/tmp/prebattle-team-viewer-summary-info-layout-info-fixed.png`
- `prebattle-team-viewer-w-debug3`
- `/tmp/prebattle-team-viewer-w-debug-route-start.png`
- `/tmp/prebattle-team-viewer-w-debug-four-selected.png`
- `/tmp/prebattle-team-viewer-w-debug-battle-intro.png`
- `/tmp/prebattle-team-viewer-w-debug-action-hint.png`
- `/tmp/prebattle-team-viewer-w-debug-inbattle-viewer.png`
- `/tmp/prebattle-team-viewer-w-debug-moveinfo-aligned-action-hint.png`

Additional manual cases:

- Double battle path selects 4 through `Party -> Team Viewer W`.
- Trainer Party Pool path source audit confirms preview generation and battle
  load use the same cached generated opponent team; add a concrete runtime
  assertion if this shelf is selected for adoption.
- Override trainer path uses the effective trainer party; add a concrete
  runtime assertion if this shelf is selected for adoption.
- Player with only the required number of valid Pokemon still follows the
  intended skip / selection policy.
- Whiteout / loss restores party order and clears viewer state.

## Merge Handoff Notes

Phase 2 implementation evidence has been written to:

- `docs/features/prebattle_team_viewer/implementation.md`
- `docs/features/prebattle_team_viewer/test_plan.md`
- `docs/features/prebattle_team_viewer/risks.md`
- `docs/features/prebattle_team_viewer/dependencies.md`
- `docs/manuals/prebattle_team_viewer_manual.md`

For master handoff, copy / cherry-pick docs only. Do not merge runtime source into
`master` as part of a docs-only update.

If the user wants docs-only master intake before source integration, use a
docs-only branch from `master`; do not merge a branch containing `src/`,
`include/`, `data/`, `graphics/`, or generated runtime changes into `master`.
