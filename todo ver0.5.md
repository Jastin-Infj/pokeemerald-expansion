場所別ランダマイザー 詳細設計（WL方式、BSTなし）  
※実装は未着手。設計と運用を固めるためのメモ。

1. 目的と前提
- 全種は初期BL（出現不可）。WLに載った種のみ出現可。伝説もWL＋allowLegendOverrideで明示解禁。
- ランダマイザーON/OFFは既存フラグを使用。エリアWL適用は新規フラグ/Varで切替。
- シードは現行どおりトレーナーID＋秘密ID。テーブルをバイナリに静的生成して再現性を確保。
- タマゴは対象外。ギフトは対象（エリア特定できない場合は専用AreaKeyを使う）。

2. フラグ/Var（案）
- マスター: 既存ランダマイザーフラグ（変更なし）。
- WL適用フラグ: `FLAG_UNUSED_0x04A` → `FLAG_RANDOMIZER_AREA_WL`（ONでエリアWL/BLを適用）。
- モードVar（必要なら）: `VAR_UNUSED_0x4083` → `VAR_RANDOMIZER_AREA_MODE`（0=従来、1=WL適用など）。
- デバッグログ: `FLAG_UNUSED_0x04B` → `FLAG_RANDOMIZER_DEBUG_LOG`（Scriptでset/clear）。

3. データ構造（ランタイム）
- RandomizerAreaKey { u8 mapGroup; u8 mapNum; u8 areaMask } // Land/Water/Fish/Rock/Hiddenビット
- RandomizerAreaRule { key; const u16 *wl; const u16 *bl; u16 wlCount; u16 blCount; u8 maxRerolls; bool8 allowLegendOverride; }
- ギフト用の専用AreaKey例: mapGroup=0xFF, mapNum=0xFF, areaMask=0。
- 配列はプール＋offset/length方式でも可。ソート済み配列を二分探索。

4. フィルタ順序（WL方式）
1) MON_RANDOMIZER_INVALID除外  
2) 初期BL（全禁止の前提）  
3) エリアWLで許可  
4) エリアBLで追加除外（任意）  
5) フォーム処理（既存Random/Special）  
伝説はWLに載せ、allowLegendOverrideがtrueのときのみ解禁。

5. ランタイムフロー
- 共通: シードは`RandomizerRandSeed`のdata1にmapGroup/mapNum/area/slot（ギフトは専用Key）を詰める現行仕様を維持。
- 野生: AreaKey検索→既存モードで候補→WLに無ければリロール（maxRerolls）→上限時は「最初の許可候補」か「シャッフルWLの次」を返す（仕様で固定）→フォーム処理。
- トレーナー/固定/ギフト: 同上。data1のキーはtrainerId/partySlot/localId等を維持。ギフトは専用AreaKeyのWLを適用。
- モード優先順: マスターON＋WL適用OFFなら従来の全ポケ対象ランダムのみ。マスターON＋WL適用ONなら既存モードで候補→WL/BLで最終判定。両方ONにしない限り特定エリア許可制は動かない。

6. リロール・空WL
- maxRerollsをエリアごとに持たせる（目安6〜10）。候補200〜300想定なら上限到達は稀。
- WLが0ならビルドエラー（ランタイムフォールバック無し）。
- リロール上限超過時の挙動は仕様で固定（推奨: 最初の許可候補、または事前シャッフルWLの循環）。

7. WLキット運用（加算/減算）
- Base: フォーマット/エリア固有WL。Patch: キット（starter_base, mid_game, starter_trim など）を加算/減算で適用。適用順を明示し、減算優先で相殺。重複は自動除去。
- 終盤で序盤キットを引くなど、加減算で調整しやすい構成にする。

8. ビルドパイプライン方針
- 人が編集: `data/randomizer/area_rules.yml`（例）にキットと適用指示を書く（species名でOK）。
- スクリプト: `dev_scripts/build_randomizer_area_rules.py`（例）がBase＋Patchをマージし、重複除去・空WL/閾値チェック・ソートを実施し、`generated/randomizer_area_rules.h`を出力。
- Makefile: 「YAMLが新しければ生成」程度の依存を追加。生成物は再現性重視ならコミット。

9. デバッグ/検証
- `FLAG_RANDOMIZER_DEBUG_LOG`でログON/OFF。内容: 種族ID、使用ルールID(AreaKey/tierId相当)、リロール回数、候補数など。必要ならVarでログレベルを拡張。
- ビルドスクリプトで警告/エラー: WL空、候補数閾値未満、apply/removeの相殺結果が0件など。

10. 実データ作成の目安
- 序盤エリア: WL10〜30程度、低進化段階、伝説なし。  
- 中盤: 序盤キット＋中盤キット、starter_trimで序盤を削る。  
- 終盤: 序盤削除＋中盤＋終盤キット、高BST/最終進化、必要なら伝説キット＋allowLegendOverride。  
- 水路/洞窟など地形別キットを用意すると管理が楽。

参照元: include/config/randomizer.h, include/randomizer.h, include/pokemon.h, include/constants/flags.h, include/constants/vars.h, src/randomizer.c, src/wild_encounter.c, ~/dev/Imperium/src/randomizer.c（ギフト/タマゴの抽選方式参考）。

11. 実装ステップ（具体化案・コード未変更）
- ステップA: フラグ/Varの確定
  - `FLAG_RANDOMIZER_AREA_WL`（0x04A）、`FLAG_RANDOMIZER_DEBUG_LOG`（0x04B）、必要なら`VAR_RANDOMIZER_AREA_MODE`（0x4083）をconstantsに追加。マスターON/OFFは既存のまま。
  - ScriptでON/OFFできるようにする（簡易テスト用のinc/スクリプトにsetflag/clearflagを用意）。

- ステップB: データ入力フォーマットの決定
  - `data/randomizer/area_rules.yml`に決定（雛形追加済み）。構造: kits（共通キット）、areas（エリア/フォーマットごとのapply/remove/maxRerolls/allowLegendOverride）、gifts（ギフト用AreaKey）。areaMaskはLand/Water/Fish/Rock/Hidden/Gift等で表記。
  - species名は`SPECIES_***`で記述。areaは`mapGroup/mapNum/areaMask`をキーにする。

- ステップC: ビルドスクリプト設計
  - 場所: `dev_scripts/build_randomizer_area_rules.py`（例）。
  - 入力: `area_rules.yml`。
  - 処理: (1) kitsを辞書化、(2) areasのapply/removeを展開し重複除去（減算優先）、(3) 空WL検出/閾値チェック、(4) ソート、一意化、(5) `generated/randomizer_area_rules.h`を出力（プール＋offset/length方式が望ましい）。
  - 出力内容: `const u16 gRandomizerAreaSpeciesPool[]; const struct RandomizerAreaRule gRandomizerAreaRules[]; const u16 gRandomizerAreaRulesCount;` など。
  - ログ/警告: 空WL、候補数少なすぎ、apply/remove競合、未定義キット名など。

- ステップD: Makefile組込み
  - 依存関係: `generated/randomizer_area_rules.h` は `area_rules.yml` とスクリプトに依存。YAMLが新しければ再生成。
  - 生成物をコミットするかどうかを決定（再現性重視ならコミット）。

- ステップE: ランタイム統合（変更候補箇所）
  - `include/randomizer.h`: 新規APIやAreaKey型の宣言が必要なら追加。
  - `src/randomizer.c`:
    - `RandomizeWildEncounter`/`RandomizeTrainerMon`/`RandomizeFixedEncounterMon`/ギフト用呼び出しにAreaKey→Rule検索→WLフィルタを挿入。
    - Rule検索は`gRandomizerAreaRules`の二分探索ヘルパを追加（キャッシュしても可）。
    - リロール処理に`maxRerolls`とフォールバック（最初の許可候補 or シャッフルWL循環）を組み込む。
    - デバッグログは`FLAG_RANDOMIZER_DEBUG_LOG`を見て、Rule ID/候補数/リロール回数/最終種族を出力（mgbaやserial_printf等、既存手段に合わせる）。
  - ギフト: `RandomizeStarterAndGiftMon`呼び出しのシード生成を維持しつつ、専用AreaKey経由でWLを適用（タマゴは対象外）。

- ステップF: データ初期投入
  - kits例: `starter_base`（序盤弱ポケ）、`mid_game`、`late_game`、`starter_trim`（序盤削除）、`legend_unlock`（伝説専用）。
  - areas例: 序盤（10〜30種, maxRerolls=6, 伝説不可）、中盤（序盤＋中盤−削除, maxRerolls=8）、終盤（序盤削除＋中盤＋終盤＋必要ならlegend_unlock, maxRerolls=8〜10）。
  - gifts: `gift_default`に序盤キット＋必要ならmid_gameを適用。

- ステップG: テスト/検証
  - ログフラグONで、特定マップ/スロットを歩いて再現性確認。
  - WL空/少数エリアがビルドで弾かれることを確認。
  - リロール上限に達したケースでフォールバックが仕様どおり動くか確認。

- ステップH: 将来拡張メモ（検証プラン付き）
  - 互換性/優先順: モード追加（area-direct等）をする場合も、マスターON→WLフラグON→既存モード候補→WL/BLフィルタの順を維持。
  - 伝説解禁は`allowLegendOverride`と伝説キットで二重に明示する。タマゴは非対応のまま。対応するなら専用AreaKey＋WLを追加。
  - 手動検証シナリオ
    - フラグ切替: `FLAG_RANDOMIZER_AREA_WL` ON/OFFで野生/トレーナー/固定/ギフトの挙動が切り替わること。`FLAG_RANDOMIZER_DEBUG_LOG` ONでログが出ること（NDEBUGビルドでは出ない点に注意）。
    - ルール適用: Route101/102/110/121/128でWLに無い種が出ないこと。Route128水路のみallowLegendOverride=trueでlegend_unlockが出ること。
    - 減算確認: Route102/121でstarter_trim種が除外されていること。
    - リロール上限: maxRerollsを意図的に低くしてビルドし、フォールバック時にログへ"(fallback)"が出ること。
    - ギフト: giftルール（0xFF/0xFF/Gift）が適用され、WL内の種になること（タマゴは対象外）。
  - 作業メモ
    - データ更新後は `python3 dev_scripts/build_randomizer_area_rules.py --in data/randomizer/area_rules.yml --out generated/randomizer_area_rules.h` → `make`。
    - ログは`FLAG_RANDOMIZER_DEBUG_LOG`をset/clearで切替。mgba/no$gba/AGBPrintいずれのハンドラでも出力される。
