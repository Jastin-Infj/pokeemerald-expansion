# GitHub Workflow

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-07-18 |
| Baseline | fork `master` `b65afd70d7`; upstream release `expansion/1.16.2` at `ad0fd4d17f` |
| Code status | Docs-only workflow manual |
| Provenance | Local project overlay |

このプロジェクトでは、作業前に GitHub 運用を確認します。
特に docs / Lua-only の依頼、調査のみの依頼、ソース変更や画像 assets を含む依頼を混ぜないことが重要です。

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
docs / Lua-only の依頼では、eligible files 以外を stage しません。Markdown docs
は `docs/` と、`CREDITS.md` のような root documentation `.md` を含みます。
`AGENTS.md` は agent-facing documentation として扱い、ユーザーが運用ルール更新を求めた場合だけ docs / Lua-only 作業に含めます。
Lua script files are allowed in the same dedicated PR when they are user-approved
shortcuts, debug commands, or validation automation. This means the script files
themselves may be committed; they are not limited to docs describing scripts.
PNG などの画像 assets は docs / Lua-only PR には含めません。

## Branch Roles

この repository では、`master` を「公式 release tag 追従の受け皿」として扱う。
local feature implementation を直接 `master` に積むと、upstream upgrade 時に
source conflict と再適用判断が増えるため、通常の開発 branch と分ける。

| Branch kind | Role | Allowed content |
|---|---|---|
| `master` | 採用済み公式 `expansion/<version>` source baseline + local docs / Lua script overlay | Markdown docs, workflow-only `AGENTS.md`, approved Lua script files; source-like tree と画像 assets は原則触らない |
| `docs/*` | `master` へ入れる調査・運用・handoff docs / Lua scripts | Markdown docs, 必要な `AGENTS.md`, approved Lua script files |
| `feature/*` | 1 feature の実装と検証 | source / include / data / graphics / tools を含んでよい。`master` へは直接 merge しない |
| `shelf/runtime-<version>/*` | 完成済みで再利用する単体実装 | immutable evidence / re-apply source。削除せず凍結する |
| `integration/active-runtime-<version>` | その世代で唯一の現行 playable 開発基準 | 新機能を直接実装せず、feature PR だけを順番に統合する |
| `snapshot/runtime-<version>/*` | 完成時点の複合 runtime 保存 | immutable comparison / recovery point。削除せず凍結する |
| `RHH` remote | release 検出と比較専用 | fetch / compare only。push せず、moving `RHH/master` は採用しない |

GitHub の Sync Fork や moving upstream branch の merge は使わない。local 実装を
遊べる状態で残したい場合は `integration/*` に積み、公式 release tag の intake 後に
current `master` から作り直す。

## Version Tags And Branches

- 公式 `expansion/<version>` tag を upstream version の正本とする。同じ commit を示す
  ためだけの新規 branch は作らない。
- branch は、PR、継続開発、re-apply、比較、復旧に使う work line / evidence とする。
- local 統合版の重要な検証完了点には、たとえば
  `local/runtime-1.16.2/validated-20260718` のような annotated tag を付け、upstream
  base、integration branch、検証 evidence を tag message に残す。
- 公開済み tag を別 commit へ付け替えない。追加修正は新しい tag として記録する。
- 既存の exact-release copy branch は直ちに削除しない。canonical tag で代替でき、
  unique work と recovery value がないことを別途確認してから削除判断する。

## Upstream Release Intake

2026-07-18 時点で、fork `master` は source version `1.16.1`、RHH の最新確認済み
release は `expansion/1.16.2` (`ad0fd4d17f`) である。この値は固定ルールではなく、
次回 1.17 以降でも intake 開始時に再取得する観測値として扱う。

`RHH/master` は release 検出前の開発途中を確認する比較資料にすぎず、intake source
ではない。commit が採用可能になるのは、pokeemerald-expansion が GitHub 上で公式の
non-draft / non-prerelease Release を公開し、その `expansion/<version>` tag に commit
が含まれた後だけである。master 上に先行して存在する修正は、重要に見えても個別採用
しない。

新しい upstream release が出たときは、次の順番を守る。

1. GitHub Release metadata と `RHH` remote の公式 release tag / commit を取得し、
   release が draft / prerelease でないこと、移行元 fork `master`、既存 active
   integration、採用予定 shelf の commit を記録する。
2. 既存 playable line を `snapshot/runtime-<old-version>/*` として凍結する。削除や
   force-push は行わない。
3. current `master` から `upgrade/<new-version>-intake-*` を作り、release tag の
   upstream-authored source / data / tools / generated 差分だけを取り込む。
4. intake PR では pinned official release tag の変数名、API、struct、save/data
   layout、生成形式、ownership、config default を正とする。local feature の意図を
   released contract 上へ移植し、古い定義を復元しない。
5. conflict は file 単位の blanket `ours` / `theirs` で終わらせない。producer、
   consumer、serialization、generated-data tool、test を確認し、採用判断を記録する。
6. generated data は新 release の tool/schema で再生成する。古い branch の生成物を
   そのまま持ち込まない。
7. upstream intake を `master` へ PR で反映した後、fresh
   `integration/active-runtime-<new-version>` を作る。
8. 採用 feature を manifest 順に separate PR で re-apply し、各段階と最終合成で
   normal/debug build、focused/full checks、mGBA evidence を残す。

`master` に source-like files を入れられる通常の例外は、この upstream intake だけで
ある。local implementation は intake PR に混ぜず、active integration 向けの別 PR に
する。GitHub の Sync Fork button は moving upstream branch を対象にするため、この
release intake には使用しない。

### Pinned Official Release Wins During Porting

pinned official release tag の新設計を local feature より優先する。たとえば release で
field 名や型、構造体の ownership、保存形式、ID 幅、config、generated table が変わった
場合、local branch の古いファイルを戻すのではなく、新 API / 新 data structure を使って
同じ機能意図を再実装する。moving `RHH/master` にだけ存在する未リリース設計は、この判断
根拠にしない。compatibility shim は、現行 feature contract に必要で、期限と削除条件を
docs へ残せる場合だけ追加する。

### Historical 16.0 Runtime Lineage

`snapshot/runtime-1.16.0/full-stack-20260531`（旧名
`integration/runtime-dev-16-20260531`）/ closed PR #69 は、1.15.3 runtime
integration を upstream 16.0 baseline へ replay した completed snapshot として扱う。これは
「15.3 で完成扱いにした実装が、16.0 の API / config / generated-data 方針へ
変換された状態」を保存する branch であり、今後の新規 runtime 実装を直接積み続ける
dev branch ではない。

この分け方の目的は、レビュー時に次の 3 点を分けて比較できるようにすること。

- current `master`: upstream 16.0 source baseline + docs / Lua-only overlay。
- completed 16.0 port snapshot: 15.3 から 16.0 へ移植済みの runtime 集合。
- future 16.0 dev branch: snapshot 以後の新規実装や再実装を載せた差分。

今後の作業では、 clean PR / clean review が必要な runtime feature は current
`master` から fresh `feature/*` または `integration/*` branch を作る。過去の 15.3
branch や PR #69 snapshot は参考元であって、絶対にそのまま再採用するものではない。
16.0-native に最適化し直した実装は、新しい branch、docs、validation evidence、PR を
持つ。

一方で、すでに移植済みの runtime 集合を遊べる dev 環境として維持したい場合は、
completed snapshot を複製して別名の `integration/*` branch を作り、その複製に新規差分を
積む。元の snapshot は比較対象として残す。これにより、問題が出たときに
`master` vs completed snapshot vs new dev branch の差分を切り分けやすくなる。

`master` へ入れるのは、15.3 / 16.0 の handoff docs、branch policy、validation
evidence などの docs / Lua-only 差分だけにする。PR #69 の runtime source / data /
tools / generated output を `master` へ merge しない。

## Work Type Labels

作業開始時と PR 作成時は、次のどれかに分類して記録する。

| Type | Meaning | Master handling |
|---|---|---|
| docs / Lua-only | Markdown docs、必要な workflow-only `AGENTS.md`、approved Lua script files だけを更新する。Lua scripts are allowed for shortcuts, debug commands, and validation automation. | `master` へ取り込み候補にしてよい。 |
| new implementation | current `master` には存在しない source / include / data / graphics / tools / generated 差分を新しく作る。 | `feature/*` / `integration/*` に置き、`master` へ直接 merge しない。 |
| upstream intake | 新しい公式 `expansion/<version>` Release tag の source / data / tool / generated contract を fork baseline に取り込む。 | `upgrade/*` PR で `master` に入れてよい唯一の通常 source 例外。moving `RHH/master` と local implementation を混ぜない。 |
| re-apply / port | 過去の validated branch にある実装 slice を current generation の selected base から切り直した branch へ載せる。 | runtime PR として扱う。新規実装とは呼ばず、移植元 commit、upstream差分、再検証結果を書く。 |
| docs handoff | runtime 実装は持ち込まず、branch evidence、採否判断、残リスクだけを docs に残す。 | docs / Lua-only branch で `master` へ取り込み候補にしてよい。 |

`re-apply / port` は「同じものをもう一度実装した」作業ではない。PR title、
PR body、`implementation.md`、`test_plan.md` では、移植元 branch / commit、
current base、再検証結果を明記する。source-like files が含まれる限り、
docs / Lua-only として扱わない。

## Master Docs-Only Merge Policy

`master` は採用済み公式 `expansion/<version>` Release tag 由来の source code を基準にする。feature branch、moving `RHH/master`、別 project branch の source、include、data、graphics、tools non-Lua、generated files を `master` に混ぜない。

この節の excluded-path gate は local docs / Lua / feature handoff に適用する。前述の
`Upstream Release Intake` は別 work type であり、pinned official release tag 由来であることを
監査した source / data / tools / generated 差分を dedicated upgrade PR から `master` へ
入れられる。upstream intake に local feature implementation を混ぜてはならない。

許容するもの:

- Markdown documentation files。`docs/` 配下の調査結果と、root documentation
  `.md` such as `CREDITS.md` を含む。
- `AGENTS.md` の agent-facing workflow instruction。source / build rule を変えず、作業手順だけを更新する場合に限る。
- ユーザーが許可した Lua script files。用途は shortcuts、debug commands、validation automation に限る。
- feature branch で得た設計判断、検証記録、運用ルール。
- docs navigation、manual、test plan の更新。

禁止するもの:

- feature 実装 commit の merge。
- source / include / data / graphics / tools non-Lua / generated output を含む branch merge。
- PNG、JPG、BMP などの画像 assets。credit と source URL は docs に残してよいが、画像ファイル本体は実装 PR に入れる。
- local save、ROM、screenshot、cache、debug artifact の commit。

`master` に docs を入れる前に必ず確認する。

```sh
git diff --name-only master..HEAD
```

出力が Markdown docs、`AGENTS.md`、approved Lua script files 以外を含む場合、その branch は merge しない。eligible commit だけを cherry-pick するか、`master` から docs / Lua-only branch を切り直す。

feature 実装が完了した branch では、merge 前に owning feature の
`implementation.md`、`test_plan.md`、必要な manual を更新する。実装 commit
を `master` に入れない運用の場合でも、設計判断、検証結果、manual check、
GitHub Actions を再待機しなかった理由は docs / Lua-only commit として残す。

validated branch が存在する場合も、`master` に持ち込むのは evidence と判断、必要な Lua script files だけにする。
たとえば `feature/no-random-encounters` のように 3 file 実装と mGBA evidence が
ある branch でも、docs / Lua-only 依頼では `include/` や `src/` を cherry-pick しない。
runtime 実装が必要になった時点で、current `master` から新しい `feature/*` または
`integration/*` branch を切り、必要な source slice だけ再適用して検証する。
画像 assets も同じ扱いにする。たとえば `.png` icon は、credit / source URL を
docs に記録しても、画像ファイル本体はそれを消費する実装 PR に含める。

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
特に source / include / data / graphics / tools / generated file を含む PR は、開いたまま
でも勝手に merge しない。ユーザーが明示的に merge を頼むまで、GitHub の
merge button や `gh pr merge` は使わない。

### Open PR を残す基準

- まだ採用候補だが、feature_registry の順序では今すぐ入れない。
- CI / validation evidence があり、後で fresh branch へ分割 cherry-pick
  する価値がある。
- 大型 feature の review 単位として意味があり、branch に unique work が残る。

### Close する基準

- 後継 PR が同じ commit または同じ成果物を含んでいる。
- docs / Lua snapshot が後続専用 commit で `master` に反映済み。
- draft / prototype が stale で、CI failure や conflict を持ったまま queue を
  汚している。

remote branch は evidence / recovery point として原則保存する。completed reusable
implementation は `shelf/*`、frozen combined runtime は `snapshot/*` へ rename する。
削除は unique work と recovery value がないことを確認し、ユーザーが明示した場合だけ行う。

### Open PR から master へ docs / Lua を入れる手順

1. `docs/features/feature_registry.md` の順序と owning feature docs を確認する。
2. PR が source / include / data / graphics / tools non-Lua / generated files を含むか確認する。
3. source-like files や画像 assets を含む PR は直接 merge しない。
4. current `master` から docs / Lua-only branch を切り、必要な Markdown docs /
   `AGENTS.md` / approved Lua script files だけを cherry-pick または
   再編集する。
5. 実装 branch の commit、diff scope、validation evidence、未反映理由を
   `implementation.md` / `test_plan.md` / registry に記録する。
6. `rtk git diff --name-only master..HEAD` が Markdown docs、`AGENTS.md`、approved
   Lua script files だけであることを確認する。
7. ユーザーが明示した場合だけ merge する。長い GitHub Actions は待ち続けず、
   local validation と未待機理由を handoff に残す。

### 実装を試す / 遊べる状態へ持っていく手順

1. current generation に `integration/active-runtime-<version>` がある場合、playable-line
   feature はその pinned head から `feature/<name>` を切る。standalone compatibility
   shelf または upstream intake 中は current `master` を使う。
2. validated branch から必要な source slice だけを cherry-pick / re-apply する。
3. 古い docs を持ち込んで current docs を巻き戻さない。
4. source / data / config 変更に応じて local make、focused check、可能なら
   mGBA runtime validation を行う。
5. 検証結果を owning feature docs に追記する。
6. 実装 PR は staging shelf として扱い、`master` merge は別途ユーザー確認を取る。

PR 説明には次を残します。

- 何を変えたか。
- docs / Lua-only か、ソース変更や画像 assets を含むか。
- new implementation か、re-apply / port か、docs handoff か。
- re-apply / port の場合は、移植元 branch / commit と current base。
- どの検証を実行したか。
- 未検証のリスクがあるか。

## 突き返すべきケース

次の場合は、実装へ進む前に docs へリスクを残し、必要ならユーザーに確認します。

- アップストリームへ直接 push するように見える。
- docs / Lua-only 指定なのにソース変更や画像 assets が必要になる。
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
- docs / Lua-only merge の名目で feature code や画像 assets を `master` に入れること
