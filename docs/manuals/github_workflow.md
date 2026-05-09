# GitHub Workflow

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `050a5ab7a3`; GitHub PR queue checked 2026-05-09 |
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

## Branch Roles

この repository では、`master` を「upstream 追従の受け皿」として扱う。
local feature implementation を直接 `master` に積むと、upstream upgrade 時に
source conflict と再適用判断が増えるため、通常の開発 branch と分ける。

| Branch kind | Role | Allowed content |
|---|---|---|
| `master` | upstream / RHH 由来の source baseline + local docs overlay | `docs/`, workflow-only `AGENTS.md`; source-like tree は原則触らない |
| `docs/*` | `master` へ入れる調査・運用・handoff docs | `docs/`, 必要な `AGENTS.md` |
| `feature/*` | 1 feature の実装と検証 | source / include / data / tools を含んでよい。`master` へは直接 merge しない |
| `integration/*` | 複数 feature を重ねた playable / review 用 branch | current `master` から作り直し、必要な slice だけ再適用する |
| upstream remote | 新バージョン取り込み元 | fetch / compare only。push しない |

GitHub の fork sync や upstream merge は便利だが、local implementation を含む
`master` で実行すると source baseline が壊れやすい。sync 前に `master` が
docs-only baseline であることを確認する。local 実装を遊べる状態で残したい場合は
`integration/*` に積み、upstream 更新後に current `master` から作り直す。

## Master Docs-Only Merge Policy

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

validated branch が存在する場合も、`master` に持ち込むのは evidence と判断だけにする。
たとえば `feature/no-random-encounters` のように 3 file 実装と mGBA evidence が
ある branch でも、docs-only 依頼では `include/` や `src/` を cherry-pick しない。
runtime 実装が必要になった時点で、current `master` から新しい `feature/*` または
`integration/*` branch を切り、必要な source slice だけ再適用して検証する。

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

### Open PR から master へ docs を入れる手順

1. `docs/features/feature_registry.md` の順序と owning feature docs を確認する。
2. PR が source / include / data / tools / generated files を含むか確認する。
3. source-like files を含む PR は直接 merge しない。
4. current `master` から docs-only branch を切り、必要な docs / `AGENTS.md`
   だけを cherry-pick または再編集する。
5. 実装 branch の commit、diff scope、validation evidence、未反映理由を
   `implementation.md` / `test_plan.md` / registry に記録する。
6. `rtk git diff --name-only master..HEAD` が `docs/` と `AGENTS.md` だけで
   あることを確認する。
7. ユーザーが明示した場合だけ merge する。長い GitHub Actions は待ち続けず、
   local validation と未待機理由を handoff に残す。

### 実装を試す / 遊べる状態へ持っていく手順

1. current `master` から `feature/<name>` または `integration/<name>` を切る。
2. validated branch から必要な source slice だけを cherry-pick / re-apply する。
3. 古い docs を持ち込んで current docs を巻き戻さない。
4. source / data / config 変更に応じて local make、focused check、可能なら
   mGBA runtime validation を行う。
5. 検証結果を owning feature docs に追記する。
6. 実装 PR は staging shelf として扱い、`master` merge は別途ユーザー確認を取る。

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
