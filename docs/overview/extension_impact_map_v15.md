# Extension Impact Map v15

調査日: 2026-05-01

この文書は、今後の独自拡張で「最初にどの範囲を読むべきか」を整理するための横断マップです。現時点では実装・改造は行っていません。確認したファイル名、関数名、構造体名、グローバル変数名、マクロ名だけを記載し、未確認の設計判断は Open Questions に残します。

## Purpose

- トレーナーバトル前選出だけでなく、マート設定、野生ポケモン差し替え、TM/HM、アイテム、特性、技、フィールド秘伝技廃止などを将来の拡張対象として扱う。
- 実装前にソース全体の影響範囲を確認するため、データ所有者、runtime entry point、UI、script、build tool、test の観点を固定する。
- `SetMainCallback2`、`CreateTask`、`ScrCmd_*`、`special`、field callback のような間接呼び出しを見落とさないための入口を示す。

## Source-Wide Investigation Rule

新しい機能を実装する前は、対象が小さく見えても以下を確認する。

1. constants / data owner を確認する。
2. `rg` で対象 symbol の全参照を確認する。
3. `SetMainCallback2`、`CreateTask`、`ScrCmd_*`、`special`、`gFieldCallback` などの dispatch / callback 経路を確認する。
4. `gPlayerParty`、`gEnemyParty`、`gSaveBlock1Ptr`、`gSaveBlock2Ptr` など runtime state / save state への影響を確認する。
5. UI / window / text / sprite / icon の表示経路を確認する。
6. JSON / generated header / tools など build-time 生成経路を確認する。
7. `test/`、`include/config/`、upstream 更新 checklist への影響を確認する。

読み取り用の代表コマンド:

```bash
rg --files src include data asm tools test
rg -n "SYMBOL_OR_FUNCTION" src include data asm tools test
rg -l "SetMainCallback2\\(|CB2_|CreateTask\\(|ScrCmd_|ScriptContext_Stop|trainerbattle|special" src include data asm
```

## High-Risk Expansion Areas

| Area | Primary data owner | Runtime entry points | UI / script / tools | Risk |
|---|---|---|---|---|
| Pokemart / shop settings | `data/maps/*/scripts.inc`, `src/shop.c` | `ScrCmd_pokemart`, `CreatePokemartMenu`, `Task_BuyMenu` | `asm/macros/event.inc`, `data/script_cmd_table.inc`, `src/shop.c` | Medium |
| Wild Pokemon / randomizer | `src/data/wild_encounters.json`, generated `src/data/wild_encounters.h` | `StandardWildEncounter`, `TryGenerateWildMon`, `CreateWildMon` | `tools/wild_encounters/wild_encounters_to_header.py`, DexNav, Pokedex area | High |
| Trainer party pools / trainer randomizer | `src/data/trainers.h`, `src/data/trainers.party`, `src/data/battle_pool_rules.h` | `CreateNPCTrainerPartyFromTrainer`, `DoTrainerPartyPool`, `RandomizePoolIndices` | `tools/trainerproc/main.c`, `include/constants/battle_ai.h` | High |
| TM/HM / move machines | `include/constants/tms_hms.h`, `include/constants/items.h`, `src/data/items.h` | `ItemUseOutOfBattle_TMHM`, `ItemUseCB_TMHM`, `GetItemTMHMMoveId` | `src/item_menu.c`, `src/party_menu.c`, `test/text.c` | High |
| Field HM removal / field moves | `src/field_move.c`, `include/constants/field_move.h` | `ScrCmd_checkfieldmove`, `CursorCb_FieldMove`, field effect callbacks | `data/scripts/field_move_scripts.inc`, `src/field_control_avatar.c` | Very High |
| Items | `include/constants/items.h`, `src/data/items.h`, `include/item.h` | `AddBagItem`, `RemoveBagItem`, `GetItemFieldFunc`, item use callbacks | `src/item_menu.c`, `src/item_use.c`, shop, text tests | High |
| Moves | `include/constants/moves.h`, `src/data/moves_info.h`, `src/data/battle_move_effects.h` | `GetMoveEffect`, `GetMoveBattleScript`, battle script commands | `data/battle_scripts_1.s`, `data/battle_scripts_2.s`, move tests | Very High |
| Abilities | `include/constants/abilities.h`, `src/data/abilities.h` | `GetMonAbility`, `GetSpeciesAbility`, `AbilityBattleEffects` | battle AI, ability popup, text tests | Very High |
| Pokemon species / learnsets / forms | `include/constants/species.h`, `src/data/pokemon/` | `GetSpeciesInfo`, learnset/evolution/form helpers | summary, Dex, graphics, cry, icon, tests | Very High |
| Options / summary / status | `src/option_menu.c`, `src/pokemon_summary_screen.c`, `include/config/summary_screen.h` | `CB2_InitOptionMenu`, `ShowPokemonSummaryScreen` | save options, window templates, status UI | Medium |
| Battle UI / battle controller | `src/battle_interface.c`, `src/battle_bg.c`, `src/battle_controller_player.c` | `CB2_InitBattle`, `BattleMainCB1`, `BattleMainCB2`, controller commands | healthbox, party status summary, battle intro | Very High |
| Save data / config | `include/global.h`, `src/load_save.c`, `include/config/*.h` | `SavePlayerParty`, `LoadPlayerParty`, save option initializers | migration, save compatibility | Very High |
| Build tools / generated data | `tools/`, generated headers under `src/data/` | tool-dependent | trainerproc, wild encounter generation, text/gfx tools | High |

## Pokemart / Shop Settings

確認した入口:

| File | Symbols / facts |
|---|---|
| `asm/macros/event.inc` | `pokemart products:req`, `pokemartlistend` macro。 |
| `data/script_cmd_table.inc` | `SCR_OP_POKEMART` が `ScrCmd_pokemart` に対応。 |
| `src/scrcmd.c` | `ScrCmd_pokemart` が script pointer を読み、`CreatePokemartMenu(ptr)` を呼んで `ScriptContext_Stop()` する。 |
| `include/shop.h` | `CreatePokemartMenu(const u16 *itemsForSale)`、`CB2_ExitSellMenu`、`gMartPurchaseHistory`。 |
| `src/shop.c` | `SetShopItemsForSale`, `CreatePokemartMenu`, `CB2_InitBuyMenu`, `CB2_BuyMenu`, `Task_BuyMenu`, `BuyMenuTryMakePurchase`, `BuyMenuSubtractMoney`, `ExitBuyMenu`, `Task_ExitBuyMenu`。 |
| `data/maps/*Mart*/scripts.inc` | 通常のマート script。 |
| `data/maps/*DepartmentStore*/scripts.inc` | デパート系のマート script。 |

拡張時の注意:

- マートの品揃えを script 固定リストで増やすだけなら `data/maps/.../scripts.inc` 側が中心。
- ランダム品揃え、条件付き品揃え、独自カテゴリを入れる場合は `ScrCmd_pokemart` と `CreatePokemartMenu` の間、または `SetShopItemsForSale` の前後が候補。ただし script pointer と `ScriptContext_Stop()` の復帰処理を壊さないこと。
- Shop UI は `src/shop.c` 内の CB2 / Task で動くため、画面遷移を変える場合は `CB2_InitBuyMenu`、`CB2_BuyMenu`、`Task_BuyMenu`、`Task_ExitBuyMenu` を読む。

## Wild Pokemon / Randomizer

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/wild_encounter.h` | `struct WildPokemon`, `struct WildPokemonInfo`, `struct WildEncounterTypes`, `struct WildPokemonHeader`, `gWildMonHeaders`, `StandardWildEncounter`, `SweetScentWildEncounter`, `FishingWildEncounter`, `CreateWildMon`, `ChooseWildMonIndex_Land`, `ChooseWildMonIndex_Water`, `ChooseWildMonIndex_Rocks`。 |
| `src/wild_encounter.c` | `StandardWildEncounter`, `TryGenerateWildMon`, `CreateWildMon`, `ChooseWildMonIndex_*`, `RockSmashWildEncounter`, `SweetScentWildEncounter`, `FishingWildEncounter`, `GetLocalWildMon`, `GetLocalWaterMon`。 |
| `src/data/wild_encounters.json` | source data。root に `wild_encounter_groups`、label `gWildMonHeaders`。 |
| `tools/wild_encounters/wild_encounters_to_header.py` | `src/data/wild_encounters.h` を生成する tool。生成 header に `DO NOT MODIFY` comment がある。 |
| `src/data/wild_encounters.h` | generated `gWildMonHeaders[]`。直接編集対象ではなく生成物。 |
| `src/field_control_avatar.c` | `CheckStandardWildEncounter` から `StandardWildEncounter` へ進む。 |
| `src/dexnav.c` | `gWildMonHeaders` と `CreateWildMon` を参照。 |
| `src/pokedex_area_screen.c` | area display が `gWildMonHeaders` を参照。 |
| `src/match_call.c` | match call 周辺で `gWildMonHeaders` を参照。 |

拡張時の注意:

- build-time randomizer なら `src/data/wild_encounters.json` と `tools/wild_encounters/wild_encounters_to_header.py` の扱いが中心。
- runtime randomizer なら `TryGenerateWildMon`、`CreateWildMon`、`ChooseWildMonIndex_*`、DexNav 経由の生成を分けて検討する必要がある。
- `StandardWildEncounter` だけを変えると、DexNav、Fishing、Sweet Scent、Rock Smash、Pokedex area 表示とずれる可能性がある。
- seed / difficulty / option で変える場合は save data と config の設計が必要。

## Trainer Party Pools / Trainer Randomizer

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/trainer_pools.h` | `struct PoolRules`, `enum PoolRulesets`, `enum PoolPickFunctions`, `enum PoolPruneOptions`, `enum PoolTags`, `DoTrainerPartyPool`。 |
| `src/trainer_pools.c` | `DoTrainerPartyPool`, `PickMonFromPool`, `RandomizePoolIndices`, `PrunePool`。 |
| `src/data/battle_pool_rules.h` | pool ruleset definitions。 |
| `tools/trainerproc/main.c` | trainer party pool DSL / generated trainer data 周辺。 |
| `include/constants/battle_ai.h` | `AI_FLAG_RANDOMIZE_PARTY_INDICES`。 |
| `src/battle_main.c` | `CreateNPCTrainerPartyFromTrainer`, `CreateNPCTrainerParty`。 |

拡張時の注意:

- 「相手パーティを並び替える」だけでも、trainer party pool、difficulty、AI flag、相手 party preview の表示タイミングに影響する。
- battle 前に相手 party を表示したい場合、`gEnemyParty` 生成前後の timing を確認する。未生成の trainer data を直接読むと、pool / randomize 反映後と一致しない可能性がある。

## TM/HM / Move Machines

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/constants/tms_hms.h` | `FOREACH_TM(F)`, `FOREACH_HM(F)`, `FOREACH_TMHM(F)`。現在確認範囲では `FOREACH_HM` に `CUT`, `FLY`, `SURF`, `STRENGTH`, `FLASH`, `ROCK_SMASH`, `WATERFALL`, `DIVE`。 |
| `include/item.h` | `enum TMHMIndex`, `NUM_TECHNICAL_MACHINES`, `NUM_HIDDEN_MACHINES`, `GetItemTMHMIndex`, `GetItemTMHMMoveId`, `GetTMHMItemIdFromMoveId`, `GetTMHMItemId`, `GetTMHMMoveId`。 |
| `include/constants/items.h` | `ITEM_TM01` 系、`ITEM_HM01` 系、TM/HM alias enum。 |
| `src/item.c` | `gTMHMItemMoveIds`, `SetBagItemsPointers`。TM/HM pocket は `gSaveBlock1Ptr->bag.TMsHMs`。 |
| `src/data/items.h` | TM/HM item definitions。`fieldUseFunc = ItemUseOutOfBattle_TMHM`。 |
| `src/item_use.c` | `ItemUseOutOfBattle_TMHM`, `UseTMHM`。 |
| `src/party_menu.c` | `ItemUseCB_TMHM`, `CanTeachMove`, `GiveMoveToMon`, `Task_LearnedMove`。 |
| `src/item_menu.c` | `PrepareTMHMMoveWindow`, `PrintTMHMMoveData`。 |
| `test/text.c` | item text / name fit の test。 |

拡張時の注意:

- TM の追加は `FOREACH_TM`、item constants、item data、bag capacity、item menu 表示、text tests に波及する。
- `include/constants/global.h` の `BAG_TMHM_COUNT` が TM/HM pocket capacity に関係する。
- 既存には `ITEM_TM100` まで item 定数があるが、`FOREACH_TM` の実際の対応 move 数とは別に確認する必要がある。

## Field HM Removal / Field Moves

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/constants/field_move.h` | `enum FieldMove`, `FIELD_MOVES_COUNT`。 |
| `include/field_move.h` | `struct FieldMoveInfo`, `gFieldMoveInfo`, `FieldMove_GetMoveId`, `FieldMove_GetPartyMessage`。 |
| `src/field_move.c` | `gFieldMoveInfo[FIELD_MOVES_COUNT]`、`SetUpFieldMove_*`、`IsFieldMoveUnlocked_*`。 |
| `src/scrcmd.c` | `ScrCmd_checkfieldmove`。`IsFieldMoveUnlocked` と `MonKnowsMove` を確認し、`gSpecialVar_Result` を設定する。 |
| `data/scripts/field_move_scripts.inc` | `EventScript_CutTree`, `EventScript_RockSmash`, `EventScript_StrengthBoulder`, `EventScript_UseWaterfall`, `EventScript_UseDive`, `EventScript_UseDiveUnderwater`, `EventScript_UseDefog`, `EventScript_UseRockClimb`。 |
| `src/field_control_avatar.c` | water interaction / dive / waterfall などの field script 起動。 |
| `src/party_menu.c` | `SetPartyMonFieldSelectionActions`, `CursorCb_FieldMove`, `SetUpFieldMove`, `DoesSelectedMonKnowHM`, `IsLastMonThatKnowsSurf`。 |
| `src/pokemon.c` | `IsMoveHM`, `CannotForgetMove`。 |
| `src/pokemon_summary_screen.c` | `ShowCantForgetHMsWindow`, `Task_HandleInputCantForgetHMsMoves`, `PrintHMMovesCantBeForgotten`。 |
| `include/config/pokemon.h` | `P_CAN_FORGET_HIDDEN_MOVE`。 |
| `include/config/battle.h` | `B_CATCH_SWAP_CHECK_HMS`。 |
| `include/config/overworld.h` | `OW_DEFOG_FIELD_MOVE`, `OW_ROCK_CLIMB_FIELD_MOVE`, `OW_FLAG_POKE_RIDER`。 |

拡張時の注意:

- 「秘伝技をなくす」は複数の意味に分かれる。フィールドで技所持を不要にする、HM item を消す、忘れられるようにする、badge / flag 解禁だけにする、Poke Rider 化する、は別変更。
- `ScrCmd_checkfieldmove` だけを変えると party menu の field move action、summary の HM forget 制限、捕獲入れ替え HM check とずれる可能性がある。
- `data/scripts/field_move_scripts.inc` の script flow と `gFieldCallback2` / `gPostMenuFieldCallback` の復帰を確認する。

## Items

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/constants/items.h` | `ITEM_*` 定数、item count / pocket 関連定数。 |
| `include/item.h` | `struct ItemInfo`, `ItemUseFunc`, `GetItemName`, `GetItemPrice`, `GetItemFieldFunc`, `GetItemBattleUsage` など item getter。 |
| `src/data/items.h` | `gItemsInfo` item database。 |
| `src/item.c` | bag pocket pointer、`AddBagItem`, `RemoveBagItem`, `CheckBagHasItem`, `GetBagItemQuantity` 系。 |
| `src/item_menu.c` | bag UI、TM/HM 表示、registered item 処理。 |
| `src/item_use.c` | field item use。 |
| `src/shop.c` | shop 購入 UI と価格処理。 |
| `test/text.c` | item names / plural names / descriptions の text fit test。 |

拡張時の注意:

- item 追加は constants、`gItemsInfo`、pocket capacity、field use callback、battle usage、shop、text fit に波及する。
- 新しい item effect は `ItemUseFunc` と battle item command のどちらで使うかを最初に分ける。

## Moves

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/constants/moves.h` | `enum Move`, `MOVES_COUNT_ALL`, `MOVE_DEFAULT`, `MOVE_UNAVAILABLE`。 |
| `include/move.h` | `struct MoveInfo`, `gMovesInfo[MOVES_COUNT_ALL]`, `gBattleMoveEffects[]`, `GetMoveName`, `GetMoveEffect`, `GetMoveBattleScript` など。 |
| `src/data/moves_info.h` | move data table。 |
| `src/data/battle_move_effects.h` | `gBattleMoveEffects[NUM_BATTLE_MOVE_EFFECTS]`。 |
| `data/battle_scripts_1.s`, `data/battle_scripts_2.s` | battle script sources。 |
| `test/battle/move_effect/`, `test/battle/move_flags/`, `test/battle/move_animations/` | move tests。 |

拡張時の注意:

- 新技は move constant、data table、effect、battle script、animation、AI、learnset、TM/HM、text tests を分けて調査する。
- `MoveInfo` の field は多く、effect 追加と data 追加は同じ作業ではない。

## Abilities

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/constants/abilities.h` | `enum Ability`, `ABILITIES_COUNT`。 |
| `include/pokemon.h` | `struct AbilityInfo`, `struct SpeciesInfo.abilities`, `GetAbilityBySpecies`, `GetMonAbility`, `GetSpeciesAbility`。 |
| `src/data/abilities.h` | `gAbilitiesInfo[ABILITIES_COUNT]`。 |
| `src/data/pokemon/species_info/*.h` | species ごとの `.abilities = { ... }`。 |
| `src/pokemon.c` | `GetAbilityBySpecies`, `GetMonAbility`, `GetSpeciesAbility`。 |
| `src/battle_util.c` | `AbilityBattleEffects`, battler ability helper、copy / suppress / overwrite 系。 |
| `src/battle_ai_main.c`, `src/battle_ai_util.c`, `src/battle_ai_switch.c` | AI の ability 参照。 |
| `src/battle_interface.c` | ability popup が `gAbilitiesInfo[ability].name` を使う。 |
| `test/battle/ability/`, `test/text.c` | ability behavior / text tests。 |

拡張時の注意:

- 新特性は constants と description だけでは足りない。battle trigger、AI 評価、copy/suppress/tracing 可否、popup、tests を確認する。
- species への割り当て変更は `struct SpeciesInfo.abilities[NUM_ABILITY_SLOTS]` と form / regional data に波及する。

## Pokemon Species / Learnsets / Forms

確認した入口:

| File | Symbols / facts |
|---|---|
| `include/constants/species.h` | species constants。 |
| `include/pokemon.h` | `struct SpeciesInfo`, `struct Pokemon`, `struct BoxPokemon`。 |
| `src/data/pokemon/species_info.h` | generation / family files の include 集約。 |
| `src/data/pokemon/species_info/*.h` | base stats、types、abilities、graphics、learnsets、evolutions、forms。 |
| `src/data/pokemon/level_up_learnsets/` | level-up learnsets。 |
| `src/data/pokemon/teachable_learnsets.h` | teachable moves。 |
| `src/data/pokemon/egg_moves.h` | egg moves。 |
| `src/data/pokemon/form_change_tables.h` | form change tables。 |
| `src/pokemon.c` | party / box data accessor、species helper、HM forget logic など。 |

拡張時の注意:

- 新 species は constants、species info、graphics、icon、cry、dex、learnsets、evolution、forms、summary/dex display に波及する。
- ランダム化で species を差し替える場合、DexNav、Pokedex area、trainer preview、battle assets が参照できる species か確認する。

## Callback / Dispatch Watch List

大きい改造で必ず読む indirect call:

| Pattern | Confirmed owners |
|---|---|
| `SetMainCallback2(...)` | `src/main.c`, 多数の screen / menu / battle file。 |
| `CB2_*` | callback2 screen routine の命名規約。battle, party, bag, shop, summary, option, overworld return に多い。 |
| `CreateTask(...)` / `gTasks[taskId].func` | `src/task.c`, UI / animation / script wait / field effect。 |
| `ScrCmd_*` | `src/scrcmd.c`, `data/script_cmd_table.inc`。 |
| `special` / `specialvar` | `data/specials.inc`, `src/scrcmd.c`, `data/event_scripts.s`。 |
| `gFieldCallback` / `gFieldCallback2` | `src/overworld.c`, `include/overworld.h`, field return / field effect。 |
| `gPostMenuFieldCallback` | `src/party_menu.c`, `include/field_effect.h`, `include/party_menu.h`。 |
| item use callback | `struct ItemInfo.fieldUseFunc`, `ItemUseFunc`, `gItemUseCB`。 |
| menu exit callback | `gPartyMenu.exitCallback`, `gBagPosition.exitCallback`, `newScreenCallback` など screen-local callback。 |

詳細は `docs/overview/callback_dispatch_map_v15.md` を参照。

## Open Questions

- 野生ランダム化を build-time JSON 変換で行うか、runtime hook で行うか未決定。
- ランダム化 seed を save data、option、new game 固定値、外部生成のどれで管理するか未決定。
- マートのランダム品揃えを script 側で管理するか、C 側 provider に寄せるか未決定。
- TM 追加は現在の `FOREACH_TM` と `ITEM_TM01`..`ITEM_TM100` の関係をさらに確認してから設計する。
- フィールド秘伝技廃止の方針は未決定。少なくとも field script、party menu、summary、catch swap、config を同時に見る必要がある。
- Options / status / summary に独自 option を保存する場合、save block layout 変更の扱いが未決定。
