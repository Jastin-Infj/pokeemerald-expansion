# Pokemon Stats Manual

この manual は、既存ポケモンの種族値、タイプ、特性などを変えるための入口です。
新規ポケモン追加は影響範囲が広いため、ここだけで進めず [How To Add A New Pokemon](../tutorials/how_to_new_pokemon.md) と [Pokemon Data Map](../overview/pokemon_data_map_v15.md) を確認します。

## 既存ポケモンの定義場所

主な定義は世代別 family file にあります。

```text
src/data/pokemon/species_info/gen_1_families.h
src/data/pokemon/species_info/gen_2_families.h
src/data/pokemon/species_info/gen_3_families.h
...
src/data/pokemon/species_info/gen_9_families.h
```

対象を探すときは、species constant で検索します。

```sh
rtk rg -n "SPECIES_TREECKO" src/data/pokemon/species_info
```

## 種族値を変える

対象 species の `gSpeciesInfo` entry にある次の値を編集します。

```c
.baseHP        = 40,
.baseAttack    = 45,
.baseDefense   = 35,
.baseSpeed     = 70,
.baseSpAttack  = 65,
.baseSpDefense = 55,
```

既存 species の値だけを変更する場合、species ID や save layout は増えません。
ただし、戦闘バランス、AI 評価、野生ポケモン、トレーナー戦には影響します。

## タイプを変える

同じ entry の `types` を編集します。

```c
.types = MON_TYPES(TYPE_GRASS),
.types = MON_TYPES(TYPE_GRASS, TYPE_DRAGON),
```

タイプを変えると、弱点、耐性、STAB、AI、ジム戦の難易度に影響します。

## 特性を変える

同じ entry の abilities を編集します。

```c
.abilities = { ABILITY_OVERGROW, ABILITY_NONE, ABILITY_UNBURDEN },
```

特性は battle script や AI と強く結びつくため、新しい特性挙動を同時に作る場合は別タスクとして扱います。

## EV yield を変える

努力値 yield は同じ species entry にあります。

```c
.evYield_SpAttack = 1,
```

EV/IV UI を作る場合、この値は「倒したときにもらえる努力値」であり、プレイヤーが割り振る UI の保存先ではありません。
UI 実装時は [Champions Training UI Feasibility](../overview/champions_training_ui_feasibility_v15.md) を確認します。

## 確認すること

```sh
rtk rg -n "SPECIES_TREECKO" src data include
make -j4
```

見る観点は次です。

- 該当 species を使う wild encounter や trainer party がないか。
- 変更後のステータスが想定通りか。
- タイプ、特性、EV yield が summary や戦闘に反映されるか。
- 難易度に関わる変更なら、該当ジムやボス戦で確認したか。
