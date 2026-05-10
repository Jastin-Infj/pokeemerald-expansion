# Pre-Battle / In-Battle Team Viewer Risks

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `7c19f56901`; `git describe` = `expansion/1.15.2-38-g7c19f56901` |
| Code status | Docs-only risk register |
| Provenance | Local project feature docs |

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Preview / battle mismatch | High | Player chooses based on a team that is not used in battle. | Generate once, cache opponent party, and make battle init consume the cache. |
| RNG drift for pool trainers | High | Trainer Party Pools currently default to global RNG. Moving generation before the viewer changes when the party is rolled. | Define that viewer freezes the team at encounter start, and use cached party for battle. Record this as intentional behavior. |
| Calling battle party builder too early | High | `CreateNPCTrainerPartyFromTrainer()` touches `gEnemyParty` and `gBattleStruct`; before `CB2_InitBattle()` this can crash or corrupt state. | Refactor into a preview-safe helper or cache builder that does not require battle resources. |
| Old branch base deletes current docs | High | Merging `feature/battle-selection-mvp` directly can remove newer master docs. | Start from current `master`; cherry-pick / reapply only intended source changes. |
| Player party restore regression | High | Team viewer depends on selection MVP; player `gPlayerParty` is temporarily compressed for battle. | Keep player restoration owned by battle selection and test viewer + selection + battle end together. |
| Sprite / palette exhaustion | Medium | Two 6-mon lists plus type icons may exceed comfortable UI budget. | Use dedicated screen ownership; start with Pokemon icons and compact text, add type icons only after checking budget. |
| Over-disclosing opponent data | Medium | Showing moves / ability / held item can change game balance. | MVP shows species / gender / type only; no moves / ability / item by default. |
| Misreading Champions detail UI | Medium | User reference implies `Y` strength view, but official text did not confirm exact behavior. | Treat `Y` detail view as a requirement, but verify official footage/runtime before copying exact labels or revealing opponent private details. |
| GBA button mismatch | Low | GBA has no physical `Y_BUTTON`, so the exact Champions input cannot be copied. | MVP maps strength view to `SELECT_BUTTON` via `B_TEAM_VIEWER_DETAILS_BUTTON`. |
| Over-weighting legacy games | Low | Colosseum / XD / Battle Revolution have useful battle UI ideas but different hardware, modes, and information rules. | Keep Champions as primary target; use legacy references only for framing and risk checks. |
| Callback re-entry | Medium | Selection MVP already found that starting battle directly from party menu callback can create repeated battle start tasks. | Route viewer and selection exits through field callback / one-shot callback pattern. |
| In-battle button conflict | Medium | `L`, `R`, `START`, and `SELECT` all have existing battle menu uses. | MVP opens only from trainer action menu, uses configurable `B_TEAM_VIEWER_BUTTON`, and starts with `R_BUTTON`. |
| Battle command corruption | High | Opening a viewer from action selection could accidentally emit a command or lose cursor state. | Viewer close must restore action selection without calling `BtlController_EmitTwoReturnValues`. |
| Unsupported trainer modes | Medium | Two opponents, Frontier, link, partner, secret base, Pyramid, Hill have special party generation or return paths. | Exclude them in MVP using the same guard style as battle selection. |
| Integrated selection flow | High | Moving from viewer -> party-menu selection to direct selection inside the viewer changes input ownership, party restore semantics, cancel behavior, and battle start callbacks. | Keep it as Phase 2 and use [Phase 2 Integrated Selection Flow Checklist](phase2_selection_flow_checklist.md) before implementation. |

## Impact Notes

| Feature | Impact |
|---|---|
| Battle Selection | Direct dependency. Team viewer should run before selection and must not take over player party restore in MVP. |
| Trainer Party Pools / partygen | Preview must decide whether it shows raw data or post-pool generated party. MVP contract requires post-pool cached party. |
| Battle UI | In-battle viewer touches player action input. It should not alter healthbox, party status summary, move menu, or command emission. |
| Battle Item Restore / Aftercare | These run at battle end. Team viewer should clear its state before then; selection restore order remains more important. |
| Champions Challenge | Future challenge runtime may replace this viewer with a challenge-specific pre-battle roster screen. Do not hard-code Champions trainer IDs here. |
| Pokemon Icon UI | Dedicated viewer screen should own icon sprites and palettes to avoid party menu lifetime conflicts. |
| Future Integrated Selection | High impact across viewer state, battle selection, party restore, debug route, and validation. See [Phase 2 Integrated Selection Flow Checklist](phase2_selection_flow_checklist.md). |

## Accepted Risks For MVP

- The first implementation may use a two-screen flow: team viewer first, existing selection UI second.
- UI may be visually simpler than the reference image because GBA screen / palette constraints are real.
- Type icons may be deferred if the icon / window budget is tight; species icons are required.
- Viewer cancel can be disabled for the first slice to avoid trainerbattle script ambiguity.
- In-battle viewer may be limited to the action menu at first. Move menu / target selection shortcuts are future work.
- Opponent strength detail may be public-summary-only until official Champions behavior is verified.
- Legacy console references are inspiration, not implementation contract.
- The first implementation maps Champions `Y` to GBA `SELECT`.

## Open Questions

- Whether moving trainer pool roll timing to "encounter start before viewer" is acceptable for all pool trainers.
- Whether exact opponent levels should be shown.
- Whether shiny / form / gender visuals need special handling in the first UI slice.
- Whether `R_BUTTON` remains the best default if future trainer battles add usable quick actions on R.
- Whether `Y` detail view should page between Summary / Moves / Stats or use one compact panel.
