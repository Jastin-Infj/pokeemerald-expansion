# Battle Team Boxes Implementation

## Summary

The implementation adds a persistent three-by-six Box reference registry, a
Pokemon Storage management UI, registered-source protection, and Box NPC debug
battle routes for each team.

## July 20, 2026 Cursor and Native-Icon Follow-Up

The follow-up sprite screenshot exposed two independent presentation bugs in
the integrated Storage screen:

- the generic Storage cursor animation treated every intermediate X position
  below 64 as an ordinary Box-edge wrap, so a Team target at X=16 or X=48
  briefly appeared at the right side of the normal Box; and
- occupied Team slots shared a `0xC0` affine matrix, shrinking only those
  Pokemon icons to 75% while an OAM-matrix allocation failure silently rendered
  them at their native size.

Cursor interpolation now recognizes a move whose source or destination is the
Team surface. Such a move bypasses only the ordinary left-edge X wrap and
clears horizontal-wrap, vertical-wrap, and flip-timer state before calculating
its six animation steps. Logical Box and Team cursor positions remain separate,
and ordinary Box-to-Box edge wrapping is unchanged.

Team Pokemon icons now use the same non-affine native 32x32 rendering path as
Box icons. The shared matrix field, allocation, scaling assignment, per-sprite
affine setup, and teardown were removed together. Their centers are X=16/48
and Y=32/64/96. The existing 32x24 filled/empty frame art is expanded to 32x32
by repeating its middle tile row, so no icon pixels are resampled. Compile-time
layout checks keep the first frame/icon at or below the 16-pixel header boundary
and the last frame/icon at or above the detail window boundary at Y=112.

Focused mGBA Live session `battle-team-cursor-native-20260720` confirmed all
four Team directions during intermediate animation frames. A right move sampled
cursor OAM X positions `0, 10, 15, 26, 32`, all inside the 64-pixel tray, while
a normal Box left-edge move still wrapped through the right side. Moving from a
saved Box cell to Team slot 5 and back restored the same Box cell. OAM entries
for the six occupied Team slots used regular 32x32 sprites at the intended
coordinates with no affine bit set.

The same rebuilt-ROM run covered a full six-member tray, a one-member/five-empty
mixed tray, exact three- and four-member previews, and Cave, Sky, Machine, and
Friends wallpapers. Header, frames, icons, details, and Box badges remained
disjoint. The one temporary Team 2 registration and Friends unlock flag existed
only in live RAM; no save was written. The session stopped cleanly and final
managed status was `[]`. After the final layout assertions were added, the
rebuilt debug ROM was booted again as `battle-team-native-final-20260720`; its
feature screen and six regular non-affine Team OAM entries matched the focused
run. That session also stopped cleanly with final status `[]`. Exact commands
and screenshot paths are in [the test plan](test_plan.md).

## July 20, 2026 Review Closure

The final review found that the selected opponent count was exact while the
player still entered 3v3 and 4v4 routes with every usable party member. Battle
Lab now stages an exact player party before leaving Storage:

- all six original `struct Pokemon` records and the logical party count are
  backed up;
- the first three or four alive, non-Egg records are copied into a zeroed
  six-slot battle party, so unused slots are empty and the cached count is
  exactly three or four;
- the backup is restored byte-for-byte from the dedicated debug-battle end
  callback on wins, losses, and forfeits; and
- a defeated staged route is converted to the non-whiteout return outcome after
  restoration. This prevents the normal recovery healer from changing restored
  HP, status, or PP.

The logical count is recalculated before backup because Pokemon Storage can
leave `gPartiesCount` stale even though the party records are valid. Direct
legacy/debug callers use the same staging API, and failed setup restores before
returning to the field.

The visible controls and layout now match their behavior:

- `PARTY POKEMON` opens the ordinary party icon surface as a read-only inspector;
  its compact detail reports party slot, nickname, level, and usable/fainted/Egg
  state, while `B` or `SELECT` returns to the Box.
- Box help advertises `SELECT:TRAY`, and tray help advertises `SELECT:BOX`.
- The six tray cells use the existing filled/empty party-slot frames in two
  columns. Pokemon icons render at their native 32x32 size without an affine
  matrix; the frames repeat their middle tile row to match that height.
- Tray cells and icons were moved inward, the compact details begin below the
  tray, and Box membership badges occupy the upper-right of Box cells instead
  of the icon/detail boundary.

Badge rendering now has explicit data contracts. The generated badge PNG
palette must contain the Storage interface palette as its first 16 colors; a
static size check plus debug assertion/release guard verifies this before badge
tiles are used. The 14 badge tiles are also tied at compile time to two tiles
for each nonzero three-team mask:
`14 == 2 * ((1 << BATTLE_TEAM_COUNT) - 1)`. Refresh builds all 30 Box-cell masks
by scanning the 18 registry records once, then expands the masks into badge
tiles, replacing the former 30-by-18 repeated lookup path.

The focused suite now contains seven Battle Team tests. It covers exact player
three/four staging and lossless six-record restoration, stale cached counts,
Double4 random-member uniqueness/source identity, incomplete-team rejection,
pending gimmick apply/cancel/natural behavior, batch team masks, shared-source
removal, and the existing registry/reorder/source-copy rules.

Final local validation passed normal/debug builds, the complete headless suite,
and focused `TESTS='Battle Team'` 7/7. mGBA Live confirmed the read-only party
button, tray/Box `SELECT` mnemonic, exact three- and four-icon previews, preview
cancel, `[3,3,0,0]` and `[4,4,0,0]` live party counts, empty unused player
slots, and zero byte differences across all 600 restored player-party bytes.
The same run exercised Cave, Sky, Machine, and Friends wallpapers with the tray
and badges visible. Full commands, sizes, RAM reads, screenshots, setup notes,
and cleanup evidence are recorded in [the test plan](test_plan.md).

## July 19, 2026 Follow-Up Review Remediation

The follow-up review found that tray Summary still inherited ordinary Box
navigation and mutation behavior. If a registered source was in another Box,
the Summary screen received the current Box base pointer plus the source's slot
index. That could display or rename the Pokemon occupying the same slot in the
current Box, and up/down could move away from the selected team member.

The corrected route passes the registered source's exact `BoxPokemon *` as a
single-record list and uses `SUMMARY_MODE_BOX_READ_ONLY`. That mode keeps the
full Summary page set while locking Pokemon navigation, rename, move
reordering, and move-relearner mutations. Returning from Summary restores the
same tray slot independently of `gLastViewedMonIndex`.

The same pass also completed the reviewed UI/resource cleanup:

- The required fixed badge PNG is tracked. It indexes the already-loaded
  Storage interface BG palette 0; there is no dedicated badge palette asset or
  runtime palette load.
- Occupied and empty tray details identify `SLOT 1` through `SLOT 6`. Empty Box
  cells say `EMPTY BOX SLOT`, empty tray cells say `EMPTY TEAM SLOT`, and Box
  title/buttons leave the compact detail area blank.
- Choosing the reorder source as its own destination now reports
  `Choose a different slot.` and remains in reorder mode instead of claiming a
  completed update.
- Cursor movement and completed Box scrolling each refresh the Battle Team
  panel once. Box changes still retain the fallback refresh for paths that skip
  display-mon refresh.

## July 19, 2026 Source/ELF Review Remediation

The final review pass replaced the two resource-unsafe parts of the first
in-storage implementation. The former 12x20 BG1 window began at relative tile
`0x200`, which is physical VRAM `0x06008000` and therefore the start of BG2's
wallpaper character data. The former per-Box OBJ badges could also raise the
worst visible sprite count to 66, beyond the GBA's 64-sprite table.

The corrected implementation has these properties:

- The left text allocation is a 9x2 `TEAM n x/6` header. The existing 9x7
  Pokemon data window supplies the two compact detail lines, so no large
  second text buffer is reserved.
- `graphics/pokemon_storage/battle_team_badges.png` is a fixed 112x8 indexed
  asset containing the seven 16x8 masks (`1`, `2`, `1/2`, `3`, `1/3`, `2/3`,
  `1/2/3`). Its pixel indices reuse the established Storage interface BG
  palette 0, which is loaded before badge initialization. No separate `.pal`
  asset or palette reload is required. The asset is repository-native pixel
  art; no runtime procedural graphics buffer is used.
- Badges replace two saved BG1 tilemap cells over each visible Box icon and
  restore those cells before party display, Box changes, or teardown. They use
  zero OBJ entries and zero OBJ palette slots.
- Tray Pokemon and empty-ball sprites use subpriority 12 while the hand cursor
  uses subpriority 6, so the hand remains in front of the tray. Badges use BG1
  priority 1: priority-1 OBJ renders above them, while they render above
  priority-2 Box icons and BG2.
- Marking-menu tile and palette tags now name both the base and implicit
  `base + 1` allocations, with compile-time consecutive-tag assertions. The
  empty-slot sheet/palette and BG badge tile load paths all check failure
  returns and avoid creating dependent sprites/overlays after failure.
- Removing a source recomputes its remaining team mask. The UI reports either
  that the Box Pokemon is unlocked or the exact other teams that still use it.
  A regression test covers the shared-Team-1/Team-2 removal sequence.
- The superseded text-only Battle Team dialog manager, its callbacks, task
  fields, EWRAM buffers, window templates, and route option were physically
  removed rather than left unreachable. A final link audit also removed the
  unreachable nested registered-team debug submenu, its copied label buffer,
  and its obsolete per-team battle configurations; the top-level Battle Lab
  and explicit legacy Box 1 submenu remain.

### BG tile/VRAM allocation

| BG | Existing ownership | Battle Team allocation | Boundary result |
| --- | --- | --- | --- |
| BG0, charbase 0, screenblock 29 | Message/item windows | None | Unchanged and independent of the Lab panel |
| BG1, charbase 1 (`0x06004000`), base tile `0x100`, screenblock 30 | Storage menu tiles use relative `0x100-0x18F` (`0x06006000-0x060071FF`) | Header `0x190-0x1A1` (`0x06007200-0x0600743F`); badge masks `0x1A2-0x1AF` (`0x06007440-0x060075FF`); existing detail window `0x1C0-0x1FE` (`0x06007800-0x06007FDF`) | Header, badges, and detail window are disjoint; relative tile `0x1FF` remains free |
| BG2, charbase 2 (`0x06008000`), screenblock 27 | Double-buffered Box wallpaper | None | The highest BG1 allocation ends below `0x06008000`; Box switching preserves both wallpapers |
| BG3, charbase 3 (`0x0600C000`), screenblock 31 | Scrolling Storage background | None | Unchanged |

Compile-time assertions enforce header-before-badge, badge-before-detail, and
detail-before-BG2 boundaries. The removed panel's physical start address was
exactly `0x06008000`, explaining the reviewed wallpaper corruption.

### OBJ budget

| Visible state | Active OBJ sprites | Margin below 64 | Evidence |
| --- | ---: | ---: | --- |
| Full Box, ordinary Storage | 40 | 24 | Static composition: base UI 10 + 30 Box icons |
| Full Box plus marking menu | 48 | 16 | Final mGBA measurement |
| Move Items plus six-member party display | 49 | 15 | Final mGBA measurement |
| All three Battle Teams full | 46 | 18 | Final mGBA measurement: base UI 10 + 30 Box icons + 6 tray icons |
| Battle Lab during the existing 37-icon scroll maximum | 53 | 11 | Static upper bound: base UI 10 + 37 Box icons + 6 tray icons |

The badge contribution in every row is zero sprites. The all-team runtime dump
also read cursor subpriority 6 and all six tray subpriorities as 12.

## July 19, 2026 In-Storage Battle Lab UX Rework

The July 18 diagnostics fixed misleading incomplete-team errors, but the
registration loop still left Pokemon Storage for a text-only manager after
every selection. The current fix branch now makes Pokemon Storage itself the
editing and Battle Lab surface:

- `BATTLE TEAMS` enters the normal live Box view with a 96-pixel left tray.
  The tray shows six ordered slots as real Pokemon icons, using the existing
  small Poke Ball graphic for empty slots. `SELECT` moves focus between the Box
  and tray without closing or reopening the screen, and `L/R` changes teams
  while the tray has focus.
- Pressing `A` on a valid Box Pokemon fills the first empty slot and leaves the
  Box cursor in place. A full team asks which slot to replace, then shows a
  `CHANGE` confirmation. A team-local duplicate is rejected with its existing
  slot number; sharing the same source across different teams remains valid.
- Occupied tray slots expose `CHANGE`, `REMOVE`, `REORDER`, and `SUMMARY`.
  Change and removal require confirmation, reorder swaps the two saved Box
  references, and Summary returns to the same tray slot with its source data
  restored.
- Every visible Box source registered to a team gets a compact fixed BG badge
  containing team number 1, 2, or 3. Multi-team masks render the applicable
  numbers together without consuming OBJ sprites or OBJ palettes.
- Normal Storage menus label a protected source `TEAM LOCK` and state every
  team that references it. Move, shift, withdraw, release, destructive PC
  selection, and multi-move pickup remain blocked until all references are
  removed.
- The debug menu now exposes `Battle Lab` directly in its top-level list. This
  opens the same Storage screen with `START` enabled. The legacy Box 1 routes
  remain under `Party -> Box NPC Legacy...`.
- `START` offers single first/random 3 and double first/random 4. A successful
  choice replaces the six-slot tray with the exact three or four opponent
  icons. `A` starts the prepared battle, `B` returns to editing, and `R`
  rebuilds only a random preview. The player party gate now requires three
  usable Pokemon for 3v3 and four for 4v4; the opponent candidate team still
  requires all six valid registered sources.

The layout was implemented against the GBA's 240x160 display. It reuses Pokemon
icon infrastructure and existing Storage/party graphics; the small team-number
badge sheet is a fixed indexed Storage asset.

## July 18, 2026 Incomplete-Team UX Fix

The combined runtime-lab snapshot exposed a misleading registration path: when
the same Box source was selected for another slot in one team, registration
silently cleared the old slot and moved the reference. A user could therefore
return to battle selection believing the latest registration had completed the
team, then receive only the generic `Battle Team needs six valid Pokemon.`
message. Selecting an empty team or leaving any slot incomplete produced the
same message.

The fix keeps the intended six-candidate requirement and changes the UI and
registration contract:

- Team-local duplicate registration is rejected. The original slot remains
  registered and the manager identifies its one-based slot number.
- The Battle Team manager list renders each team as `TEAM n  x/6`.
- `Party -> Box NPC Battle...` renders each battle entry as
  `Battle Team n  x/6` before entering its format submenu.
- Incomplete battle startup reports the selected team, valid count, and first
  empty or invalid slot, for example `TEAM 1: 5/6 valid.` and
  `SLOT 3 is empty or invalid.`
- New registry helpers find an existing source within one team and identify the
  first invalid slot. The save schema and registry version are unchanged.

Branch lineage for this fix:

- Frozen source snapshot: `snapshot/runtime-1.16.1/battle-lab-20260716` at
  `4e960aecea`.
- Mutable PR base: `integration/runtime-lab-current-1.16.1`, created at the
  same snapshot commit.
- Fix branch: `fix/runtime-lab-battle-team-incomplete-20260718`; draft PR #84.
- The fix is runtime implementation and must not target `master`.

## July 16, 2026 Runtime-Lab Reapply

- Integration target before this slice: `17b67dfe98`, produced by merged Box
  NPC reapply PR #76.
- Candidate: `integration/runtime-lab-battle-team-pr-20260716`.
- Source shelf: `feature/battle-team-boxes-20260716` at `ba98fed382`, draft
  PR #75.
- Reapplied commits: `993549034c`, `71518fec1d`, and `ba98fed382`.
- Runtime/source files match the standalone Battle Team shelf; the only
  additional differences from that shelf are newer Box integration handoff
  documentation.
- The reapplied Battle Team slice changes 19 files. The final PR changes 23
  files after adding four Box-side integration handoff documents. The
  highest-risk contracts are the SaveBlock3 increase from 4 to 84 bytes and the
  registered-source restrictions across Pokemon Storage destructive actions.
- This candidate does not contain Smart Gimmick AI and does not target
  `master`.

Fresh validation passed:

- `rtk make -j16 -O all`.
- `rtk make -j16 -O debug`.
- `rtk make -j16 -O check`.
- mGBA Live session `battle-team-reapply-20260716` continued the existing
  copied save, rendered the Battle Team list and empty six-slot grid, opened the
  register action and Box-only selector, returned to the same manager slot on
  cancel, rendered all four Team 1 battle modes, and rejected the incomplete
  team with `Battle Team needs six valid Pokemon.`.
- The session stopped cleanly and final managed status was `[]`.

## Runtime Changes

### Registry

- `include/battle_team.h` defines the saved schema and public API.
- `src/battle_team.c` owns initialization, validation, registration, stale
  cleanup, team-local duplicate rejection, and registered-source queries.
- `struct SaveBlock3` stores `struct BattleTeamRegistry` after existing fields.
- `ResetPokemonStorageSystem()` clears the registry for a new save.
- Magic/version validation lazily migrates old saves to three empty teams.

### Pokemon Storage

- The PC menu now includes `BATTLE TEAMS` between item movement and exit.
- Pokemon Storage itself shows the current two-column six-slot icon tray beside
  the live Box. Registration, replacement, team switching, reordering, removal,
  and summary all stay in this screen.
- Box icons carry fixed BG team-number badges, and the information panel shows
  the selected tray slot and nickname plus Box/slot coordinates and all team
  references. Box focus shows the selected nickname and the same compact source
  details; empty Box and team slots have distinct labels.
- Registered sources hide move, withdraw, shift, and release actions.
- Auto multi-move cannot begin on a registered source, and a selection that
  expands across one cannot be picked up.
- The daycare and trade PC-selection types, which may transfer a Pokemon out of
  storage, omit the select action for registered sources. Non-destructive move
  tutor, move deleter, and move relearner routes remain available.
- The existing Lotad/Seedot size check now has a distinct non-destructive
  selection type instead of sharing the trade selection type, so registered
  sources remain eligible for that check.
- Tray Summary is a read-only, single-source view that cannot navigate, rename,
  reorder moves, or enter the move relearner. Marking and held-item workflows
  remain unchanged.

### Box NPC Party Pool

- `BoxNpcPartyPoolConfig` now carries an optional `battleTeamId`.
- Candidate and final source metadata now use `{boxId, boxPosition}` so a roster
  may span multiple Boxes.
- `BOX_NPC_POOL_REGISTERED_BATTLE_TEAM` requires all six team slots to resolve.
- Copy, healing, random member selection, AI flags, and pending gimmick policy
  reuse the existing Box NPC implementation.
- Debug output logs candidate and final sources as `box:slot` pairs.

### Debug menu

The top-level debug menu now contains `Battle Lab`, which opens the integrated
Storage editor and four-mode preview/start surface. `Party -> Box NPC Legacy...`
keeps the six previous Box 1 routes without presenting them as the primary
workflow.

## Validation

Follow-up review gate on July 19:

- `rtk make -j16 -O debug`: passed; EWRAM 230,348 bytes (87.87%),
  IWRAM 28,444 bytes (86.80%), ROM 26,610,780 bytes (79.31%).
- `rtk make -j16 -O all`: passed; EWRAM 230,356 bytes (87.87%),
  IWRAM 28,384 bytes (86.62%), ROM 26,563,012 bytes (79.16%).
- Full `rtk make -j16 -O check` passed, and focused
  `TESTS='Battle Team'` passed 3/3. Both ROM builds retained only the existing
  linker RWX warning.
- `rtk git diff --check`, `rtk git diff --cached --check`, and
  `rtk mdbook build docs` passed. mdBook retained the documented missing root
  `CHANGELOG.md`, `CREDITS.md` closing-tag, and large search-index warnings.
- mGBA Live session `battle-team-summary-followup-20260719` used live RAM only
  to place an Incineroar decoy in Box 1 slot 6 and register a Dragonite source
  from Box 2 slot 6. Tray detail showed `SLOT 1 Dragonite` / `B02-06 T1`, and
  Summary displayed Dragonite rather than the current-Box decoy.
- Down input could not change the Summary Pokemon. Info-page `A` returned
  directly to the same tray slot without Naming, and Battle Moves `A` exposed
  only read-only move information. Exact 80-byte comparisons reported zero
  mismatches for both source and decoy after the route; registry coordinates
  remained `[1,5]` and `[0,0]`.
- The same session confirmed distinct empty Box/team labels, numbered empty
  team slots, a blank detail area on the Box title, Box 1/Box 2 scrolling,
  visible shared-palette badges, and the same-slot reorder failure message.
  Evidence: `/tmp/battle-team-summary-followup-decoy-box1-slot6.png`,
  `/tmp/battle-team-summary-followup-slot1.png`,
  `/tmp/battle-team-summary-followup-cross-box-summary.png`,
  `/tmp/battle-team-summary-followup-reorder-same-slot.png`,
  `/tmp/battle-team-summary-followup-empty-team.png`, and
  `/tmp/battle-team-summary-followup-empty-box.png`.
- The session stopped with `stopped: true`; final managed status was `[]`.
  Long GitHub Actions were not re-waited for this local handoff.

Earlier resource/ELF review gate on July 19:

- `rtk make -j16 -O debug`: passed; EWRAM 230,348 bytes (87.87%),
  IWRAM 28,444 bytes (86.80%), ROM 26,610,528 bytes (79.31%).
- `rtk make -j16 -O all`: passed; EWRAM 230,356 bytes (87.87%),
  IWRAM 28,384 bytes (86.62%), ROM 26,562,788 bytes (79.16%).
- Full `rtk make -j16 -O check`: passed. Focused
  `TESTS='Battle Team'` passed 3/3, including the new shared-reference removal
  regression. Both ROM builds retained only the existing linker RWX warning.
- `rtk mdbook build docs`: passed with the existing missing root
  `CHANGELOG.md`, `CREDITS.md` closing-tag, and large search-index warnings.
- Exact-final mGBA Live sessions `review-resource-final2-20260719` and
  `review-resource-final3-20260719` used the required script-capable wrapper
  and the rebuilt debug ROM after dead-menu removal. Live-RAM-only fixtures
  filled Box 1, all 18 distinct team sources, and the six-player party without
  saving those mutations.
- The final screen showed a clean Box title and wallpaper with the all-full
  Battle Lab (46 sprites), survived Box 1 -> Box 2 -> Box 1 switching, and
  rendered masks 1 through 7 simultaneously with the hand over mask 3.
  Screenshots: `/tmp/review-resource-final2-all-teams.png` and
  `/tmp/review-resource-final2-masks-1-7.png`.
- Normal Storage with the marking menu measured 48 sprites and loaded the
  explicit marking tags 13/14 and palette tags 56014/56015 in separate slots.
  Move Items with the full party measured 49 sprites and left two OBJ palette
  slots free. Screenshots: `/tmp/review-resource-final2-marking-menu.png` and
  `/tmp/review-resource-final3-move-items-party.png`.
- Controlled end-turn outcomes confirmed Battle Lab win and loss return to the
  original field. Forfeit followed the engine's normal whiteout/heal route and
  returned to a responsive Pokemon Center field. Each finished at
  `gMain.callback2 = 0x081B9C41` (`CB2_Overworld` Thumb entry) with
  `gFieldCallback = NULL`; outcome bytes were 1, 2, and 9 respectively.
  Screenshots: `/tmp/review-resource-final3-field-return-win.png`,
  `/tmp/review-resource-final3-field-return-loss.png`, and
  `/tmp/review-resource-final3-field-return-forfeit.png`.
- Both managed sessions stopped with `stopped: true`; final status was `[]`.
  Long GitHub Actions were not re-waited for this local review handoff.

July 19 in-storage UX validation in
`/home/jastin/dev/pokeemerald-expansion`:

- `rtk git diff --check`: passed.
- `rtk make -j16 -O all`: passed with the existing linker RWX warning.
- `rtk make -j16 -O debug`: passed with EWRAM 230,892/262,144 bytes
  (88.08%) and ROM 26,614,320/33,554,432 bytes (79.32%).
- Full `rtk make -j16 -O check`: passed with exit code 0; the new team-mask
  and reorder assertions passed, alongside the existing registry/party-copy
  coverage. Existing expected/known-failing test annotations were unchanged.
- mGBA Live session `battle-lab-ui-20260719b` used the required
  `~/.local/bin/mgba-qt` script-build wrapper. It confirmed the top-level
  `Battle Lab` entry, six direct registrations without a screen transition,
  live tray icons, numbered Box badges, duplicate diagnostics, `CHANGE` and
  `REMOVE` confirmations, reorder, 3-icon and 4-icon previews, `B` edit return,
  random `R` reroll, and `A` startup into an actual 4v4 double battle.
- The repeatable debug `Set Party` fixture provides one usable Pokemon. For
  preview validation only, the live emulator RAM cloned that valid party record
  into four slots; no repository file or persisted save was changed by this
  setup.
- After an initialization-order correction, session
  `battle-lab-summary-20260719` confirmed tray `SUMMARY` opens the standard
  Pokemon Summary and returns to the same tray slot with the correct Dragonite
  source details.
- Final-ROM session `battle-lab-final-hints-20260719` confirmed the single-line
  240x160 hints show `A:START B:EDIT R:REROLL` in full, and normal Storage shows
  `LOCKED by TEAM 1; remove there.` for the registered source. It also rechecked
  six same-screen registrations and random reroll after the final input guards.
- All accepted sessions stopped cleanly; final managed status was `[]`.
  An earlier explicit binary launch failed before ROM input with Qt `xcb`
  because `DISPLAY` was absent; retrying through the required wrapper resolved
  it.
- Long GitHub Actions were not re-waited for this local handoff.

July 18 fix validation in `/home/jastin/dev/pokeemerald-expansion`:

- `rtk git diff --check`: passed before documentation updates.
- `rtk make -j16 -O check TESTS='Battle Team'`: passed.
- `rtk make -j16 -O debug`: passed with the existing linker RWX warning.
- `rtk make -j16 -O all`: passed with the existing linker RWX warning.
- Full `rtk make -j16 -O check`: passed; the changed duplicate-rejection,
  stale-reference, and registered-party construction tests all passed.
- mGBA Live session `battle-team-fix-20260718` booted the rebuilt root ROM,
  continued the existing save, rendered `0/6` counts in both menus, displayed
  `TEAM 1: 0/6 valid.` plus `SLOT 1 is empty or invalid.`, rejected a duplicate
  Delcatty registration into slot 2 while preserving slot 1, completed Team 1
  with six distinct Box references, rendered `6/6`, and started
  `Single first 3` with Delcatty as the opponent lead.
- The mGBA session stopped cleanly and final managed status was empty.
- Long GitHub Actions were not waited before the local handoff.

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
- The final visual run filled all three teams and exercised every badge mask.
  It used live RAM fixtures rather than an in-game save/restart cycle, so the
  earlier Team 1 save/restart evidence still owns persistence validation.
- The exact normal-Storage `TEAM LOCK` reason was rechecked on the final ROM.
  Prior automated/manual evidence still owns the broader daycare, trade, and
  multi-move variants, which were not all replayed on July 19.
- Smart Gimmick AI and battle item restore are not part of this standalone
  shelf. Smart is composed by the dedicated runtime-lab candidate at
  `611933abf0`; battle item restore remains a separate previously implemented
  feature shelf rather than being reimplemented here.

## Merge Handoff

Draft PR #84 carries the July 18 diagnostics and July 19 in-storage UX rework
from `fix/runtime-lab-battle-team-incomplete-20260718` into
`integration/runtime-lab-current-1.16.1`. The frozen July 16 snapshot remains
unchanged for comparison. Neither implementation branch is eligible for a
docs-only `master` merge.

This implementation is staged as draft PR #75 and is not eligible for a
docs-only `master` merge. While Box NPC PR #74 remains open, review it as a
stacked PR with base `feature/box-npc-party-pool-20260705`. Do not merge either
PR automatically.

All seven required GitHub checks on the implementation snapshot passed:
Emerald, FireRed, LeafGreen, release, test, docs validation, and the aggregate
build check. Label and all-contributors jobs were skipped by workflow policy.

Battle Team reapply PR #77 subsequently merged into
`integration/runtime-lab-20260716` at `b0ac9061ce`. Smart Gimmick AI is layered
after it through a separate candidate and PR. See
[Runtime Lab Integration](../runtime_lab_integration/implementation.md) for the
combined dependency graph, conflict resolution, and validation evidence.
