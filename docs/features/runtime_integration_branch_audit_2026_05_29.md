# Runtime Integration Branch Audit 2026-05-29

## Purpose

This audit is the staging map for a future runtime integration branch.
`master` remains the upstream intake baseline plus docs / Lua overlay; runtime
source is not merged directly into `master`.

The proposed integration branch is:

```text
integration/runtime-dev-20260529
```

Use it as a development lane for conflict resolution, build validation, mGBA
Live checks, and cross-feature review. Keep feature PRs and old branches as
evidence shelves until their code is actually re-applied or merged into the
integration lane.

## Rules

- Do not merge implementation branches into `master`.
- Do not treat an open PR as the full runtime queue. Some implemented features
  only exist as closed PRs, preserved branches, or branch-only shelves.
- Before adopting any branch, inspect `master...branch`, not just the PR page.
- For PR-backed work, use PR review commands after creating or updating the
  integration PR. For branch-only shelves, record local review notes in this
  document or the owning feature docs.
- For each adopted feature, validate locally before push:
  `rtk make -j16 -O all`, `rtk make -j16 -O debug` when debug routes are
  touched, and `rtk make -j16 -O check` or focused tests for battle / Pokemon /
  item / save / script logic.

## Open PR Triage

| PR | Branch | Current role | Recommended action |
|---|---|---|---|
| #47 | `feature/battle-item-restore-current-master-20260519` | Runtime shelf for berry-inclusive battle-end held item restore. | Adopted into `integration/runtime-dev-20260529` on 2026-05-29. Keep PR open as source evidence until the integration PR supersedes or closes it. |
| #48 | `feature/held-item-catalog-current-master-20260519` | Runtime shelf for held-item ownership token / catalog assignment. | Adopted into `integration/runtime-dev-20260529` on 2026-05-29 after #47. Keep PR open as source evidence until the integration PR supersedes or closes it. |
| #51 | `feature/scout-selection-runtime-20260520` | Runtime shelf for Pokemon Champions-style scout selection. | Adopted into `integration/runtime-dev-20260529` on 2026-05-29 after #47/#48/#54. Keep PR open as source evidence until the integration PR supersedes or closes it. |
| #54 | `feature/party-status-ui-overhaul-20260521` | Runtime shelf for 2x3 party menu layout. | Adopted into `integration/runtime-dev-20260529` on 2026-05-29 after #47/#48. Keep PR open as source evidence until the integration PR supersedes or closes it. |
| #57 | `feature/global-no-evolution-20260523` | Runtime shelf for Friendly Shop Pokemon Vendor plus global no-evolution policy. | Adopted into `integration/runtime-dev-20260529` on 2026-05-29 after #47/#48/#54/#51. Keep PR open as source evidence until the integration PR supersedes or closes it. |
| #60 | `feature/all-ability-slots-runtime-20260523` | Runtime shelf for battle-only all-active ability slots. | Keep open. Adopt after the item and party-menu stack because it has battle-core and Summary / party overlap. |
| #62 | `feature/champions-run-session-runtime-20260524` | Runtime shelf for Champions run-session save / restore MVP. | Keep open. Adopt late, after roster, item, party, and battle policy branches are settled. |
| #64 | `docs/map-asset-relinker-20260525` | Superseded docs-only map relinker plan PR. | Close as superseded by #66, which already merged the master docs handoff. |
| #65 | `feature/map-asset-relinker-20260525` | Tool implementation lane for map relinker GUI / Rust core. | Keep separate from runtime-dev. This is a tooling lane, not a ROM runtime adoption lane. |

## Runtime Branches Without An Active PR

These branches must be checked alongside open PRs. Some are more important than
the PR list because they hold validated work that was closed or never opened as
a current PR.

| Branch | PR status | Runtime status | Recommended action |
|---|---|---|---|
| `feature/battle-selection-mvp` | No active PR; docs PR #18 exists. | Implemented trainer battle party selection MVP. | Consider as a supporting shelf if the integration does not take Team Viewer wholesale. It overlaps `src/battle_setup.c`, `src/party_menu.c`, and `include/config/battle.h`. |
| `feature/field-move-modernization-mvp` | No active PR. | Implemented HM-free field move MVP. | Adoption candidate if the next integration lane includes HM / field policy. Otherwise keep as a validated shelf. |
| `feature/field-move-toolkit-item` | No active PR. | Implemented Field Kit itemization on top of field move modernization. | Adopt only with the field-move lane; includes graphics and item data, so it is not docs-only. |
| `feature/prebattle-team-viewer` | Closed PR #20. | Implemented pre-battle / in-battle team viewer MVP. | Completed shelf. Consider after party-selection policy is chosen. |
| `feature/prebattle-team-viewer-phase2` | No active runtime PR; docs PR #22 exists. | Implemented phase 2 selection flow on top of Team Viewer. | Prefer this over the older viewer branch if adopting the integrated viewer path. Recheck against Scout Selection and Party UI. |
| `feature/pokemon-state-editor-expansion` | Closed PR #23. | Implemented Summary-launched Pokemon State Editor MVP. | Completed shelf. Integration depends on Summary / Party UI and vendor edit-entitlement decisions. |
| `feature/summary-tera-type-badge` | Closed PR #26. | Implemented display-only Summary Tera icon. | Small UI shelf; adopt only after asset credit and Summary layout ownership are settled. |
| `feature/unified-move-relearner` | Closed PR #28. | Implemented unified level / egg / TM / tutor / special move candidate list. | Completed shelf. Prefer after TM Shop Migration; teach / overwrite runtime proof remains recommended before final adoption. |
| `feature/tm-shop-migration` | Closed PR #31. | Implemented TM/HM acquisition retirement and reusable TM config. | Clean adoption shelf, but not a missing implementation. Prefer before Unified Move Relearner if the TM/HM lane resumes. |
| `feature/no-random-encounters-step-only-runtime-20260517` | Closed PR #41. | Implemented and user-confirmed step-only random encounter suppression. | Completed shelf. Re-apply only if runtime-dev wants the no-random flag now. |
| `feature/battle-bgm-selector-mvp-20260517` | Closed PR #39. | Implemented battle BGM selector plus large imported audio set. | Separate audio lane. Do not mix into the first runtime-dev pass unless audio becomes the selected focus. |
| `feature/trainer-battle-aftercare-heal` | Closed PR #10. | Implemented default-off trainer battle win heal hook. | Completed shelf needing focused exclusion tests before adoption. |
| `feature/trainer-partygen-catalog-expansion` | Closed PR #7. | Implemented partygen CLI / catalog / Elite Four and Wallace trainer pool data. | Tool/data shelf for Champions integration. Review generated data before adopting into runtime-dev. |
| `feature/champions-partygen-next-slice` | No active PR. | Docs-only follow-up for partygen / battle selection ordering. | Reference only unless docs need to be re-synced. |

## Legacy Or Reference Branches

These branches may still contain useful code or context, but they should not be
merged wholesale into runtime-dev without a separate restart plan.

| Branch | Notes |
|---|---|
| `feature/EX/ex-rz-upstream1` | Large randomizer branch with item ball icons, trainer rank parties, randomizer config, graphics, generated headers, tools, and tests. Treat as a separate randomizer lane. |
| `feature/ex-rz-upstream1` | Older randomizer branch. Superseded by the broader `feature/EX/ex-rz-upstream1` work for most purposes. |
| `feature/new-map` | Older map / region map / Fly experiment. Reference for map relinker and Fly icon behavior, not a clean runtime adoption branch. |
| `feature/new-map-test-v15` | Rouge Cave map draft from closed PR #4. It had CI / draft risk. Restart as a fresh map branch if needed. |
| `feature/qol_field_moves`, `feature/modern-qol-field-moves`, `feature/field-surfboard` | Older field move experiments. Prefer `feature/field-move-modernization-mvp` and `feature/field-move-toolkit-item` as the current field-move shelves. |
| `feature/party-select-ui` | Older party selection UI experiment. Prefer `feature/battle-selection-mvp` or Team Viewer phase 2. |
| `feature/docs-first-next-work-20260515` | Early Summary Tera docs / graphics branch. Prefer `feature/summary-tera-type-badge`. |
| `feature/no-random-encounters`, `feature/no-random-encounters-step-only` | Earlier no-random branches. Prefer `feature/no-random-encounters-step-only-runtime-20260517`. |

## Zero-Diff Or Already-Absorbed Branches

These currently have no meaningful `master...branch` diff or are already
absorbed by docs-only master work:

- `feature/bag-expansion`
- `feature/summary-first-relearner-runtime-20260522`
- `feature/battleMain`
- `feature/birch_case`
- `feature/dynamicmulti`
- `feature/main_menu`
- `feature/move_relearner`
- `feature/poryscript`
- `feature/releaseSystem`
- `feature/sandbox_v12`

Keep them for history unless the user explicitly asks to prune stale branches.

## Conflict Hotspots

These are the files most likely to define the integration order.

| File | Branches / PRs touching it | Integration risk |
|---|---|---|
| `src/party_menu.c` | #48, #54, #57, #60, #62, plus battle selection / Team Viewer shelves | Highest risk. Decide the Party UI baseline before broad adoption. |
| `src/battle_util.c` | #47, #60, #62 | Battle-end restore, all ability slots, and Champions outcome/restore paths overlap. |
| `include/config/battle.h` | #47, #60, battle selection / Team Viewer | Config naming and defaults need one owner per rule. |
| `data/scripts/debug.inc` | #51, #57, #62, #65, field/tool branches | Debug menu conflicts are easy but noisy; rename debug commands before adopting. |
| `data/specials.inc` | #51, #57, #62 | Special registration ordering and duplicate names must be checked after every adoption. |
| `data/event_scripts.s` | #51, #57, #65, map / field branches | Script include order can produce hidden build or runtime issues. |
| `src/pokemon_summary_screen.c` | #57, #60, State Editor, Summary Tera | Summary UI ownership must be settled before combining editor, vendor, ability selector, and Tera badge. |
| `src/pokemon_storage_system.c` | #48, #57 | Held item token policy and sealed recruit PC display overlap. |
| `src/battle_setup.c` | #57, #62, battle selection / Team Viewer | Trainer battle rewards, party selection, and Champions loss policy overlap. |
| `src/battle_script_commands.c` | #60, #62 | Ability slot behavior and Champions battle outcome hooks both touch battle script logic. |

## Dependency Findings

## Integration Progress

| Date | Runtime branch | Adopted feature | Evidence | Next likely target |
|---|---|---|---|---|
| 2026-05-29 | `integration/runtime-dev-20260529` | #47 Battle Item Restore | Source / tests re-applied from `feature/battle-item-restore-current-master-20260519`; `rtk git diff --check`, focused `TESTS=battle_item_restore`, `all`, `debug`, and mGBA Live boot/input smoke passed. | #48 Held Item Catalog, then #54 Party / Status UI if item ownership remains the first lane. |
| 2026-05-29 | `integration/runtime-dev-20260529` | #48 Held Item Catalog | Source / tests re-applied from `feature/held-item-catalog-current-master-20260519`; focused `TESTS=test/bag.c`, `all`, `debug`, mdBook, and mGBA Live boot/input smoke passed. | #54 Party / Status UI, because #48 now owns item-assignment hooks in `src/party_menu.c`. |
| 2026-05-29 | `integration/runtime-dev-20260529` | #54 Party / Status UI | Source re-applied from `feature/party-status-ui-overhaul-20260521`; `party_menu.c` applied cleanly after #48 and retained the held-item catalog helpers; `all`, `debug`, full `check`, and mGBA Live boot/input smoke passed. | #51 Scout Selection or #57 Pokemon Vendor, with manual Party / item UI recheck first. |
| 2026-05-29 | `integration/runtime-dev-20260529` | #51 Scout Selection | Source / tools / docs re-applied from `feature/scout-selection-runtime-20260520`; `make generated`, `all`, `debug`, full `check`, and mGBA Live Scout route validation passed. | #57 Pokemon Vendor, because Scout gift + 2x3 Party menu now prove the party baseline can accept new Pokemon gifts. |
| 2026-05-29 | `integration/runtime-dev-20260529` | #57 Friendly Shop Pokemon Vendor | Source / tests re-applied from `feature/global-no-evolution-20260523`; `data/scripts/debug.inc` conflict was resolved as vendor `Script 1`, Scout pick-6 `Script 2`, and vendor trainer reward `Script 3`; focused vendor check, `all`, `debug`, and mGBA Live vendor purchase smoke passed. Full hydra `check` is not green due only to timing-sensitive `test/random.c` benchmark jitter that passed when focused. | #60 All Ability Slots, because the item / party / Summary / vendor stack is now present. |
| 2026-05-29 | `integration/runtime-dev-20260529` | #60 All Ability Slots | Source / tests re-applied from `feature/all-ability-slots-runtime-20260523`; integration default changed to `B_ALL_ABILITY_SLOTS FALSE` while keeping the config max `TRUE`, so normal ROM behavior remains opt-in. Focused All Ability, AI thinking-time, battle item restore, vendor, `all`, `debug`, full `check`, and mGBA Live Pattern A recoil smoke passed. | #62 Champions Run Session, because battle / item / party / Summary / vendor policy is now present. |

### Item Policy Stack

| Feature | Depends on | Notes |
|---|---|---|
| #47 Battle Item Restore | `include/config/battle.h`, `src/battle_main.c`, `src/battle_util.c` | This is the smallest item-policy foundation. It should be adopted before any feature assumes battle-consumed Berries return after battle. |
| #48 Held Item Catalog | Bag / Party / Storage item assignment paths | This changes physical held-item quantity semantics. It can exist without #47, but testing is clearer after #47 because battle-end restoration and catalog ownership are separate but adjacent policies. |
| #57 Pokemon Vendor | Held item catalog, party menu, storage display | Vendor sealed recruits need item-edit entitlement hooks and PC / party visibility. Adopt after #48 if vendor-held-item editing is enabled. |
| #62 Champions Run Session | Bag snapshot / restore, held item carryover policy | Champions clear can strip, keep, or carry held items. It should consume the already-decided item policy rather than define one itself. |

Open decision before runtime-dev adoption:

- whether #47 `B_RESTORE_HELD_BATTLE_BERRIES` defaults `TRUE` in integration;
- whether #48 catalog mode applies only to held-effect items or a broader item
  list;
- whether Champions clear carryover merges held items into the normal bag,
  leaves them on deposited Pokemon, or discards them for the first integration
  pass.

### Party And Summary UI Stack

| Feature | Depends on | Notes |
|---|---|---|
| #54 Party / Status UI | `include/constants/party_menu.h`, `src/data/party_menu.h`, `src/party_menu.c` | Should set the baseline for all later party-menu changes. |
| #48 Held Item Catalog | `src/party_menu.c`, `src/item_menu.c`, `src/shop.c`, `src/pokemon_storage_system.c` | Item give/take behavior must be rechecked after the 2x3 party layout is adopted. |
| #57 Pokemon Vendor | `src/party_menu.c`, `src/pokemon_summary_screen.c`, `src/pokemon_storage_system.c` | Needs locked / sealed-origin labels in party, Summary, and PC. |
| #60 All Ability Slots | `src/party_menu.c`, `src/pokemon_summary_screen.c` | Ability Capsule / Patch path and Summary ability display overlap with vendor / editor / relearner UI. |
| State Editor | `src/pokemon_summary_screen.c` | Summary-launched editor should not add a party-menu action. It should wait until Summary ownership is settled. |
| Unified Move Relearner | `src/pokemon_summary_screen.c`, `src/party_menu.c` | Existing docs prefer Summary-first integration. Direct party action can remain optional or debug/fallback. |
| Summary Tera Badge | `src/pokemon_summary_screen.c`, graphics | Small display shelf, but it competes for Summary visual space. |
| Team Viewer Phase 2 | `src/party_menu.c`, `src/pokemon_summary_screen.c` | Uses Summary return and selection state. It should be tested after party layout changes. |

Open decision before runtime-dev adoption:

- adopt #54 before #48 / #57 / #60 / #62, or intentionally preserve the old
  party layout for the first integration pass;
- keep Summary as the canonical entry for Move Relearner and State Editor;
- defer full BW Summary replacement until after the first integration branch is
  stable.

### Battle Flow Stack

| Feature | Depends on | Notes |
|---|---|---|
| #47 Battle Item Restore | Battle end restore path | Runs at battle shutdown and must not fight party-restore hooks. |
| Battle Selection MVP | `src/battle_setup.c`, `src/party_menu.c` | Compresses selected Pokemon for battle, then restores state. |
| Team Viewer Phase 2 | Battle Selection MVP, `src/battle_main.c`, `src/battle_controller_player.c` | Owns pre-battle preview, in-battle read-only viewer, and cached opponent party. |
| #57 Pokemon Vendor | Trainer win bond EXP queue in battle victory text | Needs battle-after text flow but should not own generic reward tables. |
| #60 All Ability Slots | Battle utility predicates and battle script commands | High battle-core blast radius. Adopt after smaller battle-end and selection hooks are stable. |
| #62 Champions Run Session | Battle outcome interception, EXP suppression, loss restore | Should be late because it changes loss / draw / forfeit semantics and save behavior. |
| Trainer Aftercare | Normal trainer win post-battle hook | Needs exclusion tests and ordering relative to #47, #57 reward text, and #62 Champions outcome. |

Open decision before runtime-dev adoption:

- whether Team Viewer Phase 2 replaces the separate Battle Selection MVP, or
  whether both are adopted as separate entry points;
- whether Pokemon Vendor bond EXP remains product-specific script reward first,
  trainer-win queued reward second;
- whether Trainer Aftercare is included in the first runtime-dev pass or left
  as a later post-battle policy lane.

### Save, PC, And Run-State Stack

| Feature | Save / storage dependency | Notes |
|---|---|---|
| #51 Scout Selection | No SaveBlock field in MVP | Candidate selection is script-driven and can remain stateless. |
| #57 Pokemon Vendor | Per-mon sealed marker bits, PC display, optional progress | The current branch uses Pokemon data / helper policy rather than a broad SaveBlock product table for every state. |
| #62 Champions Run Session | Adds `struct ChampionsRunSession` to `SaveBlock1` | Highest save-risk feature in the current queue. It snapshots normal party / bag / money / location and blocks PC during active run. |
| Partygen Catalog | Tool/data only unless generated trainer data is adopted | Do not make ROM build depend on partygen until generated drift checks are accepted. |
| Runtime Rule Options | SaveBlock2 / option UI candidate | Keep out of the first integration unless a concrete option owner exists. |

Open decision before runtime-dev adoption:

- verify #62 SaveBlock1 budget again after any branch that changes
  `include/global.h` or save structs;
- decide whether vendor sealed progress is allowed while the Pokemon is in PC;
- keep normal PC access blocked during active Champions runs until a run-only
  storage design exists.

### Script, Special, And Debug Namespace Stack

The current queue has many debug and special registrations:

- #51 Scout Selection: `data/event_scripts.s`, `data/scripts/debug.inc`,
  `data/specials.inc`, `src/debug.c`.
- #57 Pokemon Vendor: `asm/macros/event.inc`, `data/event_scripts.s`,
  `data/script_cmd_table.inc`, `data/scripts/debug.inc`,
  `data/specials.inc`, `src/debug.c`, `src/scrcmd.c`.
- #62 Champions Run Session: `data/scripts/debug.inc`, `data/scripts/pc.inc`,
  `data/specials.inc`, `src/debug.c`.
- #65 Map Asset Relinker: debug map / Fly validation changes, but this is a
  tooling lane and should not be mixed into runtime-dev by default.

Integration rule:

- rename generic debug entries before combining branches;
- keep feature-specific debug labels such as `Scout Selection`, `Pokemon
  Vendor`, and `Champs: ...`;
- re-run `make debug` after each branch that changes `data/event_scripts.s`,
  `data/specials.inc`, `data/script_cmd_table.inc`, or `asm/macros/event.inc`.

### Generated Data And Tool Stack

| Branch | Role | Runtime-dev guidance |
|---|---|---|
| `feature/trainer-partygen-catalog-expansion` | Rust CLI / catalog / generated trainer data shelf | Review generated trainer data separately. Useful for Champions and Scout pools, but not required for the first save/session integration. |
| #51 Scout Selection | Consumes a generated partygen JSON pool for the demo pool | Can be adopted without turning partygen into a ROM build dependency. |
| `feature/unified-move-relearner` | Generates learnset candidate headers from porymoves JSON | Re-run generator after TM/HM policy changes. Do not mix with partygen generation. |
| #65 Map Asset Relinker | Tooling lane with Rust core / Tauri GUI / Python compatibility | Keep outside ROM runtime-dev. Its generated/fixture map data should not be adopted into gameplay integration. |

## Hard Gates Before First Runtime-Dev Merge

1. Choose whether #54 Party UI is the first UI baseline.
2. Choose whether the first runtime-dev pass includes Team Viewer Phase 2 or
   only Scout Selection / Vendor debug routes.
3. Choose #47 and #48 item-policy defaults.
4. Decide whether #62 Champions Run Session is included in the first pass or
   waits until item / party / scout / vendor integration is green.
5. Keep #65 Map Asset Relinker out of the ROM runtime integration lane.
6. Close #64 only after recording that #66 superseded it.

## Proposed Runtime-Dev Order

This is a starting point, not a merge command list.

1. Create `integration/runtime-dev-20260529` from current `master`.
2. Add this audit doc and keep `master` docs-only.
3. Adopt #47 Battle Item Restore first if the item policy stack is selected.
4. Adopt #48 Held Item Catalog after #47.
5. Adopt #54 Party / Status UI before branches that depend on party layout.
6. Adopt #51 Scout Selection after the party baseline can be tested.
7. Adopt #57 Friendly Shop Pokemon Vendor after held-item and party UI policy.
8. Adopt #60 All Ability Slots after battle core and Summary ownership are
   less volatile. Adopted on `integration/runtime-dev-20260529` with
   `B_ALL_ABILITY_SLOTS` default `FALSE`.
9. Adopt #62 Champions Run Session after roster, item, party, and battle policy
   are present.
10. Add real Champions facility connection work after #62, not before.

Optional lanes that should be explicitly selected before inclusion:

- TM/HM lane: `feature/tm-shop-migration` ->
  `feature/unified-move-relearner` -> field move shelves.
- Team preview lane: `feature/prebattle-team-viewer-phase2`, optionally with
  `feature/battle-selection-mvp` if not taking the integrated viewer path.
- Editor lane: `feature/pokemon-state-editor-expansion`, after Summary / Party
  UI and Pokemon Vendor entitlement hooks are settled.
- Audio lane: `feature/battle-bgm-selector-mvp-20260517`.
- Tool lane: #65 Map Asset Relinker, outside ROM runtime-dev.
- Randomizer lane: `feature/EX/ex-rz-upstream1`, outside the first integration
  pass.

## Review Workflow

For each candidate branch:

1. Inspect `git diff --name-status master...<branch>`.
2. Inspect `git log --oneline --no-merges master..<branch>`.
3. If the branch has a PR, review the PR diff and comments before adoption.
4. Apply into `integration/runtime-dev-*` with either merge, cherry-pick, or
   manual re-application; do not use the GitHub merge button into `master`.
5. Resolve conflicts while preserving current `master` docs and branch-specific
   runtime source.
6. Run the relevant local validation.
7. Attempt one focused mGBA Live check for runtime-visible behavior.
8. Record adopted commit, conflict choices, validation, and remaining risks in
   the owning feature docs.

## Current Recommendation

Do not close runtime PRs yet, except the superseded docs-only #64. Keep them as
source shelves until `integration/runtime-dev-20260529` contains equivalent or
better code. Once a feature has been adopted into runtime-dev and validated,
update the owning docs and then close the old PR with a superseded-by note.
