## アイテムランダマイザー拡張 実装タスク（v1.70 / TODO Script）

### 目的
- フィールドアイテムのランダマイズを「カテゴリ付きマスターテーブル」で運用し、個数固定・カテゴリ切替・将来の重み調整を可能にする。
- 隠しアイテムは完全無効化（入手不可）とし、実装はスクリプト側で分岐する。

### スコープ（確定）
- 対象: フィールドアイテムのみ。
- 対象外: 隠しアイテム（完全無効化）、NPC配布、キーアイテム、戦闘報酬。
- TM/HM は抽選対象に含める（カテゴリ切替で管理可能）。
- 抽選は **全カテゴリから**行う（比率はカテゴリ倍率で調整）。
- TM無限使用は既存設定 `I_REUSABLE_TMS` を利用（`TRUE` に設定する方針）。

### 参照/関連ファイル
- `src/randomizer.c` … `RandomizeFoundItem` と `FindItemRandomize_NativeCall` の実装箇所。
- `src/data/randomizer/item_whitelist.h` … 現行ホワイトリスト（置き換え対象）。
- `include/randomizer.h` … ランダマイザーAPI宣言。
- `asm/macros/event.inc` … `finditem` が `VAR_0x8000/0x8001` をセットする仕様。
- `data/scripts/obtain_item.inc` … `Std_FindItem` / `EventScript_HiddenItemScript`。
- `data/scripts/item_ball_scripts.inc` … `Common_EventScript_FindItem` の入口。
- `src/item_ball.c` … テンプレ量取得（上書き対象）。
- `include/config/randomizer.h` … ランダマイザー関連の設定追加先（ALLOW_EMPTY等）。
- `include/config/item.h` … `I_REUSABLE_TMS`。
- 参照URL: https://github.com/Pokabbie/pokeemerald-rogue/tree/expansion-dev

---
## データ定義（確定案）

### 1) カテゴリ enum（命名案）
```
enum RandomizerItemCategory
{
    RANDOMIZER_ITEMCAT_HEAL,
    RANDOMIZER_ITEMCAT_BALL,
    RANDOMIZER_ITEMCAT_BATTLE_USE,
    RANDOMIZER_ITEMCAT_HELD,
    RANDOMIZER_ITEMCAT_TOOL,
    RANDOMIZER_ITEMCAT_TM,
    RANDOMIZER_ITEMCAT_HM,
    RANDOMIZER_ITEMCAT_MEGA,
    RANDOMIZER_ITEMCAT_Z,
    RANDOMIZER_ITEMCAT_COUNT
};
```

### 2) カテゴリ設定テーブル（enabled + 既定個数 + 倍率）
```
struct RandomizerItemCategoryConfig
{
    bool8 enabled;     // カテゴリON/OFF
    u8 defaultQty;     // 既定個数
    u16 weightMul;     // カテゴリ倍率（初期は1）
};
```
配列名案:
- `static const struct RandomizerItemCategoryConfig sRandomizerItemCategoryConfig[RANDOMIZER_ITEMCAT_COUNT]`

初期値（確定）:
- HEAL=10, BALL=5, BATTLE_USE=3, HELD=1, TOOL=1, TM/HM/MEGA/Z=1
- 木の実は HELD、戦闘用は BATTLE_USE

### 3) アイテムテーブル
```
struct RandomizerItemEntry
{
    u16 itemId;
    u16 weight;       // 0禁止
    u8 category;      // RandomizerItemCategory
    u8 qtyOverride;   // 0ならカテゴリ既定
};
```
配列名案:
- `static const struct RandomizerItemEntry sRandomizerItemTable[]`
- `#define RANDOMIZER_ITEM_TABLE_COUNT ARRAY_COUNT(sRandomizerItemTable)`

運用ルール:
- `qtyOverride=0` → `defaultQty` を使用。
- `weight=0` はビルドエラー。
- キーアイテム/ITEM_NONE はテーブルに入れない。
- HMカテゴリは enabled で切替。

### 4) 追加設定（意図的に空を許可）
`include/config/randomizer.h` に以下を追加:
```
#define ALLOW_EMPTY_ITEM_POOL FALSE
```
- TRUE のときのみ「カテゴリONなのに候補0」を許可。

---
## ランダマイズ処理（設計詳細）

### 抽選フロー（方針）
1. `RandomizeFoundItem` を **カテゴリテーブル方式**に置き換える。
2. `RANDOMIZER_REASON_FIELD_ITEM` + (mapGroup/mapNum/localId + 元アイテム) でシード生成。
3. 有効カテゴリのみを抽出し、`weight * weightMul` の合計から重み抽選。
4. 選択結果から `itemId` と `quantity` を決定。
5. `FindItemRandomize_NativeCall` で `gSpecialVar_0x8000` と `gSpecialVar_0x8001` を上書き。

### 量の適用（上書き）
- `finditem` は `VAR_0x8001` を使用するため、`FindItemRandomize_NativeCall` で上書きすれば **テンプレ量を無視**できる。
- 既存テンプレ量（`GetItemBallIdAndAmountFromTemplate`）は **参照のみ**で上書きする。

### TM/HMの扱い
- 抽選は **全カテゴリから**（TM/HM固定ではない）。
- TM無限使用は `I_REUSABLE_TMS=TRUE` を想定。

---
## 隠しアイテム無効化（スクリプト方針）

### 対象
- `data/scripts/obtain_item.inc` の `EventScript_HiddenItemScript`

### 方針
- **早期終了**して「何も得られない」挙動にする。
- ダウジング反応を止めるため、**隠しアイテムの取得フラグ**を立てる。
  - `Script_ClearDowsingColor` / `SetHiddenItemFlag` / `Script_UpdateDowseState` の扱いを整理して実装。

---
## ビルド時バリデーション（厳格）

### 検証スクリプト（案）
- `dev_scripts/validate_randomizer_item_table.py` を新規作成。
- Makefile のビルド前フックに追加。

### 主要チェック（全てビルドエラー）
- `weight=0` の検出。
- `qtyOverride` が 0 以外のとき `>=1` であること。
- `category` が `RANDOMIZER_ITEMCAT_COUNT` 未満。
- `itemId` 重複の検出。
- `ITEM_NONE` / キーアイテム混入の検出（可能なら `src/data/items.h` から pocket を参照）。
- **有効カテゴリの候補が0**（全体で0）の場合はエラー  
  - ただし `ALLOW_EMPTY_ITEM_POOL=TRUE` のときは許可。
- エラーメッセージは **無効カテゴリ一覧** や **有効カテゴリ数=0** を明示する。

---
## 実装タスク一覧（コード変更は別タスク）

1. **データ定義**
   - `src/data/randomizer/item_table.h` を新設（`sRandomizerItemTable`）。
   - `src/data/randomizer/item_category_config.h` を新設（`sRandomizerItemCategoryConfig`）。
2. **ランダマイザー本体**
   - `src/randomizer.c` の `RandomizeFoundItem` をカテゴリテーブル式へ置換。
   - 量を返すためのヘルパ（例: `RandomizeFoundItemEx`）を追加。
   - `FindItemRandomize_NativeCall` で `gSpecialVar_0x8001` を上書き。
3. **隠しアイテム無効化**
   - `data/scripts/obtain_item.inc` の `EventScript_HiddenItemScript` に早期終了を追加。
4. **バリデーション**
   - `dev_scripts/validate_randomizer_item_table.py` を新規作成。
   - Makefile にバリデーション実行を組み込み。
5. **設定**
   - `include/config/randomizer.h` に `ALLOW_EMPTY_ITEM_POOL` を追加。
   - `include/config/item.h` の `I_REUSABLE_TMS` を TRUE 前提で運用。

---
## 既知の懸念点（実装時に注意）
- 隠しアイテム無効化は、**ダウジング反応の消し込み**まで処理しないと UX が崩れる可能性。
- `I_REUSABLE_TMS` は **全TM一律**。将来「一部だけ無限」にする場合は追加ロジックが必要。
- 一部の取得スクリプトが `Std_FindItem` を経由しない可能性があるため、**適用範囲の確認**が必要。
