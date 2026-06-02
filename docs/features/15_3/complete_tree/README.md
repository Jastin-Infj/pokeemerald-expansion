# 1.15.3 CompleteTree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| PR | #68 `[codex] Runtime integration staging` |
| Completed feature list | [completed_features.md](completed_features.md) |
| Config / Debug | [completed_config_debug_matrix.md](completed_config_debug_matrix.md) |

この CompleteTree は、1.15.3 で作ってきたもののうち、現在の integration branch
に統合済みとして扱うものをまとめた tree です。`completed_...` で始まる file は、
1.15.3 完了棚の証跡として固定します。

## Completion Rule

| Mark | Meaning |
|---|---|
| Complete | #68 に実装が入っている、または同等以上の実装が #68 に統合済み。 |
| Superseded by #68 | 古い open PR / closed PR / branch は evidence shelf として残すが、個別 merge せず #68 を正とする。 |
| Tool release done | ROM runtime ではないが、tooling として release / artifact 導線まで完了。 |
| Docs retained | feature docs は canonical path に残し、ここから参照する。 |

## Where To Check Status

| Need | File |
|---|---|
| 実装済み feature 一覧 | [completed_features.md](completed_features.md) |
| 追加 / 変更の全体像 | [completed_runtime_summary.md](completed_runtime_summary.md) |
| config / debug command 一覧 | [completed_config_debug_matrix.md](completed_config_debug_matrix.md) |
| 取り込み監査の詳細 | [completed_runtime_integration_checklist_ja_2026_05_30.md](completed_runtime_integration_checklist_ja_2026_05_30.md) |
| branch / PR 監査 | [completed_branch_audit_2026_05_29.md](completed_branch_audit_2026_05_29.md) |

## Related 15.3 Docs

- [Completed Feature Inventory](completed_features.md)
- [Runtime Summary Markdown](completed_runtime_summary.md)
- [Config / Debug Matrix](completed_config_debug_matrix.md)
- [1.15.3 Runtime Integration Checklist](completed_runtime_integration_checklist_ja_2026_05_30.md)
- [Runtime Integration Branch Audit](completed_branch_audit_2026_05_29.md)

## Validation Snapshot

The integration evidence is recorded in the owning feature `test_plan.md` files
and in [completed_runtime_summary.md](completed_runtime_summary.md). The branch has local `all`,
`debug`, full `check`, docs build, codex review follow-ups, and focused mGBA
Live smoke evidence. GitHub CI failures are not treated as 1.15.3 feature
blockers here; they are tracked as 16.0 OpenTree workflow cleanup.
