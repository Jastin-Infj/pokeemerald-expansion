# Validation Evidence Matrix

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `ff4e825258`; `git describe` = `expansion/1.15.2-59-gff4e825258` |
| Code status | Docs-only evidence index |
| Provenance | Feature test plans and current `gh pr list` snapshot |

この matrix は open implementation shelf の横断 evidence。未確認は未確認として残す。
採用前は owning feature の `test_plan.md` を source of truth として再確認する。

## Current Open PR Snapshot

2026-05-17 の `gh pr list` では、#31 / #28 / #26 / #23 / #20 はすべて
`mergeStateStatus = UNKNOWN`。CI は label / allcontributors skip を除き成功済み。

| PR | Feature | Draft | Branch | Merge state | CI snapshot |
|---|---|---|---|---|---|
| #31 | TM Shop Migration | Yes | `feature/tm-shop-migration` | `UNKNOWN` | build / release / test / docs_validate success; label/allcontributors skipped. |
| #28 | Unified Move Relearner | Yes | `feature/unified-move-relearner` | `UNKNOWN` | build / release / test / docs_validate success; label/allcontributors skipped. |
| #26 | Summary Tera Type Icon | Yes | `feature/summary-tera-type-badge` | `UNKNOWN` | build / release / test / docs_validate success; label/allcontributors skipped. |
| #23 | Pokemon State Editor | No | `feature/pokemon-state-editor-expansion` | `UNKNOWN` | build / release / test / docs_validate success; label/allcontributors skipped. |
| #20 | Pre-Battle / In-Battle Team Viewer | Yes | `feature/prebattle-team-viewer` | `UNKNOWN` | build / release / test / docs_validate success; label/allcontributors skipped. |

## Evidence Matrix

| Feature | Docs | mdBook | Local make | Focused tests | mGBA / manual evidence | Known gaps |
|---|---|---|---|---|---|---|
| TM Shop Migration | [test_plan](../features/tm_shop_migration/test_plan.md) | Passed with existing warnings on branch. | `all`, `debug`, `check` passed on 2026-05-16. | Static grep and map JSON parse checks recorded. | Booted, continued save, opened Start menu; HM source route not fully confirmed. | FRLG routes follow-up; debug TM shop screen not confirmed in mGBA; old stale defunct mGBA entry noted. |
| Unified Move Relearner | [test_plan](../features/unified_move_relearner/test_plan.md) | Required / recorded in branch validation context. | `all`, `debug`, `check` passed on 2026-05-16. | JSON audits, candidate list checks, special/form/LGPE smoke checks. | mGBA evidence covers Mew long list, Arceus special moves, Rotom, Cosplay Pikachu, LGPE partners, NPC cancel. | Actual teach / overwrite pass across summary / party / NPC remains recommended before merge. |
| Summary Tera Type Icon | [test_plan](../features/summary_tera_type_icon/test_plan.md) | Passed with existing warnings. | `all`, `debug`, `check` passed on 2026-05-15. | Diff lint only; UI is visual. | mGBA Summary Info screenshot for dual-type Magearna at `(205, 48)`. | Single-type Pokemon and egg stale-icon path not run. |
| Pokemon State Editor | [test_plan](../features/pokemon_state_editor/test_plan.md) | Not separately summarized in test_plan; local docs should be rebuilt before adoption. | `all`, `debug`, `check` passed after final visual follow-up. | Full `check`; no focused unit test listed. | Multiple mGBA sessions confirmed editor pages, values, slide/layout, redraw, and direct Lua data checks. | Several mGBA cleanup attempts left stale status entries; box Summary and legality locks remain follow-up. |
| Pre-Battle / In-Battle Team Viewer | [test_plan](../features/prebattle_team_viewer/test_plan.md) | Not separately summarized in test_plan; local docs should be rebuilt before adoption. | `all`, `debug`, `check` passed after W route. | Full `check`; no automated preview-cache assertion yet. | Extensive mGBA screenshots cover single, double, in-battle viewer, held-key guard, Summary return, details persistence. | Trainer pool / randomized party identity still needs focused validation. |

## Integration Candidates Without Open PR

| Feature | Docs | mdBook | Local make | Focused tests | mGBA / manual evidence | Known gaps |
|---|---|---|---|---|---|---|
| No Random Encounters step-only | [test_plan](../features/no_random_encounters/test_plan.md) | Required for the docs-only adoption branch. | `all`, `debug`, `check` passed on `feature/no-random-encounters-step-only` on 2026-05-09. Not rerun in the 2026-05-17 docs-only pass. | No separate unit test; implementation is a flag id allocation using the existing `CheckStandardWildEncounter` gate. | Route 101 flag OFF wild battle and flag ON no-encounter walking evidence recorded on 2026-05-09. | Current `master` still has `OW_FLAG_NO_ENCOUNTER 0`; runtime adoption needs a fresh branch, 3 file reapply, OFF / ON / OFF-restored mGBA pass, and Fishing / Sweet Scent / Rock Smash / scripted wild remain out of scope. |

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
- Do not change `UNKNOWN` merge state to `CLEAN` without fresh `gh` output.
- Keep skipped long GitHub Actions waits in the feature `test_plan.md`.
