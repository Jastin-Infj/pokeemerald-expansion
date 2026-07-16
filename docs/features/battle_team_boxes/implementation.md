# Battle Team Boxes Implementation

## Summary

The implementation adds a persistent three-by-six Box reference registry, a
Pokemon Storage management UI, registered-source protection, and Box NPC debug
battle routes for each team.

## Runtime Changes

### Registry

- `include/battle_team.h` defines the saved schema and public API.
- `src/battle_team.c` owns initialization, validation, registration, stale
  cleanup, team-local uniqueness, and registered-source queries.
- `struct SaveBlock3` stores `struct BattleTeamRegistry` after existing fields.
- `ResetPokemonStorageSystem()` clears the registry for a new save.
- Magic/version validation lazily migrates old saves to three empty teams.

### Pokemon Storage

- The PC menu now includes `BATTLE TEAMS` between item movement and exit.
- The manager shows Team 1-3, then a two-column six-slot grid with clear/back
  commands.
- Empty and occupied slots expose register/remove actions as appropriate.
- Registration opens Pokemon Storage in a Box-only selection mode and returns to
  the same team and cursor after selection or cancel.
- Registered sources hide move, withdraw, shift, and release actions.
- Auto multi-move cannot begin on a registered source, and a selection that
  expands across one cannot be picked up.
- The daycare and trade PC-selection types, which may transfer a Pokemon out of
  storage, omit the select action for registered sources. Non-destructive move
  tutor, move deleter, and move relearner routes remain available.
- The existing Lotad/Seedot size check now has a distinct non-destructive
  selection type instead of sharing the trade selection type, so registered
  sources remain eligible for that check.
- Summary, marking, and held-item workflows are unchanged.

### Box NPC Party Pool

- `BoxNpcPartyPoolConfig` now carries an optional `battleTeamId`.
- Candidate and final source metadata now use `{boxId, boxPosition}` so a roster
  may span multiple Boxes.
- `BOX_NPC_POOL_REGISTERED_BATTLE_TEAM` requires all six team slots to resolve.
- Copy, healing, random member selection, AI flags, and pending gimmick policy
  reuse the existing Box NPC implementation.
- Debug output logs candidate and final sources as `box:slot` pairs.

### Debug menu

`Party -> Box NPC Battle...` now contains:

- `Manage Battle Teams`
- `Battle Team 1...`
- `Battle Team 2...`
- `Battle Team 3...`
- `Legacy Box 1 pool...`

Each registered team offers single first 3, single random 3, double first 4,
and double random 4. The legacy submenu keeps all six previous Box 1 routes.

## Validation

Completed July 16, 2026 in `/tmp/pokeemerald-battle-team-boxes-work`:

- `rtk make -j16 -O debug`: passed.
- `rtk make -j16 -O all`: passed.
- `rtk make -j16 -O check TESTS='Battle Team'`: two matching tests passed.
- `rtk make -j16 -O check TESTS='Registered Battle Team'`: one matching test
  passed, including multi-Box source logs.
- `rtk make -j16 -O check TESTS='SaveBlock3'`: passed after updating the
  intentional SaveBlock3 size baseline to 84.
- Full `rtk make -j16 -O check`: passed after the SaveBlock3 baseline update,
  with 4,783 passed of 5,399 total, 12 existing known failures, 598 TODO tests,
  and 6 expected failures.
- `rtk mdbook build docs`: passed with existing missing `CHANGELOG.md`,
  `CREDITS.md` closing-tag, and large search-index warnings.
- Existing linker warning remains: the ELF has a LOAD segment with RWX
  permissions.

mGBA Live confirmed:

- Debug ROM boot and existing-save continue.
- `R+START -> Party -> Box NPC Battle... -> Manage Battle Teams`.
- Team list, six-slot grid, register modal, Box-only selector, and return to the
  same slot after registration.
- Occupied slot exposes register/remove/cancel.
- Registered source context menu exposes summary/mark/cancel and omits move,
  withdraw, and release.
- The normal Pokemon Storage PC menu includes `BATTLE TEAMS`, enters the manager,
  and returns to the same PC menu item.
- Incomplete registered Team 1 rejects its debug battle route with
  `Battle Team needs six valid Pokemon.`
- Direct manager exit clears its dialogue window after the runtime-found fix.
- After adding the destructive PC-selection guard, the rebuilt ROM again
  reached the title screen, Continue menu, and overworld from the existing save.
- Team 1 was filled through the manager with six live Box references:
  Delcatty (`Box 1:1`), Spearow (`Box 1:2`), Vaporeon (`Box 1:4`),
  Zubat (`Box 1:3`), and two Pikachu (`Box 1:5-6`). The saved coordinates were
  `0,1,3,2,4,5`, preserving manager order independently of Box order.
- `Single first 3` started with Delcatty and exposed Spearow and Vaporeon as the
  remaining registered opponents. `Double first 4` started with Delcatty and
  Spearow as the opposing leads. The battles were ended with a controlled
  `gBattleOutcome = WIN` write only after the route and party evidence had been
  observed, avoiding an unrelated full battle playthrough.
- The first six Box records (480 bytes) and the full Battle Team registry
  (84 bytes) were byte-identical before and after the registered-team battles.
- An in-game save, clean emulator stop, new emulator process, and Continue
  restored the complete Team 1 grid and the exact 84-byte registry. The other
  twelve team slots remained empty as expected.
- Final mGBA Live status returned an empty session list after clean stop.

The first two mGBA attempts failed before feature input because a direct binary
launch lacked `DISPLAY`, then `/tmp` ROM loading opened mGBA's `Temporary file
loaded` modal before the Lua bridge could become ready. Copying the worktree ROM
to the repository's ignored `.cache/mgba-live-roms/` path and using the required
`~/.local/bin/mgba-qt` wrapper resolved both setup failures.

## Remaining Risks

- External scripts or tools that directly mutate storage can still move or
  erase a Box source without using the protected Pokemon Storage and PC
  selection routes. The next registry read safely clears that stale reference,
  but it does not follow the moved Pokemon.
- Team slots use Box coordinates, not immutable Pokemon identities. This matches
  the chosen save-efficient contract and UI lock policy.
- The manager uses species names, not nicknames, in its compact grid.
- Smart Gimmick AI and battle item restore are not integrated on this branch.
  Battle item restore remains available as a separate previously implemented
  feature shelf rather than being reimplemented here.

## Merge Handoff

This implementation is staged as draft PR #75 and is not eligible for a
docs-only `master` merge. While Box NPC PR #74 remains open, review it as a
stacked PR with base `feature/box-npc-party-pool-20260705`. Do not merge either
PR automatically.

All seven required GitHub checks on the implementation snapshot passed:
Emerald, FireRed, LeafGreen, release, test, docs validation, and the aggregate
build check. Label and all-contributors jobs were skipped by workflow policy.
