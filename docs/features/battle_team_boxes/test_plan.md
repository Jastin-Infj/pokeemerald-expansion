# Battle Team Boxes Test Plan

## Automated Checks

| Check | Result |
| --- | --- |
| `rtk git diff --check` | Passed |
| `rtk make -j16 -O debug` | Passed |
| `rtk make -j16 -O all` | Passed |
| `rtk make -j16 -O check TESTS='Battle Team'` | Passed, 2 tests |
| `rtk make -j16 -O check TESTS='Registered Battle Team'` | Passed, 1 test |
| `rtk make -j16 -O check TESTS='SaveBlock3'` | Passed, 1 test |
| `rtk make -j16 -O check` | Passed: 4,783 passed / 5,399 total; 12 existing known failures, 598 TODO, 6 expected failures |
| `rtk mdbook build docs` | Passed with existing missing include, closing-tag, and large-index warnings |

New tests in `test/battle_team.c` cover:

- Box-and-slot reference registration.
- Same source in multiple teams.
- Team-local duplicate relocation.
- Empty, Egg, and out-of-range rejection.
- Stale reference cleanup after source deletion.
- Six sources distributed across multiple Boxes.
- 3v3 first-N opponent copy.
- Fully healed enemy copies.
- Source HP loss and held item preservation.

`test/save.c` records the intentional SaveBlock3 size change from 4 to 84 bytes.

## mGBA Live Checks

Passed with the debug ROM:

- Boot, continue, and overworld load.
- Open debug menu with `R+START`.
- Reach `Party -> Box NPC Battle...`.
- Reach `Manage Battle Teams`.
- Render Team 1-3 list without overlap.
- Render six slots, clear team, and back commands.
- Open empty-slot register/cancel modal.
- Enter the Box-only selector.
- Select a valid Box Pokemon and return to the same manager slot.
- Show species plus one-based Box/slot source coordinates.
- Open occupied-slot register/remove/cancel modal.
- Confirm registered source context menu omits move, withdraw, and release.
- Reach Battle Team 1's four single/double first/random routes.
- Reject incomplete team with a concise field message.
- Reach Battle Teams from the normal Pokemon Storage PC menu and return.
- Exit the direct manager without leaving dialogue graphics on the field.
- Reboot the final rebuilt ROM after the PC-selection guard, continue the
  existing save, and reach the overworld.
- Stop every managed session; final status is `[]`.

Setup failures retained as evidence:

- Direct script-build path without the wrapper failed with Qt `xcb` because
  `DISPLAY` was missing.
- Loading the ROM from `/tmp` opened mGBA's `Temporary file loaded` modal and
  prevented Lua bridge readiness.
- The successful route used `~/.local/bin/mgba-qt` and a copied ROM under the
  repository's ignored `.cache/mgba-live-roms/` directory.

## Manual Follow-Up

- Fill all six slots and run one registered-team single battle.
- Fill all six slots and run one registered-team double battle.
- Confirm first-N lead order and random-N uniqueness in repeated visible runs.
- Consume an opponent Berry and confirm the Box source item is unchanged after
  battle.
- Save, restart, and confirm all 18 references persist.
- Attempt shift, multi-move, withdraw, and release against sources referenced by
  one team and by multiple teams.
- Confirm registered sources cannot be selected through the daycare or trade
  PC-selection types, while move tutor/relearner selection remains available.
- Confirm a registered Lotad or Seedot remains selectable for the Sootopolis
  size check through the new non-destructive selection type.
- Remove the final reference and confirm movement actions become available.

The automated copy test covers battle-copy healing and source preservation, but
the full six-member battle and save/restart flows remain useful manual evidence.

## GitHub Actions

Draft PR #75 initial snapshot after push on July 16, 2026:

- `build-emerald`, `build-firered`, `build-leafgreen`, `release`, `test`, and
  `docs_validate` started and were still in progress at handoff.
- Label and all-contributors jobs reported skipped.

The long checks were not re-waited because they can take 20-30 minutes. Local
make and mGBA Live evidence are the primary handoff gate for this branch.
