# 16.0 Feature Tree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Purpose | 16.0 upgrade / next runtime planning tree |
| Previous tree | [1.15.3 CompleteTree](../15_3/complete_tree/) |

16.0 は、1.15.3 integration branch を一度整理したあとに進める次の作業 tree です。
1.15.3 で完了扱いにしたものは [15.3 CompleteTree](../15_3/complete_tree/)
へ寄せ、未実装・別 lane・CI 整備は 16.0 OpenTree で扱います。

## OpenTree

- [16.0 OpenTree](open_tree/)

## Policy

| Rule | Detail |
|---|---|
| Version split | 1.15.3 の runtime integration result は 15.3 tree に固定する。16.0 では upstream 差分と次の feature だけを追う。 |
| Docs first | 16.0 で新しい feature を始める前に、OpenTree に entry を追加する。 |
| CI cleanup | GitHub Actions の FireRed / LeafGreen / test job failures は、16.0 の最初の workflow cleanup candidate として扱う。 |
| `master` | 16.0 upstream sync でも、branch policy が変わらない限り runtime source は `master` に直接入れない。 |
