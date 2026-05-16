# Unified Move Relearner MVP Plan

## MVP

- Add a guarded unified Move Relearner mode that can aggregate level-up, egg,
  current TM/HM, and current tutor candidates.
- Always allow all level-up moves from the selected level-up learnset when the
  unified mode is enabled.
- Reuse existing Summary, party menu, and NPC / event script entry points.
- Preserve source duplicates across level / virtual TM / tutor / Battle Tower
  sources, while excluding moves the Pokemon already knows.
- Add hard candidate cap handling before writing to `movesToLearn`.
- Avoid a single 250-300 entry vertical scroll list for Mew; add a chunked or
  tabbed navigation plan before implementation.
- Keep existing category-specific behavior available when unified mode is off.

## Non-Goals

- Do not add all Gen 1-9 physical TM/TR item IDs in the MVP. The expected TM
  range is now 250-300 candidate moves, but these should be virtual/generated
  relearner candidates rather than bag items.
- Do not expand bag / physical item save layout in the MVP. A compact virtual
  TM unlock bitset is acceptable if story gating is required.
- Do not require source badges in the first pass unless the UI needs them for
  usability.
- Do not rewrite the whole Move Relearner UI unless candidate count forces a
  source-tab fallback.

## Implementation Steps

| Step | Files | Notes |
|---|---|---|
| 1 | `include/config/summary_screen.h` | Add guarded unified config and source inclusion configs. |
| 2 | `include/constants/move_relearner.h` | Add a unified state or define a config-only route without shifting Summary mode ids. |
| 3 | `src/move_relearner.c` | Replace move-only candidate storage with move + source entries, cap guards, and unified candidate builder. |
| 4 | `src/move_relearner.c`, `src/menu_specialized.c` | Decide / implement source tabs or 50-60 move page chunks for broad candidate lists. |
| 5 | `src/pokemon_summary_screen.c` | Simplify prompt / L/R cycling when unified mode is active, or map L/R to tabs/chunks. |
| 6 | `src/party_menu.c` | Add a single party action or route existing source submenu to unified mode. |
| 7 | `data/scripts/move_relearner.inc` | Add an event path that can open unified mode directly while preserving existing category script. |
| 8 | `tools/learnset_helpers/` | Only if MVP includes Gen allow-list filtering: emit source metadata or generated per-generation pools. |
| 9 | `docs/features/unified_move_relearner/test_plan.md` | Record build, mGBA evidence, skipped Actions wait, and remaining manual checks. |

## Current Contract

- Unified off: existing level / egg / TM / tutor category behavior stays intact.
- Unified on: the active entry point opens one relearner list for all enabled
  sources.
- Level-up: use all levels up to `MAX_LEVEL`, regardless of the Pokemon's
  current level.
- Egg: use species egg move data, respecting the existing egg unlock policy
  unless configured otherwise.
- TM/HM: use a generated virtual TM candidate pool when enabled. Current
  physical machine registry can remain as a fallback/source, but the broad
  250-300 candidate set should not iterate bag items.
- Story reward unlocks: event / rank / clear flag may unlock a subset of the
  virtual TM pool directly, without granting a TM item.
- Source duplicates: if a move is legal through both virtual TM and tutor / tower,
  display both source entries. Existing-known filtering remains move-based, so a
  known move is hidden from every source.
- Broad list UX: Mew must be navigable without requiring a long single-list
  scroll. Initial candidates are 50-move chunks, 60-move chunks, or source tabs.
- Tutor: use `gTutorMoves[]`, currently generated from move tutor scripts and
  `extraTutors`.
- Cost: event scripts should charge only after successful learning, matching
  Fallarbor behavior.
- Cancel: cancel from list, give-up prompt, overwrite summary, and party/menu
  return paths must all leave `gSpecialVar_0x8004` and callbacks coherent.

## Future Work

- Historical Gen 1-9 machine/tutor allow-list configs.
- Source badges or source tabs for long candidate lists.
- Virtual TM ownership / unlock registry if story progression should gate the
  broad pool.
- 250-300 virtual TM data model: always available, generation-gated,
  rank/clear-flag/story-reward-gated, or mixed with current physical machines.
- PC box Summary route validation after party and field routes are stable.

## Open Questions

- Is "all Gen 1-9" required for MVP, or is current Gen 9 level-up plus current
  generated teachable/tutor data acceptable for the first implementation PR?
- Should a move with no physical TM item but historical TM legality appear as
  `TM`, `Virtual TM`, or `Other` in source metadata?
- Should duplicate source rows be adjacent under the same move name, or grouped
  by source tab / chunk?
- If story / rank gating is needed, should unlock state be move bitset, rank
  group bitset, generation group bitset, or a mixed metadata table?
- If unlock state is persistent, should the compact bitset live in SaveBlock1,
  SaveBlock2, or SaveBlock3?
- Should source-tab fallback be enabled automatically when candidate count
  exceeds one page / cap, or should it be a separate config?
- Should chunk size be 50 for round navigation, or 60 to match the old
  `MAX_RELEARNER_MOVES` mental model?
