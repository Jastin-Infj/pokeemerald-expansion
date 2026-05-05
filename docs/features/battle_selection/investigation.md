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

## Separate Follow-up Investigations

以下は player party 選出 MVP から切り出して、別件 docs / task として扱う。

| Topic | Why separate | Suggested doc target |
|---|---|---|
| Opponent party preview | pool / randomize / rematch / difficulty を battle init 前に再現する必要があり、`gEnemyParty` 汚染と RNG 消費のリスクがある。 | `docs/features/battle_selection/opponent_party_and_randomizer.md` |
| Trainer Party Pools の通常 trainer 展開 | `.party` DSL、`trainerproc`、pool fallback、AI flag の仕様確認が必要。player party 復元とは別の data pipeline。 | `docs/features/battle_selection/opponent_party_and_randomizer.md` |
| Partygen build integration | generated fragment include は現行 `trainer_rules.mk` の dependency tracking 外。copy-paste MVP 後に Makefile / CI 方針を決める。 | `docs/features/battle_selection/opponent_party_and_randomizer.md` |
| Battle-end mutation ordering | level up / move learn / evolution / held item restore / whiteout は battle end callback 側の順序問題。選出 UI の入口設計とは分ける。 | `docs/features/trainer_battle_aftercare/investigation.md` |
| SaveBlock を使う runtime option | option menu / SaveBlock2 / migration を伴う。選出 default を compile-time で始めるなら MVP から外せる。 | `docs/flows/save_data_flow_v15.md` |
| Battle UI の 3/4 slot 表示 | `CreatePartyStatusSummarySprites` など battle UI 複数箇所に波及する。MVP は既存 6 slot 表示を許容する。 | `docs/flows/battle_ui_flow_v15.md` |
| Battle Frontier / link / multi 対応 | 既存 facility rules と通信 battle flow に入るため、通常 trainer battle の後に個別 audit する。 | feature-specific follow-up doc |

## Restore Timing (CB2_EndTrainerBattle)

`src/battle_setup.c:1428-1494` の `CB2_EndTrainerBattle` を読み、battle 終了後の処理順序を確定した。

```text
DoTrainerBattle()
   ↓ (battle main loop)
gMain.savedCallback = CB2_EndTrainerBattle
   ↓
CB2_EndTrainerBattle:
   1. HandleBattleVariantEndParty()        # Sky Battle 等の party swap 復元 hook
   2. (FollowerNPC) RestorePartyAfterFollowerNPCBattle()
   3. early-rival branch / forfeit branch / defeat branch / win branch
   4. SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic)
        または SetMainCallback2(CB2_WhiteOut)
   ↓ whiteout の場合
CB2_WhiteOut:                              # src/overworld.c:1903
   - 120 frame 待機後に DoWhiteOut()
   ↓
DoWhiteOut():                              # src/overworld.c:390
   - RunScriptImmediately(EventScript_WhiteOut)
   - HealPlayerParty()                     # 全 party 全回復
   - Overworld_ResetStateAfterWhiteOut()
   - SetWarpDestinationToLastHealLocation()
```

`HandleBattleVariantEndParty` の実装 (src/battle_setup.c:1419-1426):

```c
static void HandleBattleVariantEndParty(void)
{
    if (B_FLAG_SKY_BATTLE == 0 || !FlagGet(B_FLAG_SKY_BATTLE))
        return;
    SaveChangesToPlayerParty();   // 参加 mon の battle 後 state を SaveBlock1.playerParty へ書き戻す
    LoadPlayerParty();             // SaveBlock1.playerParty から live gPlayerParty を復元
    FlagClear(B_FLAG_SKY_BATTLE);
}
```

これは battle_selection MVP がそのまま参考にできる pattern。`B_FLAG_SKY_BATTLE` 相当の `B_FLAG_PARTY_SELECTION_ACTIVE` を作り、`SaveChangesToPlayerParty` 相当 (`gSelectedOrderFromParty[i]` を index として書き戻す) と `LoadPlayerParty` を呼ぶ。

### Critical Finding: Restore は whiteout より前

`HandleBattleVariantEndParty` は `CB2_EndTrainerBattle` の最初に走る。つまり:

1. battle 中に選出 3 匹がすべて瀕死。
2. `CB2_EndTrainerBattle` 冒頭で **6 匹 full party へ復元**。
3. 続く defeat 判定で `CB2_WhiteOut` へ遷移。
4. `DoWhiteOut` の `HealPlayerParty()` が **6 匹全員を全回復**。

これは仕様上の選択肢になる。

| Option | 挙動 | 評価 |
|---|---|---|
| MVP 現行案 (whiteout 前に復元) | 全滅後 6 匹全員が pokemon center で healed | Sky Battle と同じ pattern。実装最小。non-選出 mon が「battle 中だった」ことに気付かないが、whiteout は元々 full heal なので破綻しない |
| 復元を whiteout 後に遅延 | 選出 3 匹だけが healed され、非選出 3 匹は元 HP | 実装重 (CB2_WhiteOut からも hook する)。Frontier の選出感に近い |

MVP は **現行案 (Sky Battle 互換)** で進める。

### `SavePlayerParty` / `LoadPlayerParty` の中身

`src/load_save.c:169-195` と `src/pokemon.c:7371-7374`:

```c
void SavePlayerParty(void)
{
    *GetSavedPlayerPartyCount() = gPlayerPartyCount;
    for (i = 0; i < PARTY_SIZE; i++)
        SavePlayerPartyMon(i, &gPlayerParty[i]);
}

void SavePlayerPartyMon(u32 index, struct Pokemon *mon)
{
    gSaveBlock1Ptr->playerParty[index] = *mon;     // !!! これが backup buffer ではなく "本来の slot"
}

void LoadPlayerParty(void)
{
    gPlayerPartyCount = *GetSavedPlayerPartyCount();
    for (i = 0; i < PARTY_SIZE; i++)
        gPlayerParty[i] = *GetSavedPlayerPartyMon(i);
    // ...
}
```

**`SaveBlock1.playerParty[]` は backup 用の別 buffer ではなく、保存される本来の party slot。** `SavePlayerParty()` は live `gPlayerParty` (EWRAM) を SaveBlock1 にコピーするだけ。Sky Battle / battle_selection の用途では、

- battle 開始前に `SavePlayerParty()` (= 元 6 匹を SaveBlock1 へ書く)
- `gPlayerParty` を選出 3 匹に縮約
- battle 中はずっと `gPlayerParty` = 3 匹、`SaveBlock1.playerParty` = 元 6 匹
- battle 後に `SaveChangesToPlayerParty()` で参加 3 匹の最新 state を SaveBlock1 の対応 slot へ戻す
- `LoadPlayerParty()` で SaveBlock1 → `gPlayerParty` に復元

という流れ。SaveBlock1.playerParty が backup の役を兼ねるので、**追加 EWRAM buffer は不要**。

### Risk: Save While Selecting

battle 中に save game (Frontier 等で `SaveGameFrontierLike` を呼ぶ場合) が走ると、`SaveBlock1.playerParty` の現在内容 (= 元 6 匹) が flash に書かれる。つまり battle 中の 3 匹の中間状態は flash に乗らない。これは MVP では問題にならないが、Champions Challenge の party snapshot 設計には大きな制約 → `docs/features/champions_challenge/risks.md` 側に転記が必要。

### Reflection on Open Investigation Queue

`docs/manuals/open_investigation_queue.md` の High Priority に挙げていた「battle selection restore timing」は、本 section で解決。queue 側は本 section へリンクして status を下げる。

## Open Questions

- `MAX_FRONTIER_PARTY_SIZE` の値と、4 匹選出に使えるかは追加確認が必要。
- 選出 UI 起動前に double battle 判定を完全に再現できるか未確認。
- battle 後の level up / move learn / evolution が完了する timing と、復元処理の正しい placement は未確認。
- `Task_ValidateChosenHalfParty` の duplicate species/item validation を通常 trainer battle では無効にすべきか未決定。
- cancel 時の仕様は未決定。通常 trainer battle で cancel を許すと trainer encounter flow が不自然になる可能性がある。
- battle 開始後の party status summary を 3/4 匹表示へ変えるか、既存 6 slot 表示のままにするか未決定。
- 相手 party preview を pool / randomize 反映済みで出す場合、battle init 前にどの API で生成するか未決定。
- UI option を増やす場合、save data layout 変更を許容するか未決定。
