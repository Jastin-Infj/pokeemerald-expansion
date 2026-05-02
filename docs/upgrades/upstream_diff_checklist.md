# Upstream Diff Checklist

v15.x 以降へ upstream 更新する時、独自機能や調査 docs への影響を確認するための checklist。

## Always Check

| Area | Files / Symbols | Why |
|---|---|---|
| Event script dispatch | `src/script.c`, `src/scrcmd.c`, `data/script_cmd_table.inc` | `special`、`trainerbattle`、`waitstate` の動きが変わると feature insertion が壊れる |
| Map scripts / generated events | `map_data_rules.mk`, `asm/macros/map.inc`, `data/event_scripts.s`, `data/map_events.s`, `data/maps/*/map.json`, `data/maps/*/scripts.inc`, generated `data/maps/*/events.inc` | map JSON 生成、NPC hide flag、coord/bg event、item ball flow が変わると map script 改造が壊れる |
| Flag / var runtime | `include/constants/flags.h`, `include/constants/flags_frlg.h`, `include/constants/vars.h`, `src/event_data.c`, `include/global.h` | saved flag / var layout、temp clear、special vars が変わると script state が壊れる |
| Trainer battle setup | `src/battle_setup.c`, `include/battle_setup.h`, `include/constants/battle_setup.h` | `gTrainerBattleParameter`、battle mode、callback が中心 |
| Trainer see | `src/trainer_see.c`, `include/trainer_see.h` | field trainer encounter の入口 |
| Trainer battle scripts | `data/scripts/trainer_battle.inc`, `data/scripts/trainer_script.inc` | `dotrainerbattle` 前後の flow |
| Party menu | `src/party_menu.c`, `include/party_menu.h`, `include/constants/party_menu.h` | choose half UI と選出順管理 |
| Choose half helpers | `src/script_pokemon_util.c`, `data/specials.inc` | `ChooseHalfPartyForBattle`、`ReducePlayerPartyToSelectedMons` |
| Frontier selection | `src/frontier_util.c`, `asm/macros/battle_frontier/frontier_util.inc` | 既存選出 flow との衝突確認 |
| Battle main | `src/battle_main.c`, `include/constants/battle.h` | battle init/end、enemy party 作成、battle flags |
| Player controller | `src/battle_controller_player.c` | battle end callback |
| Battle UI | `src/battle_interface.c`, `include/battle_interface.h`, `src/battle_bg.c`, `src/battle_intro.c`, `include/battle_controllers.h` | healthbox、party status summary、battle window、intro、controller command |
| Options / summary | `src/option_menu.c`, `src/new_game.c`, `include/config/battle.h`, `include/config/summary_screen.h`, `src/pokemon_summary_screen.c` | UI option、battle config、summary/status 表示 |
| Trainer party pools | `include/trainer_pools.h`, `src/trainer_pools.c`, `src/data/battle_pool_rules.h`, `tools/trainerproc/main.c`, `include/constants/battle_ai.h` | opponent party preview、randomizer、party order |
| Party storage | `src/pokemon.c`, `src/load_save.c`, `include/global.h` | `gPlayerParty`、save/restore |
| Callback / dispatch | `src/main.c`, `include/main.h`, `src/task.c`, `include/task.h`, `src/overworld.c`, `include/overworld.h`, `include/field_effect.h` | `SetMainCallback2`、`CB2_*`、`CreateTask`、field callback の意味が変わると screen / menu insertion が壊れる |
| Pokemart / shop | `src/shop.c`, `include/shop.h`, `src/scrcmd.c`, `data/script_cmd_table.inc`, `asm/macros/event.inc`, `data/maps/*Mart*/scripts.inc`, `data/maps/*DepartmentStore*/scripts.inc` | shop script、dynamic shop、purchase UI、script return |
| Wild encounter | `src/wild_encounter.c`, `include/wild_encounter.h`, `src/data/wild_encounters.json`, `tools/wild_encounters/wild_encounters_to_header.py`, generated `src/data/wild_encounters.h` | wild randomizer、DexNav、Pokedex area、generated data |
| TM/HM / field move | `include/constants/tms_hms.h`, `include/item.h`, `src/item.c`, `src/item_use.c`, `src/item_menu.c`, `src/field_move.c`, `include/field_move.h`, `data/scripts/field_move_scripts.inc`, `data/scripts/surf.inc`, `src/party_menu.c`, `src/pokemon.c`, `src/field_effect.c`, `src/fldeff_rocksmash.c`, `src/fldeff_cut.c`, `src/fldeff_strength.c`, `src/fldeff_flash.c`, `src/field_player_avatar.c` | TM/HM 追加、field HM removal、forget HM、field move script / animation |
| TM shop migration | `include/constants/tms_hms.h`, `include/constants/items.h`, `include/constants/flags.h`, `include/constants/flags_frlg.h`, `data/maps/*/map.json`, `data/maps/*/scripts.inc`, `data/scripts/obtain_item.inc`, `data/scripts/item_ball_scripts.inc`, `src/item_ball.c`, `src/shop.c` | TM 取得元、item ball / hidden item、shop 販売への移行 |
| Trainer battle aftercare / forced release | `src/battle_setup.c`, `src/battle_main.c`, `src/battle_controller_player.c`, `src/battle_script_commands.c`, `include/config/battle.h`, `src/overworld.c`, `src/script_pokemon_util.c`, `src/pokemon_storage_system.c`, `include/pokemon_storage_system.h` | trainer loss / no-whiteout / heal / forced release / battle selection restore の順序 |
| Item / move / ability data | `include/constants/items.h`, `src/data/items.h`, `include/constants/moves.h`, `include/move.h`, `src/data/moves_info.h`, `src/data/battle_move_effects.h`, `include/constants/abilities.h`, `src/data/abilities.h`, `src/battle_util.c` | 独自 item / move / ability 追加で upstream table layout が変わると壊れる |

## Suggested Diff Commands

実際の revision 名は更新作業時に置き換える。

```bash
git diff OLD_REV..NEW_REV -- src/battle_setup.c
git diff OLD_REV..NEW_REV -- map_data_rules.mk asm/macros/map.inc data/event_scripts.s data/map_events.s
git diff OLD_REV..NEW_REV -- include/constants/flags.h include/constants/flags_frlg.h include/constants/vars.h src/event_data.c include/global.h
git diff OLD_REV..NEW_REV -- data/scripts/trainer_battle.inc
git diff OLD_REV..NEW_REV -- src/party_menu.c
git diff OLD_REV..NEW_REV -- src/script_pokemon_util.c
git diff OLD_REV..NEW_REV -- src/battle_main.c
git diff OLD_REV..NEW_REV -- src/load_save.c src/pokemon.c
git diff OLD_REV..NEW_REV -- src/battle_interface.c src/battle_bg.c src/battle_intro.c
git diff OLD_REV..NEW_REV -- src/option_menu.c src/pokemon_summary_screen.c include/config/battle.h
git diff OLD_REV..NEW_REV -- include/trainer_pools.h src/trainer_pools.c src/data/battle_pool_rules.h tools/trainerproc/main.c
git diff OLD_REV..NEW_REV -- src/main.c include/main.h src/task.c include/task.h src/overworld.c include/overworld.h
git diff OLD_REV..NEW_REV -- src/shop.c include/shop.h src/scrcmd.c data/script_cmd_table.inc asm/macros/event.inc
git diff OLD_REV..NEW_REV -- data/scripts/obtain_item.inc data/scripts/item_ball_scripts.inc src/item_ball.c
git diff OLD_REV..NEW_REV -- src/wild_encounter.c include/wild_encounter.h src/data/wild_encounters.json tools/wild_encounters/wild_encounters_to_header.py
git diff OLD_REV..NEW_REV -- include/constants/tms_hms.h include/item.h src/item.c src/item_use.c src/item_menu.c src/field_move.c include/field_move.h data/scripts/field_move_scripts.inc data/scripts/surf.inc
git diff OLD_REV..NEW_REV -- src/field_effect.c src/fldeff_rocksmash.c src/fldeff_cut.c src/fldeff_strength.c src/fldeff_flash.c src/field_player_avatar.c
git diff OLD_REV..NEW_REV -- src/battle_setup.c src/battle_main.c src/battle_controller_player.c src/battle_script_commands.c include/config/battle.h src/overworld.c src/script_pokemon_util.c src/pokemon_storage_system.c include/pokemon_storage_system.h
git diff OLD_REV..NEW_REV -- include/constants/items.h src/data/items.h include/constants/moves.h include/move.h src/data/moves_info.h src/data/battle_move_effects.h include/constants/abilities.h src/data/abilities.h
```

## Symbol Checklist

更新後に存在と意味を確認する。

- `TrainerBattleParameter`
- `gTrainerBattleParameter`
- `TRAINER_BATTLE_PARAM`
- `TrainerBattleLoadArgs`
- `BattleSetup_ConfigureTrainerBattle`
- `BattleSetup_StartTrainerBattle`
- `CB2_EndTrainerBattle`
- `EventScript_DoTrainerBattle`
- `EventScript_TryDoNormalTrainerBattle`
- `EventScript_TryDoDoubleTrainerBattle`
- `InitChooseHalfPartyForBattle`
- `ChooseHalfPartyForBattle`
- `ChoosePartyForBattleFrontier`
- `ReducePlayerPartyToSelectedMons`
- `gSelectedOrderFromParty`
- `gPlayerParty`
- `gEnemyParty`
- `gBattleTypeFlags`
- `gMain.savedCallback`
- `CreatePartyStatusSummarySprites`
- `CreateBattlerHealthboxSprites`
- `BattleInitBgsAndWindows`
- `PlayerHandleChooseAction`
- `PlayerHandleChooseMove`
- `DoTrainerPartyPool`
- `RandomizePoolIndices`
- `CreateNPCTrainerPartyFromTrainer`
- `AI_FLAG_RANDOMIZE_PARTY_INDICES`
- `SetMainCallback2`
- `AgbMainLoop`
- `CallCallbacks`
- `CreateTask`
- `RunTasks`
- `SetTaskFuncWithFollowupFunc`
- `SwitchTaskToFollowupFunc`
- `gFieldCallback`
- `gFieldCallback2`
- `gPostMenuFieldCallback`
- `ScrCmd_pokemart`
- `CreatePokemartMenu`
- `SetShopItemsForSale`
- `Task_BuyMenu`
- `map_script`
- `map_script_2`
- `object_event`
- `coord_event`
- `bg_hidden_item_event`
- `MapHeaderCheckScriptTable`
- `ShouldTriggerScriptRun`
- `TryRunCoordEventScript`
- `GetInteractedBackgroundEventScript`
- `TrySpawnObjectEvents`
- `RemoveObjectEventByLocalIdAndMap`
- `TrySpawnObjectEvent`
- `SetObjectInvisibility`
- `GetItemBallIdAndAmountFromTemplate`
- `EventScript_HiddenItemScript`
- `SetHiddenItemFlag`
- `FLAG_RECEIVED_TM_*`
- `FLAG_ITEM_*_TM_*`
- `FLAG_HIDDEN_ITEM_*_TM_*`
- `StandardWildEncounter`
- `TryGenerateWildMon`
- `CreateWildMon`
- `gWildMonHeaders`
- `FOREACH_TM`
- `FOREACH_HM`
- `gTMHMItemMoveIds`
- `ItemUseOutOfBattle_TMHM`
- `ScrCmd_checkfieldmove`
- `gFieldMoveInfo`
- `FieldMove_GetMoveId`
- `FieldMove_GetPartyMsgID`
- `CreateFieldMoveTask`
- `SetPlayerAvatarFieldMove`
- `FldEff_FieldMoveShowMonInit`
- `FldEff_FieldMoveShowMon`
- `FldEff_UseSurf`
- `FldEff_UseWaterfall`
- `FldEff_UseDive`
- `IsMoveHM`
- `CannotForgetMove`
- `P_CAN_FORGET_HIDDEN_MOVE`
- `B_CATCH_SWAP_CHECK_HMS`
- `B_FLAG_NO_WHITEOUT`
- `NoAliveMonsForPlayer`
- `BS_JumpIfNoWhiteOut`
- `HealPlayerParty`
- `DoWhiteOut`
- `CB2_WhiteOut`
- `Task_ReleaseMon`
- `PurgeMonOrBoxMon`
- `CompactPartySlots`
- `sRestrictedReleaseMoves`
- `struct ItemInfo`
- `struct MoveInfo`
- `gMovesInfo`
- `gBattleMoveEffects`
- `struct AbilityInfo`
- `gAbilitiesInfo`
- `AbilityBattleEffects`

## Behavior Checklist

- `trainerbattle` command が battle を即開始せず、trainer battle script へ差し替える構造が維持されているか。
- `dotrainerbattle` が `BattleSetup_StartTrainerBattle` を呼ぶ構造が維持されているか。
- choose half の選出 order が 1-based slot のままか。
- choose half の max/min entry 計算が変わっていないか。
- `ReducePlayerPartyToSelectedMons` のコピー範囲と zero 処理が変わっていないか。
- `CB2_EndTrainerBattle` の先頭や return path が変わっていないか。
- `gEnemyParty` 作成 timing が変わっていないか。
- Sky Battle など既存 subset party 復元処理が変わっていないか。
- battle party status summary が `PARTY_SIZE` 前提のままか、選出数表示に使える余地が増えたか。
- trainer party pool の seed / RNG / pick function が変わっていないか。
- `AI_FLAG_RANDOMIZE_PARTY_INDICES` の扱いが変わっていないか。
- option menu の save field や default option 初期化が変わっていないか。
- `SetMainCallback2` が `gMain.state` を reset する仕様が変わっていないか。
- `RunTasks` が `gTasks[taskId].func(taskId)` を呼ぶ構造と task priority / active list が変わっていないか。
- `ScrCmd_pokemart` が `CreatePokemartMenu` と `ScriptContext_Stop()` を使う構造が変わっていないか。
- `src/shop.c` の buy menu が `CB2_InitBuyMenu` -> `CB2_BuyMenu` -> `Task_BuyMenu` の構造を維持しているか。
- `map_data_rules.mk` の `mapjson` output が `events.inc` / `header.inc` / `connections.inc` のままか。
- `object_event` の `event_flag` が set なら object が spawn しない構造が変わっていないか。
- `removeobject` が hide flag を set する構造が変わっていないか。
- `coord_event` の trigger が var pointer なしの場合 flag として扱われる構造が変わっていないか。
- item ball が `gSpecialVar_LastTalked - 1` から template item id を読む構造が変わっていないか。
- hidden item が `gSpecialVar_0x8004` に hidden item flag、`gSpecialVar_0x8005` に item、`gSpecialVar_0x8009` に quantity を入れる構造が変わっていないか。
- `src/data/wild_encounters.h` が generated file のままか、source JSON / tool の形式が変わっていないか。
- `StandardWildEncounter` 以外の DexNav / Fishing / Sweet Scent / Rock Smash が wild encounter data を読む経路が変わっていないか。
- `FOREACH_TM` / `FOREACH_HM` と `NUM_TECHNICAL_MACHINES` / `NUM_HIDDEN_MACHINES` の生成関係が変わっていないか。
- `ScrCmd_checkfieldmove` と `gFieldMoveInfo` の技所持 / unlock 判定が変わっていないか。
- `checkfieldmove` の `VAR_RESULT` が party slot または `PARTY_SIZE` を返す前提が変わっていないか。
- Cut / Rock Smash の `removeobject VAR_LAST_TALKED`、Strength / Flash の `FLAG_SYS_USE_STRENGTH` / `FLAG_SYS_USE_FLASH` が変わっていないか。
- `CreateFieldMoveTask` と `FldEff_FieldMoveShowMonInit` が party slot を読む前提が変わっていないか。
- Surf / Waterfall / Dive の field effect task が `gFieldEffectArguments[0]` を mon slot として使う前提が変わっていないか。
- `CannotForgetMove` / `IsMoveHM` / `P_CAN_FORGET_HIDDEN_MOVE` の関係が変わっていないか。
- `CB2_EndTrainerBattle` の lost / forfeit / no-whiteout / facility 分岐が変わっていないか。
- `B_FLAG_NO_WHITEOUT` が trainer loss 後に party を自動 heal しない前提が変わっていないか。
- `HealPlayerParty` が party だけでなく boxes / Tera Orb なども触る仕様が変わっていないか。
- PC release の `Task_ReleaseMon` / `PurgeMonOrBoxMon` / `CompactPartySlots` / `sRestrictedReleaseMoves` が変わっていないか。
- item / move / ability の data table field が増減していないか。

## Docs to Update After Upstream Merge

- `docs/overview/project_overview_v15.md`
- `docs/overview/source_map_v15.md`
- `docs/overview/extension_impact_map_v15.md`
- `docs/overview/callback_dispatch_map_v15.md`
- `docs/flows/event_script_flow_v15.md`
- `docs/flows/trainer_battle_flow_v15.md`
- `docs/flows/party_menu_flow_v15.md`
- `docs/flows/choose_half_party_flow_v15.md`
- `docs/flows/battle_start_end_flow_v15.md`
- `docs/flows/battle_ui_flow_v15.md`
- `docs/flows/options_status_flow_v15.md`
- `docs/flows/script_inc_audit_v15.md`
- `docs/flows/map_script_flag_var_flow_v15.md`
- `docs/flows/field_move_hm_flow_v15.md`
- `docs/features/battle_selection/investigation.md`
- `docs/features/battle_selection/opponent_party_and_randomizer.md`
- `docs/features/battle_selection/risks.md`
- `docs/features/battle_selection/mvp_plan.md`
- `docs/features/field_move_modernization/README.md`
- `docs/features/field_move_modernization/investigation.md`
- `docs/features/field_move_modernization/risks.md`
- `docs/features/field_move_modernization/mvp_plan.md`
- `docs/features/field_move_modernization/test_plan.md`
- `docs/features/trainer_battle_aftercare/README.md`
- `docs/features/trainer_battle_aftercare/investigation.md`
- `docs/features/trainer_battle_aftercare/risks.md`
- `docs/features/trainer_battle_aftercare/mvp_plan.md`
- `docs/features/trainer_battle_aftercare/test_plan.md`
- `docs/features/tm_shop_migration/README.md`
- `docs/features/tm_shop_migration/investigation.md`
- `docs/features/tm_shop_migration/risks.md`
- `docs/features/tm_shop_migration/mvp_plan.md`
- `docs/features/tm_shop_migration/test_plan.md`

## Open Questions

- upstream の changelog だけで判断せず、上記 files は毎回 diff する。
- v15.x から v16 系へ上げる場合は、battle engine と party menu の設計が大きく変わる可能性があるため、docs 全体の再調査が必要。
