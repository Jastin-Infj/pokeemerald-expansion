# Feature Registry

この docs 配下で管理する独自機能候補の一覧。

## Status Values

| Status | Meaning |
|---|---|
| Investigating | 既存コード調査中 |
| Planned | 設計方針を作成済み、未実装 |
| Implementing | 実装中 |
| Testing | 実装済み、検証中 |
| Shipped | 利用可能 |
| Paused | 保留 |

## Features

| Feature | Status | Code Status | Docs | Notes |
|---|---|---|---|---|
| Project Work Manuals | Investigating | No code changes | `docs/manuals/` | 作業者向けの入口 manual。環境構築、GitHub 運用、データ編集、種族値、技、TM/HM、Map/Fly の初動を整理。 |
| Trainer Battle Party Selection | Investigating | No code changes | `docs/features/battle_selection/` | 通常 trainer battle 前に 6 匹から 3/4 匹を選出する候補。UI / opponent preview / randomizer は追加調査済みで MVP からは分離。 |
| Pokemart / Shop Configuration | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | `ScrCmd_pokemart`、`CreatePokemartMenu`、`Task_BuyMenu`、`data/maps/*Mart*/scripts.inc` を入口に調査。 |
| Wild Pokemon Randomizer | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | `src/wild_encounter.c`、`src/data/wild_encounters.json`、DexNav / Pokedex area への影響を確認済み。build-time か runtime かは未決定。 |
| DexNav / Encounter UI | Investigating | No code changes | `docs/flows/dexnav_flow_v15.md` | Start menu DexNav、detector mode、SaveBlock3、12 land slots、Pokemon icon 描画を整理。 |
| Trainer Party Reorder / Randomizer | Investigating | No code changes | `docs/features/battle_selection/opponent_party_and_randomizer.md` | `DoTrainerPartyPool`、`RandomizePoolIndices`、`AI_FLAG_RANDOMIZE_PARTY_INDICES` を確認。相手 party preview と関係。 |
| TM/HM and Field Move Policy | Investigating | No code changes | `docs/overview/tm_hm_expansion_250_v15.md` | 250 TM 前提の item ID / bag / relearner / field HM coupling を確認。`FOREACH_TM`、`FOREACH_HM`、`ScrCmd_checkfieldmove`、`gFieldMoveInfo`、`CannotForgetMove` も継続参照。 |
| Field Move Modernization / HM Removal | Investigating | No code changes | `docs/features/field_move_modernization/` | Gen7/Gen8 風に HM 技所持へ依存しない field move / obstacle / animation / forget restriction を調査。`docs/flows/field_move_hm_flow_v15.md` を参照。 |
| Champions-style EV/IV Training UI | Investigating | No code changes | `docs/overview/champions_training_ui_feasibility_v15.md` | EV/IV/nature/moveset 編集 UI は実装可能。32 point EV は UI 表示と内部 EV 変換を分ける方針が安全。EV total 518 と wild IV mode も調査済み。 |
| Scout Selection / Starting Battlefield Status | Investigating | No code changes | `docs/overview/scout_selection_and_battlefield_status_v15.md` | Battle Factory / Champions 風の候補 Pokemon 選択 UI、gift mon 付与 flow、trainer flag cleanup、Frontier 風 save / pause / power-cut recovery、held item duplicate restriction、post-battle heal / item restore / PP-EP policy、pickup object / sprite / UI asset 注意点、starting status、PB / ability 強化の調査観点を整理。 |
| Champions Challenge Facility | Planned | No code changes | `docs/features/champions_challenge/` | 0 匹開始、6 匹作成、Lv.50 battle-only、EXP 無効、bag 退避 / 空 challenge bag、egg-only default eligibility、optional Frontier ban、敗北時 challenge party 破棄と通常 party / bag 復元の仕様を整理。runtime より先に party generator を進め、copy-paste 可能な generated `.party` fragment / validation / diff を予約出力にする方針。 |
| Wild Moveset Randomization | Investigating | No code changes | `docs/overview/wild_moveset_randomization_v15.md` | 野生初期技の現行「最後 4 level-up 技」flow、weighted level-up、TM/tutor 混在、外部 weight table の候補を整理。 |
| Runtime Rule Options | Investigating | No code changes | `docs/overview/runtime_rule_options_feasibility_v15.md` | Nuzlocke、release、difficulty、EXP/catch/shiny 倍率、Mega/Z/Dynamax/Tera、trade、randomizer の option 化候補を整理。 |
| Battle AI Decision / Switching | Investigating | No code changes | `docs/flows/battle_ai_decision_flow_v15.md` | move scoring、smart switching、double battle partner 評価、dynamic AI function の入口を整理。 |
| Roguelike Party / Held Item Policy | Investigating | No code changes | `docs/overview/roguelike_party_policy_impact_v15.md` | 100 戦型 facility、held item lock、item clause、battle item restore、release/swap policy の影響を整理。 |
| Map Registration / Region Map / Fly | Investigating | No code changes | `docs/flows/map_registration_fly_region_flow_v15.md` | 新規 map の `MAPSEC_*`、Town Map/Region Map、Fly icon、visited/world map flag、warp callback を整理。FRLG map preview と Fly 点滅の疑いどころも記録。 |
| NPC / Object Event / Conditional Tiles | Investigating | No code changes | `docs/flows/npc_object_event_flow_v15.md` | `events.inc` の `object_event`、`scripts.inc` の `applymovement` / `setobjectxyperm` / `setmetatile` / `setmaplayoutindex`、Town Map R Fly 後の popup リスクを整理。 |
| Battle Frontier Level Scaling | Investigating | No code changes | `docs/flows/battle_frontier_level_scaling_flow_v15.md` | 現行 Lv.50 course は低レベルを Lv.50 化しない。対戦用に battle-only Lv.50 補正を入れる場合の hook とリスクを整理。 |
| TM Shop Migration | Investigating | No code changes | `docs/features/tm_shop_migration/` | 50 TM 定義と取得元 flag は別物として整理。`FLAG_RECEIVED_TM_*` 21 件、visible TM item ball flag 14 件、hidden TM item flag 1 件を確認。 |
| Custom Items / Moves / Abilities | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | constants、data table、UI、battle behavior、AI、tests への影響範囲を横断 map に整理。 |
| Battle Item Restore Policy | Investigating | No code changes | `docs/features/battle_item_restore_policy/` | `B_RESTORE_HELD_BATTLE_ITEMS`、きのみ復元、`usedHeldItem`、Recycle/Pickup/Harvest/Cud Chew/G-Max Replenish、item clause の影響を整理。 |
| Trainer Battle Aftercare / Forced Release | Investigating | No code changes | `docs/features/trainer_battle_aftercare/` | trainer battle 後の heal、no-whiteout、wipeout 強制 release の hook を調査。`CB2_EndTrainerBattle`、`B_FLAG_NO_WHITEOUT`、`HealPlayerParty`、PC release flow を確認。 |
| Callback / Dispatch Audit | Investigating | No code changes | `docs/overview/callback_dispatch_map_v15.md` | `SetMainCallback2`、`CB2_*`、`CreateTask`、`ScrCmd_*`、`special`、field callback の確認用 docs。 |
| Map Script / Flag / Var Audit | Investigating | No code changes | `docs/flows/map_script_flag_var_flow_v15.md` | `map.json`、generated `.inc`、hand-written `scripts.inc`、NPC hide flag、coord/bg event、item ball / hidden item flow を整理。 |
| Move Relearner / Summary Menu | Investigating | No code changes | `docs/flows/move_relearner_flow_v15.md` | summary / party / script からの技思い出し flow、`MAX_RELEARNER_MOVES`、TM 追加リスクを整理。 |
| Save Data / Runtime Flags | Investigating | No code changes | `docs/flows/save_data_flow_v15.md` | SaveBlock1/2/3、flag / var、DexNav save field、option save field を整理。 |
| Pokemon Icon UI | Investigating | No code changes | `docs/flows/pokemon_icon_ui_flow_v15.md` | `CreateMonIcon`、icon palette、sprite lifetime、DexNav / custom UI 影響を整理。 |
| Upstream 1.15.2 Upgrade Impact | Investigating | No code changes | `docs/upgrades/1_15_1_to_1_15_2_impact.md` | `expansion/1.15.2` tag の差分を確認。INCGFX migration、DexNav、map script、battle engine、SaveBlock3 影響を整理。 |

## Rules for Future Entries

- 実装前に `README.md`、`investigation.md`、`mvp_plan.md`、`risks.md`、`test_plan.md` を用意する。
- 実装に使う既存 symbol は、推測ではなく実ファイル名と関数名を記録する。
- upstream 追従で壊れそうなファイルは `docs/upgrades/upstream_diff_checklist.md` に追加する。
- 未確認事項は各 feature の `Open Questions` に残す。
