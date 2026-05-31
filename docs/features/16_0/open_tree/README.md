# 16.0 OpenTree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Purpose | 1.15.3 で完了扱いにしないもの、16.0 で再開するもの |
| Previous complete tree | [15.3 CompleteTree](../../15_3/complete_tree/) |

## Open / Carryover Items

| Priority | Item | Why it is open | Suggested 16.0 action |
|---|---|---|---|
| High | CI / workflow cleanup | Latest PR #68 CI has `build-firered`, `build-leafgreen`, and `test` failures while `build-emerald`, docs validation, and release jobs pass. Local `all` / `debug` / `check` are green for the Emerald integration target. | Decide whether branch CI should only gate Emerald/local target for runtime-dev, or repair FRLG/test jobs for the full matrix. |
| High | Upstream 16.0 rebase / resync | 16.0 will change baseline source and docs, so 1.15.3 integration docs may become stale. | Create a fresh 16.0 integration branch, replay #68 intentionally, and update this OpenTree with conflicts. |
| Medium | Randomizer / EX lane | `feature/EX/ex-rz-upstream1` remains a large separate randomizer lane. | Re-audit after 16.0 baseline. Do not merge wholesale into 1.15.3 CompleteTree. |
| Medium | Map / Fly experiment data | Map Asset Relinker tool is complete, but Route301 / sample gameplay maps are not part of 1.15.3 runtime integration. | Start a dedicated 16.0 map content branch if actual map data should ship. |
| Medium | Bag Expansion | Still planning / investigation, not implemented in #68. | Revisit after Champions run / Field Kit pressure and 16.0 SaveBlock changes are known. |
| Medium | Future feature candidates | Jukebox, Weather Lab, Bounty Board, Field Notes, Route Mastery, Trainer Titles remain docs-only candidates. | Pick one candidate only after 16.0 baseline is stable. |
| Low | Battle BGM asset policy | Runtime works locally, but long-term asset permission / licensing review remains separate from implementation. | Keep provenance docs updated before public distribution. |
| Low | 1v2 double trainer battle | Config hook exists only as experiment requiring `B_TRAINER_BATTLE_SELECTION_SHORT_DOUBLE_MIN_COUNT == 1` and `OW_DOUBLE_APPROACH_WITH_ONE_MON == TRUE`. | Decide if 1v2 should ever be a supported rule; default remains 2 mons for double. |

## CI / Workflow Cleanup Notes

Current observed run for #68:

| Job | Status | Note |
|---|---|---|
| docs_validate | Pass | SUMMARY markdown check passes. |
| release | Pass | Release build job passes. |
| build-emerald | Pass | Emerald ROM builds in CI. |
| build-firered | Fail | Needs 16.0 workflow / branch matrix triage. |
| build-leafgreen | Fail | Needs 16.0 workflow / branch matrix triage. |
| test | Fail | Local `rtk make -j16 -O check` passes; CI environment / target should be investigated separately. |

Recommended first 16.0 workflow decision:

1. Keep #68 as Emerald runtime integration candidate and document FRLG/test CI as non-blocking, or
2. Add branch-specific workflow rules so runtime-dev only gates the relevant target, or
3. Fix FRLG/test compatibility before any integration merge.

Option 1 is the least disruptive for current 1.15.3 preservation. Option 2 is likely the most maintainable if runtime-dev remains an Emerald-first branch. Option 3 is highest rigor but may delay 16.0 migration.
