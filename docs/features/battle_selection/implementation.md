# Trainer Battle Party Selection Implementation

## Branch

| Field | Value |
|---|---|
| Implementation branch | `feature/battle-selection-mvp` |
| Baseline | current `master` worktree baseline |
| Related prototype | `feature/party-select-ui` |
| Prototype handling | 参考のみ。古い `vanilla/v14_1` 系 branch なので直接 merge / cherry-pick しない。 |

## Summary

通常 trainer battle の開始直前に player party selection を挟む MVP を実装した。
専用 UI は作らず、既存の `PARTY_MENU_TYPE_CHOOSE_HALF` と
`gSelectedOrderFromParty` を trainer battle 用 mode で再利用する。

single trainer battle は 3 匹、double trainer battle は 4 匹を選出する。
battle 中は選出順に詰めた一時 `gPlayerParty` だけを使い、battle end で
選出 Pokémon の battle 後状態を元 slot に戻してから、元の party 順へ復元する。

## Code Changes

| File | Change |
|---|---|
| `include/config/battle.h` | `B_TRAINER_BATTLE_SELECTION` を追加。この branch では `TRUE`。 |
| `include/party_menu.h` / `src/party_menu.c` | trainer battle selection 用の choose-half entrypoint と専用 validation mode を追加。 |
| `include/trainer_battle_selection.h` / `src/trainer_battle_selection.c` | 元 party 保存、選出 party 構築、battle end restore の state helper を追加。 |
| `src/battle_setup.c` | 通常 trainer battle flow に selection gate を追加し、`CB2_EndTrainerBattle` で restore を呼ぶ。 |

## Runtime Gate

`TrainerBattleSelection_ShouldOffer()` は次を満たす場合だけ selection UI を開く。

| Condition | Policy |
|---|---|
| `B_TRAINER_BATTLE_SELECTION` | `TRUE` の時だけ有効。 |
| battle type | normal trainer battle のみ。 |
| excluded battle flags | link、Frontier、multi、partner、two opponents、Pyramid、Trainer Hill、secret base、recorded battle を除外。 |
| first battle | tutorial / first battle は除外。 |
| party size | required count 以下なら UI を出さない。 |
| eligible count | egg / fainted / empty を除いて required count 未満なら UI を出さない。 |
| Cancel / B button | trainer encounter の script 復帰先が曖昧なため、selection 中は無効。 |

## Restore Order

`CB2_EndTrainerBattle()` の先頭で次の順序にした。

1. `HandleBattleVariantEndParty()`
2. `TrainerBattleSelection_RestoreIfActive()`
3. existing follower / whiteout / trainer flag flow

Sky Battle など既存 variant restore が一時 party を元 party 側へ戻す可能性があるため、
trainer battle selection restore はその後に置く。

## Validation

2026-05-09 に `feature/battle-selection-mvp` で実行。

| Command / Check | Result | Notes |
|---|---|---|
| `rtk git diff --check` | Pass | whitespace check。 |
| `rtk make -j16 -O all` | Pass | `B_TRAINER_BATTLE_SELECTION=TRUE` で通常 ROM build。linker RWX warning は既存。 |
| `rtk make -j16 -O check` | Pass | test runner build warning と linker RWX warning は既存。 |
| mGBA Live smoke | Pass | session `codex-battle-selection-smoke-20260509c`。script-capable wrapper `/home/jastin/.local/bin/mgba-qt` で boot、Lua START/A input、New Game / Option menu screenshot。 |
| mGBA cleanup | Pass | `mgba-live-cli status --all` returned `[]`。 |

Direct selection-screen runtime validation は未実施。prepared save / savestate が無く、
new-game setup から対象 trainer まで進める手順をこの turn では作っていないため。

## Remaining Manual Checks

| Check | Expected |
|---|---|
| single trainer with 6 eligible mons | selection UI が出て 3 匹選出後に single battle が始まる。 |
| double trainer with 6 eligible mons | selection UI が出て 4 匹選出後に double battle が始まる。 |
| party restore | slot 2/5/6 など非連続 selection 後、battle 後に元 party 順へ戻る。 |
| HP / PP / status / level changes | selected original slot に battle 後状態が反映される。 |
| Cancel behavior | B / Cancel で selection UI から抜けず、encounter を中断しない。 |
| excluded flows | Frontier / link / follower partner / two trainers / Pyramid / Hill では selection UI が出ない。 |

## Merge Handoff Notes

`master` へ runtime source を直接入れない。最終統合時は current `master` から
fresh integration branch を切り、必要なら `B_TRAINER_BATTLE_SELECTION` の default
を統合方針に合わせて決める。

docs-only master update を行う場合は、この document と `test_plan.md` などの
Markdown だけを cherry-pick / reapply する。`src/`、`include/`、generated output、
ROM は master docs PR に含めない。
