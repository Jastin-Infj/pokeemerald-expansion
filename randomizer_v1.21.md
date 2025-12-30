# randomizer v1.21 実装メモ

## 修正内容（実装済み）
- 生成スクリプト刷新: `dev_scripts/build_randomizer_area_rules.py`
  - rod別WL/BLオフセット（normal/rare）を持つ `RandomizerFishingRule`、normal/rare offset/length・timeSlot付きの `RandomizerAreaRule` に拡張。
  - allowEmpty/allowLegendOverride/maxRerolls/slotMode/rareRate/rareSlots を厳格バリデーション。timeSlotsは any/false/各時間帯必須、空継承は allowEmpty+null 明示のみ許容。
  - 出力をプール+offset/length方式で `generated/randomizer_area_rules.h` へ。定数ヘッダ参照を追加。
- データ更新: `data/randomizer/area_rules.yml`
  - ver1.21フォーマットへ再構成（default/rareネスト、timeSlots any/時間帯、Fishはrod別定義＋allowEmpty明示、maxSpeciesは定数利用）。
- 定数ヘッダ追加: `include/constants/randomizer_slots.h`
  - *_MAX_SLOTS と timeSlotコード値を定義。生成物とランタイムが参照。
- ランタイム対応: `src/randomizer.c`
  - 新構造に合わせたプール参照ヘルパ追加（normal/rare・rod別）。allowLegendOverride継承を統一。
  - 野生抽選を rod別WL/BL + rare pool で行い、maxRerollsはrod優先。デバッグログをWARNで新構造情報を出力。
  - トレーナー側フォールバックも新プール参照に合わせて修正。
- 生成物: `generated/randomizer_area_rules.h` を新structで生成済み。
- ビルド: `make -j4` 成功（再生成→ビルド通過を確認）。

## 主な変更ファイル
- dev_scripts/build_randomizer_area_rules.py … 生成ロジック/バリデーション/出力構造を全面更新。
- data/randomizer/area_rules.yml … 新フォーマットに合わせて再構成。
- include/constants/randomizer_slots.h … スロット上限・timeSlotコード定数を追加。
- src/randomizer.c … 新構造の参照（rod別WL/BL、rare pool、allowEmpty継承、ログ更新）。
- generated/randomizer_area_rules.h … 生成物（新struct＋プール）。

## 動作確認ログ抜粋
- Route103 Night Land: `[WARN] ... mask=1 wl=6 bl=0 attempts=4 (fallback)` → WL6/BL0・maxRerolls=3に伴うフォールバックで仕様通り。
- Route103 Night Good Rod: `[WARN] ... mask=4 wl=4 bl=0 attempts=7 (fallback)` → WLが4種でフォールバック多めだが仕様通り。
- Route103 Night Super Rod: `[WARN] ... mask=4 wl=5 bl=13 attempts=7 (fallback)` → 伝説5種WL・BL13、maxRerolls=6に伴うフォールバックで仕様通り。

## 今後のタスク候補
- テストYAMLを `dev_scripts/tests/randomizer_yamls/` に追加し、正常/エラーケースの自動検証を整備。
- 釣りレア/allowEmptyの運用をデータ側で微調整する場合はWL拡張やmaxRerolls調整で対応。
