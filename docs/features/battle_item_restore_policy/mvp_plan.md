# Battle Item Restore Policy MVP Plan

## Scope

MVP は「battle 中の消費処理は変えず、battle 終了後に selected policy に従って held item を復元する」ことに限定する。

## Proposed Config Shape

候補:

```c
#define B_RESTORE_CONSUMED_BERRIES_AFTER_BATTLE FALSE
```

または、将来的に mode を増やすなら:

```c
#define RESTORE_BATTLE_ITEMS_NON_BERRIES 0
#define RESTORE_BATTLE_ITEMS_ALL_ORIGINAL 1
#define RESTORE_BATTLE_ITEMS_COMPETITIVE 2
#define B_RESTORE_HELD_BATTLE_ITEMS_MODE RESTORE_BATTLE_ITEMS_NON_BERRIES
```

既存 `B_RESTORE_HELD_BATTLE_ITEMS` が generational config なので、互換性を重視するなら新規 config を足す方が読みやすい。

## Proposed Runtime Policy

1. Battle start で保存済みの `itemLost[side][slot].originalItem` を source of truth にする。
2. Battle 中の item 消費、`usedHeldItem`、`canPickupItem` は変更しない。
3. Battle end の `TryRestoreHeldItems` 相当で、policy に応じて Berry pocket も復元対象に含める。
4. Air Balloon / Corrosive Gas のように「復元不可」と明記されている処理は別扱いを検討する。
5. Link / Frontier / wild / trainer で挙動を分ける必要があるか test で決める。

## Do Not Do

- held item 消費コマンドで berry を消さないようにする。
- `usedHeldItem` を消費直後に消す。
- `Recycle`、`Pickup`、`Harvest`、`Cud Chew` の成功条件を先に変える。
- item clause と battle-end restore を同じ patch に混ぜる。

## Implementation Candidate

将来実装する場合の候補は `src/battle_util.c` の `TryRestoreHeldItems` 周辺。ここだけで済む可能性はあるが、`usedHeldItem` を使う battle 中 mechanics の regression test が必須。

実装時は `TryRestoreHeldItems` 内へ条件式を直接増やしすぎない。
候補:

```c
static bool32 ShouldRestoreHeldBattleItem(u16 originalItem, u16 currentItem, u32 partySlot);
static bool32 ShouldApplyCompetitiveHeldItemRestore(void);
```

初期 contract:

1. `ShouldApplyCompetitiveHeldItemRestore()` が false の場合は既存挙動。
2. true の場合も battle 中の消費、`usedHeldItem`、`canPickupItem` は変更しない。
3. Link / recorded battle / Frontier / Trainer Hill / Battle Pyramid は MVP で除外するか、既存 generational config のみを使う。
4. `ChampionsChallenge_IsActive()` が将来 true なら challenge item policy へ渡し、normal trainer restore と二重適用しない。
5. Berry pocket を復元対象に含めるかは新規 config / mode で明示する。

## Related but Separate

Town Map から R で Fly した後の cleanup は、`src/field_region_map.c` の R button path と通常 A/B close path の後始末差分を設計対象にする。ただし、この feature の MVP では扱わない。
