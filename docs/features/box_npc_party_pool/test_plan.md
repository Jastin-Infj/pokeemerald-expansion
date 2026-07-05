# Box NPC Party Pool Test Plan

## Local Build Checks

- Passed: `rtk git diff --check`
- Passed: `rtk make -j16 -O debug`
- Passed: `rtk make -j16 -O all`
- Passed: `rtk make -j16 -O check`
- Passed with existing warnings: `rtk mdbook build docs`
  - existing warning: missing top-level `CHANGELOG.md` include.
  - existing warning: unexpected `</img>` in `CREDITS.md`.
  - existing warning: large search index.

## Runtime Checks

- Passed: mGBA Live started the rebuilt ROM with session
  `box-npc-party-pool-final`.
- Passed: title screen, continue menu, and overworld loaded.
- Passed: opened the overworld debug menu with `R+START`.
- Passed: confirmed `Party -> Box NPC Battle...` is reachable.
- Passed: confirmed the Box NPC submenu shows:
  - `Single slots 1-6`
  - `Single first valid`
  - `Single random`
  - `Double slots 1-6`
  - `Double first valid`
  - `Double random`
- Passed: with the current save's invalid Box 1 slots, `Single slots 1-6`
  rejects cleanly and shows `Box 1 slots 1-6 need valid Pokemon.`
- Passed: after filling Box 1 with
  `PC/Bag -> Fill -> Fill PC Boxes Fast`, `Single slots 1-6` started a trainer
  battle and `gPartiesCount` readback showed player=1, opponentA=3, partner=0,
  opponentB=0.
- Passed: after filling Box 1 and using an in-session Lua copy of player party
  slot 0 to slot 1 for the current save's double guard, `Double slots 1-6`
  started a trainer battle and `gPartiesCount` readback showed player=2,
  opponentA=4, partner=0, opponentB=0.
- Passed: mGBA Live validation sessions stopped cleanly; final `status --all`
  returned `[]`.

## GitHub Actions

- PR #74 check snapshot after push: `docs_validate` passed.
- PR #74 check snapshot after push: `build-emerald`, `build-firered`,
  `build-leafgreen`, `release`, and `test` were pending. They were not re-waited
  locally because the project handoff rule says not to block on long GitHub
  Actions runs.

## Behavioral Cases To Confirm

- Confirmed for current save invalid slots: fixed-slot route rejects before
  battle start.
- First-valid and random routes reject when Box 1 has fewer than six valid
  non-egg Pokemon: still requires manual Box setup.
- Confirmed with debug-filled Box 1: singles create exactly three opponent party
  slots.
- Confirmed with debug-filled Box 1: doubles create exactly four opponent party
  slots.
- Copied opponent Pokemon start with max HP, no non-volatile status, and restored
  PP: code path uses `HealPokemon()` after Box copy; still requires manual
  summary / memory inspection to observe each copied stat directly.
- Source Box 1 Pokemon are not healed, moved, consumed, removed, or overwritten:
  code path only reads Box storage and copies into `gParties`; still requires
  manual before/after storage inspection.
- Player-side held item / Berry restore is intentionally not revalidated in this
  standalone Box NPC branch. Use the existing Battle Item Restore Policy shelf
  (`feature/battle-item-restore-current-master-20260519` /
  `feature/battle-item-restore-policy`) when building the later integrated debug
  lab branch.
- DebugPrintf selection output contains the candidate source slots and final
  source slots: covered by debug-build code path, but the AGBPrint line was not
  separately exported in the mGBA sessions.
- Opponent Tera / Dynamax permission is available for copied final members when
  their items / species / config otherwise allow the gimmick: code path applies
  pending bits after `gBattleStruct` allocation; still requires an eligible
  Pokemon/item setup to observe the actual gimmick button in battle.
