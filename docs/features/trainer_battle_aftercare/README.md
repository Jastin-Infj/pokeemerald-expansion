# Trainer Battle Aftercare / Forced Release

Status: Investigating
Code status: No code changes

## Goal

通常 trainer battle 終了後に、以下のような独自処理を安全に追加できるか調査する。

- trainer battle 後に party を回復する。
- 全滅時に whiteout せず field へ戻す。
- 全滅時に手持ち Pokemon を強制 release する rogue-like / Nuzlocke-style system を作る。
- battle selection と併用する場合、一時 `gPlayerParty` から元 slot へ状態反映した後に処理できるようにする。

## Primary Docs

- `docs/flows/battle_start_end_flow_v15.md`
- `docs/features/trainer_battle_aftercare/investigation.md`
- `docs/features/trainer_battle_aftercare/mvp_plan.md`
- `docs/features/trainer_battle_aftercare/risks.md`
- `docs/features/trainer_battle_aftercare/test_plan.md`

## Current Conclusion

最重要 hook は `src/battle_setup.c` の `CB2_EndTrainerBattle`。ここで `B_FLAG_NO_WHITEOUT`、`IsPlayerDefeated(gBattleOutcome)`, `NoAliveMonsForPlayer()`, `HealPlayerParty()`, `CB2_WhiteOut`, `CB2_ReturnToFieldContinueScriptPlayMapMusic` が分岐する。

ただし、PC の release UI は `src/pokemon_storage_system.c` 内の static task / static helper に強く依存しているため、battle end からそのまま呼ぶ設計は危険。強制 release 用には、将来 public helper を切り出すか、専用の post-battle state machine を作る方が安全。

## Cross-Feature Guard Contract

Champions partygen の現行 branch は、`src/data/trainers.party` の既存
trainer block を置き換えるだけで、ROM runtime state はまだ持たない。
そのため `TRAINER_SIDNEY`、`TRAINER_PHOEBE`、`TRAINER_GLACIA`、
`TRAINER_DRAKE`、`TRAINER_WALLACE` は engine から見ると通常 trainer
battle と同じ `CB2_EndTrainerBattle` flow に入る。

初回実装では、巨大な条件式を `CB2_EndTrainerBattle` に直接広げない。
代わりに aftercare 専用 helper に入口を集約する。

```c
static bool32 TrainerBattleAftercare_ShouldApply(void);
static void TrainerBattleAftercare_Apply(void);
```

判定 helper の初期 contract:

| Case | Behavior |
|---|---|
| feature config が off | 何もしない。既存挙動を完全維持する。 |
| 通常 trainer battle | heal-only MVP の対象候補。default は off。 |
| Champions partygen trainer | runtime flag が無い間は通常 trainer battle と同じ扱い。 |
| 将来 `ChampionsChallenge_IsActive()` が true | 通常 aftercare ではなく challenge aftercare policy に渡す。 |
| Battle selection が active | 選出 party を元 slot へ復元してから aftercare を適用する。 |
| Frontier / Pyramid / Trainer Hill / link / recorded link | MVP では除外する。 |
| Secret Base / early rival / follower partner | 既存の専用分岐を優先し、MVP では除外する。 |

`partygen_owned` や `champions_challenge` tag は tool / audit log 用で、
ROM 側の runtime guard としては使わない。trainer ID 固定リストで
Champions を判定する案も、通常 Elite Four を partygen 管理 trainer として
使う期間と将来の challenge facility 期間が混ざるため、MVP では採用しない。
