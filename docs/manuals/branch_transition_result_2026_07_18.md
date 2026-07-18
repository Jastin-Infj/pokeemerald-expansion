# Branch Transition Result 2026-07-18

## Document Metadata

| Field | Value |
|---|---|
| Completed | 2026-07-18 |
| Repository | `Jastin-Infj/pokeemerald-expansion` |
| Preflight | [Branch Inventory](branch_inventory_2026_07_18.md) and [Rename Manifest](branch_rename_manifest_2026_07_18.md) |
| Policy PR | PR #82, merged as `6acbe887a38c40aa997945ef8a01772b4fd3d4d3` |
| Stable upstream observed | official Release tag `expansion/1.16.2` at `ad0fd4d17f546ca6fd8d785c8724f9382e6e9382` |
| Result | Completed; no runtime implementation was merged into `master` |

This is the post-execution companion to the read-only inventory and preflight
manifest. The old names remain in those documents as provenance. This report
records the current names and final repository state.

## Final State

| Check | Result |
|---|---:|
| GitHub branches | 92 |
| Canonical `master` branches | 1 |
| Renamed branches | 91 |
| Missing manifest targets | 0 |
| Unexpected branches | 0 |
| Old remote names still present | 0 |
| Target SHA mismatches | 0 |
| Open pull requests | 0 |
| Runtime PRs merged into `master` | 0 |

`master` was not renamed or force-pushed. All 91 transitions used GitHub's
branch rename API, so each target retained the exact source commit. The branch
count returned to 92 after every batch and remained 92 after the final audit.

## Accepted Repository Policy

- `master` is the official-release intake baseline plus the local Markdown,
  workflow-only `AGENTS.md`, and approved Lua overlay.
- Only a non-draft, non-prerelease official GitHub Release tag named
  `expansion/<version>` is eligible for upstream intake.
- Moving `RHH/master` is comparison-only. Do not Sync Fork, merge, cherry-pick,
  or otherwise adopt it as the runtime baseline.
- Official release tags are canonical version identities. Branches are active
  work lines or retained evidence.
- Completed reusable implementations use `shelf/runtime-<version>/*`. Frozen
  combined states use `snapshot/runtime-<version>/*`.
- A future playable generation uses one
  `integration/active-runtime-<version>` created after the matching release
  intake reaches `master`.

## Runtime PR Finalization

The remaining implementation PRs were given a final comment containing the
fixed head SHA, disposition, integration evidence, new branch name, and the
explicit statement that the PR was not merged into `master`. They were then
closed before the corresponding branch rename.

| PR | Fixed head | Final disposition | Current branch |
|---|---|---|---|
| #75 | `ba98fed3829d3cb03c213d5b7af114782deba67f` | Complete; integrated through PR #77 | `shelf/runtime-1.16.1/battle-team-boxes` |
| #74 | `71e9d602f00660f31d662af4eee69f5680d566c0` | Complete; integrated through PR #76 | `shelf/runtime-1.16.1/box-npc-party-pool` |
| #72 | `e71c48c886be387047166a06e1edcb99e063abb6` | Complete; integrated through PR #78 | `shelf/runtime-1.16.1/smart-gimmick-ai` |
| #71 | `46774d096777d15be577c1d12dd617f69000312a` | Complete standalone shelf; integration pending | `shelf/runtime-1.16.1/champions-partygen` |
| #69 | `125d8c4b52243c2f57569c12015aff5bde1d7be5` | Completed 1.16.0 port; frozen comparison snapshot | `snapshot/runtime-1.16.0/full-stack-20260531` |

GitHub retains each closed PR's original head name in its historical metadata.
That does not mean the old remote ref still exists; the current refs are the
names in the table above.

## Current Runtime Evidence

| Branch | Role |
|---|---|
| `snapshot/runtime-1.16.1/battle-lab-20260716` | Frozen playable composition of Box NPC Party Pool, Battle Team Boxes, and Smart Gimmick AI at `4e960aecea0185912e21a28f40e00fab1b388874` |
| `shelf/runtime-1.16.1/box-npc-party-pool` | Standalone completed Box-authored NPC party implementation |
| `shelf/runtime-1.16.1/battle-team-boxes` | Standalone completed persistent Box Battle Team implementation |
| `shelf/runtime-1.16.1/smart-gimmick-ai` | Standalone completed Smart Gimmick AI implementation and planner backlog |
| `shelf/runtime-1.16.1/champions-partygen` | Standalone completed Champions PartyGen implementation; not yet in the battle-lab snapshot |
| `snapshot/runtime-1.16.0/full-stack-20260531` | Frozen 15.3-to-16.0 full-stack replay evidence |

There is no active 1.16.2 integration branch yet. The 1.16.1 battle lab and
1.16.0 full-stack snapshot are evidence sources, not permission to restore old
APIs or generated data over a newer official release contract.

## Worktrees And Local-Only Tips

Five registered worktrees pointed to directories that no longer existed. Their
absence was verified, `git worktree prune --dry-run --verbose` was reviewed,
and only the stale metadata was pruned. The live worktrees after transition
were:

| Worktree | Branch / state |
|---|---|
| `/home/jastin/dev/pokeemerald-expansion` | `shelf/runtime-1.16.1/smart-gimmick-ai` at `e71c48c886`; existing untracked files preserved unchanged |
| `/tmp/pokeemerald-runtime-lab-test` | `snapshot/runtime-1.16.1/battle-lab-20260716` at `4e960aecea` |
| `/tmp/pokeemerald-runtime-lab-docs` | Dedicated Docs-only handoff worktree |

Three old local branch names have unique tips that do not match the former
remote source SHA. They were deliberately preserved rather than overwritten:

| Local-only name | Local tip | Renamed remote evidence |
|---|---|---|
| `vanilla/v14_1` | `328ec9d708a2ba6afcfd646de8e3b65282ad1d1c` | `archive/baseline-1.14.1-plus28/local-init-20251207` at `15ede047f9` |
| `feature/trainer-partygen-catalog-expansion` | `f71efd0633b331f59cf7a5b9a5ce23422668be5d` | `shelf/runtime-1.15.2/trainer-partygen-catalog` at `670d929750` |
| `feature/ex-rz-upstream1` | `7770652825dfdc0b40280615abafb545bfaf60ab` | `archive/prototype-1.14.1/trainer-randomizer` at `fdd4c0340d` |

These are local recovery refs, not active GitHub feature branches. Audit their
unique commits separately before any later rename or deletion.

## Master Protection

GitHub branch protection now applies to `master`:

| Setting | Value |
|---|---|
| Changes through pull requests | Required |
| Required approving reviews | 0 |
| Required status check | `docs_validate` |
| Require branch up to date | No |
| Enforce for administrators | Yes |
| Force pushes | Blocked |
| Branch deletion | Blocked |
| Conversation resolution | Not required |
| Lock branch | Disabled |
| Fork syncing | Disabled |

This keeps a sole-owner Docs workflow usable while preventing accidental direct
history replacement or deletion. A future aggregate runtime gate should only
become required after that stable check actually exists.

## Verification

- GitHub's paginated branch API returned exactly the 92 expected names.
- Every renamed target matched its captured source SHA; every former remote
  name was absent.
- PR #82 was merged before its head was renamed to
  `archive/docs/20260718/branch-inventory` at `aa7bf7a3e47a`.
- PRs #69, #71, #72, #74, and #75 were closed without merge; the open PR query
  returned `[]`.
- Local target branches track the renamed `origin/*` refs.
- Renaming the checked-out Smart AI branch changed no tracked or untracked
  worktree content.
- `rtk mdbook build docs` passed with the existing missing root
  `CHANGELOG.md` include, `CREDITS.md` closing-tag, and large search-index
  warnings.
- The pull-request form of the local `docs_validate` SUMMARY check exited zero
  in warning-only mode. Its existing allowlist warnings were unchanged.

## Next Runtime Generation

When runtime development resumes, first intake the newest eligible official
Release tag through a dedicated `upgrade/<version>-intake-<date>` PR. After
that PR updates `master`, create `integration/active-runtime-<version>` fresh
from the updated baseline. Reapply selected shelves through separate PRs in a
documented order, adapting each feature to the release's current APIs, data
structures, save layout, config, and generation tools. Do not use the branch
transition itself as a reason to combine or merge runtime source into
`master`.
