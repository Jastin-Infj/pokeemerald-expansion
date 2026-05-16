# Unified Move Relearner

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-16 |
| Baseline | `master` `459703c0aa`; `git describe` = `expansion/1.15.2-48-g459703c0aa` |
| Code status | Implemented draft on `feature/unified-move-relearner` |
| Provenance | User request and local code/docs review |

## Status

Status: Implemented draft. Unified mode is guarded by config, uses generated
historical candidate data, and has local build plus mGBA Live evidence. The
special event / XD / Ranger / form-specific / LGPE partner seed data is now
connected as a `Sp` source; story unlock gating and per-entry special labels
remain future work.

## Goal

Move Relearner を「レベル技 / egg move / TM / tutor」を別メニューで選ばせる
形式から、対象 Pokemon が覚えられる全候補を 1 つの list で見られる形式へ寄せる。
入口は summary の move page、party menu の行動、NPC / event script の 3 つを
維持し、どこから入っても同じ候補生成を使えるようにする。

要求の中心は次の通り。

- level に関係なく、level-up learnset の全技を思い出せる。
- egg move を同じ list に混ぜる。
- TM/HM 互換技を同じ list に混ぜる。
- tutor / Battle Frontier 系の教え技を同じ list に混ぜる。
- Gen 1 から Gen 9 までの historical TM / tutor data をどこまで許可するか
  config で選べる余地を残す。
- TM は item として増やさず、Move Relearner 用の virtual TM candidate pool
  として 250-300 程度の技を扱う方針を優先する。
- historical TM 候補が tutor / Battle Tower 系候補と重複する場合でも、
  source ごとの候補としてそのまま表示する。ユーザーが「TM にあったはずなのに
  tower 側にだけある」と混乱しないよう、候補単位は move ではなく move + source
  として扱う。
- virtual TM は全体 ON/OFF だけでなく、story rank / clear flag / group unlock により
  「TM の個別技が解放された」という粒度で制御できるようにする余地を残す。
- 報酬文脈は「TM item をもらう」ではなく、「virtual TM pool の一部が解放され、
  対応する技を Move Relearner で覚えられるようになった」として扱える。
- 現在覚えている 4 技は候補から除外するか、少なくとも選択時に二重習得にならないようにする。
- UI は既存 Move Relearner を流用してよいが、候補生成と表示上限を安全にする。
- 候補数が大きい species では、1 list が苦しい場合に level / egg / TM /
  tutor のタブ表示へ切り替えられるようにする。

## Docs

- [Investigation](investigation.md)
- [Candidate Data Flow](candidate_data_flow.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)

## Implementation Handoff

The current feature branch is a functional implementation candidate rather than
only a design branch. The remaining work is mostly policy and polish:

- Decide whether broad historical TM / tutor sources stay always-on, or become
  story/rank/clear-flag gated.
- Decide whether `Sp` stays as one compact label, or whether JSON `display`
  metadata becomes visible as per-entry badges such as `EV`, `XD`, `FC`, or
  `LP`.
- Run one manual pass that actually teaches / overwrites a move from each major
  entry route. Current mGBA evidence covers rendering, page navigation, cancel,
  and selected special candidates.
- Continue expanding and auditing special-event data in small source-referenced
  JSON commits.

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

現行 config だけでは不足している点:

- `P_LVL_UP_LEARNSETS` は level-up learnset を 1 世代に切り替える config であり、
  Gen 1 から Gen 9 までを統合して relearner に出す機能ではない。
- `src/data/pokemon/all_learnables.json` は `tools/learnset_helpers/porymoves_files/*.json`
  の union だが、世代別の source metadata は runtime へ残っていない。
- `src/data/pokemon/teachable_learnsets.h` は current TM/HM list と script tutor list
  から生成される。現在の physical TM/HM は Gen 3 相当の 50 TM + 8 HM で、
  historical TM/TR を全部含むものではない。今回の relearner では physical TM item
  を増やさず、virtual/generated pool から候補を出す設計を優先する。
- `src/data/tutor_moves.h` は script scan 由来で、Battle Frontier / FRLG 系 tutor を含む。
  `MOVE_MEGA_PUNCH` は既に含まれている。

## Proposed Contract

MVP は新しい unified mode を追加し、既存 state は互換用に残す。

| Case | Behavior |
|---|---|
| unified mode off | 既存 Move Relearner flow のまま。 |
| unified mode on | level-up / egg / TM / tutor 候補を同じ入口で扱う。候補過多の場合は source tab / chunked page fallback を許容する。 |
| level-up moves | `P_ENABLE_ALL_LEVEL_UP_MOVES` 相当で level 制限を外す。 |
| TM moves | current physical TM/HM、bag 所持、all-compatible、always-on virtual TM、rank / flag / story reward gated virtual TM のどれを使うか config で分ける。 |
| egg moves | species の egg move table を候補に入れる。 |
| tutor moves | `src/data/tutor_moves.h` の generated tutor list を候補に入れる。 |
| special moves | 配布 / XD / Ranger transfer など通常 learnset では拾えない候補を `Sp` source として入れる。 |
| duplicate source | 同じ move が複数 source に出た場合、source ごとの別 entry として表示する。 |
| source grouping | Level / virtual TM / tutor / Battle Tower は同じ入口で扱い、必要なら source tab / chunk で分ける。 |
| move source display | 重複を許容するため、`Lv` / `Egg` / `TM` / `Tutor` / `Sp` などの source 表示が実質必須。 |
| too many candidates | `MAX_RELEARNER_MOVES` 超過で memory overwrite しない。300 前後の候補を 1 本の縦 list で選ばせる UX は避け、50-60 件単位の tab / page chunk を検討する。 |

## Impact Surface

| File / area | Impact |
|---|---|
| `src/move_relearner.c` | `CreateLearnableMovesList()` と各 `GetRelearner*Moves()` を unified candidate builder へ寄せる。 |
| `include/constants/move_relearner.h` | `MOVE_RELEARNER_*` に unified state を足すか、config-only path にするか決める。`MAX_RELEARNER_MOVES` を再評価する。 |
| `include/config/summary_screen.h` | unified mode、all-level、all-TM、source inclusion の build-time config が必要。 |
| `data/scripts/move_relearner.inc` | dynmulti の category choice を出すか、unified 入口へ直行するか決める。 |
| `src/pokemon_summary_screen.c` | START prompt と L/R state cycling が category 前提なので、unified mode の copy / state 表示が必要。 |
| `src/party_menu.c` | `P_PARTY_MOVE_RELEARNER` は既存だが、現在は source submenu 型。unified 行動を追加するか置換する必要がある。 |
| `src/menu_specialized.c` | source badge や long list 表示を足す場合に影響。 |
| `src/data/pokemon/level_up_learnsets/` | source data。level 制限を外すだけなら変更不要。 |
| `src/data/pokemon/egg_moves.h` | egg candidates。species / form / pre-evo policy の確認が必要。 |
| `src/data/pokemon/teachable_learnsets.h` | TM/HM compatibility。generated data なので直接編集しない方針を確認する。 |
| `include/constants/tms_hms.h` / `src/item.c` | Current physical TM/HM registry。MVP では item 数を増やさず、relearner pool とは分離する。 |
| `tools/learnset_helpers/` | Gen 1-9 の許可範囲や virtual TM group / rank metadata を config にするなら generated data の形式変更が必要。 |

## Implementation Notes

候補 builder は move ID だけの配列ではなく、source 付き entry を中心にする方が安全。
同じ move が TM と tower にある場合も、`{ move, source }` が違えば別候補として残す。
ただし、既に覚えている move は source 違いを含めて候補から除外する。

```text
BuildUnifiedRelearnerMoves(mon)
  -> AppendLevelUpMoves(allLevels = TRUE)
  -> AppendEggMoves(if enabled)
  -> AppendVirtualTMMoves(if enabled / unlock policy)
  -> AppendTutorMoves(if enabled)
  -> AppendSpecialMoves(if enabled)
  -> AppendTowerMoves(if enabled)
  -> RemoveAlreadyKnownMoves(source-insensitive)
  -> Preserve source duplicates
  -> Sort/group by source if enabled
  -> Fail cleanly if candidate count exceeds cap
```

`MAX_RELEARNER_MOVES 60` は Mew + virtual TM + tutor 混在では足りない可能性がある。
単純に増やすと候補保持はできるが、300 件近い list を下までスクロールする UX が重い。
実装前に候補 storage cap と、50-60 件単位の tab / page chunk / source tab のどれを採るかを決める。

## Risks

| Risk | Severity | Notes |
|---|---|---|
| Candidate overflow | High | unified list は現在の 60 cap を超えやすい。最初に guard を入れる。 |
| TM ownership policy mismatch | Medium | `P_ENABLE_ALL_TM_MOVES` を使うと bag 所持や TM shop progression の価値が変わる。 |
| Hidden source confusion | High | 重複 source を残す方針では、source 表示なしだと同じ技名が複数並び混乱する。source badge / label が必要。 |
| Long-list UX | High | 300 件近い候補を既存 list UI で縦スクロールすると選択に時間がかかる。50-60 件単位の tab / page chunk を検討する。 |
| Summary state cycling | Medium | 既存 summary prompt は relearner state 切替を持つため、unified mode では L/R 表示を整理する。 |
| Tutor / egg unlock flags | Medium | script の flag 解禁を無視すると、既存 progression と食い違う。 |
| Virtual TM pool size | High | Mew が 250-300 件近い TM 候補を持つ場合、現在の 60 cap には収まらない。item / bag ではなく UI と candidate storage の問題として扱う。 |
| Unlock granularity | Medium | virtual TM を rank / clear flag で個別解放する場合、candidate pool に unlock group metadata と compact save state が必要。 |
| Historical generation filtering | High | 現在の generated learnables は source generation を runtime に残さないため、Gen 1 OK / Gen 2 OK のような config は generator 変更が必要。 |

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
- A move present in multiple sources appears once per source, with a visible source label.
- Already-known moves do not create source-duplicate learn options.
- Cancel returns to script / summary / party menu correctly.
- A species with many TM/tutor options does not overflow or corrupt the list.

## Open Questions

- NPC / event script 経由は existing Heart Scale cost を成功時だけ消費する仕様でよいか。
- TM moves は always-available virtual pool、rank / clear-flag unlock bitset、current physical TM registry fallback のどれを初期 default にするか。
- virtual TM unlock は move 単位、rank group 単位、世代 group 単位のどれで管理するか。
- 250-300 TM 前提では、Mew の virtual TM 候補が必ず UI / storage cap に収まる方式を先に決める。
- 50 件刻み、60 件刻み、source tab、または search/sort shortcut のどれを MVP UX にするか。
- Historical Gen 1-9 filtering は generated data まで変更するか、MVP では existing union data を使って後続に送るか。
- UI は初期から single unified list にするか、候補数が多い場合だけ source tab に逃がすか。
- `MAX_RELEARNER_MOVES` は raise / pagination / source-tab / dynamic allocation のどれで扱うか。
