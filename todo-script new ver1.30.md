## 目的と前提（ver1.30 実装設計）
- ランダマイザーの「未定義エリア＝遭遇なし」運用と、カテゴリフィルタ（legend/mythical/ub/paradox/subLegend 等）を `specialOverrides` で厳格化する。フォームは別種扱い。
- `schemaVersion: 1.30` を必須とし、キー集合＋型が既知スキーマと完全一致する場合のみ将来値を許容。未指定/未知はエラー。
- allowEmpty: true ならスロットをゼロ扱い（遭遇なし）。null 以外の値があればエラー。釣りも同様にスキップし、ミニゲーム前に専用メッセージで中断。
- デバッグ用のバニラ遭遇トグルはデバッグビルド専用で全種別を一括許可（カテゴリフィルタは無効化しない）。本番ビルドでは強制OFF。

## 現状確認（ソース読み取りのみ、修正なし）
- 入力YAML: `data/randomizer/area_rules.yml`（v1.21仕様のまま）。
- 生成スクリプト: `dev_scripts/build_randomizer_area_rules.py`  
  - 構造体 `RandomizerAreaRule` / `RandomizerFishingRule` を `generated/randomizer_area_rules.h` に吐き出す。
  - `schemaVersion` 未検証、カテゴリフィルタや allowEmpty の概念なし。
- ランタイム: `src/randomizer.c` + `include/randomizer.h` + `include/config/randomizer.h`  
  - `RandomizeWildEncounterBlocked` / `RandomizerIsEncounterBlocked` で areaRules を参照。  
  - areaMask 定義（LAND/WATER/FISH/ROCK/HIDDEN）あり。時間帯・釣りロッド・slotMode/rareSlots/rareRate は現行実装済み（v1.22時点）。
  - デバッグログは WARN で `RandomizerIsEncounterBlocked` などから出力。
- 定数類: `include/constants/flags.h`（デバッグフラグ追加の候補）、`include/config/randomizer.h`（ビルド時定数の候補）。`include/constants/randomizer.h` は未作成。

## 変更予定ファイルと役割（計画）
- `data/randomizer/area_rules.yml`  
  - v1.30スキーマへ更新（schemaVersion 追加、timeSlots/allowEmpty/specialOverrides/カテゴリ指定/rare & default ネスト対応）。
- `dev_scripts/build_randomizer_area_rules.py`  
  - スキーマ検証（schemaVersion==1.30、既知キー集合＋型一致）。  
  - allowEmpty 処理（null以外混入でエラー、rareSlots/rareRateもnull強制）。  
  - specialOverrides（legend/mythical/ub/paradox/subLegend）と specialOverridesMode（OR/ANDグローバル）を生成物に反映。  
  - 未定義エリアスキップ、例外マップ、バニラトグル、警告の全件出力＋サマリを実装。  
  - 生成物に新フィールド（timeSlotコード、specialOverrides等）を追加。  
- `generated/randomizer_area_rules.h`（スクリプト出力物）  
  - 新フィールド追加に合わせた出力フォーマットへ変更。
- `src/randomizer.c`  
  - 未定義エリアなら遭遇なしを返す処理（釣りはミニゲーム前に即中断）。  
  - specialOverrides フィルタ適用、OR/ANDモード対応。  
  - allowEmptyスキップ時の分岐とWARNログ（抑制キー: mapGroup/mapNum/areaMask/timeSlot/fishing）。  
  - バニラ遭遇デバッグトグル／例外マップ対応。  
  - 釣りメッセージ呼び出し（新規text ID）とフリーズしないフロー確認。  
- `include/randomizer.h`  
  - 新定数/enum/struct フィールドの宣言。specialOverridesMode などの型定義。  
- `include/constants/flags.h`  
  - デバッグ用バニラ遭遇許可フラグ、デバッグログフラグなどを追加（空きに割当）。  
- `include/config/randomizer.h`  
  - 本番OFFスイッチ（例外マップ無効化）、specialOverridesMode デフォルト（AND/OR）、WARNログ抑制の挙動などを定数化。  
- `strings.[ch]`  
  - 釣りスキップ専用メッセージ（英語のみ、例: `gText_NoFishingTargets`）。必要なら時間帯未定義スキップ用も共通文言で流用。  
- `data/randomizer/` 以下に例外マップリストがあるなら追加/分離（デバッグ限定）。  
- 新規: `include/constants/randomizer.h`（案）  
  - ランダマイザ専用のフラグ/定数/bitmask をまとめる場合に追加。

## 現状ソースの観察メモ（修正は未実施）
- スクリプト: `dev_scripts/build_randomizer_area_rules.py`  
  - v1.21仕様（timeSlots必須、allowEmptyあり、rare/uniform検証あり）。specialOverrides/schemaVersion検証/カテゴリ系は未実装。  
  - 釣りは rod 別に default/rare を持ち、allowEmpty=trueで空スキップ。  
- 生成物: `generated/randomizer_area_rules.h`（現行構造体）  
  - `RandomizerAreaRule` / `RandomizerFishingRule` に allowLegendOverride/maxRerolls/slotMode/rare/allowEmpty など。specialOverrides/timeSlotコード以外の新フィールドはなし。  
- ランタイム: `src/randomizer.c`  
  - `RandomizerIsEncounterBlocked`/`RandomizeWildEncounterBlocked` で areaRule/fishingRule を参照。未定義エリアはスキップせず Land フォールバックあり。allowEmpty が 0件ならブロックする設計。  
  - デバッグログは WARN で map/timeSlot/areaMask などを出力（抑制キーは未実装）。釣りはミニゲーム後に分岐。  
- 定数/設定:  
  - `include/randomizer.h`: slot/time enums、RandSeed/RandomizeWildEncounter などの宣言あり。  
  - `include/config/randomizer.h`: ビルド時定数の候補。  
  - `include/constants/flags.h`: デバッグフラグ等を追加する余地あり（空き要確認）。  
  - `include/constants/randomizer.h`: 未作成（専用定数を集約する案）。  

## 実装フロー案（ソース修正はまだNG）
1) スキーマとバリデーション更新  
   - `build_randomizer_area_rules.py` に v1.30 スキーマ定義、schemaVersionチェック、キー集合＋型チェック、allowEmpty/nullチェック、specialOverrides/specialOverridesMode対応を入れる設計。  
   - 未定義エリア/未分類カテゴリ/フォーム漏れの警告を全件出力＋サマリ。  
   - 例外マップリストの本番OFFスイッチを config 値から読むようにする設計。  
2) 生成物フィールド拡張  
   - `RandomizerAreaRule` / `RandomizerFishingRule` に timeSlotコード、specialOverrides、allowEmptyフラグ、バニラ例外フラグなど必要なフィールドを追加するフォーマット案を固める。  
3) ランタイムロジック設計  
   - `RandomizerIsEncounterBlocked` で未定義/allowEmpty/カテゴリ不許可を早期スキップ。釣りはミニゲーム前にメッセージを出して中断。  
   - specialOverridesMode（OR/AND）を config か flag で参照。  
   - バニラトグル/例外マップ時はフィルタを飛ばしてバニラ遭遇を許可（カテゴリフィルタは維持）。  
4) フラグ配置・定数整理  
   - `config/randomizer.h` にデフォルト設定（specialOverridesMode初期値AND/OR）、本番OFFスイッチ（例外マップ無効化）を定義。  
   - `include/constants/flags.h` にデバッグ用フラグ（バニラ遭遇許可、デバッグログON/OFFなど）を追加（空き確認）。  
   - 必要なら `include/constants/randomizer.h` でランダマイザ関連定数を集約。  
5) メッセージ追加とデバッグログ  
   - `strings.[ch]` に釣りスキップ専用文言を追加（英語のみ、例: `gText_NoFishingTargets`）。  
   - WARNログの抑制キーと出力内容を設計（mapGroup/mapNum/areaMask/timeSlot/fishingで1回）。  
6) テストデータと動作確認計画  
   - 簡易YAMLを複数用意（未定義スキップ、allowEmpty陸/釣り、カテゴリON/OFF、OR/AND切替、例外マップON/OFF、バニラトグルON/OFF、rare+allowEmptyの組み合わせ）。  
   - `python3 dev_scripts/build_randomizer_area_rules.py` → `make -j4` でビルド通ることを確認。デバッグビルドでWARNログと釣りメッセージを確認。  

## 影響範囲メモ
- YAMLスキーマ互換性: 1.21以前は全てエラー。既存データは書き換え必須。  
- 生成物構造体が変わるため、`src/randomizer.c` での参照箇所をすべて見直し。  
- デバッグ/本番切替の定数値を `config.h` で管理するため、ビルド設定への影響あり。  
- 釣りUIフローにメッセージ追加が入るため、GBAメッセージリソースが増える。  

## 参照ファイル
- 入力: `data/randomizer/area_rules.yml`  
- スクリプト: `dev_scripts/build_randomizer_area_rules.py`  
- 出力: `generated/randomizer_area_rules.h`  
- ランタイム: `src/randomizer.c`, `include/randomizer.h`, `include/config/randomizer.h`  
- 定数/フラグ: `include/constants/flags.h`, （新規案）`include/constants/randomizer.h`  
- 文言: `strings.c`, `strings.h`  
