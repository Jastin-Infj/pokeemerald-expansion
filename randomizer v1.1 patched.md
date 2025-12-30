ランダマイザー（釣り周り）v1.0 変更メモ
===========================================

主な実装
--------
- デバッグフラグ追加（`include/constants/flags.h`）  
  - `FLAG_RANDOMIZER_FISHING_AUTO_HOOK`: バイト+ミニゲームをスキップして必ず遭遇。  
  - `FLAG_RANDOMIZER_FISHING_AUTO_BITE`: バイトのみ100%にし、ミニゲームは原作通り。  
- 釣りルールのロッド別上限を適用（`src/randomizer.c`）  
  - `fishingRule->maxSpecies` を優先して WL の有効件数をクランプ。0ならエリア上限→件数。  
  - `rareRate` は「先頭 `rareSlots` 件を引く確率」。レア/一般のバイト率とは無関係。  
- エリアルール検索のフォールバック修正（`src/randomizer.c`）  
  - 同一 `areaMask` を最優先し、無い場合のみ Land にフォールバック。釣りで Land ルールを誤参照しないように修正。  
- ログの調整（`src/randomizer.c`）  
  - デバッグログを WARN 出力に統一し、メッセージ先頭に `[INFO]` を付与。`FLAG_RANDOMIZER_DEBUG_LOG` で有効化。  
- デバッグスクリプト更新（`data/scripts/debug.inc`）  
  - Script 1 で `FLAG_RANDOMIZER_AREA_WL` / `FLAG_RANDOMIZER_DEBUG_LOG` に加え `FLAG_RANDOMIZER_FISHING_AUTO_HOOK` もセット。  
- 釣りミニゲームロジックは原作のまま（`src/fishing.c`）。追加フラグで挙動を切り替えるだけ。

挙動・仕様ポイント
------------------
- 釣り遭遇の流れ: バイト判定 → 「！」後ミニゲーム成功 → RandR で種選定。Rare/slotMode は選定のみで、バイト率やミニゲーム成功率には影響しない。  
- 釣りルールの適用: `areaMask: Fish` を持つエリアルールがある場合のみ釣り専用ルールが使われる。無いエリアは通常ランダマイズ（ログも出ない）。  
- ロッド別設定: Old/Good/Super は `sRandomizerFishingRules` から `slotMode/rareSlots/rareRate/maxSpecies` を取得。`maxSpecies` は有効候補数の上限として適用。  
- フォールバック: `maxRerolls` を超えると WL から直接選択。ログでは `(fallback)` と表示。  
- レア抽選: `rareSlots` が `wlLimit` より大きい場合は `wlLimit` に丸め、`rareRate`% で先頭 `rareSlots` 件を選ぶ。外れは残りから選ぶ。  
- バイト率設定: 現在 `I_FISHING_BITE_ODDS = GEN_LATEST`（Old 25% / Good 50% / Super 75%）。バニラ相当に戻す場合は `include/config/fishing.h` で `GEN_3` に変更。  
- デバッグ用フラグで遭遇を強制  
  - バイトだけ100%: `FLAG_RANDOMIZER_FISHING_AUTO_BITE`  
  - バイト＋ミニゲームスキップ: `FLAG_RANDOMIZER_FISHING_AUTO_HOOK`  

ビルド・確認手順
----------------
1) スクリプト再生成が必要な場合: `python3 dev_scripts/build_randomizer_area_rules.py`  
2) 通常ビルド: `make -j4`  
   - デバッグログを見たい場合: `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4`  
3) エミュ上で Debug Script 1 を実行し、`FLAG_RANDOMIZER_AREA_WL` / `FLAG_RANDOMIZER_DEBUG_LOG` / `FLAG_RANDOMIZER_FISHING_AUTO_HOOK` を一括セットして釣りテスト。  
4) ログ確認: `[INFO] RandR wild ... fishing=1` が出れば釣りルール適用中。`mask=4` が Fish。

影響ファイル
-------------
- include/constants/flags.h  
- include/config/fishing.h  
- src/randomizer.c  
- src/fishing.c  
- data/scripts/debug.inc  
- generated/randomizer_area_rules.h（スクリプト生成物）

留意点
------
- 他エリアで釣りのランダマイザを効かせたい場合は `areaMask: Fish` のエントリを追加し、スクリプト再生成→ビルドが必要。  
- バイトが厳しいと感じる場合は `I_FISHING_BITE_ODDS` を `GEN_3` に変更するか、デバッグフラグで切り分けて確認する。  
- Rare指定は「先頭 `rareSlots` 件をレア枠」とする仕様。WLの並び順に注意。


ファイル一覧と役割
------------------
- data/randomizer/area_rules.yml  
  - エリア別WL/BL定義と釣りルール（ロッド別slotMode/rare/maxSpeciesなど）を記述する入力YAML。kitsもここで定義。  
- dev_scripts/build_randomizer_area_rules.py  
  - 上記YAMLを読み、重複除去・ソート・空WL検出を行い `generated/randomizer_area_rules.h` を生成するスクリプト。  
- generated/randomizer_area_rules.h  
  - ビルド生成物。エリアルール・釣りルール・プール配列を定義し、ランタイムで参照。  
- data/randomizer/trainer_dup_rules.h  
  - トレーナーごとの重複制御ルール（maxSame/minDistinct）を列挙するヘッダ。未記載はデフォルト（制限なし）。  
- src/randomizer.c  
  - ランダマイザ本体。WL/BLフィルタ、トレーナー重複制御、釣りルール適用、デバッグログなどを実装。  
- src/fishing.c  
  - 釣りミニゲーム・バイト判定の本体。デバッグフラグ（AUTO_BITE/AUTO_HOOK）を見て挙動を切替。  
- include/config/fishing.h  
  - 釣りのバイト率・ミニゲーム世代などの設定（`I_FISHING_BITE_ODDS` など）。  
- include/config/randomizer.h  
  - ランダマイザー全体のON/OFF（`RANDOMIZER_AVAILABLE`）、マスター用フラグ/Varの割り当て（wild/items/trainers/fixed/starters）、シード方式などを定義。マスターをOFFにすればランダム化を無効化できる。  
- マスター無効化（フラグ/Var）
  - `include/config/randomizer.h` で `RANDOMIZER_AVAILABLE` を FALSE にすると全ランダマイザーをビルド時に無効化。  
  - 各マスターON/OFFフラグは同ファイルの `RANDOMIZER_FLAG_WILD_MON` などで未使用Flagに割り当て済み（flags.hの0x020〜0x024）。フラグを下ろせばランダマイザーを個別に無効化できる。  
- include/randomizer.h  
  - ランダマイザの公開API・enum（理由/オプション）・SFC32ユーティリティ。  
- include/constants/vars.h  
  - `VAR_RANDOMIZER_AREA_MODE` などランダマイザ関連のVar定義。  
- include/constants/flags.h  
  - ランダマイザ関連フラグの定義（WL適用、デバッグログ、釣りオート系など）。  
- data/scripts/debug.inc  
  - デバッグメニューのスクリプト。Script 1 で WL＋ログ＋釣りオートフックを一括ONにする。  
- randomizer_v1.0.md / randomizer_fishing_v1.0.md / todo.md / todo-script*.md  
  - 仕様メモ・進行メモ。v1.0のまとめ、本タスク用のTODO、釣り専用メモなど。