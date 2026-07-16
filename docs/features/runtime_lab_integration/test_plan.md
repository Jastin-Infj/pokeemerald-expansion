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
- GitHub PR metadata and `docs_validate` remain pending until the handoff
  branch is pushed.
