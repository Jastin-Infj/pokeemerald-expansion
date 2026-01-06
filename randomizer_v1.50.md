randomizer v1.50 現状メモ（DexNav連携含む）
================================================

実装済み
--------
- スロットマップ（slotSpecies）を生成物に出力。末尾がレア枠（slotRareStart）で、rareRateヒット時は末尾から抽選。
- ランタイム：RandomizerGetAreaRuleView で slotSpecies/Count/RareStart を参照可能。
- ランタイム：RandomizeWithAreaRule が slotSpecies を優先（useSlotMap=trueの場合）。固定/ギフトも slotMap を使う。
- DexNav:
  - ランダマイザー有効時は slotSpecies を優先してUIに反映（land/water/fishing/hidden）。allowEmpty や未定義エリアは × 表示。
  - rareStart 以降は★（既存星グラフィック流用、8x8）を重ねて視認性を上げた。
  - slotSpecies に無い種・allowEmpty エリアは R/A 入力で選択不可（SE_FAILURE）。デバッグログに1マップ1回「DexNav register blocked ...」を出力。
  - Register不可時はウィンドウに "Cannot register here."（allowEmpty/未定義系） or "Not in slot map."（スロット外）のいずれかを表示。
  - デバッグログに DexNav 読み込み件数を WARN レベルで出力（FLAG_RANDOMIZER_DEBUG_LOG が ON のとき）。
- DexNav スピードペナルティを config で無効化可能（DEXNAV_IGNORE_SPEED_PENALTY）。デフォルト TRUE。
- ビルドスクリプト: schemaVersion 1.50 対応、slotSpeciesPool 生成、rareSlots>slotCount/rareWL でエラー。
- サンプル: Route103 Fishing (rareありのold/super、goodは通常)、Route101 Hidden の slotSpecies 付きサンプルを追加。
  - Route102 Land は rareLandSample + legend_unlock を適用しつつ、default apply に starter_land/mid_land を付与（空WLエラーを防止）。

未対応/残課題
------------
- Rare枠のUI強化（★オーバーレイなど）が必要なら追加検討。
- Fishing/Hidden のサンプルデータは簡易。slotSpecies を使うテストケースの拡充が必要。
- Register時の禁止理由をより明示（現在はメッセージ＋SE_FAILUREのみ）。
- DexNav Register -> 強制遭遇の連携（現状は従来動作のまま）。
- ドキュメント整備（v1.30以前との差分、例外マップ運用、カテゴリ連携など）。

ビルド手順
----------
```
python3 dev_scripts/build_randomizer_area_rules.py --in data/randomizer/area_rules.yml --out generated/randomizer_area_rules.h --exceptions data/randomizer/exception_maps.yml --exceptions-out generated/randomizer_exception_maps.h --report
make -j4
```

デバッグTips
------------
- FLAG_RANDOMIZER_DEBUG_LOG=ON で RandR/DexNav 関連の WARN ログが出る。
- DexNav選択不可時：SE_FAILURE＋登録ウィンドウに "Cannot register here."。
- DexNav読み込みログ: `[INFO] DexNav randomizer map=... time=... land=... water=... hidden=... fish=... allowEmpty(...)=...`
