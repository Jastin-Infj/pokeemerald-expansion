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
