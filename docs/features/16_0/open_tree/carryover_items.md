# 16.0 Carryover Items

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Purpose | 1.15.3 で完了扱いにしないもの、16.0 / 15.x sync 後に再確認するもの |
| Previous complete tree | [15.3 CompleteTree](../../15_3/complete_tree/) |

この file は、まだ完了扱いにしない作業だけを置く場所です。
15.3 の実装済み feature と混ぜないため、16.0 側では `carryover` として扱います。

## Carryover List

| Priority | Item | Why it is open | Suggested 16.0 action |
|---|---|---|---|
| High | CI / workflow cleanup | Latest PR #68 CI still goes red because `build-firered` and `build-leafgreen` fail while `build-emerald` and docs validation pass. Local `all` / `debug` / `check` are green for the Emerald integration target. | Decide whether branch CI should only gate Emerald/local target for runtime-dev, or repair FRLG/test jobs for the full matrix. |
| High | Upstream 15.6 / 16.0 resync | Upstream may move master to a newer 15.x or 16.0 baseline, changing README, feature docs, configs, generated data, and build rules. | Rebase / replay #68 intentionally and update [upgrade_sync_policy.md](../upgrade_sync_policy.md) with conflicts. |
| Medium | Randomizer / EX lane | `feature/EX/ex-rz-upstream1` remains a large separate randomizer lane. | Re-audit after the new upstream baseline. Do not merge wholesale into 1.15.3 CompleteTree. |
| Medium | Map / Fly experiment data | Map Asset Relinker tool is complete, but Route301 / sample gameplay maps are not part of 1.15.3 runtime integration. | Start a dedicated 16.0 map content branch if actual map data should ship. |
| Medium | Bag Expansion | Still planning / investigation, not implemented in #68. | Revisit after Champions run / Field Kit pressure and new SaveBlock changes are known. |
| Medium | Future feature candidates | Jukebox, Weather Lab, Bounty Board, Field Notes, Route Mastery, Trainer Titles remain docs-only candidates. | Pick one candidate only after the new baseline is stable. |
| Low | Battle BGM asset policy | Runtime is user-confirmed on #68, but long-term asset permission / licensing review remains separate from implementation. | Keep provenance docs updated before public distribution. |
| Low | 1v2 double trainer battle | Config hook exists only as experiment requiring `B_TRAINER_BATTLE_SELECTION_SHORT_DOUBLE_MIN_COUNT == 1` and `OW_DOUBLE_APPROACH_WITH_ONE_MON == TRUE`. | Decide if 1v2 should ever be a supported rule; default remains 2 mons for double. |

## Runtime Smoke Closeout Notes

2026-05-31 user closeout confirmed:

| Area | #68 status |
|---|---|
| Battle BGM | User confirmed runtime behavior is OK. Remaining listening / licensing checks are not #68 blockers. |
| No Random Encounters | User confirmed integrated no-random behavior is OK. Fishing / Sweet Scent / Rock Smash / scripted wild remain out of MVP scope. |
| Trainer Battle Selection short-party behavior | User confirmed the short-party behavior is OK. Default double minimum remains 2; 1v2 is an optional experiment only. |

## CI / Workflow Cleanup Notes

Current observed run for #68:

| Job | Status | Note |
|---|---|---|
| docs_validate | Pass | SUMMARY markdown check passes. |
| release | Pending | Do not block on long Actions waits; use local validation until the run settles. |
| build-emerald | Pass | Emerald ROM builds in CI. |
| build-firered | Fail | Needs workflow / branch matrix triage. |
| build-leafgreen | Fail | Needs workflow / branch matrix triage. |
| test | Pending / previously red | Local `rtk make -j16 -O check` passes; CI environment / target should be investigated separately. |

Recommended first workflow decision:

1. Keep #68 as Emerald runtime integration candidate and document FRLG/test CI as non-blocking.
2. Add branch-specific workflow rules so runtime-dev only gates the relevant target.
3. Fix FRLG/test compatibility before any integration merge.

Option 1 is least disruptive for current 1.15.3 preservation. Option 2 is likely the most maintainable if runtime-dev remains an Emerald-first branch. Option 3 is highest rigor but may delay 16.0 migration.

## Recommended Order Before 16.0 Runtime Work

1. Decide #68 CI policy first.
2. Keep `master` docs / Lua-only and do not merge #68 runtime source there.
3. If a docs handoff to `master` is needed, cherry-pick only the version-tree docs needed for handoff, not the whole #68 docs state.
4. After `master` is updated to the next upstream baseline, create a fresh integration branch from that new `master`.
5. Replay / reapply #68 runtime feature slices intentionally, using [completed_features.md](../../15_3/complete_tree/completed_features.md) as the checklist.
6. Record every upstream conflict or changed assumption here before starting new runtime feature work.
