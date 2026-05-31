# 1.15.3 Runtime Integration Summary

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Branch | `integration/runtime-dev-20260529` |
| PR | #68 `[codex] Runtime integration staging` |
| Base policy | `master` is docs / Lua-only. Runtime source stays on the integration lane. |
| Japanese HTML | [html/index.html](html/index.html) |
| Config / Debug matrix | [config_debug_matrix.md](config_debug_matrix.md) |

この folder は、1.15.3 期間にこちらで追加・統合した runtime / tooling 差分を
日本語で読むためのまとめです。詳細な採用履歴は
[1.15.3 Runtime Integration Checklist](runtime_integration_15_3_checklist_ja_2026_05_30.md)
と [Runtime Integration Branch Audit](runtime_integration_branch_audit_2026_05_29.md)
に残し、この summary では「何が増えたか」「何が切り替え可能か」「どこから確認するか」を優先しています。

## Executive Summary

`integration/runtime-dev-20260529` は、1.15.3 系で作った主要 runtime feature を
ほぼ集約した統合 branch です。大きく分けると次の 6 系統です。

| Group | Added / Changed |
|---|---|
| Battle / team flow | Trainer battle selection、Pre-Battle / In-Battle Team Viewer、battle item restore、trainer battle aftercare、All Ability Slots |
| Pokemon ownership / edit | Friendly Shop Pokemon Vendor、sealed recruit、nonconsumable held item catalog、Pokemon State Editor、global no evolution |
| Summary / move UX | Unified Move Relearner、Summary Tera badge、All Ability Summary slot switch |
| Challenge runtime | Champions run session、checkpoint / retire / clear restore、clear reward carryover |
| Field / map / encounter | Field Kit / HM-free field moves、No Random Encounters flag、Map Asset Relinker、Region Map Fly icon palette-blink support |
| Tools / data | Trainer partygen catalog、Scout pool generator、learnset generator、Battle BGM selector and imported sound assets、Map Asset Relinker Tauri GUI release workflow |

## Added

| Feature | What was added | Entry docs |
|---|---|---|
| Battle Item Restore | Battle-end held item / berry restore policy. | [battle_item_restore_policy](../../battle_item_restore_policy/) |
| Nonconsumable Held Items | Held-effect items can behave like one ownership token instead of physical quantities. | [nonconsumable_held_items](../../nonconsumable_held_items/) |
| Party / Status UI 2x3 | Party screen layout changed from `1 + 5` to `2 / 2 / 2`, with switch redraw fixes. | [party_status_ui_overhaul](../../party_status_ui_overhaul/) |
| Scout Selection | Pokemon Champions-style selection pool with up to 12 candidates, 6 visible rows, scroll, Summary preview, and configurable pick count. | [scout_selection](../../scout_selection/) |
| Pokemon Vendor | Shop command for Pokemon products, normal / sealed recruit delivery, one-time rows, long-list scrolling, icon display, locked PC / Summary / party handling, bond EXP rewards. | [friendly_shop_pokemon_vendor](../../friendly_shop_pokemon_vendor/) |
| All Ability Slots | Battle-only mode where ability slots 1 / 2 / hidden can all apply, plus runtime debug toggle and focused A-U test battles. | [all_ability_slots](../../all_ability_slots/) |
| Champions Run Session | Roguelike run snapshot / restore, temporary reports, retire / lose / clear behavior, bag / party / money restore, clear deposit and carryover options. | [champions_challenge](../../champions_challenge/) |
| No Random Encounters | Assigns `OW_FLAG_NO_ENCOUNTER` so the existing debug toggle can actually disable step-based random encounters. | [no_random_encounters](../../no_random_encounters/) |
| TM Shop Migration | Reusable TM policy and old TM/HM acquisition retirement for the local integration lane. | [tm_shop_migration](../../tm_shop_migration/) |
| Unified Move Relearner | Summary-first all-source move relearn list, including level / egg / TM / tutor / special candidates. | [unified_move_relearner](../../unified_move_relearner/) |
| Pre-Battle / In-Battle Team Viewer | Opponent team preview, Summary transition, selection flow, in-battle read-only team viewer. | [prebattle_team_viewer](../../prebattle_team_viewer/) |
| Trainer Battle Selection | Normal trainer battle party selection with 3/4 default, short-party `1/1` / `2/2` support, and restore handling. | [battle_selection](../../battle_selection/) |
| Pokemon State Editor | Summary Skills page editor for level and Pokemon state editing, gated by summary config. | [pokemon_state_editor](../../pokemon_state_editor/) |
| Summary Tera Badge | Tera type badge display on Summary Info page. | [summary_tera_type_icon](../../summary_tera_type_icon/) |
| Trainer Battle Aftercare | Configurable post-win party heal hook for normal trainer battles. | [trainer_battle_aftercare](../../trainer_battle_aftercare/) |
| Field Move Modernization / Field Kit | HM-free field move flow, Field Kit key item, capability flags, and debug unlock routes. | [field_move_modernization](../../field_move_modernization/) |
| Battle BGM Selector | Trainer / wild BGM selector, imported DPPt / Platinum / HGSS / BW battle tracks, sound tool fixes. | [battle_bgm_selector](../../battle_bgm_selector/) |
| Map Asset Relinker | CUI relinker, Rust core, Tauri GUI, test data, Windows portable release workflow. | [map_asset_relinker](../../map_asset_relinker/) |

## Changed

| Area | Change |
|---|---|
| `master` policy | Runtime source is still not merged to `master`. This branch is the runtime integration candidate. |
| Battle ability checks | Battle helper paths now understand All Ability Slots while preserving representative `abilityNum` semantics. |
| Battle defeat flow | Trainer Battle Selection snapshots selected-party defeat before party restore so whiteout / Champions loss logic does not get hidden by restored bench Pokemon. |
| Item quantities | Non-mail held-effect items can be assigned without consuming / duplicating physical Bag quantities. |
| TM behavior | `I_REUSABLE_TMS` is enabled in the integration lane. |
| Evolution | Ordinary evolutions are globally disabled for this game direction. Battle form-change gimmicks remain separate. |
| Summary ownership | Summary is the canonical entry point for move relearn, state editor, Tera badge, and ability-slot display. |
| SaveBlock usage | Champions run session uses SaveBlock1 space and requires freeing Mystery Event / Mystery Gift buffers. |
| Debug menu | Generic Script 1 / 2 / 3 usage was renamed into feature-specific routes where possible. |
| Tool release | `map-asset-relinker-v0.1.0` tag triggers GitHub Actions to build and publish the Windows portable zip to Releases. |

## Config And Runtime Toggles

The full table is in [config_debug_matrix.md](config_debug_matrix.md). The most important switches are:

| Token | Current integration value | Meaning |
|---|---:|---|
| `B_ALL_ABILITY_SLOTS` | `FALSE` | Default battle behavior remains single representative ability. |
| `B_ALL_ABILITY_SLOTS_RUNTIME_TOGGLE` | `TRUE` | Debug menu can override All Ability Slots per save. |
| `B_TRAINER_BATTLE_SELECTION` | `TRUE` | Normal trainer battles can open selection/team viewer. |
| `B_TRAINER_BATTLE_SELECTION_SHORT_PARTY_MIN_COUNT` | `1` | Single battle can use `1/1` selection. |
| `B_TRAINER_BATTLE_SELECTION_SHORT_DOUBLE_MIN_COUNT` | `2` | Double battle requires 2 eligible mons by default. |
| `B_PREBATTLE_TEAM_VIEWER` / `B_IN_BATTLE_TEAM_VIEWER` | `TRUE` / `TRUE` | Team Viewer before and during battle. |
| `B_RESTORE_HELD_BATTLE_BERRIES` | `TRUE` | Berries can be restored after battle. |
| `B_TRAINER_BATTLE_AFTERCARE` | `TRUE` | Normal trainer wins heal the party after battle. |
| `I_HELD_ITEM_CATALOG_ASSIGNMENT` | `TRUE` | Held item assignment uses ownership-token semantics. |
| `I_REUSABLE_TMS` | `TRUE` | TMs are reusable. |
| `OW_FIELD_MOVE_MODERNIZATION` | `TRUE` | HM field moves use modern unlock flow. |
| `OW_FLAG_NO_ENCOUNTER` | `FLAG_NO_ENCOUNTER` | Debug encounter-off flag is wired to real runtime behavior. |
| `P_EVOLUTIONS_ENABLED` | `FALSE` | Ordinary evolution is disabled. |
| `SAVE_CHAMPIONS_RUN_SESSION` | `TRUE` | Champions run snapshot / restore is compiled in. |
| `P_UNIFIED_MOVE_RELEARNER` | `TRUE` | Summary / relearner entry uses unified all-source move list. |
| `P_SUMMARY_SCREEN_STATE_EDITOR` | `TRUE` | Summary Skills page can open the State Editor. |

## Debug Commands

The full debug command list is in [config_debug_matrix.md](config_debug_matrix.md). Primary routes:

| Menu | Command | Purpose |
|---|---|---|
| `Party...` | `Selection Battle` | Single trainer selection / team viewer route. |
| `Party...` | `Selection Double` | Double trainer selection / team viewer route. |
| `Party... > All Ability...` | `A` to `U` patterns | Focused All Ability Slots behavior tests. |
| `Scripts...` | `Scout Selection` | 12-candidate scout selection, 1 pick. |
| `Scripts...` | `Scout Pick 6` | 12-candidate scout selection, 6 picks. |
| `Scripts...` | `Pokemon Vendor` | Friendly Shop Pokemon Vendor list. |
| `Scripts...` | `Vendor Reward` | Trainer battle that grants sealed bond EXP in battle-win text. |
| `Scripts...` | `TM Shop Test` | TM shop migration smoke route. |
| `Scripts...` | `Champs: Start / Give Mon / Checkpoint / Retire / Lose Test / Clear` | Champions run session validation flow. |
| `Scripts...` | `Field Kit Full / Item / Flags / Clear` | Field Kit unlock and reset validation. |
| `Flags & Vars...` | `All Abilities: Default/OFF/ON` | Runtime all-ability-slot mode toggle. |
| `Flags & Vars...` | `Toggle Encounter OFF` | No random encounters. |
| `Sound...` | `Trainer BGM...` / `Wild BGM...` | Battle BGM selector and preview. |

## Validation Snapshot

| Check | Status |
|---|---|
| `rtk make -j16 -O all` | Passed on integration branch, existing RWX linker warning only. |
| `rtk make -j16 -O debug` | Passed on integration branch, existing RWX linker warning only. |
| `rtk make -j16 -O check` | Passed with expected / known-failing markers and exit 0. |
| `rtk mdbook build docs` | Passed with existing `CHANGELOG.md` include, `CREDITS.md </img>`, and large search-index warnings. |
| mGBA Live smoke | Runtime boot / focused feature smokes recorded in owning test plans. |
| `codex review --base master` / `--uncommitted` | Integration blockers were fixed; final short-party split review found no introduced bug. |
| Map Asset Relinker release | Tag `map-asset-relinker-v0.1.0` pushed; Actions builds Windows portable zip and publishes Release asset when complete. |

## Next Decision Gate

Before moving to the next phase:

1. Read [html/index.html](html/index.html) visually and confirm the feature set is understandable.
2. Use [config_debug_matrix.md](config_debug_matrix.md) to decide which defaults should remain enabled in the runtime integration lane.
3. Confirm whether PR #68 should remain a staging branch, be narrowed, or become the base for a future 16.x migration branch.
4. Keep `master` docs / Lua-only unless that branch policy is explicitly changed.
