# Feature Registry

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `master` `187df44eb6`; `git describe` = `expansion/1.15.2-73-g187df44eb6` |
| Code status | Docs-only registry / PR queue snapshot |
| Provenance | Local project overlay, `gh pr list --state all`, fetched PR refs, branch merge-base diffs, 2026-05-17 PR cleanup, 2026-05-18 Team Viewer / partygen source audit, 2026-05-18 comprehensive inventory |

この docs 配下で管理する独自機能候補の一覧。

Latest cross-branch audit:
[Implementation Shelf Audit 2026-05-17](implementation_shelf_audit_2026_05_17.md).
Comprehensive feature inventory:
[Comprehensive Feature Inventory 2026-05-18](comprehensive_feature_inventory_2026_05_18.md).
Feature-by-feature branch audit:
[Feature Branch Audit 2026-05-18](feature_branch_audit_2026_05_18.md).
Latest next-work triage:
[Next Runtime Triage 2026-05-18](next_runtime_triage_2026_05_18.md).
Closed PRs and PR-less branches are implementation evidence when they carry
runtime source and validation notes; do not infer "not implemented" from closed
status alone.

## Current Order

実装着手の優先順位。`open_investigation_queue.md` の High Priority、feature の `Status`、現行 `master` に入っている実コードの有無を組み合わせた snapshot。
順序は固定ではなく、上から「次の feature branch で着手しやすい」順に並ぶ。新規 task 開始時はここを基準に owning feature docs を更新する。

### 2026-05-18 Practical Recommendation

Default next implementation:
**TM Shop Migration #31** on a fresh current-master adoption branch. This is
adoption of an implemented shelf, not a from-scratch implementation. #41 No
Random Encounters is complete and should only be adopted if a playable
integration branch needs it now.

Important correction:
The HM replacement item is also already implemented. `ITEM_FIELD_KIT`, Field Kit
capability flags, the Field Kit menu, and Emerald HM acquisition replacement live
on `feature/field-move-toolkit-item`. TM Shop Migration retires the old TM/HM
acquisition paths; Field Kit is the player-facing HM replacement shelf.

Default new feature if not adopting an existing shelf:
**Jukebox / Sound Archive**. It avoids SaveBlock, battle hooks, Summary, TM/HM,
Move Relearner, Bag, and Champions systems while still creating visible
in-game value.

High-impact alternative:
**Field Move Modernization / Field Kit**, after deciding whether TM Shop
Migration should land first. This is also an implemented shelf adoption, not
initial feature work.

2026-05-18 correction:
**Pre-Battle / In-Battle Team Viewer pool/randomizer consistency is implemented
on the feature shelf**. Source audit found preview generation flowing through
`CreateNPCTrainerPartyForPreview()` and `DoTrainerPartyPool()`, with battle init
loading the same cached result through `PreBattleTeamViewer_LoadCachedOpponentParty()`.
If this shelf is selected, the remaining work is regression evidence / adoption
conflict handling, not the initial implementation.

If the goal is to continue cleaning up already-built feature requirements
instead of adopting shelves, the recommended next work is **Pokemon State Editor
polish**. If the goal is a fresh runtime feature, choose **Nonconsumable Held
Item catalog assignment**.

### Master Baseline Snapshot (2026-05-17)

`master` は upstream 追従の受け皿で、local runtime source は原則入れない。
次の branch / docs は存在するが、runtime source は `master` にはまだ入って
いないものとして扱う。実装は current `master` から fresh `feature/*` または
`integration/*` branch を切って再適用する。

| Topic | Evidence | Master action |
|---|---|---|
| No Random Encounters | Completed implementation shelf #41 on `feature/no-random-encounters-step-only-runtime-20260517`; closed 2026-05-17 after CI success. User-confirmed Route 101 behavior; docs handoff merged through #42. | Runtime behavior is done. Re-apply from shelf on a fresh integration branch if selected; `master` still has `OW_FLAG_NO_ENCOUNTER 0`. |
| TM Shop Migration | Completed implementation shelf #31 on `feature/tm-shop-migration`; closed 2026-05-17 after CI success. Docs handoff was merged through PR #30. | Best next adoption candidate after no-random cleanup; re-apply from shelf on a fresh integration branch if selected. |
| Unified Move Relearner | Completed implementation shelf #28 on `feature/unified-move-relearner`; closed 2026-05-17 after CI success. Docs-only handoff was merged through PR #29. | Rebase / conflict-check after TM Shop Migration before any implementation merge. |
| Summary Tera Type Icon | Completed implementation shelf #26 on `feature/summary-tera-type-badge`; closed 2026-05-17 after CI success. | Keep runtime source / imported graphics off `master` unless a fresh integration branch is explicitly selected. |
| Pokemon State Editor | Completed implementation shelf #23 on `feature/pokemon-state-editor-expansion`; closed 2026-05-17 after CI success. | Treat as an implementation shelf; resolve adoption order and UI/data policy before touching `master`. |
| Pre-Battle / In-Battle Team Viewer | Completed implementation shelf #20 on `feature/prebattle-team-viewer`; closed 2026-05-17 after CI success. Source audit confirms pool/randomizer cache behavior is implemented on the shelf. | Keep as a validated implementation shelf; add optional automated/focused cache regression when selecting it for integration. |
| Trainer Battle Aftercare / Battle Item Restore | `feature/battle-item-restore-policy` に berry-inclusive held item restore と focused tests がある。`feature/trainer-battle-aftercare-heal` には aftercare heal-only hook も含む旧 evidence がある。`master` には `B_TRAINER_BATTLE_AFTERCARE` / `B_RESTORE_HELD_BATTLE_BERRIES` が無い。 | Docs に evidence を残す。item restore と aftercare は `master` ではなく fresh branch で分割して取り込む。 |
| Nonconsumable Held Items | #48 / `feature/held-item-catalog-current-master-20260519` に catalog / unique-token assignment source がある。Battle-end restore は PR #47 として別 shelf。 | Source は `master` 未反映。catalog branch は held-effect items を1 Bag token化し、Party / Bag / Storage item quantity drift を helper 化する。Mail / Battle Pyramid / no-hold-effect items は物理扱いのまま残す。 |
| Champions Partygen | `feature/trainer-partygen-catalog-expansion` に Rust CLI、catalog、Elite Four / Wallace Trainer Party Pool data diff、mGBA ROM-memory evidence がある。`master` には `tools/champions_partygen/README.md` だけがある。 | tool / data / generated workflow の review 後、大型 feature / integration branch として扱う。Closed #5/#7 は未実装ではなく実装棚。 |
| Bag Expansion | `docs/features/bag_expansion/` に docs-only kickoff を追加。通常 bag は SaveBlock1 の `struct Bag` で、1 slot 約 4 B。`test/save.c` 上の SaveBlock1 余りは 304 B。 | 実装前に pocket target と save compatibility / migration 方針を決める。SaveBlock3 の空きは通常 bag には使わない。 |
| Field Move Modernization / HM Removal | `feature/field-move-modernization-mvp` and `feature/field-move-toolkit-item` hold runtime slices. Docs say both the HM-free MVP and Field Kit itemization were locally validated and user-confirmed. | Do not describe this as unimplemented / no-code. Runtime source still stays off `master` until a selected implementation PR / integration branch is created. |
| Legacy item / randomizer / map / UI prototypes | Older remote branches such as `origin/item_clock`, `origin/item_heal_patry`, `origin/item_keyfly`, `origin/TM_v12_0`, `origin/feature/ex-rz-upstream1`, `origin/feature/EX/ex-rz-upstream1`, `origin/feature/releaseSystem`, `origin/feature/move_relearner`, `origin/feature/main_menu`, `origin/feature/qol_field_moves`, `origin/feature/party-select-ui`, `origin/feature/birch_case`, and `origin/feature/new-map` contain real implementations on older baselines. | Use as behavior references only. Do not merge older-baseline branches into current `master`; re-implement selected behavior on a fresh v15 branch. Full inventory: [Comprehensive Feature Inventory 2026-05-18](comprehensive_feature_inventory_2026_05_18.md). |

### New Feature Candidate Snapshot (2026-05-17)

These are fully new local feature candidates. They are docs-only planning
entries, not open runtime implementation shelves, and they have no source
changes on `master`.

| Feature | Status | Code status | Docs | Runtime branch | Notes |
|---|---|---|---|---|---|
| Jukebox / Sound Archive | Planned | Docs only / No code changes | `docs/features/jukebox_sound_archive/` | None | Recommended first new runtime candidate. Avoids SaveBlock, battle hooks, Summary, TM/HM, Move Relearner, Bag, and Champions systems in MVP. |
| Battle BGM Selector / Sound Archive | Implemented draft | Runtime source plus BW/BW2, DPPt, Platinum, and HGSS battle BGM imports on feature branch; no code changes on `master` | `docs/features/battle_bgm_selector/` | `feature/battle-bgm-selector-mvp-20260517` | Debug Trainer/Wild selector plus selected Modern Emerald import slices, including Galactic/Rocket, Platinum, HGSS Kanto, beast, Frontier, and legendary follow-up choices. No SaveBlock or trainer data changes. DPPt / Platinum / HGSS uses imported `.aif` sample tooling. Modern Emerald license / permission status remains a master-adoption risk. |
| Weather Lab Terminal | Planned | Docs only / No code changes | `docs/features/weather_lab_terminal/` | None | Debug / presentation utility for existing weather types. Default weather restore is TBD. |
| Bounty Board / Request Board | Planned | Docs only / No code changes | `docs/features/bounty_board/` | None | Script-only item delivery first slice. No battle / catch hooks in MVP. |
| Field Notes / Lore Codex | Planned | Docs only / No code changes | `docs/features/field_notes_codex/` | None | Worldbuilding archive with static text entries. No save / unlock / PokeNav in MVP. |
| Route Mastery Passport | Planned | Docs only / No code changes | `docs/features/route_mastery_passport/` | None | Medium complexity. Needs route-specific map / trainer / item flag ownership audit. |
| Trainer Titles / Achievement Badges | Planned | Docs only / No code changes | `docs/features/trainer_titles_achievement_badges/` | None | Event-flag title clerk MVP. Avoid Trainer Card and selected-title save state. |
| Nonconsumable Held Items | Integration candidate | Catalog assignment implemented on #48 / `feature/held-item-catalog-current-master-20260519`; not on `master` | `docs/features/nonconsumable_held_items/` | `feature/held-item-catalog-current-master-20260519` | Champions-style held item policy. Catalog / unique-token assignment is implemented as a separate UI/ownership slice; battle-end restore remains separate in PR #47. |

### GitHub PR / Shelf Snapshot (2026-05-17)

Open runtime PR は 0 件に整理済み。成功済み runtime PR は completed shelf として
close し、branch は preserved のまま残す。open / closed どちらも merge 許可ではなく、
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
| #41 `[codex] Enable no random encounters step-only` | `feature/no-random-encounters-step-only-runtime-20260517` | Closed 2026-05-17 after CI success; branch preserved. Fresh `gh pr view` reported `CLEAN` before close. | Completed implementation shelf. Runtime behavior and user confirmation are done. |
| #39 `[codex] Implement battle BGM selector and imports` | `feature/battle-bgm-selector-mvp-20260517` | Closed 2026-05-17 after CI success; branch preserved. | Completed implementation shelf with large audio asset diff; adoption requires asset permission / conflict review. |
| #31 `[codex] Implement TM shop migration` | `feature/tm-shop-migration` | Closed 2026-05-17 after CI success; branch preserved. Previously reported `CLEAN`. | Completed implementation shelf and best next re-apply candidate if adoption resumes. Docs-only handoff is already on `master` via #30. |
| #28 `[codex] Implement unified move relearner` | `feature/unified-move-relearner` | Closed 2026-05-17 after CI success; branch preserved. | Completed implementation shelf for the unified relearner. Rebase/conflict resolution needed after TM Shop Migration. |
| #26 `[codex] Add summary Tera type badge` | `feature/summary-tera-type-badge` | Closed 2026-05-17 after CI success; branch preserved. | Completed display-only Summary Tera icon shelf. Verify asset credit before adoption. |
| #23 `[codex] Add Pokemon state editor` | `feature/pokemon-state-editor-expansion` | Closed 2026-05-17 after CI success; branch preserved. | Completed Pokemon State Editor MVP shelf. Confirm adoption order and remaining UI/data risks before integration. |
| #20 `[codex] Implement prebattle team viewer` | `feature/prebattle-team-viewer` | Closed 2026-05-17 after CI success; branch preserved. | Completed Pre-battle / in-battle team viewer shelf. Source audit confirms trainer-pool preview cache implementation; add a focused assertion/evidence pass only if selected for integration. |
| #14 `[codex] Add battle item restore policy` | `feature/battle-item-restore-policy` | Closed 2026-05-09, unmerged. | Implemented and validated closed shelf: berry-inclusive held item restore with focused tests and mGBA test-runner evidence. Reopen / recreate only if this slice becomes active again. |
| #10 `Add trainer battle aftercare heal hook` | `feature/trainer-battle-aftercare-heal` | Closed 2026-05-10, unmerged. | Implemented closed shelf: default-off trainer win heal hook. Needs focused exclusion tests before adoption. |
| #7 `Add Elite Four partygen pools and battlefield lint` | `feature/trainer-partygen-catalog-expansion` | Closed 2026-05-10, unmerged. | Implemented closed shelf: Rust CLI, catalog, Elite Four / Wallace data diff, lint data, and validation evidence. Preserve for a later partygen / Champions integration branch. |
| #5 `Add trainer party generator MVP` | `feature/trainer-party-generator` | Closed 2026-05-09, remote branch deleted. | #7 が後継で同一 MVP commit を含むため superseded。 |
| #4 `Add Rouge Cave map draft` | `feature/new-map-test-v15` | Closed 2026-05-09, branch preserved. | CI failure 付き draft map work。今回の feature queue からは外し、再開時は新 branch で復帰する。 |
| #2 `Document v15 source investigation` | `codex-docs-v15-investigation` | Closed 2026-05-09, remote branch deleted. | docs は後続 snapshot / handoff で `master` に反映済み。古い branch は stale diff が大きいため閉じた。 |

### Recommended Implementation Order

| # | 対象 | 期待 status 遷移 | Why first | 依存 |
|---|---|---|---|---|
| 0 | `docs/flows/save_data_flow_v15.md` | Planned を維持 | 既に SaveBlock / saved flag 方針は決定済み。実装 item ではなく、各 branch の gate として参照する。 | なし (docs only) |
| 1 | `docs/features/no_random_encounters/` / completed shelf #41 | Completed shelf → Fresh adoption branch if selected | ユーザー確認済み。差分は 3 file の flag/config だけ。もう追加実装ではなく、必要になった時に fresh branch で採用するだけ。 | GitHub CI success, local all/debug/check, and mGBA evidence are recorded. |
| 2 | `docs/features/tm_shop_migration/` / completed shelf #31 | Completed shelf → Fresh adoption branch if selected | #31 was closed after CI success and previously reported `CLEAN`; Emerald normal-progression TM/HM acquisition retirement is implemented. | Unified Move Relearner virtual TM policy stays separate; FRLG-specific routes remain follow-up. |
| 3 | `docs/features/unified_move_relearner/` / completed shelf #28 | Completed shelf → Conflict resolution if selected | Broad move candidate builder and long-list UX are implemented; shelf is closed and preserved. | Prefer after TM Shop Migration; teach/overwrite runtime pass still recommended before merge. |
| 4 | `docs/features/field_move_modernization/` | Validated branch evidence → Fresh PR / integration branch | HM-free field move MVP and Field Kit itemization are implemented and user-confirmed, but there is no current open runtime PR. | TM Shop Migration retired old HM receive flags; capability flags must stay owned by field move feature. |
| 5 | `docs/features/summary_tera_type_icon/` / completed shelf #26 | Validated branch → Conflict / asset check if selected | Small display-only UI slice with imported icon assets and local validation. | Pokemon Icon UI flow; imported graphics credit policy. |
| 6 | `docs/features/pokemon_state_editor/` / completed shelf #23 | Implemented MVP → Adoption review if selected | Summary-launched editor is implemented; remaining risk is UI polish / box-summary / value legality, not initial investigation. | Summary UI ownership and future move editor separation. |
| 7 | `docs/features/prebattle_team_viewer/` / completed shelf #20 | Implemented MVP → Conflict / optional cache regression if selected | Team preview / selection, in-battle viewer, Summary return, and pool/randomizer cache path are implemented with focused mGBA evidence. | Battle selection restore flow and trainer pool behavior. |
| 8 | `docs/features/battle_item_restore_policy/` | Closed PR evidence → Fresh branch if resumed | #14 は閉じたため open queue ではない。item restore 自体は focused tests と mGBA evidence を持つ integration candidate として docs に残す。 | aftercare と同一旧 branch 由来だが独立して取り込む |
| 9 | `docs/features/nonconsumable_held_items/` | Planned → Fresh runtime PR | User-requested Champions-style held item policy. The first slice can reuse battle-end restore evidence, but catalog / unique-token assignment must be a separate UI ownership branch. | battle_item_restore_policy; Party / Bag / Storage held-item paths; optional Champions Challenge runtime. |
| 10 | `docs/features/trainer_battle_aftercare/` | Closed PR evidence → Fresh branch if resumed | #10 は閉じたため open queue ではない。default off の heal-only hook を再開するなら focused test gate を先に置く。 | battle item restore の取り込み後に競合を避ける |
| 11 | `docs/features/champions_challenge/` partygen CLI + catalog | Closed implemented shelf → Review / Testing | #7 は closed だが実装済み。Rust CLI、catalog、`src/data/trainers.party` の大型差分、lint、mGBA ROM-memory validation evidence がある。 | no_random / battle-end policy とは独立。Champions runtime とは分ける。 |
| 12 | `docs/features/bag_expansion/` | Investigating → Planned | Field Kit の Key Items 圧迫、250 TM の TM/HM pocket 不足、Champions bag snapshot が同じ `struct Bag` / SaveBlock1 decision に集まる。 | save_data flow |
| 13 | runtime rule options | Investigating → Planned | no_random や aftercare を runtime option に束ねる前に、保存先と UI owner を確定する。 | save_data flow + concrete feature behavior |
| 14 | `docs/features/champions_challenge/` runtime | Planned → Implementing | challenge party / bag / EXP / loss policy / reward state が重い。2026-05-20 の run session restore 調査で、Battle Pyramid 風の中断 / 復帰は再利用可能だが、`SavePlayerParty()` 流用は不可、通常 PC rollback は MVP では禁止 / run-only stash 優先と整理。partygen output と battle selection / aftercare 知見を使ってから入る。 | save_data flow + `docs/features/champions_challenge/run_session_restore.md` + bag_expansion + partygen CLI + battle_selection / team_viewer / aftercare ordering |

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
| Future Feature Candidates | Planned | Docs only / No code changes | `docs/features/future_feature_candidates.md` | 完全新規 feature 候補の一覧。Jukebox / Weather Lab / Bounty Board / Field Notes / Route Mastery / Trainer Titles を初期候補として整理。 |
| Jukebox / Sound Archive | Planned | Docs only / No code changes | `docs/features/jukebox_sound_archive/` | 推奨 first new runtime candidate。既存 BGM の小型 sound test。save / battle / Summary / TM/HM / Bag / Champions を避ける。 |
| Weather Lab Terminal | Planned | Docs only / No code changes | `docs/features/weather_lab_terminal/` | 既存 weather を debug / presentation 用に切り替える小型 terminal 候補。story weather と default restore が主リスク。 |
| Bounty Board / Request Board | Planned | Docs only / No code changes | `docs/features/bounty_board/` | script-driven item delivery request board 候補。battle / catch / daily / SaveBlock は MVP 外。 |
| Field Notes / Lore Codex | Planned | Docs only / No code changes | `docs/features/field_notes_codex/` | worldbuilding / lore archive 候補。3 fixed text entries の MVP から開始する。 |
| Route Mastery Passport | Planned | Docs only / No code changes | `docs/features/route_mastery_passport/` | route completion checklist 候補。flag ownership と map section audit が必要。 |
| Trainer Titles / Achievement Badges | Planned | Docs only / No code changes | `docs/features/trainer_titles_achievement_badges/` | event-flag based title clerk 候補。Trainer Card UI と selected-title save state は MVP 外。 |
| Trainer Battle Party Selection | Validated branch | Implemented on `feature/battle-selection-mvp`; not on `master` | `docs/features/battle_selection/` | 通常 trainer battle 前に 6 匹から 3/4 匹を選出する MVP。既存 choose-half UI を流用し、single / double / party restore の user manual validation 済み。transition animation の影表示は cosmetic accepted issue。 |
| Pokemart / Shop Configuration | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | `ScrCmd_pokemart`、`CreatePokemartMenu`、`Task_BuyMenu`、`data/maps/*Mart*/scripts.inc` を入口に調査。 |
| Wild Pokemon Randomizer | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | `src/wild_encounter.c`、`src/data/wild_encounters.json`、DexNav / Pokedex area への影響を確認済み。build-time か runtime かは未決定。 |
| No Random Encounters | Implemented runtime branch | Implemented on `feature/no-random-encounters-step-only-runtime-20260517`; not on `master` | `docs/features/no_random_encounters/` | `OW_FLAG_NO_ENCOUNTER` を使い、通常歩行中の land / water random encounter を止める候補。fresh branch の source diff は 3 file。MVP は step-only。Fishing / Sweet Scent / Rock Smash / static wild battle / option UI は後続扱い。 |
| DexNav / Encounter UI | Investigating | No code changes | `docs/flows/dexnav_flow_v15.md` | Start menu DexNav、detector mode、SaveBlock3、12 land slots、Pokemon icon 描画を整理。 |
| Trainer Party Reorder / Randomizer | Investigating | No code changes | `docs/features/battle_selection/opponent_party_and_randomizer.md` | `DoTrainerPartyPool`、`RandomizePoolIndices`、`AI_FLAG_RANDOMIZE_PARTY_INDICES` を確認。相手 party preview と関係。 |
| TM/HM and Field Move Policy | Investigating | No code changes | `docs/overview/tm_hm_expansion_250_v15.md` | 250 TM 前提の item ID / bag / relearner / field HM coupling を確認。`FOREACH_TM`、`FOREACH_HM`、`ScrCmd_checkfieldmove`、`gFieldMoveInfo`、`CannotForgetMove` も継続参照。 |
| Summary Tera Type Icon | Completed shelf #26 | Implemented and locally validated on `feature/summary-tera-type-badge`; not on `master` | `docs/features/summary_tera_type_icon/` | Summary Info page の通常タイプ欄の右側に RavePossum / Zatsu 由来の16x16 Tera badge だけを表示する display-only UI slice。現在の badge 位置は `P_SUMMARY_TERA_TYPE_ICON_X/Y = (205, 48)`。 |
| Bag Expansion | Investigating | No code changes | `docs/features/bag_expansion/` | 通常 bag pocket capacity の新規 feature。`struct Bag` は SaveBlock1 layout なので、Key Items / TM-HM pocket 拡張は save compatibility、bag UI、debug fill、ROM header count、`test/save.c` に波及する。 |
| Field Move Modernization / HM Removal | Validated branch | Implemented on `feature/field-move-modernization-mvp` and `feature/field-move-toolkit-item`; not on `master` | `docs/features/field_move_modernization/` | README records the HM-free field move MVP, Field Kit itemization, menu entry, icon/palette handoff, and manual validation as implemented / user-confirmed. |
| Champions-style EV/IV Training UI | Investigating | No code changes | `docs/overview/champions_training_ui_feasibility_v15.md` | EV/IV/nature/moveset 編集 UI は実装可能。32 point EV は UI 表示と内部 EV 変換を分ける方針が安全。EV total 518 と wild IV mode も調査済み。 |
| Scout Selection / Starting Battlefield Status | Implemented on feature branch | Runtime MVP on `feature/scout-selection-runtime-20260520`; not on `master` | `docs/features/scout_selection/`, `docs/overview/scout_selection_and_battlefield_status_v15.md` | Pokemon Champions 風の候補 Pokemon 選択 UI。New `scout_selection` module implements script-driven pool, 12 candidate debug pool, 6 visible slots, scroll, Summary preview/return, 1/N picks, and gift path handoff. The dummy starter pool has been replaced by a generated partygen JSON pool with species de-duplication. mGBA Live validated both the original runtime path and the partygen-derived pool update. |
| Champions Challenge Facility | Planned | Partygen implementation exists on feature branch; runtime not on `master` | `docs/features/champions_challenge/` | 0 匹開始、6 匹作成、Lv.50 battle-only、EXP 無効、bag 退避 / 空 challenge bag、egg-only default eligibility、optional Frontier ban、敗北時 challenge party 破棄と通常 party / bag 復元の仕様を整理。Run session restore 調査では、Frontier-style partial save は参考になるが、通常 party / bag snapshot は Champions 専用 state が必要、PC rollback は MVP で通常 PC disabled が安全と結論。`feature/trainer-partygen-catalog-expansion` に CLI / catalog / trainer data 差分がある。runtime は未実装。 |
| Party / Status UI Overhaul | Runtime branch | `feature/party-status-ui-overhaul-20260521` | `docs/features/party_status_ui_overhaul/` | Party screen を現行 `1 + 5` から `2 columns x 3 rows` (`2 / 2 / 2`) へ寄せる first slice を実装中。`PARTY_LAYOUT_GRID_2X3` を追加し、既存の `PARTY_LAYOUT_SINGLE` entry を grid へ解決する。外部画像 assets は未取り込みで、Emerald Extra equal-column party menu reference を元にした source-level tile-number table で `14 x 5` framed slot を描画。BW Summary full port は別 slice。UI 自体は SaveBlock 非消費。 |
| Wild Moveset Randomization | Investigating | No code changes | `docs/overview/wild_moveset_randomization_v15.md` | 野生初期技の現行「最後 4 level-up 技」flow、weighted level-up、TM/tutor 混在、外部 weight table の候補を整理。 |
| Runtime Rule Options | Investigating | No code changes | `docs/overview/runtime_rule_options_feasibility_v15.md` | Nuzlocke、release、difficulty、EXP/catch/shiny 倍率、Mega/Z/Dynamax/Tera、trade、randomizer の option 化候補を整理。 |
| Battle AI Decision / Switching | Investigating | No code changes | `docs/flows/battle_ai_decision_flow_v15.md` | move scoring、smart switching、double battle partner 評価、dynamic AI function の入口を整理。 |
| Roguelike Party / Held Item Policy | Investigating | No code changes | `docs/overview/roguelike_party_policy_impact_v15.md` | 100 戦型 facility、held item lock、item clause、battle item restore、release/swap policy の影響を整理。 |
| Map Registration / Region Map / Fly | Investigating | No code changes | `docs/flows/map_registration_fly_region_flow_v15.md` | 新規 map の `MAPSEC_*`、Town Map/Region Map、Fly icon、visited/world map flag、warp callback を整理。FRLG map preview と Fly 点滅の疑いどころも記録。 |
| NPC / Object Event / Conditional Tiles | Investigating | No code changes | `docs/flows/npc_object_event_flow_v15.md` | `events.inc` の `object_event`、`scripts.inc` の `applymovement` / `setobjectxyperm` / `setmetatile` / `setmaplayoutindex`、Town Map R Fly 後の popup リスクを整理。 |
| Battle Frontier Level Scaling | Investigating | No code changes | `docs/flows/battle_frontier_level_scaling_flow_v15.md` | 現行 Lv.50 course は低レベルを Lv.50 化しない。対戦用に battle-only Lv.50 補正を入れる場合の hook とリスクを整理。 |
| TM Shop Migration | Completed shelf #31 | Implemented on `feature/tm-shop-migration`; not on `master` | `docs/features/tm_shop_migration/` | Emerald normal-progression legacy TM/HM acquisition retirement is preserved in completed shelf #31. Docs-only handoff is on `master` via PR #30. FRLG-specific routes remain follow-up. |
| Custom Items / Moves / Abilities | Investigating | No code changes | `docs/overview/extension_impact_map_v15.md` | constants、data table、UI、battle behavior、AI、tests への影響範囲を横断 map に整理。 |
| Battle Item Restore Policy | Integration candidate | Branch implementation exists; not on `master` | `docs/features/battle_item_restore_policy/` | `feature/battle-item-restore-policy` に `B_RESTORE_HELD_BATTLE_BERRIES` default `TRUE`、`TryRestoreHeldItems()` の berry restore、direct / full battle tests、mGBA Live evidence がある。`master` へは source 未反映。 |
| Nonconsumable Held Items | Integration candidate | Catalog assignment branch implemented; not on `master` | `docs/features/nonconsumable_held_items/` | #48 / `feature/held-item-catalog-current-master-20260519` に `I_HELD_ITEM_CATALOG_ASSIGNMENT` default `TRUE`、held-effect unique Bag token化、Party / Bag / Storage quantity-drift helpers、focused bag testsがある。Battle-end restoreはPR #47。 |
| Trainer Battle Aftercare / Forced Release | Planned / branch implementation | Heal-only branch implementation exists; not on `master` | `docs/features/trainer_battle_aftercare/` | `feature/trainer-battle-aftercare-heal` に `B_TRAINER_BATTLE_AFTERCARE` default off の通常 trainer battle 勝利後 heal-only hook がある。no-whiteout、forced release、battle selection integration は後続。 |
| Callback / Dispatch Audit | Investigating | No code changes | `docs/overview/callback_dispatch_map_v15.md` | `SetMainCallback2`、`CB2_*`、`CreateTask`、`ScrCmd_*`、`special`、field callback の確認用 docs。 |
| Map Script / Flag / Var Audit | Investigating | No code changes | `docs/flows/map_script_flag_var_flow_v15.md` | `map.json`、generated `.inc`、hand-written `scripts.inc`、NPC hide flag、coord/bg event、item ball / hidden item flow を整理。 |
| Move Relearner / Summary Menu | Investigating | No code changes | `docs/flows/move_relearner_flow_v15.md` | summary / party / script からの技思い出し flow、`MAX_RELEARNER_MOVES`、TM 追加リスクを整理。 |
| Unified Move Relearner | Completed shelf #28 | Implemented on `feature/unified-move-relearner`; not on `master` | `docs/features/unified_move_relearner/` | Unified level / egg / TM / tutor / special move candidate list is preserved in completed shelf #28. Docs-only handoff is on `master` via PR #29; conflict refresh is needed before source integration. |
| Pokemon State Editor | Completed shelf #23 | Implemented on `feature/pokemon-state-editor-expansion`; not on `master` | `docs/features/pokemon_state_editor/` | Summary-launched EV/IV/core/status editor MVP is preserved in completed shelf #23. Remaining items are adoption order, UI polish, box-summary follow-up, and legality policy decisions. |
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
