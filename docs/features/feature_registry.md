# Feature Registry

この docs 配下で管理する独自機能候補の一覧。

## Status Values

| Status | Meaning |
|---|---|
| Investigating | 既存コード調査中 |
| Planned | 設計方針を作成済み、未実装 |
| Implementing | 実装中 |
| Testing | 実装済み、検証中。テストで設計ミスが見つかった場合は docs に戻して計画を更新する |
| Shipped | 利用可能。feature complete として現在の仕様を固定し、以後の変更は別 task / revision として扱う |
| Paused | 保留 |

## Feature Docs Workflow

feature docs は、実装前の一時メモではなく、その branch で守る作業契約として扱う。
実装中に判断が変わった場合は、コードだけを進めず、先に owning feature の docs を更新する。

### Branch Loop

| Phase | Branch behavior | Docs update |
|---|---|---|
| Investigating | 既存コードと既存 docs を読む。docs-only 指定時はソース変更しない。 | `investigation.md` に実ファイル、symbol、未確認事項を残す。 |
| Planned | 実装方針を決める。まだ code contract は固定しない。 | `README.md`、`mvp_plan.md`、`risks.md`、`test_plan.md` に current decision と影響範囲を書く。 |
| Implementing | feature branch で実装する。実装中に方針が変わったら docs を更新してから続ける。 | 設計との差分、採用しなかった案、後続 phase を追記する。 |
| Testing | focused test / build / manual check を実行する。失敗が実装ミスなら修正し、設計ミスなら Planned に戻す。 | `test_plan.md` に結果を書き、設計へ戻した理由は `risks.md` か `mvp_plan.md` に残す。 |
| Shipped | feature complete として current behavior を固定する。 | README の current contract、test 結果、残リスク、future work を清書する。 |

### Impact Notes

- 影響範囲は、原則として変更の原因になる owning feature docs に書く。
- downstream feature の仕様が直接変わる場合だけ、その downstream docs へも短い参照を追加する。
- 例えば party generator が team display / opponent preview に影響する可能性は、まず battle selection / party generator 側の `risks.md` や調査 docs に Cross-Feature Notes として残す。
- team display 側の docs は、team display 自体の要件を変更する段階まで無理に更新しない。
- 長い設計 docs だけに影響範囲を閉じ込めず、feature folder の `README.md` か `risks.md` から辿れるようにする。

### Feature Complete Contract

feature complete は「今後一切変更しない」という意味ではない。
その時点の仕様、入力、出力、test gate、既知の残リスクを固定し、以後の変更を別 feature / revision として追える状態を指す。

feature complete にする前に、最低限次を確認する。

- `README.md` の current decision が実装済みの挙動と一致している。
- `test_plan.md` に実行した test / 未実行 test / manual check が残っている。
- `risks.md` の未解決項目が、blocker、accepted risk、future work に分かれている。
- 他 feature への影響が `Impact Notes` または `Cross-Feature Notes` から辿れる。
- registry の `Status` と `Code Status` が現在の branch 状態と矛盾していない。

## Features

| Feature | Status | Code Status | Docs | Notes |
|---|---|---|---|---|
| Project Work Manuals | Investigating | No code changes | `docs/manuals/` | 作業者向けの入口 manual。docs navigation、環境構築、GitHub 運用、データ編集、rebuild/test、generated data workflow、未調査 queue、種族値、技、TM/HM、Map/Fly の初動を整理。 |
| Trainer Battle Party Selection | Investigating | No code changes | `docs/features/battle_selection/` | 通常 trainer battle 前に 6 匹から 3/4 匹を選出する候補。UI / opponent preview / randomizer は追加調査済みで MVP からは分離。 |
| Pokemart / Shop Configuration | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | `ScrCmd_pokemart`、`CreatePokemartMenu`、`Task_BuyMenu`、`data/maps/*Mart*/scripts.inc` を入口に調査。 |
| Wild Pokemon Randomizer | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | `src/wild_encounter.c`、`src/data/wild_encounters.json`、DexNav / Pokedex area への影響を確認済み。build-time か runtime かは未決定。 |
| No Random Encounters | Planned | No code changes | `docs/features/no_random_encounters/` | `OW_FLAG_NO_ENCOUNTER` を使い、通常歩行中の land / water random encounter を止める候補。MVP は step-only。Fishing / Sweet Scent / Rock Smash / static wild battle / option UI は後続扱い。 |
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
- 新規 feature folder は `docs/templates/feature_folder_template.md` を基準に作る。
- 実装に使う既存 symbol は、推測ではなく実ファイル名と関数名を記録する。
- 実装 branch では、影響範囲と current decision を owning feature docs に更新しながら進める。
- upstream 追従で壊れそうなファイルは `docs/upgrades/upstream_diff_checklist.md` に追加する。
- 未確認事項は各 feature の `Open Questions` に残す。
