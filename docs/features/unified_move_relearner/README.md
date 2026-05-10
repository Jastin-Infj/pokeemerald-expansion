# Unified Move Relearner

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `8502839610`; `git describe` = `expansion/1.15.2-40-g8502839610` |
| Code status | Planned; docs-only feature log |
| Provenance | User request and local code/docs review |

## Goal

Move Relearner を「レベル技 / egg move / TM / tutor」を別メニューで選ばせる
形式から、対象 Pokemon が覚えられる全候補を 1 つの list で見られる形式へ寄せる。

要求の中心は次の通り。

- level に関係なく、level-up learnset の全技を思い出せる。
- egg move を同じ list に混ぜる。
- TM/HM 互換技を同じ list に混ぜる。
- 将来 tutor move も同じ list に混ぜられる余地を残す。
- 重複 move は 1 回だけ表示する。
- 現在覚えている 4 技は候補から除外するか、少なくとも選択時に二重習得にならないようにする。
- UI は既存 Move Relearner を流用してよいが、候補生成と表示上限を安全にする。

## Current Baseline

既存 flow は [Move Relearner Flow](../../flows/move_relearner_flow_v15.md) が owning
調査 docs。

現在の入口:

| Area | Current owner |
|---|---|
| config | `include/config/summary_screen.h` |
| state enum / cap | `include/constants/move_relearner.h` |
| UI / candidate generation | `src/move_relearner.c` |
| description list | `src/menu_specialized.c` |
| summary prompt / START flow | `src/pokemon_summary_screen.c` |
| party menu action | `src/party_menu.c` |
| script flow | `data/scripts/move_relearner.inc` |
| TM registry | `include/constants/tms_hms.h`, `src/item.c`, `src/data/pokemon/teachable_learnsets.h` |

既存 config には近い部品がある。

| Config | Relevance |
|---|---|
| `P_ENABLE_ALL_LEVEL_UP_MOVES` | level に関係なく level-up moves を候補にする。 |
| `P_PRE_EVO_MOVES` | pre-evolution level-up moves を候補にする。 |
| `P_TM_MOVES_RELEARNER` | TM move relearner を有効化する。 |
| `P_ENABLE_ALL_TM_MOVES` | bag 所持に関係なく compatible TM moves を候補にする。 |
| `P_ENABLE_MOVE_RELEARNERS` | egg / TM / tutor relearners をまとめて有効化する。 |
| `P_FLAG_EGG_MOVES`, `P_FLAG_TUTOR_MOVES` | script 側の解禁条件。 |

## Proposed Contract

MVP は新しい unified mode を追加し、既存 state は互換用に残す。

| Case | Behavior |
|---|---|
| unified mode off | 既存 Move Relearner flow のまま。 |
| unified mode on | level-up / egg / TM / tutor 候補を 1 list にまとめる。 |
| level-up moves | `P_ENABLE_ALL_LEVEL_UP_MOVES` 相当で level 制限を外す。 |
| TM moves | `P_ENABLE_ALL_TM_MOVES` 相当で compatible TM/HM を候補にするか、bag 所持制限を残すかを config で分ける。 |
| egg moves | species の egg move table を候補に入れる。 |
| tutor moves | 初回 MVP では optional。入れる場合は tutor generated data と解禁 flag を確認する。 |
| duplicate source | 同じ move が複数 source に出ても list は 1 entry。 |
| move source display | 初回 MVP は表示しなくてよい。後続で `Lv` / `Egg` / `TM` / `Tutor` badge を検討する。 |
| too many candidates | `MAX_RELEARNER_MOVES` 超過で memory overwrite しない。list cap と overflow message / truncation policy を決める。 |

## Impact Surface

| File / area | Impact |
|---|---|
| `src/move_relearner.c` | `CreateLearnableMovesList()` と各 `GetRelearner*Moves()` を unified candidate builder へ寄せる。 |
| `include/constants/move_relearner.h` | `MOVE_RELEARNER_*` に unified state を足すか、config-only path にするか決める。`MAX_RELEARNER_MOVES` を再評価する。 |
| `include/config/summary_screen.h` | unified mode、all-level、all-TM、source inclusion の build-time config が必要。 |
| `data/scripts/move_relearner.inc` | dynmulti の category choice を出すか、unified 入口へ直行するか決める。 |
| `src/pokemon_summary_screen.c` | START prompt と L/R state cycling が category 前提なので、unified mode の copy / state 表示が必要。 |
| `src/menu_specialized.c` | source badge や long list 表示を足す場合に影響。 |
| `src/data/pokemon/level_up_learnsets/` | source data。level 制限を外すだけなら変更不要。 |
| `src/data/pokemon/egg_moves.h` | egg candidates。species / form / pre-evo policy の確認が必要。 |
| `src/data/pokemon/teachable_learnsets.h` | TM/HM compatibility。generated data なので直接編集しない方針を確認する。 |
| `include/constants/tms_hms.h` / `src/item.c` | TM/HM registry。TM 追加や virtual ownership と連動する。 |

## Implementation Notes

候補 builder は source ごとに append するより、dedupe helper を中心にする方が安全。

```text
BuildUnifiedRelearnerMoves(mon)
  -> AppendLevelUpMoves(allLevels = TRUE)
  -> AppendEggMoves(if enabled)
  -> AppendTMMoves(if enabled / owned policy)
  -> AppendTutorMoves(if enabled)
  -> RemoveAlreadyKnownMoves()
  -> Sort if P_SORT_MOVES
  -> Fail cleanly if candidate count exceeds cap
```

`MAX_RELEARNER_MOVES 60` は Mew + all machines + tutor 混在では足りない可能性がある。
単純に増やすと EWRAM / menu item buffer が増えるため、候補数 cap、paging、または
dynamic allocation のどれを採るかを実装前に決める。

## Risks

| Risk | Severity | Notes |
|---|---|---|
| Candidate overflow | High | unified list は現在の 60 cap を超えやすい。最初に guard を入れる。 |
| TM ownership policy mismatch | Medium | `P_ENABLE_ALL_TM_MOVES` を使うと bag 所持や TM shop progression の価値が変わる。 |
| Hidden source confusion | Medium | 1 list 化すると、なぜ覚えられるかが見えにくい。必要なら source badge を後続で足す。 |
| Summary state cycling | Medium | 既存 summary prompt は relearner state 切替を持つため、unified mode では L/R 表示を整理する。 |
| Tutor / egg unlock flags | Medium | script の flag 解禁を無視すると、既存 progression と食い違う。 |
| TM expansion interaction | Medium | 250+ TM や virtual TM ownership と同時にやると影響が広がる。 |

## Validation Plan

Minimum local checks after implementation:

- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- focused `rtk make -j16 -O check` if candidate helpers get test coverage
- mGBA Live route from a party / debug menu Pokemon into Move Relearner

Manual cases:

- A low-level Pokemon can learn a high-level level-up move.
- A Pokemon with egg moves shows those egg moves in the same list.
- A TM-compatible move appears according to the selected TM ownership policy.
- A move present in multiple sources appears once.
- Already-known moves do not create duplicate move slots.
- Cancel returns to script / summary / party menu correctly.
- A species with many TM/tutor options does not overflow or corrupt the list.

## Open Questions

- Should unified mode ignore Heart Scale / cost, or keep the existing script cost after successful learning?
- Should TM moves require bag ownership, global TM registry ownership, or only compatibility?
- Should tutor moves be included in MVP, or staged after level-up / egg / TM are stable?
- Should source badges be mandatory before implementation, or deferred UI polish?
- Should `MAX_RELEARNER_MOVES` be raised, paged, or replaced with dynamic allocation?
