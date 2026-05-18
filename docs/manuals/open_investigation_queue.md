# Open Investigation Queue

この文書は、今後の branch で優先的に追加調査する候補をまとめる。
各項目は、実装へ進む前に owning feature docs へ移すか、既存 feature docs の `Open Questions` / `risks.md` / `test_plan.md` に反映する。

## Current Open Implementation Shelves (2026-05-17)

2026-05-17 cleanup 後、GitHub 上で open PR として残っている runtime
implementation shelf は 0 件。closed PR / PR-less branch に実装済み shelf があるため、
終わっているかどうかは [Implementation Shelf Audit](../features/implementation_shelf_audit_2026_05_17.md) と
owning feature docs で確認する。`master` へ直接 source を入れる許可ではない。

| Feature | PR / branch | Current state | Remaining investigation |
|---|---|---|---|
| None | - | - | - |

The runtime PRs #41 / #39 / #31 / #28 / #26 / #23 / #20 were closed on 2026-05-17
as completed implementation shelves after successful CI. Their branches remain
preserved; future adoption should start from a fresh integration branch or a
selected cherry-pick.

## High Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| battle item restore adoption defaults | `docs/features/battle_item_restore_policy/` | Closed PR #14 and older branch evidence use `B_RESTORE_HELD_BATTLE_BERRIES TRUE`, while conservative branch policy may prefer default off. | Decide default TRUE/FALSE before source re-apply; keep the decision in `adoption_investigation_2026_05_09.md` and `mvp_plan.md`. |
| nonconsumable held item catalog policy | `docs/features/nonconsumable_held_items/` | User wants a Champions-style held item policy where battle items do not disappear and one item can be assigned without physical quantity friction. Battle-end restore and catalog assignment touch different code paths. | Implement battle-end restore first if selected. Before catalog runtime, decide global/debug/facility scope and audit Party / Bag / Storage Take/Give/Switch/Toss behavior. |
| trainer aftercare focused test gate | `docs/features/trainer_battle_aftercare/` | The source hook is small but touches `CB2_EndTrainerBattle`, a central callback for win/loss/facility return. | Add focused config-off, normal-win, and exclusion-path tests before adopting the aftercare slice. |
| unified move relearner adoption policy | `docs/features/unified_move_relearner/` | Completed shelf #28 implements the candidate builder and long-list handling. Policy decisions still affect runtime semantics. | Resolve conflicts after TM Shop Migration, then decide virtual TM unlock default, special-source labels, and manual teach/overwrite validation coverage. |
| pokemon state editor adoption polish | `docs/features/pokemon_state_editor/` | Completed shelf #23 implements the party Summary MVP. Remaining questions are box support, redraw artifacts, legality locks, and whether this shelf is still the desired next adoption point. | Confirm box-summary policy and remaining UI/data polish before re-applying or replacing the shelf. |

### Resolved (2026-05-18)

| Topic | Resolution |
|---|---|
| ~~prebattle team viewer pool validation~~ | Source audit of `feature/prebattle-team-viewer` confirms the mechanism is implemented: `PreBattleTeamViewer_Begin()` builds the preview cache through `CreateNPCTrainerPartyForPreview()`, the party generation path calls `DoTrainerPartyPool()`, and battle init copies the same cache through `PreBattleTeamViewer_LoadCachedOpponentParty()`. Keep optional automated / mGBA regression evidence for adoption, but do not track this as missing implementation. |
| ~~partygen / pool randomizer implemented state~~ | Closed PRs #5 and #7 are implementation shelves, not abandoned plans. #7 supersedes #5 and carries Rust CLI, catalog, Trainer Party Pool output for Elite Four / Wallace, lint data, local checks, and mGBA ROM-memory evidence. |

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
| ~~HM / field move modernization scope~~ | `docs/features/field_move_modernization/` | Resolved 2026-05-05: `mvp_plan.md#Per-HM Decision Table` で HM ごとの badge / map obstacle / MVP slice 順序 (Cut → Rock Smash → Strength → Flash → ... → Fly) を確定。README now records implemented MVP / Field Kit branches, so this is not an unimplemented investigation item. | — |
| TM shop migration follow-up scope | `docs/features/tm_shop_migration/` | Completed shelf #31 implements the Emerald normal-progression retirement slice. Legacy Gen 3 TM acquisition is no longer just an investigation item. | Decide whether to widen scope to FRLG-specific routes, and keep 200+ Gen 9 reusable TM itemization as a separate feature. |
| summary tera type icon merge-state check | `docs/features/summary_tera_type_icon/` | Completed shelf #26 is the display-only implementation shelf. | Resolve conflicts and asset credit before selecting it for implementation merge. |
| generated data rebuild flow | `docs/manuals/generated_data_workflow.md` | partygen 以外にも shop / encounter randomizer で同じ問題が起きる。 | CLI doctor / lint / diff / drift check の共通 contract を feature に適用する。 |
| partygen player-style optimization | `docs/features/champions_challenge/partygen_player_style_logging.md` | Player style に合わせた partygen tuning は最適化 branch として catalog / profile / validation の境界を決める必要がある。 | style signal の入力形式、重み更新、regression lint を設計してから partygen branch に入る。 |
| forced release system | `docs/overview/roguelike_party_policy_impact_v15.md` / future feature docs | 禁止条件に入った Pokemon を自動で逃がす rule は party state、PC storage、challenge aftercare、player messaging に跨る。 | release 条件、保護対象、PC box 処理、battle end / field return の実行タイミングを owning feature docs に切り出す。 |
| docs organization | `docs/manuals/docs_navigation.md` | manual / tutorial / feature / overview / flow の境界が曖昧だと重複が増える。 | 移動ではなく index / navigation / template の更新で整理する。 |

## Lower Priority

| Topic | Current owner | Why it matters | Next investigation |
|---|---|---|---|
| battle UI party status display | `docs/flows/battle_ui_flow_v15.md` | 選出後に 3/4 匹表示にするか、既存 6 slot 表示を許容するかで UI 変更量が変わる。 | MVP では既存表示を許容し、後続 UI branch で再調査する。 |
| custom selection UI | `docs/features/battle_selection/` | Champions 風 UI は見た目と input state の設計が重い。MVP は既存 choose-half UI で成立した。 | `feature/party-select-ui` は古い `vanilla/v14_1` 系 prototype として参考に留め、専用 UI が必要になった時だけ現行 branch へ再設計する。 |
| external rogue references | `docs/features/trainer_battle_aftercare/` | 強制 release / wipeout 実装の参考になる可能性がある。 | 必要になった段階で external repo の source path を確認する。 |

## How to Use This Queue

1. 実装対象に関係する項目を選ぶ。
2. owning feature docs に移す。
3. `investigation.md` に確認した file / symbol を書く。
4. `mvp_plan.md` に採用方針を書く。
5. `risks.md` に未解決項目と accepted risk を残す。
6. 解決済みになったら、この queue から削除するか status を下げる。
