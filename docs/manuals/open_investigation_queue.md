# Open Investigation Queue

この文書は、今後の branch で優先的に追加調査する候補をまとめる。
各項目は、実装へ進む前に owning feature docs へ移すか、既存 feature docs の `Open Questions` / `risks.md` / `test_plan.md` に反映する。

## High Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| opponent preview / team display impact | `docs/features/battle_selection/` | MVP からは分離するが、後続で preview と battle 実体を一致させる必要がある。 | `gEnemyParty` 生成 timing、seed、pool randomize 再現性を調査する。 |

### Resolved (2026-05-05)

| Topic | Resolution |
|---|---|
| ~~party generator input design~~ | `docs/features/champions_challenge/mvp_plan.md#Catalog Schema (MVP draft)` で sets / blueprints / journey / rulesets の最小 4 種 JSON schema と sample を確定。 |
| ~~`trainer.party` integration~~ | `docs/features/champions_challenge/mvp_plan.md#trainers.party Integration: Plan A vs Plan B` で `trainer_rules.mk` の build path、CPP `#include` 経由、duplicate id 検出 (`-Werror -Woverride-init`) まで確認し、MVP は Plan A 採用と確定。 |
| ~~battle selection restore timing~~ | `docs/features/battle_selection/investigation.md#Restore Timing (CB2_EndTrainerBattle)` で `HandleBattleVariantEndParty` / `SavePlayerParty` / `LoadPlayerParty` を確認し、`SaveBlock1.playerParty` が backup buffer ではなく persistent slot 自体だと判明。Sky Battle pattern を流用する方針で確定。 |
| ~~SaveBlock / runtime option policy~~ | `docs/flows/save_data_flow_v15.md#Allocation Decision Summary` で 4 feature の owner block を確定。SaveBlock1/2/3 上限、`FREE_*` recoverable bytes (3790 B 合計)、saved flag/var headroom を audit 済み。 |

## Medium Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| no random encounters scope | `docs/features/no_random_encounters/` | step-only MVP と broad-wild mode の境界を決めないと option 名が曖昧になる。 | Fishing / Sweet Scent / Rock Smash / static wild battle の call path を分ける。MVP scope は `mvp_plan.md` で確定済み、broad mode の追加 hook は実装着手時に再調査。 |
| ~~HM / field move modernization scope~~ | `docs/features/field_move_modernization/` | Resolved 2026-05-05: `mvp_plan.md#Per-HM Decision Table` で HM ごとの badge / map obstacle / MVP slice 順序 (Cut → Rock Smash → Strength → Flash → ... → Fly) を確定。 | — |
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
