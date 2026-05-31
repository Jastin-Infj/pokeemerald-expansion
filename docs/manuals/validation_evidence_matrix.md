# Validation Evidence Matrix

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Baseline | `master` `0cdd416376`; `git describe` = `expansion/1.15.2-71-g0cdd416376` |
| Code status | Docs-only evidence index |
| Provenance | Feature test plans, current `gh pr list --state all`, fetched PR refs, branch merge-base diffs, 2026-05-17 PR cleanup, 2026-05-18 Team Viewer / partygen source audit |

この matrix は open / closed implementation shelf の横断 evidence。未確認は未確認として残す。
採用前は owning feature の `test_plan.md` を source of truth として再確認する。

## Current PR Snapshot

2026-05-31 時点では、open PR は PR #68 の runtime-dev 統合 staging だけです。
旧 implementation shelf PR は #68 へ採用済みのため close 済みで、個別に
`master` へ merge する対象ではない。

| PR | Feature | State | Branch | Handling |
|---|---|---|---|---|
| #68 | Runtime integration staging | Open draft | `integration/runtime-dev-20260529` | Current runtime integration lane. Local `all` / `debug` / full `check` / focused feature checks pass; All Ability Slots is default-`FALSE` with a save-backed debug runtime override. |
| #47 / #48 / #51 / #54 / #57 / #60 / #62 | Older runtime shelves | Closed 2026-05-31 | feature branches | Adopted into #68; keep closed PRs / branches as evidence shelves only. |
| #65 | Map Asset Relinker tooling | Closed 2026-05-31 | `feature/map-asset-relinker-20260525` | Adopted into #68 for source/workflow tracking and released through `map-asset-relinker-v0.1.0`; Route301 / sample map experiment data remains separate. |

## Runtime Integration #68 Evidence

| Date | Check | Result | Notes |
|---|---|---|---|
| 2026-05-30 | Runtime branch audit | Pass | [Japanese checklist](../features/15_3/complete_tree/completed_runtime_integration_checklist_ja_2026_05_30.md) marks the major 1.15.3 runtime shelves adopted into `integration/runtime-dev-20260529`. Map relinker, randomizer, and map / Fly experiments remain separate lanes. |
| 2026-05-30 | `codex review --base master` | Partially complete | Completed review passes found and drove fixes for trainer battle selection defeat restore, live battler all-slot field helpers, mandatory Field Kit handoff retry, in-battle Team Viewer hint gating, all-slot paired weather end-turn queue, Pokemon Vendor long-list cursor clamp, Pokemon Vendor allocation guards, trainer-selection queued vendor rewards, battle item borrowed-item cleanup, Cut repeat branch, runtime ability-copy switches, and third-block end-turn recursion. Final rerun was blocked by Codex usage limit after the last P2 was fixed. |
| 2026-05-30 | Local validation | Pass | `rtk git diff --check`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check` pass. Existing RWX linker warning and expected / known-failing test markers only. |
| 2026-05-30 | mGBA Live smoke | Pass | Session `runtime-integration-review-20260530` booted `pokeemerald.gba`, accepted START input, captured `/tmp/runtime-integration-review-20260530.png`, stopped cleanly, and `status --all` returned `[]`. |
| 2026-05-31 | Missing runtime/tooling follow-up | Pass | Reconfirmed Trainer Battle Selection config is `TRUE`; enabled Trainer Battle Aftercare; restored Map Asset Relinker source/workflow; added generic `palette-blink` Fly icon runtime support; persisted Battle BGM choices in SaveBlock2 filler with stale-filler test coverage. Local `all`, `debug`, `check`, Battle BGM focused tests, Map Relinker fixture, Rust core check/test, GUI build, and Tauri Linux native build pass. `codex review --uncommitted` relinker findings for move preflight, selected-map self-reference preview, Windows Python fallback, transition labels, shared layout rename, generated map output directory warnings, empty map ids, GUI reviewed-plan apply, and Rust CUI broken-pipe handling were fixed. mGBA Live `runtime-dev-followup-20260531` booted the ROM, captured `/tmp/runtime-dev-followup-20260531.png`, stopped cleanly, and `status --all` returned `[]`. |
| 2026-05-31 | All Ability Slots runtime-toggle follow-up | Pass | `B_ALL_ABILITY_SLOTS` defaults to `FALSE` and `B_ALL_ABILITY_SLOTS_RUNTIME_TOGGLE` stays `TRUE` in `include/config/battle.h`; `SaveBlock2.optionsAllAbilitySlotsMode` adds `DEFAULT` / `OFF` / `ON` runtime override support. `BattleStartClearSetData()`, Summary, and Party UI all resolve through `GetConfig(B_ALL_ABILITY_SLOTS)`, so the override affects normal battles without rebuilding. `rtk make -j16 -O check TESTS='All Ability Slots'`, `rtk make -j16 -O all`, `rtk make -j16 -O debug`, full `rtk make -j16 -O check`, docs build, and mGBA Live boot pass. mGBA Lua changed the save option from `DEFAULT` to `ON` and read back `afterMode = 2`; screenshot `/tmp/all-ability-runtime-toggle-final-20260531.png`; cleanup `[]`. |
| 2026-05-31 | Version-tree docs and PR shelf cleanup | Pass | Closed completed / superseded PR shelves #47, #48, #51, #54, #57, #60, #62, and #65, leaving #68 as the only open runtime staging PR. Renamed 15.3 files to `completed_*`, added 16.0 carryover / sync policy docs, and updated SUMMARY navigation. `rtk git diff --cached --check`, `rtk mdbook build docs`, focused generated HTML link check, and `codex review --uncommitted` passed; review reported no actionable issues. |

## Closed / PR-less Implementation Shelves

| Feature | Branch / PR | Docs | Local make | Focused tests | mGBA / manual evidence | Known gaps |
|---|---|---|---|---|---|---|
| No Random Encounters step-only | Closed PR #41 / `feature/no-random-encounters-step-only-runtime-20260517`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/no_random_encounters/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test / docs_validate passed. Integration: `all`, `debug`, full `check` passed. | No separate unit test; implementation is a flag id allocation using the existing `CheckStandardWildEncounter` gate. | Route 101 flag OFF wild Wurmple battle, flag ON no-encounter walking for 2400 macro frames, flag OFF-restored Poochyena battle, and user confirmation recorded on 2026-05-17. Integration smoke confirmed boot / field state and SaveBlock flag set / clear. | Current `master` still has `OW_FLAG_NO_ENCOUNTER 0`; Fishing / Sweet Scent / Rock Smash / scripted wild remain out of MVP scope. |
| Battle BGM Selector / Sound Archive | Closed PR #39 / `feature/battle-bgm-selector-mvp-20260517`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/battle_bgm_selector/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test passed. Integration: `all`, `debug`, full `check` passed. | `test/battle_bgm.c` covers routing and imported song choices. Integration round-tripped odd-length `dp_016bongo.aif` through `aif2pcm --compress` after fixing the reviewed final half-byte length bug. | Shelf mGBA selector, preview, debug trainer battle, and song-header evidence recorded. Integration smoke opened Debug -> `Sound...`, confirmed `Trainer BGM...` / `Wild BGM...`, selected `Hoenn Wild` from Trainer BGM, previewed it, captured screenshots, and stopped cleanly. | Actual wild step encounter after split choice and actual Trainer/Wild battle starts using the expanded imported batch remain manual follow-up. MCP cannot confirm audible playback. Modern Emerald source license / permission status remains unresolved. |
| TM Shop Migration | Closed PR #31 / `feature/tm-shop-migration`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/tm_shop_migration/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test passed. Integration: `all`, `debug`, full `check` passed. | Static grep and map JSON parse checks recorded; integration preserved no-random `FLAG_NO_ENCOUNTER` while retiring TM/HM legacy flags. | Shelf booted and opened Start menu. Integration smoke opened Debug menu `Scripts... > TM Shop Test` and stopped cleanly. | FRLG routes follow-up; route-specific retired NPC/gym/HM-source conversations were not walked in integration. |
| Unified Move Relearner | Closed PR #28 / `feature/unified-move-relearner`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/unified_move_relearner/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test passed. Integration: `all`, `debug`, full `check` passed. | JSON audits, candidate list checks, special/form/LGPE smoke checks; integration JSON and Python compile checks passed. | Shelf mGBA evidence covers Mew long list, Arceus special moves, Rotom, Cosplay Pikachu, LGPE partners, NPC cancel. Integration smoke confirmed Summary `START RELEARN`, unified list, `Lv` / `TM` source labels, D-pad right page-scroll, and no direct party `RELEARN` action with party route disabled. | Actual teach / overwrite pass remains recommended before final merge. |
| Summary Tera Type Icon | Closed PR #26 / `feature/summary-tera-type-badge`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/summary_tera_type_icon/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test passed. Integration: `all`, `debug`, full `check` passed. | Diff lint only; UI is visual. | Shelf mGBA Summary Info screenshot for dual-type Magearna at `(205, 48)`. Integration smoke confirmed the Info-page Tera badge on Wobbuffet and that the Skills-page State Editor `START EDIT` prompt still renders after adoption. | Single-type Pokemon and egg stale-icon path not run. |
| Pokemon State Editor | Closed PR #23 / `feature/pokemon-state-editor-expansion`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/pokemon_state_editor/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test passed. Integration: `all`, `debug`, full `check` passed. | Full `check`; no focused unit test listed. | Multiple shelf mGBA sessions confirmed editor pages, values, slide/layout, redraw, and direct Lua data checks. Integration smoke confirmed Party -> Summary -> Skills `START EDIT`, right-pane editor open, page-tab switch, B close, and clean mGBA session cleanup. | Box Summary and legality locks remain follow-up. |
| Pre-Battle / In-Battle Team Viewer | Closed PR #20 / `feature/prebattle-team-viewer`; phase2 shelf `feature/prebattle-team-viewer-phase2`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/prebattle_team_viewer/test_plan.md) | Shelf: `all`, `debug`, `check`, CI build / release / test passed. Integration: `all`, `debug`, full `check` passed. | Full `check`; no automated preview-cache assertion yet. Source audit confirms preview generation calls the trainer pool path and battle init consumes the same cached party. | Extensive shelf mGBA screenshots cover single, double, in-battle viewer, held-key guard, Summary return, details persistence. Integration smoke confirmed the pre-rename `Party -> Team Viewer Battle` route; current labels are `Party -> Selection Battle` and `Party -> Selection Double`. | Optional automated preview-cache assertion / focused pool trainer route before final merge; not a missing runtime mechanism. |
| Battle Item Restore Policy | Closed PR #14 / `feature/battle-item-restore-policy` | [test_plan](../features/battle_item_restore_policy/test_plan.md) | `all`, `debug`, focused `check` routes recorded in PR body. | Direct restore and full Oran Berry consume/restore tests recorded. | mGBA test-runner memory read reported pass / exit state. | Default TRUE/FALSE adoption policy still needs final decision. |
| Trainer Battle Aftercare heal hook | Closed PR #10 / `feature/trainer-battle-aftercare-heal`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 and enabled on 2026-05-31 | [test_plan](../features/trainer_battle_aftercare/test_plan.md) | Shelf: `all`, `debug`, `check` recorded in PR body. Integration: `all`, `debug`, full `check` passed before the enable follow-up. | No focused exclusion test suite yet; integration now sets `B_TRAINER_BATTLE_AFTERCARE TRUE`. | Integration mGBA title/boot smoke reached title splash and cleanup returned `[]`. | Needs config-on normal-win and exclusion-path tests as acceptance evidence. |
| Champions Partygen catalog expansion | Closed PR #7 / `feature/trainer-partygen-catalog-expansion`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [partygen validation report](../features/champions_challenge/partygen_validation_report.md) | Shelf: `make`, `make debug`, mdBook recorded in PR body. Integration: `all`, `debug`, full `check` passed after adoption. | Cargo test / clippy / partygen doctor-generate-validate-diff recorded on shelf. | mGBA trainer memory read confirmed generated Elite Four party pools. | Further generated pool balance / drift review remains follow-up. |
| Trainer Battle Party Selection | `feature/battle-selection-mvp` | [test_plan](../features/battle_selection/test_plan.md) | Branch docs record build/manual evidence. | No current fresh v15 check in this audit. | User manual validation for single/double/party restore recorded. | Re-apply before Team Viewer if not taking #20 wholesale. |
| Field Move Modernization / Field Kit | `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item`; adopted into `integration/runtime-dev-20260529` on 2026-05-30 | [test_plan](../features/field_move_modernization/test_plan.md) | Branch docs record local validation. Integration: `all`, `debug`, full `check` passed after adoption and after HM flag / item-restore follow-up fixes. | Full `check`; no focused automated field-use test listed in this audit. | Docs record user-confirmed HM-free MVP and Field Kit itemization. Integration preserved named Field Kit debug routes. | Graphics remain implementation artifacts and stay off docs-only `master`; final manual field-use sweep remains useful before a runtime release merge. |

## Docs-Only Planned Policy Features

| Feature | Docs | mdBook | Local make | Focused tests | mGBA / manual evidence | Known gaps |
|---|---|---|---|---|---|---|
| Nonconsumable Held Items | #48 / `feature/held-item-catalog-current-master-20260519` | [test_plan](../features/nonconsumable_held_items/test_plan.md) | 2026-05-19 `all`, `debug`, full `check`, and focused `test/bag.c` passed. | Bag quantity drift helper tests passed for catalog Give / Take / first-copy preservation / Mail exclusion / duplicate normalization / ordinary consumable exclusion. | mGBA Live boot/input smoke passed; Bag token marker UI route exported screenshot evidence and was user-confirmed; cleanup returned `status --all` to `[]`. | Battle-end restore remains separate in PR #47; stolen / swapped item ownership remains open. |

## Docs-only Baseline Check

Current `master` docs build was checked before this matrix update:

| Check | Result |
|---|---|
| `rtk mdbook build docs` | Exit 0 |
| Existing warning | Missing root `CHANGELOG.md` include from `docs/CHANGELOG.md`. |
| Existing warning | `CREDITS.md` has unexpected `</img>`. |
| Existing warning | Search index is large. |

## How To Update

- Add a row when a new implementation shelf opens.
- Move a row out only after the PR is merged, closed as superseded, or explicitly
  abandoned.
- Do not change merge state without fresh `gh` output.
- Keep skipped long GitHub Actions waits in the feature `test_plan.md`.
