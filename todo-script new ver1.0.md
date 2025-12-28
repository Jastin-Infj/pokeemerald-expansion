場所別ランダマイザー 修正タスクメモ（ver1.0）

- ゴール: Route102（Land）のWL内容を再調整する方針を整理する。
- 現状参照: data/randomizer/area_rules.yml（route_102_landエントリ）、src/randomizer.c（WLフィルタ/リロール実装）、generated/randomizer_area_rules.h（ビルド生成物）。

やりたいこと（案）
- Route102のWLを再編する（例: 序盤キットの一部削除、進化段階を揃えるなど）。
- maxRerollsやlegend解禁有無は現状維持のまま、WL中身のみ調整を検討。
- kits側（starter_land/mid_land/starter_trim 等）をいじるか、route_102_land専用のapply/removeを増やすかを決める。
- デバッグ確認手順を明記（Script 1でFLAG_RANDOMIZER_AREA_WL＋DEBUG_LOGをON、デバッグログで候補を確認）。

作業ステップ（ドラフト）
1) route_102_land用のWL方針を決める（採用したい種と除外種をリストアップ）。
2) kitsを更新するか、route_102_landのapply/removeを個別に設定するか決定。
3) data/randomizer/area_rules.yml を更新 → python3 dev_scripts/build_randomizer_area_rules.py → make。
4) デバッグメニュー Script 1 でWL＋ログON、実機/エミュでRoute102を歩き、期待する種だけが出るか確認。ログにfallbackが出ないかもチェック。

メモ
- 空WLは禁止（スクリプトでエラー）。
- allowLegendOverrideは現状falseのまま（Route102では伝説解禁しない）。

トレーナー重複NGオプション（今後の実装アイデア）
- ジム/四天王など特定のトレーナーだけ「重複禁止」モードを使えるようにする。通常トレーナーは現状どおり重複可。
- 手順案:
  - 重複禁止トレーナーIDのリストを用意（例: gRandomizerTrainerNoDupList[] + END）。
  - RandomizeTrainerMon内で判定し、重複禁止ならユニークリストを生成してスロットに割り当て（`GetUniqueMonList` 相当）。seedは trainerId/totalMons などで再現性を確保。
  - WLがパーティ数より少ない場合は「WL全種を一巡させて残りは再度抽選（重複許容）」などフォールバックを明示。
  - 重複可モードは現行のリロール/フォールバックを維持。

トレーナー重複制御（maxSame/minDistinct＋デフォルト）案
- デフォルト: `defaultMaxSame = 255`（制限なし）、`defaultMinDistinct = 0`（要求なし）を静的定数にする。
- 個別ルール: `struct { u16 trainerId; u8 maxSame; u8 minDistinct; } gRandomizerTrainerDupRules[]` を `RANDOMIZER_TRAINER_ID_END` 終端で持ち、ジム/四天王/ボスのみ設定。未登録はデフォルトを使う。
- 判定: `GetTrainerDupRule(trainerId)` でルール取得。`minDistinct > 0` または `maxSame < 255` なら重複抑制モード。
- 振る舞い（実装時の目安）:
  - `minDistinct <= wlCount` の場合: ユニークリストを優先して埋める。残りスロットは `maxSame` を守りつつ抽選/リロール。
  - `minDistinct > wlCount` の場合: 「WL全種を一巡→残りは重複許容」といったフォールバックを明示。
  - 重複抑制を使わない通常トレーナーは現行のリロール/フォールバックをそのまま。
