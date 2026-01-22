# todo v1.71（フィールドアイテム：カテゴリ別アイコン）

## 目的
- フィールド上のアイテム（OBJ_EVENT_GFX_ITEM_BALL）を、
  **カテゴリごとに見た目を分ける** ことで判別性を上げる。
- ランダマイザーの結果は決定論的なので、**取得前でも同じ見た目**にできる。

## 方針
- v1.70 までは `OBJ_EVENT_GFX_ITEM_BALL` 固定。
- v1.71 では **カテゴリごとに専用の OBJ_EVENT_GFX を追加**し、
  マップロード時などに動的に切り替える。
- **サイズは 16x32 で統一**（既存のボールと同サイズ）。
  16x16相当の見た目にしたい場合は、上段を透明にして下寄せ表示する。

## 実装タスク
### 1) 画像素材の追加
- `graphics/object_events/pics/misc/` にカテゴリ別の画像を追加。
  - 例: `item_heal.4bpp`, `item_tm.4bpp`, `item_tool.4bpp` など。
- 可能なら **16x32（2x4 タイル）** で `PokeBall` と同サイズに揃える。
- アニメフレームは `sPicTable_PokeBall` と同じ 6 フレーム構成を想定。

### 2) GFX 定義の追加（オーバーワールド）
- `include/constants/event_objects.h`
  - `OBJ_EVENT_GFX_ITEM_HEAL`, `OBJ_EVENT_GFX_ITEM_TOOL` などを追加。
  - 末尾の `NUM_OBJ_EVENT_GFX` を更新。
- `src/data/object_events/object_event_graphics.h`
  - `gObjectEventPic_ItemHeal` 等を追加。
- `src/data/object_events/object_event_pic_tables.h`
  - `sPicTable_ItemHeal` 等を追加。
- `src/data/object_events/object_event_graphics_info.h`
  - `gObjectEventGraphicsInfo_ItemHeal` 等を追加。
- `src/data/object_events/object_event_graphics_info_pointers.h`
  - 上記をテーブルに登録。

### 3) 表示切り替えロジック
- **マップロード時**または**オブジェクト生成後**に、
  `OBJ_EVENT_GFX_ITEM_BALL` のイベントを走査して差し替える。
- 差し替え候補は `RandomizeFoundItemEx` で取得したカテゴリ。
  - `mapGroup/mapNum/localId + 元アイテムID` がシードになるため、
    **取得前でも結果は再現可能**。
- 実装方法（案）
  1. map load フックで全オブジェクトイベントを走査
  2. `graphicsId == OBJ_EVENT_GFX_ITEM_BALL` を対象に判定
  3. 対象の itemId を取得（テンプレートや item ball 用データから）
  4. `RandomizeFoundItemEx` でカテゴリを決定
  5. `ObjectEventSetGraphicsIdByLocalIdAndMap` で `OBJ_EVENT_GFX_ITEM_*` に切り替え

## カテゴリ → アイコン（予定）
- HEAL  : OBJ_EVENT_GFX_ITEM_HEAL
- BALL  : OBJ_EVENT_GFX_ITEM_BALL（既存）
- BATTLE_USE : OBJ_EVENT_GFX_ITEM_BATTLE
- HELD  : OBJ_EVENT_GFX_ITEM_HELD
- TOOL  : OBJ_EVENT_GFX_ITEM_TOOL
- TM    : OBJ_EVENT_GFX_ITEM_TM
- HM    : OBJ_EVENT_GFX_ITEM_HM
- MEGA  : OBJ_EVENT_GFX_ITEM_MEGA
- Z     : OBJ_EVENT_GFX_ITEM_Z
※ このカテゴリ粒度で確定。TM/HMは別アイコンで運用する。

## 設計メモ（任せ・詳細）
### 適用範囲
- 対象は **フィールドの item ball（OBJ_EVENT_GFX_ITEM_BALL）相当**のみ。
- ランダマイザー無効時は **何もしない**（既存ボール維持）。
- 特殊マップ（バトルピラミッド等）や動的生成のオブジェクトは、
  itemId の取得方法が異なる可能性があるため **除外/個別対応**の方針にする。
- **適用範囲は通常マップのみ**。上記の特殊系は全て除外する前提。

### itemIdの取得
- 通常の item ball は `gMapHeader.events->objectEvents[localId-1].trainerRange_berryTreeId` に itemId が入る。
  amount は `movementRangeX`（`src/item_ball.c` 参照）。
- localId とテンプレ配列の一致が前提。
  一致しないケースでは **処理をスキップ**する安全設計にする。

### 表示切り替えのタイミング
- 候補: map load 後（オブジェクト生成完了後）に一括走査して差し替え。
  `ObjectEventSetGraphicsIdByLocalIdAndMap` を使用。
- ロード時のみ行えば、常時の負荷は小さい。
※ このタイミングで確定。

### 分類ルール
- v1.70 の `RandomizeFoundItemEx` を **表示用にも再利用**する（決定論的）。
  RNG消費の問題は起きない。
- フラグ/設定によりカテゴリが無効の場合は **既定のボール表示にフォールバック**。
- 万一 `itemId == ITEM_NONE` の場合もフォールバック。
※ フォールバックは常にボール表示で確定。

### GFX/サイズ/パレット
- **16x32 統一**（既存ボールと同サイズ）。
  16x16相当の見た目は上段を透明にして下寄せ表示。
- `sPicTable_PokeBall` と同じ 6フレーム構成に合わせる。
  1フレーム素材のみの場合は同一フレームを繰り返す方式も検討。
- パレットは可能な限り **共通化**して枠不足を避ける。
- 素材は **Rogueの16x16素材を16x32化して移植**する方針。
※ 16x16素材は下寄せ固定で統一。

### パレット運用方針（決定）
- **カテゴリ別に独立パレットを持たせない**（枠不足回避）。
- 基本は **共通パレット1種**で全カテゴリを運用。
- 視認性が落ちる場合のみ **最大2種**まで許可（例: 特殊カテゴリ用のアクセント）。
- どうしても衝突が起きた場合は **共通1種に統合**して安全側に倒す。

### 将来メモ（未実装・予知）
- カテゴリとは別に、**特定アイテムだけ専用アイコンにする**案を残す。
  - 例: レアキャンディ / ミント / 特殊進化石 など。
  - 実装する場合は「カテゴリ判定より優先」の例外ルールが必要。

### 既存ロジックとの整合
- `graphicsId == OBJ_EVENT_GFX_ITEM_BALL` 前提の処理が残っている場合、
  別GFXに差し替えると判定対象外になる。
  → 切替範囲の限定 or 判定条件の拡張が必要。

### 互換性/セーブ影響
- ランダマイザーテーブルやカテゴリ構成変更により、
  **同じ場所の見た目/中身が変わる**可能性がある。
  運用中のテーブル更新は注意。

### 想定しないケースの扱い
- NPC配布・スクリプト入手・隠しアイテムには適用しない。
- itemId が判定できない場合は **必ず既存ボール表示に戻す**。

## 参照ファイル
- `src/randomizer.c` / `include/randomizer.h` : RandomizeFoundItemEx
- `data/scripts/item_ball_scripts.inc` : フィールドアイテム取得の入口
- `src/item_icon.c` : UI用のアイコン処理（フィールドとは別系統）
- `include/constants/event_objects.h`
- `src/data/object_events/object_event_graphics.h`
- `src/data/object_events/object_event_pic_tables.h`
- `src/data/object_events/object_event_graphics_info.h`
- `src/data/object_events/object_event_graphics_info_pointers.h`

## 参考実装（Pokemon Emerald Rogue: expansion-dev）
※ **本リポジトリでは未実装**。方針整理のための参照のみ。

- 追加されている OBJ_EVENT_GFX の例  
  `OBJ_EVENT_GFX_ITEM_SILVER_TM`, `OBJ_EVENT_GFX_ITEM_GOLD_TM`, `OBJ_EVENT_GFX_ITEM_POKE_BALL`,  
  `OBJ_EVENT_GFX_ITEM_MEDICINE`, `OBJ_EVENT_GFX_ITEM_HOLD_ITEM`, `OBJ_EVENT_GFX_ITEM_MASTER_BALL`,  
  `OBJ_EVENT_GFX_ITEM_EVO_STONE`, `OBJ_EVENT_GFX_ITEM_RARE_CANDY`, `OBJ_EVENT_GFX_ITEM_MINT`,  
  `OBJ_EVENT_GFX_ITEM_MEGA_STONE`, `OBJ_EVENT_GFX_ITEM_Z_CRYSTAL`, `OBJ_EVENT_GFX_ITEM_DYNAMAX_BALL`,  
  `OBJ_EVENT_GFX_ITEM_HEALING`, `OBJ_EVENT_GFX_ITEM_TERA_ORB`, `OBJ_EVENT_GFX_ITEM_TERA_SHARD`
- オブジェクトのGFX切替: `src/rogue_controller.c` で itemId / pocket 判定により
  `objectEvents[write].graphicsId` を上書きしている。
- アイコンサイズ: **16x16 / 1フレーム** を使用（`obj_frame_tiles` のみ）。

参照パス（Rogue側）:
- `include/constants/event_objects.h`
- `src/rogue_controller.c`
- `src/data/object_events/object_event_graphics_info.h`
- `src/data/object_events/object_event_graphics_info_pointers.h`
- `src/data/object_events/object_event_pic_tables.h`
- `src/data/object_events/object_event_graphics.h`
- `graphics/object_events/pics/rogue/`

## 注意点
- 画像が未用意の場合は `OBJ_EVENT_GFX_ITEM_BALL` を継続使用できるようにする。
- HM/TM を区別する場合は **別 GFX** を用意する。
- 将来的に「サブカテゴリ」表示をするなら、`OBJ_EVENT_GFX_ITEM_*` をさらに細分化する。
- `OBJ_EVENT_GFX_ITEM_BALL` 前提の既存処理が残っている場合、
  差し替え後に影響する可能性があるため注意。
- `NUM_OBJ_EVENT_GFX` の増加により動的GFX範囲との干渉が起きないか要確認。
- 16x16→16x32移植の際は、下寄せ固定でも**見た目のズレ**が起きないか確認する。
