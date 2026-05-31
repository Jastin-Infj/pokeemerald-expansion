# 1.15.3 Completed Feature Inventory

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| PR | #68 `[codex] Runtime integration staging` |
| Status meaning | `Completed` means the feature is implemented in #68 or superseded by #68 for the 1.15.3 runtime integration lane. |

この file は「結局どれを実装済み扱いにしたのか」を見るための一覧です。
ファイル名を `completed_...` に寄せているものは、1.15.3 の完了棚として固定します。

## Completed Runtime / Tooling Set

| Status | Feature | PR / branch evidence | Canonical docs | 1.15.3 result |
|---|---|---|---|---|
| Completed | Battle Item Restore | #47 / `feature/battle-item-restore-current-master-20260519` | [battle_item_restore_policy](../../battle_item_restore_policy/) | Battle-end held item / berry restore policy is integrated. |
| Completed | Nonconsumable Held Items | #48 / `feature/held-item-catalog-current-master-20260519` | [nonconsumable_held_items](../../nonconsumable_held_items/) | Held-effect items can use ownership-token semantics. |
| Completed | Party / Status UI 2x3 | #54 / `feature/party-status-ui-overhaul-20260521` | [party_status_ui_overhaul](../../party_status_ui_overhaul/) | 2x3 party screen baseline is integrated. |
| Completed | Scout Selection | #51 / `feature/scout-selection-runtime-20260520` | [scout_selection](../../scout_selection/) | 12-candidate / 6-visible / Summary-preview scout UI is integrated. |
| Completed | Friendly Shop Pokemon Vendor | #57 / `feature/global-no-evolution-20260523` | [friendly_shop_pokemon_vendor](../../friendly_shop_pokemon_vendor/) | Pokemon vendor, sealed recruits, bond EXP, icons, long-list fixes are integrated. |
| Completed | All Ability Slots | #60 / `feature/all-ability-slots-runtime-20260523` | [all_ability_slots](../../all_ability_slots/) | Battle all-slot engine and runtime debug toggle are integrated. |
| Completed | Champions Run Session | #62 / `feature/champions-run-session-runtime-20260524` | [champions_challenge](../../champions_challenge/) | Run snapshot / checkpoint / retire / loss / clear restore is integrated. |
| Completed | No Random Encounters | #41 / `feature/no-random-encounters-step-only-runtime-20260517` | [no_random_encounters](../../no_random_encounters/) | `FLAG_NO_ENCOUNTER` is wired to step random encounter suppression. |
| Completed | TM Shop Migration | #31 / `feature/tm-shop-migration` | [tm_shop_migration](../../tm_shop_migration/) | Reusable TM and old TM/HM acquisition policy are integrated. |
| Completed | Unified Move Relearner | #28 / `feature/unified-move-relearner` | [unified_move_relearner](../../unified_move_relearner/) | Summary-first all-source move relearner is integrated. |
| Completed | Pre-Battle / In-Battle Team Viewer | #20 / `feature/prebattle-team-viewer`, `feature/prebattle-team-viewer-phase2` | [prebattle_team_viewer](../../prebattle_team_viewer/) | Opponent preview, Summary transition, selection, and in-battle viewer are integrated. |
| Completed | Trainer Battle Selection | `feature/battle-selection-mvp` | [battle_selection](../../battle_selection/) | Normal trainer selection and short-party support are integrated. |
| Completed | Pokemon State Editor | #23 / `feature/pokemon-state-editor-expansion` | [pokemon_state_editor](../../pokemon_state_editor/) | Summary Skills-page editor is integrated. |
| Completed | Summary Tera Type Badge | #26 / `feature/summary-tera-type-badge` | [summary_tera_type_icon](../../summary_tera_type_icon/) | Summary Info-page Tera badge is integrated. |
| Completed | Trainer Battle Aftercare | #10 / `feature/trainer-battle-aftercare-heal` | [trainer_battle_aftercare](../../trainer_battle_aftercare/) | Normal trainer win heal hook is integrated and enabled in runtime lane. |
| Completed | Field Move Modernization / Field Kit | `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item` | [field_move_modernization](../../field_move_modernization/) | HM-free field moves and Field Kit are integrated. |
| Completed | Battle BGM Selector / Sound Archive | #39 / `feature/battle-bgm-selector-mvp-20260517` | [battle_bgm_selector](../../battle_bgm_selector/) | BGM selector, imported tracks, and sound tool fixes are integrated. |
| Completed | Trainer Partygen Catalog | #7 / `feature/trainer-partygen-catalog-expansion` | [champions_challenge partygen](../../champions_challenge/partygen_impact_and_next_steps.md) | Partygen catalog / generated trainer pool support is integrated. |
| Tool completed | Map Asset Relinker | #65 / `feature/map-asset-relinker-20260525` | [map_asset_relinker](../../map_asset_relinker/) | CUI / Rust core / Tauri GUI / Windows release workflow are integrated; `map-asset-relinker-v0.1.0` release is published. |

## PR Handling

| PR | Handling |
|---|---|
| #47 / #48 / #51 / #54 / #57 / #60 / #62 | Closed on 2026-05-31 as superseded by #68 for runtime integration. Keep the closed PRs / branches as evidence shelves only. |
| #65 | Closed on 2026-05-31 as superseded by #68 for integration tracking and by `map-asset-relinker-v0.1.0` for tool distribution. |
| #68 | Current 1.15.3 runtime integration staging PR. |
