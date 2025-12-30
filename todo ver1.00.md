場所別ランダムマイザー設計メモ（コード未変更）

- 目的: 序盤は弱め/後半は強めにするBSTスケール型ランダム化。ImperiumのScaled Randomizationを主に参考にし、既存のシード挙動を維持。
- エリアキー: `mapGroup/mapNum`＋`areaMask(Land/Water/Fish/Rock/Hidden)`で特定。エリア→Tier表は`mapGroup/mapNum`でソートし、将来的に`mapGroup`索引＋二分探索でサーチ負荷を抑える。
- Tier定義: `targetBST`/`window`/`allowLegendary`/`blacklist`/`whitelist`/`bstOffsetPerLevel`/`maxRerolls`。基底テーブル＋`TierPatch`(BST・ウィンドウの加減算、BL追加・削除)で配列加算/減算運用。
- フロー（野生例）: 1) 既存`RandomizerRandSeed`を使い`data1`に`mapGroup/mapNum/area/slot`＋解決済み`tierId`を混ぜてシード安定。2) 既存モードで候補抽出（`MON_RANDOM`/`MON_RANDOM_BST`/`MON_RANDOM_LEGEND_AWARE`/`MON_EVOLUTION`）。3) フィルタ順: `MON_RANDOMIZER_INVALID`除外 → グローバルBL → エリアTier（BST帯・BL/WL・伝説可否）。4) リロールは同一`Sfc32State`を進め`maxRerolls`上限、超過時は帯を内側に縮め最寄りBSTへクランプ。5) 種族決定後にフォーム処理（既存Random/Special）。
- BSTスケール: `targetBST = base + slope*level + bstOffsetPerLevel*level`を基本に、`window`は±10%程度を目安。レアスロットのみBST小幅上乗せも可。
- トレーナー/固定: シード`data1`に`trainerId/localId`＋`tierId`を含め、配置マップからTier取得。
- メモリ/性能: 既存動的テーブル(~6B/種)を流用。ユニーク保証が必要な場面だけビットセット確保。エリア表が増えたら`mapGroup`索引でログ時間化。
- モード併用: デフォルトは「モードで候補 → Tierでフィルタ」。`MON_RANDOM_BST`はTierの`targetBST/window`で帯決定。`MON_RANDOM_LEGEND_AWARE`はレジェンドを別グループ化し、エリア`allowLegendary`で最終判断。
- ループ仕様明記: リロール上限超過時は帯縮小→最寄りBST返却を仕様としてコメント/設定に残す。

参照ファイル: include/config/randomizer.h, include/randomizer.h, include/pokemon.h, src/randomizer.c, src/wild_encounter.c

Imperium版実装の観察ポイント（参考用）
- コアのランダマイザー構造はほぼ同じで、BST帯は±約10.24%（`GetGroupRange`の`group * 100 / 1024`）で計算。エリア別スケールは未実装なので、本設計で新規追加が必要。
- シード源は同様にトレーナーID+秘密ID（`RANDOMIZER_SEED_IS_TRAINER_ID`）。種族テーブルは動的生成（EWRAM）で、`GetSpeciesTable`のキャッシュを利用。
- 追加機能: スターター/ギフト（72種, `gStarterAndGiftMonTable`）とタマゴ（56種, `gEggMonTable`）をユニーク抽選する枠があり、リストのdjb2ハッシュをシードに混ぜて再現性を確保。能力ランダム化はホワイトリスト方式で禁則（Wonder Guard/Ability Noneなど）を避ける (`src/randomizer.c`, `src/data/randomizer/ability_whitelist.h`)。
- 特殊フォームのレア抽選や除外フォームは個別テーブルで処理 (`src/data/randomizer/special_form_tables.h`)。本設計のエリアフィルタ導入時もフォーム処理は最終段に据える。
- `RandomizeWildEncounter`のシードは`mapGroup/mapNum/area/slot`で一致しており、今回のTierID混在案と互換性がある。テーブル更新タイミングを固定すれば、トレーナーID起点のシード再現性を保ちやすい。

詳細設計（固定シード運用を前提）
- テーブル生成タイミング: ビルド時にBase＋Patchをマージして静的配列を生成し、バイナリに埋め込む。起動時/Preloadでの再構築はしない（種族テーブルのキャッシュのみ利用）。
- データ構造案:
  - `RandomizerAreaKey { u8 mapGroup; u8 mapNum; u8 areaMask; }` areaMaskはLand/Water/Fish/Rock/Hiddenのビット。
  - `RandomizerTier { u16 targetBST; u16 window; s8 bstOffsetPerLevel; u8 allowLegendary; u8 maxRerolls; const u16 *blacklist; const u16 *whitelist; s8 slotRareBoost; }`
  - `RandomizerAreaRule { RandomizerAreaKey key; u16 tierId; u8 slotOffsetTable[NUM_SLOTS?]; }` slotごとに微調整したい場合は`s8 slotBSTOffset[slotCount]`でも可。未使用なら0固定。
  - TierPatch: `{ u16 targetTierId; s16 bstOffset; s16 windowOffset; const u16 *addBlacklist; const u16 *removeBlacklist; }`などをビルドスクリプトで適用。
  - 生成物: `generated/randomizer_area_rules.h` に `const struct RandomizerAreaRule gRandomizerAreaRules[]; const struct RandomizerTier gRandomizerTiers[];` をソート済みで出力。
- 参照ロジック（ランタイム）:
  1) マップ遷移や遭遇時に二分探索でAreaRuleを取得（`mapGroup/mapNum/areaMask`）。`tierId`を得る。
  2) シード生成は現行どおり`RandomizerRandSeed`（トレーナーID＋秘密ID）を使い、`data1`に`mapGroup/mapNum/area/slot`＋`tierId`をxor混入。テーブルが静的なので白OUT後も再現される。
  3) 既存モードで候補抽出→フィルタ（INVALID→グローバルBL→Tierルール）。`allowLegendary`とTier BL/WLで制御。
  4) リロールは`maxRerolls`上限。超過時は`window`を内側へ縮め、BST距離が最も近い種へクランプ。フォーム処理は最後。
  5) トレーナー/固定/ギフト/タマゴも同様に`tierId`を混ぜて種族決定、ただしシードの`data1`は各コンテキストのキー（trainerId/localIdなど）を維持。
- ビルドスクリプト案:
  - 置き場: `dev_scripts/merge_randomizer_tiers.py`（例）。Base/PatchのYAML/CSVを読み、重複キー検出・ソート・Patch適用（加算/減算/BL/WL変更）を行い、Cヘッダを生成。
  - 生成ファイルはGit管理するか、`generated/`をクリーン生成にするかは要決定。再現性優先なら生成物もコミット。
- 互換性・再現性:
  - バイナリが同じなら、トレーナーID＋秘密IDが同じ限り結果は固定。設定（ランダマイズモード）を変えたときだけ結果が変わるのは現仕様どおり。
  - テーブルを更新したビルドでは結果が変わる点は仕様として明示。必要ならテーブルのハッシュを計算・セーブに記録し、異なる場合に警告する運用も検討可（実装コストは高めなので任意）。
- テスト観点:
  - 白OUT後の再戦で同一マップ・同一スロットが同一種になることを確認（デバッグ用にseed/tierId/候補種族をログ出力するフラグがあると良い）。
  - リロール上限に達するケース（BLが厳しいエリア、低BST帯）でクランプ結果が暴れないか確認。
  - レジェンド可否: グローバル禁止＋エリア許可ケースをどう扱うか事前に仕様を固定（現提案はグローバル優先）。

詳細設計（WL方式、BSTなし版）
- 目的/前提: 全種デフォルト禁止（初期BL）。エリア/TierのWLに列挙された種のみ出現可。伝説も同様にWLに載せたときだけ出る。ランダマイザーON/OFFと「エリアWL適用」をフラグ/Varで切替。
- データ構造: `RandomizerAreaKey { u8 mapGroup; u8 mapNum; u8 areaMask; }`（Land/Water/Fish/Rock/Hiddenをビット保持）。`RandomizerAreaRule { RandomizerAreaKey key; const u16 *whitelist; const u16 *blacklist; u16 whitelistCount; u16 blacklistCount; u8 maxRerolls; bool8 allowLegendOverride; }`。生成ヘッダ例`generated/randomizer_area_rules.h`に配列と件数を出力。ROM節約ならプール＋offset/lengthでも可。
- フィルタ順: 1) `MON_RANDOMIZER_INVALID`、2) 初期BL（全禁止の前提）、3) エリアWLで許可、4) エリアBLで追加除外（任意）、5) フォーム処理。伝説はWL＋`allowLegendOverride`がtrueのエリアのみ解禁。
- シード/再現性: `RandomizerRandSeed`の`data1`に`mapGroup/mapNum/area/slot`を詰める現行仕様を維持。テーブルはビルド時静的生成（起動時再構築なし）。トレーナーID＋秘密IDが同じなら白OUT後も結果固定。WL適用フラグを触らない限り結果は不変。
- ランタイムフロー（野生例）: `RandomizeWildEncounter`でAreaKey検索（ソート表を二分探索）。既存モードで候補抽出→WLに無ければリロール（`maxRerolls`上限まで進める）。上限超過時は「最初に許可された候補を返す」か「元種族を返す」のどちらかを仕様で固定（推奨: 最初の許可候補）。最後にフォーム処理。
- トレーナー/固定/ギフト: 同じルール。`data1`のキーはtrainerId/partySlot/localId等を維持しつつAreaKeyでWLフィルタ。エリアを特定できないギフト用に専用AreaKey（例: 0xFF/0xFF）を持たせる。タマゴは今回は対象外とする。
- フラグ/オプション: 1) マスターON/OFF（既存フラグで全ランダマイザー有効/無効）。2) WL適用フラグOFFなら既存「何でもあり」モード、ONならエリアWL/BLでフィルタ。マスターOFF時は全無効。
- 新規フラグ/Var候補: エリアWL適用用フラグに`FLAG_UNUSED_0x04A`を割り当て（仮名`FLAG_RANDOMIZER_AREA_WL`）。モードVarが必要なら`VAR_UNUSED_0x4083`（仮名`VAR_RANDOMIZER_AREA_MODE`）を利用。マスターON/OFFは既存ランダマイザーフラグのまま。
- ビルドパイプライン: Base＋Patch（WL/BL加除）をYAML/CSVで管理し、スクリプト（例`dev_scripts/build_randomizer_area_rules.py`）でマージ→重複検知→ソート→生成ヘッダ出力。再現性重視なら生成物もコミット。
- リスク/注意: WL漏れで空エリアがないかをスクリプトで警告。リロール上限超過時のFallback仕様をテーブル生成時に検証。伝説解禁はWL明示＋`allowLegendOverride`で二重の安全弁。
- リロール上限の扱い: `maxRerolls`をエリアに持たせる（目安6〜10）。上限到達時のフォールバックは仕様で固定（推奨: 最初の許可候補、より多様性が欲しければWLを事前シャッフルし循環）。WLは配列加算/減算で徐々に広げる想定なので、候補が200〜300程度あれば上限到達は稀だが、WLが極端に少ない場合の警告をビルドスクリプトで出す。
- WLが空の扱い: 特定エリアランダマイザー適用かつマスターON時にWLが0ならビルドエラーにする（RANDOMIZER_TRAINER_ID_ENDのように定義ファイル必須扱い）。ランタイムでのフォールバック（元種族やデフォルトWL）は設けず、空定義は設計ミスとして検出する。
- 配列管理と重複対応: Base＋Patchのマージはスクリプトで行い、WL/BLの重複は同一IDとして自動重複除去（上書き扱い）。加算と減算がぶつかる場合は減算を優先して除去する。手動編集は避け、マージ結果はソートして一意化。
- モード組み合わせの優先順: 1) マスターON＋WL適用OFFなら従来の「全ポケ対象」ランダム（MON_RANDOM/BST等）だけ（特定エリア許可制は無効）。2) マスターON＋WL適用ONなら「既存モードで候補抽出→WL/BLで最終フィルタ」（最終可否はWLが決める）。両方ONにしない限り特定エリア許可制は動かない。将来エリア直指定モードを追加する場合は、優先順を設計書に明記（例: area-directならモード抽選をスキップしWLから直接選ぶ）。
- WLキット運用: Baseはフォーマット/エリア固有のWL、Patchは共通キット（例: スタートキット、中盤キット、終盤削除など）を加算/減算指示で適用。適用順を明示し、減算優先で相殺。再利用しやすいキット名で組み立てる運用を想定。
- ビルドスクリプトの扱い: 人が編集するのはYAML/CSV（例`data/randomizer/area_rules.yml`）のみ。ビルド時にスクリプト（`dev_scripts/build_randomizer_area_rules.py`など）が1回走り、重複除去・相殺・空WLチェックをして最終ヘッダ（例`generated/randomizer_area_rules.h`）を出力。Makefileには「YAMLが新しければ生成」程度の依存を足すだけ。実装詳細は後回しでOK、方針のみ共有。
- デバッグ/検証フック: ログ出力ON/OFF用に未使用Flagを1本割り当て（例: `FLAG_UNUSED_0x04B`を`FLAG_RANDOMIZER_DEBUG_LOG`に）。Scriptで`setflag/clearflag`して切替。必要ならVarでレベル指定（0=なし,1=基本,2=詳細）も検討。ログ内容は種族ID・使用ルールID・リロール回数など。コンパイルスイッチよりランタイム切替を優先。

実装済みメモ（釣り関連 v1.0）
- デバッグフラグ: `FLAG_RANDOMIZER_FISHING_AUTO_HOOK`（バイト+ミニゲーム省略で必ず遭遇）、`FLAG_RANDOMIZER_FISHING_AUTO_BITE`（バイト100%、ミニゲームは原作通り）。`debug.inc` Script 1 でオートフックもON。
- ロッド別上限: `fishingRule->maxSpecies` でWLの有効件数をクランプ（0ならエリア上限→件数）。Route102/Fishで実装例あり。
- レア抽選: `rareRate` は「先頭`rareSlots`件を引く確率」。バイト率とは独立。`rareSlots > wlLimit` は丸め。
- ルール検索: Fish/Water/Rock は同一mask優先、無い場合のみLandにフォールバック（釣りでLandを誤参照しない）。
- ログ: WARNで `[INFO] RandR ... fishing=1` が出る。`FLAG_RANDOMIZER_DEBUG_LOG` で有効。

実装済みメモ（WL/BL・トレーナー重複制御）
- エリアWL/BL適用: `FLAG_RANDOMIZER_AREA_WL` ONでエリア別WL/BLを適用。`data/randomizer/area_rules.yml` → `dev_scripts/build_randomizer_area_rules.py` → `generated/randomizer_area_rules.h` を生成し、野生・トレーナー双方でフィルタ。Land/Water/Fish/Rockのマスク検索は同一mask優先、無い場合のみLandにフォールバック。
- ランタイム: 野生は `RandomizeWildEncounter` でWLフィルタ＋`maxRerolls`リロール、超過時はWLから直接ピック（フォールバック）。トレーナーも同じWLフィルタを使用。
- トレーナー重複制御: `data/randomizer/trainer_dup_rules.h` でトレーナー別の `maxSame` / `minDistinct` を指定可能。未指定はデフォルト制限なし（255/0）。WL不足時は重複許容で妥協し、`maxSame=0`は避ける。
- デバッグフラグ: `FLAG_RANDOMIZER_DEBUG_LOG` でログ出力、Script 1 でWL＋ログ＋釣りオートフックを一括ON。ランダマイザーマスターは既存フラグ（WILD/TRAINERなど）でON/OFF。
- シード安定: 現行のシード計算（トレーナーID＋秘密ID、野生は mapGroup/mapNum/area/slot、トレーナーは trainerId/partySize/slot）を維持。テーブルはビルド時静的生成で白OUT後も結果固定。
