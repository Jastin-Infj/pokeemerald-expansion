# Runtime Lab Integration

This integration line combines three separately reviewable runtime features into
one practical battle-debug ROM:

- Smart Gimmick AI supplies read-mode decisions, short-horizon risk handling,
  gimmick timing, debug opponents, and battle action logging.
- Box NPC Party Pool copies valid Box Pokemon into a healed, non-consuming NPC
  opponent party for 3v3 singles or 4v4 doubles.
- Battle Team Boxes supplies three persistent six-Pokemon source registries and
  routes those registered teams through the Box NPC battle builder.

The completed playable target is retained as
`snapshot/runtime-1.16.1/battle-lab-20260716` at merge commit `4e960aecea`
(formerly `integration/runtime-lab-20260716`). It was created from current
`master`, then received Box NPC and Battle Team through PRs #76 and #77. Smart
Gimmick AI entered through separate PR #78 after combined validation.

Runtime implementation does not merge into `master`. A later `master` handoff
may contain only eligible Markdown documentation, approved Lua scripts, and
`AGENTS.md` when workflow policy itself changes.

The corresponding `master` handoff is a separate branch and PR. It does not
reuse or retarget any runtime implementation PR.

- [Goal and Dependencies](goal.md)
- [Implementation and Handoff](implementation.md)
- [Test Plan](test_plan.md)
