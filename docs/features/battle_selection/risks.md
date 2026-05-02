# Trainer Battle Party Selection Risks

## Risk Summary

| Risk | Severity | Affected Symbols / Files | Notes |
|---|---|---|---|
| 元 party を失う | Very High | `gPlayerParty`, `SavePlayerParty`, `LoadPlayerParty`, `ReducePlayerPartyToSelectedMons` | 一時 party 構築前に完全な backup が必要 |
| battle 後状態が消える | Very High | `gPlayerParty`, `CB2_EndTrainerBattle`, `HandleBattleVariantEndParty` | `LoadPlayerParty` だけでは battle 中の変化を上書きする可能性 |
| callback chain の破損 | High | `gMain.savedCallback`, `CB2_EndTrainerBattle`, party menu callbacks | party menu と battle end が同じ callback field を使う |
| script waitstate softlock | High | `ScriptContext_Stop`, `ScriptContext_Enable`, `CB2_ReturnToFieldContinueScriptPlayMapMusic` | UI 終了時に script を正しく再開できないと停止する |
| double 判定ずれ | High | `TRAINER_BATTLE_PARAM`, `GetTrainerBattleType`, `gBattleTypeFlags` | 選出数 3/4 を間違えると battle party が不正になる |
| existing facilities 破壊 | High | `VAR_FRONTIER_FACILITY`, `gSelectedOrderFromParty`, `frontier_util.c` | choose half は Frontier / cable club でも使用中 |
| 4 匹選出制限 | Medium | `MAX_FRONTIER_PARTY_SIZE`, `gSelectedOrderFromParty` | double battle 4 匹選出に既存配列長が足りるか確認必要 |
| duplicate validation が不適切 | Medium | `Task_ValidateChosenHalfParty`, `CheckBattleEntriesAndGetMessage` | Frontier ルールが通常 trainer battle に混ざる可能性 |
| cancel handling | Medium | `CB2_ReturnFromChooseHalfParty`, `gSpecialVar_Result` | 通常 trainer encounter で cancel をどう扱うか未定 |
| special vars 汚染 | Medium | `gSpecialVar_Result`, `gSpecialVar_0x8004`, `gSpecialVar_0x8005` | trainer battle scripts も `VAR_RESULT` を使う |
| battle outcome 分岐漏れ | High | `gBattleOutcome`, `CB2_EndTrainerBattle` | 勝利以外でも復元が必要 |
| form/evolution/move learn | Medium | battle end / evolution flow | 状態反映 timing が未確認 |
| battle UI 表示ずれ | High | `CreatePartyStatusSummarySprites`, `CreateBattlerHealthboxSprites`, `gPlayerPartyCount` | 選出後 party count / empty slot が party status summary に影響 |
| move reorder 反映漏れ | Medium | `B_MOVE_REARRANGEMENT_IN_BATTLE`, `HandleMoveSwitching`, `GetBattlerMon` | battle 中に move order が変わる場合、元 slot へ戻す必要 |
| opponent preview 不一致 | High | `CreateNPCTrainerPartyFromTrainer`, `DoTrainerPartyPool`, `gEnemyParty` | pool / randomize / override 反映前の party を表示すると本戦とずれる |
| RNG 二重消費 | High | `DoTrainerPartyPool`, `RandomizePoolIndices`, `Random32` | preview 用生成で本戦と違う party になる可能性 |
| option save layout 変更 | Medium | `struct SaveBlock2`, `src/option_menu.c`, `SetDefaultOptions` | runtime UI option を増やす場合に save 互換性が絡む |
| upstream merge conflict | Medium | `src/battle_setup.c`, `src/party_menu.c`, `src/script_pokemon_util.c` | v15.x 更新で変わりやすい領域 |

## Details

### Party Restore

`ReducePlayerPartyToSelectedMons` は `gPlayerParty` を選出順に詰めるが、元 party の保存は行わない。Battle Frontier scripts では `SavePlayerParty` などと組み合わせている。

通常 trainer battle では、battle 後の選出 Pokémon の状態を元 slot へ戻す必要があるため、単純な `SavePlayerParty` -> `ReducePlayerPartyToSelectedMons` -> `LoadPlayerParty` では不十分な可能性が高い。

### Callback Chain

`ChooseHalfPartyForBattle` は `gMain.savedCallback` を party menu 終了 callback に使う。一方で trainer battle 開始時も `BattleSetup_StartTrainerBattle` が `gMain.savedCallback = CB2_EndTrainerBattle` を設定する。

同じ field を複数段階で使うため、選出 UI を battle 前に挟む場合は callback を退避するか、専用の state machine を作る必要がある。

### Existing Choose Half Rules

既存 choose half は facility 種別により:

- 選出数
- level cap
- fainted Pokémon の扱い
- species / held item duplicate validation
- 表示 message

が変わる。

通常 trainer battle 用に使う場合、Frontier ルールが混ざらないように専用 mode を検討する。

### Battle Type

`BATTLE_TYPE_DOUBLE` は `BattleSetup_StartTrainerBattle` 内で設定される。選出 UI はその前に起動する可能性が高いため、`gBattleTypeFlags` だけに依存して選出数を決める設計は危険。

`TRAINER_BATTLE_PARAM.mode` や `GetTrainerBattleType(TRAINER_BATTLE_PARAM.opponentA)` から同等の判定を行う必要がある可能性がある。

### Battle UI

`CreatePartyStatusSummarySprites` は `PARTY_SIZE` 分の ball tray を扱う。選出後に `gPlayerParty` の残り slot を空にする場合、battle 中 UI は「選出していない slot」を empty として表示する可能性がある。

この見え方を変えるには `src/battle_interface.c` の party status summary と、場合によっては healthbox / battle controller 側も触る必要があるため、MVP では既存 UI のまま許容する案が安全。

### Opponent Party Preview

Trainer Party Pools は `DoTrainerPartyPool` と `CreateNPCTrainerPartyFromTrainer` により battle init 中に反映される。battle 前 UI で相手 party を見せる場合、同じ pool / randomize / override 結果を battle 前に得る必要がある。

preview のために party を二重生成すると RNG 消費や `gEnemyParty` 汚染が起きる可能性がある。

## Mitigation Ideas

| Risk | Mitigation |
|---|---|
| 元 party 消失 | 専用 EWRAM state に `struct Pokemon originalParty[PARTY_SIZE]` を保存 |
| battle 後状態消失 | 復元前に一時 party の selected mons を元 slot へ copy |
| callback 破損 | `gMain.savedCallback` wrapper を明確にし、active flag を持つ |
| script softlock | `special` + `waitstate` pattern を既存に合わせ、return callback を限定 |
| double 判定ずれ | battle setup の判定 helper を共通化または同じ条件を文書化 |
| facility 破壊 | Frontier / cable / Union Room の scripts では feature を無効化 |
| validation 不一致 | 通常 trainer battle 専用 validation path を用意 |
| battle UI 表示ずれ | MVP では battle 開始後 UI を変更せず、3/4 匹 selected party + empty slot 表示を許容するか仕様化 |
| opponent preview 不一致 | MVP から除外し、後続 phase で non-mutating preview helper を設計 |
| RNG 二重消費 | consistent seed か preview/battle 共通の generated party cache を検討 |
| option save layout | 最初は compile-time config に留め、runtime option は save migration 方針確定後に実施 |
| upstream conflict | 差し込み file を最小化し、`docs/upgrades` の checklist で毎回確認 |

## Open Questions

- `MAX_FRONTIER_PARTY_SIZE` が double 4 匹選出に十分か未確認。
- battle 後 evolution / move learn のタイミングと復元タイミングの相互作用は未確認。
- whiteout 時の復元 ordering は未確認。
- cancel を禁止する場合、既存 party menu の B button behavior をどう変えるべきか未決定。
- battle 中 party status summary の 6 slot 表示を仕様として受け入れるか未決定。
- 相手 party preview で trainer pool の randomize を正確に再現する方法は未決定。
