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

クランプ方針メモ（現状コードには未反映）
- `minDistinct` がパーティ数やWL件数を上回ってもエラーにはせず「できる範囲でユニーク→残りは重複許容」のままにする（WL優先）。例: WLが2種・パーティ5匹・minDistinct=5でも、2種を一巡させ残りは重複を許容。
- もし自動クランプを入れるなら `effectiveMinDistinct = min(minDistinct, totalMons, wlCount)` で上限を丸めるが、WLを優先するならクランプ無しのままにする。今回の運用方針は「WL優先・クランプ無し」で進める。
- ビルド時チェック方針: `minDistinct > wlCount` は達成不能なのでビルドエラーにする方向で進める（まだ実装なし）。実装する際は生成スクリプトに検証を追加する。

maxSame / minDistinct の動作例（イメージ）
- 例1: WL=3種、パーティ3匹、`maxSame=1`、`minDistinct=3` → 3枠ともユニークになる（重複禁止でちょうど3種）。
- 例2: WL=3種、パーティ5匹、`maxSame=2`、`minDistinct=3` → まず3種をユニークで埋め、残り2枠は最大2回まで同種OK。結果は3〜5種のいずれか（WL優先・重複許容）。
- 例3: WL=2種、パーティ5匹、`maxSame=1`、`minDistinct=5` → ユニーク要求は達成不能。2種を一巡させた後、残りは重複を許容（重複禁止は守れず、WL優先で妥協）。

Route102（Land）の課題と方針
- 現状: 野生はエリアWLで想定どおり抽選されているが、トレーナー手持ちが重複しがち。Route102用WLを再調整しつつ、重複制御（maxSame/minDistinct）の導入で改善する方針。
- 対応案:
  - Route102のWLを見直す（starter_landなどキット調整 or area専用apply/remove）。
  - トレーナー重複制御を実装し、特定トレーナーにだけ `maxSame=1` などを適用して手持ち重複を抑制。
  - `minDistinct` とWL件数が矛盾する場合はビルドエラーで検出する（未実装。スクリプト修正で対応予定）。
- 備考: Route102は例示に過ぎず、同様の仕様（WL適用＋重複制御の課題/解決策）は他の指定エリアでも同様に適用される想定。

デバッグログ（FLAG_RANDOMIZER_DEBUG_LOG）留意点
- NDEBUGビルドでは`DebugPrintf`系が無効になるため、ログを見たい場合はDEBUGビルド（NDEBUG無効）＋`LOG_HANDLER_MGBA_PRINT`（等のログハンドラ設定）でビルドすること。フラグを立ててもNDEBUGだと出力されない。

詳細設計メモ（実装前に固めるポイント）
- 重複制御（maxSame/minDistinct）
  - デフォルト: maxSame=255, minDistinct=0（制限なし）。重複NGにしたいトレーナーだけテーブルに記載。
  - ルール適用対象: ジム/四天王などの特定トレーナー。未記載トレーナーは制限なし。
  - 矛盾チェック: `minDistinct > wlCount` は達成不能 → 生成スクリプトでビルドエラーにする方針（未実装）。
  - フォールバック: WL優先。ユニーク確保が足りなくても「できる範囲でユニーク→残りは重複許容」。
  - 実装イメージ（詳細）:
    - ルール: `gRandomizerTrainerDupRules[]` を終端ID付きで用意（例: `{ TRAINER_ROXANNE, 1, 3 }, { END, 0, 0 }`）。未登録トレーナーはデフォルト値で制限なし。
    - ランタイム分岐: `RandomizeTrainerMon` でルール取得 → デフォルトなら既存処理。制限ありなら、
      - パーティごとにキャッシュをリセット（trainerId/partySizeが変わるとリセット）。slot順に処理。
      - 抽選: 既存のRandomizeWithAreaRule/RandomizeMonで候補生成 → `maxSame` 超過ならリロール、`minDistinct`を満たすために必要なら新規種を優先（残りスロットとの兼ね合いで必須判定）。
      - リロール上限超過時: WL未使用種を線形サーチ→なければ最後の候補で妥協。WLが少ないと重複が出るのは仕様。
      - キャッシュ: 決定した種をキャッシュに追加し、次スロットで重複判定に使う。
    - シード: 現行のtrainerId/totalMons/slotを維持し、再現性を壊さない。
- 実装ステップ案
  1) ルールテーブル追加（重複制御したいトレーナーだけ `trainerId, maxSame, minDistinct` を記載、終端ID付き）。
  2) ランタイム分岐: `RandomizeTrainerMon` でルールを取得し、制限なしなら従来処理、制限ありなら重複判定を挟む。
  3) ビルド時チェック: 生成スクリプトに `minDistinct > wlCount` エラーを追加。
  4) デバッグ確認: FLAG_RANDOMIZER_AREA_WL/DEBUG_LOG をONにしてログ出力を確認（DEBUGビルド＋mgba等）。
- WLデータ調整
  - Route102は例示。各エリアで「採用したい種・除外したい種」を決め、kits調整 or 個別apply/removeで反映。
  - 空WLは禁止（スクリプトエラー）。伝説解禁は `allowLegendOverride` true ＋ WLに明示。
- テスト方針
  - 野生: 指定エリアでWL外の種が出ないことを確認（ログでフォールバックが出ていないかも見る）。
  - トレーナー: 重複NG設定を入れたトレーナーが期待どおりバラけるか、`minDistinct` 矛盾でビルドが止まるかを確認。
  - デバッグメニュー: Script 1 でFLAG_RANDOMIZER_AREA_WL/DEBUG_LOGをONにする手順を周知。

ビルド時チェック案（まだ未実装）
- WL設計ミスを早期に検出したい場合、生成スクリプト側で `minDistinct > wlCount` を検出し、ビルドエラーにする運用を検討する。ただし、現時点のコードには入れていないため、導入する際は別途スクリプトを修正する。
