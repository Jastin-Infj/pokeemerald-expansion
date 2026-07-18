# Branch Rename Manifest 2026-07-18

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-07-18 |
| Repository | `Jastin-Infj/pokeemerald-expansion` |
| Branch snapshot | 92 GitHub branches at `2026-07-18T09:03:45Z` |
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
| Captured source SHAs | 92 / 92 |
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

The source SHA column is the immutable preflight snapshot for eligible
`rename` rows. Any mismatch at execution time stops that row and the batch.
The `docs/branch-inventory-20260718` SHA necessarily advances when this
snapshot is committed; that row is already `defer-open-pr` and its final SHA
must be recaptured after PR #82 is finalized.

## Approval Gate

The inventory request requires the naming and `master` policy to be reviewed
before remote mutation. The recommended decision set is:

| ID | Decision | Recommended approval |
|---|---|---|
| D1 | `master` role | Latest accepted stable RHH release plus minimal Markdown, `AGENTS.md`, and approved Lua overlay. |
| D2 | Upstream source | Intake the pinned stable release through `upgrade/<version>-intake-<date>`; never sync the moving RHH `master` blindly. |
| D3 | Playable development base | Maintain exactly one `integration/active-runtime-<version>` and accept feature/tuning PRs into it. |
| D4 | Retention | Preserve unique implementation and recovery refs under `shelf/`, `snapshot/`, or `archive/`; do not mass-delete them. |
| D5 | Exact upstream baselines | Keep them as `snapshot/upstream-<version>` branches during this transition. Annotated tags may be added later without deleting the branches. |
| D6 | `master` protection | Require a PR, block force-push and deletion, require no unavailable outside approval, and do not lock the branch. |
| D7 | Rename execution | Merge Docs PR #82 first, then execute only eligible rows in the recorded batches. |

Until D1-D7 are explicitly accepted, this document remains a dry-run plan and
no remote rename is authorized. Accepting the complete recommended set is
sufficient; the decisions do not need separate replies.

## Repository Setting Proposal

Use a repository ruleset or branch protection rule targeting only `master`:

- require all changes to arrive through a pull request;
- require `docs_validate` initially;
- block force pushes and branch deletion;
- apply the rule to administrators as well, so an accidental direct push is
  rejected;
- require zero external approvals while this remains a sole-owner fork;
- do not enable `Lock branch`;
- leave `Allow fork syncing` disabled. GitHub exposes that option for a locked
  branch, but locking would also block the Docs and upstream-intake merges this
  repository needs.

As a follow-up, add one stable aggregate `master-gate` check that passes via a
short Docs path for Docs/Lua-only PRs and requires runtime build/test evidence
for upstream-intake PRs. Require that aggregate check only after it exists, so
the ruleset cannot reference an unavailable job.

## AGENTS, Docs, Clone, And Worktree Impact

After each successful rename batch:

1. Update current workflow references in `AGENTS.md`, `docs/manuals/`, feature
   registries, scripts, and Actions refs. Preserve historical names in evidence
   records when they identify the branch used at the time, and add the new name
   beside them instead of rewriting history.
2. Existing clones do not need to be replaced. Repair a locally checked-out
   renamed branch from its owning worktree:

   ```sh
   rtk git branch -m OLD-BRANCH NEW-BRANCH
   rtk git fetch origin
   rtk git branch -u origin/NEW-BRANCH NEW-BRANCH
   rtk git remote set-head origin -a
   ```

3. Do not run `remote prune` during a rename batch. After all target refs and
   local upstreams are verified, optionally run `rtk git remote prune origin`.
4. For a `prunable` worktree, prove its directory is absent with
   `rtk git worktree list`, inspect `rtk git worktree prune --dry-run`, and only
   then prune stale metadata. Do not remove a live or dirty worktree.
5. A fresh clone checks out only `master` by default. Create an explicit
   worktree for `integration/active-runtime-<version>` when playable feature
   development is needed; do not change the repository default branch away
   from `master`.

## Complete Manifest

| Current | Source SHA | Action | Target | Note |
|---|---|---|---|---|
| `dev` | `e90d51c31a06a51b43a2407a1f11bbb79da10348` | `rename` | `snapshot/runtime-1.12.0/mixed-dev-20251026` | Old mixed integration |
| `docs/16-runtime-lineage-handoff` | `139ef1285f61c52412de96cf9a739142fd2e8e03` | `rename` | `archive/docs/20260602/16-runtime-lineage-handoff` | Merged #70 |
| `docs/all-ability-slots-handoff-20260524` | `8327745d06a3e4c8d5410309d07ef3e8d72252c4` | `rename` | `archive/docs/20260524/all-ability-slots-handoff` | Merged #61 |
| `docs/branch-inventory-20260718` | `940c7dee244c55a72ed7021845293bfec22e567e` | `defer-open-pr` | `archive/docs/20260718/branch-inventory` | PR #82 |
| `docs/champions-run-session-handoff-20260525` | `e7ea27161b7c2c66b89609e07c3f0bab15ea7337` | `rename` | `archive/docs/20260525/champions-run-session-handoff` | Merged #63 |
| `docs/comprehensive-feature-inventory-20260518` | `cac95037c6985cf8929100dd2c08e02e733be45a` | `rename` | `archive/docs/20260518/comprehensive-feature-inventory` | Merged #46 |
| `docs/feature-branch-audit-20260518` | `fda9bb74283dc47881c895ab3215bf8b508f5f07` | `rename` | `archive/docs/20260518/feature-branch-audit` | Merged #44 |
| `docs/field-styler-icon-handoff` | `835520e44456a8ae96bc359606a27f522c495cc5` | `rename` | `archive/docs/20260509/field-styler-icon-handoff` | Merged evidence |
| `docs/friendly-shop-pokemon-vendor-20260522` | `f0f01cca024e423b89b751b2121bd144db874177` | `rename` | `archive/docs/20260522/friendly-shop-pokemon-vendor` | Merged #56 |
| `docs/implementation-shelf-cleanup-20260517` | `abbbf4755498497c0aaf9d6250522abecf80c80a` | `rename` | `archive/docs/20260517/implementation-shelf-cleanup` | Merged evidence |
| `docs/lua-pr-scope-rules` | `c94a8189ecb68e1eaea0da94b79fc40b8f9be695` | `rename` | `archive/docs/20260515/lua-pr-scope-rules` | Merged #25 |
| `docs/map-asset-relinker-20260525` | `dc91e601aa2f609e569a8a3b1d7bdc6470cdf0ae` | `rename` | `archive/docs/20260525/map-asset-relinker-plan` | Closed #64 |
| `docs/mgba-cache-preservation` | `846b7e6269a06b9cbcf1451e00285d61ce9bea50` | `rename` | `archive/docs/20260506/mgba-cache-preservation` | Merged evidence |
| `docs/mgba-init-build-validation` | `1bfcacec2d69241190d2d16094868af7916783ea` | `rename` | `archive/docs/20260506/mgba-init-build-validation` | Merged evidence |
| `docs/mgba-live-runtime-validation` | `a78cd4913b0e37c73717f00486f12f89c6403c7e` | `rename` | `archive/docs/20260506/mgba-live-runtime-validation` | Merged evidence |
| `docs/new-feature-candidates-master-20260517` | `3805bef89f90e70b465b408375fc69b35e31b75a` | `rename` | `archive/docs/20260517/new-feature-candidates` | Merged #35 |
| `docs/next-runtime-triage-20260518` | `91a8ae90215a916c0347a4a6f6ca975adde348ff` | `rename` | `archive/docs/20260518/next-runtime-triage` | Merged #43 |
| `docs/pokemon-vendor-handoff-20260523` | `6e3ae17f97b443c9ed080093f85adec4f3800e8e` | `rename` | `archive/docs/20260523/pokemon-vendor-handoff` | Merged #58 |
| `docs/prebattle-team-viewer-handoff` | `2f72e4a3644046bc8169e8131b60cacc176fd00d` | `rename` | `archive/docs/20260510/prebattle-team-viewer-handoff` | Merged #21 |
| `docs/prebattle-team-viewer-phase2-handoff` | `1b6dbd8f3c7df9fad21c2e7a3e5e84c036be9b3b` | `rename` | `archive/docs/20260510/prebattle-team-viewer-phase2-handoff` | Merged evidence |
| `docs/prebattle-team-viewer-phase2-pr` | `1f04584a80f203e0492852d317c272bf6b348022` | `rename` | `archive/docs/20260510/prebattle-team-viewer-phase2-pr-handoff` | Merged #22 |
| `docs/runtime-lab-handoff-20260716` | `300e0147e0e85eb062d534886cb508cede89bf42` | `defer-worktree` | `archive/docs/20260716/runtime-lab-handoff` | Prunable worktree; merged #79 |
| `docs/scout-selection-runtime-plan-20260519` | `297b2edb6c435d8318130203745ed00af3286e81` | `rename` | `archive/docs/20260519/scout-selection-runtime-plan` | Merged #50 |
| `docs/team-viewer-pool-partygen-audit-20260518` | `5c4edc5b31e8c393a69790765b4f1902dd7899c2` | `rename` | `archive/docs/20260518/team-viewer-pool-partygen-audit` | Merged #45 |
| `docs/trainer-battle-reward-audio-metadata-20260517` | `77da0acaba6c529fac2ea55928a0036170e5e855` | `rename` | `archive/docs/20260517/trainer-battle-reward-audio-metadata` | Merged #36 |
| `docs/unified-move-relearner-master-md` | `71d573f0add9ad5518d854943568a58d8d387ba4` | `rename` | `archive/docs/20260516/unified-move-relearner-handoff` | Merged #29 |
| `docs/unified-move-relearner-scope` | `19f89ee95322695e7defe650981a285ca832fec0` | `rename` | `archive/docs/20260516/unified-move-relearner-scope` | Closed #27 |
| `docs/upstream-generation-policy-20260718` | `f169a2457d164f3a478dd12285d17bf9abf40e7c` | `rename` | `archive/docs/20260718/upstream-generation-policy` | Merged #81 |
| `Documents/GPT` | `257c8211c46a7c8755694b0b0f4719479a7d576b` | `rename` | `archive/docs/20251208/early-agents-project-links` | Old Docs experiment |
| `feature/all-ability-slots-runtime-20260523` | `bb5fbce95587e9732d187aa04a34c1b0b0bdd14c` | `rename` | `shelf/runtime-1.15.2/all-ability-slots` | Validated shelf |
| `feature/battle-bgm-selector-mvp-20260517` | `a4c6035968ec0fd2890b12c17caad4f2658fa3f6` | `rename` | `shelf/runtime-1.15.2/battle-bgm-selector` | Validated shelf |
| `feature/battle-item-restore-current-master-20260519` | `feb7c4742766877b092e6169adb01f42ea0858d3` | `rename` | `shelf/runtime-1.15.2/battle-item-restore` | Validated shelf |
| `feature/battle-item-restore-policy` | `bf382e0f59f290183626a663cc0a18e1bdf0615d` | `rename` | `archive/prototype-1.15.2/battle-item-restore-policy` | Superseded shelf |
| `feature/battle-selection-mvp` | `ee0c89f77ea71f7b405c78ae7c2dcc992d306b96` | `rename` | `shelf/runtime-1.15.2/battle-selection` | Validated shelf |
| `feature/battle-team-boxes-20260716` | `ba98fed3829d3cb03c213d5b7af114782deba67f` | `defer-open-pr` | `shelf/runtime-1.16.1/battle-team-boxes` | PR #75; registered worktree |
| `feature/battleMain` | `9226420d6aacccc352497aef68298ae6979e190a` | `rename` | `archive/prototype-1.11.1/battle-party-randomizer` | Unrelated early history |
| `feature/birch_case` | `b2104203ca35be7f692f1f05a0a4b21808a2f1ed` | `rename` | `archive/prototype-1.13.3/birch-case` | Old prototype |
| `feature/box-npc-party-pool-20260705` | `71e9d602f00660f31d662af4eee69f5680d566c0` | `defer-open-pr` | `shelf/runtime-1.16.1/box-npc-party-pool` | PR #74; base of #75 |
| `feature/champions-partygen-16-20260603` | `46774d096777d15be577c1d12dd617f69000312a` | `defer-open-pr` | `shelf/runtime-1.16.1/champions-partygen` | PR #71 |
| `feature/champions-run-session-runtime-20260524` | `8edd1c97c582f28a831cf44a8576ded322a39556` | `rename` | `shelf/runtime-1.15.2/champions-run-session` | Validated shelf |
| `feature/dynamicmulti` | `9f1869a83c08956d613faaf54b50d509e41e8983` | `rename` | `archive/prototype-1.12.0/dynamic-multi` | Old prototype |
| `feature/ex-rz-upstream1` | `fdd4c0340d91b0dec6569f2d218e6e20da304996` | `rename` | `archive/prototype-1.14.1/trainer-randomizer` | Superseded prototype |
| `feature/EX/ex-rz-upstream1` | `64271a3aa2734655e5f47188f3f3c71ec3455f96` | `rename` | `shelf/runtime-1.14.1/extended-randomizer` | Broad preserved randomizer |
| `feature/field-move-modernization-mvp` | `2092900459e959a01ab12b2da3d63f36eda70a8d` | `rename` | `shelf/runtime-1.15.2/field-move-modernization` | Validated shelf |
| `feature/field-move-toolkit-item` | `6365b22e57afd838e82b22506aa0a6366e45beb3` | `rename` | `shelf/runtime-1.15.2/field-move-toolkit` | Validated dependent shelf |
| `feature/field-surfboard` | `f5e552b5188563c5d7cf23a3556cdfd561da6755` | `rename` | `archive/prototype-1.14.1/field-surfboard` | Incomplete prototype |
| `feature/global-no-evolution-20260523` | `51e777ad8ac1541d6eae01ac49b1a6d54cb9cae8` | `rename` | `shelf/runtime-1.15.2/pokemon-vendor-no-evolution` | Validated shelf |
| `feature/held-item-catalog-current-master-20260519` | `c4f36f6b3baf832cd087a62d8711fa9ff2b70348` | `rename` | `shelf/runtime-1.15.2/held-item-catalog` | Validated shelf |
| `feature/main_menu` | `747a415125489fe95d781ad8df394bc3e0a8e06b` | `rename` | `archive/prototype-1.11.1/main-menu` | Unrelated early history |
| `feature/map-asset-relinker-20260525` | `eb5c6cbaaf156ea37079008e7f0d6f2ce963a34a` | `rename` | `tool/1.15.2/map-asset-relinker` | Separate tooling lane |
| `feature/modern-qol-field-moves` | `8e3629473346e9d4f99b8af4780305bd5675fdbb` | `rename` | `archive/prototype-1.14.1/modern-qol-field-moves` | Superseded prototype |
| `feature/move_relearner` | `a1517e0c5453030885dfe00ecc70041f25edec1c` | `rename` | `archive/prototype-1.11.1/move-relearner` | Unrelated early history |
| `feature/new-map` | `a721e706051a724de8aa37c25796e977e5d4c812` | `rename` | `archive/prototype-1.14.1/map-region-fly` | Old map experiment |
| `feature/new-map-test-v15` | `1ac8244f79ed7fbca5f87fa25652eba0565a575b` | `rename` | `archive/prototype-1.15.1/rouge-cave-map` | Closed #4 |
| `feature/no-random-encounters` | `545989b69580d83b0d5ef4067561020ff82bb75e` | `rename` | `archive/prototype-1.15.2/no-random-encounters` | Superseded prototype |
| `feature/no-random-encounters-step-only` | `574ef6dd61f3e90c62027a9aee6a848be85a8258` | `rename` | `archive/prototype-1.15.2/no-random-step-only` | Superseded prototype |
| `feature/no-random-encounters-step-only-adopt-20260517` | `c4a02d94e20ad93f15642abf72e7d9426f026351` | `rename` | `archive/docs/20260517/no-random-adoption` | Merged Docs-only #34 |
| `feature/no-random-encounters-step-only-runtime-20260517` | `c9eba94c79d934900b3cd5588739c03fbd55517f` | `rename` | `shelf/runtime-1.15.2/no-random-step-only` | Validated shelf |
| `feature/party-select-ui` | `b5ef280867686f185e9897f977076a831fde55d4` | `rename` | `archive/prototype-1.14.1/party-select-ui` | Known-bug prototype |
| `feature/party-status-ui-overhaul-20260521` | `4a022554dc1d4bdf9250b0271b717981157723d7` | `rename` | `shelf/runtime-1.15.2/party-status-ui` | Validated shelf |
| `feature/pokemon-state-editor-expansion` | `d80e7a9b157f4a4d720cfcfbe5b8d43fc5221831` | `rename` | `shelf/runtime-1.15.2/pokemon-state-editor` | Validated shelf |
| `feature/poryscript` | `19d51e2e481cc357caab661cdca6a7cebd4f6ef0` | `rename` | `archive/prototype-1.11.1/poryscript` | Unrelated early history |
| `feature/prebattle-team-viewer` | `d597041bf9432e8cc67d143915ef828b9b139030` | `rename` | `shelf/runtime-1.15.2/prebattle-team-viewer` | Validated shelf |
| `feature/prebattle-team-viewer-phase2` | `e5f9f2a7c39849f5cd02523e31e34e6269e9dab8` | `rename` | `shelf/runtime-1.15.2/prebattle-team-viewer-phase2` | Validated shelf |
| `feature/qol_field_moves` | `30f1096b6dbc9e1999a6fe681a6b5ca5e1caf3f1` | `rename` | `archive/prototype-1.14.1/qol-field-moves` | Superseded prototype |
| `feature/releaseSystem` | `3d7dcd51ded8af115a5679a7d202890a0260d5c3` | `rename` | `archive/prototype-1.11.1/release-system` | Unrelated early history |
| `feature/sandbox_v12` | `9ecc7a8f81f64492de332507bb83f0a0053d2469` | `rename` | `archive/prototype-1.12.0/ability-ui-sandbox` | Old prototype |
| `feature/scout-selection-runtime-20260520` | `554849169cedd2604e6ad4544d3650d5d2eae5a7` | `rename` | `shelf/runtime-1.15.2/scout-selection` | Validated shelf |
| `feature/smart-gimmick-ai-16-20260604` | `e71c48c886be387047166a06e1edcb99e063abb6` | `defer-open-pr` | `shelf/runtime-1.16.1/smart-gimmick-ai` | PR #72; active dirty worktree |
| `feature/summary-tera-type-badge` | `bee3f54025b94f1156c3e44259720b883055c26f` | `rename` | `shelf/runtime-1.15.2/summary-tera-badge` | Validated shelf |
| `feature/tm-shop-migration` | `3209469da2e07d663136434dbec9b95da0be39a3` | `rename` | `shelf/runtime-1.15.2/tm-shop-migration` | Validated shelf |
| `feature/trainer-battle-aftercare-heal` | `7a255a4cbd0061d4c5d1625a25b36a18388799b1` | `rename` | `shelf/runtime-1.15.2/trainer-battle-aftercare` | Validated shelf |
| `feature/trainer-partygen-catalog-expansion` | `670d9297508f5a2c19990834538bec182e91425d` | `rename` | `shelf/runtime-1.15.2/trainer-partygen-catalog` | Validated shelf |
| `feature/unified-move-relearner` | `23a71463140ffcf7da1de783f1d803a9407e1db7` | `rename` | `shelf/runtime-1.15.2/unified-move-relearner` | Validated shelf |
| `fix/docs-validate-policy` | `c7a298933ac6d42acbbd2d5bfc737410b71a3462` | `rename` | `archive/workflow/20260506/docs-validate-policy` | Merged #6 |
| `integration/runtime-dev-16-20260531` | `125d8c4b52243c2f57569c12015aff5bde1d7be5` | `defer-open-pr` | `snapshot/runtime-1.16.0/full-stack-20260531` | PR #69 |
| `integration/runtime-dev-20260529` | `c3e50e9427dd6b0e82ee6fa285d20002c8672443` | `rename` | `snapshot/runtime-1.15.3/full-stack-20260529` | Closed #68 |
| `integration/runtime-lab-20260716` | `4e960aecea0185912e21a28f40e00fab1b388874` | `defer-worktree` | `snapshot/runtime-1.16.1/battle-lab-20260716` | Live worktree |
| `integration/runtime-lab-battle-team-pr-20260716` | `0f3a4da6480dc610d7b739587a103d7975870424` | `defer-worktree` | `archive/staging/runtime-1.16.1/battle-team-20260716` | Prunable worktree; merged #77 |
| `integration/runtime-lab-box-npc-pr-20260716` | `8f8bfa6bbade4aba167e43b6c0fd5345eebfca24` | `defer-worktree` | `archive/staging/runtime-1.16.1/box-npc-20260716` | Prunable worktree; merged #76 |
| `integration/runtime-lab-smart-ai-pr-20260716` | `703509f9d1cc761f70b754835c97b33ea078d626` | `defer-worktree` | `archive/staging/runtime-1.16.1/smart-ai-20260716` | Prunable worktree; merged #78 |
| `item_clock` | `a10d2c59d990148980c4cf6115e18c41fbb9c231` | `rename` | `archive/prototype-1.12.0/clock-key-item` | Old prototype |
| `item_heal_patry` | `4b34d278b7fc30c3f38aa864c5ff89a085b38f86` | `rename` | `archive/prototype-1.12.0/party-heal-item` | Old prototype |
| `item_keyfly` | `8267035eb37d9eea5a55a7de777013ca12bedee4` | `rename` | `archive/prototype-1.12.0/fly-key-item` | Old prototype |
| `master` | `9bd7ce7ff1a4202fff4fefc37e0b494fd84b9208` | `keep` | `master` | Default branch |
| `TM_v12_0` | `87d7ff7d56aea4b510880cb425517d0df527794d` | `rename` | `archive/prototype-1.12.0/gen9-tm-save` | Old prototype |
| `upgrade/1.15.2` | `6798f72e037d4af6d1a797835f3f3f11591f8c1c` | `rename` | `snapshot/upstream-1.15.2` | Exact upstream snapshot |
| `upgrade/master-before-1.16.0-sync-20260531` | `4e48ff993f8dc0e294bbe68c7dedb23ac680db2b` | `rename` | `snapshot/master-pre-1.16.0/20260531` | Recovery baseline |
| `vanilla/v11_1_1` | `cdab19a84caa9830c8438bed1bdf771f255b63e2` | `rename` | `snapshot/upstream-1.11.1-import` | Early imported root |
| `vanilla/v12_0_0` | `c120fa71dcdc02ba8bf7f0b9f4b5067339d3dd74` | `rename` | `snapshot/upstream-1.12.0` | Exact upstream snapshot |
| `vanilla/v13_3_3` | `cbcb7202860cac4807c6ef4ab02a4a2c00273a76` | `rename` | `snapshot/upstream-1.13.3` | Exact upstream snapshot |
| `vanilla/v14_1` | `15ede047f990e8eb1aed68ccba51c7b8a85d425c` | `rename` | `archive/baseline-1.14.1/local-init` | Upstream plus local init marker |

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
