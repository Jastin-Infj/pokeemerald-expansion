# Pokemon Data Map v15

## Purpose

今後の独自拡張で影響が大きい Pokemon data、learnset、TM/HM、move relearner、species/move/item limit を整理する。

この文書は source 読解メモであり、現時点では実装変更はしない。

## Key Headers

| File | Important symbols | Notes |
|---|---|---|
| `include/pokemon.h` | `enum MonData`, `struct BoxPokemon`, `struct Pokemon`, `struct BattlePokemon`, `struct SpeciesInfo`, `struct LevelUpMove`, `struct Evolution`, `gPlayerParty`, `gEnemyParty` | Pokemon 個体 data、species data、party 配列、取得/設定 API の中心。 |
| `include/constants/species.h` | `SPECIES_*`, `SPECIES_EGG`, `NUM_SPECIES` | species ID 定義。現在確認した範囲では `SPECIES_EGG` は `SPECIES_GLIMMORA_MEGA + 1`、`NUM_SPECIES` は `SPECIES_EGG`。 |
| `include/constants/moves.h` | `enum Move`, `MOVES_COUNT_GEN*`, `MOVES_COUNT`, `MOVE_UNAVAILABLE` | move ID 定義。Gen 9 までの通常 move と Z/Max move 群を持つ。 |
| `include/constants/items.h` | `enum Item`, `ITEM_TM01` - `ITEM_TM100`, `ITEM_HM01` - `ITEM_HM08`, `ITEMS_COUNT` | item ID 定義。TM/HM item slot は 100 + 8 枠ある。 |
| `include/constants/tms_hms.h` | `FOREACH_TM`, `FOREACH_HM`, `FOREACH_TMHM` | 実際に使う TM/HM move list。現在 `FOREACH_TM` は 50 moves、`FOREACH_HM` は 8 moves。 |
| `include/item.h` | `enum TMHMIndex`, `GetItemTMHMIndex`, `GetItemTMHMMoveId`, `GetTMHMItemIdFromMoveId`, `GetTMHMItemId` | TM/HM item と move の対応を macro から生成する。 |
| `include/config/pokemon.h` | `P_LVL_UP_LEARNSETS`, `P_LEARNSET_HELPER_TEACHABLE`, `P_TM_LITERACY`, `P_CAN_FORGET_HIDDEN_MOVE` | Pokemon data と learnset generation の主要 config。 |
| `include/config/species_enabled.h` | `P_GEN_*_POKEMON`, `P_FAMILY_*`, `P_MEGA_EVOLUTIONS`, `P_REGIONAL_FORMS` | 有効 species family を制御。comment に saveblock / Dex flags 影響ありと明記されている。 |
| `include/config/summary_screen.h` | `P_ENABLE_MOVE_RELEARNERS`, `P_TM_MOVES_RELEARNER`, `P_ENABLE_ALL_TM_MOVES`, `P_FLAG_EGG_MOVES`, `P_FLAG_TUTOR_MOVES` | summary screen と move relearner UI/機能の config。 |
| `include/constants/move_relearner.h` | `MAX_RELEARNER_MOVES`, `enum MoveRelearnerStates`, `enum RelearnMode` | relearner の表示上限と mode 定義。 |

## Core Data Files

| File | Role |
|---|---|
| `src/pokemon.c` | `gPlayerParty` / `gEnemyParty` 定義、Pokemon 生成、learnset 取得、move 習得、evolution、heal、save party helper。 |
| `src/data/pokemon/species_info.h` | `gSpeciesInfo[]` 本体。世代別 family file を include する。 |
| `src/data/pokemon/species_info/gen_1_families.h` - `gen_9_families.h` | species ごとの base stats、types、abilities、graphics、learnset pointer、evolution、form data。 |
| `src/data/pokemon/level_up_learnsets/gen_1.h` - `gen_9.h` | `static const struct LevelUpMove s<Name>LevelUpLearnset[]` の世代別定義。 |
| `src/data/pokemon/teachable_learnsets.h` | `s<Name>TeachableLearnset[]`。`tools/learnset_helpers/make_teachables.py` による自動生成対象。 |
| `src/data/pokemon/all_learnables.json` | species ごとの learnable move 集約 JSON。`tools/learnset_helpers/make_learnables.py` により `porymoves_files/*.json` から生成される。 |
| `src/data/pokemon/special_movesets.json` | `universalMoves`、`signatureTeachables`、`extraTutors`。teachable 生成時に参照される。 |
| `src/data/pokemon/egg_moves.h` | egg move list。 |
| `src/data/pokemon/form_species_tables.h` | form species table。 |
| `src/data/pokemon/form_change_tables.h` | form change 条件 table。 |
| `src/data/pokemon/form_change_table_pointers.h` | form change pointer table。 |
| `src/data/pokemon/experience_tables.h` | growth rate ごとの exp table。 |
| `src/data/pokemon/item_effects.h` | item effect data。 |
| `src/data/moves_info.h` | move power/effect/type/flags などの move data。`src/move.c` から include。 |
| `src/data/items.h` | item data。`src/item.c` から include。 |
| `src/data/trainer_parties.h` | trainer party data。`src/data.c` から include。 |
| `src/data/wild_encounters.json` | wild encounter source。`Makefile` rule で `src/data/wild_encounters.h` に変換される。 |

## Pokemon Storage Layout Limits

`include/pokemon.h` の bitfield は、species/move/item を増やすときの上限になる。

| Field | Definition | Practical limit / risk |
|---|---|---|
| Species | `struct PokemonSubstruct0::species:11` | 11 bits。最大 2047 species ID まで。現在 `SPECIES_GLIMMORA_MEGA` が 1572、`SPECIES_EGG` が 1573 なので余裕はあるが無限ではない。 |
| Move | `struct PokemonSubstruct1::move1:11` through `move4:11` | 11 bits。最大 2047 move ID まで。Gen 9 + Z/Max move でも現状は範囲内。 |
| Held item | `struct PokemonSubstruct0::heldItem:10` | 10 bits。最大 1023 item ID まで。item を 1024 以上に増やす場合、持ち物化できる item と save layout に注意。 |
| Pokeball | `struct PokemonSubstruct0::pokeball:6` | 6 bits。最大 63 balls。 |
| PP | `struct PokemonSubstruct1::pp1:7` through `pp4:7` | 7 bits。最大 127 PP。 |
| Form-change day counter | `struct BoxPokemon::daysSinceFormChange:3` | 3 bits。最大 7 days。 |
| Dynamax level | `struct PokemonSubstruct3::dynamaxLevel:4` | 4 bits。 |
| Tera type | `struct PokemonSubstruct0::teraType:5` | 5 bits。comment 上は 30 types。 |

Save compatibility 上の注意:

- `struct BoxPokemon` / substruct bitfield のサイズや意味を変える変更は save data 互換性が高リスク。
- `include/config/species_enabled.h` の comment では、latest generation の変更は Dex flags / saveblock に影響し、新規 save が必要と明記されている。
- トレーナーバトル前選出のように一時的に `gPlayerParty` を組み替える場合、`struct Pokemon` のコピーだけでなく `gPlayerPartyCount` と `gSaveBlock1Ptr->playerParty` への反映境界を分けて考える必要がある。

## Important Runtime Symbols

| Symbol | Defined / declared | Main use |
|---|---|---|
| `gPlayerParty` | `src/pokemon.c`, `include/pokemon.h` | 現在の player party。battle、party menu、storage、field move、summary が直接読む。 |
| `gPlayerPartyCount` | `src/pokemon.c`, `include/pokemon.h` | current party count。`CalculatePlayerPartyCount()` で更新される箇所がある。 |
| `gEnemyParty` | `src/pokemon.c`, `include/pokemon.h` | enemy party。trainer/wild/facility battle setup が構築する。 |
| `gSpeciesInfo[]` | `src/data/pokemon/species_info.h`, `include/pokemon.h` | species master data。base stats、types、abilities、graphics、learnsets、evolutions。 |
| `gTMHMItemMoveIds[]` | `src/item.c`, `include/item.h` | TM/HM index -> item/move mapping。`FOREACH_TMHM` から生成。 |
| `gTutorMoves[]` | `src/data/tutor_moves.h` generated | relearner の tutor move list。`make_teachables.py --tutors` が生成。 |

## Key Functions

| Function | File | Why it matters |
|---|---|---|
| `GetSpeciesLevelUpLearnset(u16 species)` | `src/pokemon.c` | `gSpeciesInfo[species].levelUpLearnset` を返す。NULL 時は `SPECIES_NONE` の fallback。 |
| `GetSpeciesTeachableLearnset(u16 species)` | `src/pokemon.c` | TM/HM/tutor/relearner compatibility 判定の中心。 |
| `GetSpeciesEggMoves(u16 species)` | `src/pokemon.c` | egg move / relearner で参照。 |
| `CanLearnTeachableMove(u16 species, enum Move move)` | `src/pokemon.c` | `teachableLearnset` を `MOVE_UNAVAILABLE` まで走査する。 |
| `GiveBoxMonInitialMoveset(struct BoxPokemon *boxMon)` | `src/pokemon.c` | level-up learnset から初期技 4 個を構築する。 |
| `MonTryLearningNewMoveAtLevel(struct Pokemon *mon, bool32 firstMove, u32 level)` | `src/pokemon.c` | level-up 時の新技習得。 |
| `MonTryLearningNewMoveEvolution(struct Pokemon *mon, bool8 firstMove)` | `src/pokemon.c` | evolution 後の level 0 / current level move 習得。`P_EVOLUTION_LEVEL_1_LEARN` が関係する。 |
| `IsMoveHM(enum Move move)` | `src/pokemon.c` | `FOREACH_HM` から HM 判定を生成。 |
| `CannotForgetMove(enum Move move)` | `src/pokemon.c` | `P_CAN_FORGET_HIDDEN_MOVE` と HM 判定で忘却可否を決める。 |
| `HealPokemon(struct Pokemon *mon)` | `src/pokemon.c` | HP/status/PP 回復。trainer battle aftercare で候補になる。 |
| `GetSavedPlayerPartyMon(u32 index)` / `SavePlayerPartyMon(u32 index, struct Pokemon *mon)` | `src/pokemon.c` | `gSaveBlock1Ptr->playerParty` との境界。battle facility でも使用あり。 |

## Future Feature Impact

| Planned area | Related files | Risk |
|---|---|---|
| New species / forms | `include/constants/species.h`, `include/config/species_enabled.h`, `src/data/pokemon/species_info*.h`, graphics/icon/cry/form tables | Very High |
| New moves | `include/constants/moves.h`, `src/data/moves_info.h`, battle scripts/effects/AI, learnset JSON/header | Very High |
| New TMs / TM shop | `include/constants/tms_hms.h`, `include/constants/items.h`, `src/data/items.h`, `src/item.c`, `tools/learnset_helpers/*`, map scripts | High |
| New items | `include/constants/items.h`, `src/data/items.h`, `src/item.c`, bag pocket/save capacity, item effects | High |
| New abilities | `include/constants/abilities.h`, `src/data/abilities.h`, battle ability handlers, species info | High |
| Randomizer / trainer party reorder | `src/data/trainer_parties.h`, `src/data/trainers.h`, `src/trainer_pools.c`, `src/battle_setup.c`, `src/pokemon.c` creation helpers | High |
| Wild randomizer | `src/data/wild_encounters.json`, `tools/wild_encounters/wild_encounters_to_header.py`, `src/wild_encounter.c`, DexNav config/data | High |
| Field HM modernization | `include/constants/tms_hms.h`, `src/field_move.c`, `src/party_menu.c`, `src/pokemon.c`, `data/scripts/field_move_scripts.inc` | High |
| Move relearner UI expansion | `include/constants/move_relearner.h`, `include/config/summary_screen.h`, `src/move_relearner.c`, `src/pokemon_summary_screen.c` | Medium/High |

## Open Questions

- `ITEMS_COUNT` は現状 `ITEM_GLIMMORANITE = 873` の次で 874。250 TM 化で `ITEM_TM101` 以降を 150 個そのまま足すと `ITEMS_COUNT == 1024` になり、`src/pokemon.c` の `ITEMS_COUNT < (1 << 10)` static assert に当たる。詳細は [tm_hm_expansion_250_v15.md](tm_hm_expansion_250_v15.md)。
- `MAX_RELEARNER_MOVES` は 60。TM 追加・全 TM relearner 有効化・Mew のような広い compatibility で上限超過しないか要検証。
- `src/data/pokemon/all_learnables.json` は生成済みファイルとして存在するが、独自 move/species 追加時に porymoves input を更新する運用にするか、JSON を直接管理するか要方針決め。
- `P_LVL_UP_LEARNSETS` は `GEN_LATEST`。v15.2 以降の upstream 追従で level-up learnset 差分が入りやすいので、custom edits をどこに隔離するか要設計。
- `config/` directory はこの checkout では見つからず、確認できた config は `include/config/*.h`。別 branch / upstream version で layout が変わる可能性は未確認。
