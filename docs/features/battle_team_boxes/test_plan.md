# Battle Team Boxes Test Plan

## September 7, 2026 preflight implementation and validation

Branch: `integration/team-box-ai-validation-20260907`, starting at
`72dc4574ff`, in `/home/jastin/dev/pokeemerald-validation-20260907`.

- Fresh `rtk make -j16 -O debug`: exit 0 (PNG metadata and linker RWX warnings).
- Fresh `rtk make -j16 -O all`: exit 0 (existing linker RWX warning).
- Fresh `rtk make -j16 -O check TESTS='Battle Team'`: 8/8 passed.
- Fresh `rtk make -j16 -O check TESTS='Battle AI trace'`: 5/5 passed.
- Fresh `rtk git diff --check`: passed.
- `rtk mdbook build docs`: exit 0; existing missing `CHANGELOG.md`,
  `CREDITS.md` closing-tag, and large search-index warnings remain.
- The Battle Team battle menu now builds the opponent preview before checking
  the player party, so the user can see the selected test roster first.
- An insufficient player party enters a dedicated preflight state with a
  one-line usable/required count and `A:PREP B:BACK` actions.
- `A:PREP` opens normal `MOVE POKéMON` storage mode; closing it returns to the
  same Team Manager preview and preserves the pending opponent selection.
- Direct mGBA session `team-box-validation-20260907` booted the new ROM and
  completed New Game. No existing user save was imported.
- Prepared Box Pokemon through `R+START -> PC/Bag -> Fill -> Fill PC Boxes Fast`.
- Opened the current `Battle Lab` debug entry and registered Box 1 slots 1-6
  into Team 1 through normal A-button input. The visible tray reached 6/6:
  Bulbasaur, Ivysaur, Venusaur, Charmander, Charmeleon, Charizard.
- `START -> SINGLE FIRST 3` rendered the opponent preview, then A entered the
  preflight state for the empty/insufficient player party.
- A opened the normal MOVE POKéMON screen; B -> YES returned to the same Team 1
  opponent preview. This route was observed on the rebuilt debug ROM before
  the final one-line message-width tightening; the final tightening is covered
  by the subsequent successful debug rebuild.
- A separate direct-party run reached the registered Team 1 battle, displayed
  the opponent intro, ended with a controlled `gBattleOutcome = WIN` write,
  and returned to the field. Team 1 remained 6/6 after that battle.
- The saved Team 1 registry was restored after a clean mGBA stop, new process,
  and Continue. The restarted session then cleared the player party and
  exercised the complete preparation route: `0/3` preflight, `A:PREP`,
  normal `MOVE POKéMON`, three unregistered Box sources placed into the
  party, close-box confirmation, the same Team 1 preview, and actual battle
  intro.
- While preparing the party, a registered source opened the `TEAM LOCK`
  context action and could not be moved. An unregistered source opened the
  normal `MOVE` action and could be placed into an empty party slot.
- The AI trace test compares matching snapshots, detects weather/timer/battler
  differences, and resolves a predicted/actual pair by plan ID.
- The random-member test fixes the structured RNG at `5` and verifies the
  without-replacement result `5,0,1,2` for Double 4.
- The item-isolation test clears the held item on the live opponent copy and
  verifies that the registered Box source still holds its Oran Berry.
- The mGBA session was stopped cleanly; final managed status was `[]`.

The new live-export field `ai_plans[].board_comparison` reports whether the
predicted-after and actual-after snapshots match and names any differences
(`weather`, `field`, `side`, `timers`, `battler_mask`, or `battler`).

## September 8, 2026 interaction-order checks

The following manual checks cover the interaction contract added after the
preflight route:

- Open an occupied Team slot and confirm the initial cursor is on `SUMMARY`.
- Confirm the occupied-slot order is `SUMMARY`, `CHANGE`, `REORDER`, `REMOVE`,
  `CANCEL`.
- Open an empty Team slot and confirm the only actions are `REGISTER` and
  `CANCEL`; no empty-slot reorder action is shown.
- Open a registered source from normal PC storage and confirm its order starts
  with `SUMMARY`, followed by `TEAM LOCK`, `MARK`, and `CANCEL`.
- Trigger Team replacement and removal confirmations and confirm `YES` is
  selected immediately. `A` accepts; `B` and `NO` cancel.
- Trigger release, put-away, exit-box, and continue-box confirmations and
  confirm they use the same `YES` initial cursor.
- Re-enter the Team battle-mode menu and confirm its existing order remains
  `SINGLE FIRST 3`, `SINGLE RANDOM 3`, `DOUBLE FIRST 4`, `DOUBLE RANDOM 4`,
  `CANCEL`: format is grouped first, deterministic selection precedes random,
  and cancel remains last.

### mGBA runtime result — September 8, 2026

The four interaction checks above passed on the rebuilt current ROM in the
managed session `ux-current-save-20260908`. The session used a fresh New Game,
the debug PC fill action, and normal A/B input; it did not depend on an older
save schema.

- Occupied Team slot: initial `SUMMARY`; order `SUMMARY`, `CHANGE`,
  `REORDER`, `REMOVE`, `CANCEL`.
- Empty Team slot: initial `REGISTER`; only `REGISTER`, `CANCEL`.
- Registered source from normal PC storage: `SUMMARY`, `TEAM LOCK`, `MARK`,
  `CANCEL`, with `SUMMARY` initially selected.
- Continue-box and Team removal confirmations opened with `YES` selected;
  `B`/`NO` remained the cancel path.

The session was stopped after the check and its managed status returned to an
empty list.

### Continuation runtime gate — September 8, 2026

The remaining live checks ran on the rebuilt ROM in the managed session
`ux-remaining-20260908`, starting from New Game. The session used the project
local script-capable mGBA path and was stopped cleanly; final managed status
was `[]`.

- `START -> SINGLE FIRST 3` rendered the three-member opponent preview before
  the player-party check. With no usable party, preflight showed
  `0/3 PARTY A:PREP B:BACK`.
- `A:PREP` opened the normal `MOVE POKEMON` storage route. Unregistered Box 2
  sources were moved into party slots, and closing storage returned to the
  same `TEAM 1 TEST` preview rather than rebuilding the pending selection.
- The fixed Team tray rendered native 32x32 Pokemon icons. A Box 2 source was
  also used to replace a full Team 1 slot; the manager showed `TEAM 1 6/6`
  and the source coordinate `B02-01`. This confirms cross-Box source
  replacement. The separate cross-Box read-only Summary screen was not
  captured in this pass.
- `Party -> Joint Trace Double` reached an active double battle. After
  confirming player-left `Tailwind` and player-right `Geomancy`, the turn was
  allowed to resolve and the live export was written to
  `/tmp/ux-remaining-20260908-live-export-turn0.json`.
- The export is `pokeemerald.battle_action_log.v5` with a valid trace
  signature, trace magic `41308` (`0xA15C`), version 5, one plan, eight
  candidates, three boards, and four action entries. The plan flags include
  `joint` and `deepest_complete_used`; the chosen candidate includes `joint`
  and `chosen`. Retained candidates include a Mega action and a switch to the
  Xerneas reserve, both opponent entries link to plan 1 / rank 0, and
  before/predicted-after/actual-after snapshots are present.
- `ai_plans[0].board_comparison` was available, but this run reported
  `matches: false` with `side`, `timers`, and `battler` differences. The live
  export gate is therefore closed for obtaining a v5 comparison sample, while
  exact predicted-board = actual-board equality remains an open follow-up.
- The temporary ROM and save were moved to `/tmp` after shutdown; no mGBA
  session or process remained.


## Integration Staging Gate - September 2, 2026

This gate stages Team Box PR #84 on
`integration/runtime-lab-current-1.16.1` at `4e960aecea`. The newer
RH Hideout-side implementation in `fc3835f5db` is the source of truth for
current Battle Team APIs, menu structure, and duplicate-registration behavior.
The integration correction is recorded in `404cbd192d`; it does not retarget
the work to `master`.

| Check | Result |
|---|---|
| PR #84 source commits | `83e76f3e83` and `fc3835f5db`; `97d44ca4c8` was empty against the selected base and was skipped |
| `rtk make -j16 -O debug` | Passed |
| `rtk make -j16 -O check TESTS='Battle Team'` | Passed, 7/7 |
| `rtk make -j16 -O check TESTS='Battle Team registered'` | Passed, 2/2 |
| `rtk make -j16 -O check TESTS='SaveBlock3'` | Passed, 1/1 |
| `rtk make -j16 -O all` | Passed; existing linker RWX warning only |
| `rtk make -j16 -O check` | Exit 0; existing `KNOWN_FAILING` / expected labels remain |
| `rtk mdbook build docs` | Exit 0; existing missing `CHANGELOG.md`, `CREDITS.md` closing-tag, and large-index warnings remain |
| mGBA Live | Session `team-box-runtime-20260901` reached the current Battle Lab UI and rendered the six-slot Team 1 grid; no prepared save was available for registration or battle. Final managed status was `[]`. |

This is staging evidence for the isolated integration branch. It does not
claim a master merge or a complete registered-team battle run from a fresh
save.

## Fresh Reapply Gate - July 16, 2026

| Check | Result |
|---|---|
| Base | Box NPC integration merge `17b67dfe98` from PR #76 |
| Source shelf | `feature/battle-team-boxes-20260716` at `ba98fed382`; PR #75 |
| Source comparison | Runtime/source files match the standalone Battle Team shelf |
| Diff scope | 23 files total: 19-file Battle Team slice plus 4 Box integration handoff docs |
| `rtk make -j16 -O all` | Passed; existing linker RWX and source PNG warnings only |
| `rtk make -j16 -O debug` | Passed; existing linker RWX warning only |
| `rtk make -j16 -O check` | Passed; existing test-runner format and linker RWX warnings only |
| mGBA Live | Passed `battle-team-reapply-20260716`; final status `[]` |

The fresh mGBA run used copied ROM/save files under `/tmp`. It confirmed old
save continuation, lazy empty-registry initialization, Team 1-3 list rendering,
the six-slot grid, register/cancel actions, Box-only selection, return to the
same manager slot, all four Team 1 battle modes, and incomplete-team rejection.
The exhaustive standalone evidence for six registrations, battle construction,
source-byte preservation, in-game save, process restart, and registry
persistence remains recorded below and in PR #75.

This is the second runtime-lab PR and targets only the dedicated integration
branch. It must not be retargeted to `master`.

## Automated Checks

| Check | Result |
| --- | --- |
| `rtk git diff --check` | Passed |
| `rtk make -j16 -O debug` | Passed |
| `rtk make -j16 -O all` | Passed |
| `rtk make -j16 -O check TESTS='Battle Team'` | Passed, 8 tests |
| `rtk make -j16 -O check TESTS='Battle AI trace'` | Passed, 5 tests |
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
- Deterministic random-N selection without duplicate members.
- Consumed opponent held-item isolation from the Box source.
- Predicted-after versus actual-after AI board comparison and plan pairing.

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
- Register all six Team 1 slots through the manager in the order Delcatty,
  Spearow, Vaporeon, Zubat, Pikachu, Pikachu, backed by Box positions
  `0,1,3,2,4,5`.
- Start `Single first 3` and observe Delcatty, Spearow, and Vaporeon in the
  opponent party.
- Start `Double first 4` and observe Delcatty and Spearow as the opposing leads.
- End each battle with a controlled `gBattleOutcome = WIN` write after roster
  evidence was visible; this shortens battle play only and does not bypass team
  construction.
- Compare the first six Box records before and after battle: all 480 bytes were
  unchanged.
- Compare the full saved registry before and after battle: all 84 bytes were
  unchanged.
- Save in game, stop mGBA, start a new process, Continue, and confirm the full
  six-member Team 1 grid and exact 84-byte registry were restored. The other 12
  slots remained empty.
- Stop every managed session; final status is `[]`.

Setup failures retained as evidence:

- Direct script-build path without the wrapper failed with Qt `xcb` because
  `DISPLAY` was missing.
- Loading the ROM from `/tmp` opened mGBA's `Temporary file loaded` modal and
  prevented Lua bridge readiness.
- The successful route used `~/.local/bin/mgba-qt` and a copied ROM under the
  repository's ignored `.cache/mgba-live-roms/` directory.

## Manual Follow-Up

- Repeat the live AI export with a board-comparison-matching fixture if exact
  predicted-after = actual-after equality is required. The v5 export and
  `ai_plans[].board_comparison` sample are now recorded above.
- Confirm first-N lead order and random-N rerolls in repeated visible runs.
- Consume an opponent Berry in a visible battle and confirm the Box source item
  is unchanged after battle.
- Optionally fill Teams 2 and 3 to exercise all 18 slots as non-empty references
  through a save/restart cycle. Runtime persistence of Team 1 and the complete
  18-slot registry has already been confirmed.
- Attempt shift, multi-move, withdraw, and release against sources referenced by
  one team and by multiple teams.
- Confirm registered sources cannot be selected through the daycare or trade
  PC-selection types, while move tutor/relearner selection remains available.
- Confirm a registered Lotad or Seedot remains selectable for the Sootopolis
  size check through the new non-destructive selection type.
- Remove the final reference and confirm movement actions become available.

The core six-member registration, deterministic single/double construction,
source preservation, and save/restart path now have both automated or memory
evidence and visible mGBA evidence. The remaining checks broaden destructive UI
route coverage and random/item behavior rather than gate the core feature.

## GitHub Actions

Draft PR #75 snapshot on July 16, 2026:

- Seven required checks passed: `build-emerald`, `build-firered`,
  `build-leafgreen`, `release`, `test`, `docs_validate`, and aggregate `build`.
- Label and all-contributors jobs reported skipped by workflow policy.
- The PR is open, draft, mergeable, and remains stacked on
  `feature/box-npc-party-pool-20260705`.
