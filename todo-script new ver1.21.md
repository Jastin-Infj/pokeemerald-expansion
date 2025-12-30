# TODO-Script ver1.21（詳細設計）

## 目的
- timeSlots×rod分離・rare枠強化を前提に、生成スクリプト／ランタイム／データ仕様を明確化し、バリデーションを厳格にする。

## 現状の仕様メモ（todo ver1.21.md 要旨）
- areaMask/timeslot/rodは固定キー（Land/Water/Rock/Fish, morning/day/evening/night/any, old/good/super）。
- slotMode必須。uniformでrare指定禁止、rareはrareRate/rareSlotsセット必須（片方のみ/0はエラー）。rareRateは1〜100、rareSlotsは1〜種別上限。rareRate=100で最大までレアを埋め得る。
- allowEmptyはtimeSlot単位。allowEmpty=true時のみmaxRerolls/null可。空継承はNG、null＋allowEmptyで明示。
- allowLegendOverrideは共通→timeSlot→rodで上書き可、normal/rareで分割しない。伝説を含むrare.applyはallowLegendOverride=true必須。falseで伝説のみなら警告（allowEmptyで出現ゼロ許容）。
- fishingはrod完全分離。Fishでfishing未記載やrod欠落はエラー。各rodにmaxSpecies/maxRerolls必須（allowEmpty時のみnull可）。
- timeSlots=false/any：OW_TIME_OF_DAY_ENCOUNTERS=TRUEでfalseはエラー、anyで共通明記。FALSE環境はfalse/any許可（anyは非推奨、0xFF扱い）。
- timeSlotコード値: morning=0, day=1, evening=2, night=3, any/false=0xFF。
- プールはoffset/lengthをu32で保持、u16上限超過でエラー。apply/remove後は昇順ソート＋重複除去。
- ログはWARN基調。空WL/レア抽選などもWARN。

## 追加で明記する実装方針
1) データフォーマット（YAML想定）
- トップ: key{mapGroup,mapNum,areaMask}, slotMode, maxSpecies, maxRerolls, allowLegendOverride。
- default/rareをネスト（apply/remove）。rare未使用はrareRate/rareSlotsをnull。timeSlots有効時はトップのdefault/rare禁止、timeSlot配下で必須。
- timeSlots: { morning/day/evening/night/any or false }。falseはTRUE環境でエラー。
- fishing（Fishのみ必須）: old/good/super 各ブロックに default/rare + maxSpecies/maxRerolls（allowEmpty時のみnull可）。
- allowEmpty: timeSlot単位で一つ。空にしたい場合のみtrue＋null群で明示。
- maxSpecies/rareSlotsは種別定数（LAND_MAX_SLOTS/FISH_MAX_SLOTS/WATER_MAX_SLOTS/ROCK_MAX_SLOTS）以内。リテラル禁止。

2) 生成スクリプト改修（例: dev_scripts/build_randomizer_area_rules.py）
- バリデーション強化: 固定キー外エラー、必須欠落、rare片側指定、0指定、上限超過、timeSlots=false in TRUE環境など。
- エラー出力: mapGroup/mapNum/areaMask/timeSlot/rod を含めWARN/ERRORで出力（実行はエラーで止める）。
- ソート＋重複除去を標準化。
- プール構造: 単一u16配列をu32 offset/lengthで参照。u16上限超過でエラー。
- timeSlotsを有効化するとAreaRuleをtimeSlotごとに分割生成（timeSlotコード埋め込み）。false/anyは0xFF。
- fishing: rod別にRandomizerFishingRuleを生成。old/good/super全部必要。allowLegendOverride継承（共通→timeSlot→rod）。
- 出力先: generated/randomizer_area_rules.{h,c}（RandomizerAreaRule + RandomizerFishingRule定義とテーブル）。
- 定数参照: *_MAX_SLOTS, timeSlotコード値。ヘッダ例: include/constants/randomizer_slots.h。
- テストデータ: dev_scripts/tests/randomizer_yamls/ に正常/エラーケースを配置し、スクリプトで回す。

3) ランタイム改修（例: src/randomizer/*.c）
- AreaRule検索時にtimeSlotキーを含める（data1にtimeSlot/rodを詰める想定を維持）。
- Fishing導線: rod別FishingRuleを参照し、共通WLフォールバックを廃止。rare抽選はFishingRuleのrareRate/rareSlotsを使用。
- allowLegendOverride: 共通→timeSlot→rodの値で判定。rare/normal共通。
- デバッグログ: WARNでrareヒット/空WL/allowEmptyなどを出力。

4) データサンプル（要件準拠版）
- timeSlotsあり・夜だけrare（空継承はallowEmpty+nullで明示）
  ```
  key: { mapGroup: MAP_GROUP(MAP_ROUTE103), mapNum: MAP_NUM(MAP_ROUTE103), areaMask: Fish }
  slotMode: rare
  maxSpecies: FISH_MAX_SLOTS
  maxRerolls: 6
  allowLegendOverride: false
  timeSlots:
    morning:
      default: { apply: [starter_water, late_water], remove: null }
      rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
    day:
      allowEmpty: true
      default: null
      rare:    null
    evening:
      allowEmpty: true
      default: null
      rare:    null
    night:
      allowLegendOverride: true
      default: { apply: [starter_water], remove: null }
      rare:    { apply: [legend_unlock], remove: [starter_water, late_water], rareRate: 100, rareSlots: 1 }
  ```
- rod分離（morningのみ稼働、他はallowEmptyで空を明示）
  ```
  key: { mapGroup: 2, mapNum: 10, areaMask: Fish }
  slotMode: rare
  maxSpecies: FISH_MAX_SLOTS
  maxRerolls: 6
  allowLegendOverride: false
  timeSlots:
    morning:
      fishing:
        old:   { default: { apply: [starter_water], remove: null }, rare: { apply: null, remove: null, rareRate: null, rareSlots: null }, maxSpecies: FISH_MAX_SLOTS, maxRerolls: 6 }
        good:  { default: { apply: [starter_water, mid_water], remove: null }, rare: { apply: null, remove: null, rareRate: null, rareSlots: null }, maxSpecies: FISH_MAX_SLOTS, maxRerolls: 6 }
        super: { allowLegendOverride: true, default: { apply: [starter_water, late_water], remove: null }, rare: { apply: [legend_unlock], remove: [starter_water, late_water], rareRate: 100, rareSlots: 1 }, maxSpecies: FISH_MAX_SLOTS, maxRerolls: 6 }
    day:
      allowEmpty: true
      fishing:
        old:   { default: null, rare: null, maxSpecies: null, maxRerolls: null }
        good:  { default: null, rare: null, maxSpecies: null, maxRerolls: null }
        super: { default: null, rare: null, maxSpecies: null, maxRerolls: null }
    evening: 同上
    night:   同上
  ```
- timeSlots無し（OW_TIME_OF_DAY_ENCOUNTERS=FALSE想定）
  ```
  key: { mapGroup: MAP_GROUP(MAP_ROUTE101), mapNum: MAP_NUM(MAP_ROUTE101), areaMask: Land }
  slotMode: uniform
  maxSpecies: LAND_MAX_SLOTS
  maxRerolls: 6
  allowLegendOverride: false
  default: { apply: [starter_land], remove: null }
  rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
  timeSlots: false  # FALSE環境のみ許可。TRUEではanyを明記。
  ```
- OW_TIME_OF_DAY_ENCOUNTERS=FALSEでany（非推奨）
  ```
  key: { mapGroup: 3, mapNum: 5, areaMask: Land }
  slotMode: uniform
  maxSpecies: LAND_MAX_SLOTS
  maxRerolls: 6
  allowLegendOverride: false
  timeSlots:
    any:
      default: { apply: [starter_land, mid_land], remove: null }
      rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
  ```

## 変更予定ファイルと主な役割
- dev_scripts/build_randomizer_area_rules.py（生成スクリプト）
  - 新フォーマットのパース・バリデーション・ソート・プール構築・生成物出力。
  - timeSlot分割、rod分離、定数参照、エラーメッセージ強化。
- generated/randomizer_area_rules.{h,c}
  - RandomizerAreaRule/RandomizerFishingRule定義とテーブル出力（生成物）。
- include/constants/randomizer_slots.h（新設想定）
  - LAND_MAX_SLOTS/WATER_MAX_SLOTS/ROCK_MAX_SLOTS/FISH_MAX_SLOTS と timeSlotコード値を定義。
- src/randomizer/*（ランタイム）
  - timeSlotキー対応のAreaRule検索、FishingRule参照、rare抽選ログのWARN出力、allowLegendOverride継承。
- dev_scripts/tests/randomizer_yamls/*（新設）
  - サンプル・正常系・エラー系を配置し、スクリプトのバリデーションテストに使用。

## 作業順（推奨）
1. 生成スクリプトのバリデーション強化（既存データを落とさずにエラー粒度を揃える）。
2. 出力フォーマットをrod分離/timeSlot分割版に切替（ヘッダ/テーブル出力を更新）。
3. ランタイムを新フォーマット参照に更新（fishing導線置換、allowLegendOverride継承固定）。
4. サンプルYAMLとテストケースを追加し、生成スクリプトテストを整備。

## 現状コード調査の抜粋（修正前の挙動と差分）
- 生成スクリプト: dev_scripts/build_randomizer_area_rules.py
  - 入出力: data/randomizer/area_rules.yml → generated/randomizer_area_rules.h
  - 構造体: RandomizerFishingRule{u8 slotMode,maxSpecies,rareSlots,rareRate} / RandomizerAreaRule{mapGroup,mapNum,areaMask,timeSlot,wl ptr/bl ptr,count,maxRerolls,allowLegendOverride,slotMode,maxSpecies,rareSlots,rareRate,fishingRules ptr,count}
  - areaMask解決: Land/Water/Rock/Fishing(=Fish)/Hidden/Gift。整数1bitも許容。
  - スロット上限: Land=12, Water/Rock=5, Fishingはrod別; rod上限 old=2, good/super=5。
  - timeSlots: 4キー(morning/day/evening/night)必須。未指定はWARNでANY(0xFF)扱い。extraキーはエラー。空{}でも通る。
  - rare検証: slotMode=uniformでrareRate/rareSlots禁止。rareSlots=0でrareRate>0はエラー。rareSlots>maxSpecies/slotLimitはエラー。rareRate 0-100。0は未使用扱い。
  - maxSpecies: defaultはslotLimit、0は無効扱い。allowEmpty相当なし。
  - fishing処理: rod apply/removeを基底のapply/removeにマージし、rodごとのWL/BLは持たない。rod rare/maxSpeciesはFishingRuleに格納するが、WLは共通。
  - timeSlot分割: timeSlotsあり→AreaRuleを4エントリ生成（timeSlot=0..3）。false/anyの概念なし。
  - 生成物: wl/blはpool配列+ポインタ、fishingRulesは単一配列でareaRuleがoffsetを指す（rodWLなし）。
- データ例: data/randomizer/area_rules.yml
  - route_103_fishingのtimeSlotsに{}があり、現行スクリプトでは許容。slotMode=uniformトップ + rod rare が混在（rod rareのみ有効）。legend解禁はremoveで共通WLを削除する形。
- ランタイム: src/randomizer.c
  - FindAreaRule(mapGroup,mapNum,areaMask,timeSlot)でAreaRule取得。timeSlotをdata1に詰めるシードは mapGroup<<24 | mapNum<<16 | area<<8 | slot。
  - Fishing: SelectFishingRule(areaRule, rodType)でFishingRuleを取得。slotMode/rareSlots/rareRateはFishingRuleを優先。WL/BLはAreaRule共通を使用（rod別WLなし）。
  - rare抽選: rareSlots>wlLimit → rareSlots=wlLimit。rareCount>0 && rareRate>0 でレア抽選。maxRerollsまでリロール。許容されるか SpeciesAllowedByRule() で判定。
  - allowLegendOverride: AreaRule単位。rare/normalで分離なし。
  - Debugログ: DebugLogWildRareでrareHit/targetStart等をWARN出力（NDEBUG無効時）。
- ビルド統合: Makefile に RANDOMIZER_RULES := generated/randomizer_area_rules.h があり、生成物を前提にビルド。
- 不足/乖離点（今回補う対象）
  - rod別WL/BLなし（apply/removeを共通にマージするため、rodごと伝説解禁ができない）。
  - timeSlots未指定をWARN+ANYにしている（仕様では禁止/明示）。
  - allowEmptyなし、空{}許容。空WLは無条件エラーでallowEmptyフラグがない。
  - rareRate/rareSlots=0が未使用扱い。slotMode=rareで全スロットnull/0でも通る。
  - timeSlotキーは4つ固定、any/falseなし。OW_TIME_OF_DAY_ENCOUNTERS設定との整合なし。
  - FishingRuleにWLオフセットが無い。出力も.hのみ（.c分離なし）。

## 今後の設計反映のポイント（調査踏まえた修正対象）
- スクリプト側
  - rod別WL/BLを持つ構造に変更（pool offset/lengthをFishingRuleに追加）。
  - timeSlots: false/any導入。TRUE環境でfalseエラー、FALSE環境でfalse/any許容（any非推奨）。未指定はエラーに変更。
  - allowEmpty導入: timeSlot単位。空{}禁止、null+allowEmptyで明示。maxRerolls null可（allowEmpty時のみ）。
  - rare必須セット: slotMode=rareならrareRate/rareSlotsセット必須（0/片側/全slot nullはエラー）。uniformでrare指定があればエラー。
  - maxSpecies: 定数参照必須、0は禁止（allowEmpty時のみnull）。rareSlots/rareRateと矛盾チェック。
  - エラー/警告をWARN/ERRORでstderrに出し、mapGroup/mapNum/areaMask/timeSlot/rodを含める。
  - 生成物: .h/.c分離（または.h内static）。timeSlotコード値定義、*_MAX_SLOTS定義を共通ヘッダに切り出す。
  - テストYAMLを dev_scripts/tests/randomizer_yamls/ に追加し、正常/エラーケースをCI風に回す。
- ランタイム側
  - AreaRuleをtimeSlot含めて検索、FishingRuleでrod別WL/BLを参照するよう変更（共通WL依存を廃止）。
  - allowLegendOverrideの継承を共通→timeSlot→rodで固定。rare/normal共通値。
  - rare抽選はFishingRuleのpool範囲（rare/normal offset/len）を使うようにする。
  - デバッグログはWARNで出す。allowEmptyや空WL/BLでの挙動もログ補足。

