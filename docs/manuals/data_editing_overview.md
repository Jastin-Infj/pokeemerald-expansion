# Data Editing Overview

このページは、データ変更を始める前の共通フローです。
個別の細かい実装は、目的別 manual と既存調査 docs を参照します。

## 基本フロー

1. 目的を 1 つに絞る。
2. データの持ち主を探す。
3. 既存例を検索する。
4. 1 行変更で済むか、ID や generated data まで増えるかを分ける。
5. 影響範囲を確認する。
6. 変更する。
7. ビルド、mdBook、ROM 上の確認を実施する。

## データの持ち主

| 対象 | 主な定義元 | 参照 docs |
| --- | --- | --- |
| 種族値、タイプ、特性、EV yield | `src/data/pokemon/species_info/gen_*_families.h` | [Pokemon Data Map](../overview/pokemon_data_map_v15.md) |
| 種族 ID | `include/constants/species.h` | [Pokemon Data Map](../overview/pokemon_data_map_v15.md) |
| レベル技 | `src/data/pokemon/level_up_learnsets/gen_*.h` | [Pokemon Data Map](../overview/pokemon_data_map_v15.md) |
| 教え技、TM learnset | `src/data/pokemon/teachable_learnsets.h` | [Pokemon Data Map](../overview/pokemon_data_map_v15.md) |
| 技 ID | `include/constants/moves.h` | [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) |
| 技データ | `src/data/moves_info.h` | [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) |
| 技効果 enum | `include/constants/battle_move_effects.h` | [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) |
| 技効果 script | `src/data/battle_move_effects.h`, `data/battle_scripts_*.s` | [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) |
| 技 animation | `data/battle_anim_scripts.s` | [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) |
| TM/HM 一覧 | `include/constants/tms_hms.h` | [TM/HM Expansion](../overview/tm_hm_expansion_250_v15.md) |
| アイテムデータ | `include/constants/items.h`, `src/data/items.h` | [Move/Item/Ability Map](../overview/move_item_ability_map_v15.md) |
| マップ script | map data, event script | [Map Script Flow](../flows/map_script_flow_v15.md) |
| NPC / object event | `events.inc`, `scripts.inc`, object movement | [NPC Object Event Flow](../flows/npc_object_event_flow_v15.md) |
| 条件付き tile / layout | `setmetatile`, `setmaplayoutindex`, map scripts | [NPC Object Event Flow](../flows/npc_object_event_flow_v15.md) |
| Region map / Fly | region map data, Fly flags | [Map Registration Fly Region Flow](../flows/map_registration_fly_region_flow_v15.md) |
| BGM | `include/constants/songs.h`, `sound/song_table.inc`, map music | [How to add or change BGM](../tutorials/how_to_add_bgm.md) |
| 新規 item | `include/constants/items.h`, `src/data/items.h` | [How to add a new item](../tutorials/how_to_add_new_item.md) |
| EV/IV/nature/moveset UI | `src/pokemon.c`, `src/party_menu.c`, `src/move_relearner.c` | [Champions Training UI Feasibility](../overview/champions_training_ui_feasibility_v15.md) |
| 野生初期技 random | `src/wild_encounter.c`, `src/pokemon.c`, `src/move_relearner.c` | [Wild Moveset Randomization Feasibility](../overview/wild_moveset_randomization_v15.md) |
| trainer party pool / generator | `src/data/trainers.party`, `tools/trainerproc/main.c`, `src/trainer_pools.c` | [Opponent Party Preview and Randomizer Investigation](../features/battle_selection/opponent_party_and_randomizer.md) |
| option menu / runtime battle options | `src/option_menu.c`, `include/global.h`, `include/config/battle.h` | [Options and Status UI Flow](../flows/options_status_flow_v15.md) |
| runtime rule options | `include/config/battle.h`, `include/config/pokemon.h`, `include/config/item.h`, `src/difficulty.c` | [Runtime Rule Options Feasibility](../overview/runtime_rule_options_feasibility_v15.md) |
| NPC battle AI | `include/constants/battle_ai.h`, `include/config/ai.h`, `src/battle_ai_*.c` | [Battle AI Decision Flow](../flows/battle_ai_decision_flow_v15.md) |
| roguelike party / held item policy | party menu, storage, battle aftercare | [Roguelike Party Policy Impact](../overview/roguelike_party_policy_impact_v15.md) |

## 1 行変更で済みやすいもの

次は、既存 ID を増やさず既存フィールドの値だけを変えるなら比較的狭い変更です。

- 既存ポケモンの種族値。
- 既存ポケモンのタイプ。
- 既存ポケモンの通常特性。
- 既存技の威力、命中、PP、タイプ。
- 既存技の category や flags。

ただし、バトル挙動や AI、難易度には影響します。
値だけの変更でも、実際に該当する戦闘で確認します。

## 影響範囲が広がりやすいもの

次は、実装前に調査 docs を追加または更新します。

- 新しい種族 ID。
- 新しい技 ID。
- 新しいアイテム ID。
- TM/HM 数の拡張。
- 新しい技効果や battle script。
- セーブに入る値、ビット幅、保存互換性。
- マップ追加、タウンマップ、Fly 登録の同期。
- NPC cutscene、条件付き tile / layout、Town Map / Fly 後の popup 復帰。
- Battle Frontier など、実 level と battle-only effective level が分かれるルール。
- 新規 BGM、新規 item。
- EV/IV UI のような UI、データ、計算式をまたぐ機能。
- 野生初期技 random、TM/tutor 技の自動混在、move weight table 追加。
- trainer party pool や外部 generator による `trainers.party` 大量生成。
- option menu 複数 page 化、runtime battle option 保存先追加。
- Nuzlocke、release、difficulty、EXP/catch/shiny 倍率、Mega/Z/Dynamax/Tera、trade/randomizer のような runtime rule option。
- held item lock、battle item restore、forced release のような party state をまたぐ施設ルール。
- AI scoring / switching / double battle へ独自評価を足す変更。
