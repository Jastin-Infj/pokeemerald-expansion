# 1.15.3 Feature Tree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| PR | #68 `[codex] Runtime integration staging` |
| Status | Frozen 1.15.3 completed runtime integration tree |
| Next version tree | [16.0 OpenTree](../16_0/) |

この folder は、1.15.3 期間に個別 feature branch で作ってきた runtime / tooling
の完了棚です。ここは「次に実装する場所」ではなく、「何が完了済みかを忘れないための棚」として固定します。

## Tree Policy

| Tree | Meaning | Path |
|---|---|---|
| CompleteTree | 1.15.3 の runtime integration branch に取り込み済み、または #68 が supersede する completed shelf。 | [complete_tree](complete_tree/) |
| OpenTree | 1.15.3 では完了扱いにせず、16.0 / 15.x sync 後に再調査・再実装・CI 整備するもの。 | [../16_0/open_tree](../16_0/open_tree/) |

既存の `docs/features/<feature>/` folder は、相対リンクを壊さないため canonical
feature docs として残します。この `15_3/` folder は、それらを versioned tree
として束ねる index です。

## Current Decision

| Topic | Decision |
|---|---|
| Runtime implementation | #68 / `integration/runtime-dev-20260529` に集約済み。 |
| Old shelf PRs | #47 / #48 / #51 / #54 / #57 / #60 / #62 / #65 は、2026-05-31 に #68 superseded として close 済み。branch / PR は evidence shelf として残す。 |
| `master` | 引き続き docs / Lua-only。runtime source は直接 merge しない。 |
| CI status | Local `all` / `debug` / `check` と mGBA evidence を優先。GitHub CI の FireRed / LeafGreen / test job 整備は 16.0 Carryover に送る。 |
| 15.6 / 16.0 sync | 15.3 docs を一括で戻さない。同期方針は [upgrade_sync_policy.md](../16_0/upgrade_sync_policy.md) を正とする。 |

## Main Entry Points

- [CompleteTree](complete_tree/)
- [Completed Feature Inventory](complete_tree/completed_features.md)
- [Completed Runtime Summary](complete_tree/completed_runtime_summary.md)
- [Completed Runtime Summary HTML](complete_tree/html/index.html)
- [Completed Config / Debug Matrix](complete_tree/completed_config_debug_matrix.md)
- [16.0 Carryover Items](../16_0/open_tree/carryover_items.md)
- [15.3 / 15.6 / 16.0 Sync Policy](../16_0/upgrade_sync_policy.md)
