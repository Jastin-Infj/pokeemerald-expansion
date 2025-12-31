# Randomizer v1.30 実装メモ

## スキーマ/生成
- `schemaVersion: 1.30` 固定。`allowLegendOverride` は廃止（YAMLに書くと生成エラー）。伝説可否は `specialOverrides.legend` で明示。
- `dev_scripts/build_randomizer_area_rules.py` を更新。specialOverrides のビットのみでカテゴリ判定。deprecated キー検出でエラー。
- 例外マップ（デバッグ専用）は `data/randomizer/exception_maps.yml` → `generated/randomizer_exceptions.h`。`RANDOMIZER_DISABLE_EXCEPTION_MAPS`=FALSE で有効。

## データ (`data/randomizer/area_rules.yml`)
- 全 `allowLegendOverride` を削除。必要箇所は `specialOverrides` で明示（例: `route_103_land` morning は legend=false で伝説禁止）。
- Route101/102/103/110/121/128 など定義を 1.30 仕様に整備。例外: Route101 Land は allowEmpty+例外マップでバニラ化を確認済み。

## ランタイム (`src/randomizer.c`, `src/fishing.c`)
- カテゴリフィルタは specialOverrides のみで判定（legend許可ビットの足し込みを廃止）。
- 未定義エリアはブロック時に WARN ログを出力（map/mask/time/rod を含む）。
- 釣りブロックメッセージのフリーズ/座標ズレを修正し、A/Bで閉じられるようにした。

## フラグ/定数
- `FLAG_RANDOMIZER_VANILLA_ENCOUNTER` にコメント追記: 全域バニラ化スイッチ（例外や areaMask に関係なくランダマイザー無効）。部分バニラを試すときは OFF のまま。
- 例外マップはビルド定数 `RANDOMIZER_DISABLE_EXCEPTION_MAPS` で有効/無効を切替（デバッグ用）。

## 種族フラグ整理
- サブ伝説 `.isSubLegendary` を多数追加（レジ系・三湖・カプ・UB・パラドックス等）。同時に `.isLegendary` を外して準伝扱いへ移行。
- Poipole/Meltan は一般枠に戻し（サブ伝説/幻なし）。メルメタルも幻なし（Frontier禁止は維持）。Calyrex 騎乗形態は伝説のまま。
- Cosmog/Cosmoem を一般枠に移行。

## デバッグ/ログ
- 未定義エリア・例外・ブロック時は WARN ログで確認可能（FLAG_RANDOMIZER_DEBUG_LOG ON, NDEBUG OFF）。
- 釣りブロック時にメッセージが閉じない/座標ずれの不具合を修正済み。

## 使い方メモ
1. `python3 dev_scripts/build_randomizer_area_rules.py`
2. `make -j4`
3. 部分バニラを試す: `RANDOMIZER_DISABLE_EXCEPTION_MAPS=FALSE` ビルド + `FLAG_RANDOMIZER_VANILLA_ENCOUNTER`=OFF。
4. 全域バニラを試す: 上記フラグを ON（例外設定は無視される）。

## 既知挙動
- 未定義エリアは遭遇ブロック＋WARNログ。
- 例外マップは areaMask/timeSlot/rodType が一致したときのみバニラ化。フラグ `FLAG_RANDOMIZER_VANILLA_ENCOUNTER` ON 時は全域バニラが優先される。

## 影響ファイル・フラグ・変数まとめ
- スクリプト/生成: `dev_scripts/build_randomizer_area_rules.py`（allowLegendOverride廃止、specialOverridesのみ）、`generated/randomizer_area_rules.h`（再生成）。
- データ: `data/randomizer/area_rules.yml`（allowLegendOverride削除、route_103_land morningでlegend=falseを明示）、`data/randomizer/exception_maps.yml`（デバッグ例外）。 
- ランタイム: `src/randomizer.c`（specialOverridesのみでカテゴリ判定、未定義エリアのWARNログ追加）、`src/fishing.c`（釣りブロック時のメッセージ固まり/座標ズレ修正）。
- フラグ: `include/constants/flags.h` の `FLAG_RANDOMIZER_VANILLA_ENCOUNTER` に全域バニラ化スイッチであることを追記。例外はビルド定数 `RANDOMIZER_DISABLE_EXCEPTION_MAPS` で有効/無効を切替。
- 種族定義: `src/data/pokemon/species_info/gen_*.h`（サブ伝説整理、Poipole/Meltan一般化、Calyrex騎乗は伝説維持、Cosmog/Cosmoem一般化など）。
