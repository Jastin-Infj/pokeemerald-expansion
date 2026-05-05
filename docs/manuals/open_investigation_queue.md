# Open Investigation Queue

この文書は、今後の branch で優先的に追加調査する候補をまとめる。
各項目は、実装へ進む前に owning feature docs へ移すか、既存 feature docs の `Open Questions` / `risks.md` / `test_plan.md` に反映する。

## High Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| party generator input design | `docs/features/battle_selection/` | global set、blueprint、materialized pool、lint の入力形式が固まらないと実装と運用がぶれる。 | catalog / blueprint / rule dictionary の最小 schema と sample を作る。 |
| `trainer.party` integration | `docs/features/battle_selection/` | rename / replace 方式と generated file 参照方式で build、review、upstream merge、team display 影響が変わる。 | trainerproc と build path を確認し、Plan A / B の差分を決める。 |
| opponent preview / team display impact | `docs/features/battle_selection/` | MVP からは分離するが、後続で preview と battle 実体を一致させる必要がある。 | `gEnemyParty` 生成 timing、seed、pool randomize 再現性を調査する。 |
| battle selection restore timing | `docs/features/battle_selection/` | battle 後の level up、move learn、evolution、whiteout、cancel で party restore が壊れやすい。 | `CB2_EndTrainerBattle` 以降の反映 timing を focused に読む。 |
| SaveBlock / runtime option policy | `docs/flows/save_data_flow_v15.md` | seed、option、challenge state、no encounter option の保存場所を決める必要がある。 | make output の free space、SaveBlock2/3 の project policy、migration 方針を確認する。 |

## Medium Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| no random encounters scope | `docs/features/no_random_encounters/` | step-only MVP と broad-wild mode の境界を決めないと option 名が曖昧になる。 | Fishing / Sweet Scent / Rock Smash / static wild battle の call path を分ける。 |
| HM / field move modernization | `docs/features/field_move_modernization/` | field move 廃止、badge/key item 判定、release 制限、Secret Power の扱いが連鎖する。 | Cut / Rock Smash / Strength / Surf / Dive / Waterfall ごとに in/out を決める。 |
| TM shop migration | `docs/features/tm_shop_migration/` | 全 TM の販売時期、既存 reward、visible / hidden item 置換、save flag の扱いが未確定。 | stage unlock、price tier、NPC/gym reward replacement の候補を整理する。 |
| generated data rebuild flow | `docs/manuals/generated_data_workflow.md` | partygen 以外にも shop / encounter randomizer で同じ問題が起きる。 | CLI doctor / lint / diff / drift check の共通 contract を feature に適用する。 |
| docs organization | `docs/manuals/docs_navigation.md` | manual / tutorial / feature / overview / flow の境界が曖昧だと重複が増える。 | 移動ではなく index / navigation / template の更新で整理する。 |

## Lower Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| battle UI party status display | `docs/flows/battle_ui_flow_v15.md` | 選出後に 3/4 匹表示にするか、既存 6 slot 表示を許容するかで UI 変更量が変わる。 | MVP では既存表示を許容し、後続 UI branch で再調査する。 |
| custom selection UI | `docs/features/battle_selection/` | Champions 風 UI は見た目と input state の設計が重い。 | 既存 party menu branch で足りるか、専用 UI が必要かを確認する。 |
| external rogue references | `docs/features/trainer_battle_aftercare/` | 強制 release / wipeout 実装の参考になる可能性がある。 | 必要になった段階で external repo の source path を確認する。 |

## How to Use This Queue

1. 実装対象に関係する項目を選ぶ。
2. owning feature docs に移す。
3. `investigation.md` に確認した file / symbol を書く。
4. `mvp_plan.md` に採用方針を書く。
5. `risks.md` に未解決項目と accepted risk を残す。
6. 解決済みになったら、この queue から削除するか status を下げる。
