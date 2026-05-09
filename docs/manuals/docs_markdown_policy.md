# Docs Markdown Policy

## Document Metadata

この project で追加・更新する調査 docs には、可能な限り先頭付近に
metadata table を置く。古い docs は一括変換せず、次に触った時点で追加する。

推奨形:

```markdown
## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | YYYY-MM-DD |
| Baseline | `master` `<commit>`; upstream `expansion/<version>` |
| Code status | Docs-only / No code changes / Branch implementation / Shipped |
| Provenance | Local project overlay / Upstream doc with local notes / Feature handoff |
```

`Last reviewed` は、その docs の事実関係を最後に確認した日を書く。
`Baseline` は、調査対象の repo state を書く。version だけでは不足するので、
可能なら `git describe --tags --always` と commit を併記する。

2026-05-09 時点の local baseline:

| Field | Value |
|---|---|
| Branch | `master` plus docs branch updates |
| Commit | `8d2664af9a` |
| Git describe | `expansion/1.15.2-24-g8d2664af9a` |
| README credit baseline | `pokeemerald-expansion 1.15.2` |
| Version constants | `EXPANSION_VERSION_MAJOR 1`, `MINOR 15`, `PATCH 3`, `EXPANSION_TAGGED_RELEASE FALSE` |
| Practical label | 1.15.2 tag plus post-release 1.15.3-dev style commits |

## Provenance Labels

Use one of these labels in `Provenance` or in the opening paragraph.

| Label | Meaning | Edit policy |
|---|---|---|
| Upstream doc | Existing RHH / pokeemerald-expansion documentation. | Preserve structure. Prefer small corrections and links. |
| Upstream doc with local notes | Upstream doc plus local project supplement. | Keep upstream content recognizable; put local additions under `Local Notes` or in manuals. |
| Local project overlay | Docs created for this project workflow, investigation, or feature planning. | Actively update as decisions change. |
| Feature handoff | Implementation summary, validation, risks, and merge notes from a feature branch. | Keep exact branch / commit / validation evidence. |
| Generated / audit report | Output from a focused audit or tool run. | Include date, command, scope, and stale-data warning. |

## GitHub-Friendly Markdown

Write for GitHub first, mdBook second.

- Use relative links that work in GitHub, for example `[GitHub Workflow](github_workflow.md)`.
- Use fenced code blocks with language tags when possible.
- Keep Mermaid diagrams in fenced `mermaid` blocks. GitHub renders Mermaid, and diagrams are useful for callback / script / battle flows.
- Prefer pipe tables for compact facts. Keep cells short enough to read in GitHub.
- Avoid relying on mdBook-only behavior for critical navigation.
- When a table grows too wide, split it into smaller sections instead of adding nested bullets.

## Japanese Supplements

Existing `docs/tutorials/` are mostly upstream user-facing tutorials. Do not
translate or rewrite all of them in place.

Recommended pattern:

1. Keep upstream tutorial content intact.
2. Add a short `Local Notes` section only when the note is directly tied to this repo's workflow.
3. Prefer Japanese project-specific explanation in `docs/manuals/`.
4. Link from the manual to the upstream tutorial.

This keeps upstream merge conflicts small while still giving Japanese guidance
where it helps day-to-day work.

## Diagrams

Continue using diagrams for:

- callback transitions
- script command dispatch
- battle start/end flow
- generated data pipelines
- save / runtime state ownership

Diagrams should show the real runtime path, not the intended feature design
alone. If a diagram describes a proposal, label it as planned.

## Review Cadence

When upstream moves from 1.15.2 to 1.15.3 / 1.16, update docs in this order:

1. `docs/upgrades/update_migration_notes.md`
2. `docs/upgrades/upstream_diff_checklist.md`
3. affected `docs/flows/*_v15.md`
4. affected `docs/features/*`
5. manual entry points and `docs/SUMMARY.md`

Do not rename all `*_v15.md` files just because a new upstream release exists.
Rename or fork to `*_v16.md` only when the underlying flow changes enough that
the old document would mislead implementation work.
