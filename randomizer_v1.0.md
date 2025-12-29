ランダマイザー v1.0 実装メモ
==============================

概要
----
- エリア別WL/BLで野生・トレーナー双方をフィルタする仕組みを導入。  
- 釣りのロッド別ルール対応、デバッグ用フラグ追加。  
- シード計算は現行仕様（トレーナーID＋秘密ID、野生は mapGroup/mapNum/area/slot、トレーナーは trainerId/partySize/slot）を維持。

データ生成
----------
- 入力: `data/randomizer/area_rules.yml`（kits＋areas/gifts）  
- 生成: `python3 dev_scripts/build_randomizer_area_rules.py` → `generated/randomizer_area_rules.h`  
- ルール検索: `mapGroup/mapNum/areaMask`。Fish/Water/Rockは同一mask優先、無ければLandにフォールバック。

WL/BL適用（野生/トレーナー共通）
--------------------------------
- フィルタ順: `MON_RANDOMIZER_INVALID` → グローバルBL → エリアWL/BL → フォーム処理。  
- `maxRerolls` までリロール、超過時はWLから直接ピック（フォールバック）。  
- 伝説解禁はWLに載せ、かつ `allowLegendOverride` がtrueのエリアのみ。  
- 空WLはスクリプトでエラー（設計ミス扱い）。

トレーナー重複制御
------------------
- ルール表: `data/randomizer/trainer_dup_rules.h` に `{ trainerId, maxSame, minDistinct }` を列挙（終端ID付き）。未指定トレーナーはデフォルト `maxSame=255, minDistinct=0`。  
- WL不足時は「できる範囲でユニーク→残り重複許容」。`maxSame=0` は避ける。  
- 将来的に `minDistinct > wlCount` は生成スクリプトでビルドエラーにする方針。

釣り（v1.0）
------------
- ロッド別ルール: `fishingRule->maxSpecies` を優先してWL有効件数をクランプ（0ならエリア上限→件数）。  
- レア抽選: `rareRate` は先頭 `rareSlots` 件を引く確率。`rareSlots > wlLimit` は丸め。バイト率とは無関係。  
- デバッグフラグ:  
  - `FLAG_RANDOMIZER_FISHING_AUTO_HOOK` … バイト＋ミニゲーム省略で必ず遭遇。  
  - `FLAG_RANDOMIZER_FISHING_AUTO_BITE` … バイトのみ100%、ミニゲームは原作通り。  
- デバッグスクリプト: `data/scripts/debug.inc` Script 1 で WL＋ログ＋AUTO_HOOK を一括ON。  
- ログ: WARN出力で `[INFO] RandR ... fishing=1` が出る（`FLAG_RANDOMIZER_DEBUG_LOG` をON、DEBUGビルドで有効）。

フラグ/ビルド
-------------
- WL適用: `FLAG_RANDOMIZER_AREA_WL`  
- デバッグログ: `FLAG_RANDOMIZER_DEBUG_LOG`  
- 釣りデバッグ: 上記 AUTO_HOOK / AUTO_BITE  
- ビルド: `make -j4`。デバッグログを見る場合は `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4`。

既知の仕様/注意
---------------
- ランタイムのモード併用: 既存ランダマイズモードで候補抽出 → WL/BLで最終フィルタ。WL適用フラグOFFなら従来の「何でもあり」ランダム。  
- フォールバック時のログに表示される `wl` は元のWL件数で、ロッド上限によるクランプ後とは異なる（挙動には影響なし）。  
- 釣りバイト率は `include/config/fishing.h` の `I_FISHING_BITE_ODDS` に依存（現在 GEN_LATEST: Old25/Good50/Super75）。バニラ寄りにするなら `GEN_3` に変更。

参考ファイル
-------------
- include/constants/flags.h  
- include/config/fishing.h  
- data/randomizer/area_rules.yml → generated/randomizer_area_rules.h  
- data/randomizer/trainer_dup_rules.h  
- data/scripts/debug.inc  
- src/randomizer.c, src/fishing.c
