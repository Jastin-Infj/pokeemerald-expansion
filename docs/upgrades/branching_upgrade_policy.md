# Branching and Upgrade Policy

調査日: 2026-05-02

この文書は、この fork を長期的に upstream 追従しながら独自機能を増やすための branch 運用方針を定義する。現時点では実装・改造は行っていない。

## Purpose

- `main` をどの程度 upstream に近く保つかを明確にする。
- GitHub の Sync fork / Update branch と、手元で作る upgrade branch / feature branch の使い分けを決める。
- ランダムマイザー、トレーナーバトル前選出、TM shop 化、field HM removal などを「参考書 branch」から差分採用できる運用にする。

## Important Distinction

GitHub の Sync fork / Update branch は便利だが、これは「upstream の commit を fork の branch へ取り込む」操作であって、独自機能の移植可否を判断する設計レビューではない。

| Situation | Sync fork button is enough? | Reason |
|---|---:|---|
| fork の `main` に独自 commit がほぼなく、upstream に fast-forward できる | Often yes | conflict がなければ短時間で追従できる。 |
| `main` に独自 source 改造が積まれている | No | upstream 変更と独自変更が混ざり、conflict や挙動差分を理解しにくい。 |
| release tag `expansion/1.15.2` へ上げたい | Maybe | GitHub UI は branch sync が中心。release tag 単位の確認、build、docs 更新は別途必要。 |
| 大型 feature を入れながら upstream 追従したい | No | feature branch / upgrade branch / docs branch を分けて差分を管理する方が安全。 |

結論: ボタンで済む時は使ってよい。ただし、この project では今後独自改造が大きくなるため、「ボタンで更新して終わり」を標準手順にはしない。

## Upstream Visibility Rule

この project は報告・調査用途も含むため、PR の target repository を厳密に区別する。

原則:

- `RHH` / `rh-hideout/pokeemerald-expansion` など親元 upstream、コミュニティ運営 repo、有志プロジェクト、他者が管理する shared repo へ、事前許可なしに PR を作成しない。
- PR を作れるのは、自分の fork / 自分が管理する repo、または明示的に許可された repo / branch だけ。
- 「branch を作って検証すること」と「PR を作って他者に見える review request にすること」は別扱いにする。
- 許可が曖昧な場合は upstream / shared repo へ PR を作らず、local branch、fork branch、compare URL、diff summary、docs report で止める。
- GitHub CLI や GitHub app が使える状態でも、upstream / shared repo を target にする PR 作成は別途明示確認を取る。

この rule は upstream RHH だけでなく、共同管理 repo や有志メンバーが見る repository にも適用する。自分の fork 内に PR や branch が見えること自体は許容できるが、親元 upstream に user name、PR、commit、review request を出してはならない。

今回の教訓:

- `upgrade/1.15.2` のような検証 branch を作ること自体は有効。
- 自分の fork / origin 内で PR を作ることは、target が自分の repo に閉じている限り許容範囲。
- ただし、合意なしに upstream RHH や shared repo へ PR として出すと、他メンバーや有志プロジェクト側に不要な review / merge 判断を発生させるため NG。
- upgrade 検証の結果は、まず docs / report として共有する。upstream / shared repo への PR は project owner / upstream maintainer が許可した時だけ作成する。
- PR #3 は `Jastin-Infj/pokeemerald-expansion` の fork 内に作成された draft PR であり、RH Hideout upstream へは送っていない。ただし誤解を避けるため close 済み。

## Branch Roles

推奨 branch 役割:

| Branch kind | Example | Role | Merge policy |
|---|---|---|---|
| stable base | `main` / `master` | project の採用済み安定版。できるだけ upstream release に近く保つ。 | 管理者が許可した方法でだけ更新する。 |
| upgrade trial | `upgrade/1.15.2`, `upgrade/1.15.3` | upstream release 追従を試す branch。conflict、build、docs impact を確認する。 | 報告用 branch。自分の fork 内 PR は可。upstream / shared repo PR は明示許可がある時だけ。 |
| docs investigation | `codex-docs-v15-investigation` | 調査 Markdown、設計メモ、差分分析を育てる branch。 | docs が安定した節目で取り込み候補にする。PR は target repository を確認してから作る。 |
| feature implementation | `feature/battle-selection`, `feature/tm-shop-migration` | 実装作業用 branch。 | できるだけ小さい差分に分ける。upstream / shared repo PR は許可された時のみ。 |
| reference / prototype | `reference/randomizer`, `prototype/dexnav-ui` | 参考書 branch。動く保証より調査・比較・試作を優先する。 | 原則そのまま `main` へ merge しない。必要差分だけ採用する。 |
| upstream refs | `RHH/expansion/1.15.x` tags / remote refs | upstream の正本。 | 編集しない。diff の比較元。 |

## Recommended Main Policy

`main` は「全部を突っ込む場所」ではなく、「採用済みの安定ベース」として扱う。

- `main` へ直接作業 commit しない。
- upstream 追従は `upgrade/*` branch で試す。
- 実装は `feature/*` branch で行う。
- docs は `docs` branch で育て、区切りごとに `main` へ入れる。
- reference branch は比較・試作・調査に使い、直接 `main` へ merge しない。

この方針なら、今後 1.15.3 / 1.16.x が来た時も、`main` と upstream の差分を小さく保ちやすい。

## How to Use Reference Branches

「15.2 の中途半端なプロジェクト branch」「ランダムマイザー参考書 branch」の考え方は有効。

使い方:

1. `reference/randomizer` のような branch で外部実装・試作・調査を行う。
2. その branch は動作確認や理解のために使う。
3. `main` へ直接 merge せず、必要な考え方・関数・data layout だけを抽出する。
4. 採用する時は `feature/randomizer` branch を新しく切り、この repo の現行 baseline に合わせて実装する。
5. 差分を docs に残し、なぜその部分だけ採用したかを記録する。

避けること:

- reference branch をそのまま `main` へ merge する。
- upstream 追従 branch と feature 試作 branch を兼用する。
- generated files や asset migration の巨大差分と feature logic を同じ PR に混ぜる。

## Upgrade Flow

1. upstream tag / changelog / compare を確認する。
2. `main` から `upgrade/<version>` を切る。
3. `RHH` の対象 release を取り込む。
4. conflict が出たら、まず build / asset migration / generated data の conflict を解決する。
5. `docs/upgrades/upstream_diff_checklist.md` で影響範囲を確認する。
6. build / relevant tests を走らせる。
7. `docs/upgrades/<version>_impact.md` を更新する。
8. report として branch 名、compare、build 結果、risk を共有する。
9. 自分の fork 内で review したい場合は、自分の repo を base にした PR を作ってよい。
10. upstream RHH / shared repo へ PR を作るのは、project owner / maintainer から明示許可がある場合だけ。
11. merge 後、active docs / feature branch に必要なら `main` / `master` を取り込む。

「active docs / feature branch に main を取り込む」とは、古い baseline の branch が最新 `main` とズレ続けないようにすること。長期 branch では定期的に行う。

## Docs Branch Policy

今回の `codex-docs-v15-investigation` は残してよい。

ただし docs の扱いは二段階にする。

| Stage | Meaning |
|---|---|
| docs branch | 調査中、追記中、未確認事項あり。自由に更新する作業場。 |
| main | 採用済みの設計台帳。実装者が参照してよい baseline docs。 |

つまり、docs branch は残すが、確定した docs は節目で `main` へ入れる。`main` に docs を全部入れるか迷う場合は、まず docs-only PR として review し、source 改造 PR と分離する。

## PR Policy

PR は target repository で扱いを分ける。

OK:

- 自分の fork / `origin` 内で、base も head も自分の repo に閉じている PR。
- 自分の fork 内で、project owner が明示的に PR 作成を許可した場合。
- 事前に合意された review flow があり、base branch / head branch が指定されている場合。
- docs-only / feature / upgrade など差分範囲が明確で、関係者が review する前提がある場合。

NG:

- upstream RHH など外部コミュニティ repo へ、事前根回しなしに PR を送る。
- PR base repository が `rh-hideout/pokeemerald-expansion` になっている PR を許可なしに作る。
- 有志メンバーが管理する shared repo へ、許可なしに PR を作る。
- 親元 upstream に自分の user name、commit、PR、review request が見える形にする。
- upstream / shared repo に対して「とりあえず見えるようにする」目的で PR を作る。
- upgrade 検証 branch を、合意なしに merge 候補として提示する。

PR を作らない場合の代替:

- local branch で検証する。
- fork branch へ push して、PR ではなく compare URL を共有する。
- docs report に diff summary / build result / risk を書く。
- 必要なら patch file や commit SHA を報告する。

## Naming Convention

推奨 branch 名:

| Purpose | Pattern | Example |
|---|---|---|
| upstream upgrade | `upgrade/<version>` | `upgrade/1.15.2` |
| docs investigation | `docs/<topic>` or existing `codex-docs-*` | `codex-docs-v15-investigation` |
| feature | `feature/<topic>` | `feature/battle-selection` |
| reference | `reference/<topic>` | `reference/randomizer` |
| prototype | `prototype/<topic>` | `prototype/champions-selection-ui` |
| hotfix | `hotfix/<topic>` | `hotfix/build-instructions` |

## Open Questions

- `main` を常に upstream latest release にするか、安定確認済み release で止めるかは運用判断が必要。
- docs branch をいつ `main` に merge するかの cadence は未決定。
- reference branch の保存期限は未決定。大きく古くなった branch は docs に要点を移して閉じる方がよい可能性がある。
- `upgrade/1.15.2` branch は検証用に作成済み。PR #3 は運用方針に合わせて close 済み。remote branch を残すか削除するかは owner 判断。
