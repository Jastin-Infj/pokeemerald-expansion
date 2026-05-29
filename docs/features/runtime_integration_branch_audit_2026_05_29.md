# Runtime Integration Branch Audit 2026-05-29

## Purpose

This audit is the staging map for a future runtime integration branch.
`master` remains the upstream intake baseline plus docs / Lua overlay; runtime
source is not merged directly into `master`.

The proposed integration branch is:

```text
integration/runtime-dev-20260529
```

Use it as a development lane for conflict resolution, build validation, mGBA
Live checks, and cross-feature review. Keep feature PRs and old branches as
evidence shelves until their code is actually re-applied or merged into the
integration lane.

## Rules

- Do not merge implementation branches into `master`.
- Do not treat an open PR as the full runtime queue. Some implemented features
  only exist as closed PRs, preserved branches, or branch-only shelves.
- Before adopting any branch, inspect `master...branch`, not just the PR page.
- For PR-backed work, use PR review commands after creating or updating the
  integration PR. For branch-only shelves, record local review notes in this
  document or the owning feature docs.
- For each adopted feature, validate locally before push:
  `rtk make -j16 -O all`, `rtk make -j16 -O debug` when debug routes are
  touched, and `rtk make -j16 -O check` or focused tests for battle / Pokemon /
  item / save / script logic.

## Open PR Triage

| PR | Branch | Current role | Recommended action |
|---|---|---|---|
| #47 | `feature/battle-item-restore-current-master-20260519` | Runtime shelf for berry-inclusive battle-end held item restore. | Keep open as a source shelf. First or near-first runtime-dev adoption candidate. |
| #48 | `feature/held-item-catalog-current-master-20260519` | Runtime shelf for held-item ownership token / catalog assignment. | Keep open. Adopt after #47 or with the item-policy stack. |
| #51 | `feature/scout-selection-runtime-20260520` | Runtime shelf for Pokemon Champions-style scout selection. | Keep open. Adopt after the party / item baseline is stable enough to test gifts and Summary return. |
| #54 | `feature/party-status-ui-overhaul-20260521` | Runtime shelf for 2x3 party menu layout. | Keep open. Adopt early enough to set the `party_menu.c` layout baseline before vendor / ability / Champions work piles on. |
| #57 | `feature/global-no-evolution-20260523` | Runtime shelf for Friendly Shop Pokemon Vendor plus global no-evolution policy. | Keep open. Adopt after #48 and #54 because it touches Bag / Storage / Summary / party UI. |
| #60 | `feature/all-ability-slots-runtime-20260523` | Runtime shelf for battle-only all-active ability slots. | Keep open. Adopt after the item and party-menu stack because it has battle-core and Summary / party overlap. |
| #62 | `feature/champions-run-session-runtime-20260524` | Runtime shelf for Champions run-session save / restore MVP. | Keep open. Adopt late, after roster, item, party, and battle policy branches are settled. |
| #64 | `docs/map-asset-relinker-20260525` | Superseded docs-only map relinker plan PR. | Close as superseded by #66, which already merged the master docs handoff. |
| #65 | `feature/map-asset-relinker-20260525` | Tool implementation lane for map relinker GUI / Rust core. | Keep separate from runtime-dev. This is a tooling lane, not a ROM runtime adoption lane. |

## Runtime Branches Without An Active PR

These branches must be checked alongside open PRs. Some are more important than
the PR list because they hold validated work that was closed or never opened as
a current PR.

| Branch | PR status | Runtime status | Recommended action |
|---|---|---|---|
| `feature/battle-selection-mvp` | No active PR; docs PR #18 exists. | Implemented trainer battle party selection MVP. | Consider as a supporting shelf if the integration does not take Team Viewer wholesale. It overlaps `src/battle_setup.c`, `src/party_menu.c`, and `include/config/battle.h`. |
| `feature/field-move-modernization-mvp` | No active PR. | Implemented HM-free field move MVP. | Adoption candidate if the next integration lane includes HM / field policy. Otherwise keep as a validated shelf. |
| `feature/field-move-toolkit-item` | No active PR. | Implemented Field Kit itemization on top of field move modernization. | Adopt only with the field-move lane; includes graphics and item data, so it is not docs-only. |
| `feature/prebattle-team-viewer` | Closed PR #20. | Implemented pre-battle / in-battle team viewer MVP. | Completed shelf. Consider after party-selection policy is chosen. |
| `feature/prebattle-team-viewer-phase2` | No active runtime PR; docs PR #22 exists. | Implemented phase 2 selection flow on top of Team Viewer. | Prefer this over the older viewer branch if adopting the integrated viewer path. Recheck against Scout Selection and Party UI. |
| `feature/pokemon-state-editor-expansion` | Closed PR #23. | Implemented Summary-launched Pokemon State Editor MVP. | Completed shelf. Integration depends on Summary / Party UI and vendor edit-entitlement decisions. |
| `feature/summary-tera-type-badge` | Closed PR #26. | Implemented display-only Summary Tera icon. | Small UI shelf; adopt only after asset credit and Summary layout ownership are settled. |
| `feature/unified-move-relearner` | Closed PR #28. | Implemented unified level / egg / TM / tutor / special move candidate list. | Completed shelf. Prefer after TM Shop Migration; teach / overwrite runtime proof remains recommended before final adoption. |
| `feature/tm-shop-migration` | Closed PR #31. | Implemented TM/HM acquisition retirement and reusable TM config. | Clean adoption shelf, but not a missing implementation. Prefer before Unified Move Relearner if the TM/HM lane resumes. |
| `feature/no-random-encounters-step-only-runtime-20260517` | Closed PR #41. | Implemented and user-confirmed step-only random encounter suppression. | Completed shelf. Re-apply only if runtime-dev wants the no-random flag now. |
| `feature/battle-bgm-selector-mvp-20260517` | Closed PR #39. | Implemented battle BGM selector plus large imported audio set. | Separate audio lane. Do not mix into the first runtime-dev pass unless audio becomes the selected focus. |
| `feature/trainer-battle-aftercare-heal` | Closed PR #10. | Implemented default-off trainer battle win heal hook. | Completed shelf needing focused exclusion tests before adoption. |
| `feature/trainer-partygen-catalog-expansion` | Closed PR #7. | Implemented partygen CLI / catalog / Elite Four and Wallace trainer pool data. | Tool/data shelf for Champions integration. Review generated data before adopting into runtime-dev. |
| `feature/champions-partygen-next-slice` | No active PR. | Docs-only follow-up for partygen / battle selection ordering. | Reference only unless docs need to be re-synced. |

## Legacy Or Reference Branches

These branches may still contain useful code or context, but they should not be
merged wholesale into runtime-dev without a separate restart plan.

| Branch | Notes |
|---|---|
| `feature/EX/ex-rz-upstream1` | Large randomizer branch with item ball icons, trainer rank parties, randomizer config, graphics, generated headers, tools, and tests. Treat as a separate randomizer lane. |
| `feature/ex-rz-upstream1` | Older randomizer branch. Superseded by the broader `feature/EX/ex-rz-upstream1` work for most purposes. |
| `feature/new-map` | Older map / region map / Fly experiment. Reference for map relinker and Fly icon behavior, not a clean runtime adoption branch. |
| `feature/new-map-test-v15` | Rouge Cave map draft from closed PR #4. It had CI / draft risk. Restart as a fresh map branch if needed. |
| `feature/qol_field_moves`, `feature/modern-qol-field-moves`, `feature/field-surfboard` | Older field move experiments. Prefer `feature/field-move-modernization-mvp` and `feature/field-move-toolkit-item` as the current field-move shelves. |
| `feature/party-select-ui` | Older party selection UI experiment. Prefer `feature/battle-selection-mvp` or Team Viewer phase 2. |
| `feature/docs-first-next-work-20260515` | Early Summary Tera docs / graphics branch. Prefer `feature/summary-tera-type-badge`. |
| `feature/no-random-encounters`, `feature/no-random-encounters-step-only` | Earlier no-random branches. Prefer `feature/no-random-encounters-step-only-runtime-20260517`. |

## Zero-Diff Or Already-Absorbed Branches

These currently have no meaningful `master...branch` diff or are already
absorbed by docs-only master work:

- `feature/bag-expansion`
- `feature/summary-first-relearner-runtime-20260522`
- `feature/battleMain`
- `feature/birch_case`
- `feature/dynamicmulti`
- `feature/main_menu`
- `feature/move_relearner`
- `feature/poryscript`
- `feature/releaseSystem`
- `feature/sandbox_v12`

Keep them for history unless the user explicitly asks to prune stale branches.

## Conflict Hotspots

These are the files most likely to define the integration order.

| File | Branches / PRs touching it | Integration risk |
|---|---|---|
| `src/party_menu.c` | #48, #54, #57, #60, #62, plus battle selection / Team Viewer shelves | Highest risk. Decide the Party UI baseline before broad adoption. |
| `src/battle_util.c` | #47, #60, #62 | Battle-end restore, all ability slots, and Champions outcome/restore paths overlap. |
| `include/config/battle.h` | #47, #60, battle selection / Team Viewer | Config naming and defaults need one owner per rule. |
| `data/scripts/debug.inc` | #51, #57, #62, #65, field/tool branches | Debug menu conflicts are easy but noisy; rename debug commands before adopting. |
| `data/specials.inc` | #51, #57, #62 | Special registration ordering and duplicate names must be checked after every adoption. |
| `data/event_scripts.s` | #51, #57, #65, map / field branches | Script include order can produce hidden build or runtime issues. |
| `src/pokemon_summary_screen.c` | #57, #60, State Editor, Summary Tera | Summary UI ownership must be settled before combining editor, vendor, ability selector, and Tera badge. |
| `src/pokemon_storage_system.c` | #48, #57 | Held item token policy and sealed recruit PC display overlap. |
| `src/battle_setup.c` | #57, #62, battle selection / Team Viewer | Trainer battle rewards, party selection, and Champions loss policy overlap. |
| `src/battle_script_commands.c` | #60, #62 | Ability slot behavior and Champions battle outcome hooks both touch battle script logic. |

## Proposed Runtime-Dev Order

This is a starting point, not a merge command list.

1. Create `integration/runtime-dev-20260529` from current `master`.
2. Add this audit doc and keep `master` docs-only.
3. Adopt #47 Battle Item Restore first if the item policy stack is selected.
4. Adopt #48 Held Item Catalog after #47.
5. Adopt #54 Party / Status UI before branches that depend on party layout.
6. Adopt #51 Scout Selection after the party baseline can be tested.
7. Adopt #57 Friendly Shop Pokemon Vendor after held-item and party UI policy.
8. Adopt #60 All Ability Slots after battle core and Summary ownership are
   less volatile.
9. Adopt #62 Champions Run Session after roster, item, party, and battle policy
   are present.
10. Add real Champions facility connection work after #62, not before.

Optional lanes that should be explicitly selected before inclusion:

- TM/HM lane: `feature/tm-shop-migration` ->
  `feature/unified-move-relearner` -> field move shelves.
- Team preview lane: `feature/prebattle-team-viewer-phase2`, optionally with
  `feature/battle-selection-mvp` if not taking the integrated viewer path.
- Editor lane: `feature/pokemon-state-editor-expansion`, after Summary / Party
  UI and Pokemon Vendor entitlement hooks are settled.
- Audio lane: `feature/battle-bgm-selector-mvp-20260517`.
- Tool lane: #65 Map Asset Relinker, outside ROM runtime-dev.
- Randomizer lane: `feature/EX/ex-rz-upstream1`, outside the first integration
  pass.

## Review Workflow

For each candidate branch:

1. Inspect `git diff --name-status master...<branch>`.
2. Inspect `git log --oneline --no-merges master..<branch>`.
3. If the branch has a PR, review the PR diff and comments before adoption.
4. Apply into `integration/runtime-dev-*` with either merge, cherry-pick, or
   manual re-application; do not use the GitHub merge button into `master`.
5. Resolve conflicts while preserving current `master` docs and branch-specific
   runtime source.
6. Run the relevant local validation.
7. Attempt one focused mGBA Live check for runtime-visible behavior.
8. Record adopted commit, conflict choices, validation, and remaining risks in
   the owning feature docs.

## Current Recommendation

Do not close runtime PRs yet, except the superseded docs-only #64. Keep them as
source shelves until `integration/runtime-dev-20260529` contains equivalent or
better code. Once a feature has been adopted into runtime-dev and validated,
update the owning docs and then close the old PR with a superseded-by note.
