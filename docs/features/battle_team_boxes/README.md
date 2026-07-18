# Battle Team Boxes

Battle Team Boxes add three persistent six-member teams whose slots reference
Pokemon already stored in Pokemon Storage. A registered Pokemon stays in its
original Box slot and can be shared by more than one team.

The feature also extends Box NPC Party Pool so each registered team can be used
as the six-Pokemon opponent candidate roster for 3v3 singles and 4v4 doubles.

## Current Lineage

| Field | Value |
|---|---|
| Standalone source shelf | `shelf/runtime-1.16.1/battle-team-boxes` at `ba98fed382`; closed PR #75 |
| Required base | Box NPC reapply PR #76 at integration target `17b67dfe98` |
| Archived staging branch | `archive/staging/runtime-1.16.1/battle-team-20260716` |
| Runtime equality | Source files match the standalone Battle Team shelf |
| Master policy | This runtime slice is not eligible for the docs / Lua-only master path |

- [Goal and Dependencies](goal.md)
- [Implementation](implementation.md)
- [Test Plan](test_plan.md)
