# Docs Audit 2026-05-09

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `8d2664af9a`; `git describe` = `expansion/1.15.2-24-g8d2664af9a` |
| Code status | Docs-only audit |
| Provenance | Local project audit report |

## Summary

The docs are broadly usable and the main investigation conclusions still match
the current repository shape. The most important update is baseline wording:
the repo is no longer only a 1.15.1 investigation target. Current `master` is
based on the 1.15.2 release and has post-1.15.2 commits; version constants show
`1.15.3` with `EXPANSION_TAGGED_RELEASE FALSE`.

This means future docs should say "1.15.2 tag plus post-release commits" when
talking about the current baseline, while older `*_v15.md` investigation files
can remain named `v15` until upstream changes the actual flow.

## Commands Used

```sh
rtk git describe --tags --always --dirty
rtk rg -n "1\\.15\\.1|1\\.15\\.2|baseline|Baseline" docs
rtk rg -n "^(Status|Code status|Code Status):|Status: Shipped|Status: Implementing|No code changes|Validated branch" docs
rtk bash -lc 'for f in docs/features/*.md docs/features/*/*.md docs/flows/*.md docs/overview/*.md docs/upgrades/*.md docs/manuals/*.md docs/templates/*.md; do [ -f "$f" ] || continue; if ! head -n 12 "$f" | grep -Eq "調査日|Date:|Reviewed:|Last reviewed|最終更新|Audit date"; then echo "$f"; fi; done'
```

## Findings

| Finding | Severity | Resolution |
|---|---|---|
| `field_move_modernization/README.md` said `Status: Investigating`, while `mvp_plan.md` and registry had promoted the feature to Planned. | Medium | Updated README to Planned. |
| `update_migration_notes.md`, `project_overview_v15.md`, `source_map_v15.md`, and `1_15_1_to_1_15_2_impact.md` still spoke as if local baseline was 1.15.1. | Medium | Updated baseline notes to current 1.15.2-plus state. |
| Many local docs do not have a standardized `Last reviewed` / `Baseline` block. | Low | Added `docs_markdown_policy.md`; future edits should add metadata rather than churn every file at once. |
| Open PR meaning was ambiguous. | Medium | Added PR staging rules to `AGENTS.md`, `github_workflow.md`, and `feature_registry.md`. |

## Current Baseline

| Source | Observed value |
|---|---|
| `README.md` credit example | `pokeemerald-expansion 1.15.2` |
| `include/constants/expansion.h` | `1.15.3`, `EXPANSION_TAGGED_RELEASE FALSE`, comment `Last version: 1.15.2` |
| `git describe` | `expansion/1.15.2-24-g8d2664af9a` before local docs edits |
| Practical docs label | `expansion/1.15.2` plus post-release `1.15.3-dev` style commits |

## Recommended Metadata Rollout

Do not add metadata to every docs file in one giant diff. Add it when a doc is
already being touched for a real reason.

Priority order:

1. `docs/features/*/README.md`
2. feature `investigation.md`
3. high-impact `docs/flows/*_v15.md`
4. `docs/overview/*_v15.md`
5. manuals and tutorials

## Tutorial / Manual Boundary

The upstream tutorial tree should remain recognizable. For Japanese guidance,
prefer one of these patterns:

- Add Japanese project guidance to `docs/manuals/` and link to upstream tutorial.
- Add a short `Local Notes` section to a tutorial only when it prevents a real mistake in this repo.
- Avoid full translation unless the tutorial becomes a daily operational dependency.

## Remaining Watch Items

| Area | Why |
|---|---|
| `*_v15.md` naming | Keep for now. Rename only when upstream 1.16 changes flow enough to invalidate the doc. |
| Partygen PR #7 | Open staging PR, not direct-merge ready; dirty against current master. |
| Aftercare PR #10 | Open staging PR; if adopting only item restore first, split from a fresh branch. |
| mdBook warnings | Existing warnings are known; GitHub readability is the primary target for local planning docs. |
