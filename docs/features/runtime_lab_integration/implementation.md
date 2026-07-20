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

The standalone PRs remain evidence shelves. The integration target is the
playable composition and is never a runtime PR to `master`.

## July 20 Battle-Side and UI Review Closure

The current fix candidate now applies Battle Lab's selected count to both
sides. Before battle startup it backs up all six player records plus the
recalculated logical count, stages exactly three or four usable Pokemon into a
zeroed party, and restores every byte through the debug-battle end callback.
Loss/forfeit returns bypass whiteout after restoration so the recovery healer
cannot mutate the backup.

The Storage surface also makes `PARTY POKEMON` a read-only party inspector,
documents `SELECT` as the Box/tray toggle, uses existing party-slot frames for
the six tray cells, and keeps scaled icons, details, and upper-right Box badges
inside their regions. Badge palette compatibility and the three-team mask/tile
relationship are checked explicitly, while one registry scan now builds all
visible Box masks.

Final validation passed normal/debug ROM builds, the full test suite, and seven
focused Battle Team tests. Live runtime evidence showed exact `[3,3,0,0]` and
`[4,4,0,0]` parties, zeroed unused slots, and zero differences after restoring
all 600 player-party bytes. It also covered preview cancel, the party inspector,
and Cave/Sky/Machine/Friends wallpaper contrast. The final managed mGBA session
stopped cleanly; long GitHub Actions were not re-waited.

## July 19 In-Storage Battle Lab Surface

The current fix candidate replaces the nested text-manager workflow with one
debug-first surface:

- `R+START -> Battle Lab` is a direct top-level route.
- Pokemon Storage stays open while Box sources are registered, changed,
  reordered, removed, or summarized through a six-icon tray.
- Box icons show fixed BG-tile team-number badges and protected sources expose
  an explicit multi-team lock reason. The seven masks use no OBJ sprites or OBJ
  palettes and stay below BG2's wallpaper character range.
- `START` chooses first/random 3v3 or 4v4, then displays the exact selected
  opponent icons. `A` starts, `B` edits, and `R` rerolls random selection.
- Player-side usable-count gates match three for 3v3 and four for 4v4; the
  July 20 closure additionally stages those exact counts and restores the full
  party after battle.
- The former Box 1 pool remains under `Party -> Box NPC Legacy...`.

This surface is implemented on
`fix/runtime-lab-battle-team-incomplete-20260718` / draft PR #84. It reuses the
existing Box NPC builder and pending gimmick policy; it does not alter Smart AI
move selection or the Battle Team save schema.

The follow-up review makes tray Summary an exact, read-only Box-reference view.
It receives the selected source pointer rather than indexing the current Box,
disables Pokemon navigation and mutation actions, and restores the same tray
slot on return. The badge sheet now explicitly reuses Storage BG palette 0 with
no dedicated palette asset/load; compact details distinguish numbered team
slots from Box slots, and same-slot reorder attempts report a retry message.

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

The debug menu exposes both feature families in one ROM. `Battle Lab` is the
primary Box/registered-team route; legacy Box 1 routes remain in Party. Smart
fixed fixtures stay available for deterministic AI regressions, and Gauntlet
routes remain available for read-mode, partner-tech, gimmick, champion, and
elite manual checks.

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

## Merge Handoff

The Smart candidate is staged as PR #78 into
`integration/runtime-lab-20260716`. Its base/head SHAs, complete 83-file list,
local validation, and mGBA Live evidence for both feature families were
reviewed; GitHub `docs_validate` passed. Do not use this candidate for a
docs-only `master` PR.

Fresh combined validation passed normal/debug ROM builds, full `make check`,
mdBook, registered Team 1 3v3 startup with party counts `[3,3,0,0]`, Smart Read
Single startup, and action-log v4 export. The mGBA Live session stopped cleanly.
Exact evidence and remaining user-acceptance scope are recorded in
[the integration test plan](test_plan.md).

The July 19 candidate also passed normal/debug builds, full `make check`, direct
Storage editing, 3/4-icon preview, random reroll, actual 4v4 startup, Summary
return, and clean mGBA session shutdown. Long GitHub Actions were not re-waited.

The final source/ELF remediation run additionally filled all 18 registered
sources, exercised masks 1-7, Box wallpaper switching, marking-menu and Move
Items/party pressure states, and measured 46, 48, and 49 active sprites in the
respective worst focused screens. Controlled win/loss outcomes returned to the
original field; forfeit completed the normal whiteout/heal path and returned to
a responsive Pokemon Center field. Exact-final sessions
`review-resource-final2-20260719` and `review-resource-final3-20260719`
stopped cleanly and final managed status was `[]`. The complete BG/OBJ resource
tables and screenshot paths are owned by the Battle Team implementation and
test plan.

The July 19 follow-up gate passed its then-current normal/debug builds, full
`make check`, and focused 3/3 Battle Team suite. Session
`battle-team-summary-followup-20260719` proved that a Box 2 slot 6 Dragonite
opens from the tray even when current Box 1 slot 6 is Incineroar, cannot be
navigated or mutated through Summary, and leaves both 80-byte records and saved
coordinates unchanged. It also confirmed the corrected empty-slot labels,
same-slot reorder message, shared-palette badge rendering, Box scrolling, and
clean session shutdown. Long GitHub Actions were not re-waited.
