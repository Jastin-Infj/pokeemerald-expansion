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
  eligible docs / `AGENTS.md` / Lua script commits, then check
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
  `docs/`, approved Lua script files for shortcuts, debug commands, and
  validation automation, and, when workflow rules change, `AGENTS.md`.
- A validated feature branch is evidence, not permission to update `master`.
  Record the branch, commit, diff scope, and validation evidence in docs; keep
  runtime source changes on a fresh `feature/` or `integration/` branch created
  from the current `master`.
- If a branch contains both docs and implementation, never merge the branch into
  `master` for a docs / Lua-only request. Cherry-pick or re-apply only eligible
  docs / `AGENTS.md` / Lua script changes onto a fresh branch.
- Before any `master` PR or merge, confirm the file list with
  `rtk git diff --name-only master..HEAD`. Anything outside `docs/`,
  `AGENTS.md`, and approved Lua script files means the branch is not
  eligible for a docs / Lua-only master merge.
- Graphics and other image assets, including `.png` icons, are implementation
  artifacts. Keep them on a feature / integration implementation PR with the
  source changes that consume them; record source URLs and credit in docs, but
  do not include the image files themselves in a docs / Lua-only PR.

## GitHub PR Staging

- Open PRs are review / staging shelves, not permission to merge into
  `master`. Do not press the GitHub merge button or run `gh pr merge` unless
  the user explicitly asks to merge that PR.
- Avoid direct pushes to `master`. Push feature / docs branches and let the
  user choose when to merge, unless the user explicitly asks for a direct
  master update.
- For implementation PRs that contain source / include / data / tools /
  graphics / generated changes, prefer a fresh branch from current `master` and
  cherry-pick or re-apply only the intended slice when the planned order
  changes. Close the older PR only after recording why it was superseded.
- Keep an open implementation PR if it is still a valid candidate but not next
  in the order. Close stale PRs that are already superseded, failed drafts, or
  docs snapshots that would reintroduce old diff.
- Delete a remote branch only when the PR is merged, fully superseded, or has
  no unique work. Preserve branches that hold unique draft work even if their
  PR is closed.
