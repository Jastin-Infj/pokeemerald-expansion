# Implementation Shelf Audit 2026-05-17

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `b31c695dc5`; `git describe` = `expansion/1.15.2-66-gb31c695dc5` |
| Code status | Docs-only audit / no source changes |
| Provenance | `gh pr list --state all`, fetched PR refs, local and remote branch merge-base diffs, 2026-05-17 PR cleanup |

This audit records which features have real implementation branches, including
closed PRs and branches that never had a current PR. Closed does not mean
unimplemented; several closed PRs are validated implementation shelves that were
closed after handoff or because the master intake policy changed.

## Current Master Reality

`master` is still the upstream intake baseline plus docs / workflow overlay.
Local runtime source is not generally on `master`. Runtime feature work should be
adopted from a fresh branch or selected implementation shelf.

## Current Open Runtime PR Shelves

None. #41 finished CI, reported `mergeStateStatus = CLEAN`, and was closed on
2026-05-17 as a completed implementation shelf. All runtime implementation
shelves are now closed / branch-preserved.

## Completed Runtime PR Shelves

The following PRs were closed on 2026-05-17 after their CI had already passed
or their runtime evidence had been recorded. Branches were preserved. Closing
does not mean abandoned; it means the implementation shelf is complete and
`master` remains docs-only.

| PR | Feature | Branch | Implementation reality | Validation evidence |
|---|---|---|---|---|
| #41 | No Random Encounters step-only | `feature/no-random-encounters-step-only-runtime-20260517` | Implemented 3-file step-only random encounter suppression through the existing `OW_FLAG_NO_ENCOUNTER` gate. | CI success, local all/debug/check, mGBA Route 101 OFF / ON / OFF-restored evidence, and user confirmation. |
| #39 | Battle BGM Selector / Sound Archive | `feature/battle-bgm-selector-mvp-20260517` | Implemented selector plus BW/BW2, DPPt, Platinum, and HGSS battle BGM imports. | CI success, local all/debug/check, focused battle BGM tests, and mGBA selector / song-header evidence. Large asset/audio shelf; Modern Emerald permission / source risk remains before adoption. |
| #31 | TM Shop Migration | `feature/tm-shop-migration` | Implemented Emerald normal-progression legacy TM/HM acquisition retirement. | CI success and local all/debug/check; this is the cleanest completed shelf to re-apply first if adoption resumes. |
| #28 | Unified Move Relearner | `feature/unified-move-relearner` | Implemented level / egg / TM / tutor / special candidate builder and long-list UI. | CI success, JSON audits, candidate checks, and mGBA long-list / form checks. Needs conflict refresh before adoption. |
| #26 | Summary Tera Type Icon | `feature/summary-tera-type-badge` | Implemented Summary Info display-only Tera icon with imported graphics. | CI success, local all/debug/check, and mGBA Summary screenshot evidence. |
| #23 | Pokemon State Editor | `feature/pokemon-state-editor-expansion` | Implemented Summary-launched editor MVP. | CI success, local all/debug/check, and multiple mGBA UI / direct Lua checks. |
| #20 | Pre-Battle / In-Battle Team Viewer | `feature/prebattle-team-viewer` | Implemented pre-battle preview, cached opponent party, and in-battle viewer. | CI success, local all/debug/check, and mGBA single/double/in-battle viewer evidence. |
| #16 | No Random Encounters old shelf | `feature/no-random-encounters-step-only` | Superseded by #41 fresh runtime branch. | Keep for history only. |
| #14 | Battle Item Restore Policy | `feature/battle-item-restore-policy` | Implemented `B_RESTORE_HELD_BATTLE_BERRIES`, berry restore through `TryRestoreHeldItems()`, direct and full battle tests. | Focused `battle_item_restore` checks, full `all` / `debug`, and mGBA test-runner memory evidence were recorded in the PR body and feature test plan. |
| #10 | Trainer Battle Aftercare heal hook | `feature/trainer-battle-aftercare-heal` | Implemented default-off normal trainer battle win heal hook with exclusions. | `all`, `debug`, `check`, docs validation, and mGBA title/boot smoke were recorded. Needs focused exclusion tests before adoption. |
| #7 | Champions Partygen catalog expansion | `feature/trainer-partygen-catalog-expansion` | Implemented Rust partygen CLI, catalog, lint data, Elite Four / Wallace trainer data diff, and validation commands. | Cargo tests / clippy / partygen doctor-generate-validate-diff and mGBA trainer memory read were recorded. |
| #4 | Rouge Cave map draft | `feature/new-map-test-v15` | Implemented map/layout draft branch, not a validated adoption shelf. | Closed with CI failure / draft status; reopen only as a fresh map branch. |

## Implemented Branches Without Current PR

| Feature | Branch | Merge-base diff shape | Adoption note |
|---|---|---|---|
| Trainer Battle Party Selection | `feature/battle-selection-mvp` | 16 files: battle config, party menu, battle setup, selection source, docs. | Validated MVP branch; can be re-applied before team viewer if the viewer branch is not selected wholesale. |
| Field Move Modernization MVP | `feature/field-move-modernization-mvp` | 17 files: field move scripts, Surf text, config, field move / control source, docs. | Implemented and user-confirmed. |
| Field Kit itemization | `feature/field-move-toolkit-item` | 41 files: Field Kit item, scripts, item use, region map, graphics, docs. | Implemented stacked follow-up; graphics are implementation artifacts, not docs-only. |
| Pre-Battle Team Viewer phase 2 | `feature/prebattle-team-viewer-phase2` | 37 files: phase 2 viewer / Summary integration and docs. | Related to #20; treat as a separate evidence shelf if the newer branch is preferred. |

## Legacy / Older-Baseline Implementation Branches

These branches contain real implementations, but their merge bases are old
upstream baselines (`vanilla/v12_0_0` or `vanilla/v14_1`). They should be used
as behavior references, not merged into current `master`.

| Branch | Inferred feature | Current use |
|---|---|---|
| `origin/item_clock` | Clock key item | Reference for future item / utility work. |
| `origin/item_heal_patry` | Heal party key item | Reference for aftercare / utility item behavior. |
| `origin/item_keyfly` | Fly key item | Reference for Field Kit / Fly itemization. |
| `origin/TM_v12_0` | Gen 9 / expanded TM implementation | Reference only; current TM Shop Migration and Unified Relearner are the v15-compatible path. |
| `origin/feature/sandbox_v12` | UI stat editor prototype | Reference for Pokemon State Editor / UI controls. |
| `origin/feature/ex-rz-upstream1` and `origin/feature/EX/ex-rz-upstream1` | Randomizer / item-ball icon / trainer-rank experiments | Reference for future randomizer and trainer-rank work. |
| `origin/feature/modern-qol-field-moves`, `origin/feature/qol_field_moves`, `origin/feature/field-surfboard` | Older field move / surfboard prototypes | Reference for Field Move Modernization; current v15 branches supersede them. |
| `origin/feature/new-map` | Older map / Fly / region map prototype | Reference for map workflow only. |
| `origin/feature/party-select-ui` | Older custom party select UI prototype | Reference only; current `feature/battle-selection-mvp` is the v15 path. |

## Updated Working Priority

1. No Random Encounters is complete as a shelf. Runtime behavior and CI are
   confirmed; do not spend another implementation slice on it unless the scope
   expands beyond step-only random encounters.
2. If adoption resumes, re-apply TM Shop Migration first from its completed
   shelf; it was the only runtime PR that reported `CLEAN` before cleanup.
3. Rebase / refresh Unified Move Relearner after TM Shop Migration because it
   depends on the TM/HM policy split.
4. Decide whether Field Move Modernization / Field Kit should enter before
   player-facing UI shelves. It is implemented and user-confirmed, but has no
   current open PR.
5. Then pick among Summary Tera Icon, Pokemon State Editor, and Pre-Battle Team
   Viewer based on desired UI surface and conflict tolerance.
6. Treat Battle Item Restore / Aftercare / Nonconsumable Held Items as a grouped
   policy lane. Battle-end restore is implemented; catalog assignment is not.
7. Treat Champions Partygen as implemented tool/data work, but defer Champions
   runtime until the battle / bag / aftercare policy stack is settled.
