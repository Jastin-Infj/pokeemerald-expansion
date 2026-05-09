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
- For docs-only merge work, do not merge an implementation branch that contains
  source / include / data / tools / generated changes into `master`. Use a
  docs-only branch from `master` or cherry-pick only docs / `AGENTS.md`
  workflow commits, then check `rtk git diff --name-only master..HEAD` before
  merging.
- Before handoff, ensure the feature `test_plan.md` records local make results,
  mGBA Live / manual evidence, skipped long GitHub Actions waits, and any
  accepted remaining risk.

## GitHub PR Staging

- Open PRs are review / staging shelves, not permission to merge into
  `master`. Do not press the GitHub merge button or run `gh pr merge` unless
  the user explicitly asks to merge that PR.
- Avoid direct pushes to `master`. Push feature / docs branches and let the
  user choose when to merge, unless the user explicitly asks for a direct
  master update.
- For implementation PRs that contain source / include / data / tools /
  generated changes, prefer a fresh branch from current `master` and
  cherry-pick or re-apply only the intended slice when the planned order
  changes. Close the older PR only after recording why it was superseded.
- Keep an open implementation PR if it is still a valid candidate but not next
  in the order. Close stale PRs that are already superseded, failed drafts, or
  docs snapshots that would reintroduce old diff.
- Delete a remote branch only when the PR is merged, fully superseded, or has
  no unique work. Preserve branches that hold unique draft work even if their
  PR is closed.
