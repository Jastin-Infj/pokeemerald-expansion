# Local Config And Flag Ledger

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `ff4e825258`; `git describe` = `expansion/1.15.2-59-gff4e825258` |
| Code status | Docs-only ledger |
| Provenance | Local source read and feature docs |

この ledger は local feature branch が追加・変更する config macro、event flag、
capability flag、save state の索引。`master` の値と branch-only の値を混同しない。

## Config / Flag Table

| Token / state | Type | Owner | Current `master` | Branch / integration note |
|---|---|---|---|---|
| `OW_FLAG_NO_ENCOUNTER` | Event flag id config | `docs/features/no_random_encounters/` | `include/config/overworld.h` で `0`。未割り当て。 | bool ではない。`TRUE` / `1` ではなく明示 flag id を割り当てる。step-only adoption candidate は `FLAG_UNUSED_0x8E5` を `FLAG_NO_ENCOUNTER` (`SYSTEM_FLAGS + 0x85`) に rename して使う。 |
| `I_REUSABLE_TMS` | Item config | `docs/features/tm_shop_migration/` | `include/config/item.h` で `FALSE`。 | PR #31 branch では `TRUE`。既存 50 TM を持っている場合だけ reusable になる。 |
| `P_SHOW_TERA_TYPE` | Pokemon / Summary config | `docs/features/summary_tera_type_icon/` | `include/config/pokemon.h` で `GEN_8`。Summary Tera表示は既定で無効。 | PR #26 branch では Summary Tera badge 用に有効化する前提。 |
| `P_SUMMARY_SCREEN_MOVE_RELEARNER` | Summary config | `docs/features/unified_move_relearner/` | `TRUE`。Move page の `START RELEARN` 表示に使う。 | Unified mode では prompt copy / L-R category cycling の扱いを確認する。 |
| `P_ENABLE_MOVE_RELEARNERS` | Relearner config | `docs/features/unified_move_relearner/` | `FALSE`。egg / TM / tutor category をまとめて有効化しない。 | PR #28 branch は unified source toggles を追加。採用前に default を確認する。 |
| `P_TM_MOVES_RELEARNER` / `P_ENABLE_ALL_TM_MOVES` | Relearner config | `docs/features/unified_move_relearner/` | `FALSE` / `FALSE`。 | Virtual TM pool と physical TM item ownership を分ける。 |
| `P_FLAG_EGG_MOVES` / `P_FLAG_TUTOR_MOVES` | Event flag id config | `docs/features/unified_move_relearner/` | `0` / `0`。未割り当て。 | Runtime unlock に使うなら flag id を割り当て、save/flag ledger に追記する。 |
| `P_PARTY_MOVE_RELEARNER` | Party menu config | `docs/features/unified_move_relearner/` | `FALSE`。 | PR #28 branch は party entry を使う。field move / party action order と一緒に確認する。 |
| `P_UNIFIED_MOVE_RELEARNER` | Branch-only Summary config | `docs/features/unified_move_relearner/` | Not present on `master`。 | PR #28 implementation docs に記録あり。採用時に exact default と per-source toggles を確認する。 |
| `P_SUMMARY_STATE_EDITOR_*` | Branch-only Summary config | `docs/features/pokemon_state_editor/` | Not present on `master`。 | PR #23 branch owns editor enable, layout, palette, slide, level cap/edit defines. |
| `B_TRAINER_BATTLE_SELECTION` | Branch-only battle config | `docs/features/battle_selection/`, `docs/features/prebattle_team_viewer/` | Not present on `master`。 | Battle selection branch uses `TRUE` for validation. Integration default must be chosen with team viewer. |
| `B_PREBATTLE_TEAM_VIEWER` | Branch-only battle config | `docs/features/prebattle_team_viewer/` | Not present on `master`。 | Enables pre-battle viewer / selection surface in PR #20 branch. |
| `B_IN_BATTLE_TEAM_VIEWER` | Branch-only battle config | `docs/features/prebattle_team_viewer/` | Not present on `master`。 | Enables read-only action-menu viewer in PR #20 branch. |
| `B_TEAM_VIEWER_BUTTON` | Branch-only button config | `docs/features/prebattle_team_viewer/` | Not present on `master`。 | Current branch docs use `R_BUTTON`. Check conflicts with Move Info / L=A before adoption. |
| `B_TEAM_VIEWER_DETAILS_BUTTON` | Branch-only button config | `docs/features/prebattle_team_viewer/` | Not present on `master`。 | Current branch docs use `SELECT_BUTTON` for Champions-style detail / Summary because GBA has no Y button. |
| `B_RESTORE_HELD_BATTLE_BERRIES` | Branch-only battle config | `docs/features/battle_item_restore_policy/` | Not present on `master`。 | Re-applied on `feature/battle-item-restore-current-master-20260519` with default `TRUE` per user direction. Reconfirm only if adopting a conservative integration default. |
| Held item catalog mode | Future runtime / UI policy | `docs/features/nonconsumable_held_items/` | Not present on `master`。 | Planned docs-only. Would let one owned / unlocked held item be assigned to multiple Pokemon without Bag quantity transfer. Must own Party / Bag / Storage item paths before source adoption. |
| `B_TRAINER_BATTLE_AFTERCARE` | Branch-only battle config | `docs/features/trainer_battle_aftercare/` | Not present on `master`。 | Heal-only branch default is `FALSE`; forced release / no-whiteout are future work. |
| Field Kit capability flags | Branch-only event/capability flags | `docs/features/field_move_modernization/` | Not present on `master` as final capability model. | Field Kit branch owns capability flags; TM Shop Migration only retires old HM receive flags. |
| Old HM receive flags | Legacy event flags | `docs/features/tm_shop_migration/` | Still present in current `master` source as legacy Emerald/FRLG routes. | PR #31 retires Emerald normal-progression refs and returns old values to `FLAG_UNUSED_0x...`; FRLG routes are follow-up. |
| Virtual TM unlock state | Future save / flag state | `docs/features/unified_move_relearner/` | Not present. | If story/rank unlocks are needed, use compact bitset or explicit flags; see `save_data_flow_v15.md`. |

## Save State Notes

- Prefer existing event flags for simple route unlocks.
- `OW_FLAG_NO_ENCOUNTER` is a compile-time pointer to an event flag id, not a
  saved bool. Runtime code should set / clear the assigned event flag.
- For hundreds of virtual TM unlocks, do not add bag items just to represent ownership.
- If per-move virtual TM unlocks are required, use compact save state and document
  the owner in [Save Data Flow](../flows/save_data_flow_v15.md).
- Capability flags for field moves belong to Field Move Modernization, not TM Shop
  Migration. TM Shop can retire old HM item grants, but should not silently own new
  movement unlock semantics.

## Open Questions

- Which integration default should own `B_TRAINER_BATTLE_SELECTION` when Team Viewer
  and Battle Selection are adopted together?
- Should `B_RESTORE_HELD_BATTLE_BERRIES` remain default `TRUE` in a future
  integration branch, or become opt-in for challenge modes?
- Should held item catalog mode be global, debug-only, or owned by a future
  Champions / Training Dojo facility?
- Should virtual TM unlocks use saved bitsets, grouped story flags, or always-on
  generated pools for the first shipped baseline?
