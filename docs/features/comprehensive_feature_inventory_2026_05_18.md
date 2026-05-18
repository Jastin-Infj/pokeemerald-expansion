# Comprehensive Feature Inventory 2026-05-18

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `master` `187df44eb6`; `git describe` = `expansion/1.15.2-73-g187df44eb6` |
| Code status | Docs-only audit / no source changes |
| Provenance | `gh pr list --state all --limit 120`, `git for-each-ref`, branch logs, branch head file lists, existing feature docs |

This inventory exists to prevent priority drift. A closed PR, stale PR, or
branch without an open PR can still contain real runtime implementation. It is
not permission to merge runtime source into `master`; it is evidence for choosing
the next fresh feature / integration branch.

## Current-Master-Lineage Implementation Shelves

These branches are close enough to the current v15 workstream to be treated as
implementation shelves. They still need fresh adoption branches before source
integration because `master` remains docs-only for local runtime code.

| Feature | Branch / PR | Implementation state | Priority impact |
|---|---|---|---|
| No Random Encounters step-only | `feature/no-random-encounters-step-only-runtime-20260517` / #41 | Implemented, CI-passed, locally built, mGBA/user-confirmed. | Do not re-build. Adopt only when a playable integration wants the flag. |
| TM Shop Migration | `feature/tm-shop-migration` / #31 | Implemented Emerald TM/HM acquisition retirement and reusable TM config. | Clean adoption candidate, but not a missing feature. |
| Field Move Modernization / Field Kit | `feature/field-move-modernization-mvp`, `feature/field-move-toolkit-item` | Implemented HM-free field move access, Field Kit item/menu/icon/capability flow, and user-confirmed validation. | Treat HM replacement as done on branch; adoption/conflict work only. |
| Unified Move Relearner | `feature/unified-move-relearner` / #28 | Implemented unified level/egg/TM/tutor/special candidate list and long-list UI. | Runtime exists. Remaining work is adoption refresh and teach/overwrite proof. |
| Summary Tera Type Icon | `feature/summary-tera-type-badge` / #26 | Implemented display-only Summary badge with graphics and mGBA evidence. | Small adoption candidate after asset credit review. |
| Pokemon State Editor | `feature/pokemon-state-editor-expansion` / #23 | Implemented Summary-launched party editor MVP with mGBA/direct Lua evidence. | Polish candidate: box Summary, legality locks, redraw edge cases. |
| Pre-Battle / In-Battle Team Viewer | `feature/prebattle-team-viewer`, `feature/prebattle-team-viewer-phase2` / #20 | Implemented preview, integrated selection, in-battle viewer, Summary return, and pool/randomizer cache path. | Not missing. Add optional cache regression only if adopting. |
| Battle Item Restore Policy | `feature/battle-item-restore-policy` / #14 | Implemented berry-inclusive battle-end held item restore with focused tests and mGBA evidence. | Decide default/adoption scope; catalog assignment is separate. |
| Trainer Battle Aftercare heal hook | `feature/trainer-battle-aftercare-heal` / #10 | Implemented default-off trainer win heal hook. | Needs focused exclusion tests before adoption. |
| Trainer Battle Party Selection | `feature/battle-selection-mvp` / docs #18 | Implemented choose-half selection MVP and user validation. | Mostly superseded by Team Viewer integrated flow unless used separately. |
| Champions Partygen catalog | `feature/trainer-partygen-catalog-expansion` / #7, supersedes #5 | Implemented Rust CLI, catalog, lint, Elite Four/Wallace Trainer Party Pool output, and mGBA ROM-memory evidence. | Tool/data shelf is real; Champions facility runtime is separate. |
| Battle BGM Selector / imports | `feature/battle-bgm-selector-mvp-20260517` / #39 | Implemented debug selector and large imported battle BGM set with validation. | Audio lane only; asset/source review before adoption. |
| Rouge Cave map draft | `feature/new-map-test-v15` / #4 | Runtime map draft exists, but PR was closed with CI failure/draft risk. | Reference or restart on fresh branch; not a default next pick. |

## Docs-Only Or Not-Yet-Implemented Runtime

These are still real candidates, but the audit did not find current v15 runtime
implementation shelves equivalent to the docs.

| Feature | Current state | Priority impact |
|---|---|---|
| Nonconsumable Held Item catalog assignment | Docs-only. Battle-end restore exists separately, but one-copy Party/Bag/Storage assignment is not implemented. | Best fresh gameplay implementation candidate if not adopting an existing shelf. |
| Bag Expansion | Docs-only investigation. | Needs save compatibility decision before source work. |
| Champions Challenge runtime facility | Docs-only runtime design. Partygen exists, facility loop does not. | Too broad until bag, item restore, battle selection, and partygen adoption decisions are settled. |
| Jukebox / Sound Archive | Docs-only candidate. Battle BGM selector exists separately. | Best low-risk new feature if the goal is visible new runtime work. |
| Weather Lab, Bounty Board, Field Notes, Route Mastery, Trainer Titles | Docs-only candidates. | Start only if intentionally choosing a new lane. |
| Broad no-random encounter mode | Step-only branch exists; fishing/Sweet Scent/Rock Smash/scripted wild hooks are out of scope. | Not needed unless the scope expands beyond walking encounters. |

## Legacy Runtime Reference Branches

These branches contain real implementation work, but they are older baseline
references. Do not merge them into current `master`. Use them as behavior
references and re-implement selected behavior on a fresh current-master branch.

| Branch | Evidence from head commit / files | How to treat it |
|---|---|---|
| `origin/TM_v12_0` / PR #1 | "TM Gen9 complated. save block used"; touches item constants, TM/HM constants, teachable learnsets, item menu. | Real old TM Gen 9 implementation. Reference only; current TM policy should use #31 / current docs. |
| `origin/item_clock` | Clock key item with icon/palette, custom script, item use hook. | Real item prototype. Reference for key item UX, not current candidate. |
| `origin/item_heal_patry` | Heal party key item with icon/palette, obtain text, item use hook. | Real item prototype. Reference for reusable item behavior. |
| `origin/item_keyfly` | Flying key item with icon/palette and item use hook. | Real key-item Fly prototype; overlaps Field Kit / field move work. |
| `origin/feature/ex-rz-upstream1` | Trainer randomizer code functions and whitelist/blacklist work. | Old randomizer reference; not current partygen shelf. |
| `origin/feature/EX/ex-rz-upstream1` | Item randomizer specs, configurable item weights, item ball icon support, trainer rank randomization docs. | Real randomizer/reference branch; separate from current partygen tool. |
| `origin/feature/battleMain` | "randomizer patry teams"; touches `battle_main.c`, `trainers.h`, `trainers.party`. | Old party randomizer implementation reference. |
| `origin/dev` | "debug command patry select"; touches debug scripts and Battle Tower scripts. | Old debug/party-select scratch reference only. |
| `origin/feature/releaseSystem` | "release system worked"; adds `pokemon_release_mon` and battle setup hook. | Real forced release prototype; relevant to future aftercare/roguelike policy. |
| `origin/feature/move_relearner` | "move learn complated"; touches move relearner scripts, constants, party menu, Summary, learnset helper. | Old move relearner reference; current shelf is `feature/unified-move-relearner`. |
| `origin/feature/main_menu` | Custom main menu UI assets and `ui_main_menu.c`. | Real UI prototype; not part of current runtime order. |
| `origin/feature/qol_field_moves` | Large QOL field move branch with scripts, flags, item use, party menu, region map, field move source. | Old field move reference; current Field Kit branches supersede for v15. |
| `origin/feature/modern-qol-field-moves` / `origin/feature/field-surfboard` | Field move visual/effect experiments. | Reference only. |
| `origin/feature/party-select-ui` | Battle party select UI prototype with debug script and battle setup/main hooks. | Reference only; current battle selection / Team Viewer shelves supersede. |
| `origin/feature/new-map` | Older map/Fly/region map work. | Reference only; current map draft is #4 and map manual docs. |
| `origin/feature/birch_case` | Birch case UI assets and script/start menu hooks. | Real UI prototype, currently outside feature registry priorities. |
| `origin/feature/dynamicmulti` | Tutorial/map data draft. | Reference only. |
| `origin/feature/sandbox_v12` | Ability/stat editor UI fix on old v12 baseline. | Reference only; current State Editor shelf supersedes for v15. |

## Priority Correction From This Audit

Do not pick a task just because docs say "investigating" if an implementation
shelf exists. The next runtime decision should first choose between two modes:

| Mode | Best next target | Why |
|---|---|---|
| Adopt an implemented shelf | TM Shop Migration, Field Kit, Unified Relearner, Team Viewer, Pokemon State Editor, Summary Tera, Battle Item Restore, or Battle BGM depending on desired lane. | Most gameplay-visible features already exist on branches. Work is re-apply/conflict/validation, not first implementation. |
| Finish a shelf's remaining hardening | Pokemon State Editor polish, Team Viewer cache regression, Battle Item Restore default decision, Trainer Aftercare exclusion tests. | Smallest way to turn a shelf into a cleaner adoption candidate. |
| Build genuinely new runtime | Nonconsumable Held Item catalog assignment or Jukebox. | These are still not implemented on current v15 shelves. |
| Mine old references | Release system, old key items, old randomizer, old main menu, old Birch case. | Only after deciding the behavior is still wanted; re-implement on current master. |

My current recommendation after this broader audit:

1. If we are adopting implemented work: **TM Shop Migration** remains the cleanest first adoption branch.
2. If we are hardening before adoption: **Pokemon State Editor polish** is the narrowest useful cleanup.
3. If we are writing new runtime: **Nonconsumable Held Item catalog assignment** is the clearest not-yet-implemented gameplay policy; **Jukebox** is the lower-risk visible feature.
4. Do not spend priority on implementing Team Viewer, partygen, Field Kit, no-random step-only, Summary Tera, or Unified Move Relearner from scratch; those are already implemented shelves.
