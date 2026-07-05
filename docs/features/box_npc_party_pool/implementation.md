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

## Not In This Slice

- Player party truncation to strict 3v3 / 4v4 is not implemented. The route uses
  the current player party.
- Smart Gimmick AI read-mode flags and battle action log export are not included
  because they are not on `master`.
- Player-side held item restore remains a dependency for a later debug lab
  integration branch.
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
- mGBA Live session `box-npc-party-pool-battle` used debug
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
