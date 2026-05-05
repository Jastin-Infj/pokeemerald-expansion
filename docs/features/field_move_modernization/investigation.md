# Field Move Modernization Investigation

調査日: 2026-05-02

## Confirmed Source Facts

### TM / HM Constants

| File | Facts |
|---|---|
| `include/constants/tms_hms.h` | `FOREACH_TM(F)` は 50 entries。`FOREACH_HM(F)` は `CUT`, `FLY`, `SURF`, `STRENGTH`, `FLASH`, `ROCK_SMASH`, `WATERFALL`, `DIVE`。 |
| `include/item.h` | `enum TMHMIndex`, `NUM_TECHNICAL_MACHINES`, `NUM_HIDDEN_MACHINES`, `GetItemTMHMIndex`, `GetItemTMHMMoveId`, `GetTMHMItemIdFromMoveId`。 |
| `src/data/items.h` | HM item も TM/HM pocket item として定義され、`fieldUseFunc = ItemUseOutOfBattle_TMHM` を使う。 |
| `src/item_use.c` | `ItemUseOutOfBattle_TMHM`, `UseTMHM`。 |

### Field Move Table

| File | Facts |
|---|---|
| `include/constants/field_move.h` | `enum FieldMove` は HM 系だけでなく `TELEPORT`, `DIG`, `SECRET_POWER`, `MILK_DRINK`, `SOFT_BOILED`, `SWEET_SCENT` も含む。 |
| `include/field_move.h` | `struct FieldMoveInfo` は `fieldMoveFunc`, `isUnlockedFunc`, `moveID`, `partyMsgID` を持つ。 |
| `src/field_move.c` | `gFieldMoveInfo[FIELD_MOVES_COUNT]` が field move と move ID / unlock / party message を結合する。 |

### Script Flow

| File | Facts |
|---|---|
| `asm/macros/event.inc` | `checkfieldmove fieldMove:req, checkUnlocked=FALSE`。 |
| `data/script_cmd_table.inc` | `SCR_OP_CHECKFIELDMOVE` は `ScrCmd_checkfieldmove`。 |
| `src/scrcmd.c` | `ScrCmd_checkfieldmove` は `IsFieldMoveUnlocked` と `MonKnowsMove` を使う。`VAR_RESULT` は party slot または `PARTY_SIZE`。 |
| `data/scripts/field_move_scripts.inc` | `EventScript_CutTree`, `EventScript_RockSmash`, `EventScript_StrengthBoulder`, `EventScript_UseWaterfall`, `EventScript_UseDive` など。 |
| `data/scripts/surf.inc` | `EventScript_UseSurf`。 |

### Party Menu Flow

| File | Facts |
|---|---|
| `src/party_menu.c` | `SetPartyMonFieldSelectionActions` が selected mon の moves と `FieldMove_GetMoveId(j)` を照合する。 |
| `src/party_menu.c` | `CursorCb_FieldMove` が `IsFieldMoveUnlocked`、`SetUpFieldMove`、`gPartyMenu.exitCallback`、`gPostMenuFieldCallback` を使う。 |
| `src/party_menu.c` | `SetUpFieldMove_Surf`, `SetUpFieldMove_Fly`, `SetUpFieldMove_Waterfall`, `SetUpFieldMove_Dive` は follower flag、map type、metatile、warp を確認する。 |

### Field Effects and Animation

| File | Facts |
|---|---|
| `src/fldeff_rocksmash.c` | `CreateFieldMoveTask`、`Task_DoFieldMove_Init`、`Task_DoFieldMove_WaitForMon`、`Task_DoFieldMove_RunFunc`。Cut / Rock Smash / Strength / Flash / Dig / Teleport / Sweet Scent などの共通 path。 |
| `src/field_player_avatar.c` | `SetPlayerAvatarFieldMove` が player graphics を `PLAYER_AVATAR_STATE_FIELD_MOVE` にし、`ANIM_FIELD_MOVE` を再生。 |
| `src/field_effect.c` | `FldEff_FieldMoveShowMonInit` は `gFieldEffectArguments[0]` を party slot として `gPlayerParty` を読む。 |
| `src/field_effect.c` | Surf / Waterfall / Dive は `CreateFieldMoveTask` とは別 task。 |
| `src/data/object_events/object_event_anims.h` | `sAnim_FieldMove`, `sAnimTable_FieldMove`。 |
| `src/data/object_events/object_event_graphics_info.h` | Brendan / May / Red / Green field move graphics info。 |
| `graphics/field_effects/pics/` | `field_move_streaks.*`, `field_move_streaks_indoors.*`。 |

### Map Object and State

| Area | Facts |
|---|---|
| Cut tree | `EventScript_CutTreeDown` が `removeobject VAR_LAST_TALKED`。 |
| Rock Smash | `EventScript_SmashRock` が `removeobject VAR_LAST_TALKED`、`TryUpdateRusturfTunnelState`、`RockSmashWildEncounter`。 |
| Strength | `EventScript_ActivateStrength` が `FLAG_SYS_USE_STRENGTH`。 |
| Flash | `FldEff_UseFlash` が `FLAG_SYS_USE_FLASH`。 |
| Reset | `Overworld_ResetStateAfterWhiteOut` などが `FLAG_SYS_USE_STRENGTH` / `FLAG_SYS_USE_FLASH` を clear。 |

### Forget and Softlock Prevention

| Area | Facts |
|---|---|
| Core rule | `src/pokemon.c` の `CannotForgetMove` は `P_CAN_FORGET_HIDDEN_MOVE` が `FALSE` の場合、`IsMoveHM(move)` を返す。 |
| Learn move | `src/battle_script_commands.c`、`src/evolution_scene.c` が `CannotForgetMove` を参照。 |
| Summary | `src/pokemon_summary_screen.c` が `CannotForgetMove` を参照。 |
| Move Deleter | `data/maps/LilycoveCity_MoveDeletersHouse/scripts.inc` と `data/maps/FuchsiaCity_House3_Frlg/scripts.inc`。 |
| Surf-specific check | `src/party_menu.c` の `IsLastMonThatKnowsSurf` は `MOVE_SURF` のみを確認。 |
| PC release | `src/pokemon_storage_system.c` の `sRestrictedReleaseMoves` は Surf / Dive と一部 map の Strength / Rock Smash を制限。 |
| Catch swap | `include/config/battle.h` の `B_CATCH_SWAP_CHECK_HMS`。 |

## Confirmed Script Labels

| Label | File | Notes |
|---|---|---|
| `EventScript_CutTree` | `data/scripts/field_move_scripts.inc` | `checkfieldmove FIELD_MOVE_CUT, TRUE`。 |
| `EventScript_UseCut` | `data/scripts/field_move_scripts.inc` | party menu から使う Cut tree path。 |
| `EventScript_RockSmash` | `data/scripts/field_move_scripts.inc` | `checkfieldmove FIELD_MOVE_ROCK_SMASH, TRUE`。 |
| `EventScript_UseRockSmash` | `data/scripts/field_move_scripts.inc` | party menu から使う Rock Smash path。 |
| `EventScript_StrengthBoulder` | `data/scripts/field_move_scripts.inc` | `checkfieldmove FIELD_MOVE_STRENGTH, TRUE`。 |
| `EventScript_UseStrength` | `data/scripts/field_move_scripts.inc` | party menu から使う Strength path。 |
| `EventScript_UseSurf` | `data/scripts/surf.inc` | `checkfieldmove FIELD_MOVE_SURF`。 |
| `EventScript_UseWaterfall` | `data/scripts/field_move_scripts.inc` | `checkfieldmove FIELD_MOVE_WATERFALL`。 |
| `EventScript_UseDive` | `data/scripts/field_move_scripts.inc` | `checkfieldmove FIELD_MOVE_DIVE`。 |
| `EventScript_UseDiveUnderwater` | `data/scripts/field_move_scripts.inc` | `checkfieldmove FIELD_MOVE_DIVE`。 |
| `EventScript_UseRockClimb` | `data/scripts/field_move_scripts.inc` | config enabled path。 |
| `EventScript_UseDefog` | `data/scripts/field_move_scripts.inc` | config enabled path。 |

## External Reference Notes

- `PokemonSanFran/pokeemerald` の No Whiteout wiki は、loss 後に field へ戻す場合でも party は倒れたままなので、必要なら `special HealPlayerParty` を使う、という注意点を示している。
- `Pokabbie/pokeemerald-rogue` と `DepressoMocha/emerogue` の公開 repo は確認した。ただし、強制 release / rogue-like wipeout の具体 source path は今回未確認。

## Open Questions

- `P_CAN_FORGET_HIDDEN_MOVE` を `FALSE` のままにして「HM は忘れられない」を維持するのか、HM move 自体を通常技化するのか。
- field move を key item 化する場合、party slot を要求する animation はどの Pokemon を表示するか。
- Cut / Rock Smash object は削除するのか、自動処理するのか、key item unlock で処理するのか。
- `sRestrictedReleaseMoves` と `B_CATCH_SWAP_CHECK_HMS` は HM 廃止後に残すか。
- Secret Power / secret base を HM modernize の範囲に含めるか。
