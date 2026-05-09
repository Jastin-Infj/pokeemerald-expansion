# GitHub Workflow

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `8d2664af9a`; GitHub PR queue checked 2026-05-09 |
| Code status | Docs-only workflow manual |
| Provenance | Local project overlay |

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
`AGENTS.md` は agent-facing documentation として扱い、ユーザーが運用ルール更新を求めた場合だけ docs-only 作業に含めます。

## master への docs-only merge policy

`master` は upstream / RHH 由来の source code を基準にする。feature branch や別 project branch の source、include、data、tools、generated files を `master` に混ぜない。

許容するもの:

- `docs/` 配下の調査結果。
- `AGENTS.md` の agent-facing workflow instruction。source / build rule を変えず、作業手順だけを更新する場合に限る。
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

出力が `docs/` と `AGENTS.md` 以外を含む場合、その branch は merge しない。docs commit だけを cherry-pick するか、`master` から docs-only branch を切り直す。

feature 実装が完了した branch では、merge 前に owning feature の
`implementation.md`、`test_plan.md`、必要な manual を更新する。実装 commit
を `master` に入れない運用の場合でも、設計判断、検証結果、manual check、
GitHub Actions を再待機しなかった理由は docs-only commit として残す。

merge checklist は `docs/team_procedures/merge_checklist.md` の
Local docs-only merge note を使う。

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

Open PR は「master に入れる許可」ではなく、review / staging shelf として扱う。
特に source / include / data / tools / generated file を含む PR は、開いたまま
でも勝手に merge しない。ユーザーが明示的に merge を頼むまで、GitHub の
merge button や `gh pr merge` は使わない。

### Open PR を残す基準

- まだ採用候補だが、feature_registry の順序では今すぐ入れない。
- CI / validation evidence があり、後で fresh branch へ分割 cherry-pick
  する価値がある。
- 大型 feature の review 単位として意味があり、branch に unique work が残る。

### Close する基準

- 後継 PR が同じ commit または同じ成果物を含んでいる。
- docs snapshot が後続 docs-only commit で `master` に反映済み。
- draft / prototype が stale で、CI failure や conflict を持ったまま queue を
  汚している。

remote branch は慎重に扱う。fully superseded / merged / unique work なしなら
削除してよい。unique work が残る draft は、PR だけ close して branch は残す。

### Open PR から master へ入れる手順

1. `docs/features/feature_registry.md` の順序と owning feature docs を確認する。
2. PR が現在の `master` に clean merge できるか確認する。
3. planned order と PR の粒度がずれている場合は、PR を直接 merge せず、
   current `master` から fresh branch を切る。
4. 必要な commit / file だけを cherry-pick または再実装する。
5. source / data / config 変更なら local make、focused check、可能なら mGBA
   runtime validation を行う。
6. `implementation.md` / `test_plan.md` / registry の status を更新する。
7. ユーザーが明示した場合だけ merge する。長い GitHub Actions は待ち続けず、
   local validation と未待機理由を handoff に残す。

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
