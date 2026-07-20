# Battle Team Boxes Test Plan

## Cursor / Native-Icon Follow-Up Gate - July 20, 2026

| Check | Result |
|---|---|
| Branch | `fix/runtime-lab-battle-team-incomplete-20260718`; runtime candidate only |
| `rtk make -j16 -O all` | Pass; EWRAM 230,960 B (88.10%), IWRAM 28,384 B (86.62%), ROM 26,565,316 B (79.17%); existing linker RWX warning only |
| `rtk make -j16 -O debug` | Pass; EWRAM 230,952 B (88.10%), IWRAM 28,444 B (86.80%), ROM 26,613,216 B (79.31%); existing linker RWX warning only |
| `rtk make -j16 -O check` | Pass, full suite; existing expected-failure/crash harness cases remained expected |
| `rtk make -j16 -O check TESTS='Battle Team'` | Pass, 7/7 |
| Focused mGBA Live | Pass, low-speed interpolation session `battle-team-cursor-native-20260720` plus exact-final-ROM smoke session `battle-team-native-final-20260720` |
| Cleanup | Both accepted sessions returned `stopped: true`; final managed status `[]` |
| GitHub Actions | Not re-waited; local builds/check/runtime evidence are the handoff gate |

| Runtime assertion | Result |
|---|---|
| Team cursor intermediate frames | Right movement remained entirely in the tray; sampled cursor OAM X values were `0, 10, 15, 26, 32` rather than the old right-edge wrap. Up/down/left/right screenshots likewise kept the hand at X<80; `/tmp/battle-team-team-right-frame-1.png` through `-4.png`, `/tmp/battle-team-down-slot3-mid.png`, `/tmp/battle-team-left-slot2-mid.png`, `/tmp/battle-team-down-slot4-mid.png`, `/tmp/battle-team-up-slot2-mid.png` |
| Box/Team cursor independence | Box cell 16 (zero-based position 15) was captured before entering Team, Team focus moved to slot 5, and `SELECT` restored the same empty Box cell; `/tmp/battle-team-box-cursor-before.png`, `/tmp/battle-team-down-return-slot4-end.png`, `/tmp/battle-team-box-cursor-after.png` |
| Ordinary Box wrapping | Moving left from Box column 1 still exited at the left edge, reappeared at the right, and finished in column 6; `/tmp/battle-team-box-wrap-left-frame-1.png` through `-4.png`, `/tmp/battle-team-box-wrap-left-final.png` |
| Native Pokemon icons | The six occupied Team OBJ entries were regular non-affine 32x32 sprites centered at X=16/48 and Y=32/64/96. They visually matched the native Box icons, including on the final post-assertion ROM; `/tmp/battle-team-native-storage3.png`, `/tmp/battle-team-native-final-storage.png` |
| Full and mixed trays | Six occupied slots and one occupied/five empty slots both kept the 32x32 frames between the header and details; `/tmp/battle-team-native-storage3.png`, `/tmp/battle-team-native-mixed-empty.png` |
| Exact previews | Single First rendered exactly three native icons and Double First rendered exactly four; `/tmp/battle-team-native-preview-3.png`, `/tmp/battle-team-native-preview-4.png` |
| Wallpaper matrix | Cave, Sky, Machine, and Friends kept the header, tray frames/icons, lower detail, and upper-right Box badges disjoint; `/tmp/battle-team-native-wallpaper-cave-detail.png`, `/tmp/battle-team-native-wallpaper-sky.png`, `/tmp/battle-team-native-wallpaper-machine-final.png`, `/tmp/battle-team-native-wallpaper-friends-final.png` |

The run used 10 FPS specifically to expose six-step cursor interpolation rather
than checking only its final snap. Team 2 received one temporary registration
for the mixed occupied/empty case. Friends was unlocked only in live RAM at
`gSaveBlock1Ptr + 0x3CCE`, confirmed from the linked
`IsWaldaWallpaperUnlocked`; no in-game save was written.

## Final Review Gate - July 20, 2026

| Check | Result |
|---|---|
| Branch | `fix/runtime-lab-battle-team-incomplete-20260718`; runtime candidate only |
| `rtk make -j16 -O debug` | Pass; EWRAM 230,952 B (88.10%), IWRAM 28,444 B (86.80%), ROM 26,613,256 B (79.31%); existing linker RWX warning only |
| `rtk make -j16 -O all` | Pass; EWRAM 230,964 B (88.11%), IWRAM 28,384 B (86.62%), ROM 26,565,348 B (79.17%); existing linker RWX warning only |
| `rtk make -j16 -O check` | Pass, full suite; existing expected/known-failing annotations unchanged |
| `rtk proxy make -j16 -O check TESTS='Battle Team'` | Pass, 7/7 |
| Badge generated-data contract | Pass; generated PNG palette is 512 B, interface palette is 32 B, and their first 32 B compare identical; badge 4bpp is 448 B |
| `rtk git diff --check` / `rtk git diff --cached --check` | Pass |
| `rtk mdbook build docs` | Pass; existing missing root `CHANGELOG.md`, `CREDITS.md` closing-tag, and large-index warnings only |
| Focused mGBA Live | Pass, final rebuilt-debug-ROM session `battle-team-review-final-20260720` |
| Cleanup | `stop` returned `stopped: true`; final managed status `[]` |
| GitHub Actions | Not re-waited; local builds/check/runtime evidence are the handoff gate |

The focused suite is seven tests, not the historical 3/3 count in the July 19
gates below. New coverage includes exact player-side three/four staging and
full-party restoration, Double4 random uniqueness and source/result identity,
incomplete-team rejection, and pending gimmick apply/cancel/natural behavior.

| Runtime assertion | Result |
|---|---|
| Read-only party button | `PARTY POKEMON` opened four party icons and compact `PARTY 1 Incineroar` / `Lv50 USABLE` detail; `SELECT` returned to the Box; `/tmp/battle-team-review-party-open.png`, `/tmp/battle-team-review-party-select-return.png` |
| Tray and badge placement | Existing party-slot frames contained the six tray icons; badges stayed in Box-cell upper-right corners and details began below the tray; `/tmp/battle-team-review-final-storage.png` |
| Preview cancel | Single First showed exactly three icons and `B` returned to edit without staging; `/tmp/battle-team-review-final-single-preview.png`, `/tmp/battle-team-review-final-preview-cancel.png` |
| Exact Single staging | Live counts `[3,3,0,0]`; staging bytes `[1,3,4,0]` mean staged=true, staged count=3, saved logical count=4. Player slots 0-2 were populated and slots 3-5 were 100-byte zero records; `/tmp/battle-team-review-final-single-intro.png` |
| Single lossless return | A real turn changed the staged lead, then forfeit returned directly to the original field. Comparing the pre-battle and returned 600-byte party produced `byteDiffs=0`; counts returned to `[4,0,0,0]`, staging bytes cleared to zero, and the non-whiteout outcome was 5; `/tmp/battle-team-review-final-after-single.png` |
| Exact Double4 staging | Preview showed four icons; live counts `[4,4,0,0]`, staging bytes `[1,4,4,0]`, player slots 0-3 populated, and slots 4-5 zero; `/tmp/battle-team-review-final-double4-preview.png`, `/tmp/battle-team-review-final-double4-intro2.png` |
| Double4 lossless return | Battle Debug `Instant Win` completed the route; the returned 600-byte player party had `byteDiffs=0`, count 4, and cleared staging state |
| Wallpaper matrix | Cave, Sky, Machine, and Friends all retained the inward tray, lower details, and upper-right badges; `/tmp/battle-team-wallpaper-cave.png`, `/tmp/battle-team-wallpaper-sky.png`, `/tmp/battle-team-wallpaper-machine.png`, `/tmp/battle-team-wallpaper-friends.png` |

The Friends wallpaper was unlocked only in live RAM using the linked final-ROM
offset; no save was written. Double4 used Battle Debug `Instant Win` before the
second player action was selected, leaving controller bit 1 pending; the run
cleared that one debug-controller bit in live RAM so the already-selected win
could complete. This setup mutation did not touch party records, and the exact
600-byte before/after comparison owns the restoration result. All managed
sessions were stopped; final status was `[]`.

## Follow-Up Review Gate - July 19, 2026

| Check | Result |
|---|---|
| Branch | `fix/runtime-lab-battle-team-incomplete-20260718`; runtime candidate only |
| Required badge asset | `graphics/pokemon_storage/battle_team_badges.png` is tracked; no separate badge `.pal` exists |
| `rtk make -j16 -O debug` | Pass; EWRAM 230,348 B, IWRAM 28,444 B, ROM 26,610,780 B; existing linker RWX warning only |
| `rtk make -j16 -O all` | Pass; EWRAM 230,356 B, IWRAM 28,384 B, ROM 26,563,012 B; existing linker RWX warning only |
| `rtk make -j16 -O check` | Pass, full suite |
| `rtk make -j16 -O check TESTS='Battle Team'` | Pass, 3/3 |
| `rtk git diff --check` / `rtk git diff --cached --check` | Pass |
| `rtk mdbook build docs` | Pass; existing missing root `CHANGELOG.md`, `CREDITS.md` closing-tag, and large-index warnings only |
| Focused mGBA Live | Pass, `battle-team-summary-followup-20260719` |
| Cleanup | `stop` returned `stopped: true`; final managed status `[]` |
| GitHub Actions | Not re-waited; local builds/check/runtime evidence are the handoff gate |

The live-RAM fixture deliberately created the reviewed collision:

- Current Box 1 slot 6 contained Incineroar.
- Team 1 slot 1 referenced Dragonite at Box 2 slot 6.
- Team 1 slot 2 referenced Box 1 slot 1 so reorder had two valid members.

| Runtime assertion | Result |
|---|---|
| Tray source identity | `SLOT 1 Dragonite` and `B02-06 T1`; `/tmp/battle-team-summary-followup-slot1.png` |
| Same-slot current-Box decoy | Incineroar at `B01-06 T-`; `/tmp/battle-team-summary-followup-decoy-box1-slot6.png` |
| Summary identity | Opened Dragonite from Box 2, not Incineroar from the current Box; `/tmp/battle-team-summary-followup-cross-box-summary.png` |
| Read-only Summary | Down did not change Pokemon; Info-page `A` returned directly without Naming; Battle Moves `A` showed read-only move information only |
| Raw source integrity | Source checksum 370,441 and zero mismatches against its 80-byte template after Summary/move-info input |
| Raw decoy integrity | Decoy checksum 339,809 and zero mismatches against its 80-byte template after Summary/move-info input |
| Registry integrity | Team slots remained `[1,5]` and `[0,0]` after Summary and same-slot reorder |
| Same-slot reorder | Failure sound/message `Choose a different slot.`, no swap/update claim, then `B` cancelled; `/tmp/battle-team-summary-followup-reorder-same-slot.png` |
| Empty detail labels | Empty Box cell: `EMPTY BOX SLOT`; empty tray slot: numbered `SLOT 4` plus `EMPTY TEAM SLOT`; `/tmp/battle-team-summary-followup-empty-box.png`, `/tmp/battle-team-summary-followup-empty-team.png` |
| Non-slot detail | Box title left the compact detail area blank |
| Badge palette/resource | Fixed BG badges rendered through Box 1/Box 2 switching while reusing already-loaded BG palette 0; source has no badge palette load |

All fixture writes were confined to live emulator RAM; no in-game save was
written. The selected source and decoy were compared byte-for-byte after the
tested interactions. The session stopped cleanly and final status was `[]`.
The read-only Summary interaction remains an mGBA/manual UI regression rather
than a headless input test; the new mode's pointer, navigation lock, and
mutation locks are also covered by source audit, while the full suite guards
the surrounding registry and Storage behavior.

## Earlier Resource/ELF Review Gate - July 19, 2026

| Check | Result |
|---|---|
| Branch | `fix/runtime-lab-battle-team-incomplete-20260718`; runtime candidate only |
| `rtk make -j16 -O debug` | Pass; EWRAM 230,348 B, IWRAM 28,444 B, ROM 26,610,528 B; existing linker RWX warning only |
| `rtk make -j16 -O all` | Pass; EWRAM 230,356 B, IWRAM 28,384 B, ROM 26,562,788 B; existing linker RWX warning only |
| `rtk make -j16 -O check` | Pass, full suite |
| `rtk make -j16 -O check TESTS='Battle Team'` | Pass, 3/3 |
| `rtk mdbook build docs` | Pass; existing missing root `CHANGELOG.md`, `CREDITS.md` closing-tag, and large-index warnings only |
| Final mGBA Live | Pass, exact-final sessions `review-resource-final2-20260719` and `review-resource-final3-20260719` |
| Cleanup | `stop` returned `stopped: true`; final managed status `[]` |
| GitHub Actions | Not re-waited; local builds/check/runtime evidence are the handoff gate |

The final mGBA runs used only temporary live-RAM fixture mutations: Box 1 was
filled from one valid source record, registry slots referenced 18 distinct Box
positions, and the six party records were made usable. No in-game save was
written. One final2 Party attempt was discarded after dead-code removal shifted
EWRAM symbols and an old hardcoded party address produced Bad Eggs. That
session was stopped without saving; final3 restarted from the untouched save
and used addresses read from the final map. The accepted party and outcome
evidence below is from final3.

| Required runtime state | Result |
|---|---|
| Full Box + 18 distinct sources + all-full Lab | Pass; clean Box title/wallpaper, 46 active sprites, 18 free; `/tmp/review-resource-final2-all-teams.png` |
| Box switch before/after | Pass; Box 1 -> empty Box 2 -> Box 1 restored wallpaper, title, icons, and badges |
| Badge masks 1-7 + hand layering | Pass; all masks visible together, hand over mask 3; cursor subpriority 6, tray subpriorities all 12; `/tmp/review-resource-final2-masks-1-7.png` |
| Full Box + marking menu | Pass; 48 active sprites, 16 free; tile tags 13/14 and palette tags 56014/56015 loaded separately; `/tmp/review-resource-final2-marking-menu.png` |
| Move Items + six-member party | Pass; 49 active sprites, 15 free, two OBJ palette slots free; badges were removed for party display and restored afterward; `/tmp/review-resource-final3-move-items-party.png` |
| Battle Lab win return | Pass; returned to original field at `CB2_Overworld`, no pending field callback; `/tmp/review-resource-final3-field-return-win.png` |
| Battle Lab loss return | Pass; returned to original field at `CB2_Overworld`, no pending field callback; `/tmp/review-resource-final3-field-return-loss.png` |
| Battle Lab forfeit return | Pass; normal whiteout/heal dialogue returned to a responsive Pokemon Center field at `CB2_Overworld`; `/tmp/review-resource-final3-field-return-forfeit.png` |

All three returns read `gMain.callback2 = 0x081B9C41` (the Thumb entry for
final-ELF `CB2_Overworld` at `0x081B9C40`), `gFieldCallback = NULL`, and outcome
bytes 1, 2, and 9 respectively.

Static review also confirmed the BG1 allocation ends at relative tile `0x1FE`
before BG2 begins at physical `0x06008000`, badges consume no OBJ/OAM or OBJ
palette entries, the marking tag pairs have compile-time `base + 1`
assertions, all newly introduced resource loads are checked, and the old
text-only dialog manager has no remaining option, task, callback, window, or
buffer symbols. The unreachable nested registered-team debug submenu, configs,
and EWRAM label buffer were also absent from the final link. The owning
implementation document contains the complete BG-by-BG and OBJ-budget tables.

## Earlier In-Storage Battle Lab UX - July 19, 2026

| Check | Result |
|---|---|
| Branch | `fix/runtime-lab-battle-team-incomplete-20260718`; draft PR #84 |
| Diff scope before docs | Six tracked runtime/test files; unrelated untracked user files excluded |
| `rtk git diff --check` | Passed |
| `rtk make -j16 -O all` | Passed; existing linker RWX warning only |
| `rtk make -j16 -O debug` | Passed; EWRAM 88.08%, ROM 79.32% |
| `rtk make -j16 -O check` | Passed with exit code 0 |
| `rtk mdbook build docs` | Passed; existing missing `CHANGELOG.md`, `CREDITS.md` tag, and large-index warnings only |
| mGBA live editor/start route | Passed `battle-lab-ui-20260719b` |
| mGBA Summary return route | Passed `battle-lab-summary-20260719` |
| mGBA final hint/lock route | Passed `battle-lab-final-hints-20260719` |
| mGBA cleanup | All accepted sessions stopped; final status `[]` |

Automated coverage added in `test/battle_team.c`:

- A Box source shared by Teams 1 and 2 reports the combined team-number mask.
- Clearing one team's reference leaves the source locked by the other team;
  clearing the final reference unlocks it.
- Reordering swaps two saved Box references without changing either source and
  records the affected team as last viewed.
- The existing duplicate, stale-reference, invalid-source, full-roster,
  first/random selection, healing, and Box preservation tests still pass in the
  full suite.

mGBA Live confirmed on the 240x160 output:

- `R+START` showed `Battle Lab` directly in the top-level debug menu.
- The Lab opened Pokemon Storage itself: six empty Poke Ball slots at left,
  live Box Pokemon icons at right, and no intervening text manager.
- Six consecutive `A` registrations stayed in the same Box and preserved cursor
  position. Each registration immediately updated the six-slot tray and a
  numbered `1` badge on the source icon.
- Re-selecting an already registered source visibly reported its existing slot.
  Cross-team sharing remains covered by the registry test, and the final
  one-line diagnostic also states `Other TEAMS: OK`.
- Tray focus exposed `CHANGE`, `REMOVE`, `REORDER`, `SUMMARY`, and `CANCEL`.
  Replacement prompted after choosing a Box source; removal prompted before
  clearing; reorder visibly swapped the first two icons.
- `START -> SINGLE FIRST 3` displayed exactly three opponent icons. `B` returned
  to Box editing without closing Storage.
- `SINGLE RANDOM 3` displayed three icons and `R` changed the sampled set.
- `DOUBLE FIRST 4` displayed exactly four opponent icons, and `A` reached an
  actual double-battle command menu with four opposing party indicators.
- `SUMMARY` from the tray opened the standard Summary screen and `B` returned to
  the same tray slot with the correct nickname, level, item, and Box source.
- The final rebuilt ROM displayed `A:START B:EDIT R:REROLL` on one unclipped
  line and displayed `LOCKED by TEAM 1; remove there.` from a registered
  source's normal `MOVE POKéMON` context menu.
- The successful sessions used `~/.local/bin/mgba-qt`, which resolves to the
  required script-capable project build. The first explicit-binary attempt
  failed before ROM input with Qt `xcb` because it lacked `DISPLAY`; it was not
  accepted as runtime evidence.

The repeatable debug `Set Party` fixture provides one usable Pokemon. To reach
both preview gates, the validation run copied that already-valid party record
into four live RAM slots and set the live count to four. This was test setup
only: no repository file or persisted save was changed. The UI correctly
rejected the first 3v3 attempt before this setup with `needs 3 usable`.

Accepted remaining risk at that earlier handoff (closed or narrowed by the
final source/ELF review gate above):

- The visual run exercised Team 1 badges. Combined Team 1/2 masks and reorder
  source integrity are automated, but Teams 2/3 were not both filled visually.
- The final ROM visibly confirmed the exact lock reason. Prior evidence covers
  move/withdraw/release/destructive selection more broadly; this focused run did
  not replay every multi-move, daycare, and trade variant.
- Long GitHub Actions were not re-waited; local normal/debug builds, full check,
  and focused mGBA evidence are the handoff gate.

## Incomplete-Team UX Fix - July 18, 2026

| Check | Result |
|---|---|
| Frozen source | `snapshot/runtime-1.16.1/battle-lab-20260716` at `4e960aecea` |
| Mutable base | `integration/runtime-lab-current-1.16.1` |
| Fix branch | `fix/runtime-lab-battle-team-incomplete-20260718`; draft PR #84 |
| `rtk make -j16 -O check TESTS='Battle Team'` | Passed |
| `rtk make -j16 -O debug` | Passed; existing linker RWX warning only |
| `rtk make -j16 -O all` | Passed; existing linker RWX warning only |
| `rtk make -j16 -O check` | Passed |
| mGBA Live | Passed `battle-team-fix-20260718`; session stopped cleanly |

Automated coverage now proves that a team-local duplicate is rejected without
clearing the original slot, cross-team sharing remains allowed, the first
invalid position is reported, stale references still clear safely, and a full
six-source team still builds the healed NPC party without mutating Box data.

mGBA Live confirmed:

- The Box NPC battle menu displayed Team 1-3 counts without clipping.
- The manager list displayed Team 1-3 counts without clipping.
- Empty Team 1 startup displayed `TEAM 1: 0/6 valid.` and identified slot 1.
- Registering the Box 1 slot 1 Delcatty into Team 1 slot 2 displayed
  `Already in team slot 1.` and left slot 1 registered and slot 2 empty.
- Six distinct Box references produced `6/6` in the Box NPC battle menu.
- `Single first 3` started normally and used the registered slot 1 Delcatty as
  the opponent lead.
- The managed emulator session was stopped; final mGBA Live status was empty.

The required roster remains six valid references for both 3v3 singles and 4v4
doubles because those modes select three or four battle members from a
six-member candidate pool. Partial-team battle startup is intentionally not
enabled. Long GitHub Actions were not re-waited for this local handoff.

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
| `rtk make -j16 -O check TESTS='Battle Team'` | Passed, 2 tests |
| `rtk make -j16 -O check TESTS='Registered Battle Team'` | Passed, 1 test |
| `rtk make -j16 -O check TESTS='SaveBlock3'` | Passed, 1 test |
| `rtk make -j16 -O check` | Passed: 4,783 passed / 5,399 total; 12 existing known failures, 598 TODO, 6 expected failures |
| `rtk mdbook build docs` | Passed with existing missing include, closing-tag, and large-index warnings |

New tests in `test/battle_team.c` cover:

- Box-and-slot reference registration.
- Same source in multiple teams.
- Team-local duplicate rejection with original-slot preservation.
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

- Confirm first-N lead order and random-N uniqueness in repeated visible runs.
- Consume an opponent Berry and confirm the Box source item is unchanged after
  battle.
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
