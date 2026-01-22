## フィールドアイテム カテゴリ別アイコン 実装タスク（v1.71 / TODO Script）

### 目的
- フィールド上の item ball を **カテゴリ別アイコン**で可視化する。
- ランダマイザーの決定論的な結果を利用し、**取得前でも同じ見た目**にする。

### スコープ（確定）
- 対象: **通常マップのフィールドアイテム**のみ。
- 除外: 特殊マップ（バトルピラミッド等）、動的生成オブジェクト、隠しアイテム、NPC配布。
- ランダマイザー無効時は **一切変更しない**（既存ボール表示）。

### 決定事項（確定）
- **カテゴリ粒度**: HEAL / BALL / BATTLE_USE / HELD / TOOL / TM / HM / MEGA / Z
  - TM/HM は **別アイコン**で運用。
- **優先度**: まずは **大カテゴリのみ**で実装し、サブカテゴリの細分化は後回し。
- **TOOL代表アイコン**: `item_rare_candy` を採用。
- **通常マップ範囲**: 地下/ダンジョン系は含めるが、`MAP_TYPE_UNDERWATER` は除外。
- **サイズ**: 16x32 統一（既存ボールと同サイズ）。
  - Rogue の 16x16 素材を **下寄せ固定**で 16x32 に移植。
- **切替タイミング**: map load 後（オブジェクト生成完了後）に一括走査。
- **フォールバック**: itemId 取得失敗 / カテゴリ無効 / 未定義は常にボール表示。
- **パレット**: 基本 1種で共通、必要なら最大 2種まで（衝突したら統一優先）。

---
## 実装タスク（手順）

### 0) 最小実装ルート（順番固定）
1. 画像素材の移植（Rogue 16x16 → 16x32 下寄せ）を完了
2. OBJ_EVENT_GFX 定義と GFX 登録（graphics/pic_tables/info/pointers）を追加
3. マップロード後の一括差し替え関数を追加
4. `overworld.c` から差し替え関数を呼ぶ（通常マップのみ）
5. デバッグマップで表示確認

### 1) 素材の移植（Rogue 16x16 → 16x32）
- Rogue の `graphics/object_events/pics/rogue/item_*.png` を参照して移植する。
- 本リポの格納先は `graphics/object_events/pics/misc/` を想定。
- 16x16素材は **下寄せ固定**で 16x32化。
- フレームは `sPicTable_PokeBall` と同じ 6フレーム構成に合わせる。
  - 1フレーム素材のみの場合は同じフレームを並べる方式でOK。
具体例:
- `item_healing.png` を `graphics/object_events/pics/misc/item_healing.png` に追加
- 上段 16x16 を透明、下段 16x16 に Rogue 素材を配置
- 6フレーム分を 2x4 タイルの並びで作成（ボールと同じ構成）

### 1.1) Rogue素材の候補リスト（実移植対象）
- `item_healing.png`
- `item_medicine.png`
- `item_poke_ball.png`
- `item_gold_tm.png`
- `item_silver_tm.png`
- `item_held_item.png`
- `item_mega_stone.png`
- `item_z_crystal.png`
- `item_rare_candy.png`（TOOL用の候補）
- `item_evo_stone.png`（TOOL用の候補）
- `item_mint.png`（TOOL用の候補）
備考:
- `item_master_ball.png` / `item_dynamax_ball.png` / `item_tera_orb.png` / `item_tera_shard.png` は **将来用の予備**として残す。

### 2) OBJ_EVENT_GFX 定義の追加
- `include/constants/event_objects.h`
  - `OBJ_EVENT_GFX_ITEM_HEAL`, `OBJ_EVENT_GFX_ITEM_BATTLE`, `OBJ_EVENT_GFX_ITEM_HELD`,
    `OBJ_EVENT_GFX_ITEM_TOOL`, `OBJ_EVENT_GFX_ITEM_TM`, `OBJ_EVENT_GFX_ITEM_HM`,
    `OBJ_EVENT_GFX_ITEM_MEGA`, `OBJ_EVENT_GFX_ITEM_Z` などを追加。
  - `NUM_OBJ_EVENT_GFX` を更新。

### 3) GFX登録（オーバーワールド）
- `src/data/object_events/object_event_graphics.h`
  - `gObjectEventPic_ItemHeal` などを追加。
- `src/data/object_events/object_event_pic_tables.h`
  - `sPicTable_ItemHeal` などを追加。
  - `sPicTable_PokeBall` と同様の 6フレーム構成で定義。
- `src/data/object_events/object_event_graphics_info.h`
  - `gObjectEventGraphicsInfo_ItemHeal` 等を追加。
  - `gObjectEventGraphicsInfo_PokeBall` の設定を踏襲（16x32 / inanimate / same anims）。
- `src/data/object_events/object_event_graphics_info_pointers.h`
  - 上記の `OBJ_EVENT_GFX_ITEM_*` を登録。

### 4) 表示切り替えロジックの追加
- map load 後に **OBJ_EVENT_GFX_ITEM_BALL** を走査し、カテゴリ別 GFX に差し替える。
- `RandomizeFoundItemEx` を **表示用にも再利用**して、ランダマイザー結果を得る。
  - 決定論的なので RNG 消費問題はなし。
- itemId 取得は `gMapHeader.events->objectEvents[localId-1].trainerRange_berryTreeId` を基本とする。
  - localId とテンプレ配列の不一致に備え、**安全スキップ**できる分岐を用意する。
- 取得済みフラグが立っている item ball はスキップする。

#### 具体的な挿入ポイント（候補）
- `src/overworld.c`
  - `InitObjectEventsLocal` の `TrySpawnObjectEvents(0, 0);` の直後
  - `InitObjectEventsReturnToField` の `SpawnObjectEventsOnReturnToField(0, 0);` の直後
- ここで `Randomizer_UpdateItemBallGfxForMap();` を呼ぶ
  - 通常マップのみ / ランダマイザーONのみ / イベントありのみで早期return

#### 具体的なヘルパ追加（候補）
- `src/randomizer.c` に以下を追加し、`include/randomizer.h` で宣言
  - `bool8 Randomizer_GetItemCategory(u16 itemId, u8 *outCategory);`
  - `u16 Randomizer_GetItemBallGfxByCategory(u8 category);`
  - `void Randomizer_UpdateItemBallGfxForMap(void);`

#### 具体的な判定条件（案）
- `if (!RandomizerFeatureEnabled(RANDOMIZE_FIELD_ITEMS)) return;`
- `if (gMapHeader.events == NULL || gMapHeader.events->objectEventCount == 0) return;`
- `if (!IsNormalMap()) return;`  ※特殊マップは除外
- `if (templ->graphicsId != OBJ_EVENT_GFX_ITEM_BALL) continue;`
- `if (templ->flagId != 0 && FlagGet(templ->flagId)) continue;`
- `if (itemId <= ITEM_NONE || itemId >= ITEMS_COUNT) continue;`（安全側）

#### IsNormalMap の具体案（暫定）
目的は **「通常マップのみ」** に絞ること。
最低限の除外として Battle Frontier 系を外し、必要なら mapType でも絞る。

```
static bool8 IsNormalMap(void)
{
    // Battle Frontier / 特殊施設は除外
    if (InBattlePyramid_() || InBattlePike() || InBattleFactory() || InTrainerHillChallenge())
        return FALSE;

    // マップ種別ベースの除外（必要なら追加）
    if (gMapHeader.mapType == MAP_TYPE_SECRET_BASE || gMapHeader.mapType == MAP_TYPE_UNKNOWN)
        return FALSE;

    // 地下/屋内は対象に含めるが、海中のみ除外
    if (gMapHeader.mapType == MAP_TYPE_UNDERWATER)
        return FALSE;

    return TRUE;
}
```

補足:
- `InTrainerHillChallenge()` が未使用なら除外条件から外してOK。
- `IsMapTypeOutdoors()` のみに絞ると **屋内のボールが全て対象外**になるので注意。

#### itemIdの取得元（再掲）
- `gMapHeader.events->objectEvents[localId-1].trainerRange_berryTreeId`
- `localId == 0` または `localId-1 >= objectEventCount` は **スキップ**

---
## 参考実装（Pokemon Emerald Rogue: expansion-dev）
※ 本リポジトリでは未実装。方針整理用の参照のみ。

- `include/constants/event_objects.h` に `OBJ_EVENT_GFX_ITEM_*` を大量追加。
- `src/rogue_controller.c` で itemId / pocket に応じて `graphicsId` を上書き。
- 画像は `graphics/object_events/pics/rogue/item_*.png` を使用。
- アイコンサイズは **16x16 / 1フレーム**（本件は 16x32 統一）。

---
## Rogue素材→カテゴリ対応表（提案）
カテゴリ単位で **1アイコン**に統一する前提。サブカテゴリは将来拡張用。

| カテゴリ | Rogue素材（移植元） | 備考 |
| --- | --- | --- |
| HEAL | `item_healing.png` | 回復系全般の基本アイコン |
| BALL | `item_poke_ball.png` | 汎用ボール（Master/Dynaは将来） |
| BATTLE_USE | `item_medicine.png` | 戦闘時使用アイコンとして採用 |
| HELD | `item_held_item.png` | もちもの全般 |
| TOOL | `item_rare_candy.png` | TOOLは代表1種に固定 |
| TM | `item_gold_tm.png` | TM専用 |
| HM | `item_silver_tm.png` | HM専用 |
| MEGA | `item_mega_stone.png` | メガ系 |
| Z | `item_z_crystal.png` | Z系 |

将来用（未実装の候補）:
- `item_master_ball.png` : ボールサブカテゴリ用
- `item_dynamax_ball.png` : Dynamax系
- `item_tera_orb.png` / `item_tera_shard.png` : Tera系

---
## サンプル（擬似コード）

### カテゴリ→GFX 対応表
```
static const u16 sItemCategoryToGfx[] = {
    [RANDOMIZER_ITEMCAT_HEAL] = OBJ_EVENT_GFX_ITEM_HEAL,
    [RANDOMIZER_ITEMCAT_BALL] = OBJ_EVENT_GFX_ITEM_BALL,
    [RANDOMIZER_ITEMCAT_BATTLE_USE] = OBJ_EVENT_GFX_ITEM_BATTLE,
    [RANDOMIZER_ITEMCAT_HELD] = OBJ_EVENT_GFX_ITEM_HELD,
    [RANDOMIZER_ITEMCAT_TOOL] = OBJ_EVENT_GFX_ITEM_TOOL,
    [RANDOMIZER_ITEMCAT_TM] = OBJ_EVENT_GFX_ITEM_TM,
    [RANDOMIZER_ITEMCAT_HM] = OBJ_EVENT_GFX_ITEM_HM,
    [RANDOMIZER_ITEMCAT_MEGA] = OBJ_EVENT_GFX_ITEM_MEGA,
    [RANDOMIZER_ITEMCAT_Z] = OBJ_EVENT_GFX_ITEM_Z,
};
```

### マップロード後の一括差し替え（例）
```
void Randomizer_UpdateItemBallGfxForMap(void)
{
    if (!RandomizerFeatureEnabled(RANDOMIZE_FIELD_ITEMS))
        return;
    if (!IsNormalMap())
        return;

    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *templ = &gMapHeader.events->objectEvents[i];

        if (templ->graphicsId != OBJ_EVENT_GFX_ITEM_BALL)
            continue;
        if (FlagGet(templ->flagId))
            continue; // already collected

        itemId = templ->trainerRange_berryTreeId;
        if (itemId == ITEM_NONE)
            continue;

        if (!RandomizeFoundItemEx(itemId, mapNum, mapGroup, templ->localId, &randItem, NULL))
            continue;

        category = GetRandomizerItemCategoryForItem(randItem); // helper
        gfx = sItemCategoryToGfx[category];
        if (gfx == 0)
            gfx = OBJ_EVENT_GFX_ITEM_BALL;

        ObjectEventSetGraphicsIdByLocalIdAndMap(templ->localId, mapNum, mapGroup, gfx);
    }
}
```
補足:
- `FlagGet(templ->flagId)` の前に `templ->flagId != 0` をチェックする方が安全。
- `mapGroup/mapNum` は `gMapHeader.mapGroup/mapNum` を使用する想定。

### itemId → category の取得（案）
```
static u8 GetRandomizerItemCategoryForItem(u16 itemId)
{
    for (i = 0; i < RANDOMIZER_ITEM_TABLE_COUNT; i++)
        if (sRandomizerItemTable[i].itemId == itemId)
            return sRandomizerItemTable[i].category;
    return RANDOMIZER_ITEMCAT_COUNT; // invalid
}
```
※ 高速化が必要なら `ITEMS_COUNT` の固定配列を事前生成。

---
## 参照/関連ファイル
- `src/item_ball.c` : itemId/amount の取得方式
- `src/randomizer.c` / `include/randomizer.h` : `RandomizeFoundItemEx` / カテゴリ定義
- `data/scripts/item_ball_scripts.inc` : フィールドアイテム取得入口
- `include/constants/event_objects.h`
- `src/data/object_events/object_event_graphics.h`
- `src/data/object_events/object_event_pic_tables.h`
- `src/data/object_events/object_event_graphics_info.h`
- `src/data/object_events/object_event_graphics_info_pointers.h`

---
## 既知の懸念点（注意）
- `OBJ_EVENT_GFX_ITEM_BALL` 前提の処理が残っている場合、
  差し替え後に影響する可能性がある。
- `NUM_OBJ_EVENT_GFX` の増加による範囲干渉に注意。
- 16x16→16x32移植で見た目のズレが起きる可能性があるため要確認。

---
## 実装チェックリスト（作業用）
- [ ] Rogue素材のリストアップ（item_healing / item_medicine / item_poke_ball / item_gold_tm / item_silver_tm / item_held_item など）
- [ ] 16x32化ルールをテンプレ化（下寄せ透明上段）
- [ ] `OBJ_EVENT_GFX_ITEM_*` の命名とID確定
- [ ] graphics/pic_tables/info/pointers 登録
- [ ] `Randomizer_UpdateItemBallGfxForMap` 実装
- [ ] `overworld.c` の呼び出し位置に追加
- [ ] 通常マップでの表示確認（複数カテゴリ）

---
## 将来メモ（未実装・予知）
- カテゴリとは別に、特定アイテムだけ専用アイコンにする案を残す。
  - 例: レアキャンディ / ミント / 特殊進化石 など
  - 実装する場合は「カテゴリ判定より優先」の例外ルールが必要。
