# Team Box #84 と Smart AI の統合準備設計

## 結論

`integration/runtime-lab-current-1.16.1` を検証用の基点にし、Team Box PR #84 の差分を先にローカルstagingする。その後、Smart AI board-search の既存コミットを同じ系統へ積み、ローカルテスト・直接mGBA検証・引き継ぎ文書を更新する。

masterへのmerge、GitHubへのpush、公開PRの変更はこの作業の対象外とする。

## 背景と基点

- #84の公開PRは `fix/runtime-lab-battle-team-incomplete-20260718` から `integration/runtime-lab-current-1.16.1` へのdraft PRである。
- 検証用基点は `integration/runtime-lab-current-1.16.1`（現時点の記録では `4e960aecea`）とする。
- 元の `integration/smart-ai-board-search-20260720` にはユーザー所有のdirty差分があるため、そこでは編集・stage・commitを行わない。
- Smart AI側には、board-search本体、handoff文書、project-local mGBA wrapperの既存コミットがある。これらは内容を再実装せず、基点への適用可否とruntime evidenceを確認する。

## 実施順序

### 1. Team Box #84を先にstagingする

新しいローカル統合ブランチへ、#84のコミット系統を適用する。適用後にBattle Teamのfocused check、all/debug/full check、mdBook checkを実行し、既知のmanual follow-upを直接mGBAで再確認する。

### 2. Smart AIの既存差分を積む

Team Box側の検証が終わったstagingブランチを基点に、Smart AI board-searchとproject-local mGBA toolingのコミットを適用する。既存のunit・snapshot・planner・board evaluationの検証を再実行し、コード変更が必要になった場合だけTDDで先に失敗テストを追加する。

### 3. runtime evidenceを揃える

直接mGBA wrapperで、次の順に検証する。

1. debug ROMが起動すること
2. `Party -> Joint Trace Double` の導線に入れること
3. v5 action-logをエクスポートできること
4. predicted boardとactual boardの一致を確認できること
5. Team Boxの登録・消費・再起動・制限事項を確認できること

mGBAまたは画面操作が環境要因で実行できない場合は、試したコマンド、失敗点、未確認の受け入れ条件をtest planへ記録する。未確認を成功扱いにはしない。

### 4. 文書を更新する

差分の系譜、実行コマンド、結果、mGBAの未確認項目、Actionsを再待機していないこと、残るリスクを次の既存文書へ反映する。

- `docs/features/battle_team_boxes/test_plan.md`
- `docs/features/smart_gimmick_ai/test_plan.md`
- `docs/manuals/mgba_live_mcp_manual.md`（wrapperや操作手順に変更がある場合のみ）
- `docs/features/smart_gimmick_ai/smart-ai-board-search-handoff.md`（存在する場合）

## 受け入れ条件

- 元のdirty worktreeのファイルを新しいstagingへコピー・stage・commitしない。
- Team Box #84のfocused checkと既存のビルド系チェック結果が再現できる。
- Smart AIの既存テスト群が再現できる。
- runtime検証は、成功した証拠と未確認項目を分けて記録する。
- stagingブランチのcommit系譜と、公開PR #84との関係が文書から追跡できる。
- masterへのmergeと外部pushを行わない。

## 非対象と受け入れるリスク

- active/new Dynamax、spread・multi-hit、status Z、広い2〜3ターンのswitch/Protect/sacrifice treeは、既存のbounded simulatorの対象外である。
- Elite Singleの完全なfresh action-logは別途必要であり、取得できない場合は未完了としてhandoffする。
- Team Boxの残りmanual follow-upは、実行できた項目だけを証拠付きで完了扱いにする。

## 文書チェック

`lint.py --genre tech` では `low_burstiness` が1件出た。検証順序と受け入れ条件を短い列挙で固定した技術設計書のため、意味を変える分割はせず、このfindingを残す。
