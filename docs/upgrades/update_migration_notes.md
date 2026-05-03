# Update Migration Notes

## Current Baseline

調査時点の README では、この repo は pret `pokeemerald` をベースにした `pokeemerald-expansion` で、credit に `pokeemerald-expansion 1.15.1` ベースと記載されている。

現時点では、トレーナーバトル前選出機能の実装コードは存在しない。今回追加したのは docs のみ。

2026-05-02 に upstream `RHH` remote の `expansion/1.15.2` tag を確認した。local baseline 表記はまだ 1.15.1 だが、main を 1.15.2 へ上げる場合は `docs/upgrades/1_15_1_to_1_15_2_impact.md` を先に確認する。

branch 運用の基本方針は `docs/upgrades/branching_upgrade_policy.md` に分離した。今後の提案では、`main`、`upgrade/*`、`feature/*`、`reference/*`、docs branch の役割をこの方針に合わせる。

重要: PR 作成は target repository を確認してから行う。自分の fork / `origin` 内で閉じる PR は許容できるが、`RHH` / `rh-hideout/pokeemerald-expansion` など親元 upstream、共同管理 repo、有志プロジェクトへは、事前許可と根回しなしに PR を作らない。upgrade 検証は local branch / fork branch / docs report / compare URL で止められるようにする。

## 1.15.2 Migration Notes

1.15.2 への追従で最初に注意する点:

- `INCBIN_*` graphics declarations から `INCGFX_*` への migration が大きい。custom UI / icon / Pokemon graphics を追加する前に main を 1.15.2 へ上げるなら、asset pipeline の移行を先に終える。
- upstream changelog は `migration_scripts/1.15/migrate_incgfx.py` を clean worktree から使う前提を示している。
- `src/battle_setup.c`、`src/party_menu.c`、`src/script_pokemon_util.c` は 1.15.1 -> 1.15.2 の diff では未変更だったため、トレーナーバトル前選出の初期調査は概ね維持できる。
- `src/battle_main.c`、`src/battle_interface.c`、`src/dexnav.c`、`src/field_control_avatar.c`、`src/pokedex_area_screen.c` は変更があるため、battle end、battle UI、DexNav、follower / map interaction、wild display は merge 後に再確認する。
- `include/global.h` は変更されているが、確認した diff では `struct SaveBlock3` field 追加は見つかっていない。ただし merge 後は `STATIC_ASSERT(sizeof(struct SaveBlock3) <= ...)` と `SaveBlock3Size` を必ず確認する。

## Migration Principles

- `main` は採用済みの安定ベースとして扱い、直接作業 commit を積まない。
- upstream 追従は `upgrade/<version>` branch で試し、build / docs impact を確認してから `main` へ入れる。
- 大型機能や外部実装の検証は `reference/*` / `prototype/*` branch に分け、必要差分だけ `feature/*` branch で採用する。
- PR 作成は別権限として扱う。branch 作成や diff 調査を許可されても、PR 作成まで許可されたとは判断しない。
- upstream 更新前に local custom feature の入口 file を把握する。
- `src/battle_setup.c`、`src/party_menu.c`、`src/script_pokemon_util.c` など、upstream 変更が入りやすい file への変更は最小化する。
- 独自 state は既存 global と混ぜすぎない。
- event script flow を変更する場合は、script command と C callback の両方を docs に反映する。
- `gPlayerParty` を一時的に変更する feature は、必ず restore invariant を定義する。
- マート、野生、TM/HM、item、move、ability のような data-driven 機能は、constants、data table、generated data、runtime hook、UI、tests を分けて migration point に記録する。
- `SetMainCallback2`、`CreateTask`、`ScrCmd_*`、`special`、`gFieldCallback` を経由する変更は、直接呼び出しだけで影響範囲を判断しない。

## Expected Future Custom Touch Points

まだ実装していないが、将来的に触る可能性がある箇所:

| Area | Candidate Files | Notes |
|---|---|---|
| trainer battle selection trigger | `data/scripts/trainer_battle.inc`, `src/scrcmd.c`, `src/battle_setup.c` | どこへ選出 UI を挟むか次第 |
| choose half extension | `src/party_menu.c`, `include/party_menu.h` | 通常 trainer battle 用 mode が必要かもしれない |
| party backup/restore | `src/battle_setup.c`, `src/pokemon.c`, new helper file | 元 party 復元の中核 |
| config flag | `include/config/*.h` | feature enable/disable |
| tests | `test/` | 既存 test framework の調査が必要 |
| pokemart / shop settings | `data/maps/*Mart*/scripts.inc`, `data/maps/*DepartmentStore*/scripts.inc`, `src/scrcmd.c`, `src/shop.c`, `include/shop.h` | 固定品揃え、条件付き品揃え、ランダム shop の候補 |
| wild pokemon randomizer | `src/data/wild_encounters.json`, `tools/wild_encounters/wild_encounters_to_header.py`, `src/wild_encounter.c`, `include/wild_encounter.h` | build-time 生成か runtime hook かで migration strategy が変わる |
| trainer party randomizer / reorder | `include/trainer_pools.h`, `src/trainer_pools.c`, `src/data/battle_pool_rules.h`, `tools/trainerproc/main.c`, `src/battle_main.c` | party pool、difficulty、preview の整合性 |
| TM/HM expansion | `include/constants/tms_hms.h`, `include/item.h`, `include/constants/items.h`, `src/item.c`, `src/data/items.h`, `src/item_menu.c`, `src/item_use.c` | TM count、HM count、bag capacity、display、teach flow |
| field HM removal | `src/field_move.c`, `include/field_move.h`, `src/scrcmd.c`, `data/scripts/field_move_scripts.inc`, `src/party_menu.c`, `src/pokemon.c`, `src/pokemon_summary_screen.c` | 技所持不要化、forget HM、field script、party action の整合性 |
| custom items | `include/constants/items.h`, `src/data/items.h`, `include/item.h`, `src/item.c`, `src/item_use.c`, `src/item_menu.c`, `src/shop.c` | item ID、pocket、field use、battle use、shop、text |
| custom moves | `include/constants/moves.h`, `include/move.h`, `src/data/moves_info.h`, `src/data/battle_move_effects.h`, `data/battle_scripts_1.s`, `data/battle_scripts_2.s`, `test/battle/move_*` | move data、effect、script、AI、animation、tests |
| custom abilities | `include/constants/abilities.h`, `src/data/abilities.h`, `include/pokemon.h`, `src/battle_util.c`, `src/battle_ai_main.c`, `src/battle_ai_util.c`, `src/battle_ai_switch.c`, `test/battle/ability` | ability data、battle trigger、AI、copy/suppress flags、tests |
| callback / dispatch changes | `src/main.c`, `include/main.h`, `src/task.c`, `include/task.h`, `src/script.c`, `src/scrcmd.c`, `src/overworld.c`, `include/overworld.h` | screen transition / task / script wait / field return |

## Migration Checklist for Battle Selection

実装後に upstream 更新する場合:

- 選出 UI が出る条件を再確認する。
- single/double 判定 helper が upstream の battle type 判定とずれていないか確認する。
- `gSelectedOrderFromParty` の意味が変わっていないか確認する。
- `PARTY_MENU_TYPE_CHOOSE_HALF` の validation が変わっていないか確認する。
- `CB2_EndTrainerBattle` の全 outcome path で restore が走るか確認する。
- whiteout / forfeit / battle facility などの特殊 return path を再確認する。
- `SavePlayerParty` / `LoadPlayerParty` の副作用が変わっていないか確認する。

## Notes for Docs Maintenance

- 調査済み symbol を削除または rename した場合は、docs 内の references を更新する。
- 未確認だった項目を確認できたら、各 doc の `Open Questions` から移動する。
- Mermaid flowchart は実装に合わせて更新する。
- 実装後は `feature_registry.md` の status を `Implementing` / `Testing` / `Shipped` へ更新する。
- 新機能を追加する前に `docs/overview/extension_impact_map_v15.md` と `docs/overview/callback_dispatch_map_v15.md` の該当範囲を更新する。
- generated file を直接編集した場合は migration で破綻しやすいため、source data と tool のどちらを正式な編集対象にしたか docs に残す。

## Open Questions

- upstream tracking branch 名と更新手順は repo 運用に合わせて明文化する必要がある。
- v16 系以降で battle setup / party menu が大きく再編された場合、この v15 docs を残すか、新しい `*_v16.md` を作るか未決定。
- build-time randomizer と runtime randomizer のどちらを採用するか未決定。
- save block を変更する独自 option / seed / feature flag の migration policy は未決定。
