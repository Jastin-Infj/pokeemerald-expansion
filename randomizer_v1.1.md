ランダマイザー v1.1（patched）概要
==================================

実装状態（v1.0ベース＋パッチ）
- エリアWL/BL適用: `FLAG_RANDOMIZER_AREA_WL` ONでエリア別WL/BLを野生・トレーナーに適用。Fish/Water/Rockは同一mask優先、無いときだけLandにフォールバック。
- トレーナー重複制御: `data/randomizer/trainer_dup_rules.h` で `maxSame/minDistinct` を設定（未記載は255/0）。WL不足時は「できる範囲でユニーク→残り重複許容」。
- トレーナースキップ: `data/randomizer/trainer_skip_list.h` に列挙したIDは、ランダマイザーONでも非ランダム化（`ShouldRandomizeTrainer`）。
- 釣り: ロッド別 `maxSpecies` でWL件数をクランプ。`rareRate` は先頭 `rareSlots` の当たり確率。バイト率は `I_FISHING_BITE_ODDS` 依存（現状 GEN_LATEST: 25/50/75）。
- 釣りデバッグ: `FLAG_RANDOMIZER_FISHING_AUTO_HOOK`（バイト+ミニゲーム省略）／`FLAG_RANDOMIZER_FISHING_AUTO_BITE`（バイトのみ100%）。`debug.inc` Script 1 でWL＋ログ＋AUTO_HOOKを一括ON。
- ログ: WARNで `[INFO] RandR ...` 出力（`FLAG_RANDOMIZER_DEBUG_LOG`、DEBUGビルド時）。
- シード安定: 現行のシード計算を維持（野生: mapGroup/mapNum/area/slot、トレーナー: trainerId/partySize/slot、ベースシードはトレーナーID＋秘密ID）。

関連ファイルと役割
- data/randomizer/area_rules.yml … WL/BL定義（kits＋areas/gifts）。釣りロッド別設定もここ。将来の時間帯拡張もここを拡張予定。
- dev_scripts/build_randomizer_area_rules.py … 上記YAMLをマージ/検証し `generated/randomizer_area_rules.h` を生成。
- generated/randomizer_area_rules.h … エリアルール・釣りルール・プール配列の生成物。
- data/randomizer/trainer_dup_rules.h … トレーナーごとの重複制御テーブル。
- data/randomizer/trainer_skip_list.h … ランダマイズ除外トレーナーIDリスト。
- include/config/randomizer.h … マスターON/OFF、シード方式、各フラグ/Var割り当て。
- include/constants/flags.h … ランダマイザー用フラグ（WL適用、ログ、釣りオート系など）。
- include/constants/vars.h … `VAR_RANDOMIZER_AREA_MODE` などVar定義。
- src/randomizer.c … WL/BLフィルタ、トレーナー重複制御、釣りルール適用、デバッグログなど本体。
- src/fishing.c … 釣りミニゲーム・バイト判定。AUTO_BITE/AUTO_HOOKフラグを参照。

時間帯対応（検討中/設計メモ）
- 時間帯ON時に時間帯別WLを使えるようYAML拡張（`timeSlots.morning/evening/night` など）を検討。OFF時は共通WLのみ。空WLは時間帯ONなら出現なしを許容、OFFで共通WL空は警告止まり。

ビルド/デバッグ手順
- 通常: `make -j4`。デバッグログを見る場合は `CPPFLAGS_EXTRA="-UNDEBUG" CFLAGS_EXTRA="-UNDEBUG" make -j4`。
- エミュでScript 1実行→ `FLAG_RANDOMIZER_AREA_WL`/`FLAG_RANDOMIZER_DEBUG_LOG`/`FLAG_RANDOMIZER_FISHING_AUTO_HOOK` をセット。AUTO_BITEは別途setflag。
