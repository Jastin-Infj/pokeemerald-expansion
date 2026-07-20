# Runtime Lab Integration Test Plan

## July 20, 2026 Exact-Team Review Gate

- Final `rtk make -j16 -O debug` and `rtk make -j16 -O all` passed with only
  the existing linker RWX warning. Full `rtk make -j16 -O check` passed, and
  focused `TESTS='Battle Team'` passed 7/7.
- The focused suite now covers exact player three/four staging, all-six
  byte-for-byte restoration from stale and valid cached counts, Double4 random
  uniqueness/source identity, incomplete-team failure, and pending gimmick
  apply/cancel/natural behavior in addition to the registry tests.
- Final session `battle-team-review-final-20260720` showed exact live counts
  `[3,3,0,0]` and `[4,4,0,0]`. Single zeroed player slots 3-5; Double4 zeroed
  slots 4-5. The saved logical count was four in both routes.
- A mutated Single route forfeited without entering whiteout/healing. Single
  and Double4 each returned with count four, cleared staging state, and zero
  differences across the 600-byte player party compared with its own
  pre-battle snapshot.
- The same rebuilt ROM confirmed the read-only `PARTY POKEMON` inspector,
  `SELECT` Box/tray return, exact three/four-icon previews, `B` preview cancel,
  existing slot frames, inward tray placement, lower details, and upper-right
  badges on Cave, Sky, Machine, and live-RAM-unlocked Friends wallpapers.
- Badge generated data retained a byte-identical 32-byte interface-palette
  prefix. Static assertions cover palette size, tile count versus team-mask
  count, and BG allocation boundaries; mask refresh scans the 18 registry
  records once for all 30 Box cells.
- Friends unlock and the remaining second-player controller bit after an early
  Double Battle Debug `Instant Win` were temporary live-RAM setup only; neither
  touched party records or persistent save data.
- The session used the required script-capable wrapper. `stop` returned
  `stopped: true`, final managed status was `[]`, and long GitHub Actions were
  not re-waited. Exact screenshot paths and build sizes are in the owning
  Battle Team test plan.

## July 19, 2026 Cross-Box Summary Follow-Up

- Current debug and normal ROM builds passed with only the existing linker RWX
  warning. Full `rtk make -j16 -O check` passed, and focused
  `TESTS='Battle Team'` passed 3/3.
- Session `battle-team-summary-followup-20260719` kept Box 1 current, placed an
  Incineroar decoy at Box 1 slot 6, and registered a Dragonite source from Box 2
  slot 6. Tray `SUMMARY` displayed Dragonite, ignored Pokemon navigation, did
  not expose Naming or move mutations, and returned to the same tray slot.
- Exact post-route comparisons found zero mismatches in either 80-byte Box
  record. Team source coordinates remained `[1,5]` and `[0,0]` after Summary
  and the rejected same-slot reorder attempt.
- The same run confirmed shared-palette BG badges, Box 1/Box 2 scrolling,
  numbered occupied/empty tray details, distinct `EMPTY BOX SLOT` and
  `EMPTY TEAM SLOT` labels, a blank Box-title detail area, and
  `Choose a different slot.` for a reorder source selected as its destination.
- All fixture mutations were live RAM only. `stop` returned `stopped: true`,
  final managed status was `[]`, and long GitHub Actions were not re-waited.
- Screenshot paths and exact build sizes are recorded in the owning Battle Team
  implementation and test plan.

## July 19, 2026 Final Resource and Return Validation

- Final builds passed: `rtk make -j16 -O all`,
  `rtk make -j16 -O debug`, full `rtk make -j16 -O check`, and focused
  `TESTS='Battle Team'` (3/3). `rtk mdbook build docs` also passed with its
  existing documented warnings. Only the existing linker RWX warning remained
  in the ROM builds.
- Exact-final sessions `review-resource-final2-20260719` and
  `review-resource-final3-20260719` filled all three six-member teams from 18
  distinct live Box positions and kept Box 1 full. The corrected fixed BG badge
  sheet left the Box title and wallpaper intact before and after Box switching
  and rendered masks 1-7 simultaneously.
- Active sprite counts were 46 for the all-full Lab, 48 for full Box plus
  marking menu, and 49 for Move Items plus a six-member party. Those states
  retain 18, 16, and 15 slots of OAM headroom respectively; badges consume no
  sprites or OBJ palettes.
- Controlled outcome 1 (win) and 2 (loss) returned to the original field.
  Outcome 9 (forfeit) followed normal whiteout/heal handling and returned to a
  responsive Pokemon Center field. Every completed route read
  `gMain.callback2 = CB2_Overworld` and `gFieldCallback = NULL`.
- Screenshot paths and the BG-by-BG/OBJ resource audit are recorded in the
  owning Battle Team test plan and implementation document. All fixture changes
  were live RAM only; no persistent save was written.
- `stop` returned `stopped: true`, final managed status was `[]`, and long
  GitHub Actions were not re-waited.

## July 19, 2026 Direct Battle Lab Validation

- Candidate: `fix/runtime-lab-battle-team-incomplete-20260718`, draft PR #84.
- `rtk git diff --check`, `rtk make -j16 -O all`,
  `rtk make -j16 -O debug`, and full `rtk make -j16 -O check` passed.
- Session `battle-lab-ui-20260719b` confirmed a top-level `Battle Lab`, six
  same-screen registrations, live Pokemon tray icons, Box team-number badges,
  duplicate diagnostics, change/remove confirmations, reorder, 3v3 and 4v4
  exact-icon previews, random `R` reroll, `B` edit return, and `A` startup into
  a real double battle.
- Session `battle-lab-summary-20260719` confirmed standard Summary entry and
  return to the same tray slot after the final initialization-order fix.
- Final-ROM session `battle-lab-final-hints-20260719` confirmed the unclipped
  `A:START B:EDIT R:REROLL` prompt, live reroll, and normal-Storage
  `LOCKED by TEAM 1; remove there.` explanation.
- The successful sessions used the required script-capable
  `~/.local/bin/mgba-qt` wrapper and all stopped cleanly; final managed status
  was `[]`.
- The debug `Set Party` fixture supplied one usable party member. Four-player
  preview/start setup was created only in live emulator RAM by cloning that
  valid record; no repo or persistent save data was changed.
- The legacy Box 1 routes remain at `Party -> Box NPC Legacy...`; the visual
  run focused on the new primary Lab path rather than replaying each legacy
  battle.
- Long GitHub Actions were not re-waited. Remaining acceptance risk is extended
  repeated-match quality plus visual fill of Teams 2/3; combined masks and
  reorder integrity have automated coverage.

## Candidate

- Branch: `integration/runtime-lab-smart-ai-pr-20260716`.
- Base: `integration/runtime-lab-20260716` at `b0ac9061ce`.
- Smart reapply commit: `611933abf0`.
- Runtime PR: #78, targeting the integration target.

## Static and Build Validation

Fresh candidate results on July 16, 2026:

- `rtk git diff --cached --check` before reapply commit: passed.
- Conflict-marker scan: passed.
- `rtk make -j16 -O all`: passed.
- `rtk make -j16 -O debug`: passed.
- `rtk make -j16 -O check`: passed with exit code 0.
- Existing linker RWX and generated-tool compiler warnings remain unchanged.
- `rtk mdbook build docs`: passed. Existing warnings remain for the missing
  root `CHANGELOG.md` include, the `CREDITS.md` closing tag, and the large
  search index.

## Required mGBA Live Evidence

Use one copied ROM/save and confirm both feature families in the same process:

- Boot the combined debug ROM and continue the copied save.
- Open `Party -> Box NPC Battle...`.
- Confirm `Manage Battle Teams`, Team 1-3, and `Legacy Box 1 pool...`.
- Start one valid 3v3 singles Box or registered-team route and inspect opponent
  party count.
- Return to the overworld or restart the copied session safely.
- Open `Party -> Gauntlet Battles` and start one Read-mode route.
- Confirm the battle action log can be read or exported from the Smart route.
- Stop the session and require final managed status `[]`; otherwise record the
  exact stale entry or child process.

Completed in mGBA Live session `runtime-lab-smart-20260716`:

- The combined debug ROM booted, loaded the copied Battle Team save, continued
  to the overworld, and opened the Party debug menu.
- The same Party menu displayed `Box NPC Battle...`, Smart fixed 3v3/4v4 and
  gimmick routes, and `Gauntlet Battles` without overlap or truncation.
- The Box submenu displayed `Manage Battle Teams`, Team 1-3, and the legacy
  Box 1 pool.
- Registered `Battle Team 1 -> Single first 3` started successfully.
  `gPartiesCount` read back as `[3, 3, 0, 0]`, proving three player and three
  opponent Pokemon in the tested singles route.
- The battle returned to the overworld through the debug-battle quit flow.
- In the same emulator process, `Gauntlet Battles -> Read Single` started the
  Smart fixture and reached the command menu.
- Selecting player `Electro Drift` produced two action-log entries. The
  opponent selected Dynamax `Draco Meteor` with
  `clean_damage_preferred`, stable line `clean_damage`, and no fallback risk.
- Autosave wrote
  `/tmp/runtime-lab-smart-20260716-battle-action-log.json`. The manual exporter
  wrote `/tmp/runtime-lab-smart-20260716-manual-export.json` with schema
  `pokeemerald.battle_action_log.v4`, count 2, and no skipped invalid entries.
- The first manual-export attempt was non-evidence because the sandbox denied
  access to `~/.mgba-live-mcp/runtime/active_session`. The approved rerun
  against the same live battle succeeded.
- `stop --grace 1` returned `stopped: true`; final `status --all` returned `[]`.

## PR Audit

Before merge:

- Confirm the PR base is `integration/runtime-lab-20260716`.
- Confirm the head is `integration/runtime-lab-smart-ai-pr-20260716`.
- Review every changed path; no unrelated local/untracked file may appear.
- Confirm the three shared files contain both feature contracts.
- Confirm GitHub docs validation. Do not wait a full 20-30 minutes for long
  Actions when equivalent local validation and the standalone Smart PR checks
  already provide evidence; record any skipped wait explicitly.
- Merge only after the user-requested PR staging step. Never retarget or merge
  this runtime candidate into `master`.

## Remaining Manual Risk

The full intended user workflow is to build custom Box teams and play repeated
matches while reviewing exported AI decisions. The focused run proves route
coexistence and battle startup, but extended match quality and specific tactical
choices remain user acceptance testing rather than a compile-time guarantee.

Long GitHub Actions were not re-waited before handoff. The candidate has fresh
local normal/debug builds, full checks, mdBook output, and combined mGBA Live
evidence; the standalone Smart shelf retains its own prior CI evidence.
PR #78 `docs_validate` passed in 7 seconds. Emerald, FireRed, LeafGreen,
release, and test jobs were still pending when the branch was handed off and
were intentionally not re-waited.
