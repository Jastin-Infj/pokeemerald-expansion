# TODO-SCRIPT new ver1.50.md — DexNav整合＆スロット固定レイヤー 詳細設計

## ゴール
- ランダマイザーとDexNavの表示/遭遇を一致させるため、スロット固定マップ（末尾レア枠）を導入する。
- Register（Rボタン）で「選択スロットの種」を強制出現させる挙動をランダマイザーと整合させる。
- 速度違反（走行/自転車）のチェイン切断を無効化できるフラグを追加（デフォルト無効化ON、デバッグ連動）。
- schemaVersionを1.50に上げ、旧フォーマットはエラー扱い。

## 変更予定ファイル（調査済み）
- `dev_scripts/build_randomizer_area_rules.py`  
  - slot→species マップ出力（1本、末尾レア枠）。slotOffset/slotCount/rareStartIndex等のフィールド追加。  
  - 重み順で切り詰め、枠不足は空（×/???）。rareSlots>slotCount/rareWLはエラー。  
  - schemaVersion=1.50必須、旧版エラー。`--check/--report`の出力にslotマップ警告を追加。
- `data/randomizer/area_rules.yml`  
  - schemaVersionを1.50に。必要なら最小サンプルを追加（枠超過/枠不足/レア/釣り/BL禁止/空/速度違反無効/登録強制出現）。
- `generated/randomizer_area_rules.h`  
  - 新フィールド（スロットマップ＋オフセット/rare開始位置など）を含む生成物。
- `src/randomizer.c`  
  - ランタイム抽選をスロットマップ参照に切替（通常=先頭、レア=末尾）。maxRerolls/rareRate整合を維持。  
  - fallback時もスロットマップ内で再抽選。Register強制出現時のバイパス/固定モードを追加。
- `src/dexnav.c`  
  - UI: ×/？表示をスロットマップに基づき反映。×はスキップ＋決定時二重チェック。  
  - Register: 選択スロットの種を記憶し、遭遇時に固定出現。BL/カテゴリ禁止は×表示で選択不可。  
  - 速度違反無効化: Task_RevealHiddenMon 周辺の速度違反チェックをフラグで無効化できるように。
- `include/config/dexnav.h`  
  - `DEXNAV_IGNORE_SPEED_PENALTY`（仮名）を追加。デフォルトON。デバッグフラグとORで無効化。  
  - `DEXNAV_ENABLED` は運用時にTRUEにする前提、Flag/Varを非0に設定するメモも必要。
- `include/constants/randomizer_slots.h` など  
  - slot数や定数に変化が必要なら調整（基本は現行のLAND_MAX_SLOTS等を流用）。

## 仕様詳細（決定事項の反映）
- スロットマップ: 1本で末尾`rareSlots`ぶんがレア枠。通常は先頭、レアは末尾。  
- 切り詰めルール: 重み上位から配置（weights/rareWeightsがあればそれを使用、無ければ均等→WL順）。超過分は警告で落とす。  
- 枠不足: ???表示で空スロット（選択不可）。  
- rareSlots > slotCount/rareWL: エラー（rare未割当分は空だが、まずエラーで止める）。  
- Register: BL/カテゴリ禁止は事前フィルタで×表示・選択不可。RegisterしたスロットはrareRateに関係なく固定出現。  
- 速度違反: configフラグで完全無効化（デフォルトON、デバッグとOR）。  
- スキーマ1.50: 追加キーなし。旧フォーマットはエラー。
- 空スロット表示: 未遭遇=？（選択可）、割当なし/禁止=×（選択不可、カーソルスキップ）。

## 実装ステップ（推奨順）
1) スクリプト拡張  
   - schemaVersion=1.50必須。slotマップ生成（1本、末尾レア）。slotOffset/slotCount/rareStartIndex等をルールに付与。  
   - 重み順切り詰め、rareSlots>slotCount/rareWLはエラー。枠不足は空スロットを出力。  
   - `--check/--report` にslotマップ警告（超過/不足/rare矛盾）を追加。
2) データ更新  
   - area_rules.yml を1.50に。サンプル（枠超過/不足/レア/釣り/BL禁止/空/速度違反無効/登録強制出現）を用意。  
3) 生成物更新  
   - `generated/randomizer_area_rules.h` に新フィールドを出力。weights/levelBandsも並び替え後に固定。  
4) ランタイム（randomizer.c）  
   - 抽選をスロットマップ参照に切替。レア時は末尾範囲、通常は先頭。fallbackもマップ内再抽選。  
   - Register強制出現の経路を追加（スロット種を固定出現、禁止なら×）。  
5) DexNav（dexnav.c）  
   - スロット表示をマップに合わせ、×/？を出し分け。×はカーソルスキップ＋決定時も弾く。  
   - Registerで選択スロットを記憶し、揺れ→接触で固定出現（ランダマイザーと同マップ参照）。  
   - 速度違反無効化フラグを適用（デフォルトON、デバッグOR）。  
6) config  
   - `include/config/dexnav.h` にフラグ追加。DexNav有効時のFlag/Var割当メモ。  
7) テスト  
   - 最小サンプルで枠超過/枠不足/レア/釣り/BL禁止/空/速度違反無効/登録強制出現を確認。  
   - デバッグログ（必要ならWARN出力）でスロットマップとRegister動作を確認。

## 事前チェックしておくと良いファイル
- `src/dexnav.c`：カーソル処理、アイコン描画（DrawSpeciesIcons周辺）、Task_RevealHiddenMonの速度違反チェック、Register関連。  
- `src/randomizer.c`：Rare/normal抽選分岐、maxRerolls/rareRate周り、Register経路をどう差し込むか。  
- `include/config/dexnav.h`：フラグ名追加位置。  
- `include/constants/randomizer_slots.h`：slot数定数（必要なら拡張）。  
- `data/randomizer/area_rules.yml`：schemaVersionとサンプル配置。  
- `dev_scripts/build_randomizer_area_rules.py`：スキーマバリデーションと新フィールド出力の導入箇所。

## 留意点
- DexNavのRegisterは元々「その種を固定出現させる」仕様。ランダマイザーでもスロットマップを介して同じ種を出す。BL/カテゴリ禁止に当たる場合は×で選択不可・決定時も弾く。  
- UIページングは後回し。切り詰め＋空スロット明示でまず崩れない形を作る。  
- schemaVersionを1.50に上げた後、旧フォーマットはエラーで止めるため、マイグレーション手順をrandomizer_v1.50.md（後日）に記載すること。
