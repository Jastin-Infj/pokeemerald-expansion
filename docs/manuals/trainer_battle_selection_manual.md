# Trainer Battle Selection Manual

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Implementation branch | `feature/battle-selection-mvp` |
| Code status | Validated branch, not on `master` |
| Primary docs | `docs/features/battle_selection/` |

この manual は、通常 trainer battle 前に player party から battle 参加 Pokémon を
選出する MVP の設定、処理 flow、検証方針をまとめる。

## Current Contract

| Battle kind | Behavior |
|---|---|
| normal single trainer battle | party に eligible Pokémon が 4 匹以上いる場合、3 匹を選出して battle に入る。 |
| normal double trainer battle | party に eligible Pokémon が 5 匹以上いる場合、4 匹を選出して battle に入る。 |
| eligible Pokémon | empty slot、Egg、fainted Pokémon は選出不可。 |
| selection UI | 既存の `PARTY_MENU_TYPE_CHOOSE_HALF` を trainer battle 用 mode で流用する。 |
| Cancel / B | trainer encounter の script 復帰先が曖昧なため無効。 |
| battle end | 選出 Pokémon の battle 後状態を元 slot へ戻し、元の party 順へ復元する。 |

MVP では opponent preview、custom selection UI、battle 中 party status summary の
3/4 slot 化、runtime option menu は扱わない。

## Configuration

設定は build-time config のみ。

```c
#define B_TRAINER_BATTLE_SELECTION TRUE
```

| Value | Result |
|---|---|
| `TRUE` | 通常 trainer battle 前に selection gate を有効化する。 |
| `FALSE` | selection gate は no-op。既存の trainer battle flow を維持する。 |

設定場所:

- `include/config/battle.h`

切り替え後は ROM を rebuild する。

```sh
rtk make -j16 -O all
```

この branch では manual validation のため `TRUE` を default にしている。
最終 integration で default を `TRUE` のまま採用するか、保守的に `FALSE` へ戻すかは
integration branch で決める。

## Save / Flag / Var Policy

MVP では新しい SaveBlock field、saved flag、saved var は使わない。

| State | Storage | Lifetime |
|---|---|---|
| original party copy | `src/trainer_battle_selection.c` の EWRAM state | battle end restore まで |
| selected original slots | same EWRAM state | battle end restore まで |
| existing selection order | `gSelectedOrderFromParty` | selection confirm まで |
| enable / disable | `B_TRAINER_BATTLE_SELECTION` | build-time |

通常 trainer battle の勝敗 flag や script return は既存 flow に任せる。
selection state は save に残さないため、save migration は不要。

runtime option 化する場合は別 slice として扱う。その場合は option menu、save layout、
default migration、config fallback を別途設計する。

## Entry Gate

`BattleSetup_StartTrainerBattle()` で `gBattleTypeFlags` を確定した後、
`TrainerBattleSelection_ShouldOffer()` が selection UI を出すか判定する。

selection UI を出す条件:

- `B_TRAINER_BATTLE_SELECTION == TRUE`
- normal trainer battle
- required count より party count が多い
- eligible Pokémon が required count 以上いる

除外する flow:

- Battle Frontier
- link / recorded battle
- follower partner / multi battle
- two trainers battle
- Pyramid / Trainer Hill
- secret base
- first battle tutorial

## Processing Flow

```text
trainerbattle script
  -> dotrainerbattle
  -> BattleSetup_StartTrainerBattle()
  -> set gBattleTypeFlags
  -> TrainerBattleSelection_ShouldOffer()
      -> false: DoTrainerBattle()
      -> true: TrainerBattleSelection_Begin()
          -> InitChooseHalfPartyForTrainerBattleSelection()
          -> party menu choose-half UI
          -> Task_ValidateChosenHalfParty()
          -> CB2_StartTrainerBattleAfterPartySelection()
          -> CB2_ReturnToField()
          -> FieldCB_StartTrainerBattleAfterPartySelection()
          -> TrainerBattleSelection_StartBattleFromSelection()
          -> DoTrainerBattle()
          -> CreateBattleStartTask()
          -> CB2_InitBattle()
          -> battle
          -> CB2_EndTrainerBattle()
          -> HandleBattleVariantEndParty()
          -> TrainerBattleSelection_RestoreIfActive()
          -> existing trainer battle end flow
```

重要な点:

- party menu の exit callback は毎フレーム呼ばれる `MainCallback`。
- その callback から直接 `DoTrainerBattle()` を呼ぶと battle start task が多重生成される。
- 修正後は `CB2_ReturnToField()` を経由し、`gFieldCallback` で一度だけ battle start を作る。

## Temporary Party Build

selection confirm 後に `TrainerBattleSelection_StartBattleFromSelection()` が実行される。

1. 元 `gPlayerParty[PARTY_SIZE]` を EWRAM state に copy する。
2. `gSelectedOrderFromParty` の 1-based slot を 0-based `selectedSlots` に変換する。
3. 選出 Pokémon を選出順に一時配列へ copy する。
4. `gPlayerParty` を zero clear する。
5. 選出 Pokémon を `gPlayerParty[0..selectedCount-1]` に詰める。
6. `CalculatePlayerPartyCount()` を呼ぶ。
7. 通常 trainer battle を開始する。

## Restore Flow

`CB2_EndTrainerBattle()` の先頭で次の順に復元する。

1. `HandleBattleVariantEndParty()`
2. `TrainerBattleSelection_RestoreIfActive()`
3. existing follower / whiteout / trainer flag flow

restore は、一時 `gPlayerParty[0..selectedCount-1]` の battle 後状態を
`selectedSlots` の元 slot へ copy し、元 party 全体を元順で再構築する。

## Validation

2026-05-09 時点の確認:

| Check | Result |
|---|---|
| `rtk make -j16 -O all` | Pass |
| `rtk make -j16 -O check` | Pass |
| mGBA Live boot/input smoke | Pass |
| user manual single battle | Pass。3 匹選出、battle start、party restore を確認。 |
| user manual double battle | Pass。4 匹選出、battle start、party restore を確認。 |

User manual validation では、通常 battle の主動線は OK。
known cosmetic issue として、battle transition animation 中に player / NPC trainer
sprites が一瞬黒い影のように見える場合がある。進行不能ではなく、battle start と
party restore は正常。

## Merge Policy

`master` へ runtime source を直接入れない。

| Target | Allowed files |
|---|---|
| implementation branch | `src/`, `include/`, `docs/` |
| docs-only master PR | `docs/`, 必要なら `AGENTS.md` |

master に docs を入れる場合は docs-only branch / PR を作る。
source / include / data / tools を含む branch を `master` に直接 merge しない。
