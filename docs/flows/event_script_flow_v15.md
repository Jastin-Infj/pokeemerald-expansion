# Event Script Flow v15

## Purpose

イベントスクリプトがどのように C 側の処理へ渡り、`special` や `trainerbattle` からバトル開始処理へ接続されるかを整理する。

今回のトレーナーバトル前選出機能では、既存のイベントスクリプト進行、`waitstate`、`special`、`trainerbattle` の境界を壊さないことが重要になる。

## Key Files

| File | Role |
|---|---|
| `include/script.h` | `struct ScriptContext`、スクリプト実行 API、`SCREFF_*` 定義 |
| `src/script.c` | `RunScriptCommand`、`ScriptContext_SetupScript`、`ScriptContext_Stop`、`ScriptContext_Enable` |
| `src/scrcmd.c` | `ScrCmd_special`、`ScrCmd_specialvar`、`ScrCmd_trainerbattle`、`ScrCmd_dotrainerbattle` |
| `data/script_cmd_table.inc` | script command opcode から handler への dispatch table |
| `data/event_scripts.s` | `gSpecialVars`、`gSpecials` include、event script data の集約 |
| `data/specials.inc` | `gSpecials` table |
| `include/constants/vars.h` | `VAR_0x8000` 系、`VAR_RESULT`、`VAR_TRAINER_BATTLE_OPPONENT_A` |
| `src/event_data.c` | `gSpecialVar_0x8000` 系、`gSpecialVar_Result`、`GetVarPointer`、`VarGet`、`VarSet` |
| `data/scripts/trainer_battle.inc` | trainer battle 用 event script |
| `asm/macros/event.inc` | `trainerbattle` / `dotrainerbattle` macros |

## Script Command Dispatch

### Core Loop

`src/script.c` の `RunScriptCommand` は、現在の `ScriptContext` から opcode を 1 byte 読み、`ctx->cmdTable[cmdCode]` の handler を呼ぶ。

確認済みの要点:

- `ScriptContext_SetupScript(scriptPtr)` は global script context を初期化し、`gScriptCmdTable` を command table として設定する。
- `RunScriptCommand` は command handler が `TRUE` を返すまで command を進める。
- handler が `FALSE` を返すと、同じ script execution tick 内で次の command へ進む。
- handler が `TRUE` を返すと、そこで script が一旦 yield する。
- `ScriptContext_Stop()` は context status を `CONTEXT_WAITING` にする。
- `ScriptContext_Enable()` は context を再開可能にし、field controls を lock する。

`include/script.h` の `struct ScriptContext` には、`mode`、`scriptPtr`、`nativePtr`、`cmdTable`、`cmdTableEnd`、`waitAfterCallNative` などがある。

### Command Table

`data/script_cmd_table.inc` で確認した関連 opcode:

| Opcode | Macro | Handler | Notes |
|---|---|---|---|
| `0x25` | `SCR_OP_SPECIAL` | `ScrCmd_special` | `special` を実行 |
| `0x26` | `SCR_OP_SPECIALVAR` | `ScrCmd_specialvar` | `special` 戻り値を var へ保存 |
| `0x5c` | `SCR_OP_TRAINERBATTLE` | `ScrCmd_trainerbattle` | trainer battle args を読み込む |
| `0x5d` | `SCR_OP_DOTRAINERBATTLE` | `ScrCmd_dotrainerbattle` | 実際に trainer battle を開始 |
| `0x5e` | `SCR_OP_GOTOPOSTBATTLESCRIPT` | `ScrCmd_gotopostbattlescript` | battle 後 script へ移動 |
| `0x5f` | `SCR_OP_GOTOBEATENSCRIPT` | `ScrCmd_gotobeatenscript` | trainer beaten script へ移動 |

### trainerbattle Command

`src/scrcmd.c` の `ScrCmd_trainerbattle` は以下の流れ。

1. `TrainerBattleLoadArgs(ctx->scriptPtr)` で script data から `gTrainerBattleParameter` を読む。
2. `BattleSetup_ConfigureTrainerBattle(ctx->scriptPtr)` の戻り値で `ctx->scriptPtr` を差し替える。
3. `FALSE` を返し、差し替え先の event script が続けて実行される。

`trainerbattle` command 自体はこの時点では battle を開始しない。実際の開始は後続の `dotrainerbattle` command で行われる。

### dotrainerbattle Command

`ScrCmd_dotrainerbattle` は `BattleSetup_StartTrainerBattle()` を呼び、`TRUE` を返す。

`BattleSetup_StartTrainerBattle()` 内では `gMain.savedCallback = CB2_EndTrainerBattle` が設定され、`DoTrainerBattle()` 経由で battle start task が作られる。その後 `ScriptContext_Stop()` が呼ばれるため、field script は battle 終了後まで停止する。

### waitstate

`ScrCmd_waitstate` は `ScriptContext_Stop()` を呼んで `TRUE` を返す。

既存の使用例:

- `data/scripts/trainer_battle.inc` の `EventScript_StartTrainerApproach`
  - `special DoTrainerApproach`
  - `waitstate`
  - `src/trainer_see.c` の `Task_EndTrainerApproach` が `ScriptContext_Enable()` を呼んで再開する。
- `data/scripts/trainer_battle.inc` の `EventScript_ShowTrainerIntroMsg`
  - `special ShowTrainerIntroSpeech`
  - `waitmessage`
  - `waitstate`
- `ChooseHalfPartyForBattle` 系 scripts
  - `special ChooseHalfPartyForBattle`
  - `waitstate`
  - party menu 側 callback が `CB2_ReturnToFieldContinueScriptPlayMapMusic` へ戻す。

## special Flow

### Dispatch

`src/scrcmd.c`:

- `ScrCmd_special`
  - script から halfword index を読む。
  - `gSpecials[index]()` を呼ぶ。
  - 戻り値は script var へ保存しない。
- `ScrCmd_specialvar`
  - script から var id と special index を読む。
  - `GetVarPointer(varId)` で保存先を解決する。
  - `*ptr = gSpecials[index]();` で戻り値を保存する。

### Definition Locations

| Symbol / Data | Location | Notes |
|---|---|---|
| `gSpecials` | `data/specials.inc` | `def_special` による special table |
| `gSpecialVars` | `data/event_scripts.s` | special vars への pointer table |
| `gSpecialVar_Result` | `src/event_data.c` | `VAR_RESULT` の実体 |
| `VAR_RESULT` | `include/constants/vars.h` | `0x800D` |
| `GetVarPointer` | `src/event_data.c` | saved var と special var を分岐 |

`data/event_scripts.s` の `gSpecialVars` は、`VAR_0x8000` から `VAR_RESULT` などを `gSpecialVar_*` へ対応させる。`VAR_TRAINER_BATTLE_OPPONENT_A` は `gTrainerBattleParameter.params.opponentA` への alias として定義されている。

### Relevant Specials

今回の調査で確認した special:

| Special | Location | Notes |
|---|---|---|
| `ChooseHalfPartyForBattle` | `data/specials.inc` / `src/script_pokemon_util.c` | 既存の選出 UI 起動 special |
| `ChoosePartyForBattleFrontier` | `data/specials.inc` / `src/script_pokemon_util.c` | Battle Frontier 用の選出 UI 起動 special |
| `ReducePlayerPartyToSelectedMons` | `data/specials.inc` / `src/script_pokemon_util.c` | `gSelectedOrderFromParty` に基づき `gPlayerParty` を詰める |
| `DoTrainerApproach` | `data/specials.inc` / `src/trainer_see.c` | trainer 接近 task を開始 |
| `TryPrepareSecondApproachingTrainer` | `data/specials.inc` / `src/trainer_see.c` | 2 人 trainer 接近の切り替え |
| `GetTrainerFlag` | `data/specials.inc` | trainer 既戦闘判定で使用。実体場所は追加調査候補 |
| `GetTrainerBattleMode` | `data/specials.inc` | battle 後 script 分岐で使用。実体場所は追加調査候補 |

## Trainer Battle Script Flow

`data/scripts/trainer_battle.inc` では、`trainerbattle` command が差し替えた先の script が進む。

代表的な通常 trainer flow:

1. `EventScript_TryDoNormalTrainerBattle`
2. `specialvar VAR_RESULT, GetTrainerFlag`
3. 既戦闘なら `gotopostbattlescript`
4. 未戦闘なら intro text 表示
5. `EventScript_DoTrainerBattle`
6. `dotrainerbattle`
7. battle 終了後、`specialvar VAR_RESULT, GetTrainerBattleMode`
8. `gotobeatenscript`
9. `releaseall`
10. `end`

`dotrainerbattle` の時点で C 側の battle setup へ入るため、トレーナーバトル前選出を差し込むなら、この前後のどちらで script を止めるかが重要になる。

## Relevant Flowchart

```mermaid
flowchart TD
    A[Event Script] --> B[RunScriptCommand]
    B --> C[gScriptCmdTable]
    C --> D{Command}
    D --> E[Normal Handler]
    D --> F[ScrCmd_special]
    D --> G[ScrCmd_specialvar]
    D --> H[ScrCmd_trainerbattle]
    D --> I[ScrCmd_dotrainerbattle]
    D --> J[ScrCmd_waitstate]

    F --> K[gSpecials[index]()]
    G --> L[GetVarPointer(varId)]
    L --> K
    K --> M[Return to Script or Start Async UI]

    H --> N[TrainerBattleLoadArgs]
    N --> O[BattleSetup_ConfigureTrainerBattle]
    O --> P[Trainer Battle EventScript]

    I --> Q[BattleSetup_StartTrainerBattle]
    Q --> R[DoTrainerBattle]
    R --> S[Battle]
    S --> T[gMain.savedCallback]
    T --> U[CB2_EndTrainerBattle]
    U --> V[Return to Field Script]

    J --> W[ScriptContext_Stop]
    W --> X[Async Task or Callback]
    X --> Y[ScriptContext_Enable or ReturnToFieldContinueScript]
```

## Notes for Battle Selection

- `trainerbattle` command は battle を直接開始しないため、`gTrainerBattleParameter` を読んだ後、`dotrainerbattle` 前に選出を挟める余地がある。
- 既存の `ChooseHalfPartyForBattle` は `special` + `waitstate` 型の async UI として動くため、event script に挿入する設計とは相性がよい可能性がある。
- ただし、通常 trainer battle scripts へ単純に `special ChooseHalfPartyForBattle` を追加すると、Frontier / cable club / Steven multi battle 用の global state と衝突する可能性がある。
- `gSpecialVar_Result` は多数の script 分岐で共有されるため、選出キャンセルや成功結果を入れる場合は、直後の script command が何を期待しているかを明確にする必要がある。

## Open Questions

- `GetTrainerFlag`、`GetTrainerBattleMode`、`ShowTrainerIntroSpeech` の実体関数の詳細は未確認。
- `RunScriptImmediatelyUntilEffect` 系が `SCREFF_TRAINERBATTLE` を検出する具体的な effect 設定箇所は追加調査が必要。
- 通常 trainer battle script に選出 UI を追加する場合、全 trainerbattle mode に共通で入れるべきか、特定 mode だけに限定すべきか未決定。
- `special` の effect instrumentation は `data/specials.inc` の `def_special` macro に含まれるが、今回の調査では詳細な利用箇所までは追っていない。
