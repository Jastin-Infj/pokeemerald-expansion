# Runtime Lab Integration

This integration line combines three separately reviewable runtime features into
one practical battle-debug ROM:

- Smart Gimmick AI supplies read-mode decisions, short-horizon risk handling,
  gimmick timing, debug opponents, and battle action logging.
- Box NPC Party Pool copies valid Box Pokemon into a healed, non-consuming NPC
  opponent party for 3v3 singles or 4v4 doubles.
- Battle Team Boxes supplies three persistent six-Pokemon source registries and
  routes those registered teams through the Box NPC battle builder.

The current UX fix candidate adds a top-level debug `Battle Lab` entry. Team
editing, exact 3/4-member preview, random reroll, and battle start now share one
Pokemon Storage screen. Fixed BG-tile masks identify multi-team Box sources
without consuming OBJ sprites or a dedicated palette, and tray Summary opens
the exact registered source read-only even when it lives outside the current
Box. The Party button is a read-only player-party inspector, and battle startup
stages exactly three or four player Pokemon before restoring the original six
records byte-for-byte on return. The old Box 1 pool is retained as a legacy
Party route.

The playable target is `integration/runtime-lab-20260716`. It was created from
current `master`, then received Box NPC and Battle Team through PRs #76 and #77.
Smart Gimmick AI is reapplied on a separate candidate branch and must enter the
target through its own PR after combined validation.

The in-storage Battle Lab fix is staged on
`fix/runtime-lab-battle-team-incomplete-20260718` / draft PR #84. It remains a
runtime candidate and does not change the `master` branch policy.

Runtime implementation does not merge into `master`. A later `master` handoff
may contain only eligible Markdown documentation, approved Lua scripts, and
`AGENTS.md` when workflow policy itself changes.

- [Goal and Dependencies](goal.md)
- [Implementation and Handoff](implementation.md)
- [Test Plan](test_plan.md)
