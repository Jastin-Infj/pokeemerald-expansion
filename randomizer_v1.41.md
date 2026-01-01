# randomizer_v1.41 変更内容まとめ（詳細版）

## 目的・背景
- maxRerolls の `"auto"` をランタイムで「実効候補数（normal/rare）」から決めるようにし、WLが少ないエリアでの過度なフォールバックを抑えたい。
- スクリプトに `--check` / `--report` を追加し、生成前にバリデーションと簡易レポートを確認できるようにする。
- auto を実際に試せる検証用データを用意する（route_102_land, fishing_mixed_sample）。

## スクリプト側の変更 (`dev_scripts/build_randomizer_area_rules.py`)
- 新規オプション
  - `--check`: 生成なしでバリデーションのみ実行。
  - `--report`: 簡易レポートを標準出力（ルール数/allowEmpty数/auto件数/WL/BL/weightsプール数）。
- `maxRerolls: "auto"` の扱いを「セントネル0xFFを書き出す」に統一。静的計算（WL-BL件数からmin件数）は廃止し、ランタイム計算に一本化。
- レポート例（現状データ）: `rules=22 allowEmpty=6 autoMaxRerolls=2 fishingAreas=2`
- バリデーションは既存を踏襲（encounterRate必須・rare設定の整合など）。後続バージョンでさらに厳格化予定。

## ランタイム側の変更 (`src/randomizer.c`)
- maxRerolls=auto (0xFF) をランタイムで決定。
  - normal/rare の有効候補数を別々にカウントし、`clamp(0..8)` で上限を決定。
  - rareHit 時は rare 用の上限、通常時は normal 用の上限を使用。
  - allowEmpty で0件なら0、その他は最低1〜最大8に抑制。
- デバッグログ
  - FLAG_RANDOMIZER_DEBUG_LOG ON 時、マップ/時間/ロッド単位で WARN を1回だけ出力。
  - 例: `[INFO] RandR auto maxRerolls map=0/17 area=0 time=3 rod=255 normal=10 rare=2 -> n=8 r=2`

## データ側の変更 (`data/randomizer/area_rules.yml`)
- auto の検証用に以下を変更:
  - `slotSets.fishing_mixed_sample`: `maxRerolls: auto`
  - `route_102_land`: `maxRerolls: auto`（slotMode=rare, rareSlots=2, rareRate=20）
- 生成物 (`generated/randomizer_area_rules.h`) を再生成（maxRerolls=0xFF が含まれる）。

## ビルド/生成結果
- `python3 dev_scripts/build_randomizer_area_rules.py --check --report` 成功。
- `python3 dev_scripts/build_randomizer_area_rules.py` 成功。
- `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4` 成功。

## 動作確認メモ（Route 102 Land Night）
- autoログ: `[INFO] RandR auto maxRerolls map=0/17 area=0 time=3 rod=255 normal=10 rare=2 -> n=8 r=2`
- Fallbackが多いのはWL(10)−BL(4)=6種と少ないため。maxRerollsは8/2に動的上書きされており、仕様通り。
- ばらつきを増やすには WLを増やす/BLを減らす、rareRateやrareSlotsを調整、もしくは maxRerolls上限をさらに緩めるなどの運用が必要。

## 既知の懸念・今後
- DexNav (DevNav) はスロット固定前提のUIで、現行の「WLプール抽選」方式とは整合しない。`todo ver1.50.md` に課題/方針案を記載（スロット固定レイヤーの導入等）。
- スキーマ・レポートのさらなる厳格化（カテゴリ矛盾の詳細化、CIでの `--check`/`--report` ゲート化など）は今後のバージョンで検討。 
