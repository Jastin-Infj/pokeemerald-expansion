# 1.15.3 Feature Tree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| PR | #68 `[codex] Runtime integration staging` |
| Status | 1.15.3 runtime integration tree |
| Next version tree | [16.0 OpenTree](../16_0/) |

この folder は、1.15.3 期間に個別 feature branch で作ってきた runtime / tooling
をバージョン単位で整理する入口です。

## Tree Policy

| Tree | Meaning | Path |
|---|---|---|
| CompleteTree | 1.15.3 の runtime integration branch に取り込み済み、または #68 が supersede する completed shelf。 | [complete_tree](complete_tree/) |
| OpenTree | 1.15.3 では完了扱いにせず、16.0 で再調査・再実装・CI 整備するもの。 | [../16_0/open_tree](../16_0/open_tree/) |

既存の `docs/features/<feature>/` folder は、相対リンクを壊さないため canonical
feature docs として残します。この `15_3/` folder は、それらを versioned tree
として束ねる index です。

## Current Decision

| Topic | Decision |
|---|---|
| Runtime implementation | #68 / `integration/runtime-dev-20260529` に集約済み。 |
| Old open PRs | #47 / #48 / #51 / #54 / #57 / #60 / #62 / #65 は、#68 へ取り込み済みまたは tool release 済みの evidence shelf として扱う。 |
| `master` | 引き続き docs / Lua-only。runtime source は直接 merge しない。 |
| CI status | Local `all` / `debug` / `check` と mGBA evidence を優先。GitHub CI の FireRed / LeafGreen / test job 整備は 16.0 OpenTree に送る。 |

## Main Entry Points

- [CompleteTree](complete_tree/)
- [1.15.3 Runtime Summary HTML](complete_tree/html/index.html)
- [Runtime Summary Markdown](complete_tree/runtime_summary.md)
- [Config / Debug Matrix](complete_tree/config_debug_matrix.md)
- [16.0 OpenTree](../16_0/open_tree/)
