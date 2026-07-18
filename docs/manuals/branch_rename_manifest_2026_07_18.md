# Branch Rename Manifest 2026-07-18

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-07-18 |
| Repository | `Jastin-Infj/pokeemerald-expansion` |
| Branch snapshot | 92 GitHub branches |
| Related audit | [Branch Inventory 2026-07-18](branch_inventory_2026_07_18.md) |
| Status | Proposed and mechanically validated; no rename executed yet |

This manifest is the source-of-truth old-to-new branch map. It separates a
historical branch's content from its current workflow state. A branch can hold
valuable work while still requiring an `archive/`, `shelf/`, or `snapshot/`
name so it is not mistaken for active development.

## Validation Result

| Check | Result |
|---|---:|
| Current GitHub branches | 92 |
| Manifest rows | 92 |
| Missing current branches | 0 |
| Extra manifest sources | 0 |
| Duplicate targets | 0 |
| Existing target collisions | 0 |
| Invalid Git branch targets | 0 |
| Open PR heads protected from immediate rename | 6 / 6 |
| Registered worktrees protected from immediate rename | 8 / 8 |
| `keep` | 1 |
| `rename` | 80 |
| `defer-open-pr` | 6 |
| `defer-worktree` | 5 |

## Action Meanings

| Action | Meaning |
|---|---|
| `keep` | Name is already canonical. |
| `rename` | No open PR or registered worktree blocks the planned rename. Execute in reviewed batches. |
| `defer-open-pr` | Renaming now would close an open PR. Finalize the PR first. |
| `defer-worktree` | Repair or remove the registered worktree metadata before renaming. |

## Complete Manifest

| Current | Action | Target | Note |
|---|---|---|---|
| `dev` | `rename` | `snapshot/runtime-1.12.0/mixed-dev-20251026` | Old mixed integration |
| `docs/16-runtime-lineage-handoff` | `rename` | `archive/docs/20260602/16-runtime-lineage-handoff` | Merged #70 |
| `docs/all-ability-slots-handoff-20260524` | `rename` | `archive/docs/20260524/all-ability-slots-handoff` | Merged #61 |
| `docs/branch-inventory-20260718` | `defer-open-pr` | `archive/docs/20260718/branch-inventory` | PR #82 |
| `docs/champions-run-session-handoff-20260525` | `rename` | `archive/docs/20260525/champions-run-session-handoff` | Merged #63 |
| `docs/comprehensive-feature-inventory-20260518` | `rename` | `archive/docs/20260518/comprehensive-feature-inventory` | Merged #46 |
| `docs/feature-branch-audit-20260518` | `rename` | `archive/docs/20260518/feature-branch-audit` | Merged #44 |
| `docs/field-styler-icon-handoff` | `rename` | `archive/docs/20260509/field-styler-icon-handoff` | Merged evidence |
| `docs/friendly-shop-pokemon-vendor-20260522` | `rename` | `archive/docs/20260522/friendly-shop-pokemon-vendor` | Merged #56 |
| `docs/implementation-shelf-cleanup-20260517` | `rename` | `archive/docs/20260517/implementation-shelf-cleanup` | Merged evidence |
| `docs/lua-pr-scope-rules` | `rename` | `archive/docs/20260515/lua-pr-scope-rules` | Merged #25 |
| `docs/map-asset-relinker-20260525` | `rename` | `archive/docs/20260525/map-asset-relinker-plan` | Closed #64 |
| `docs/mgba-cache-preservation` | `rename` | `archive/docs/20260506/mgba-cache-preservation` | Merged evidence |
| `docs/mgba-init-build-validation` | `rename` | `archive/docs/20260506/mgba-init-build-validation` | Merged evidence |
| `docs/mgba-live-runtime-validation` | `rename` | `archive/docs/20260506/mgba-live-runtime-validation` | Merged evidence |
| `docs/new-feature-candidates-master-20260517` | `rename` | `archive/docs/20260517/new-feature-candidates` | Merged #35 |
| `docs/next-runtime-triage-20260518` | `rename` | `archive/docs/20260518/next-runtime-triage` | Merged #43 |
| `docs/pokemon-vendor-handoff-20260523` | `rename` | `archive/docs/20260523/pokemon-vendor-handoff` | Merged #58 |
| `docs/prebattle-team-viewer-handoff` | `rename` | `archive/docs/20260510/prebattle-team-viewer-handoff` | Merged #21 |
| `docs/prebattle-team-viewer-phase2-handoff` | `rename` | `archive/docs/20260510/prebattle-team-viewer-phase2-handoff` | Merged evidence |
| `docs/prebattle-team-viewer-phase2-pr` | `rename` | `archive/docs/20260510/prebattle-team-viewer-phase2-pr-handoff` | Merged #22 |
| `docs/runtime-lab-handoff-20260716` | `defer-worktree` | `archive/docs/20260716/runtime-lab-handoff` | Prunable worktree; merged #79 |
| `docs/scout-selection-runtime-plan-20260519` | `rename` | `archive/docs/20260519/scout-selection-runtime-plan` | Merged #50 |
| `docs/team-viewer-pool-partygen-audit-20260518` | `rename` | `archive/docs/20260518/team-viewer-pool-partygen-audit` | Merged #45 |
| `docs/trainer-battle-reward-audio-metadata-20260517` | `rename` | `archive/docs/20260517/trainer-battle-reward-audio-metadata` | Merged #36 |
| `docs/unified-move-relearner-master-md` | `rename` | `archive/docs/20260516/unified-move-relearner-handoff` | Merged #29 |
| `docs/unified-move-relearner-scope` | `rename` | `archive/docs/20260516/unified-move-relearner-scope` | Closed #27 |
| `docs/upstream-generation-policy-20260718` | `rename` | `archive/docs/20260718/upstream-generation-policy` | Merged #81 |
| `Documents/GPT` | `rename` | `archive/docs/20251208/early-agents-project-links` | Old Docs experiment |
| `feature/all-ability-slots-runtime-20260523` | `rename` | `shelf/runtime-1.15.2/all-ability-slots` | Validated shelf |
| `feature/battle-bgm-selector-mvp-20260517` | `rename` | `shelf/runtime-1.15.2/battle-bgm-selector` | Validated shelf |
| `feature/battle-item-restore-current-master-20260519` | `rename` | `shelf/runtime-1.15.2/battle-item-restore` | Validated shelf |
| `feature/battle-item-restore-policy` | `rename` | `archive/prototype-1.15.2/battle-item-restore-policy` | Superseded shelf |
| `feature/battle-selection-mvp` | `rename` | `shelf/runtime-1.15.2/battle-selection` | Validated shelf |
| `feature/battle-team-boxes-20260716` | `defer-open-pr` | `shelf/runtime-1.16.1/battle-team-boxes` | PR #75; registered worktree |
| `feature/battleMain` | `rename` | `archive/prototype-1.11.1/battle-party-randomizer` | Unrelated early history |
| `feature/birch_case` | `rename` | `archive/prototype-1.13.3/birch-case` | Old prototype |
| `feature/box-npc-party-pool-20260705` | `defer-open-pr` | `shelf/runtime-1.16.1/box-npc-party-pool` | PR #74; base of #75 |
| `feature/champions-partygen-16-20260603` | `defer-open-pr` | `shelf/runtime-1.16.1/champions-partygen` | PR #71 |
| `feature/champions-run-session-runtime-20260524` | `rename` | `shelf/runtime-1.15.2/champions-run-session` | Validated shelf |
| `feature/dynamicmulti` | `rename` | `archive/prototype-1.12.0/dynamic-multi` | Old prototype |
| `feature/ex-rz-upstream1` | `rename` | `archive/prototype-1.14.1/trainer-randomizer` | Superseded prototype |
| `feature/EX/ex-rz-upstream1` | `rename` | `shelf/runtime-1.14.1/extended-randomizer` | Broad preserved randomizer |
| `feature/field-move-modernization-mvp` | `rename` | `shelf/runtime-1.15.2/field-move-modernization` | Validated shelf |
| `feature/field-move-toolkit-item` | `rename` | `shelf/runtime-1.15.2/field-move-toolkit` | Validated dependent shelf |
| `feature/field-surfboard` | `rename` | `archive/prototype-1.14.1/field-surfboard` | Incomplete prototype |
| `feature/global-no-evolution-20260523` | `rename` | `shelf/runtime-1.15.2/pokemon-vendor-no-evolution` | Validated shelf |
| `feature/held-item-catalog-current-master-20260519` | `rename` | `shelf/runtime-1.15.2/held-item-catalog` | Validated shelf |
| `feature/main_menu` | `rename` | `archive/prototype-1.11.1/main-menu` | Unrelated early history |
| `feature/map-asset-relinker-20260525` | `rename` | `tool/1.15.2/map-asset-relinker` | Separate tooling lane |
| `feature/modern-qol-field-moves` | `rename` | `archive/prototype-1.14.1/modern-qol-field-moves` | Superseded prototype |
| `feature/move_relearner` | `rename` | `archive/prototype-1.11.1/move-relearner` | Unrelated early history |
| `feature/new-map` | `rename` | `archive/prototype-1.14.1/map-region-fly` | Old map experiment |
| `feature/new-map-test-v15` | `rename` | `archive/prototype-1.15.1/rouge-cave-map` | Closed #4 |
| `feature/no-random-encounters` | `rename` | `archive/prototype-1.15.2/no-random-encounters` | Superseded prototype |
| `feature/no-random-encounters-step-only` | `rename` | `archive/prototype-1.15.2/no-random-step-only` | Superseded prototype |
| `feature/no-random-encounters-step-only-adopt-20260517` | `rename` | `archive/docs/20260517/no-random-adoption` | Merged Docs-only #34 |
| `feature/no-random-encounters-step-only-runtime-20260517` | `rename` | `shelf/runtime-1.15.2/no-random-step-only` | Validated shelf |
| `feature/party-select-ui` | `rename` | `archive/prototype-1.14.1/party-select-ui` | Known-bug prototype |
| `feature/party-status-ui-overhaul-20260521` | `rename` | `shelf/runtime-1.15.2/party-status-ui` | Validated shelf |
| `feature/pokemon-state-editor-expansion` | `rename` | `shelf/runtime-1.15.2/pokemon-state-editor` | Validated shelf |
| `feature/poryscript` | `rename` | `archive/prototype-1.11.1/poryscript` | Unrelated early history |
| `feature/prebattle-team-viewer` | `rename` | `shelf/runtime-1.15.2/prebattle-team-viewer` | Validated shelf |
| `feature/prebattle-team-viewer-phase2` | `rename` | `shelf/runtime-1.15.2/prebattle-team-viewer-phase2` | Validated shelf |
| `feature/qol_field_moves` | `rename` | `archive/prototype-1.14.1/qol-field-moves` | Superseded prototype |
| `feature/releaseSystem` | `rename` | `archive/prototype-1.11.1/release-system` | Unrelated early history |
| `feature/sandbox_v12` | `rename` | `archive/prototype-1.12.0/ability-ui-sandbox` | Old prototype |
| `feature/scout-selection-runtime-20260520` | `rename` | `shelf/runtime-1.15.2/scout-selection` | Validated shelf |
| `feature/smart-gimmick-ai-16-20260604` | `defer-open-pr` | `shelf/runtime-1.16.1/smart-gimmick-ai` | PR #72; active dirty worktree |
| `feature/summary-tera-type-badge` | `rename` | `shelf/runtime-1.15.2/summary-tera-badge` | Validated shelf |
| `feature/tm-shop-migration` | `rename` | `shelf/runtime-1.15.2/tm-shop-migration` | Validated shelf |
| `feature/trainer-battle-aftercare-heal` | `rename` | `shelf/runtime-1.15.2/trainer-battle-aftercare` | Validated shelf |
| `feature/trainer-partygen-catalog-expansion` | `rename` | `shelf/runtime-1.15.2/trainer-partygen-catalog` | Validated shelf |
| `feature/unified-move-relearner` | `rename` | `shelf/runtime-1.15.2/unified-move-relearner` | Validated shelf |
| `fix/docs-validate-policy` | `rename` | `archive/workflow/20260506/docs-validate-policy` | Merged #6 |
| `integration/runtime-dev-16-20260531` | `defer-open-pr` | `snapshot/runtime-1.16.0/full-stack-20260531` | PR #69 |
| `integration/runtime-dev-20260529` | `rename` | `snapshot/runtime-1.15.3/full-stack-20260529` | Closed #68 |
| `integration/runtime-lab-20260716` | `defer-worktree` | `snapshot/runtime-1.16.1/battle-lab-20260716` | Live worktree |
| `integration/runtime-lab-battle-team-pr-20260716` | `defer-worktree` | `archive/staging/runtime-1.16.1/battle-team-20260716` | Prunable worktree; merged #77 |
| `integration/runtime-lab-box-npc-pr-20260716` | `defer-worktree` | `archive/staging/runtime-1.16.1/box-npc-20260716` | Prunable worktree; merged #76 |
| `integration/runtime-lab-smart-ai-pr-20260716` | `defer-worktree` | `archive/staging/runtime-1.16.1/smart-ai-20260716` | Prunable worktree; merged #78 |
| `item_clock` | `rename` | `archive/prototype-1.12.0/clock-key-item` | Old prototype |
| `item_heal_patry` | `rename` | `archive/prototype-1.12.0/party-heal-item` | Old prototype |
| `item_keyfly` | `rename` | `archive/prototype-1.12.0/fly-key-item` | Old prototype |
| `master` | `keep` | `master` | Default branch |
| `TM_v12_0` | `rename` | `archive/prototype-1.12.0/gen9-tm-save` | Old prototype |
| `upgrade/1.15.2` | `rename` | `snapshot/upstream-1.15.2` | Exact upstream snapshot |
| `upgrade/master-before-1.16.0-sync-20260531` | `rename` | `snapshot/master-pre-1.16.0/20260531` | Recovery baseline |
| `vanilla/v11_1_1` | `rename` | `snapshot/upstream-1.11.1-import` | Early imported root |
| `vanilla/v12_0_0` | `rename` | `snapshot/upstream-1.12.0` | Exact upstream snapshot |
| `vanilla/v13_3_3` | `rename` | `snapshot/upstream-1.13.3` | Exact upstream snapshot |
| `vanilla/v14_1` | `rename` | `archive/baseline-1.14.1/local-init` | Upstream plus local init marker |

## Execution Batches

### Batch 0 - Publish Policy

1. Complete Docs PR #82 and merge it to `master`.
2. Switch the Docs worktree away from `docs/branch-inventory-20260718`.
3. Rename that merged branch to its `archive/docs/` target.
4. Record the final merge commit before any other rename.

### Batch 1 - Merged Docs And Workflow

Rename merged/closed Docs and workflow branches first. These branches are
already represented by PR history or `master`, so this batch tests the rename
procedure with the lowest recovery risk.

### Batch 2 - Upstream And Runtime Snapshots

Rename immutable `vanilla/`, `upgrade/`, and completed integration branches to
`snapshot/` or `archive/baseline-*`. Verify every target points to the exact
pre-rename SHA.

### Batch 3 - Validated Shelves And Tools

Rename completed standalone implementations to `shelf/` and the Map Asset
Relinker to `tool/`. This changes names only; no branch is rebased or merged.

### Batch 4 - Legacy Prototypes

Move old, superseded, incomplete, and unrelated-history branches under
`archive/prototype-*`. Preserve them as read-only evidence.

### Batch 5 - Worktree-Registered Branches

For paths marked `prunable`, first prove the directory no longer exists and
remove only stale worktree metadata. For the live runtime-lab worktree, switch
or repair the local branch/upstream association before the remote rename.

### Batch 6 - Open Runtime PR Heads

Finalize in dependency order:

1. #75 Battle Team Boxes, because its base is the #74 branch.
2. #74 Box NPC Party Pool.
3. #72 Smart Gimmick AI.
4. #71 Champions PartyGen.
5. #69 completed runtime snapshot.

Add a final status comment and close or supersede each PR before renaming its
head branch. Do not merge these implementation PRs into `master`.

## Per-Rename Transaction

For each row:

1. Read the source SHA from GitHub and compare it with the reviewed manifest.
2. Confirm that the target branch does not exist.
3. Confirm that the source is not an open PR head and is not a registered live
   worktree branch.
4. Use GitHub's branch rename API rather than create-new/delete-old refs. This
   preserves GitHub's branch URL redirect metadata.
5. Poll until the target exists at the exact source SHA.
6. Confirm the source name is no longer returned by the branch-list API.
7. Check open PR base/head names and states for unintended changes.
8. Fetch without pruning, repair local branch upstream/worktree names, then
   optionally prune only stale remote-tracking refs after the batch is proven.
9. Append the result, timestamp, source SHA, target SHA, and rollback status to
   the execution log.

## Rollback

If a rename produces an unexpected PR or worktree result, stop the batch. When
the old source name is still free, rename the target back to the source through
the same GitHub API and verify the original SHA. Do not compensate with a force
push, history rewrite, or branch deletion.

## Invariants

- Branch count remains constant during rename-only batches.
- Every target SHA equals its source SHA.
- No commit, tag, PR discussion, or implementation diff is changed.
- `master` is never renamed or force-pushed.
- The five runtime implementation PRs are not merged into `master`.
- The active dirty Smart AI worktree is not modified by branch cleanup.
- Exact upstream snapshots are not labeled as locally authored features.
