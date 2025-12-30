# randomizer v1.22 実装メモ

## 変更概要（v1.21 との差分）
- allowEmpty をランタイムで厳格運用
  - 野生（陸/水/岩/釣り/隠し）：WL空 + allowEmpty=true なら遭遇自体をスキップ（元種へフォールバックしない）。
  - トレーナー/固定：WL空 + allowEmpty=true は手持ち枠を欠けさせず、元の種（または候補先頭）で埋める。
  - 釣り：ミニゲーム開始前に空判定。該当時は専用メッセージを出して即終了（チェイン維持、遭遇なし）。
- API/ログ拡張
  - `RandomizeWildEncounterBlocked` を標準化（blocked out-param）。`RandomizeWildEncounter` は互換ラッパ。
  - `RandomizerIsEncounterBlocked` / `RandomizerResolveTimeSlot` を追加（釣り事前判定や時間帯解決用）。
  - allowEmptyブロック時のWARNログを追加（釣り/野生）。既存のRandRログはブロック時は出ない。
- メッセージ追加
  - 釣りブロック用英語文言を追加: “No POKéMON can be\nhooked here.”
- データ更新
  - `route_101_land` を allowEmpty:true + null に変更（陸で遭遇なしにするサンプル）。  

## 変更ファイル
- `src/randomizer.c` … allowEmptyスキップの統合、blocked API公開、timeSlot解決の共通化。
- `src/wild_encounter.c` … blocked判定の呼び出し・遭遇スキップ対応（野生/大量発生/釣り）。
- `src/fishing.c` … 釣りミニゲーム前にブロック判定、専用メッセージ表示、チェイン維持。デバッグWARN追加。
- `include/randomizer.h` … 新API/blocked関数を公開。
- `src/strings.c`, `include/strings.h` … 釣りブロック用メッセージを追加。
- `data/randomizer/area_rules.yml` … Route101 Land を allowEmpty:true に変更。
- `generated/randomizer_area_rules.h` … スクリプト再生成物。

## 動作・ログ例
- Route102 釣り (allowEmpty:true):
  - `[WARN] RandR blocked map=0/17 mask=4 fishing=1`
  - `[WARN] Fishing blocked rod=0 timeSlot=3 map=0/17`
  - `[WARN] Fishing blocked end`
  - メッセージ “No POKéMON can be hooked here.” 表示後、フリーズなしで終了。
- Route101 Land (allowEmpty:true):
  - `[WARN] RandR blocked map=0/16 mask=1 fishing=0` → 陸遭遇をスキップ。

## ビルド
- `python3 dev_scripts/build_randomizer_area_rules.py`
- `make -j4` 成功（NDEBUG OFF/ON どちらでも可）。
