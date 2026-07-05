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
- Passed: mGBA Live session stopped cleanly.
- Manual follow-up: prepare six valid non-egg Pokemon in Box 1, then run each
  route above through an actual battle start.

## Behavioral Cases To Confirm

- Confirmed for current save invalid slots: fixed-slot route rejects before
  battle start.
- First-valid and random routes reject when Box 1 has fewer than six valid
  non-egg Pokemon: still requires manual Box setup.
- Singles create exactly three opponent party slots: still requires manual Box
  setup.
- Doubles create exactly four opponent party slots: still requires manual Box
  setup.
- Copied opponent Pokemon start with max HP, no non-volatile status, and restored
  PP: code path uses `HealPokemon()` after Box copy; still requires manual battle
  setup to observe in-game.
- Source Box 1 Pokemon are not healed, moved, consumed, removed, or overwritten:
  code path only reads Box storage and copies into `gParties`; still requires
  manual before/after storage inspection.
- DebugPrintf selection output contains the candidate source slots and final
  source slots: still requires a successful six-Pokemon Box route.
- Opponent Tera / Dynamax permission is available for copied final members when
  their items / species / config otherwise allow the gimmick: code path applies
  pending bits after `gBattleStruct` allocation; still requires a successful
  battle setup with eligible Pokemon.
