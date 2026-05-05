# How to add a new item

この tutorial は通常 item / held item / battle item を追加する時の入口です。詳細な挙動調査は [Move / Item / Ability Map v15](../overview/move_item_ability_map_v15.md) も参照します。

調査日: 2026-05-04。source 改造はしておらず、`docs/` への記録のみ。

## Decide the Item Type First

最初に「どの種類の item か」を決める。

| Type | Typical fields | Extra work |
|---|---|---|
| Sellable / treasure | `price`, `pocket`, `sortType`, `description`, icon | shop / pickup / hidden item に置くか確認。 |
| Medicine | `effect`, `type = ITEM_USE_PARTY_MENU`, `fieldUseFunc` | `src/data/pokemon/item_effects.h` と use validation。 |
| Held item | `holdEffect`, `holdEffectParam`, `notConsumed` | battle hooks、AI、Fling、item restore policy。 |
| Battle item | `battleUsage`, `effect`, `type` | `enum EffectItem`、battle scripts、CannotUse validation。 |
| Key item | `importance`, `notConsumed`, `fieldUseFunc` | registered item、field callback、script state。 |
| TM/HM | `FOREACH_TMHM`, move link, teachable learnsets | TM/HM flow を別途確認。 |

## Core Files

| File | Role |
|---|---|
| [include/constants/items.h](../../include/constants/items.h) | `enum Item`、`ITEMS_COUNT`、`enum ItemType`、`enum EffectItem`。 |
| [include/item.h](../../include/item.h) | `struct ItemInfo` と item getter。 |
| [src/data/items.h](../../src/data/items.h) | `gItemsInfo[]` の本体。name、price、pocket、use callback、held effect、icon。 |
| [src/item.c](../../src/item.c) | bag pocket pointers、item getters、quantity operations。 |
| [src/item_use.c](../../src/item_use.c) | field / bag / battle use callbacks。 |
| [src/data/pokemon/item_effects.h](../../src/data/pokemon/item_effects.h) | medicine / X item / PP item effect bytes。 |
| [include/constants/hold_effects.h](../../include/constants/hold_effects.h) | held item effect ID。 |
| [src/data/hold_effects.h](../../src/data/hold_effects.h) | held item effect metadata。 |
| [data/battle_scripts_2.s](../../data/battle_scripts_2.s) | battle item script table `gBattlescriptsForUsingItem`。 |
| item icon data | `graphics/items/`, `src/graphics.c`, generated asset rules | アイコン追加時。 |

## Minimal Data Item

通常 item の最小追加は:

1. [include/constants/items.h](../../include/constants/items.h) の `enum Item` に `ITEM_*` を追加する。
2. [src/data/items.h](../../src/data/items.h) の `gItemsInfo[]` に同じ index の entry を追加する。
3. `name`, `price`, `description`, `pocket`, `sortType`, `type`, `fieldUseFunc`, `iconPic`, `iconPalette` を既存例に合わせる。
4. 入手経路を追加する。例: `giveitem`, item ball, hidden item, mart table。
5. build して `ITEMS_COUNT` と table が崩れていないことを確認する。

`ITEMS_COUNT` は enum の末尾。新 ID を途中に挿入すると既存 save / bag item ID 互換に影響するため、独自 fork では追加位置を慎重に決める。

## ItemInfo Fields

`struct ItemInfo` の主な field:

| Field | Notes |
|---|---|
| `price` | shop / sell value。sell fraction config の影響を受ける。 |
| `secondaryId` | ball type、rod/bike subtype など item 固有 ID。 |
| `fieldUseFunc` | field / bag から使った時の callback。 |
| `description` | bag description。 |
| `effect` | medicine / X item / PP item などの effect byte pointer。 |
| `name`, `pluralName` | 表示名。長さ制限あり。 |
| `holdEffect`, `holdEffectParam` | held item behavior。 |
| `importance`, `notConsumed` | key item / reusable item / consumed item policy。 |
| `pocket`, `sortType`, `type` | bag pocket、sort、menu exit callback。 |
| `battleUsage` | battle item script ID。non-zero が必要。 |
| `flingPower` | Fling 等。 |
| `iconPic`, `iconPalette` | item icon。 |

## Held Item

held item を追加する場合:

1. 既存 `HOLD_EFFECT_*` を使えるか確認する。
2. 新 effect が必要なら [include/constants/hold_effects.h](../../include/constants/hold_effects.h) と [src/data/hold_effects.h](../../src/data/hold_effects.h) を確認する。
3. `ItemBattleEffects` や end-turn / switch-in / damage calc hooks で既存 effect が拾われるか確認する。
4. AI が同じ item を評価できるか確認する。
5. single-use item なら `usedHeldItem`、Recycle、Pickup、Harvest、Cud Chew、G-Max Replenish、battle-end restore policy への影響を見る。

対戦用に「消費しても battle 後に戻す」仕様を採用する場合、消費処理自体を止めるのではなく、battle 中は消費済みとして扱い、battle 後に original item を復元する方が安全。詳細は [Battle Item Restore Policy](../features/battle_item_restore_policy/README.md)。

## Battle Item

battle 中に使う item を追加する場合:

1. 既存 `enum EffectItem` で足りるか確認する。
2. 足りなければ [include/constants/items.h](../../include/constants/items.h) に `EFFECT_ITEM_*` を追加する。
3. [data/battle_scripts_2.s](../../data/battle_scripts_2.s) の `gBattlescriptsForUsingItem` に script を追加する。
4. [src/data/items.h](../../src/data/items.h) の item entry に `.battleUsage = EFFECT_ITEM_*` を設定する。
5. `src/item_use.c` の item use validation で使える条件を確認する。
6. trainer battle、frontier battle、wild battle で使用可否を確認する。

Poké Ball、escape item、Poke Flute、X item、medicine は既存 path がそれぞれ違う。似た item から始める。

## Item Distribution

入手経路ごとの入口:

| Distribution | Where |
|---|---|
| NPC gift | `giveitem`, `giveitem_msg`, `msgreceiveditem` in `scripts.inc` |
| visible item ball | `object_event ... OBJ_EVENT_GFX_ITEM_BALL ... Common_EventScript_FindItem ... FLAG_ITEM_*` |
| hidden item | `bg_hidden_item_event x, y, elevation, ITEM_*, FLAG_HIDDEN_ITEM_*, quantity, underfoot` |
| mart | map-specific `.2byte ITEM_*` lists used by `pokemart` scripts |
| Battle Pyramid | Pyramid bag item tables and `src/battle_pyramid.c` |
| held by trainer mon | trainer party data |
| wild held item | species data / wild held item fields |

Visible and hidden item flags are save-backed. Reusing a flag makes another pickup disappear; adding a new flag can affect save compatibility.

## Verification Checklist

- `ITEM_*` enum and `gItemsInfo[]` entry exist.
- Item name fits `ITEM_NAME_LENGTH`; description fits bag UI.
- Bag pocket and sort type are correct.
- Field use callback matches `type`.
- Battle usage is zero for non-battle items, non-zero for battle items.
- Held item effect is covered by battle runtime and AI if needed.
- Icon/palette renders in bag.
- Gift, item ball, hidden item, mart, and battle use paths all work where applicable.
- Save compatibility is explicitly accepted if new persistent item IDs or flags are added.
