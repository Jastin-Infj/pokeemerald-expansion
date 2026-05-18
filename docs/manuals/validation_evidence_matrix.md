# Validation Evidence Matrix

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `master` `0cdd416376`; `git describe` = `expansion/1.15.2-71-g0cdd416376` |
| Code status | Docs-only evidence index |
| Provenance | Feature test plans, current `gh pr list --state all`, fetched PR refs, branch merge-base diffs, 2026-05-17 PR cleanup, 2026-05-18 Team Viewer / partygen source audit |

この matrix は open / closed implementation shelf の横断 evidence。未確認は未確認として残す。
採用前は owning feature の `test_plan.md` を source of truth として再確認する。

## Current Open PR Snapshot

2026-05-17 cleanup 後、open runtime PR は 0 件。

| PR | Feature | Draft | Branch | Merge state | CI snapshot |
|---|---|---|---|---|---|
| None | - | - | - | - | - |

The successful runtime PRs #41 / #39 / #31 / #28 / #26 / #23 / #20 were closed
on 2026-05-17 as completed implementation shelves. Their branches remain
preserved.

## Closed / PR-less Implementation Shelves

| Feature | Branch / PR | Docs | Local make | Focused tests | mGBA / manual evidence | Known gaps |
|---|---|---|---|---|---|---|
| No Random Encounters step-only | Closed PR #41 / `feature/no-random-encounters-step-only-runtime-20260517` | [test_plan](../features/no_random_encounters/test_plan.md) | `all`, `debug`, `check`, CI build / release / test / docs_validate passed. | No separate unit test; implementation is a flag id allocation using the existing `CheckStandardWildEncounter` gate. | Route 101 flag OFF wild Wurmple battle, flag ON no-encounter walking for 2400 macro frames, flag OFF-restored Poochyena battle, and user confirmation recorded on 2026-05-17. | Current `master` still has `OW_FLAG_NO_ENCOUNTER 0`; Fishing / Sweet Scent / Rock Smash / scripted wild remain out of MVP scope. |
| Battle BGM Selector / Sound Archive | Closed PR #39 / `feature/battle-bgm-selector-mvp-20260517` | [test_plan](../features/battle_bgm_selector/test_plan.md) | `all`, `debug`, `check`, CI build / release / test passed. | `test/battle_bgm.c` covers routing and imported song choices. | mGBA selector, preview, debug trainer battle, and song-header evidence recorded. | Large asset/audio shelf; source permission risk remains before adoption. |
| TM Shop Migration | Closed PR #31 / `feature/tm-shop-migration` | [test_plan](../features/tm_shop_migration/test_plan.md) | `all`, `debug`, `check`, CI build / release / test passed. | Static grep and map JSON parse checks recorded. | Booted, continued save, opened Start menu; HM source route not fully confirmed. | FRLG routes follow-up; debug TM shop screen not confirmed in mGBA. |
| Unified Move Relearner | Closed PR #28 / `feature/unified-move-relearner` | [test_plan](../features/unified_move_relearner/test_plan.md) | `all`, `debug`, `check`, CI build / release / test passed. | JSON audits, candidate list checks, special/form/LGPE smoke checks. | mGBA evidence covers Mew long list, Arceus special moves, Rotom, Cosplay Pikachu, LGPE partners, NPC cancel. | Actual teach / overwrite pass remains recommended before adoption. |
| Summary Tera Type Icon | Closed PR #26 / `feature/summary-tera-type-badge` | [test_plan](../features/summary_tera_type_icon/test_plan.md) | `all`, `debug`, `check`, CI build / release / test passed. | Diff lint only; UI is visual. | mGBA Summary Info screenshot for dual-type Magearna at `(205, 48)`. | Single-type Pokemon and egg stale-icon path not run. |
| Pokemon State Editor | Closed PR #23 / `feature/pokemon-state-editor-expansion` | [test_plan](../features/pokemon_state_editor/test_plan.md) | `all`, `debug`, `check`, CI build / release / test passed. | Full `check`; no focused unit test listed. | Multiple mGBA sessions confirmed editor pages, values, slide/layout, redraw, and direct Lua data checks. | Box Summary and legality locks remain follow-up. |
| Pre-Battle / In-Battle Team Viewer | Closed PR #20 / `feature/prebattle-team-viewer` | [test_plan](../features/prebattle_team_viewer/test_plan.md) | `all`, `debug`, `check`, CI build / release / test passed. | Full `check`; no automated preview-cache assertion yet. Source audit confirms preview generation calls the trainer pool path and battle init consumes the same cached party. | Extensive mGBA screenshots cover single, double, in-battle viewer, held-key guard, Summary return, details persistence. | Optional automated preview-cache assertion / focused pool trainer route before adoption; not a missing runtime mechanism. |
| Battle Item Restore Policy | Closed PR #14 / `feature/battle-item-restore-policy` | [test_plan](../features/battle_item_restore_policy/test_plan.md) | `all`, `debug`, focused `check` routes recorded in PR body. | Direct restore and full Oran Berry consume/restore tests recorded. | mGBA test-runner memory read reported pass / exit state. | Default TRUE/FALSE adoption policy still needs final decision. |
| Trainer Battle Aftercare heal hook | Closed PR #10 / `feature/trainer-battle-aftercare-heal` | [test_plan](../features/trainer_battle_aftercare/test_plan.md) | `all`, `debug`, `check` recorded in PR body. | No focused exclusion test suite yet. | mGBA title/boot smoke reached title splash. | Needs focused normal-win and exclusion-path tests before adoption. |
| Champions Partygen catalog expansion | Closed PR #7 / `feature/trainer-partygen-catalog-expansion` | [partygen validation report](../features/champions_challenge/partygen_validation_report.md) | `make`, `make debug`, mdBook recorded in PR body. | Cargo test / clippy / partygen doctor-generate-validate-diff recorded. | mGBA trainer memory read confirmed generated Elite Four party pools. | Implemented closed shelf. Large tool/data/generated workflow review before adoption; Champions runtime remains separate. |
| Trainer Battle Party Selection | `feature/battle-selection-mvp` | [test_plan](../features/battle_selection/test_plan.md) | Branch docs record build/manual evidence. | No current fresh v15 check in this audit. | User manual validation for single/double/party restore recorded. | Re-apply before Team Viewer if not taking #20 wholesale. |
| Field Move Modernization / Field Kit | `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item` | [test_plan](../features/field_move_modernization/test_plan.md) | Branch docs record local validation. | No current fresh v15 check in this audit. | Docs record user-confirmed HM-free MVP and Field Kit itemization. | Needs selected fresh PR / integration branch; graphics are implementation artifacts. |

## Docs-Only Planned Policy Features

| Feature | Docs | mdBook | Local make | Focused tests | mGBA / manual evidence | Known gaps |
|---|---|---|---|---|---|---|
| Nonconsumable Held Items | [test_plan](../features/nonconsumable_held_items/test_plan.md) | Required for this docs-only branch. | Not required until runtime adoption. | Not implemented. Future tests must cover battle-end restore and Bag quantity drift separately. | Not required until runtime adoption. | Battle-end restore is separable from catalog assignment. Party / Bag / Storage UI ownership, Mail exclusion, and stolen / swapped item ownership remain open. |

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
