# Source Map v15

調査日: 2026-05-01

この文書は今後の改造時に参照するソースコード地図です。ファイル名とシンボルは実際に確認したものを記載しています。

## Directory Overview

| Directory | Role |
|---|---|
| `src/` | C 実装本体。field, battle, party menu, storage, save, UI など。 |
| `include/` | C header と constants。`include/config/` もここにある。 |
| `data/` | assembly script, map script, battle script, text, map data。 |
| `asm/` | event macro など assembly macro。`asm/macros/event.inc` が script macro の中心。 |
| `graphics/` | UI / sprite / battle / party menu 画像資産。 |
| `sound/` | 音源データ。 |
| `tools/` | preproc, jsonproc, trainerproc, gfx 変換などのビルド用ツール。 |
| `dev_scripts/` | 開発補助スクリプト。 |
| `test/` | battle tests, runner, Pokémon data tests など。 |
| `docs/` | 既存 documentation。今回の調査 docs もここに追加。 |

## Important Files by System

### Event Script / Field

| File | Important symbols / notes |
|---|---|
| `src/script.c` | `RunScriptCommand`, `ScriptContext_SetupScript`, `ScriptContext_Stop`, `ScriptContext_Enable`, `RunScriptImmediatelyUntilEffect_Internal` |
| `src/scrcmd.c` | `ScrCmd_special`, `ScrCmd_specialvar`, `ScrCmd_waitstate`, `ScrCmd_trainerbattle`, `ScrCmd_dotrainerbattle`, `ScrCmd_gotopostbattlescript`, `ScrCmd_gotobeatenscript` |
| `data/script_cmd_table.inc` | `gScriptCmdTable`, command ID 0x25 `SCR_OP_SPECIAL`, 0x26 `SCR_OP_SPECIALVAR`, 0x5c `SCR_OP_TRAINERBATTLE`, 0x5d `SCR_OP_DOTRAINERBATTLE` |
| `asm/macros/event.inc` | `trainerbattle`, `trainerbattle_single`, `trainerbattle_double`, `dotrainerbattle`, `gotopostbattlescript`, `gotobeatenscript`, `special`, `specialvar` macro |
| `asm/macros/map.inc` | `map_script`, `map_script_2`, `object_event`, `coord_event`, `bg_hidden_item_event`, `map_events` macro |
| `data/event_scripts.s` | `gSpecialVars`, `gSpecials` include, script data section |
| `data/map_events.s` | generated map events include。`data/maps/events.inc` を集約。 |
| `map_data_rules.mk` | `tools/mapjson/mapjson` で `map.json` から `events.inc` / `header.inc` / `connections.inc` を生成。 |
| `data/maps/*/map.json` | object / warp / coord / bg event の source data。 |
| `data/maps/*/events.inc` | generated map event data。直接編集しない。 |
| `data/maps/*/scripts.inc` | hand-written map script。map-specific flag / var / NPC movement / trainerbattle 入口。 |
| `data/scripts/trainer_battle.inc` | `EventScript_TryDoNormalTrainerBattle`, `EventScript_TryDoDoubleTrainerBattle`, `EventScript_DoTrainerBattle` |
| `data/scripts/trainer_script.inc` | `EventScript_TryGetTrainerScript`, `EventScript_GotoTrainerScript` |
| `src/trainer_see.c` | `CheckForTrainersWantingBattle`, `CheckTrainer`, `DoTrainerApproach`, `TryPrepareSecondApproachingTrainer` |
| `include/constants/vars.h` | `VAR_RESULT`, `VAR_0x8000`.., `VAR_TRAINER_BATTLE_OPPONENT_A` |
| `src/event_data.c` | `gSpecialVar_Result`, `GetVarPointer`, `VarGet`, `VarSet` |
| `src/field_control_avatar.c` | `ProcessPlayerFieldInput`, `TryStartCoordEventScript`, `ShouldTriggerScriptRun`, `GetInteractedObjectEventScript`, `GetInteractedBackgroundEventScript` |
| `src/event_object_movement.c` | `TrySpawnObjectEvents`, `RemoveObjectEventByLocalIdAndMap`, `TrySpawnObjectEvent`, `SetObjectInvisibility` |
| `src/item_ball.c` | `GetItemBallIdAndAmountFromTemplate`。item ball object template から item / amount を読む。 |
| `data/scripts/obtain_item.inc` | `Std_FindItem`, `EventScript_HiddenItemScript`。item ball / hidden item 取得 script。 |

### Trainer Battle

| File | Important symbols / notes |
|---|---|
| `src/battle_setup.c` | `gTrainerBattleParameter`, `TrainerBattleLoadArgs`, `BattleSetup_ConfigureTrainerBattle`, `BattleSetup_StartTrainerBattle`, `CB2_EndTrainerBattle`, `BattleSetup_GetScriptAddrAfterBattle`, `BattleSetup_GetTrainerPostBattleScript` |
| `include/battle_setup.h` | `TrainerBattleParameter`, `TRAINER_BATTLE_PARAM`, public battle setup functions |
| `include/constants/battle_setup.h` | `TRAINER_BATTLE_SINGLE`, `TRAINER_BATTLE_DOUBLE`, `TRAINER_BATTLE_CONTINUE_SCRIPT_*`, `TRAINER_BATTLE_TWO_TRAINERS_NO_INTRO`, `TRAINER_BATTLE_EARLY_RIVAL` |
| `src/trainer_see.c` | 視線検知と接近トレーナーの battle setup。 |
| `src/battle_main.c` | `CB2_InitBattle`, `CB2_InitBattleInternal`, `CreateNPCTrainerPartyFromTrainer`, `CreateNPCTrainerParty` |
| `include/data.h` | `struct Trainer`, `struct TrainerMon`, `GetTrainerStructFromId`, `GetTrainerBattleType`, `GetTrainerPartySizeFromId` |
| `src/data.c` | `gTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT]` |
| `src/data/trainers.h`, `src/data/trainers.party` | 通常 trainer data。 |
| `include/constants/trainers.h` | trainer id constants。 |

### Party Menu

| File | Important symbols / notes |
|---|---|
| `src/party_menu.c` | `gPartyMenu`, `InitPartyMenu`, `Task_HandleChooseMonInput`, `InitChooseHalfPartyForBattle`, `ClearSelectedPartyOrder`, `GetBattleEntryEligibility`, `Task_ValidateChosenHalfParty`, `GetMaxBattleEntries`, `GetMinBattleEntries` |
| `include/party_menu.h` | `struct PartyMenu`, `gSelectedOrderFromParty`, `InitChooseHalfPartyForBattle`, `OpenPartyMenuInBattle` |
| `include/constants/party_menu.h` | `PARTY_MENU_TYPE_CHOOSE_HALF`, `PARTY_ACTION_CHOOSE_MON`, `PARTY_MSG_*` |
| `graphics/party_menu/` | 既存 party menu assets。 |
| `src/pokemon_summary_screen.c` | summary 画面遷移。`CB2_ShowPokemonSummaryScreen` から `ShowPokemonSummaryScreen` を呼ぶ。 |

### Battle Engine

| File | Important symbols / notes |
|---|---|
| `src/battle_main.c` | `gBattleTypeFlags`, `gBattleOutcome`, `gBattleMons`, `gBattlerPartyIndexes`, `CB2_InitBattle`, `BattleMainCB1`, `BattleMainCB2` |
| `src/battle_controller_player.c` | `OpenPartyMenuToChooseMon`, `SetBattleEndCallbacks`, `PlayerHandleEndLinkBattle` |
| `src/battle_controller_opponent.c` | opponent controller. |
| `src/battle_script_commands.c` | battle script commands。battle 中の party / item / end 処理の参照候補。`NoAliveMonsForPlayer`, `BS_JumpIfNoWhiteOut` もここ。 |
| `src/battle_util.c`, `src/battle_util2.c` | battle utility。 |
| `src/battle_ai_main.c`, `src/battle_ai_switch.c` | AI と switch 判断。 |
| `include/constants/battle.h` | `BATTLE_TYPE_*`, `B_OUTCOME_*` |
| `include/config/battle.h` | `B_FLAG_NO_WHITEOUT`, `B_CATCH_SWAP_CHECK_HMS`。 |

### Battle UI / Controller

| File | Important symbols / notes |
|---|---|
| `include/battle_interface.h` | `CreateBattlerHealthboxSprites`, `CreatePartyStatusSummarySprites`, `UpdateHealthboxAttribute`, `SwapHpBarsWithHpText`, ability popup / move info / last used ball API。 |
| `src/battle_interface.c` | healthbox sprite、HP/status、party status summary、ability popup、last used ball、move info window。 |
| `src/battle_bg.c` | `BattleInitBgsAndWindows`, `DrawBattleEntryBackground`, `LoadBattleTextboxAndBackground`, battle window templates。 |
| `src/battle_intro.c` | `HandleIntroSlide`, `BattleIntroSlide*`, `BattleIntroNoSlide`。 |
| `include/battle_controllers.h` | controller command enum、`struct HpAndStatus`, `struct ChooseMoveStruct`。 |
| `src/battle_controller_player.c` | `PlayerHandleChooseAction`, `HandleInputChooseAction`, `PlayerHandleChooseMove`, `HandleInputChooseMove`, `OpenPartyMenuToChooseMon`。 |

### UI / Window / Sprite

| File | Important symbols / notes |
|---|---|
| `src/window.c`, `include/window.h` | Window system。 |
| `src/text.c`, `include/text.h` | Text rendering。 |
| `src/menu.c`, `src/menu_helpers.c`, `src/menu_specialized.c` | Menu framework。 |
| `src/sprite.c`, `include/sprite.h` | `gSprites[MAX_SPRITES + 1]`。 |
| `src/pokemon_icon.c` | Party menu icon rendering。 |
| `include/graphics.h` | graphics extern declarations。 |

### Options / Summary / Status

| File | Important symbols / notes |
|---|---|
| `src/option_menu.c` | `CB2_InitOptionMenu`, `Task_OptionMenuSave`, option task fields。 |
| `include/constants/global.h` | `OPTIONS_BUTTON_MODE_*`, `OPTIONS_TEXT_SPEED_*`, `OPTIONS_SOUND_*`, `OPTIONS_BATTLE_STYLE_*`。 |
| `src/new_game.c` | `SetDefaultOptions`。 |
| `include/config/battle.h` | battle UI / input / trainer pool config。 |
| `include/config/summary_screen.h` | summary screen config。 |
| `include/pokemon_summary_screen.h` | summary modes / pages。 |
| `src/pokemon_summary_screen.c` | `ShowPokemonSummaryScreen`, `ShowSelectMovePokemonSummaryScreen`, status sprite / move icon / IV EV 表示。 |

### Trainer Party Pool / Randomizer

| File | Important symbols / notes |
|---|---|
| `docs/tutorials/how_to_trainer_party_pool.md` | Trainer Party Pools の既存説明。 |
| `include/trainer_pools.h` | `struct PoolRules`, `enum PoolRulesets`, `enum PoolPickFunctions`, `enum PoolPruneOptions`, `enum PoolTags`, `DoTrainerPartyPool`。 |
| `src/trainer_pools.c` | `DoTrainerPartyPool`, `PickMonFromPool`, `RandomizePoolIndices`, `PrunePool`, default pick functions。 |
| `src/data/battle_pool_rules.h` | pool ruleset definitions。 |
| `tools/trainerproc/main.c` | trainer party pool DSL processing。 |
| `include/constants/battle_ai.h` | `AI_FLAG_RANDOMIZE_PARTY_INDICES`。 |

### Pokemart / Shop

| File | Important symbols / notes |
|---|---|
| `asm/macros/event.inc` | `pokemart products:req`, `pokemartlistend` macro。 |
| `data/script_cmd_table.inc` | `SCR_OP_POKEMART` -> `ScrCmd_pokemart`。 |
| `src/scrcmd.c` | `ScrCmd_pokemart` が pointer を読み、`CreatePokemartMenu(ptr)` 後に `ScriptContext_Stop()`。 |
| `include/shop.h` | `CreatePokemartMenu`, `CB2_ExitSellMenu`, `gMartPurchaseHistory`。 |
| `src/shop.c` | `SetShopItemsForSale`, `CreatePokemartMenu`, `CB2_InitBuyMenu`, `CB2_BuyMenu`, `Task_BuyMenu`, `BuyMenuTryMakePurchase`, `BuyMenuSubtractMoney`, `ExitBuyMenu`, `Task_ExitBuyMenu`。 |
| `data/maps/*Mart*/scripts.inc` | 通常 Pokemart の item list script。 |
| `data/maps/*DepartmentStore*/scripts.inc` | デパート系 Pokemart の item list script。 |

### Wild Encounter / Randomizer

| File | Important symbols / notes |
|---|---|
| `include/wild_encounter.h` | `struct WildPokemon`, `struct WildPokemonInfo`, `struct WildEncounterTypes`, `struct WildPokemonHeader`, `gWildMonHeaders`, `StandardWildEncounter`, `CreateWildMon`, `ChooseWildMonIndex_*`。 |
| `src/wild_encounter.c` | `StandardWildEncounter`, `TryGenerateWildMon`, `CreateWildMon`, `ChooseWildMonIndex_Land`, `ChooseWildMonIndex_Water`, `ChooseWildMonIndex_Rocks`, `SweetScentWildEncounter`, `FishingWildEncounter`。 |
| `src/data/wild_encounters.json` | wild encounter source data。label `gWildMonHeaders`。 |
| `tools/wild_encounters/wild_encounters_to_header.py` | generated `src/data/wild_encounters.h` の生成 tool。 |
| `src/data/wild_encounters.h` | generated `gWildMonHeaders[]`。`DO NOT MODIFY` comment あり。 |
| `src/field_control_avatar.c` | `CheckStandardWildEncounter` -> `StandardWildEncounter`。 |
| `src/dexnav.c` | DexNav が `gWildMonHeaders` と `CreateWildMon` を参照。 |
| `src/pokedex_area_screen.c` | Pokedex area 表示が `gWildMonHeaders` を参照。 |
| `src/match_call.c` | Match Call 周辺が `gWildMonHeaders` を参照。 |

### TM/HM / Field Move

| File | Important symbols / notes |
|---|---|
| `include/constants/tms_hms.h` | `FOREACH_TM`, `FOREACH_HM`, `FOREACH_TMHM`。 |
| `include/item.h` | `enum TMHMIndex`, `NUM_TECHNICAL_MACHINES`, `NUM_HIDDEN_MACHINES`, `GetItemTMHMIndex`, `GetItemTMHMMoveId`, `GetTMHMItemIdFromMoveId`。 |
| `include/constants/items.h` | `ITEM_TM01` 系、`ITEM_HM01` 系、TM/HM alias enum。 |
| `src/item.c` | `gTMHMItemMoveIds`, TM/HM pocket setup。 |
| `src/data/items.h` | TM/HM item data。`fieldUseFunc = ItemUseOutOfBattle_TMHM`。 |
| `src/item_use.c` | `ItemUseOutOfBattle_TMHM`, `UseTMHM`。 |
| `src/item_menu.c` | `PrepareTMHMMoveWindow`, `PrintTMHMMoveData`。 |
| `src/party_menu.c` | `ItemUseCB_TMHM`, `CanTeachMove`, `GiveMoveToMon`, `SetPartyMonFieldSelectionActions`, `CursorCb_FieldMove`, `DoesSelectedMonKnowHM`, `IsLastMonThatKnowsSurf`。 |
| `include/constants/field_move.h` | `enum FieldMove`, `FIELD_MOVES_COUNT`。 |
| `include/field_move.h` | `struct FieldMoveInfo`, `gFieldMoveInfo`, `FieldMove_GetMoveId`, `IsFieldMoveUnlocked`。 |
| `src/field_move.c` | `gFieldMoveInfo[FIELD_MOVES_COUNT]`, `SetUpFieldMove_*`, `IsFieldMoveUnlocked_*`。 |
| `src/scrcmd.c` | `ScrCmd_checkfieldmove`。 |
| `data/scripts/field_move_scripts.inc` | `EventScript_CutTree`, `EventScript_RockSmash`, `EventScript_StrengthBoulder`, `EventScript_UseWaterfall`, `EventScript_UseDive`, `EventScript_UseDefog`, `EventScript_UseRockClimb`。 |
| `src/pokemon.c` | `IsMoveHM`, `CannotForgetMove`。 |
| `src/pokemon_summary_screen.c` | HM forget warning window。 |
| `src/fldeff_rocksmash.c` | `CreateFieldMoveTask`, `Task_DoFieldMove_Init`, `Task_DoFieldMove_RunFunc`。Cut / Rock Smash / Strength などの共通 field move animation path。 |
| `src/fldeff_cut.c` | `FldEff_UseCutOnTree`, `FldEff_UseCutOnGrass`, `StartCutGrassFieldEffect`。 |
| `src/fldeff_strength.c` | `SetUpFieldMove_Strength`, `FldEff_UseStrength`。 |
| `src/fldeff_flash.c` | `SetUpFieldMove_Flash`, `FldEff_UseFlash`。 |
| `src/field_effect.c` | `FldEff_FieldMoveShowMonInit`, `FldEff_FieldMoveShowMon`, `FldEff_UseSurf`, `FldEff_UseWaterfall`, `FldEff_UseDive`。 |
| `src/field_player_avatar.c` | `SetPlayerAvatarFieldMove`, `PartyHasMonWithSurf`, `IsPlayerFacingSurfableFishableWater`。 |
| `src/pokemon_storage_system.c` | `sRestrictedReleaseMoves`, `CompactPartySlots`。PC release softlock と HM 依存。 |

### Battle End / Aftercare / Release

| File | Important symbols / notes |
|---|---|
| `src/battle_setup.c` | `CB2_EndTrainerBattle`, `HandleBattleVariantEndParty`, `SaveChangesToPlayerParty`, `B_FLAG_SKY_BATTLE`。trainer battle 後の whiteout / field return / heal hook。 |
| `src/battle_main.c` | `ReturnFromBattleToOverworld`, `gBattleOutcome`, `HandleEndTurn_BattleWon`, `HandleEndTurn_BattleLost`, `HandleEndTurn_FinishBattle`。 |
| `src/battle_controller_player.c` | battle end path で `SetMainCallback2(gMain.savedCallback)` を使う。 |
| `src/battle_script_commands.c` | `NoAliveMonsForPlayer`, `BS_JumpIfNoWhiteOut`。 |
| `include/config/battle.h` | `B_FLAG_NO_WHITEOUT`。comment 上、trainer loss 後 whiteout しないが party は自動回復されない。 |
| `src/overworld.c` | `DoWhiteOut`, `CB2_WhiteOut`, `Overworld_ResetBattleFlagsAndVars`。whiteout 時に `HealPlayerParty` を呼ぶ。 |
| `src/script_pokemon_util.c` | `HealPlayerParty`。`gPlayerPartyCount` 分 `HealPokemon`、config 次第で boxes も heal。 |
| `data/specials.inc` | `def_special HealPlayerParty`。 |
| `data/scripts/pkmn_center_nurse.inc`, `data/scripts/pkmn_center_nurse_frlg.inc` | Pokemon Center heal script。 |
| `src/pokemon_storage_system.c` | `Task_ReleaseMon`, `ReleaseMon`, `PurgeMonOrBoxMon`, `CompactPartySlots`, `sRestrictedReleaseMoves`。PC UI release flow。 |
| `include/pokemon_storage_system.h` | `CompactPartySlots` public declaration。 |

### Map Script / Flag / Var

| File | Important symbols / notes |
|---|---|
| `include/constants/flags.h` | `FLAG_TEMP_*`, `FLAG_HIDDEN_ITEMS_START`, `FLAG_RECEIVED_TM_*`, `FLAG_ITEM_*_TM_*`, `FLAGS_COUNT`。 |
| `include/constants/flags_frlg.h` | Emerald TM / item flags が `0` に定義される箇所を確認。FRLG compatibility risk。 |
| `include/constants/vars.h` | `VAR_TEMP_*`, `VAR_OBJ_GFX_ID_*`, story state vars, `VARS_COUNT`。 |
| `include/global.h` | `struct SaveBlock1.flags`, `struct SaveBlock1.vars`, `objectEventTemplates`。 |
| `src/event_data.c` | `ClearTempFieldEventData`, `GetVarPointer`, `VarGet`, `VarSet`, `FlagSet`, `FlagClear`, `FlagGet`。 |
| `src/script.c` | `MapHeaderGetScriptTable`, `MapHeaderCheckScriptTable`, `RunOnTransitionMapScript`, `TryRunOnFrameMapScript`。 |
| `include/constants/map_scripts.h` | map script type と実行 timing の説明。 |
| `src/overworld.c` | `LoadObjEventTemplatesFromHeader`, `LoadSaveblockObjEventScripts`, `SetObjEventTemplateCoords`。 |
| `src/event_object_movement.c` | NPC spawn / hide / remove / transient visibility。 |
| `src/field_control_avatar.c` | coord event、bg event、hidden item、A button interaction。 |

### Item / Move / Ability Data

| Area | File | Important symbols / notes |
|---|---|---|
| Items | `include/item.h` | `struct ItemInfo`, `ItemUseFunc`, item getter functions。 |
| Items | `src/data/items.h` | `gItemsInfo` item database。 |
| Items | `src/item.c` | bag pocket / add / remove / quantity helpers。 |
| Items | `src/item_use.c` | field item use callbacks。 |
| Items | `src/item_menu.c` | bag UI and item display。 |
| Moves | `include/constants/moves.h` | `enum Move`, `MOVES_COUNT_ALL`, `MOVE_DEFAULT`, `MOVE_UNAVAILABLE`。 |
| Moves | `include/move.h` | `struct MoveInfo`, `gMovesInfo`, `gBattleMoveEffects`, move getter helpers。 |
| Moves | `src/data/moves_info.h` | move data table。 |
| Moves | `src/data/battle_move_effects.h` | battle move effect table。 |
| Moves | `data/battle_scripts_1.s`, `data/battle_scripts_2.s` | battle script source。 |
| Abilities | `include/constants/abilities.h` | `enum Ability`, `ABILITIES_COUNT`。 |
| Abilities | `include/pokemon.h` | `struct AbilityInfo`, `struct SpeciesInfo.abilities`, `GetAbilityBySpecies`, `GetMonAbility`, `GetSpeciesAbility`。 |
| Abilities | `src/data/abilities.h` | `gAbilitiesInfo[ABILITIES_COUNT]`。 |
| Abilities | `src/battle_util.c` | `AbilityBattleEffects` and battle ability helpers。 |
| Abilities | `src/battle_ai_main.c`, `src/battle_ai_util.c`, `src/battle_ai_switch.c` | AI ability handling。 |

### Callback / Dispatch

| File | Important symbols / notes |
|---|---|
| `include/main.h` | `MainCallback`, `struct Main`, `gMain`, `SetMainCallback2`。 |
| `src/main.c` | `AgbMainLoop`, `UpdateLinkAndCallCallbacks`, `CallCallbacks`, `SetMainCallback2`。 |
| `include/task.h` | `TaskFunc`, `struct Task`, `NUM_TASKS`, `NUM_TASK_DATA`, `gTasks[]`。 |
| `src/task.c` | `CreateTask`, `RunTasks`, `DestroyTask`, `SetTaskFuncWithFollowupFunc`, `SwitchTaskToFollowupFunc`。 |
| `include/overworld.h` | `gFieldCallback`, `gFieldCallback2`。 |
| `src/overworld.c` | `RunFieldCallback`。 |
| `include/field_effect.h` | `gPostMenuFieldCallback`, `gFieldCallback2` extern。 |
| `src/party_menu.c` | `gPostMenuFieldCallback`, `gPartyMenu.exitCallback`。 |

### Save / State

| File | Important symbols / notes |
|---|---|
| `src/load_save.c` | `gSaveBlock1Ptr`, `gSaveBlock2Ptr`, `SavePlayerParty`, `LoadPlayerParty` |
| `include/load_save.h` | save block pointer externs and save/load function declarations。 |
| `include/global.h` | `struct SaveBlock1`, `struct SaveBlock2`, `struct BattleFrontier.selectedPartyMons` |
| `src/pokemon.c` | `GetSavedPlayerPartyMon`, `GetSavedPlayerPartyCount`, `SavePlayerPartyMon` |

## Files Related to Battle Selection Idea

| File | Why it matters | Important symbols | Risk |
|---|---|---|---|
| `src/battle_setup.c` | trainer battle の開始・終了 callback を握る。選出の差し込み候補。 | `BattleSetup_StartTrainerBattle`, `CB2_EndTrainerBattle`, `TRAINER_BATTLE_PARAM`, `gMain.savedCallback` | Very High |
| `src/trainer_see.c` | 視線検知トレーナーは手動会話と入口が違う。 | `CheckForTrainersWantingBattle`, `ConfigureAndSetUpOneTrainerBattle`, `ConfigureTwoTrainersBattle` | High |
| `data/scripts/trainer_battle.inc` | script レベルで battle 前の会話から `dotrainerbattle` へ進む。 | `EventScript_DoTrainerBattle`, `EventScript_ShowTrainerIntroMsg` | High |
| `src/scrcmd.c` | `trainerbattle` / `dotrainerbattle` command handler。 | `ScrCmd_trainerbattle`, `ScrCmd_dotrainerbattle` | High |
| `include/battle_setup.h` | battle parameter layout。script macro と binary layout に直結。 | `TrainerBattleParameter`, `TRAINER_BATTLE_PARAM` | High |
| `asm/macros/event.inc` | trainerbattle macro のバイナリ生成元。 | `trainerbattle`, `trainerbattle_single`, `trainerbattle_double` | High |
| `src/party_menu.c` | 既存選出 UI。MVP 流用候補。 | `InitChooseHalfPartyForBattle`, `gSelectedOrderFromParty`, `GetMaxBattleEntries`, `Task_ValidateChosenHalfParty` | Very High |
| `src/script_pokemon_util.c` | `ChooseHalfPartyForBattle` special と party 圧縮関数。 | `ChooseHalfPartyForBattle`, `ReducePlayerPartyToSelectedMons` | Very High |
| `src/load_save.c` | party 退避・復元の既存関数。 | `SavePlayerParty`, `LoadPlayerParty` | High |
| `src/pokemon.c` | party count と slot 単位保存。 | `CalculatePlayerPartyCount`, `SavePlayerPartyMon`, `GetSavedPlayerPartyMon` | Very High |
| `src/battle_main.c` | battle init 時に敵 party を生成し、battle 本体 callback へ入る。 | `CB2_InitBattle`, `CB2_InitBattleInternal`, `CreateNPCTrainerPartyFromTrainer` | High |
| `src/trainer_pools.c` | trainer party pool / randomize / reorder を行う。相手 party preview と関係。 | `DoTrainerPartyPool`, `RandomizePoolIndices`, `PickMonFromPool` | High |
| `include/trainer_pools.h` | pool rule / tag / pick function 定義。 | `struct PoolRules`, `MON_POOL_TAG_*`, `POOL_RULESET_*` | Medium |
| `tools/trainerproc/main.c` | trainer party pool data を生成する build tool。 | `Party Size`, `Pool Rules`, `Pool Pick Functions`, `Pool Prune` | Medium |
| `src/battle_interface.c` | battle 開始後 UI の healthbox / party status summary。選出後 party count の見え方に影響。 | `CreateBattlerHealthboxSprites`, `CreatePartyStatusSummarySprites`, `UpdateHealthboxAttribute` | High |
| `src/battle_bg.c` | battle window layout。battle UI 変更時の基盤。 | `BattleInitBgsAndWindows`, `sStandardBattleWindowTemplates` | High |
| `src/battle_controller_player.c` | action/move menu と battle 中 party menu。 | `PlayerHandleChooseAction`, `PlayerHandleChooseMove`, `OpenPartyMenuToChooseMon` | High |
| `src/option_menu.c` | 将来 UI option を増やす場合の入口。 | `CB2_InitOptionMenu`, `Task_OptionMenuSave` | Medium |
| `src/pokemon_summary_screen.c` | 選出 UI から summary を開く場合の遷移先。 | `ShowPokemonSummaryScreen`, `ShowSelectMovePokemonSummaryScreen` | Medium |
| `include/constants/battle.h` | single/double/multi 判定の source。 | `BATTLE_TYPE_TRAINER`, `BATTLE_TYPE_DOUBLE`, `BATTLE_TYPE_MULTI`, `BATTLE_TYPE_TWO_OPPONENTS` | High |
| `include/constants/battle_frontier.h` | 既存 choose half が Frontier facility id に依存。 | `FACILITY_MULTI_OR_EREADER`, `FACILITY_UNION_ROOM`, `FRONTIER_FACILITY_*` | Medium |
| `include/constants/frontier_util.h` | `frontier_setpartyorder` 周辺の function id。 | `FRONTIER_UTIL_FUNC_SET_PARTY_ORDER`, `FRONTIER_DATA_SELECTED_MON_ORDER` | Medium |
| `data/maps/*/scripts.inc` | 通常 trainerbattle の利用箇所。 | `trainerbattle_single`, `trainerbattle_double` | Medium |

## Related Investigation Docs

| Doc | Purpose |
|---|---|
| `docs/flows/battle_ui_flow_v15.md` | battle 開始後の UI / healthbox / battle menu / party status summary。 |
| `docs/flows/options_status_flow_v15.md` | option menu、battle UI config、summary/status 表示。 |
| `docs/flows/script_inc_audit_v15.md` | `.inc` script の battle / selection 関連 audit。 |
| `docs/flows/map_script_flag_var_flow_v15.md` | `map.json`、generated `.inc`、hand-written `scripts.inc`、flag / var、NPC visibility、item ball / hidden item flow。 |
| `docs/features/battle_selection/opponent_party_and_randomizer.md` | 相手 party preview、Trainer Party Pools、party randomize / reorder。 |
| `docs/features/tm_shop_migration/` | TM を shop へ寄せるための取得元、flag、map script 影響範囲。 |
| `docs/overview/extension_impact_map_v15.md` | マート、野生、TM/HM、field move、item、move、ability、species など将来拡張の横断影響範囲。 |
| `docs/overview/callback_dispatch_map_v15.md` | `SetMainCallback2`、`CB2_*`、`CreateTask`、`ScrCmd_*`、`special`、field callback の間接呼び出し地図。 |

## Open Questions

- 通常戦の全 `trainerbattle_*` を script 側で包むか、`BattleSetup_StartTrainerBattle` の C 側で統一的に差し込むか。
- 既存 `PARTY_MENU_TYPE_CHOOSE_HALF` は Frontier ルールを含むため、通常戦用に分岐を足す設計が必要か。
- 相手 party 表示 UI は `gEnemyParty` 生成後でないと正確な pool / override / difficulty 反映済み party が見えない可能性がある。
- battle 開始後の party status summary を 3/4 匹表示に変えるか、既存の 6 slot 表示を維持するか。
- 新しい UI 表示切替を runtime option にする場合、save data layout 変更が必要か。
