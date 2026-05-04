# Project Work Manuals

この章は、作業者が「どこから触るか」を決めるための入口です。
既存の `docs/tutorials/` は実装寄りの手順が多いため、この manual では先に作業順、編集対象、影響範囲を確認します。

## 最初に読む順番

1. [Environment Setup](environment_setup.md)
2. [GitHub Workflow](github_workflow.md)
3. [Data Editing Overview](data_editing_overview.md)

この 3 つを確認してから、目的別の manual に進みます。

## 目的別の入口

| やりたいこと | 最初に読む manual | 主な編集元 |
| --- | --- | --- |
| 既存ポケモンの種族値、タイプ、特性を変えたい | [Pokemon Stats Manual](pokemon_stats_manual.md) | `src/data/pokemon/species_info/gen_*_families.h` |
| 既存技の威力、命中、タイプなどを変えたい | [Move Data Manual](move_data_manual.md) | `src/data/moves_info.h` |
| 技を新しく追加したい | [Move Data Manual](move_data_manual.md) | `include/constants/moves.h`, `src/data/moves_info.h` |
| TM/HM を増やしたい、フィールド技を整理したい | [TM/HM Manual](tm_hm_manual.md) | `include/constants/tms_hms.h` |
| 新規マップ、タウンマップ、Fly 登録を整理したい | [Map / Fly Manual](map_fly_manual.md) | map data, region map data, Fly flags |
| Battle message、field message、UI text の影響範囲を整理したい | [Message Text Manual](message_text_manual.md) | `src/battle_message.c`, `data/battle_scripts_*.s`, `src/field_message_box.c` |
| NPC の配置、移動、条件付きタイル変更を調べたい | [Map Script Flow](../flows/map_script_flow_v15.md) / [NPC Object Event Flow](../flows/npc_object_event_flow_v15.md) | `data/maps/*/events.inc`, `data/maps/*/scripts.inc` |
| 性格、EV、IV、技を Champions 風に調整したい | [Champions Training UI Feasibility](../overview/champions_training_ui_feasibility_v15.md) | `src/pokemon.c`, `src/party_menu.c`, `src/move_relearner.c` |
| 野生 Pokemon の初期技を重み付き random にしたい | [Wild Moveset Randomization Feasibility](../overview/wild_moveset_randomization_v15.md) | `src/wild_encounter.c`, `src/pokemon.c`, `src/move_relearner.c` |
| trainer.party で候補 pool や外部 generator を使いたい | [Opponent Party Preview and Randomizer Investigation](../features/battle_selection/opponent_party_and_randomizer.md) / [Trainer Party Pools](../tutorials/how_to_trainer_party_pool.md) | `src/data/trainers.party`, `tools/trainerproc/main.c` |
| option menu を複数 page にしたい、battle option を増やしたい | [Options and Status UI Flow](../flows/options_status_flow_v15.md) | `src/option_menu.c`, `include/global.h`, `include/config/battle.h` |
| Nuzlocke、release、難易度、EXP/catch/shiny、gimmick on/off を option 化したい | [Runtime Rule Options Feasibility](../overview/runtime_rule_options_feasibility_v15.md) | `include/config/battle.h`, `include/config/pokemon.h`, `include/config/item.h`, `src/difficulty.c` |
| NPC AI、交代、double battle の挙動を調べたい | [Battle AI Decision Flow](../flows/battle_ai_decision_flow_v15.md) | `src/battle_ai_main.c`, `src/battle_ai_switch.c`, `include/config/ai.h` |
| ローグライク施設や持ち物ロックを設計したい | [Roguelike Party Policy Impact](../overview/roguelike_party_policy_impact_v15.md) | party menu, storage, battle aftercare |
| BGM を差し替えたい、新規 BGM を足したい | [How to add or change BGM](../tutorials/how_to_add_bgm.md) | `include/constants/songs.h`, `sound/song_table.inc`, `sound/songs/midi/` |
| 新しいアイテムを追加したい | [How to add a new item](../tutorials/how_to_add_new_item.md) | `include/constants/items.h`, `src/data/items.h` |

## 作業前の原則

作業者は、実装前に次を確認します。

- `git status --short --branch` で現在のブランチと未コミット差分を見る。
- ユーザーが docs-only を指定している場合、ソース、include、data、tools は読み取り専用にする。
- 既存の未コミット差分はユーザー作業として扱い、勝手に戻さない。
- 追加や変更が保存データ、ID 幅、アップストリーム追従に関係する場合は、実装前にリスクを docs に書く。
- 既存 docs と依頼内容が矛盾する場合は、そのまま進めず、どの方針を優先するかを明示して突き返す。

## 迷ったときの判断

既存値を変えるだけなら、対象データの 1 エントリ修正で済むことがあります。
ただし、ID を増やす、新しいデータ種を足す、フィールド挙動を変える、セーブに入る値を変える場合は、影響範囲を必ず洗います。

特に次の変更は manual だけで完結させず、調査 docs も確認します。

- 種族、技、アイテム、TM/HM の ID 数を増やす変更
- EV/IV UI やステータス計算に関わる変更
- マップ追加、タウンマップ表示、Fly 先登録をまたぐ変更
- NPC cutscene、条件付き map layout / metatile 変更、Town Map / Fly 後の表示復帰
- Battle Frontier など対戦用 level scaling
- 野生初期技 random、TM/tutor 候補混在、trainer party pool / external generator
- option menu 複数 page 化、runtime battle option 追加
- Nuzlocke、release、difficulty、EXP/catch/shiny、Mega/Z/Dynamax/Tera、trade/randomizer のような rule option 追加
- 新規 BGM、新規アイテム追加
- Battle message、battle effect message、field message、UI-local text の追加・差し替え
- HM をフィールド技ではなくキーアイテムやフラグ判定へ移す変更
