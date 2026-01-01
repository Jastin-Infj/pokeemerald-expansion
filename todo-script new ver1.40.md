# todo-script new ver1.40.md — 詳細設計（v1.40: slotSets/重み/レベル帯/遭遇率のデータ駆動化）

## ゴール/スコープ
- 野生遭遇テーブルをデータ駆動化し、スロット数・重み（出現確率）・レベル帯・遭遇率・レア枠抽選を YAML で定義可能にする。
- slotSets でテンプレート化し、areas から参照する方式に移行。timeSlot/rod でも一部項目を上書き可能。
- バリデーションを厳格化し、不正データはビルド時にエラーで弾く。
- allowEmpty は slotSet 側でのみ管理（遭遇なし）。エリア側指定はエラー。
- rare 抽選は multiDraw（rareSlots 回独立抽選）で本実装。レアは末尾スロット置換、weight はレア/一般で別配列。

## 影響・変更予定ファイル（設計のみ、コード変更NG）
- `dev_scripts/build_randomizer_area_rules.py`
  - slotSets セクション対応、areas 参照、timeSlot/rod 上書き処理追加、バリデーション追加。
  - 生成物 (`generated/randomizer_area_rules.h`) の構造拡張（slotCount/weights/rareWeights/levelBands/encounterRate 等）。
  - WARN ログ出力強化（適用 slotSet、rareDraws、weightMode 等）。
- `data/randomizer/area_rules.yml`
  - 新スキーマ (slotSets + areas) で記述。サンプル slotSet 追加。
- `generated/randomizer_area_rules.h`
  - 生成物拡張（slotCount、weights、rareWeights、levelBands、encounterRate、allowEmpty、rareSlots、slotMode 等）。
- 既存フラグ/VAR
  - 既存の RANDOMIZER 系フラグ/VAR を流用（新規フラグ追加予定なし）。DEBUG ログ用フラグを流用して WARN を出す想定。

## スキーマ概要（新）
- `slotSets` セクション: 重いスロット定義をテンプレート化。
  - 必須キー:  
    - `slotCount`（実使用スロット数、種別上限・maxSpecies以内）  
    - `slotMode`（uniform|rare）  
    - `weightMode`（even|vanilla）※weights 未指定時に必須、weights と併記はエラー（どちらか一方）。通常/レア共通。  
    - `weights`（合計100、未指定なら weightMode 必須）  
    - `rareWeights`（合計100、rareSlots 上限内、未指定なら weightMode をレアにも適用）  
    - `encounterRate`（1–100 または vanilla。0 はエラー）  
  - 追加キー:  
    - `rareSlots`（0〜上限、maxSpecies/種別上限以内）  
    - `levelBands`（配列: {minLv,maxLv[,step]}、長さ = slotCount、maxLv<=100）  
    - `rareLevelBands`（配列: 長さ = rareSlots）  
    - `allowEmpty`（slotSet のみ。true の場合 slotCount 未指定=0、weights/rareWeights/encounterRate/levelBands 未指定必須）  
    - `rareDrawMode`（multi 固定、将来拡張用に key は予約可）  
- `areas` セクション: mapGroup/mapNum/areaMask/timeSlot/rod で slotSet を参照し、必要項目のみ上書き可。slotSet 自体の差し替えは不可（エリア単位で固定）。  
  - 上書き許可: slotCount, weights, weightMode, rareWeights, rareSlots, encounterRate, levelBands, rareLevelBands, slotMode（優先順位: slotSet → timeSlot → rod）。未対応キーや矛盾はエラー。  
  - allowEmpty は上書き不可。エリア側に書いた場合はエラー。

## バリデーション（厳格）
- slotSet 参照未定義 → エラー。
- weight/weightMode: weights と weightMode は排他。両方未指定はエラー。weights/rareWeights 合計≠100、負数 → エラー。weights 要素数≠slotCount、rareWeights 要素数≠rareSlots はエラー。
- encounterRate: 必須 1–100 or vanilla。0 はエラー（遭遇なしは allowEmpty/slotCount=0 で表現）。
- slotCount: 種別上限・maxSpecies 超過はエラー。rareSlots は 0〜上限（slotCount・種別上限・maxSpecies 以内）。
- levelBands/rareLevelBands: 整数、min>max でエラー、maxLv>100 でエラー。配列長が slotCount/rareSlots と不一致 → エラー。
- allowEmpty: slotSet のみ。allowEmpty=true なら slotCount 未指定=0、weights/rareWeights/encounterRate/levelBands 未指定必須。1 つでも記載でエラー。エリア側に allowEmpty があればエラー。
- バニラ遭遇例外時: weightMode=vanilla を強制（他指定があればエラー）。
- timeSlot/rod 上書き: 許可キー以外の上書き、優先順位違反はエラー。

## ログ方針（WARN）
- 出力例: `[WARN] RandR slotSet=mid_land_wt time=morning rod=any slotCount=12 weightMode=even rareDraws=1/3 encounterRate=25`
- 含める情報: slotSet 名、slotCount、weightMode(even|vanilla)、rare 抽選回数/当選数(rareDraws=x/y)、encounterRate、timeSlot/rod。
- 必要なら既存 DEBUG フラグで ON/OFF。

## rare 抽選とスロット
- rareDrawMode: multi（rareSlots 回まで独立抽選）。当選数ぶん末尾スロットをレアに置換。  
- rareSlots=0 でレアなし。rareSlots>slotCount でエラー。  
- weight/rareWeights は一般/レア別の配列（合計100）。  
- allowEmpty=true なら slotCount=0 で遭遇なし（weights 等は未指定）。

## slotSet サンプル（実データ用、ビルド通る形）
- vanilla 流用: slotCount 未指定（種別デフォルト）、weightMode=vanilla、encounterRate 未指定（vanilla）、rareSlots=0、weights/rareWeights/levelBands/rareLevelBands 未指定（vanilla扱い）。vanilla セットで遭遇系を明示したらエラー。
- 均等重み: slotCount=種別上限、weightMode=even、rareSlots=0、levelBands 未指定（バニラ流用）。
- レアなし＋weights 明示: slotCount=上限、weights 合計100、rareSlots=0。
- レアあり: weights/rareWeights 合計100、rareSlots>0、encounterRate 明示、levelBands 明示。
- allowEmpty: allowEmpty=true、slotCount 未指定(=0)、weights/rareWeights/encounterRate/levelBands 未指定。
- timeSlot/rod 上書き例: base slotSet=weighted_land、timeSlot:morning で weightMode=even＋encounterRate=vanilla 上書き、rod:super で slotCount=5＋rareSlots=1 上書き。
- エラー例はコメントのみ（make 失敗しない形）。例: weights と weightMode 併記、weight 合計≠100、rareSlots>slotCount、encounterRate 未指定、allowEmpty=true なのに weights 記載、未対応キー上書き。

## 移行ステップ
- ステップ1: 全エリアに slotSet: vanilla を割り当て（既存テーブル流用）、新スキーマでビルドが通ることを確認。
- ステップ2: ごく少数のテストエリアを新 slotSet（weighted/levelBand/encounterRate 指定）に差し替えて検証。
- ステップ3: 段階的に適用範囲を拡大し、バリデーション警告/エラーが出ないことを確認しつつ移行。

## 実装作業の流れ（参考）
- ステップA: スキーマ雛形を YAML で作成（vanilla slotSet＋テスト slotSet を含む）。
- ステップB: `build_randomizer_area_rules.py` をスキーマ対応に拡張（slotSets→areas 参照、バリデーション実装、生成物拡張）。
- ステップC: テスト用エリアで生成・ビルドが通るか確認。
- ステップD: ログ内容（rareDraws 等）を実機/エミュで確認し、必要なら調整。

## スキーマ雛形（コメント付きサンプルYAML）
```yaml
# YAML schema version
schemaVersion: 1.40

slotSets:
  # バニラ流用（遭遇系は全てバニラ扱い。遭遇系を明示したらエラー）
  vanilla:
    slotCount: null            # 種別デフォルト
    slotMode: uniform
    weightMode: vanilla        # weights未指定なのでvanilla必須
    rareSlots: 0
    allowEmpty: false
    # weights/rareWeights/levelBands/rareLevelBands/encounterRate は未指定

  # 均等重み（レアなし）
  even_land:
    slotCount: LAND_MAX_SLOTS
    slotMode: uniform
    weightMode: even           # weights未指定なのでeven必須
    rareSlots: 0
    encounterRate: 15          # 任意の整数 1–100
    # levelBands 未指定 → バニラ流用

  # レアなし + weights明示
  weights_land:
    slotCount: LAND_MAX_SLOTS
    slotMode: uniform
    weights: [10,10,10,10,10,10,10,10,10,10,10,0] # 合計100, 長さ=slotCount
    rareSlots: 0
    weightMode: null           # weightsを明示したので不要
    encounterRate: 15

  # レアあり（rareSlots>0）
  rare_land:
    slotCount: LAND_MAX_SLOTS
    slotMode: rare
    weights:      [10,10,10,10,10,10,10,10,10,10,10,0]  # 一般
    rareWeights:  [50,50]                               # レア（長さ=rareSlots）
    rareSlots: 2
    encounterRate: 20
    levelBands:
      - { minLv: 3, maxLv: 5 }
      - { minLv: 3, maxLv: 5 }
      - { minLv: 4, maxLv: 6 }
      - { minLv: 4, maxLv: 6 }
      - { minLv: 4, maxLv: 6 }
      - { minLv: 5, maxLv: 7 }
      - { minLv: 5, maxLv: 7 }
      - { minLv: 5, maxLv: 7 }
      - { minLv: 6, maxLv: 8 }
      - { minLv: 6, maxLv: 8 }
      - { minLv: 7, maxLv: 9 }
      - { minLv: 7, maxLv: 9 }
    rareLevelBands:
      - { minLv: 8, maxLv: 10 }
      - { minLv: 9, maxLv: 11 }

  # allowEmpty（遭遇なし）
  empty_area:
    allowEmpty: true
    slotCount: null            # =0扱い（未指定必須）
    slotMode: null
    rareSlots: 0
    weightMode: null
    encounterRate: null
    # weights/rareWeights/levelBands/rareLevelBands 未指定必須

areas:
  route_101_land:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE101), mapNum: MAP_NUM(MAP_ROUTE101), areaMask: Land }
    slotSet: vanilla

  route_102_land:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE102), mapNum: MAP_NUM(MAP_ROUTE102), areaMask: Land }
    slotSet: rare_land
    timeSlots:
      morning:
        # timeSlot上書き例（必要な項目だけ上書き）
        weightMode: even
        encounterRate: vanilla
      day: {}
      evening: {}
      night: {}

  route_102_fishing:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE102), mapNum: MAP_NUM(MAP_ROUTE102), areaMask: Fish }
    slotSet: even_land
    timeSlots:
      any:
        rod:
          super:
            slotCount: 5
            rareSlots: 1
            encounterRate: 25
```
（注）エラー例はコメントで示し、make が失敗しない形にすること。
```
