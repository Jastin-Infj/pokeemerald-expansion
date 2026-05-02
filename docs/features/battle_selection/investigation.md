# Trainer Battle Party Selection Investigation

## Scope

v15 系の既存コードを読み、通常 trainer battle 前選出に使えそうな既存 flow と、変更時に危険な箇所を整理する。

この document は調査結果であり、実装方針の確定ではない。

## Confirmed Existing Systems

### Trainer Battle Setup

確認した files:

- `asm/macros/event.inc`
- `include/battle_setup.h`
- `include/constants/battle_setup.h`
- `src/scrcmd.c`
- `src/battle_setup.c`
- `data/scripts/trainer_battle.inc`
- `src/trainer_see.c`

確認した flow:

1. script の `trainerbattle` macro が `TrainerBattleParameter` 相当の data を出す。
2. `ScrCmd_trainerbattle` が `TrainerBattleLoadArgs` を呼び、`gTrainerBattleParameter` を埋める。
3. `BattleSetup_ConfigureTrainerBattle` が mode に応じて trainer battle event script へ差し替える。
4. `data/scripts/trainer_battle.inc` の script が intro/check/text を進める。
5. `dotrainerbattle` で `BattleSetup_StartTrainerBattle` が呼ばれる。
6. `BattleSetup_StartTrainerBattle` が `gBattleTypeFlags` と `gMain.savedCallback = CB2_EndTrainerBattle` を設定し、`DoTrainerBattle` へ進む。

### Choose Half Party

確認した files:

- `src/party_menu.c`
- `include/party_menu.h`
- `include/constants/party_menu.h`
- `src/script_pokemon_util.c`
- `src/frontier_util.c`
- `src/load_save.c`

確認した symbols:

- `ChooseHalfPartyForBattle`
- `ChoosePartyForBattleFrontier`
- `InitChooseHalfPartyForBattle`
- `Task_ValidateChosenHalfParty`
- `gSelectedOrderFromParty`
- `ReducePlayerPartyToSelectedMons`
- `SavePlayerParty`
- `LoadPlayerParty`

`gSelectedOrderFromParty` は 1-based party slot を保存する。これは battle 後に選出個体を元 slot へ戻すための mapping として使いやすい。

### Battle Start / End

確認した files:

- `src/battle_main.c`
- `src/battle_setup.c`
- `src/battle_controller_player.c`
- `src/pokemon.c`

確認した symbols:

- `gBattleTypeFlags`
- `gBattleOutcome`
- `gPlayerParty`
- `gEnemyParty`
- `CB2_InitBattle`
- `CB2_InitBattleInternal`
- `CB2_EndTrainerBattle`
- `FreeRestoreBattleData`
- `HandleBattleVariantEndParty`
- `SaveChangesToPlayerParty`

Sky Battle 用の `HandleBattleVariantEndParty` は、一時的な party subset の battle 後状態を saved party 側へ戻してから `LoadPlayerParty` する pattern として参考になる。

### Battle UI / Battle Screen

確認した files:

- `include/battle_interface.h`
- `src/battle_interface.c`
- `src/battle_bg.c`
- `src/battle_intro.c`
- `include/battle_controllers.h`
- `src/battle_controller_player.c`
- `include/config/battle.h`

確認した symbols:

- `BattleInitBgsAndWindows`
- `DrawBattleEntryBackground`
- `HandleIntroSlide`
- `CreateBattlerHealthboxSprites`
- `CreatePartyStatusSummarySprites`
- `UpdateHealthboxAttribute`
- `PlayerHandleChooseAction`
- `HandleInputChooseAction`
- `PlayerHandleChooseMove`
- `HandleInputChooseMove`
- `OpenPartyMenuToChooseMon`

`CreatePartyStatusSummarySprites` は `PARTY_SIZE` 分の party ball tray を扱う。選出後に `gPlayerParty` を 3/4 匹へ圧縮して残り slot を空にすると、battle 開始後の party status summary は空 slot を含む見え方になる可能性がある。

Battle 開始後の UI 変更は、`src/battle_interface.c`、`src/battle_bg.c`、`src/battle_controller_player.c` の複数箇所へ影響する。MVP では battle 開始後 UI を変更せず、既存表示のままにする方が安全。

詳細は `docs/flows/battle_ui_flow_v15.md` を参照。

### Options / Summary / Status

確認した files:

- `src/option_menu.c`
- `include/constants/global.h`
- `src/new_game.c`
- `include/config/battle.h`
- `include/config/summary_screen.h`
- `include/pokemon_summary_screen.h`
- `src/pokemon_summary_screen.c`
- `src/party_menu.c`

確認した points:

- Runtime option は `gSaveBlock2Ptr->optionsTextSpeed`、`optionsBattleSceneOff`、`optionsBattleStyle`、`optionsSound`、`optionsButtonMode`、`optionsWindowFrameType` に保存される。
- battle UI の多くは `include/config/battle.h` の compile-time config による。
- summary screen は `SUMMARY_MODE_*` と `PSS_PAGE_*` を持ち、選出 UI から開く場合は元 party slot と一時 party slot の区別が必要。
- `B_MOVE_REARRANGEMENT_IN_BATTLE` が有効な場合、battle 中 move slot 変更が party data へ反映されるため、選出元 slot へ戻す対象になる。

詳細は `docs/flows/options_status_flow_v15.md` を参照。

### Opponent Party / Randomizer

確認した files:

- `docs/tutorials/how_to_trainer_party_pool.md`
- `include/trainer_pools.h`
- `src/trainer_pools.c`
- `src/data/battle_pool_rules.h`
- `include/data.h`
- `src/battle_main.c`
- `tools/trainerproc/main.c`
- `include/constants/battle_ai.h`

確認した symbols:

- `DoTrainerPartyPool`
- `PickMonFromPool`
- `RandomizePoolIndices`
- `PrunePool`
- `CreateNPCTrainerPartyFromTrainer`
- `CreateNPCTrainerParty`
- `AI_FLAG_RANDOMIZE_PARTY_INDICES`

Trainer Party Pools により、trainer party の pool 選出・並び替え・tag 制御・randomize に近い仕組みが既に存在する。

ただし、通常の `gEnemyParty` は battle init 中に生成されるため、battle 前選出 UI で相手 party preview を正確に出すには、pool / randomize / override を反映した party を battle init 前に安全に得る設計が必要。

詳細は `opponent_party_and_randomizer.md` を参照。

### `.inc` Scripts

確認した files:

- `data/scripts/trainer_battle.inc`
- `data/scripts/trainer_script.inc`
- `data/scripts/gabby_and_ty.inc`
- `data/scripts/trainer_hill.inc`
- `data/scripts/trainer_tower.inc`
- `data/scripts/battle_frontier.inc`
- `data/scripts/cable_club.inc`
- `data/scripts/cable_club_frlg.inc`
- `data/scripts/trainers_frlg.inc`
- battle / selection 関連 symbol を含む `data/maps/**/*.inc`

`data/scripts/*.inc` は全件列挙し、battle / selection 関連 symbol で `data/maps/**/*.inc` を検索した。全 map script の全行レビューは未実施で、必要なら map group 単位で追加 audit する。

詳細は `docs/flows/script_inc_audit_v15.md` を参照。

## Reuse Candidates

| Existing Piece | Can Reuse? | Notes |
|---|---|---|
| `PARTY_MENU_TYPE_CHOOSE_HALF` | Maybe | UI と選出順管理は近い |
| `gSelectedOrderFromParty` | Likely | 元 slot mapping として有用 |
| `CursorCb_Enter` / `CursorCb_NoEntry` | Likely | 選出/解除の基本操作 |
| `Task_ValidateChosenHalfParty` | Maybe | 既存 validation が通常 trainer 用として適切か要確認 |
| `ReducePlayerPartyToSelectedMons` | Maybe | party を詰める処理は近いが、最大数と復元設計に注意 |
| `SavePlayerParty` / `LoadPlayerParty` | Risky | saveblock を scratch 的に使う設計になる |
| Sky Battle `SaveChangesToPlayerParty` pattern | Conceptually | 専用 flag/var 依存のため直接流用は危険 |

## Likely Integration Timing

`gTrainerBattleParameter` が確定した後、`BattleSetup_StartTrainerBattle` が `gBattleTypeFlags` を設定して battle を始める前が候補。

候補:

1. `EventScript_DoTrainerBattle` の `dotrainerbattle` 前に専用選出 special を入れる。
2. `ScrCmd_dotrainerbattle` で選出が必要なら一度 UI へ遷移する。
3. `BattleSetup_StartTrainerBattle` 前に wrapper を挟む。

現時点では 1 が既存 `special` + `waitstate` pattern に近い。ただし global trainer battle script を変えるため影響範囲が広い。

## Single / Double Selection Count

仕様候補:

| Battle Kind | Selection Count |
|---|---:|
| Single trainer battle | 3 |
| Double trainer battle | 4 |

double 判定に関係する確認済み symbol:

- `TRAINER_BATTLE_PARAM.mode`
- `TRAINER_BATTLE_PARAM.isDoubleBattle`
- `GetTrainerBattleType(TRAINER_BATTLE_PARAM.opponentA)`
- `BATTLE_TYPE_DOUBLE`

注意点:

- `gBattleTypeFlags` の確定は `BattleSetup_StartTrainerBattle` 内。
- そのため選出 UI 起動時点で 3/4 を決めるには、`TrainerBattleParameter` と trainer party type から先に判定する helper が必要になる可能性がある。

## Party Restore Requirements

一時 `gPlayerParty` を作る場合、最低限必要な data:

| Data | Purpose |
|---|---|
| original party copy | 非選出 slot を含む元 party 復元 |
| original party count | 復元後 count |
| selected original slot indexes | battle 後状態を戻す mapping |
| selected count | 3 or 4 |
| selection active flag | battle end で復元処理を走らせるか判定 |

battle 後に戻すべき可能性がある状態:

- HP
- status
- PP
- level / EXP
- EV
- friendship
- held item
- moves
- form / species changes
- nickname や misc data は通常変わらない想定だが未確認

## Initial Exclusions

MVP では以下を除外候補にするのが安全。

- Battle Frontier
- cable club / Union Room
- link battle
- wild battle
- Safari / Pyramid / Hill など特殊 battle
- follower partner battle
- two trainers battle
- multi battle
- scripted party temporary changes that already call `SavePlayerParty`

## Open Questions

- `MAX_FRONTIER_PARTY_SIZE` の値と、4 匹選出に使えるかは追加確認が必要。
- 選出 UI 起動前に double battle 判定を完全に再現できるか未確認。
- battle 後の level up / move learn / evolution が完了する timing と、復元処理の正しい placement は未確認。
- `Task_ValidateChosenHalfParty` の duplicate species/item validation を通常 trainer battle では無効にすべきか未決定。
- cancel 時の仕様は未決定。通常 trainer battle で cancel を許すと trainer encounter flow が不自然になる可能性がある。
- battle 開始後の party status summary を 3/4 匹表示へ変えるか、既存 6 slot 表示のままにするか未決定。
- 相手 party preview を pool / randomize 反映済みで出す場合、battle init 前にどの API で生成するか未決定。
- UI option を増やす場合、save data layout 変更を許容するか未決定。
