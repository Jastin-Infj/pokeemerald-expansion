# Battle Team Boxes

Battle Team Boxes add three persistent six-member teams whose slots reference
Pokemon already stored in Pokemon Storage. A registered Pokemon stays in its
original Box slot and can be shared by more than one team.

The feature also extends Box NPC Party Pool so each registered team can be used
as the six-Pokemon opponent candidate roster for 3v3 singles and 4v4 doubles.

## Current Lineage

| Field | Value |
|---|---|
| Standalone source shelf | `feature/battle-team-boxes-20260716` at `ba98fed382`; draft PR #75 |
| Required base | Box NPC reapply PR #76 at integration target `17b67dfe98` |
| Frozen combined snapshot | `snapshot/runtime-1.16.1/battle-lab-20260716` at `4e960aecea` |
| Mutable integration base | `integration/runtime-lab-current-1.16.1` |
| Current fix candidate | `fix/runtime-lab-battle-team-incomplete-20260718`; draft PR #84 |
| Current behavior | Six valid references remain required; counts, missing-slot diagnostics, and duplicate-registration rejection are visible in the UI |
| Master policy | This runtime slice is not eligible for the docs / Lua-only master path |

- [Goal and Dependencies](goal.md)
- [Implementation](implementation.md)
- [Test Plan](test_plan.md)
