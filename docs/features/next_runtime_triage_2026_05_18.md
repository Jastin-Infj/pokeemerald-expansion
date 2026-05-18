# Next Runtime Triage 2026-05-18

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `master` `abbbf47554`; `git describe` = `expansion/1.15.2-67-gabbbf47554` |
| Code status | Docs-only triage / no source changes |
| Provenance | `git log --oneline`, `gh pr list --state all`, feature registry, implementation shelf audit, validation evidence matrix |

This page answers "what should be implemented or adopted next from current
master?" after the 2026-05-17 PR cleanup. Most high-priority items below are
already implemented on preserved shelves; the next work is usually fresh-branch
adoption / integration, not writing the feature from scratch.

## Current State

- `master` is still the upstream intake baseline plus docs / workflow overlay.
- Open runtime PRs: none.
- Completed implementation shelves preserved as branches: #41, #39, #31, #28,
  #26, #23, #20, #14, #10, and #7.
- No Random Encounters step-only is complete as shelf #41. Do not spend another
  implementation slice on it unless the scope expands beyond step-only random
  encounters.

## Default Recommendation

If the next task should use already-implemented work, start with
**TM Shop Migration** from completed shelf #31, then decide whether to follow
immediately with the implemented **Field Move Modernization / Field Kit** shelf.

Why:

- It is the cleanest adoption shelf from the PR cleanup.
- It removes the old Emerald normal-progression TM/HM item acquisition path.
- It is a policy prerequisite for Unified Move Relearner and Field Move
  modernization decisions.
- It is mostly data / scripts / config, so it is easier to reason about than
  the larger UI and audio shelves.
- It does not implement the HM replacement item. The single Key Item
  `ITEM_FIELD_KIT`, Field Kit capability flags, Field Kit utility menu, and HM
  acquisition replacement are already implemented on
  `feature/field-move-toolkit-item`.

Recommended branch:

```sh
feature/tm-shop-migration-current-master-20260518
```

Adoption approach:

1. Start from current `master`.
2. Re-apply only the intended #31 runtime slice.
3. Keep FRLG-specific routes out unless explicitly selected.
4. Re-run local validation: `rtk make -j16 -O all`,
   `rtk make -j16 -O debug`, and `rtk make -j16 -O check`.
5. Use one focused mGBA check for boot / map or debug-script route.

Known gaps to keep:

- FRLG-specific TM/HM routes remain follow-up.
- The old mGBA evidence did not fully confirm every HM source route.
- Debug TM shop screen should be rechecked if kept in the slice.

## If You Want New Work Instead

Pick **Jukebox / Sound Archive**.

Why:

- It has no existing runtime shelf to reconcile.
- It uses existing BGM only.
- It avoids SaveBlock, battle hooks, Summary, TM/HM, Move Relearner, Bag, and
  Champions systems.
- It still creates immediate in-game value and can reuse the later Battle BGM
  Selector knowledge without importing any new assets.

Recommended branch:

```sh
feature/jukebox-sound-archive-mvp-20260518
```

MVP:

- Debug menu or safe test NPC entry.
- Fixed list of 8 to 12 existing BGM tracks.
- A plays the selected track.
- B exits.
- Try to restore current map BGM on exit and document any gap.

## Field Kit / HM Item Status

The HM itemization work is already implemented, locally validated, and
user-confirmed on `feature/field-move-toolkit-item`.

Implemented there:

- Single Key Item: `ITEM_FIELD_KIT`.
- Emerald HM acquisition events call `Common_EventScript_GiveFieldKit` instead
  of directly granting HM items.
- Field move capability flags are owned by the field-move feature, not by the
  retired HM receive flag values.
- Modernized HM field moves can require Field Kit + capability flag + existing
  badge gate.
- Key Items "Use" and SELECT registered-item entry open the Field Kit menu.
- Fly / Teleport / Dig are available through the Field Kit utility menu, with
  Fly gated by Field Kit + Fly capability + badge.
- Field Kit icon / palette and debug validation scripts are implemented.

Therefore, do not describe Field Kit / HM itemization as unimplemented. The
remaining work is fresh current-master adoption, conflict review, and validation.

## If You Want High Player Impact

Pick **Field Move Modernization / Field Kit** after deciding whether TM Shop
Migration should land first.

Why:

- The HM-free MVP and Field Kit itemization are already implemented and
  user-confirmed on preserved branches.
- It changes how the game feels during exploration.
- It pairs naturally with retiring old HM item grants.

Caution:

- It is larger than TM Shop Migration and Jukebox.
- It touches field move scripts, item behavior, Field Kit menu, graphics, and
  story acquisition routes.
- Graphics remain implementation artifacts; do not copy them through a
  docs-only master path.

Recommended branch:

```sh
feature/field-kit-current-master-20260518
```

## Candidate Ranking

| Rank | Candidate | Recommended next action | Why now | Main caution |
|---:|---|---|---|---|
| 1 | TM Shop Migration #31 | Fresh adoption branch | Cleanest completed shelf; unlocks TM/HM policy stack. | It retires old TM/HM acquisition but does not itself add Field Kit. |
| 2 | Jukebox / Sound Archive | Fresh new runtime branch | Lowest-risk new feature; no save or battle ownership. | Map BGM restore must be tested. |
| 3 | Field Move Modernization / Field Kit | Fresh adoption branch | HM-free field move path and `ITEM_FIELD_KIT` are already implemented and user-confirmed. | Wider script / item / graphics surface. |
| 4 | Unified Move Relearner #28 | Refresh after #31 | Big quality-of-life feature, already implemented. | Depends on TM/HM policy and still needs teach/overwrite pass. |
| 5 | Summary Tera Type Icon #26 | Fresh adoption branch | Small visible UI improvement. | Imported graphics credit / asset policy. |
| 6 | Pokemon State Editor #23 | Fresh adoption branch or polish pass | Useful debug/training UI already implemented. | Summary UI ownership, box Summary, legality locks. |
| 7 | Pre-Battle / In-Battle Team Viewer #20 | Fresh adoption branch or optional cache regression | High gameplay value, extensive mGBA evidence, and source-audited pool/randomizer cache path. | Adoption conflict handling; optional automated preview-cache assertion. |
| 8 | Battle Item Restore #14 | Fresh adoption branch | Good policy foundation for Champions-style held items. | Default TRUE/FALSE decision still needed. |
| 9 | Battle BGM Selector #39 | Hold unless audio is selected | Implemented and validated, but very large. | Audio asset permission and 487-file diff. |
| 10 | Champions Partygen #7 | Hold for later integration | Tool/data work exists. | Large generated data and runtime facility policy not ready. |

## Not Next By Default

- **No Random Encounters #41**: complete. Only adopt if a playable integration
  branch needs it now.
- **Battle BGM Selector #39**: complete but asset-heavy. Keep as an audio lane,
  not the default next task.
- **Champions runtime**: too many dependencies remain: bag, aftercare, battle
  selection, item restore, and partygen integration policy.
- **Bag Expansion**: important but save-layout-sensitive; decide pocket target
  and migration policy before source work.

## Practical Next Prompt

If no other direction is given, the next implementation prompt should be:

> Start a fresh branch from current `master` and re-apply the completed TM Shop
> Migration shelf #31 as `feature/tm-shop-migration-current-master-20260518`.
> Treat this as adoption of implemented work, not a new design. Keep FRLG routes
> out of scope, update owning docs, run `all`, `debug`, `check`, and one focused
> mGBA validation. If the goal is player-facing HM replacement, follow with a
> separate fresh adoption branch for `feature/field-move-toolkit-item`, where
> `ITEM_FIELD_KIT` and the Field Kit menu are already implemented.
