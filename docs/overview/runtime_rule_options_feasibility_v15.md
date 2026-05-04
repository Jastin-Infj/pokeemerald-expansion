# Runtime Rule Options Feasibility v15

調査日: 2026-05-04。現時点では実装なし。

## Purpose

Nuzlocke、難易度、release / wipeout、EXP / catch / shiny、Mega / Z / Dynamax / Terastal、trade、randomizer などを option menu や facility rule から切り替える場合の入口とリスクを整理する。

この文書の結論は、すべてを `src/option_menu.c` に直接持たせるより、まず **runtime rule state** を定義し、option menu / script / facility から同じ state を更新する形が安全、というもの。

## Related Docs

| Doc | Why |
|---|---|
| `docs/flows/options_status_flow_v15.md` | 現行 option menu と multi-page 化。 |
| `docs/tutorials/how_to_config_flags_vars.md` | `TRUE` 直書きではなく flag / var ID を割り当てる pattern。 |
| `docs/overview/roguelike_party_policy_impact_v15.md` | held item lock、release、100 戦 facility。 |
| `docs/features/trainer_battle_aftercare/` | battle 後の heal / release / no-whiteout。 |
| `docs/features/battle_item_restore_policy/` | battle 中消費 held item の復元。 |
| `docs/flows/battle_ai_decision_flow_v15.md` | difficulty / AI flag / switching の拡張余地。 |

## Existing Runtime Entrypoints

| Option family | Existing hook | Current default | Runtime-ready? | Notes |
|---|---|---:|---|---|
| Difficulty | `B_VAR_DIFFICULTY`, `src/difficulty.c`, `DIFFICULTY_EASY/NORMAL/HARD` | `0` | Yes, after assigning a var | `gTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT]` に fallback 付きで接続済み。 |
| No catching | `B_FLAG_NO_CATCHING` | `0` | Yes, after assigning a flag | Ball use rejection、obtainable shiny policy と連動。 |
| No running | `B_FLAG_NO_RUNNING` | `0` | Yes, after assigning a flag | wild escape / Roar / Whirlwind / Teleport に影響。 |
| No whiteout | `B_FLAG_NO_WHITEOUT` | `0` | Yes, after assigning a flag | party 自動 heal はされない。release policy と併用注意。 |
| Sleep clause | `B_FLAG_SLEEP_CLAUSE` or `B_SLEEP_CLAUSE` | `0` / `FALSE` | Flag path exists | AI 理解には `AI_FLAG_CHECK_BAD_MOVE` が必要。 |
| Dynamax battle | `B_FLAG_DYNAMAX_BATTLE` | `0` | Yes, after assigning a flag | player は `ITEM_DYNAMAX_BAND` も必要。 |
| Tera charge | `B_FLAG_TERA_ORB_CHARGED`, `B_FLAG_TERA_ORB_NO_COST` | `0` | Yes, after assigning flags | `HealPlayerParty` で charge、Tera 使用で clear。 |
| Wild shiny force | `P_FLAG_FORCE_SHINY`, `P_FLAG_FORCE_NO_SHINY` | `0` | Yes, after assigning flags | wild / gift 作成時に効く。 |
| Gen 6 Exp. Share | `I_EXP_SHARE_FLAG` | `0` | Yes, after assigning a flag | `I_EXP_SHARE_ITEM >= GEN_6` では flag 設定が必須。 |
| Bag use in battle | `B_VAR_NO_BAG_USE` | `0` | Yes, after assigning a var | no bag mode は 0/1/2。 |
| Wild AI flags | `B_VAR_WILD_AI_FLAGS` | `0` | Temporary only | comment で save まで残すなと明記されている。 |

## Compile-time-heavy Entrypoints

| Option family | Existing config / code | Why not a simple option |
|---|---|---|
| Mega forms on/off | `P_MEGA_EVOLUTIONS` in `include/config/species_enabled.h`, many `#if P_MEGA_EVOLUTIONS` tables | species / form table / cries / graphics が compile-time に消える。runtime option で完全無効化するなら `CanMegaEvolve` 側の追加 gate が安全。 |
| Tera forms data on/off | `P_TERA_FORMS`, `P_SHOW_TERA_TYPE` | form data 自体は compile-time。battle 中の許可は Tera Orb flags で制御できる。 |
| Z-Move global off | `CanUseZMove`, Z item / move tables | runtime global flag は現状見つからない。Mega と同じく activation gate 追加が必要。 |
| EXP formula generation | `B_EXP_CATCH`, `B_SPLIT_EXP`, `B_SCALED_EXP`, `B_TRAINER_EXP_MULTIPLIER` | battle script の exp 計算が compile-time config を直接読む箇所がある。倍率 option は別 hook を足す方が小さい。 |
| Catch rate multiplier | `ComputeCaptureOdds`, ball modifiers, charm modifiers | global multiplier config は現状見つからない。`ComputeCaptureOdds` に rule multiplier を足すのが入口。 |
| Shiny reroll count | `I_SHINY_CHARM_ADDITIONAL_ROLLS`, `P_FLAG_FORCE_SHINY` | force shiny は flag で可能。倍率/追加 reroll option は `CreateBoxMon` の reroll 計算に新規 rule を足す必要がある。 |
| Trade restriction | `src/trade.c`, in-game trade scripts | link trade / NPC trade / evolution trade / game stats のどこを止めるかで範囲が変わる。 |
| Randomizer | wild encounter, trainer party pool, learnset, item data | runtime randomizer は広すぎる。外部 generator か既存 Trainer Party Pool を優先。 |

## Requested Rule Matrix

| Requested rule | Recommended storage | First implementation target | Risk |
|---|---|---|---|
| Nuzlocke enabled | SaveBlock3 or event var | A single `RULE_MODE_NUZLOCKE` state | High |
| One catch per area | SaveBlock3 bitset/table | catch success path + mapsec/area resolver | High |
| Faint means dead/release | SaveBlock3 + battle aftercare | `CB2_EndTrainerBattle` after party restore | High |
| Release system enabled | SaveBlock3 or facility state | aftercare policy, not PC release UI | High |
| Difficulty Easy/Normal/Hard | `B_VAR_DIFFICULTY` | option menu writes assigned var | Medium |
| EXP multiplier | SaveBlock3 or event var | `ApplyExperienceMultipliers` extra multiplier | Medium |
| Catch rate multiplier | SaveBlock3 or event var | `ComputeCaptureOdds` extra multiplier | Medium |
| Shiny reroll multiplier | SaveBlock3 or event var | `CreateBoxMon` reroll calculation | Medium |
| Terastal on/off | assigned flags + optional runtime gate | `B_FLAG_TERA_ORB_CHARGED` / no-cost | Low/Medium |
| Dynamax on/off | `B_FLAG_DYNAMAX_BATTLE` | assign flag, script / option toggles it | Low |
| Mega disabled | SaveBlock3 or event flag + new gate | `CanMegaEvolve` runtime gate | Medium |
| Trade disabled | SaveBlock3 or event flag | first decide NPC trade vs link trade | Medium/High |
| Randomizer enabled | external generator or trainer pools | build-time data generation first | Medium/High |

## Nuzlocke / Release Notes

Nuzlocke は単一 flag ではなく複合 rule。

Minimum pieces:

1. mode enabled state。
2. area capture state。`MAPSEC_*` だけで足りるか、map group / map num で持つかを決める。
3. catch success hook。捕獲に成功した時点で area を consumed にする。
4. faint/death hook。battle 後の party restore が終わってから dead state を反映する。
5. release / box lock / cemetery box など、死んだ Pokemon の扱い。
6. softlock guard。最後の usable mon、egg、field move / HM、follower、選出 party を考慮する。

PC release UI の `Task_ReleaseMon` / `ReleaseMon` / `PurgeMonOrBoxMon` は storage UI の state machine に閉じている。強制 release は、PC UI を呼ぶより、aftercare 専用の low-level処理を設計する方がよい。

## Difficulty Notes

難易度は既にかなり近い。

- `include/constants/difficulty.h` に `DIFFICULTY_EASY`, `DIFFICULTY_NORMAL`, `DIFFICULTY_HARD` がある。
- `src/difficulty.c` は `B_VAR_DIFFICULTY` が `0` なら Normal fallback、割り当て済みなら `VarGet(B_VAR_DIFFICULTY)` を使う。
- `tools/trainerproc/main.c` は `Difficulty:` を読み、未指定なら Normal として出力する。
- `include/data.h` は `GetTrainerDifficultyLevel` / `GetBattlePartnerDifficultyLevel` 経由で `gTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT]` を参照する。

option menu から使うなら、まず `B_VAR_DIFFICULTY` に real var を割り当てる。その上で option item は Easy/Normal/Hard を `VarSet` するだけに寄せる。各 trainer の Hard party が未定義なら Normal に fallback するため、段階導入しやすい。

## EXP / Catch / Shiny Notes

EXP:

- `Cmd_getexp` は base EXP、参加数、Exp Share、trainer multiplier、split/scaled EXP を計算する。
- `ApplyExperienceMultipliers` は traded、Lucky Egg、unevolved、affection、`ITEM_EXP_CHARM` をまとめている。
- 追加の EXP 倍率 option は、`ApplyExperienceMultipliers` の最後に rule multiplier を足すのが一番狭い。

Catch:

- `ComputeCaptureOdds` が ball multiplier、badge malus、low-level bonus、status bonus をまとめる。
- Catching Charm は critical capture odds 側の `B_CATCHING_CHARM_BOOST`。
- 「全体的に捕まりやすくする」は `ComputeCaptureOdds` の最終 odds に倍率を掛ける設計が自然。ただし Master Ball / guaranteed capture、Safari、テストとの整合性を見る。

Shiny:

- `CreateBoxMon` で `P_FLAG_FORCE_SHINY` / `P_FLAG_FORCE_NO_SHINY` を見ている。
- 追加 reroll は Shiny Charm、Lure、Chain Fishing、DexNav の合算。
- option で「色違い出やすさ」を作るなら、force shiny ではなく extra reroll count か odds multiplier として入れる方が game balance を保ちやすい。

## Gimmick Notes

Terastal:

- player は `ITEM_TERA_ORB` と charge flag が必要。
- `B_FLAG_TERA_ORB_NO_COST` で消費なしにできる。
- `HealPlayerParty` は Tera Orb 所持時に charge flag を set する。

Dynamax:

- player は `ITEM_DYNAMAX_BAND` と `B_FLAG_DYNAMAX_BATTLE` が必要。
- 通常の on/off option は flag 割り当てだけで始めやすい。
- AI 側は「Dynamax を許可する flag」と「今 Dynamax すべきか」を分ける必要がある。現状は smart timing evaluation が Tera より弱く、許可された trainer mon が早く使いすぎる可能性がある。

Mega:

- form data は `P_MEGA_EVOLUTIONS` で compile-time に制御される。
- runtime option で禁止するなら `CanMegaEvolve` の先頭付近に `Rule_IsMegaAllowed()` のような gate を追加する案が現実的。
- data 自体を消すのではなく、battle activation を止める方式にする。

AI timing:

- on/off option は `CanTerastallize` / `CanDynamax` の許可条件に近い。
- 発動タイミングは `BattleAI_ChooseMoveIndex`、`SetAIUsingGimmick`、`DecideTerastal`、opponent / partner controller の `RET_GIMMICK` emission に近い。
- `AI_FLAG_SMART_TERA` はあるが single 1v1 前提で、double battle や Dynamax には追加設計が必要。
- 詳細は [Battle AI Decision Flow v15](../flows/battle_ai_decision_flow_v15.md) の "Gimmick Timing / Tera / Dynamax" を参照。

## Option Menu Architecture

大量の rule を option menu に追加する場合、現行 SaveBlock2 bitfield に詰め込むのは避けたい。rule は SaveBlock3 または dedicated runtime rule struct に置き、option menu は UI として読む/書くだけにする。

Suggested pages:

```text
Page 0: General
  Text Speed
  Battle Scene
  Battle Style
  Sound
  Button Mode
  Frame

Page 1: Battle Rules
  Difficulty
  No Bag
  Sleep Clause
  No Running
  No Catching
  No Whiteout

Page 2: Roguelike
  Nuzlocke
  Release On Loss
  Held Item Lock
  Item Clause
  Battle Item Restore

Page 3: Growth / Encounter
  EXP Multiplier
  Catch Rate
  Shiny Rolls
  Wild IV Mode
  Wild Moveset Mode

Page 4: Gimmicks
  Mega
  Z-Move
  Dynamax
  Terastal
  Tera No Cost
```

## Recommended MVP Order

1. Assign `B_VAR_DIFFICULTY` and expose Easy/Normal/Hard in scripts first.
2. Assign existing low-risk flags: `B_FLAG_DYNAMAX_BATTLE`, `B_FLAG_TERA_ORB_CHARGED`, `B_FLAG_TERA_ORB_NO_COST`, `B_FLAG_NO_CATCHING`, `B_FLAG_NO_RUNNING`, `B_FLAG_SLEEP_CLAUSE`.
3. Add a docs-only rule schema for SaveBlock3 / facility state before touching option menu storage.
4. Implement multi-page option menu only after storage policy is fixed.
5. Add EXP / catch / shiny multipliers as isolated rule helpers.
6. Add Mega / Z runtime gates.
7. Add smart Tera / Dynamax timing decisions so AI does not use a gimmick merely because it is available.
8. Design Nuzlocke / release aftercare last, because it mutates party / storage and has softlock risk.

## Open Questions

- Nuzlocke の area は `MAPSEC_*` 単位か、実 map 単位か。
- forced release は即 release、dead box、使用禁止 marker のどれにするか。
- trade disabled は link trade、NPC trade、trade evolution のどこまで含めるか。
- EXP / catch / shiny は story 全体 option か、facility 中だけの rule か。
- Mega / Z / Dynamax / Tera は player だけ禁止か、AI も禁止か。
- Tera / Dynamax を許可した trainer は即発動でよいか、Smart / AceOnly / LastMon / Scripted のような timing policy が必要か。
- Randomizer は runtime に残すか、外部 generator に逃がすか。
