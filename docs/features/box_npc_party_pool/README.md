# Box NPC Party Pool

Box NPC Party Pool is an implemented debug / test feature that lets an NPC
opponent use Pokemon copied from Pokemon Storage, starting with Box 1.

The first target is an isolated debug battle route: copy valid non-egg Pokemon
from Box 1 into a six-Pokemon candidate roster, choose three battle members for
singles or four for doubles, and battle without mutating the PC box. The feature
exists to make Smart Gimmick AI and future trainer AI tuning easier to test
against user-authored sets.

## Current Lineage

| Field | Value |
|---|---|
| Standalone source shelf | `feature/box-npc-party-pool-20260705` at `71e9d602f0`; draft PR #74 |
| Fresh integration target | `integration/runtime-lab-20260716`, created from `master` `1f77705e45` |
| Reapply candidate | `integration/runtime-lab-box-npc-pr-20260716` |
| Scope | Box NPC only; Battle Team Boxes and Smart Gimmick AI use later PRs |
| Master policy | Runtime files are not eligible for the docs / Lua-only master path |

- [Goal](goal.md)
- [Implementation](implementation.md)
- [Test Plan](test_plan.md)
