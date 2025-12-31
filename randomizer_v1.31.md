# randomizer_v1.31.md — timeSlotsマージ拡張まとめ

## 変更概要
- timeSlotsで `any` をベースにしつつ特定時間帯（morning/day/evening/night）を部分上書きできるようにスクリプトを拡張。
- allowEmptyを any→時間帯→rod の各階層で「データを書いたら未指定でも自動false」にする統一ルールを導入。
- バリデーションをマージ後の値で実施し、パターン違反やslotMode/rareRate/rareSlots不整合を明確なエラー文で弾く。
- テスト用に `route_104_land` を追加し、`any`で遭遇なし・`morning`のみWL許可の挙動を検証可能にした。
- schemaVersionは現行通り `1.30` のまま運用（必要なら後日スクリプトの `SCHEMA_VERSION_EXPECTED` とYAMLを1.31に揃える）。

## 影響ファイル
- `dev_scripts/build_randomizer_area_rules.py`
  - timeSlotsのany+部分指定を許容するマージ処理を追加。
  - allowEmptyの自動false落ち（データ記載時）をany/時間帯/rod共通で適用。
  - バリデーションをマージ後の値に変更し、エラーメッセージ粒度を整理。
  - 依然 `allowLegendOverride` 使用はエラー（specialOverridesのみ許容）。
- `data/randomizer/area_rules.yml`
  - テスト用 `route_104_land` 追加（any:遭遇なし、morningだけWL許可）。
  - 既存データはschemaVersion 1.30のまま。
- `generated/randomizer_area_rules.h`（自動生成物）

## 主な仕様ポイント
- timeSlots有効パターン:  
  1) anyのみ / 2) any+任意の時間帯 / 3) morning/day/evening/nightの4キー（anyなし）。それ以外はエラー。
- マージ順: any → 時間帯 → rod（fishing）。書いたフィールドだけ上書き、未記載は継承。
- allowEmpty: 未指定でもデータを書けば自動でfalseに落ちる。明示true＋非空WLはエラー。
- slotMode/rareRate/rareSlots/maxSpecies/rod欠け等はマージ後にバリデーション。

## 利用フラグ・変数
- 新規フラグ/VARの追加なし。既存のランダマイザー制御フラグ（例: FLAG_RANDOMIZER_AREA_WL など）は従来通り。

## ビルド・生成
- スクリプト実行: `python3 dev_scripts/build_randomizer_area_rules.py`
- ビルド確認: `make -j4` 成功（schemaVersion 1.30のまま）。

## 確認済みテスト例
- Route104 Land: day=遭遇スキップ（any: allowEmpty true）、morning=WL許可で遭遇あり。
- 既存の fishing/land/water ルールは従来通り生成成功。
