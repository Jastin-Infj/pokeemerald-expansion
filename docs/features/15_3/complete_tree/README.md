# 1.15.3 CompleteTree

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| PR | #68 `[codex] Runtime integration staging` |
| Visual summary | [html/index.html](html/index.html) |
| Config / Debug | [config_debug_matrix.md](config_debug_matrix.md) |

この CompleteTree は、1.15.3 で作ってきたもののうち、現在の integration branch
に統合済みとして扱うものをまとめた tree です。

## Completion Rule

| Mark | Meaning |
|---|---|
| Complete | #68 に実装が入っている、または同等以上の実装が #68 に統合済み。 |
| Superseded by #68 | 古い open PR / closed PR / branch は evidence shelf として残すが、個別 merge せず #68 を正とする。 |
| Tool release done | ROM runtime ではないが、tooling として release / artifact 導線まで完了。 |
| Docs retained | feature docs は canonical path に残し、ここから参照する。 |

## Complete Runtime / Tooling Set

| Status | Feature | PR / branch evidence | Canonical docs | 1.15.3 result |
|---|---|---|---|---|
| Complete | Battle Item Restore | #47 / `feature/battle-item-restore-current-master-20260519` | [battle_item_restore_policy](../../battle_item_restore_policy/) | Battle-end held item / berry restore policy is integrated. |
| Complete | Nonconsumable Held Items | #48 / `feature/held-item-catalog-current-master-20260519` | [nonconsumable_held_items](../../nonconsumable_held_items/) | Held-effect items can use ownership-token semantics. |
| Complete | Party / Status UI 2x3 | #54 / `feature/party-status-ui-overhaul-20260521` | [party_status_ui_overhaul](../../party_status_ui_overhaul/) | 2x3 party screen baseline is integrated. |
| Complete | Scout Selection | #51 / `feature/scout-selection-runtime-20260520` | [scout_selection](../../scout_selection/) | 12-candidate / 6-visible / Summary-preview scout UI is integrated. |
| Complete | Friendly Shop Pokemon Vendor | #57 / `feature/global-no-evolution-20260523` | [friendly_shop_pokemon_vendor](../../friendly_shop_pokemon_vendor/) | Pokemon vendor, sealed recruits, bond EXP, icons, long-list fixes are integrated. |
| Complete | All Ability Slots | #60 / `feature/all-ability-slots-runtime-20260523` | [all_ability_slots](../../all_ability_slots/) | Battle all-slot engine and runtime debug toggle are integrated. |
| Complete | Champions Run Session | #62 / `feature/champions-run-session-runtime-20260524` | [champions_challenge](../../champions_challenge/) | Run snapshot / checkpoint / retire / loss / clear restore is integrated. |
| Complete | No Random Encounters | #41 / `feature/no-random-encounters-step-only-runtime-20260517` | [no_random_encounters](../../no_random_encounters/) | `FLAG_NO_ENCOUNTER` is wired to step random encounter suppression. |
| Complete | TM Shop Migration | #31 / `feature/tm-shop-migration` | [tm_shop_migration](../../tm_shop_migration/) | Reusable TM and old TM/HM acquisition policy are integrated. |
| Complete | Unified Move Relearner | #28 / `feature/unified-move-relearner` | [unified_move_relearner](../../unified_move_relearner/) | Summary-first all-source move relearner is integrated. |
| Complete | Pre-Battle / In-Battle Team Viewer | #20 / `feature/prebattle-team-viewer`, `feature/prebattle-team-viewer-phase2` | [prebattle_team_viewer](../../prebattle_team_viewer/) | Opponent preview, Summary transition, selection, and in-battle viewer are integrated. |
| Complete | Trainer Battle Selection | `feature/battle-selection-mvp` | [battle_selection](../../battle_selection/) | Normal trainer selection and short-party support are integrated. |
| Complete | Pokemon State Editor | #23 / `feature/pokemon-state-editor-expansion` | [pokemon_state_editor](../../pokemon_state_editor/) | Summary Skills-page editor is integrated. |
| Complete | Summary Tera Type Badge | #26 / `feature/summary-tera-type-badge` | [summary_tera_type_icon](../../summary_tera_type_icon/) | Summary Info-page Tera badge is integrated. |
| Complete | Trainer Battle Aftercare | #10 / `feature/trainer-battle-aftercare-heal` | [trainer_battle_aftercare](../../trainer_battle_aftercare/) | Normal trainer win heal hook is integrated and enabled in runtime lane. |
| Complete | Field Move Modernization / Field Kit | `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item` | [field_move_modernization](../../field_move_modernization/) | HM-free field moves and Field Kit are integrated. |
| Complete | Battle BGM Selector / Sound Archive | #39 / `feature/battle-bgm-selector-mvp-20260517` | [battle_bgm_selector](../../battle_bgm_selector/) | BGM selector, imported tracks, and sound tool fixes are integrated. |
| Complete | Trainer Partygen Catalog | #7 / `feature/trainer-partygen-catalog-expansion` | [champions_challenge partygen](../../champions_challenge/partygen_impact_and_next_steps.md) | Partygen catalog / generated trainer pool support is integrated. |
| Tool release done | Map Asset Relinker | #65 / `feature/map-asset-relinker-20260525` | [map_asset_relinker](../../map_asset_relinker/) | CUI / Rust core / Tauri GUI / Windows release workflow are integrated; `map-asset-relinker-v0.1.0` release is published. |

## Open PR Handling

| PR | Handling |
|---|---|
| #47 / #48 / #51 / #54 / #57 / #60 / #62 | Superseded by #68 for runtime integration. Keep until user decides to close old shelves. |
| #65 | Tooling work is integrated into #68 and released through `map-asset-relinker-v0.1.0`. Keep only if separate tool PR history is still useful. |
| #68 | Current 1.15.3 runtime integration staging PR. |

## Related 15.3 Docs

- [Runtime Summary Markdown](runtime_summary.md)
- [Runtime Summary HTML](html/index.html)
- [Config / Debug Matrix](config_debug_matrix.md)
- [1.15.3 Runtime Integration Checklist](runtime_integration_15_3_checklist_ja_2026_05_30.md)
- [Runtime Integration Branch Audit](runtime_integration_branch_audit_2026_05_29.md)

## Validation Snapshot

The integration evidence is recorded in the owning feature `test_plan.md` files
and in [runtime_summary.md](runtime_summary.md). The branch has local `all`,
`debug`, full `check`, docs build, codex review follow-ups, and focused mGBA
Live smoke evidence. GitHub CI failures are not treated as 1.15.3 feature
blockers here; they are tracked as 16.0 OpenTree workflow cleanup.
