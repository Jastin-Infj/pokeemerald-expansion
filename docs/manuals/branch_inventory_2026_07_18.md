# Branch Inventory 2026-07-18

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-07-18 |
| Fork | `Jastin-Infj/pokeemerald-expansion` |
| Fork baseline | `master` `9bd7ce7ff1` |
| Latest stable upstream release | `expansion/1.16.2` `ad0fd4d17f` |
| Current upstream master | `df30c0b1a2`, untagged 1.16.3 development line |
| Scope | GitHub branch / PR / lineage audit; no branch rename, delete, merge, or source edit |

This inventory answers four separate questions:

1. Which branches are local work owned by this fork?
2. Which branches are upstream snapshots or old reference material?
3. Which implementation shelves have already been integrated into a playable
   snapshot?
4. Which names can be changed safely without closing an open pull request or
   losing the version lineage?

The branch name alone is not evidence of ownership or completion. Check the
commits authored or pushed by the fork owner, the branch diff, the PR body, and
the owning feature docs. Community or expert-authored commits that only form an
upstream baseline are provenance, not local feature work.

## Repository Facts

| Observation | Result |
|---|---|
| GitHub branches | 92 current: 91 audited branches plus this report branch |
| Local `origin/*` tracking refs | 94 branch refs plus symbolic `origin/HEAD` |
| Stale local-only tracking refs | `docs/runtime-lab-practical-20260718`, `docs/smart-gimmick-ai-master-handoff-20260705` |
| Pull requests | 82 total, including this report PR #82 |
| Open pull requests | 6, all draft: runtime #69, #71, #72, #74, #75 plus Docs #82 |
| PR author | All 82 PRs were opened by `Jastin-Infj` |
| Branches never used as a PR head | 38 |
| Branches with no commits ahead of current `master` | 26, including `master` and preserved upstream snapshots |
| Branches with commits ahead of current `master` | 66, including this report branch |
| Branch tips not authored by Jastin | 3 upstream snapshots: `upgrade/1.15.2`, `vanilla/v12_0_0`, `vanilla/v13_3_3` |
| Unrelated early history | 6 branches rooted at `vanilla/v11_1_1` |
| Branch protection / rulesets | None; every branch, including `master`, reports unprotected |
| Fork default branch | `master` |
| Automatic branch deletion after merge | Disabled |

GitHub compares fork `master` with RHH `master` as `diverged`: the fork is 107
commits ahead and 174 commits behind, with merge base `e7c30866ef`. The local
commits are primarily the project documentation and workflow overlay. The
current fork source reports untagged 1.16.1, while the stable release is 1.16.2
and RHH `master` already reports untagged 1.16.3.

The 1.16.2 release intake is structurally manageable but is not a blind
fast-forward. A read-only `git merge-tree` audit found three paths changed on
both sides: `CREDITS.md`, `docs/SUMMARY.md`, and
`docs/tutorials/how_to_trainer_party_pool.md`. No runtime source path was found
in that changed-on-both-sides set, but the actual intake still requires a
dedicated PR and validation.

## Version Coverage

The nearest reachable expansion release tag gives this complete 92-branch
coverage check. The audit query initially returned 91 branches; publishing this
report added `docs/branch-inventory-20260718` as the 92nd branch.

| Lineage | Branches |
|---|---:|
| 1.12.0 | 8 |
| 1.13.3 | 2 |
| 1.14.1 | 9 |
| 1.15.1 | 1 |
| 1.15.2 | 52 |
| 1.16.0/post-1.16.0 | 14 |
| Early unrelated/untagged history | 6 |
| Total | 92 |

## Current Runtime Lines

| Branch / PR | Actual role | Decision |
|---|---|---|
| `master` | Untagged 1.16.1 upstream source plus project Docs / workflow overlay | Upstream intake base, not the playable all-feature ROM |
| `integration/runtime-dev-16-20260531` / #69 | Completed 1.15.3 to 1.16.0 full-stack replay | Freeze as a historical runtime snapshot |
| `integration/runtime-lab-20260716` | Smart AI + Box NPC + Battle Team combined battle-debug ROM | Freeze as a separate battle-lab snapshot; it does not contain the full older stack |
| `feature/champions-partygen-16-20260603` / #71 | Completed standalone Champions PartyGen implementation | Reusable shelf; integration pending |
| `feature/smart-gimmick-ai-16-20260604` / #72 | Completed standalone Smart Gimmick AI | Reusable shelf; integrated only in the runtime lab |
| `feature/box-npc-party-pool-20260705` / #74 | Completed standalone Box NPC Party Pool | Reusable shelf; integrated only in the runtime lab |
| `feature/battle-team-boxes-20260716` / #75 | Completed Battle Team Boxes on the Box NPC branch | Reusable dependent shelf; integrated only in the runtime lab |

There is no active 1.16.2 all-feature integration branch. The old full-stack
snapshot and the newer runtime-lab snapshot are complementary evidence lines,
not one superseding implementation.

## Complete Branch Inventory

### 1.16.0 And Later Lineage - 14

| Branch | Content / status |
|---|---|
| `master` | Upstream intake plus Docs/workflow overlay; currently untagged 1.16.1 source |
| `docs/16-runtime-lineage-handoff` | Merged lineage handoff evidence |
| `docs/branch-inventory-20260718` | Current Docs-only audit branch and draft PR #82 |
| `docs/runtime-lab-handoff-20260716` | Merged runtime-lab handoff evidence |
| `docs/upstream-generation-policy-20260718` | Merged upstream-generation policy; retained branch |
| `feature/champions-partygen-16-20260603` | Champions PartyGen source/tool/test shelf; open draft #71 |
| `feature/smart-gimmick-ai-16-20260604` | Smart battle AI, tests, debug opponents, logging; open draft #72 |
| `feature/box-npc-party-pool-20260705` | Box Pokemon to healed NPC party debug battles; open draft #74 |
| `feature/battle-team-boxes-20260716` | Persistent Box battle-team registries; open draft #75 based on #74 |
| `integration/runtime-dev-16-20260531` | Full 1.15.3 feature set replayed onto 1.16.0; open draft #69, completed snapshot |
| `integration/runtime-lab-20260716` | Combined Smart + Box NPC + Battle Team evidence snapshot |
| `integration/runtime-lab-box-npc-pr-20260716` | Merged staging head for runtime-lab PR #76 |
| `integration/runtime-lab-battle-team-pr-20260716` | Merged staging head for runtime-lab PR #77 |
| `integration/runtime-lab-smart-ai-pr-20260716` | Merged staging head for runtime-lab PR #78 |

### 1.15.2 Lineage - 52

The implementation branches below were validated as standalone shelves and,
except for the Map Asset Relinker lane, were selectively replayed into
`integration/runtime-dev-20260529` and then ported into the 1.16.0 snapshot.
Their old commits remain useful as provenance, but their old upstream APIs are
not the source of truth for a 1.16.2 or later port.

| Branch group | Branches | Content / status |
|---|---|---|
| Full integration | `integration/runtime-dev-20260529` | Completed 1.15.3 all-feature runtime snapshot; superseded as active line by the 1.16.0 replay |
| Battle/item shelves | `feature/all-ability-slots-runtime-20260523`, `feature/battle-item-restore-current-master-20260519`, `feature/battle-item-restore-policy`, `feature/held-item-catalog-current-master-20260519`, `feature/trainer-battle-aftercare-heal` | Ability, battle-end item restore, nonconsumable item ownership, and post-battle healing implementations |
| Party/Summary shelves | `feature/battle-selection-mvp`, `feature/party-status-ui-overhaul-20260521`, `feature/pokemon-state-editor-expansion`, `feature/prebattle-team-viewer`, `feature/prebattle-team-viewer-phase2`, `feature/summary-tera-type-badge`, `feature/unified-move-relearner` | Party selection, 2x3 UI, state editor, team viewer, Tera badge, and unified move learning |
| Challenge/content shelves | `feature/champions-run-session-runtime-20260524`, `feature/global-no-evolution-20260523`, `feature/scout-selection-runtime-20260520`, `feature/trainer-partygen-catalog-expansion` | Champions run restore, Pokemon Vendor/no-evolution, Scout Selection, and PartyGen catalog |
| Field/audio/system shelves | `feature/battle-bgm-selector-mvp-20260517`, `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item`, `feature/tm-shop-migration`, `feature/no-random-encounters-step-only-runtime-20260517` | BGM archive, Field Kit, TM migration, and validated step-only no-encounter mode |
| Earlier/superseded no-encounter branches | `feature/no-random-encounters`, `feature/no-random-encounters-step-only`, `feature/no-random-encounters-step-only-adopt-20260517` | Earlier implementation and adoption-doc stages; preserve only as historical evidence |
| Separate tooling lane | `feature/map-asset-relinker-20260525` | Rust/GUI/Tauri map relinker implementation; not a normal ROM feature shelf |
| Old map draft | `feature/new-map-test-v15` is listed under 1.15.1 below | Closed Rouge Cave draft, not a clean current map baseline |
| CI policy | `fix/docs-validate-policy` | Merged workflow fix |
| Upstream snapshots | `upgrade/1.15.2`, `upgrade/master-before-1.16.0-sync-20260531` | Exact/recovery baselines, not local runtime features |

The 23 retained 1.15.2 Docs branches are listed below. Most are merged into
`master`; the closed plan branches are superseded by later handoff PRs.

- `docs/all-ability-slots-handoff-20260524`
- `docs/champions-run-session-handoff-20260525`
- `docs/comprehensive-feature-inventory-20260518`
- `docs/feature-branch-audit-20260518`
- `docs/field-styler-icon-handoff`
- `docs/friendly-shop-pokemon-vendor-20260522`
- `docs/implementation-shelf-cleanup-20260517`
- `docs/lua-pr-scope-rules`
- `docs/map-asset-relinker-20260525`
- `docs/mgba-cache-preservation`
- `docs/mgba-init-build-validation`
- `docs/mgba-live-runtime-validation`
- `docs/new-feature-candidates-master-20260517`
- `docs/next-runtime-triage-20260518`
- `docs/pokemon-vendor-handoff-20260523`
- `docs/prebattle-team-viewer-handoff`
- `docs/prebattle-team-viewer-phase2-handoff`
- `docs/prebattle-team-viewer-phase2-pr`
- `docs/scout-selection-runtime-plan-20260519`
- `docs/team-viewer-pool-partygen-audit-20260518`
- `docs/trainer-battle-reward-audio-metadata-20260517`
- `docs/unified-move-relearner-master-md`
- `docs/unified-move-relearner-scope`

### 1.15.1 Lineage - 1

| Branch | Content / status |
|---|---|
| `feature/new-map-test-v15` | Rouge Cave map draft from closed PR #4; restart from a current baseline rather than merging wholesale |

### 1.14.1 Lineage - 9

| Branch | Content / status |
|---|---|
| `vanilla/v14_1` | Upstream 1.14.1 snapshot plus local `vanilla init` marker |
| `Documents/GPT` | Early AGENTS/project-link documentation experiment |
| `feature/EX/ex-rz-upstream1` | Broad randomizer line: encounter/time slots, DexNav, trainer rank/tag logic, item weighting/icons, generated data, tests, and extensive planning docs |
| `feature/ex-rz-upstream1` | Earlier two-commit trainer randomizer prototype; superseded by the broader EX branch |
| `feature/field-surfboard` | Incomplete Surfboard field-move prototype |
| `feature/modern-qol-field-moves` | Earlier field-move effect prototype |
| `feature/new-map` | Map, region-map, and Fly behavior experiment |
| `feature/party-select-ui` | Earlier party-selection UI prototype with known selection bug |
| `feature/qol_field_moves` | Earlier QoL field-move prototype |

### 1.13.3 Lineage - 2

| Branch | Content / status |
|---|---|
| `vanilla/v13_3_3` | Exact upstream snapshot; tip authored by an upstream contributor |
| `feature/birch_case` | Custom Birch case UI plus Dojo map and script/content changes |

### 1.12.0 Lineage - 8

| Branch | Content / status |
|---|---|
| `vanilla/v12_0_0` | Exact upstream snapshot; tip authored by an upstream contributor |
| `TM_v12_0` | Three local commits implementing the Gen 9 TM set and save use |
| `dev` | Old mixed integration line containing map/editor, shop, DexNav, TM, relearner, key-item, weather, relic/gem, berry restore, AI, and party-select experiments |
| `feature/dynamicmulti` | One-commit dynamic multi/tutorial prototype |
| `feature/sandbox_v12` | Two-commit ability UI sandbox |
| `item_clock` | Clock key-item prototype |
| `item_heal_patry` | Party-heal key-item prototype |
| `item_keyfly` | Fly key-item prototype |

### Early Unrelated/Untagged History - 6

These branches share the one-commit `vanilla/v11_1_1` root but do not share the
current repository history. They must never be merged or rebased wholesale into
the current line.

| Branch | Content / status |
|---|---|
| `vanilla/v11_1_1` | One-commit imported baseline |
| `feature/main_menu` | One local main-menu commit |
| `feature/poryscript` | One local Poryscript commit |
| `feature/move_relearner` | One local move-relearner commit |
| `feature/battleMain` | One local randomized battle-party commit |
| `feature/releaseSystem` | One local release-system commit |

## Open PR Rename Constraint

GitHub automatically closes an open pull request when its head branch is
renamed. Therefore these branches must not be renamed until the PR is
deliberately finalized, commented, and closed or superseded:

- #69 `integration/runtime-dev-16-20260531`
- #71 `feature/champions-partygen-16-20260603`
- #72 `feature/smart-gimmick-ai-16-20260604`
- #74 `feature/box-npc-party-pool-20260705`
- #75 `feature/battle-team-boxes-20260716`
- #82 `docs/branch-inventory-20260718`

Branch URLs redirect after a GitHub rename, but raw URLs, local pull commands,
GitHub Actions references, and checked-out local worktrees do not update
automatically. Rename work therefore needs an old-to-new manifest and a local
worktree/upstream repair pass.

## Proposed Naming Model

GitHub Flow normally recommends deleting a branch after its PR is merged so
that completed work is not mistaken for active work. This repository makes a
deliberate exception for implementation evidence and recovery points. A
preserved branch must therefore move under `shelf/`, `snapshot/`, or `archive/`
instead of remaining under an active-looking `feature/` or `integration/`
name.

| Pattern | Purpose | Mutability |
|---|---|---|
| `master` | Latest accepted stable RHH release plus minimal project Docs/AGENTS overlay | Updated only by upstream-intake or Docs/Lua PR |
| `upgrade/<version>-intake-<date>` | One upstream release intake | Short-lived PR branch |
| `integration/active-runtime-<version>` | The one current playable all-feature line | Receives PRs; no direct feature implementation |
| `tuning/runtime-<version>/<topic>` | Small balancing or compatibility change based on active integration | Short-lived PR into active integration |
| `feature/runtime-<version>/<feature>` | New standalone or integration-dependent runtime work | Active until reviewed |
| `shelf/runtime-<version>/<feature>` | Completed reusable standalone implementation | Frozen |
| `snapshot/runtime-<version>/<name>-<date>` | Completed combined runtime or recovery point | Frozen |
| `tool/<version>/<tool>` | Tooling that is not a ROM runtime feature | Active or frozen by tool status |
| `archive/prototype-<version>/<name>` | Old or superseded prototype with unique work | Frozen |
| `archive/docs/<date>/<topic>` | Retained merged/closed Docs PR branch | Frozen |
| `external/<source>/<name>` | Expert/community branch preserved without claiming local ownership | Frozen until provenance review |

The first proposed high-value renames, after the five open runtime PRs are
finalized, are:

| Current | Proposed |
|---|---|
| `integration/runtime-dev-20260529` | `snapshot/runtime-1.15.3/full-stack-20260529` |
| `integration/runtime-dev-16-20260531` | `snapshot/runtime-1.16.0/full-stack-20260531` |
| `integration/runtime-lab-20260716` | `snapshot/runtime-1.16.1/battle-lab-20260716` |
| `feature/smart-gimmick-ai-16-20260604` | `shelf/runtime-1.16.1/smart-gimmick-ai` |
| `feature/box-npc-party-pool-20260705` | `shelf/runtime-1.16.1/box-npc-party-pool` |
| `feature/battle-team-boxes-20260716` | `shelf/runtime-1.16.1/battle-team-boxes` |
| `feature/champions-partygen-16-20260603` | `shelf/runtime-1.16.1/champions-partygen` |
| `feature/map-asset-relinker-20260525` | `tool/1.15.2/map-asset-relinker` |

The baseline names should be normalized only after deciding whether exact
upstream snapshots remain branches or become annotated tags. If they remain
branches, use `snapshot/upstream-<version>` rather than `vanilla/*` and
`upgrade/*` for immutable historical points.

## Master Recommendation

Keep `master` on the latest accepted stable release, not on the moving RHH
`master`. Retain only the minimal project Docs, AGENTS, and approved Lua overlay
there. This is the recommended compromise for this repository because:

- RHH `master` is already untagged 1.16.3, while the latest stable release is
  1.16.2;
- an exact-upstream `master` would make Sync Fork easier but remove the local
  workflow contract from the default branch;
- controlled release intake can preserve upstream API/structure authority
  without silently adopting unreleased commits;
- the real missing development baseline is an active all-feature integration,
  not more runtime code on `master`.

GitHub has no suitable per-repository switch that means "hide only the Sync
Fork button but keep all other master writes." The practical control is a
branch protection/ruleset requiring PRs and blocking force-push/deletion. A
locked branch is too restrictive for normal Docs and intake PRs. Because this
is a personal fork, do not require an outside approval that the owner cannot
supply; require the PR path and selected status checks instead.

After a controlled 1.16.2 intake, create
`integration/active-runtime-1.16.2` fresh from the updated `master`. Reapply the
accepted full-stack features, runtime-lab features, and Champions PartyGen in a
recorded order through separate PRs. Create every balancing or small adjustment
as `tuning/runtime-1.16.2/<topic>` from the pinned active integration head and
merge it back through a PR. Do not implement directly on the active integration
branch.

## Proposed Execution Order

1. Review and approve this inventory and naming model.
2. Merge the inventory as a Docs-only PR if accepted.
3. Add a `master` protection/ruleset that requires PRs and blocks force-push
   and deletion, without requiring an unavailable external reviewer.
4. Create and validate `upgrade/1.16.2-intake-20260718` from `master`.
5. Freeze the pre-intake runtime lines using the approved snapshot names.
6. Create `integration/active-runtime-1.16.2` from updated `master`.
7. Finalize the five open runtime draft PRs before renaming their head
   branches. Finalize Docs PR #82 before renaming its branch.
8. Reapply accepted shelves into the active integration through separate PRs.
9. Rename closed/merged staging, prototype, Docs, and snapshot branches in
   batches, verifying refs and worktrees after every batch.
10. Update AGENTS, the GitHub workflow manual, clone/worktree instructions, and
    feature registry with the final names.

## Evidence Commands

The audit used current GitHub and local data, including:

```sh
rtk git fetch RHH --tags
rtk gh api --paginate repos/Jastin-Infj/pokeemerald-expansion/branches?per_page=100
rtk gh pr list --repo Jastin-Infj/pokeemerald-expansion --state all --limit 200
rtk git for-each-ref --format='%(refname:short)|%(ahead-behind:origin/master)'
rtk git describe --tags --match 'expansion/[0-9]*' --abbrev=0 <branch>
rtk git merge-tree <merge-base> origin/master expansion/1.16.2
```

GitHub behavior was checked against the official GitHub Flow, fork-sync,
branch-rename, and protected-branch documentation before proposing the rename
sequence.

## External References

- [GitHub Flow](https://docs.github.com/en/get-started/using-github/github-flow)
- [Syncing a fork](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/syncing-a-fork)
- [Renaming a branch](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-branches-in-your-repository/renaming-a-branch)
- [About protected branches](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches)
