# Runtime Lab Integration Goal

## Objective

Provide one reproducible debug environment where the user can construct an NPC
opponent from Box Pokemon, register reusable six-Pokemon Battle Teams, choose
3v3 singles or 4v4 doubles members, and exercise the Smart Gimmick AI against
those teams without weakening the independent feature shelves or changing the
`master` runtime baseline.

## Branch Contract

New standalone runtime features begin from current `master`. The integration
target also begins from current `master`. A dependent reapply candidate begins
from the current integration target only so its PR contains exactly the next
slice; it does not redefine the standalone feature's ancestry.

The required sequence is:

1. `master` `1f77705e45` -> `integration/runtime-lab-20260716`.
2. Box NPC Party Pool reapply -> PR #76 -> integration target.
3. Battle Team Boxes reapply -> PR #77 -> integration target.
4. Smart Gimmick AI reapply -> dedicated PR -> integration target.
5. Combined local build, battle tests, mGBA Live routes, logging, and PR scope
   audit.
6. Separate docs / Lua-only branch from the then-current `master` for handoff
   documentation. No runtime implementation is copied to that branch.

All runtime stages completed through PR #78 at integration merge commit
`4e960aecea`. The final handoff stage uses
`docs/runtime-lab-handoff-20260716`, created independently from `master`.

## Dependencies

Box NPC Party Pool owns opponent-party construction, full battle-copy healing,
member selection, dedicated RNG tags, AI flags, and pending gimmick permission.

Battle Team Boxes depends on Box NPC Party Pool. It adds persistent Box
coordinates, storage-operation locks, SaveBlock3 migration, manager UI, and a
registered-team pool mode. It must therefore be integrated after Box NPC.

Smart Gimmick AI is logically independent but shares three integration points:

- `include/random.h`: Smart AI and Box NPC require distinct RNG tags.
- `src/battle_main.c`: Smart trainer gimmick bits and Box NPC pending gimmick
  policy run at different initialization phases and both must survive.
- `src/debug.c`: Box/Battle Team and Smart/Gauntlet routes share the Party debug
  menu and must remain simultaneously reachable.

## Acceptance Criteria

- Normal and debug ROMs build from the combined branch.
- Full `make check` exits successfully, including Smart AI and SaveBlock3 / Box
  tests.
- `Party -> Box NPC Battle...` still exposes team management, Team 1-3 modes,
  and all legacy Box 1 modes.
- Smart fixed battles and `Gauntlet Battles -> Read Single/Double` remain
  reachable in the same ROM.
- A Box or registered-team battle starts with the requested 3/4 opponent count,
  and Smart/read AI flags can be applied without corrupting party sources.
- Battle action logging remains exportable for practical AI review.
- mGBA Live sessions stop cleanly, or exact stale-session cleanup state is
  recorded.
- The candidate PR targets only the integration branch and its complete file
  list is reviewed before merge.
- `master` receives no source, data, graphics, generated, or non-approved tool
  implementation from this line.

## Non-Goals

- Merging runtime implementation into `master`.
- Collapsing the three standalone evidence shelves into one historical branch.
- Reimplementing battle-item restoration or Pokemon State Editor features.
- Claiming the deferred full two-to-three-turn AI search is complete.
