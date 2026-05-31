# 16.0 Feature Tree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Purpose | 16.0 upgrade / next runtime planning tree |
| Previous tree | [1.15.3 CompleteTree](../15_3/complete_tree/) |
| Sync policy | [upgrade_sync_policy.md](upgrade_sync_policy.md) |

16.0 は、1.15.3 integration branch を一度整理したあとに進める次の作業 tree です。
1.15.3 で完了扱いにしたものは [15.3 CompleteTree](../15_3/complete_tree/)
へ寄せ、未実装・別 lane・CI 整備は 16.0 Carryover で扱います。

15.6 は専用 tree ではありません。master が 15.x 内で更新された時の一時 sync
baseline として扱い、必要な差分だけ 16.0 側の carryover / sync policy に記録します。

## Entry Points

- [16.0 OpenTree](open_tree/)
- [16.0 Carryover Items](open_tree/carryover_items.md)
- [15.3 / 15.6 / 16.0 Upgrade Sync Policy](upgrade_sync_policy.md)

## Policy

| Rule | Detail |
|---|---|
| Version split | 1.15.3 の runtime integration result は 15.3 tree に固定する。16.0 では upstream 差分と次の feature だけを追う。 |
| Docs first | 16.0 で新しい feature を始める前に、Carryover に entry を追加する。 |
| CI cleanup | GitHub Actions の FireRed / LeafGreen / test job failures は、16.0 の最初の workflow cleanup candidate として扱う。 |
| `master` | 16.0 upstream sync でも、branch policy が変わらない限り runtime source は `master` に直接入れない。 |
