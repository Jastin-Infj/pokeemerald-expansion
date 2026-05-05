# GitHub Workflow

このプロジェクトでは、作業前に GitHub 運用を確認します。
特に docs-only の依頼、調査のみの依頼、ソース変更を含む依頼を混ぜないことが重要です。

## 開始時の確認

```sh
git status --short --branch
git remote -v
```

見るものは次です。

- 今いるブランチが作業用ブランチか。
- 未コミット差分に、自分が触っていないファイルが混ざっていないか。
- push 先が個人 fork か、アップストリームか。

## ステージングの原則

未追跡ファイルやユーザー作業が混ざることがあるため、安易に全追加しません。

```sh
git add docs/manuals/index.md docs/SUMMARY.md
```

`git add -A` は、今回の作業範囲が完全に分かっているときだけ使います。
docs-only の依頼では、docs 以外を stage しません。

## master への docs-only merge policy

`master` は upstream / RHH 由来の source code を基準にする。feature branch や別 project branch の source、include、data、tools、generated files を `master` に混ぜない。

許容するもの:

- `docs/` 配下の調査結果。
- feature branch で得た設計判断、検証記録、運用ルール。
- docs navigation、manual、test plan の更新。

禁止するもの:

- feature 実装 commit の merge。
- source / include / data / tools / generated output を含む branch merge。
- local save、ROM、screenshot、cache、debug artifact の commit。

`master` に docs を入れる前に必ず確認する。

```sh
git diff --name-only master..HEAD
```

出力が `docs/` 以外を含む場合、その branch は merge しない。docs commit だけを cherry-pick するか、`master` から docs-only branch を切り直す。

## コミット前の確認

```sh
git diff --staged
rtk mdbook build docs
```

ソース変更がある場合は、該当するビルドや動作確認も追加します。
確認していないものを「動作確認済み」と書かないようにします。

## PR 運用

既存の PR がある場合は、同じブランチに積む方針を優先します。
新しい PR を作るのは、作業目的やレビュー単位が明確に別れるときです。

PR 説明には次を残します。

- 何を変えたか。
- docs-only か、ソース変更を含むか。
- どの検証を実行したか。
- 未検証のリスクがあるか。

## 突き返すべきケース

次の場合は、実装へ進む前に docs へリスクを残し、必要ならユーザーに確認します。

- アップストリームへ直接 push するように見える。
- docs-only 指定なのにソース変更が必要になる。
- 既存の未コミット差分を戻さないと進めない。
- 種族、技、アイテム、TM/HM の ID 幅や保存形式に関係する。
- マップ追加と Fly 登録のように、複数のデータ定義を同期する必要がある。
- 既存 docs の方針と依頼内容が矛盾する。

## 禁止に近い操作

ユーザーの明示指示がない限り、次は行いません。

- `git reset --hard`
- `git checkout -- path`
- unrelated files を含む一括コミット
- アップストリームへの直接 push
- 未確認の generated file 差分の巻き込み
- docs-only merge の名目で feature code を `master` に入れること
