# Feature Branch Audit 2026-05-18

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `master` `1cad0fb5e4`; `git describe` = `expansion/1.15.2-69-g1cad0fb5e4` |
| Code status | Docs-only audit / no source changes |
| Provenance | `git branch -a --sort=-committerdate`, `git log --all`, `gh pr list --state all`, feature docs, branch diffs |

This audit looks at work by feature branch / implementation shelf, not by what
is currently present on `master`. `master` is still docs-only plus workflow
overlay, so runtime feature completion must be judged from preserved feature
branches and their validation evidence.

## Executive Summary

Most recent work is already implemented on feature branches. The remaining work
is not "build every feature from scratch"; it is one of these:

- adopt an implemented shelf on a fresh current-master branch;
- run missing feature-specific validation and fix only if it fails;
- finish a documented follow-up that was explicitly out of scope;
- start a new feature only if we intentionally want a new lane.

If I were choosing the next thing to build/fix, I would start with
**Pre-Battle / In-Battle Team Viewer pool/randomizer consistency**. It is a
known gap on an otherwise implemented feature, and the work is concrete: verify
that cached opponent preview matches the actual trainer pool / randomized party,
then add a focused regression or patch the cache source if it does not.

Recommended branch:

```sh
feature/prebattle-team-viewer-pool-validation-20260518
```

## Completed Shelves

| Feature | Branch / PR | Requirement status | Remaining work |
|---|---|---|---|
| No Random Encounters step-only | `feature/no-random-encounters-step-only-runtime-20260517` / #41 | Implemented and user-confirmed. CI, local make, and Route 101 mGBA evidence recorded. | No new implementation unless scope expands to Fishing / Sweet Scent / Rock Smash / scripted wild. |
| TM Shop Migration | `feature/tm-shop-migration` / #31 | Implemented Emerald legacy TM/HM acquisition retirement and reusable TM config. | Fresh adoption only. FRLG-specific routes and exhaustive HM source NPC runtime checks remain follow-up. |
| Field Move Modernization / Field Kit | `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item` | Implemented HM-free field move path, `ITEM_FIELD_KIT`, capability flags, Field Kit menu, icon, debug scripts, and user-confirmed validation. | Fresh adoption only. Do not describe HM itemization as unimplemented. |
| Unified Move Relearner | `feature/unified-move-relearner` / #28 | Implemented unified level / egg / TM / tutor / special candidate list and long-list UI. | Actual teach / overwrite pass remains recommended before adoption; conflict refresh after TM policy. |
| Summary Tera Type Icon | `feature/summary-tera-type-badge` / #26 | Implemented display-only Summary Tera badge with imported graphics and mGBA evidence. | Single-type / egg stale-icon path and asset-credit review before adoption. |
| Pokemon State Editor | `feature/pokemon-state-editor-expansion` / #23 | Implemented Summary-launched party editor MVP with UI polish and mGBA/direct Lua checks. | Box Summary support, redraw edge cases, and legality locks remain polish/follow-up. |
| Pre-Battle / In-Battle Team Viewer | `feature/prebattle-team-viewer`, `feature/prebattle-team-viewer-phase2` / #20 | Implemented pre-battle preview, selection, cached opponent party, in-battle viewer, Summary return, and extensive mGBA routes. | Pool/randomized-party preview consistency is still the main requirement gap. |
| Battle Item Restore Policy | `feature/battle-item-restore-policy` / #14 | Implemented berry-inclusive battle-end restore with focused tests and mGBA test-runner evidence. | Decide default TRUE/FALSE and adoption scope. |
| Trainer Battle Aftercare heal hook | `feature/trainer-battle-aftercare-heal` / #10 | Implemented default-off trainer win heal hook. | Needs focused config-off, normal-win, and exclusion-path tests before adoption. |
| Trainer Battle Party Selection | `feature/battle-selection-mvp` | Implemented choose-half style trainer battle selection MVP and user validation. | Re-apply before Team Viewer only if not taking the integrated viewer branch wholesale. |
| Battle BGM Selector / Sound Archive | `feature/battle-bgm-selector-mvp-20260517` / #39 | Implemented selector plus BW/BW2, DPPt, Platinum, and HGSS battle BGM imports with build/mGBA evidence. | Asset permission / source risk and large 487-file diff make this a separate audio lane. |
| Champions Partygen catalog expansion | `feature/trainer-partygen-catalog-expansion` / #7 | Implemented Rust CLI, catalog, lint data, and Elite Four / Wallace data diff. | Generated data review and Champions runtime facility policy remain separate. |

## Planned / Not Yet Implemented Runtime

| Feature | Docs | Current status | Next real work |
|---|---|---|---|
| Nonconsumable Held Items catalog assignment | `docs/features/nonconsumable_held_items/` | Docs-only policy. Battle-end restore exists separately, but one-copy catalog assignment is not implemented. | Design Party / Bag / Storage ownership, then implement catalog assignment on a fresh branch. |
| Bag Expansion | `docs/features/bag_expansion/` | Docs-only investigation. | Decide pocket target and save migration policy before any source work. |
| Champions Challenge runtime | `docs/features/champions_challenge/` | Runtime facility not implemented. Partygen tool/data shelf exists. | Wait until battle selection, item restore, aftercare, bag, and partygen adoption are settled. |
| Jukebox / Sound Archive | `docs/features/jukebox_sound_archive/` | Docs-only new feature candidate. | Good low-risk new runtime feature if we intentionally want new work instead of cleanup. |
| Weather Lab / Bounty Board / Field Notes / Route Mastery / Trainer Titles | `docs/features/*/` | Docs-only candidates. | Start only after choosing a new feature lane. |
| Broad no-random encounters | `docs/features/no_random_encounters/` | Step-only MVP implemented; broad-wild is not. | Add Fishing / Sweet Scent / Rock Smash / scripted wild policy only if requested. |

## My Recommended Next Work

### First choice: Pre-Battle Team Viewer pool/randomizer consistency

Why I would do this:

- The feature is already implemented and high-value.
- The remaining gap is concrete and testable.
- It is feature-specific rather than another broad adoption merge.
- It can convert #20 from "implemented with one known risk" to a cleaner
  adoption candidate.

Work shape:

1. Create `feature/prebattle-team-viewer-pool-validation-20260518` from current
   `master` or from a current-master re-application of the viewer shelf.
2. Reproduce a trainer pool / randomized-party route.
3. Compare cached preview party with the actual battle party.
4. Add a focused test or Lua/mGBA validation evidence.
5. If mismatch exists, patch the preview cache source and update
   `docs/features/prebattle_team_viewer/test_plan.md`.

### Second choice: Pokemon State Editor polish

If the goal is tool quality rather than battle flow, pick this instead.

Concrete follow-ups:

- Box Summary support policy.
- Value legality locks.
- Redraw edge cases.

### Third choice: Nonconsumable Held Item catalog assignment

If the goal is new gameplay policy, this is the first not-yet-implemented
feature that connects to existing work. It should build on Battle Item Restore
but stay separate from it.

## Not My Default Next Pick

- **TM Shop Migration**: implemented. Important, but the next work is adoption,
  not building missing requirements.
- **Field Kit**: implemented and user-confirmed. The next work is adoption /
  conflict handling, not initial implementation.
- **Unified Move Relearner**: implemented, but best after TM policy adoption.
- **Battle BGM Selector**: implemented but asset-heavy and should stay in the
  audio lane.
- **Bag Expansion / Champions runtime**: important, but too broad while smaller
  feature gaps remain.
