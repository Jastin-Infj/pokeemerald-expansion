# Runtime Lab Integration Implementation

## Lineage

- `master`: `1f77705e45ec2e7f45fc4079e6ed2847105ac613`.
- Box NPC source shelf: `feature/box-npc-party-pool-20260705` at
  `71e9d602f0`, draft PR #74.
- Box NPC integration: PR #76, merge commit `17b67dfe98`.
- Battle Team source shelf: `feature/battle-team-boxes-20260716` at
  `ba98fed382`, draft PR #75.
- Battle Team integration: PR #77, merge commit
  `b0ac9061ce9030a2b1447ff6ad00a2d1f4ecc337`.
- Smart source shelf: `feature/smart-gimmick-ai-16-20260604` at
  `e71c48c886`, draft PR #72.
- Smart integration candidate:
  `integration/runtime-lab-smart-ai-pr-20260716`, reapply commit
  `611933abf0`.
- Smart integration: PR #78, final candidate head `703509f9d1`, merge commit
  `4e960aecea0185912e21a28f40e00fab1b388874`.

The standalone PRs remain evidence shelves. The integration target is the
playable composition and is never a runtime PR to `master`.

## Smart Reapply

Smart Gimmick AI was squash-reapplied onto the target after PRs #76 and #77.
Its 76-file slice matched the standalone implementation except for deliberate
three-file integration resolution:

- `include/random.h` retains `RNG_AI_AGGRESSIVE_GIMMICK` and both
  `RNG_BOX_NPC_PARTY_POOL_*` tags.
- `src/battle_main.c` retains Smart's pending opponent gimmick-bit application
  and `BoxNpcPartyPool_ApplyPendingBattleInitPolicy()` at their separate
  lifecycle points.
- `src/debug.c` retains Battle Team management, Team 1-3 and legacy Box routes,
  Smart fixed 3v3/4v4 and gimmick battles, and the full Gauntlet submenu.

No Box/Battle Team save schema, storage lock, source coordinate, or party-copy
logic was rewritten as part of Smart reapply.

## Combined Runtime Surface

The Party debug menu now exposes both feature families in one ROM. Box routes
use current player Pokemon and a Box or registered-team NPC pool. Smart fixed
fixtures remain available for deterministic AI regressions. Gauntlet routes
remain available for read-mode, partner-tech, gimmick, champion, and elite
manual checks.

Battle Team source records remain references to Box coordinates. Box copies are
healed before use, held items are copied rather than consumed from storage, and
registered source records remain protected by the Pokemon Storage operation
guards. Smart decisions operate on the resulting battle party; they do not
mutate the Box sources.

## Impact Boundaries

The highest combined risks are:

- SaveBlock3 migration from the Battle Team registry.
- Pokemon Storage destructive-operation locks.
- two independent pending-gimmick initialization paths.
- the expanded Party debug menu and its route callbacks.
- Smart AI's broad battle decision and test surface.
- action-log tooling and schema compatibility.

Battle item restoration remains a separate shelf. This integration does not
claim that consumed player items are restored after every debug battle unless
the existing base runtime already provides that policy.

## Runtime Merge Handoff

Smart PR #78 merged into `integration/runtime-lab-20260716` after its base/head
SHAs, complete 83-file list, local validation, and mGBA Live evidence for both
feature families were reviewed; GitHub `docs_validate` passed. The runtime PR
was not retargeted or merged into `master`.

Fresh combined validation passed normal/debug ROM builds, full `make check`,
mdBook, registered Team 1 3v3 startup with party counts `[3,3,0,0]`, Smart Read
Single startup, and action-log v4 export. The mGBA Live session stopped cleanly.
Exact evidence and remaining user-acceptance scope are recorded in
[the integration test plan](test_plan.md).

## Master Handoff

`docs/runtime-lab-handoff-20260716` was created fresh from `master`
`1f77705e45`. It reapplies only 30 Markdown files and the two approved mGBA
validation Lua scripts. Source, headers, data, tests, shell/batch wrappers,
Rust tooling, graphics, ROMs, saves, and generated output are absent. This
branch owns a separate `master` PR and must pass a complete
`master..HEAD` file-list audit before merge.
