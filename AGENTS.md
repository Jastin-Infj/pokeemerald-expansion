@/home/jastin/.codex/RTK.md

## Branch Runtime Verification

- For source / data / config changes that affect ROM runtime, run local
  validation before push. Use `rtk make -j16 -O all` for the normal ROM,
  `rtk make -j16 -O debug` when a debug route or debug menu is involved,
  and `rtk make -j16 -O check` or a focused `TESTS=...` check for battle,
  Pokemon, item, save, or script logic.
- When the mGBA Live MCP server is available, attempt one focused MCP runtime
  validation before push. Confirm at least ROM boot plus screenshot/input or
  the feature-specific screen/state. If MCP cannot validate the target
  behavior, record the exact failure and remaining manual check in the feature
  `test_plan.md`, then report that it could not be confirmed.
- mGBA Live must use the script-capable mGBA build for this project. The local
  `mgba-qt` wrapper should resolve to
  `/home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt`
  and default `DISPLAY` to `:0`.
- Do not block a turn waiting for long GitHub Actions runs. They can take
  roughly 20-30 minutes; use local `make` and mGBA Live evidence for the agent
  handoff, and note that Actions were not re-waited when applicable.
- Stop mGBA Live sessions after validation. If `mgba-live-cli stop` leaves a
  stale entry or zombie child, record that cleanup state in the relevant docs
  instead of treating the runtime check as fully clean.

## Documentation Handoff

- When a feature implementation is completed or user-confirmed, update the
  owning feature docs with an implementation summary. Prefer
  `docs/features/<feature>/implementation.md` for what changed, why, validation,
  manual checks, remaining risks, and merge handoff notes.
- When setup, tooling, mGBA Live MCP, GitHub workflow, or merge operation rules
  change, update the matching manual under `docs/manuals/` and link it from
  `docs/SUMMARY.md`.
- For docs / Lua-only merge work, do not merge an implementation branch that
  contains source / include / data / graphics / tools non-Lua / generated
  changes into `master`. Use a fresh branch from `master` or cherry-pick only
  eligible Markdown docs / `AGENTS.md` / Lua script commits, then check
  `rtk git diff --name-only master..HEAD` before merging.
- Before handoff, ensure the feature `test_plan.md` records local make results,
  mGBA Live / manual evidence, skipped long GitHub Actions waits, and any
  accepted remaining risk.

## Master / Upstream Baseline

- Treat `master` as the upstream intake baseline plus local documentation and
  workflow overlay. Do not merge local feature implementation into `master`
  unless the user explicitly asks to change that branch policy for a specific
  integration.
- On `master`, source-like trees are read-only for normal work: `src/`,
  `include/`, `data/`, `graphics/`, non-Lua `tools/`, generated output, ROMs,
  saves, caches, screenshots, and image assets. Docs / Lua-only work may update
  Markdown documentation files, including `docs/` and root documentation such as
  `CREDITS.md`, approved Lua script files for shortcuts, debug commands, and
  validation automation, and, when workflow rules change, `AGENTS.md`.
- A validated feature branch is evidence, not permission to update `master`.
  Record the branch, commit, diff scope, and validation evidence in docs. During
  an upstream release transition or for a standalone compatibility shelf, start
  runtime source work from current `master`. When a pinned active runtime
  integration exists for the current upstream generation, start playable-line
  feature work from that exact integration head and target it with a separate
  PR; never implement directly on the integration branch.
- If a branch contains both docs and implementation, never merge the branch into
  `master` for a docs / Lua-only request. Cherry-pick or re-apply only eligible
  Markdown docs / `AGENTS.md` / Lua script changes onto a fresh branch.
- Before any docs / Lua-only `master` PR or merge, confirm the file list with
  `rtk git diff --name-only master..HEAD`. Anything outside Markdown docs,
  `AGENTS.md`, and approved Lua script files means the branch is not eligible
  for a docs / Lua-only master merge. An upstream intake PR follows `Upstream
  Release Transitions` instead and must prove that source-like changes come
  from the pinned upstream release rather than local feature implementation.
- Graphics and other image assets, including `.png` icons, are implementation
  artifacts. Keep them on a feature / integration implementation PR with the
  source changes that consume them; record source URLs and credit in docs, but
  do not include the image files themselves in a docs / Lua-only PR.

## Upstream Release Transitions

- Treat each upstream release generation independently. This policy applies to
  1.16.1 -> 1.16.2, 1.16.x -> 1.17, and later transitions; do not hard-code the
  integration workflow to one release number.
- Never adopt, merge, cherry-pick, or Sync Fork the moving `RHH/master` branch.
  It is discovery/comparison input only, regardless of how far it has advanced.
  A commit becomes eligible for intake only after pokeemerald-expansion
  publishes it in an official, non-draft, non-prerelease
  `expansion/<version>` GitHub Release tag.
- Treat that official release tag as the canonical upstream version identity;
  do not create a new branch solely to duplicate it. Branches remain work lines
  or retained implementation evidence. Use immutable annotated local tags for
  important validated integration milestones, without moving an existing tag.
- Before porting local features, fetch the non-pushable upstream remote, verify
  the exact release tag and commit, record the current fork `master` commit, and
  preserve the previous playable line under `snapshot/runtime-<old-version>/*`.
- Intake upstream source through a dedicated `upgrade/*` branch and PR. This is
  the only normal exception that permits upstream-authored source / data /
  generated changes to enter `master`; local runtime feature implementation is
  still excluded from `master`.
- The pinned official release tag is authoritative during conflict resolution.
  When that release renames a variable, changes an API, replaces a struct,
  changes a save/data layout, moves ownership, or updates generated formats,
  preserve the released contract and port the local feature's intent onto it.
  Do not restore an old file or field merely because the local branch used it.
- Never resolve a release conflict with blanket `ours` / `theirs`. Inspect the
  semantic change, identify all producers and consumers, adapt local call sites
  and tests, and document any compatibility shim that remains necessary.
- Regenerate generated data with the new release's tools and schemas. Do not
  cherry-pick old generated output across release generations unless byte-level
  compatibility has been demonstrated.
- Create the new playable development line fresh from the updated `master` as
  `integration/active-runtime-<version>`. Re-apply selected `shelf/*` features
  in an explicit documented order through separate PRs, run combined local and
  mGBA validation, and keep the prior generation immutable for comparison.
- Use branch lifecycle names consistently: `feature/*` for active isolated
  work, `shelf/runtime-<version>/*` for completed reusable implementation,
  `integration/active-runtime-<version>` for the one current playable base,
  `snapshot/runtime-<version>/*` for frozen combined states, and `docs/*` for
  master-eligible docs / Lua handoff work. Preserve frozen branches by default;
  rename them instead of deleting them.

## Historical 16.0 Runtime Lineage

- Treat `integration/runtime-dev-16-20260531` / PR #69 as the completed
  15.3-to-16.0 runtime port snapshot. It is the comparison baseline and
  evidence shelf for that replay, not the branch where future runtime features
  should keep accumulating.
- This section is historical evidence for the 16.0 replay. Current and future
  feature branch bases follow `Upstream Release Transitions` above.
- If a playable "16.0 port snapshot plus new work" branch is needed, duplicate
  the completed snapshot into a new `integration/*` branch first, then apply the
  new work there. Keep the original snapshot available so reviewers can compare
  `master`, the completed 15.3-to-16.0 port, and the new dev branch separately.
- Historical 15.3 branches and docs are references only after the snapshot is
  complete. Future 16.0 work owns its own branch, docs, validation evidence, and
  PR.
- Publish 15.3 / 16.0 handoff docs to `master` only through a docs / Lua-only
  branch. Do not merge the runtime snapshot into `master`.

## GitHub PR Staging

- Open PRs are review / staging shelves, not permission to merge into
  `master`. Do not press the GitHub merge button or run `gh pr merge` unless
  the user explicitly asks to merge that PR.
- Avoid direct pushes to `master`. Push feature / docs branches and let the
  user choose when to merge, unless the user explicitly asks for a direct
  master update.
- For implementation PRs that contain source / include / data / tools /
  graphics / generated changes, use the branch base selected by `Upstream
  Release Transitions`, then cherry-pick or re-apply only the intended slice
  when the planned order changes. Close the older PR only after recording why
  it was superseded.
- Keep an open implementation PR if it is still a valid candidate but not next
  in the order. Close stale PRs that are already superseded, failed drafts, or
  docs snapshots that would reintroduce old diff.
- Preserve remote implementation and integration evidence by default, including
  branches behind merged or closed PRs. Rename completed reusable work to
  `shelf/*` and frozen combined work to `snapshot/*`. Delete a remote branch
  only when the user explicitly requests deletion after confirming it has no
  unique evidence or recovery value.
