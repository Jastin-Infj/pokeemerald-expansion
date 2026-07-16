# Box NPC Party Pool Implementation

## Summary

The MVP implementation adds a standalone debug battle source from current
`master`. It does not depend on the Smart Gimmick AI branch.

Implemented runtime shape:

- New module: `include/box_npc_party_pool.h` and `src/box_npc_party_pool.c`.
- Box 1 is read-only and is used as the only source box.
- A six-Pokemon candidate roster is built from Box 1.
- Singles copy three final battle members into `gParties[B_TRAINER_OPPONENT_A]`.
- Doubles copy four final battle members into `gParties[B_TRAINER_OPPONENT_A]`.
- Copied opponent Pokemon are fully healed with `HealPokemon()`.
- Unused opponent slots are zeroed through `ZeroEnemyPartyMons()`.
- Candidate selection supports:
  - exact Box 1 slots 1-6.
  - first six valid Box 1 Pokemon.
  - random six valid Box 1 Pokemon.
- Final battle-member selection supports first-N and random-N from the candidate
  roster.
- Empty slots, eggs, and bad eggs are rejected via `CheckBoxMonSanityAt()`.
- Two dedicated RNG tags were added for candidate and final member selection.
- Party debug menu now has a `Box NPC Battle...` submenu with single and double
  routes.
- Debug routes use the current player party and require at least one usable
  Pokemon for singles or two usable Pokemon for doubles.
- Debug routes recalculate the current player party count before battle start,
  then recalculate the opponent party count after copying Box 1 members.
- Debug routes set a strong existing `master` AI flag preset and mark the battle
  as a debug battle.
- A narrow battle init hook applies pending opponent Tera / Dynamax permission
  after `gBattleStruct` exists and before first-turn gimmick assignment.
- The last selection result remains available through
  `BoxNpcPartyPool_GetLastResult()`.
- Debug builds emit an mGBA / AGBPrint `DebugPrintf` line containing mode,
  candidate slots, final slots, gimmick policy, and AI flag low bits.

## July 16, 2026 Reapply Handoff

The standalone source shelf remains
`feature/box-npc-party-pool-20260705` at `71e9d602f0`, with draft PR #74 kept as
review and validation evidence. The playable integration line starts fresh from
current `master` `1f77705e45`:

- Target: `integration/runtime-lab-20260716`.
- Box-only candidate: `integration/runtime-lab-box-npc-pr-20260716`.
- Reapplied source commits: `f9c2afde2c`, `535d07582c`, `c10009df04`,
  `9522cd2978`, and `71e9d602f0`.
- The pre-handoff reapply tree matched the standalone Box branch exactly.
- Scope is ten files: five Markdown files, one new header, one new source
  module, and narrow changes to `include/random.h`, `src/battle_main.c`, and
  `src/debug.c`.
- No save layout, Box mutation, normal trainer-party generation, Battle Team,
  or Smart AI change is part of this PR.

The GitHub repository currently reports no `master` branch protection and no
matching ruleset. Repository policy is therefore enforced by branch lineage,
PR base/head inspection, complete file-list review, and the docs / Lua-only
master gate. This runtime candidate targets only the dedicated integration
branch and must not be retargeted or merged into `master`.

Fresh reapply validation passed:

- `rtk make -j16 -O all`.
- `rtk make -j16 -O debug`.
- `rtk make -j16 -O check`.
- mGBA Live session `box-npc-reapply-20260716` booted a copied ROM/save, opened
  all six Box NPC routes, confirmed invalid fixed-slot rejection, filled the
  copied save's boxes, and started `Single slots 1-6`.
- Live `gPartiesCount` bytes were `[1, 3, 0, 0]`, confirming one player party
  member and three opponent members for the tested singles route.
- The session stopped cleanly and final managed status was `[]`.

## Not In This Slice

- Player party truncation to strict 3v3 / 4v4 is not implemented. The route uses
  the current player party.
- Smart Gimmick AI read-mode flags and battle action log export are not included
  because they are not on `master`.
- Player-side held item restore is not reimplemented here because the
  berry-inclusive fix already exists on the Battle Item Restore Policy shelves
  (`feature/battle-item-restore-current-master-20260519` /
  `feature/battle-item-restore-policy`). A later debug lab integration branch
  should combine that shelf with Box NPC Party Pool instead of duplicating the
  battle-end restore logic.
- Box preview UI is not included.
- Box ranges beyond Box 1 are not included.

## Handoff Notes

This branch is suitable as the standalone Box NPC Party Pool MVP. After merge,
the next integration step is to combine it with the Smart Gimmick AI branch and
map the debug route to read-mode / smart-gimmick AI presets plus battle action
logging.

## Validation

Completed on July 5, 2026:

- `rtk git diff --check`: passed.
- `rtk make -j16 -O debug`: passed.
- `rtk make -j16 -O all`: passed.
- `rtk make -j16 -O check`: passed.
- `rtk mdbook build docs`: passed with existing warnings for the missing
  top-level `CHANGELOG.md` include and `CREDITS.md` `</img>` tag.
- mGBA Live booted the rebuilt ROM and confirmed:
  - title screen and save continue menu.
  - overworld debug menu via `R+START`.
  - `Party -> Box NPC Battle...` entry.
  - Box NPC submenu entries for all six MVP routes.
  - fixed-slot route rejects the current save's invalid Box 1 slots with the
    concise field message `Box 1 slots 1-6 need valid Pokemon.`
- mGBA Live session `box-npc-party-pool-rerun` used debug
  `PC/Bag -> Fill -> Fill PC Boxes Fast`, then ran `Single slots 1-6`.
  The battle started against Trainer Debugger, and `gPartiesCount` readback
  showed player=1, opponentA=3, partner=0, opponentB=0.
- mGBA Live session `box-npc-party-pool-double-rerun2` used debug
  `PC/Bag -> Fill -> Fill PC Boxes Fast`, copied player party slot 0 to slot 1
  in-session with Lua to satisfy the current save's two-usable-Pokemon double
  guard, then ran `Double slots 1-6`. The battle started against Trainer
  Debugger, and `gPartiesCount` readback showed player=2, opponentA=4,
  partner=0, opponentB=0.
- mGBA Live sessions were stopped cleanly after validation, and final
  `status --all` returned `[]`.
