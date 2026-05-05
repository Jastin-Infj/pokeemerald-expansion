# Move Data Manual

この manual は、既存技の変更と新規技追加の入口です。
詳細なデータ地図は [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) を参照します。

## 既存技の定義場所

技 ID は `include/constants/moves.h` にあります。
技の威力、命中、タイプなどの本体は `src/data/moves_info.h` にあります。

対象を探す例です。

```sh
rtk rg -n "MOVE_TACKLE" include/constants/moves.h src/data/moves_info.h
```

## 既存技を変える

既存技の entry にある値を編集します。

```c
[MOVE_TACKLE] =
{
    .effect = EFFECT_HIT,
    .power = 40,
    .type = TYPE_NORMAL,
    .accuracy = 100,
    .pp = 35,
    .category = DAMAGE_CATEGORY_PHYSICAL,
    .target = MOVE_TARGET_SELECTED,
    .priority = 0,
},
```

威力、命中、PP、タイプ、category の変更は比較的狭いです。
ただし、AI、習得技、TM、対戦バランスに影響します。

## 新規技を追加する

新規技は最低でも次を同期します。

| 作業 | ファイル |
| --- | --- |
| 技 ID を追加 | `include/constants/moves.h` |
| 技データを追加 | `src/data/moves_info.h` |
| animation を指定 | `data/battle_anim_scripts.s` |
| 必要なら技効果を追加 | `include/constants/battle_move_effects.h` |
| 必要なら battle script を追加 | `src/data/battle_move_effects.h`, `data/battle_scripts_*.s` |
| 習得先を追加 | level-up learnsets, teachable learnsets |

既存技と同じ効果を使うなら、新しい battle effect を増やさずに済むことがあります。
新しい挙動を作る場合は battle script、メッセージ、AI、テストまで広がります。

## 習得技へ接続する

技を作っただけでは、ポケモンはその技を覚えません。
追加先に応じて、次を確認します。

- レベル技: `src/data/pokemon/level_up_learnsets/gen_*.h`
- 教え技、TM learnset: `src/data/pokemon/teachable_learnsets.h`
- TM/HM: [TM/HM Manual](tm_hm_manual.md)

`teachable_learnsets.h` は generated data と関係するため、手動編集か generator 経由かを先に確認します。

## 既存 tutorial との関係

[How To Add A New Move](../tutorials/how_to_new_move.md) も参照できます。
ただし、このプロジェクトの現在の技データ本体は `src/data/moves_info.h` です。
古いファイル名や upstream 由来の記述と食い違う場合は、[Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) を優先して確認します。

## 確認すること

```sh
rtk rg -n "MOVE_NEW_MOVE" include src data
make -j4
```

見る観点は次です。

- 技 ID と `MOVES_COUNT` 周辺が破綻していないか。
- animation が存在するか。
- battle effect と battle script が対応しているか。
- 習得先が意図通りか。
- AI やダメージ計算に想定外の影響がないか。
