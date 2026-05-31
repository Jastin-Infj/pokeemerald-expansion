# 15.3 / 15.6 / 16.0 Upgrade Sync Policy

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Purpose | 15.3 runtime integration docs を、今後の 15.6 / 16.0 upstream baseline と混ぜないための同期方針 |
| Previous complete tree | [15.3 CompleteTree](../15_3/complete_tree/) |
| Carryover list | [16.0 Carryover Items](open_tree/carryover_items.md) |

## Version Meaning

| Tree | Meaning | What goes there |
|---|---|---|
| `15_3/complete_tree` | 1.15.3 期間にこの branch へ統合した runtime / tooling の完了棚。 | 実装済み feature、検証済み debug route、config 一覧、#68 に supersede された旧 PR 情報。 |
| `15.6` | 15.x 系の中間 upstream intake 想定。正式な local tree ではなく、master が 15.x 内で動いた時の比較対象。 | upstream README / docs / config / generated data の差分確認。15.3 完了棚をそのまま戻さない。 |
| `16_0/open_tree` | 16.0 移行時に再調査・再実装・CI 整備する棚。 | CI cleanup、upstream resync、未実装 candidate、別 lane。 |

## Why This Split Exists

1. `15_3/complete_tree` は「何を実装済みにしたか」を忘れないための凍結棚。
2. `16_0/open_tree` は「まだやること」を追うための作業棚。
3. `15.6` は folder を作らず、sync 時の一時 baseline として扱う。必要になったら `16_0/open_tree` に差分を記録する。

この分け方にしないと、master が upstream 15.6 / 16.0 に更新された時に、古い 15.3 docs をまとめて戻してしまい、README や feature status が逆戻りする。

## Sync Rules

| Situation | Rule |
|---|---|
| master が 15.6 相当に更新された | `15_3/complete_tree` は触らず、master 側の docs と #68 の docs 差分を比較する。必要な項目だけ `16_0/open_tree` に追記する。 |
| master が 16.0 に更新された | fresh integration branch を作り、#68 の runtime 実装を replay / cherry-pick / reapply する。docs は `15_3/complete_tree` を証跡として読み、16.0 docs に必要なものだけ移植する。 |
| README が conflict した | upstream README を優先し、local runtime integration の説明は `features/15_3/` と `features/16_0/` の version tree に逃がす。 |
| Feature docs が conflict した | canonical `docs/features/<feature>/` は upstream との conflict を解いて残す。完了判定は `completed_features.md` を正とする。 |
| CI workflow が conflict した | runtime feature と混ぜず、[carryover_items.md](open_tree/carryover_items.md) の CI cleanup として別作業にする。 |
| 15.3 docs を master へ入れたい | source / data / tools を含めず、docs / Lua-only branch で `features/15_3/` と `features/16_0/` の必要分だけ cherry-pick する。 |

## Recommended Sync Procedure

1. 先に #68 の CI 判定方針を決める。runtime 実装そのものの確認と、GitHub Actions の FRLG / test matrix は分けて扱う。
2. `master` は docs / Lua-only のまま維持する。#68 runtime source は `master` に直接 merge しない。
3. docs handoff が必要な場合でも、#68 docs を丸ごと `master` へ持ち込まない。`features/15_3/` / `features/16_0/` の必要 file だけを docs-only branch で cherry-pick する。
4. `master` を upstream 15.6 / 16.0 に更新する。
5. 新しい integration branch を更新後の `master` から作る。
6. `15_3/complete_tree/completed_features.md` を見て、#68 のどの実装を再適用するか決める。
7. docs は一括コピーしない。まず upstream の README / docs を残し、必要な local docs だけ移植する。
8. conflict が出た feature は `16_0/open_tree/carryover_items.md` に追記する。
9. local build / check / mGBA evidence を取ってから、16.0 用の新しい completed / carryover tree を作る。

## Practical Read Order

| Need | Read |
|---|---|
| 1.15.3 で何を実装済みにしたか | [completed_features.md](../15_3/complete_tree/completed_features.md) |
| 1.15.3 の config / debug route | [completed_config_debug_matrix.md](../15_3/complete_tree/completed_config_debug_matrix.md) |
| 1.15.3 の全体像 | [completed_runtime_summary.md](../15_3/complete_tree/completed_runtime_summary.md) |
| 16.0 に持ち越す作業 | [carryover_items.md](open_tree/carryover_items.md) |
| master 更新時の判断 | この file |
