# Runtime Lab Integration Test Plan

## Runtime Candidate

- Branch: `integration/runtime-lab-smart-ai-pr-20260716`.
- Base: `integration/runtime-lab-20260716` at `b0ac9061ce`.
- Smart reapply commit: `611933abf0`.
- Runtime PR: #78, targeting the integration target.
- Final candidate head: `703509f9d1`.
- Result: merged as `4e960aecea` on July 16, 2026.

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

## Practical Battle Validation: July 18, 2026

This pass exercised the completed integration branch rather than another
reapply candidate.

- Branch: `integration/runtime-lab-20260716`.
- Head: `4e960aecea0185912e21a28f40e00fab1b388874`.
- Debug ROM SHA-256:
  `42382f2d10514119c7170a961ec6b017f8c0323e9888f00e2918bfdb8d8703e4`.
- Copied ROM:
  `.cache/mgba-live-roms/runtime-lab-practical-20260718/runtime-lab-practical.gba`.
- Save source:
  `.cache/mgba-live-roms/battle-team-boxes-20260716/pokeemerald-debug.sav`.
- `rtk make -j16 -O debug`: passed in a dedicated worktree. Existing linker
  RWX warnings were unchanged.

### Baseline Runtime Routes

- `Battle Team 1 -> Single first 3`: passed. `gPartiesCount` was
  `[3, 3, 0, 0]`, `gBattleTypeFlags` began with `12`, and the strong debug AI
  flags were present. The command screen reached Xerneas versus Delcatty.
- `Battle Team 1 -> Double first 4`: passed. `gPartiesCount` was
  `[3, 4, 0, 0]`, `gBattleTypeFlags` began with `13`, and the same strong AI
  preset was present. The field reached Marshadow and Xerneas versus Spearow
  and Delcatty.
- `Gauntlet Battles -> Read Single`: passed. The v4 action log contained two
  entries. The AI selected Dynamax `Draco Meteor` with reason
  `clean_damage_preferred`, stable line `clean_damage`, loss clock 1, and the
  selected-gimmick flag.
- `Gauntlet Battles -> Read Double`: passed as a representative read-mode
  route. The v4 log contained five entries. One AI slot preserved itself with
  a switch classified as `switch_preserve` / `switch_survival` under known KO
  pressure, while its partner selected a known-command answer.

Local evidence paths were:

- `/tmp/runtime-lab-practical-20260718-read-single.json`
- `/tmp/runtime-lab-practical-20260718-read-double.json`
- `/tmp/runtime-lab-read-20260718-single-turn1.png`
- `/tmp/runtime-lab-read-double-20260718-field.png`

### Focused Special Boards

The completed branch did not expose deterministic menu fixtures for every
special board. Three local-only debug routes were therefore added on a
temporary `validation/runtime-lab-practical-fixtures-20260718` branch. That
branch was never committed, pushed, or proposed for merge. After evidence was
captured, all three modified source/data files were restored, the temporary
branch was deleted, and the integration worktree returned clean at
`4e960aecea`.

- **Commander and Tatsugiri: passed.** Miraidon's `Electro Drift` targeted the
  Dondozo slot and logged `commander_slot_correction`; the swallowed Tatsugiri
  was not treated as an independently actionable target. Toxic Orb poisoned
  Tatsugiri while it was swallowed. After Dondozo fainted, the poisoned
  Tatsugiri remained on the field and received a command normally.
- **Perish Song desperation: passed.** With no reserve and a two-turn loss
  clock, the AI chose `Stomp`. The action logged reason `hax_out`, risk
  `secondary_hax`, fallback `high_variance`, and threats
  `perish_trap_clock` plus `desperation`. This is the intended narrow use of a
  flinch line when clean winning lines are gone.
- **No-safe-pivot Kyogre board: missed.** Incineroar `Fake Out` and Tapu Koko Z
  `Thunderbolt` were both logged against opponent-right Kyogre. Tornadus chose
  `Tailwind`, then Kyogre still chose `Water Spout` with
  `clean_damage_preferred`, stable `clean_damage`, and no risk classification.
  It did not preserve Kyogre by switching to the limited reserve. This is a
  reproduced runtime behavior gap, not an accepted result.

Only the final no-safe-pivot attempt is evidence for that finding. Earlier
attempts used the wrong lead/target order or allowed the threatened opponent
slot to decide before the second player command was logged, so they were
excluded. The final fixture deterministically placed Tornadus opponent-left,
Kyogre opponent-right, and logged both player attacks against battler 3 before
Kyogre's action entry.

Special-board evidence paths were:

- `/tmp/runtime-lab-practical-20260718-commander.json`
- `/tmp/runtime-lab-practical-20260718-commander-after-ko.json`
- `/tmp/runtime-lab-practical-20260718-no-safe-pivot-exact3.json`
- `/tmp/runtime-lab-practical-20260718-perish-hax.json`
- `/tmp/runtime-lab-fixture-commander-20260718-commanding.png`
- `/tmp/runtime-lab-fixture-commander-20260718-tatsugiri-remains.png`
- `/tmp/runtime-lab-fixture-pivot-exact3-field-real.png`
- `/tmp/runtime-lab-fixture-perish-after-a1.png`

### Focused Automated Checks

The following battle tests passed:

- Commander target correction and poisoned-Tatsugiri continuation tests.
- Commander Dondozo switch suppression.
- 15-of-16 survival switch acceptance under focus pressure.
- desperation-only partner spread sacrifice.
- Perish Song flinch logging and clean-damage hax suppression.
- low-accuracy sleep acceptance under imminent KO.
- delayed Solar Beam acceptance under a short loss clock.
- the broader `TESTS='Commander'` set: 26 tests passed.

The practical runtime pass did not change implementation code, so long GitHub
Actions were not re-waited. Every managed mGBA session used for accepted
evidence stopped cleanly, and final `status --all` returned `[]`.

### Required Follow-up

Before the no-safe-switch objective can be called complete, add a deterministic
runtime fixture on a new implementation branch from current `master`, fix the
Kyogre decision so incoming known commands affect switch survival/preservation
scoring, and rerun the exact battler-3 log assertion. A unit-test pass alone is
not sufficient for this case because the practical command-order path is the
behavior that failed.

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

## Master Handoff Validation

The separate `docs/runtime-lab-handoff-20260716` branch must contain exactly
the intended Markdown handoff and approved Lua validation scripts. Required
checks before its `master` PR and merge:

- `rtk git diff --name-only master..HEAD` contains only `.md` and `.lua`.
- No runtime candidate commit is an ancestor of the handoff branch.
- `rtk git diff --check` passes.
- `rtk mdbook build docs` passes with only recorded existing warnings.
- Both Lua blobs match the versions loaded successfully by the combined mGBA
  runtime. This environment has no standalone `luac`, so blob identity plus the
  recorded live export is the accepted validation evidence.
- GitHub PR base is `master`, the full file list matches the local audit, and
  `docs_validate` passes.

Local handoff results on July 16, 2026:

- Staged scope is exactly 32 files: 30 Markdown files and two Lua files.
- The excluded-path audit returned no non-Markdown/Lua paths.
- `611933abf0` and `4e960aecea` are not ancestors of the handoff branch.
- `rtk git diff --cached --check`: passed.
- `rtk mdbook build docs`: passed with the existing missing `CHANGELOG.md`,
  `CREDITS.md` closing-tag, and large search-index warnings.
- Standalone `luac` is unavailable. The restored Lua blob IDs are
  `8a473503ee281dd83ef7d93904a0c7b946348ea9` and
  `d77376aec91336a2d066526fa710f015d70bb520`; both exactly match the
  integration commit versions used by the successful combined mGBA Live run.
- Master handoff PR #79 targets `master`, reports the same 32-file scope, and
  is mergeable. Its first-head `docs_validate` passed in 11 seconds. Long
  build/test Actions were pending and were intentionally not re-waited.
