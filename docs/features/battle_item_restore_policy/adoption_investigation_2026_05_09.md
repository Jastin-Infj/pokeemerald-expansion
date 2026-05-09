# Battle-End Feature Adoption Investigation: 2026-05-09

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `6d0578c188`; `git describe` = `expansion/1.15.2-26-g6d0578c188` |
| Code status | Docs-only adoption investigation |
| Provenance | Local project overlay |

## Scope

This investigation reviews the next adoption candidates after the
No Random Encounters docs-only handoff. It does not copy runtime source into
`master`.

Primary inputs:

- PR #10: `feature/trainer-battle-aftercare-heal`
- `docs/features/battle_item_restore_policy/`
- `docs/features/trainer_battle_aftercare/`
- `docs/features/feature_registry.md`
- current `master` source around battle end and held item restore

## Current Master Findings

| Area | File / symbol | Current `master` behavior |
|---|---|---|
| Battle-end item restore call | `src/battle_main.c` `HandleEndTurn_FinishBattle` | Calls `TryRestoreHeldItems()` when `B_TRAINERS_KNOCK_OFF_ITEMS == TRUE` or `B_RESTORE_HELD_BATTLE_ITEMS >= GEN_9`. |
| Restore implementation | `src/battle_util.c` `TryRestoreHeldItems()` | Uses `itemLost[B_SIDE_PLAYER][slot].originalItem`, but excludes Berry pocket items from final restore. |
| Item transfer tracking | `src/battle_util.c` `TrySaveExchangedItem()` | Marks player original items as `stolen` only for regular trainer battles, excluding Frontier. |
| Battle-time item loss | `src/battle_script_commands.c` `Cmd_removeitem()` | Saves `usedHeldItem` for battle-time mechanics, but intentionally excludes popped Air Balloon and Corrosive Gas. |
| Natural Gift | `src/battle_move_resolution.c` `EFFECT_NATURAL_GIFT` | Consumes berry during battle, sets `canPickupItem`, stores `usedHeldItem`, and triggers Unburden. |
| Trainer battle end | `src/battle_setup.c` `CB2_EndTrainerBattle()` | Handles Sky Battle party restore first, then follower partner, early rival, secret base, forfeit, defeated, and normal win branches. |

## PR #10 Split

PR #10 is a staging PR, not a direct merge candidate. It contains two runtime
slices plus docs/tooling notes.

| Slice | Commit evidence | Runtime files | Adoption note |
|---|---|---|---|
| Trainer battle aftercare | `719867d846` | `include/config/battle.h`, `src/battle_setup.c` | Default-off heal-only hook. Source change is small, but focused tests are still mostly planned rather than implemented. |
| Battle item restore | `56a826708` plus later full-battle test commit `a93305b5` | `include/config/battle.h`, `src/battle_main.c`, `src/battle_util.c`, `test/battle_item_restore.c`, `test/battle/hold_effect/battle_item_restore.c` | Best next implementation candidate if user approves a feature/integration branch. Re-apply manually from current `master`; do not merge the old branch. |

## Recommended Adoption Order

1. Battle Item Restore Policy as a standalone feature branch.
2. Trainer Battle Aftercare heal-only hook after item restore is settled.
3. Champions partygen catalog / data PR after battle-end policy is fixed.
4. Battle selection / opponent preview investigations after partygen data is
   current.

Reasoning:

- Item restore is already focused around `TryRestoreHeldItems()` and has direct
  tests plus one full-battle Oran Berry test.
- Trainer aftercare changes `CB2_EndTrainerBattle()`, which is the same area
  future battle selection and forced release will need. It should have a small
  focused test gate before adoption.
- PR #7 is large tool/data/generated work and should wait until battle-end
  policy is not moving underneath it.

## Open Decisions Before Implementation

| Decision | Why it matters | Current recommendation |
|---|---|---|
| `B_RESTORE_HELD_BATTLE_BERRIES` default | PR #10 source uses `TRUE`, while `risks.md` says default-off is safer for partygen-owned trainers. | Decide explicitly before implementation. Use `TRUE` for playable/integration testing, `FALSE` for conservative baseline branches. |
| Battle type scope | Existing `TryRestoreHeldItems()` is called from battle end broadly when restore configs are enabled. Berry restore therefore affects wild, trainer, and facility paths unless guarded. | Preserve existing call breadth for the first slice, but document that this is competitive-style restore, not RPG item loss. |
| Final ownership after item transfer | Trick, Thief, Bestow, Symbiosis, Pickpocket, Magician, Corrosive Gas, and Air Balloon can make final ownership ambiguous. PR #10 restores original berries broadly from `itemLost`, not only ordinary HP berries. | Decide whether to accept broad competitive-style restore. If the desired slice is consumed-berry-only, add an explicit guard/state and focused tests before implementation. |
| Aftercare test gate | Heal-only source is default-off, but it changes the central trainer battle return path. | Add at least one focused test or mGBA scenario for normal win, excluded facility/secret-base/early-rival, and config-off behavior. |
| Cherry-pick strategy | PR #10 docs/source were authored before current docs-only `master` policy. | Create a fresh feature branch from current `master` and re-apply only the source/test slice requested by the user. |

## If User Approves The Item-Restore Branch

Do this on a new feature branch, not on `master`:

1. Start from current `master`.
2. Decide whether to keep PR #10's broad original-berry restore or narrow it to
   consumed-berry-only cases.
3. Re-apply only:
   - `include/config/battle.h`
   - `src/battle_main.c`
   - `src/battle_util.c`
   - `test/battle_item_restore.c`
   - `test/battle/hold_effect/battle_item_restore.c`
4. Do not copy older docs from PR #10 over current docs.
5. Run:
   - `rtk make -j16 -O all`
   - `rtk make -j16 -O debug`
   - `rtk make -j16 -O check TESTS=test/battle_item_restore.c`
   - `rtk make -j16 -O check TESTS=test/battle/hold_effect/battle_item_restore.c`
6. Attempt one focused mGBA Live boot/input check if MCP is available.
7. Update `implementation.md` and `test_plan.md` with the current branch,
   commit, local make results, mGBA/manual evidence, and any skipped Actions
   waits.

## What Not To Do

- Do not merge PR #10 directly into `master`.
- Do not copy `include/`, `src/`, `test/`, `tools/`, generated output, ROMs, or
  save artifacts into a docs-only branch.
- Do not mark trainer aftercare as shipped on `master`; its source is still
  branch-only.
