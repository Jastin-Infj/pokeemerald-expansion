# Unified Move Relearner

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-16 |
| Baseline | `feature/unified-move-relearner`; current `master` documentation overlay |
| Code status | Planned; implementation branch active |
| Provenance | User request, local code/docs review, and Gen I-IX TM/tutor inventory audit |

## Goal

Move Relearner を「レベル技 / egg move / TM / tutor」を別メニューで選ばせる
形式から、対象 Pokemon が覚えられる全候補を 1 つの list で見られる形式へ寄せる。

要求の中心は次の通り。

- level に関係なく、level-up learnset の全技を思い出せる。
- egg move を同じ list に混ぜる。
- TM/HM 互換技を同じ list に混ぜる。
- Tutor / Battle Tower / Battle Frontier 系の教え技も同じ候補群で扱う。
- Event / XD purification / Cherish Ball distribution 系の特殊技を別 source として後から足せる。
- 同じ move が TM と tutor の両方にある場合、source ごとに別 entry として残す。
- TM item 自体は増やさず、virtual TM candidate pool として扱えるようにする。
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
| unified mode on | level-up / egg / TM / tutor 候補を同じ UI flow で扱う。 |
| level-up moves | `P_ENABLE_ALL_LEVEL_UP_MOVES` 相当で level 制限を外す。 |
| TM moves | item registry を増やさず、Gen I-IX の compatible TM/HM/TR moves を virtual candidate pool として扱う。 |
| egg moves | species の egg move table を候補に入れる。 |
| tutor moves | FRLG / Emerald Battle Frontier / XD / later generation tutor moves を generated source として扱う。 |
| special moves | Event / XD purification / distribution-only moves を external JSON source として扱う。 |
| duplicate source | 同じ move が複数 source に出た場合、`move + source` 単位で保持する。 |
| move source display | `Lv` / `Egg` / `TM` / `Tutor` の短い badge を表示し、候補の由来を隠さない。 |
| too many candidates | `MAX_RELEARNER_MOVES` 超過で memory overwrite しない。Mew stress count は 488 source entries なので paging/chunking が必須。 |

## TM/Tutor Inventory

Full extracted inventory: [TM/Tutor Inventory](tm_tutor_inventory.md).
Special move source design: [Special Move Candidates](special_move_candidates.md).

Mew を stress case にすると、local porymoves source からの抽出結果は次の通り。

| Bucket | Count |
|---|---:|
| Historical TM unique moves | 345 |
| Tutor / Tower unique moves | 124 |
| TM / tutor overlap moves | 100 |
| Preserved TM + tutor source entries | 469 |
| Level + TM + tutor source entries | 488 |

このため、既存の `MAX_RELEARNER_MOVES 60` を増やすだけでは UX が破綻しやすい。
候補 storage は 500 件以上を安全に持てるようにし、表示は source tabs または
50-60 件単位の chunk / paging で扱う。

Egg moves are not the primary storage risk. In the local porymoves sources,
Mew has 0 egg moves. The highest single-game egg count found in the audit is
20, and the highest all-generation union is 23. Egg moves should still be
included, but the 600+ candidate target is driven by historical TM and tutor
source entries.

Future generation planning note: 2026-05-16 時点の audit は Gen I-IX ベースだが、
将来の Gen X 以降で TM / tutor / relearner 候補がさらに増える前提で設計する。
Mew stress count 488 に対して 512 cap は余裕が薄いため、implementation target は
600 件以上、可能なら 640 件程度を安全に保持できる candidate storage とする。

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
| planned special move JSON | Event / XD / distribution-only candidates。upstream learnset 更新で消えない local overlay として扱う。 |

## Implementation Notes

候補 builder は `move + source` を entry identity にする。すでに覚えている 4 技は
source に関係なく候補から除外するが、未習得の同一 move が TM と tutor にある場合は
両方を残す。

```text
BuildUnifiedRelearnerMoves(mon)
  -> AppendLevelUpMoves(allLevels = TRUE)
  -> AppendEggMoves(if enabled)
  -> AppendVirtualTMMoves(if enabled / story unlock policy)
  -> AppendTutorMoves(if enabled / story unlock policy)
  -> AppendSpecialMoves(if enabled / special unlock policy)
  -> RemoveAlreadyKnownMoves()
  -> Keep source entries distinct
  -> Page or tab the visible list
  -> Fail cleanly if candidate storage exceeds cap
```

`numMenuChoices` は現在 `u8` 系の扱いなので、候補数が 255 を超える unified mode では
`u16` 化または visible page count との分離が必要。

## Risks

| Risk | Severity | Notes |
|---|---|---|
| Candidate overflow | High | Mew は Gen I-IX source entry で 488 件。将来世代追加を見込み、600 件以上を安全に扱う。60 cap と `u8` count はそのまま使えない。 |
| TM ownership policy mismatch | Medium | `P_ENABLE_ALL_TM_MOVES` を直接使うと story progression の価値が変わる。virtual TM unlock policy を別に持つ。 |
| Hidden source confusion | Medium | TM/tutor 重複を残すため、source badge なしだと同名 move が混乱しやすい。 |
| Summary state cycling | Medium | 既存 summary prompt は relearner state 切替を持つため、unified mode では L/R 表示を整理する。 |
| Tutor / egg unlock flags | Medium | script の flag 解禁を無視すると、既存 progression と食い違う。 |
| TM expansion interaction | Medium | 250+ TM や virtual TM ownership と同時にやると影響が広がる。 |
| Special move provenance | Medium | Event / XD candidates は通常 learnset と異なるため、source badge と external JSON audit trail が必要。 |

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
- A TM-compatible move appears according to the selected virtual TM unlock policy.
- A move present in multiple sources appears once per source.
- An event / XD special move appears only when the special source is enabled.
- Already-known moves do not create duplicate move slots.
- Cancel returns to script / summary / party menu correctly.
- A species with many TM/tutor options does not overflow or corrupt the list.

## Open Questions

- Should unified mode ignore Heart Scale / cost, or keep the existing script cost after successful learning?
- What story/rank flags should unlock each virtual TM group?
- Should tutor/tower moves use the same story flag groups as virtual TMs, or a separate unlock table?
- Should the first implementation use source tabs (`Lv`, `Egg`, `TM`, `Tutor`) or fixed 50-60 entry pages?
- Should candidate storage cap be exactly 600 entries, or align to 640 for safer future-generation headroom?
- Should special moves be enabled globally, postgame-only, debug-only, or split into `special_xd` and `special_event` unlock groups?
