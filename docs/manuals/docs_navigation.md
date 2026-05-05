# Docs Navigation Manual

この文書は、`docs/` 配下の使い分けを決めるための入口。
既存の upstream docs を壊さず、この project 用の判断基準や feature branch の作業契約を重ねる。

## Directory Roles

| Directory | Role | Edit policy |
|---|---|---|
| `docs/tutorials/` | upstream 由来の具体手順。既存機能の使い方、追加手順、設定例。 | 原則温存。誤り修正や参照追加はよいが、大きな再構成は避ける。 |
| `docs/manuals/` | この project 用の入口、判断基準、作業順、影響範囲の読み方。 | 積極的に更新する。tutorial を再説明するより、どれを読むべきかを案内する。 |
| `docs/features/` | feature branch ごとの設計契約。README / investigation / mvp_plan / risks / test_plan を持つ。 | feature の進行に合わせて更新する。実装と current decision を一致させる。 |
| `docs/overview/` | 横断的な調査ログ、広い影響範囲、将来候補の整理。 | 調査結果を残す。feature 化したら `docs/features/` から参照する。 |
| `docs/flows/` | 既存 runtime flow、callback、script、battle、UI などの流れ。 | 既存挙動の事実を優先して更新する。feature 固有判断は feature docs に寄せる。 |
| `docs/templates/` | 新規 docs を作る時の型。 | 実際の feature で使って足りない欄があれば更新する。 |
| `docs/tools/` | 調査 tooling、MCP、local rebuild checklist、tool inventory。 | 開発環境や検証導線が変わったら更新する。 |
| `docs/upgrades/` | upstream 追従、version diff、migration checklist。 | upstream 差分で壊れそうな file / symbol を追加する。 |

## Reading Order

新しい作業を始める時は、次の順で読む。

1. `docs/manuals/index.md`
2. 関係する `docs/manuals/*.md`
3. 既存手順が必要なら `docs/tutorials/*.md`
4. 既存挙動の詳細が必要なら `docs/flows/*.md` または `docs/overview/*.md`
5. feature branch で実装するなら `docs/features/<feature>/`

## When to Create a Feature Folder

次のどれかに当てはまる場合は、`docs/features/<feature>/` を作る。

- 保存データ、ID、generated data、runtime option、battle flow、field flow に影響する。
- 複数 file / module をまたいで実装する。
- MVP と future work を分けないと scope が膨らむ。
- テストで設計へ戻る可能性がある。
- downstream feature への影響を追う必要がある。

軽いデータ修正や、既存 tutorial だけで完結する作業は feature folder を作らなくてもよい。

## Manual vs Tutorial

manual は「何を読むか、どこを触るか、どこで止まるか」を書く。
tutorial は「具体的にどう追加するか、どの手順で進めるか」を書く。

同じ内容を両方に重複して書かない。
manual から tutorial へリンクし、project 固有の注意だけ manual 側へ足す。

例:

| Topic | Manual role | Tutorial role |
|---|---|---|
| BGM 追加 | どの file と generated asset に影響するかを案内する。 | 実際の BGM 追加手順を書く。 |
| trainer party pool | generator / preview / trainerproc への影響を案内する。 | `.party` の pool syntax と既存 build 手順を書く。 |
| new move | ID、battle script、AI、message、test の影響を案内する。 | move 追加の具体手順を書く。 |

## Feature Docs Rule

feature docs は branch 中の作業契約として扱う。
実装中に方針が変わったら、先に owning feature docs の current decision / risks / test plan を更新する。

影響範囲は、原則として変更の原因になる owning feature docs に書く。
downstream feature の仕様を直接変える段階になったら、downstream docs に短い参照を追加する。

## Reorganization Policy

大きな docs 整理は、既存 file を移動する前に入口 docs を追加してから行う。

- まず manual / index / SUMMARY から辿れるようにする。
- 次に重複や古い記述を issue 化する。
- 最後に必要な file だけ rename / move する。

既存 `docs/tutorials/` は upstream 追従の前提になりやすいため、移動より参照整理を優先する。
