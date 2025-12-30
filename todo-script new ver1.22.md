# TODO-Script ver1.22（詳細設計）

## 目的
- allowEmpty=true で WL が空の場合、バニラ種にフォールバックせず「遭遇なし」にする。釣りは専用メッセージで不発扱い。トレーナー/固定は手持ち枠を欠けさせず同じポケモンで埋める。

## 現状挙動（確認済みファイル）
- `src/randomizer.c`
  - Wild: wlCount==0 かつ allowEmpty=true でも species をそのまま返すため、バニラ種が出る。
  - Trainer/Fixed: allowEmptyで空WLをスキップするロジックなし。手持ち枠はそのままリロール。
  - Fishing: allowEmptyで空でも Rare/Common 抽選を通し、バニラ種が返る。メッセージ分岐なし。
  - ログ: allowEmptyスキップ用のWARNなし。rare/logはWARN統一済み。
- `dev_scripts/build_randomizer_area_rules.py`
  - すでに allowEmpty=true でWLが空でないとエラー。allowEmpty=false/nullで空はエラー。生成物は normal/rare offset/len。
- `data/randomizer/area_rules.yml`
  - Route102/103 などで allowEmpty+null を付け、空WLを意図的に作成済み。

## 改修方針（詳細）
### ランタイム（src/randomizer.c）
1) allowEmptyスキップ
   - Wild: GetRulePools で wlCount==0 && allowEmpty → 遭遇なしで即return（speciesは変えない）。WARNログ「[WARN] RandR allowEmpty: no candidates, skipping encounter」。
   - Fishing: rod別で同様判定。空ならミニゲームに入る前に不発扱い＋メッセージ表示してスキップ（Encounter結果なし）。WARNログを一度出す。
   - Trainer/Fixed: wl空+allowEmptyなら「同じポケモンで埋める」方針。現種を保持 or fallback候補先頭で埋める。枠欠けはしない。
2) メッセージ（釣り）
   - 新メッセージ: “No Pokémon can be hooked here.”
   - 表示タイミング: Old/Good/Super Rod使用直後（SELECT登録も同様）、ミニゲーム開始前のリール直後。
3) ログ
   - allowEmptyスキップ時にWARNログを追加（野生/釣り）。通常のRandRログは発生しない。

### 生成スクリプト・データ
- スクリプト: 現状の厳格バリデーションを維持（allowEmpty=falseで空はエラー、allowEmpty=trueで空以外はエラー）。
- データ: 変更不要（Route102/103のallowEmpty+null指定はこの挙動を前提に機能）。

## 影響範囲（事前調査）
- `src/randomizer.c`:
  - Wild: RandomizeWildEncounter の allowEmpty分岐追加。
  - Fishing: SelectFishingRule 後の空判定＋メッセージ表示導線追加。GetRulePoolsの使い方を拡張。
  - Trainer/Fixed: RandomizeWithAreaRule呼び出し部またはフォールバック部で「同じポケモンで埋める」処理追加。
  - ログ: DebugLogWildRare/DebugLogRandomizationと別にallowEmptyスキップ用WARNを出す処理を挿入。
- メッセージリソース:
  - 新英語メッセージ1件を適切なテキストテーブルに追加（既存の釣りメッセージとは別）。
- スクリプト/データ:
  - バリデーションは既に成立。データ側変更なし（allowEmpty+null を利用している箇所が、今回の挙動で「出現なし」になる）。

## 作業手順案
1. `src/randomizer.c`
   - Wild/Trainer/Fixed: wlCount==0 && allowEmptyの分岐でスキップ処理を追加（Trainerは枠埋め）。WARNログ追加。
   - Fishing: rod別判定で空ならメッセージ表示→処理終了。バニラへのフォールバックをやめる。
2. メッセージ追加
   - “No Pokémon can be hooked here.” をテキストリソースへ追加し、釣り不発時に表示。
3. ビルド・動作確認
   - `python3 dev_scripts/build_randomizer_area_rules.py` → `make -j4`
   - 釣り/野生/トレーナーでallowEmpty空のパスを確認（WARNログ/メッセージが出ること）。

## 開発フロー（具体）
- Step 0: 現行生成物を再生成しビルドを通す（基準作成済み）。
- Step 1: ランタイム実装
  - Wild: wlCount==0 && allowEmptyで即スキップreturn。WARNログ追加。
  - Trainer/Fixed: allowEmptyで空の場合、現ポケモンor候補先頭で埋める分岐を追加（枠欠け防止）。WARNログ追加。
  - Fishing: rod別に空判定→メッセージ表示→スキップ（ミニゲームに入らない）。WARNログ追加。
  - 新メッセージIDを定義し、英語文言をテキストリソースに追加。
- Step 2: ビルド・実機確認
  - `python3 dev_scripts/build_randomizer_area_rules.py` → `make -j4`
  - Route102/103のallowEmpty箇所で野生/釣り/トレーナーが出現しないこと、ログ/メッセージが出ることを確認。
- Step 3: 調整・ドキュメント
  - メッセージ位置やログ文言を必要に応じ調整。
  - `todo ver1.22.md` で最終仕様を更新（実装完了メモ）。

## Encounterスキップ実装の具体案（関数レベル）
- Wild/水/岩/隠し:
- `RandomizeWildEncounter` は現状speciesのみ返すため、遭遇キャンセルできない。破壊的変更を避けるため、out-paramでblockedを返すラッパを追加し、呼び出し元（`GenerateFishingWildMon`等の上位フロー in wild_encounter.c）で blocked==true のときバトル開始を止める。`SPECIES_NONE`は使わない。
- 釣り:
  - `fishing.c` のミニゲーム開始前（GotBite/ChangeMinigame手前）で randomizer判定を挟み、blockedならミニゲームに入らずメッセージ表示→終了。
  - `DoesCurrentMapHaveFishingMons` はバニラテーブルを見るだけなので、randomizer専用の「allowEmptyでblockedか」を問い合わせるAPIを追加し、そちらで判定する。
- トレーナー/固定:
  - `RandomizeWithAreaRule` に blockedフラグを渡し、blockedなら「手持ち枠を欠けさせない」ために現ポケモンまたは候補先頭で埋める処理を追加。
- ログ:
  - blocked時に1回だけWARN（例: “[WARN] RandR allowEmpty: no candidates, skipping encounter”）を出す。釣りも不発判定時にWARN。
- Chain Fishing/Streak:
  - allowEmptyで釣り遭遇をスキップした場合、バトルが発生しないためストリークは維持（リセットしない）。
