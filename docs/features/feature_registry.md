# Feature Registry

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Baseline | `master` `050a5ab7a3`; `git describe` = `expansion/1.15.2-25-g050a5ab7a3` |
| Code status | Docs-only registry / PR queue snapshot |
| Provenance | Local project overlay |

この docs 配下で管理する独自機能候補の一覧。

## Current Order

実装着手の優先順位。`open_investigation_queue.md` の High Priority、feature の `Status`、現行 `master` に入っている実コードの有無を組み合わせた snapshot。
順序は固定ではなく、上から「次の feature branch で着手しやすい」順に並ぶ。新規 task 開始時はここを基準に owning feature docs を更新する。

### Master Baseline Snapshot (2026-05-09)

`master` は upstream 追従の受け皿で、local runtime source は原則入れない。
次の branch / docs は存在するが、runtime source は `master` にはまだ入って
いないものとして扱う。実装は current `master` から fresh `feature/*` または
`integration/*` branch を切って再適用する。

| Topic | Evidence | Master action |
|---|---|---|
| No Random Encounters | `feature/no-random-encounters` に 3 file の flag 割り当て実装がある。`master` の `OW_FLAG_NO_ENCOUNTER` はまだ `0`。 | Docs に evidence を残す。実装は `master` へ入れず、必要時に fresh feature / integration branch へ再適用する。 |
| Trainer Battle Aftercare / Battle Item Restore | `feature/trainer-battle-aftercare-heal` に aftercare heal-only hook、berry-inclusive held item restore、focused tests がある。`master` には `B_TRAINER_BATTLE_AFTERCARE` / `B_RESTORE_HELD_BATTLE_BERRIES` が無い。 | Docs に evidence を残す。item restore と aftercare を分ける場合も `master` ではなく fresh branch で取り込む。 |
| Champions Partygen | `feature/trainer-partygen-catalog-expansion` に Rust CLI、catalog、Elite Four / Wallace data diff がある。`master` には `tools/champions_partygen/README.md` だけがある。 | tool / data / generated workflow の review 後、大型 feature / integration branch として扱う。 |

### GitHub PR Queue Snapshot (2026-05-09)

Open PR は「通したい実装候補」だけに絞る。ただし open は merge 許可ではなく、
review / staging shelf として扱う。マージボタンや `gh pr merge` で直接
`master` へ入れる前に、conflict / CI / docs handoff をこの registry と
owning feature docs に反映する。

planned order と PR の粒度がずれている場合は、open PR を直接 merge しない。
`master` に入れるのは docs / `AGENTS.md` の evidence だけ。runtime 実装は
current `master` から fresh feature / integration branch を切り、必要な commit
/ file だけを cherry-pick または再実装する。古い PR は後継 branch が立ってから
close する。

| PR | Branch | State | Action |
|---|---|---|---|
| #10 `Add trainer battle aftercare heal hook` | `feature/trainer-battle-aftercare-heal` | Open, non-draft, mergeable clean after stale PR cleanup. CI long `test` job was still pending when first checked; do not block on it. | 採用候補として残す。ただし item restore と aftercare を分けるなら direct merge ではなく fresh branch で分割する。 |
| #7 `Add Elite Four partygen pools and battlefield lint` | `feature/trainer-partygen-catalog-expansion` | Open, non-draft, checks passed, merge state dirty against current `master`. | 大型 tool/data PR として残す。実装順では no_random / battle-end policy の後。current `master` へ更新してから判断する。 |
| #5 `Add trainer party generator MVP` | `feature/trainer-party-generator` | Closed 2026-05-09, remote branch deleted. | #7 が後継で同一 MVP commit を含むため superseded。 |
| #4 `Add Rouge Cave map draft` | `feature/new-map-test-v15` | Closed 2026-05-09, branch preserved. | CI failure 付き draft map work。今回の feature queue からは外し、再開時は新 branch で復帰する。 |
| #2 `Document v15 source investigation` | `codex-docs-v15-investigation` | Closed 2026-05-09, remote branch deleted. | docs は後続 snapshot / handoff で `master` に反映済み。古い branch は stale diff が大きいため閉じた。 |

### Recommended Implementation Order

| # | 対象 | 期待 status 遷移 | Why first | 依存 |
|---|---|---|---|---|
| 0 | `docs/flows/save_data_flow_v15.md` | Planned を維持 | 既に SaveBlock / saved flag 方針は決定済み。実装 item ではなく、各 branch の gate として参照する。 | なし (docs only) |
| 1 | `docs/features/no_random_encounters/` | Planned → Validated branch / Integration candidate | 影響範囲が最小。`feature/no-random-encounters` の差分は flag rename と config 割り当てのみで、既存 gate / debug toggle を使う。`master` へは実装を入れない。 | save_data flow の flag region 決定済み |
| 2 | `docs/features/battle_item_restore_policy/` | Validated branch → Integration candidate | focused tests と mGBA / manual evidence が既にある。battle 中の item consumption を変えず、battle-end restore policy だけを入れる。`master` へは実装を入れない。 | なし。aftercare と同一 branch 由来だが独立して取り込む |
| 3 | `docs/features/trainer_battle_aftercare/` | Planned / branch implementation → Testing | default off の heal-only hook。battle selection / Champions runtime より先に `CB2_EndTrainerBattle` の guard helper を固める。 | battle item restore の取り込み後に競合を避ける |
| 4 | `docs/features/champions_challenge/` partygen CLI + catalog | Branch implementation → Review / Testing | ROM runtime とは切り離せるが、Rust CLI、catalog、`src/data/trainers.party` の大型差分を含む。generated workflow と data diff review が必要。 | no_random / battle-end policy とは独立 |
| 5 | `docs/features/battle_selection/` | Investigating → Planned (MVP partygen 抜き) | 一時 `gPlayerParty`、callback chain、battle-end restore ordering が絡むため、aftercare helper が固まってから着手する。 | save_data flow + aftercare ordering |
| 6 | `docs/features/field_move_modernization/` | Planned → Slice 1 Implementing | Cut / Rock Smash / Strength / Flash の object interaction から小さく進められる。battle 系とは独立だが field runtime と HM forget policy の確認が必要。 | なし |
| 7 | `docs/features/tm_shop_migration/` | Investigating → Planned | data / script 変更中心だが、販売時期、NPC reward 置換、item ball flag との整合を先に詰める。 | なし |
| 8 | runtime rule options | Investigating → Planned | no_random や aftercare を runtime option に束ねる前に、保存先と UI owner を確定する。 | save_data flow + concrete feature behavior |
| 9 | `docs/features/champions_challenge/` runtime | Planned → Implementing | challenge party / bag / EXP / loss policy / reward state が重い。partygen output と battle selection / aftercare 知見を使ってから入る。 | save_data flow + partygen CLI + battle_selection / aftercare ordering |

このリストは branch 切り替え時に必ず読む。実装着手で順序が変わった場合はこの section を更新する。

## Status Values

| Status | Meaning |
|---|---|
| Investigating | 既存コード調査中 |
| Planned | 設計方針を作成済み、未実装 |
| Implementing | 実装中 |
| Testing | 実装済み、検証中。テストで設計ミスが見つかった場合は docs に戻して計画を更新する |
| Validated branch | 実装と検証 evidence は別 branch にあるが、現行 `master` にはまだ runtime source が入っていない |
| Integration candidate | current `master` から fresh feature / integration branch を切れば再適用できる候補。`master` 自体には source を入れない |
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
| No Random Encounters | Planned | Validated branch exists; not on `master` | `docs/features/no_random_encounters/` | `OW_FLAG_NO_ENCOUNTER` を使い、通常歩行中の land / water random encounter を止める候補。`feature/no-random-encounters` に 3 file 実装と mGBA evidence がある。MVP は step-only。Fishing / Sweet Scent / Rock Smash / static wild battle / option UI は後続扱い。 |
| DexNav / Encounter UI | Investigating | No code changes | `docs/flows/dexnav_flow_v15.md` | Start menu DexNav、detector mode、SaveBlock3、12 land slots、Pokemon icon 描画を整理。 |
| Trainer Party Reorder / Randomizer | Investigating | No code changes | `docs/features/battle_selection/opponent_party_and_randomizer.md` | `DoTrainerPartyPool`、`RandomizePoolIndices`、`AI_FLAG_RANDOMIZE_PARTY_INDICES` を確認。相手 party preview と関係。 |
| TM/HM and Field Move Policy | Investigating | No code changes | `docs/overview/tm_hm_expansion_250_v15.md` | 250 TM 前提の item ID / bag / relearner / field HM coupling を確認。`FOREACH_TM`、`FOREACH_HM`、`ScrCmd_checkfieldmove`、`gFieldMoveInfo`、`CannotForgetMove` も継続参照。 |
| Field Move Modernization / HM Removal | Planned | No code changes | `docs/features/field_move_modernization/` | Gen7/Gen8 風に HM 技所持へ依存しない field move / obstacle / animation / forget restriction を調査。HM ごとの badge / map obstacle / MVP slice 表を `mvp_plan.md` に確定。Slice 1: Cut → Slice 4: Flash まで object interaction で揃えてから、Surf / Waterfall / Dive / Fly を Phase 2-3 で扱う方針。 |
| Champions-style EV/IV Training UI | Investigating | No code changes | `docs/overview/champions_training_ui_feasibility_v15.md` | EV/IV/nature/moveset 編集 UI は実装可能。32 point EV は UI 表示と内部 EV 変換を分ける方針が安全。EV total 518 と wild IV mode も調査済み。 |
| Scout Selection / Starting Battlefield Status | Investigating | No code changes | `docs/overview/scout_selection_and_battlefield_status_v15.md` | Battle Factory / Champions 風の候補 Pokemon 選択 UI、gift mon 付与 flow、trainer flag cleanup、Frontier 風 save / pause / power-cut recovery、held item duplicate restriction、post-battle heal / item restore / PP-EP policy、pickup object / sprite / UI asset 注意点、starting status、PB / ability 強化の調査観点を整理。 |
| Champions Challenge Facility | Planned | Partygen implementation exists on feature branch; runtime not on `master` | `docs/features/champions_challenge/` | 0 匹開始、6 匹作成、Lv.50 battle-only、EXP 無効、bag 退避 / 空 challenge bag、egg-only default eligibility、optional Frontier ban、敗北時 challenge party 破棄と通常 party / bag 復元の仕様を整理。`feature/trainer-partygen-catalog-expansion` に CLI / catalog / trainer data 差分がある。runtime は未実装。 |
| Wild Moveset Randomization | Investigating | No code changes | `docs/overview/wild_moveset_randomization_v15.md` | 野生初期技の現行「最後 4 level-up 技」flow、weighted level-up、TM/tutor 混在、外部 weight table の候補を整理。 |
| Runtime Rule Options | Investigating | No code changes | `docs/overview/runtime_rule_options_feasibility_v15.md` | Nuzlocke、release、difficulty、EXP/catch/shiny 倍率、Mega/Z/Dynamax/Tera、trade、randomizer の option 化候補を整理。 |
| Battle AI Decision / Switching | Investigating | No code changes | `docs/flows/battle_ai_decision_flow_v15.md` | move scoring、smart switching、double battle partner 評価、dynamic AI function の入口を整理。 |
| Roguelike Party / Held Item Policy | Investigating | No code changes | `docs/overview/roguelike_party_policy_impact_v15.md` | 100 戦型 facility、held item lock、item clause、battle item restore、release/swap policy の影響を整理。 |
| Map Registration / Region Map / Fly | Investigating | No code changes | `docs/flows/map_registration_fly_region_flow_v15.md` | 新規 map の `MAPSEC_*`、Town Map/Region Map、Fly icon、visited/world map flag、warp callback を整理。FRLG map preview と Fly 点滅の疑いどころも記録。 |
| NPC / Object Event / Conditional Tiles | Investigating | No code changes | `docs/flows/npc_object_event_flow_v15.md` | `events.inc` の `object_event`、`scripts.inc` の `applymovement` / `setobjectxyperm` / `setmetatile` / `setmaplayoutindex`、Town Map R Fly 後の popup リスクを整理。 |
| Battle Frontier Level Scaling | Investigating | No code changes | `docs/flows/battle_frontier_level_scaling_flow_v15.md` | 現行 Lv.50 course は低レベルを Lv.50 化しない。対戦用に battle-only Lv.50 補正を入れる場合の hook とリスクを整理。 |
| TM Shop Migration | Investigating | No code changes | `docs/features/tm_shop_migration/` | 50 TM 定義と取得元 flag は別物として整理。`FLAG_RECEIVED_TM_*` 21 件、visible TM item ball flag 14 件、hidden TM item flag 1 件を確認。 |
| Custom Items / Moves / Abilities | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | constants、data table、UI、battle behavior、AI、tests への影響範囲を横断 map に整理。 |
| Battle Item Restore Policy | Validated branch | Branch implementation exists; not on `master` | `docs/features/battle_item_restore_policy/` | `feature/trainer-battle-aftercare-heal` に `B_RESTORE_HELD_BATTLE_BERRIES`、`TryRestoreHeldItems()` の berry restore、focused tests、mGBA / manual evidence がある。`master` へは source 未反映。 |
| Trainer Battle Aftercare / Forced Release | Planned / branch implementation | Heal-only branch implementation exists; not on `master` | `docs/features/trainer_battle_aftercare/` | `feature/trainer-battle-aftercare-heal` に `B_TRAINER_BATTLE_AFTERCARE` default off の通常 trainer battle 勝利後 heal-only hook がある。no-whiteout、forced release、battle selection integration は後続。 |
| Callback / Dispatch Audit | Investigating | No code changes | `docs/overview/callback_dispatch_map_v15.md` | `SetMainCallback2`、`CB2_*`、`CreateTask`、`ScrCmd_*`、`special`、field callback の確認用 docs。 |
| Map Script / Flag / Var Audit | Investigating | No code changes | `docs/flows/map_script_flag_var_flow_v15.md` | `map.json`、generated `.inc`、hand-written `scripts.inc`、NPC hide flag、coord/bg event、item ball / hidden item flow を整理。 |
| Move Relearner / Summary Menu | Investigating | No code changes | `docs/flows/move_relearner_flow_v15.md` | summary / party / script からの技思い出し flow、`MAX_RELEARNER_MOVES`、TM 追加リスクを整理。 |
| Save Data / Runtime Flags | Planned | No code changes | `docs/flows/save_data_flow_v15.md` | SaveBlock1/2/3 capacity と FREE_* 切り替え分、flag/var 残量、後続 feature の割り当て先 (no encounter / champions challenge / partygen seed / runtime rule options) を確定。実装は伴わない policy doc。 |
| Pokemon Icon UI | Investigating | No code changes | `docs/flows/pokemon_icon_ui_flow_v15.md` | `CreateMonIcon`、icon palette、sprite lifetime、DexNav / custom UI 影響を整理。 |
| Upstream 1.15.2 Upgrade Impact | Investigating | No code changes | `docs/upgrades/1_15_1_to_1_15_2_impact.md` | `expansion/1.15.2` tag の差分を確認。INCGFX migration、DexNav、map script、battle engine、SaveBlock3 影響を整理。 |

## Rules for Future Entries

- 実装前に `README.md`、`investigation.md`、`mvp_plan.md`、`risks.md`、`test_plan.md` を用意する。
- 新規 feature folder は `docs/templates/feature_folder_template.md` を基準に作る。
- 実装に使う既存 symbol は、推測ではなく実ファイル名と関数名を記録する。
- 実装 branch では、影響範囲と current decision を owning feature docs に更新しながら進める。
- upstream 追従で壊れそうなファイルは `docs/upgrades/upstream_diff_checklist.md` に追加する。
- 未確認事項は各 feature の `Open Questions` に残す。
