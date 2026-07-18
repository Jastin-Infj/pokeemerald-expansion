# 16.0 Runtime Port Status

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-06-02 |
| Upstream baseline | `master` `4faec7cb08` / `expansion/1.16.0-104-g4faec7cb08` |
| Runtime branch | `snapshot/runtime-1.16.0/full-stack-20260531` (formerly `integration/runtime-dev-16-20260531`) |
| Runtime PR | Closed PR #69 / `[codex] Port runtime integration to 16.0 baseline`; not merged into `master` |
| Previous runtime shelf | PR #68 / `integration/runtime-dev-20260529` |
| Policy | This branch is the completed 15.3-to-16.0 runtime port snapshot. Upstream 16.0 source wins unless a local runtime feature must be replayed intentionally. |

## Summary

`master` was synced to the upstream 16.0 line, then the 1.15.3 runtime work was replayed onto a fresh integration branch. The 15.3 docs remain the evidence shelf; this file records what changed while moving that implementation set onto 16.0.

The port keeps the normal branch rule:

- `master` remains upstream intake plus docs / Lua-only overlay.
- Runtime source, data, graphics, generated output, and non-Lua tools stay on an integration / feature branch.
- Map Asset Relinker desktop binaries are release / Actions artifacts, not tracked source files.

## Branch Status

`snapshot/runtime-1.16.0/full-stack-20260531` is complete as the 15.3-to-16.0 runtime
port snapshot. It should remain available as the comparison point for the
finished replay from PR #68 to PR #69.

Do not use this completed snapshot as the branch where future runtime features
keep accumulating. For new 16.0-native implementation, map/content work,
generated-data work, or reworked feature ports, create a fresh branch from
current `master`. If a playable all-in runtime dev branch is needed, duplicate
this snapshot into a new `integration/*` branch first and apply the new work to
that duplicate.

This keeps three review surfaces separate: upstream `master`, the completed
15.3-to-16.0 port snapshot, and any future 16.0 dev branch built on top of it.
The historical 15.3 branches and docs remain reference material, not the
canonical implementation for future 16.0 work.

## Ported Runtime Areas

| Area | 16.0 port status | Notes |
|---|---|---|
| All Ability Slots | Ported with review fixes | Battle API / helper drift from 16.0 was resolved. Trace, Teraform Zero, Supersweet Syrup, Mega Sol, Magician, Unnerve, stat-drop blockers, priority blockers, flinch chance, and AI thinking-time guards were revalidated against the new baseline. Follow-up review fixes make defaulted `ABILITY_NONE` callers enumerate all active slots, suppress switched-out still-alive battlers, let hidden-slot status-immunity abilities cure existing status, and let hidden-slot Gen 8 Intimidate blockers / Guard Dog participate in Intimidate handling. Receiver / Power of Alchemy on fainted allies remain covered. |
| Champions Run Session | Ported | Party APIs were moved to `gParties[B_TRAINER_PLAYER]`. Loss / retire restore now clears the one-shot Continue warp flag before the normal restore save, avoiding stale reboot warp state. |
| Nonconsumable Held Items | Ported with review fix | Codex review found berries were incorrectly treated as catalog ownership tokens. `POCKET_BERRIES` is now excluded so consumable berries remain physical inventory. |
| Pokemon Vendor / Scripted GiveMon | Ported | Tests were updated for 16.0 party storage symbols. Vendor reward and sealed-origin behavior remain covered by focused tests. |
| Battle Item Restore | Ported | Restore tests were adjusted around 16.0 config interactions and berry animation expectations. |
| Trainer Battle Selection / Team Viewer | Ported | Short-party config remains integrated. Single 1-mon is allowed; double defaults to minimum 2 mons. |
| Battle BGM / No Random Encounters / Field Kit / Scout Selection / State Editor | Ported from #68 | Field Kit received one compatibility repair: old saves that already had legacy HM receipt flags but no Field Kit now auto-migrate the Field Kit item when checking an unlocked modern field move. The other areas needed no new code repair beyond shared build and smoke validation. |
| TM Shop Migration | Ported with review fixes | New Mauville / Wattson script flow was adjusted for 16.0 replay: Wattson now remains in Mauville City until the post-generator completion conversation runs, then that conversation moves him back to the Gym. A follow-up 2026-06-02 review fix preserves old-save Meteorite-return state by treating the retired `0xE5` TM Return flag as a legacy alias and migrating it to the 16.0 `FLAG_RETURNED_METEORITE_TO_COZMO` story flag without restoring the removed TM reward. |
| Map Asset Relinker | Ported as tooling | Rust core, shell wrapper, GUI, Tauri build scripts, and desktop packaging workflow remain present. Windows `.exe` distribution should come from Actions artifact / release packaging. |

## Review And Validation

| Check | Result | Notes |
|---|---|---|
| `rtk codex review --base master` | Fixed | First pass reported held-item catalog ownership included consumable berries. Second pass reported two All Ability Slots regressions: defaulted ability-effect callers skipped extra slots, and switched-out battlers could still expose abilities. Third pass reported that the New Mauville generator state moved Wattson back to the Gym before his completion conversation. The 2026-06-02 pass reported old-save Meteorite-return compatibility for Cozmo's house. All were fixed with focused coverage, script-flow validation, or source-level save-compatibility audit. |
| `rtk codex review --uncommitted` | Stopped after useful exploration | The uncommitted closeout review again expanded into a long-running broad audit and was stopped manually. Before stopping, it highlighted the same high-risk All Ability Slots families; the follow-up patch covers hidden-slot status-immunity cure and hidden-slot Gen 8 Intimidate blockers. |
| `rtk make -j16 -O check TESTS=test/bag.c` | Pass | 11 tests, including `Held item catalog ownership leaves consumable berries physical`. |
| `rtk make -j16 -O check TESTS='All Ability Slots lets non-representative Scrappy block Intimidate'` | Pass | Confirms hidden-slot Scrappy blocks Intimidate under Gen 8 Intimidate behavior. |
| `rtk make -j16 -O check TESTS='All Ability Slots lets non-representative Immunity cure existing poison'` | Pass | Confirms hidden-slot status-immunity abilities participate in switch-in / turn-0 status cleanup. |
| `rtk make -j16 -O check TESTS=test/champions_run_session.c` | Pass | 8 tests, including restore / retire / clear / checkpoint behavior. |
| `rtk make -j16 -O check TESTS='All Ability Slots'` | Pass | Focused all-slot suite passes under the 16.0 port, including defaulted move-end `Poison Touch` and off-field suppression regressions. |
| `rtk make -j16 -O check TESTS='Receiver'` | Pass | Confirms fainted ally ability-copy behavior still works after tightening `notOnField` suppression. |
| `rtk make -j16 -O check TESTS='Ability Shield on fainted ally'` | Pass | Confirms Ability Shield does not block Receiver / Power of Alchemy when the ally is already fainted. |
| `rtk make -j16 -O check` | Pass | Exits 0 with existing expected / known-failing markers and existing RWX linker warning. |
| `rtk make -j16 -O all` | Pass | Normal ROM build passes with existing RWX linker warning. |
| `rtk make -j16 -O debug` | Pass | Debug ROM build passes with existing RWX linker warning. |
| `rtk mdbook build docs` | Pass | Exits 0 with existing warnings: missing root `CHANGELOG.md` include, existing `CREDITS.md` `</img>` warning, and large search index. |
| mGBA Live | Pass | Final session `runtime-dev-16-final-20260531` booted `pokeemerald.gba`, captured `/tmp/runtime-dev-16-final-20260531.png`, and stopped cleanly. Follow-up `status --all` returned `[]`. |

Post-Wattson fix validation repeated `rtk make -j16 -O all`, `rtk make -j16 -O debug`,
and full `rtk make -j16 -O check`; all pass with the same existing RWX linker
warning / expected-marker profile.

Post-Meteorite compatibility validation repeated `rtk git diff --check`,
`rtk make -j16 -O all`, `rtk make -j16 -O debug`, full
`rtk make -j16 -O check`, and `rtk mdbook build docs`; all pass with the same
existing warning profile. mGBA Live session `runtime-dev-16-audit-20260602`
booted `pokeemerald.gba`, returned `true` through Lua, captured
`/tmp/runtime-dev-16-audit-20260602.png`, stopped cleanly, and final
`status --all` returned `[]`. The exact old-save Cozmo dialogue branch remains
a source-level migration audit unless a save fixture with old `0xE5` set is
prepared.

## 2026-06-02 Completeness Audit

The current branch was rechecked against the 15.3 completed-feature shelf and
the 16.0 upstream-first rule. No missing 15.3 runtime implementation was found
in this pass. The important 16.0 dependency choices are:

| 15.3 area | 16.0 evidence | 16.0 dependency / policy note |
|---|---|---|
| All Ability Slots | `include/config/battle.h`, `include/constants/config_changes.h`, `src/battle_*`, `test/battle/ability/all_ability_slots.c` | Default remains `FALSE`; runtime debug toggle stays compiled through the 16.0 config-change path. |
| Champions Run Session | `include/config/save.h`, `include/global.h`, `src/champions_run_session.c`, `test/champions_run_session.c` | Uses 16.0 party storage APIs and preserves the one-shot Continue warp cleanup fix. |
| Trainer Battle Selection / Team Viewer | `src/trainer_battle_selection.c`, `src/prebattle_team_viewer.c`, `src/battle_setup.c`, `src/battle_controller_player.c` | Single 1-mon selection is enabled; double remains 2-mon minimum unless the experimental 1v2 config is deliberately enabled. |
| Pokemon Vendor / sealed recruits | `src/pokemon_vendor.c`, `include/constants/pokemon_vendor.h`, `test/pokemon_vendor.c` | Uses 16.0 party / PC storage symbols and retains locked-origin handling. |
| Nonconsumable Held Items / Battle Item Restore | `include/config/item.h`, `src/item.c`, `src/battle_util.c`, `test/bag.c`, `test/battle_item_restore.c` | Consumable berry pocket remains physical inventory; held-effect ownership tokens stay enabled. |
| Unified Move Relearner / Summary editors | `src/move_relearner.c`, `src/pokemon_summary_screen.c`, `data/scripts/move_relearner.inc` | Summary remains the canonical entry point. Ability-slot Summary switching is gated by the all-slot runtime config. |
| Field Kit / No Random Encounters | `src/field_move.c`, `include/config/wild_encounter.h`, `src/field_control_avatar.c`, `src/wild_encounter_ow.c` | No Random was adapted to 16.0's upstream `WE_FLAG_NO_ENCOUNTER` config instead of reverting to the older `OW_FLAG_NO_ENCOUNTER` name. |
| Scout Selection | `src/scout_selection.c`, `include/constants/scout_selection.h`, `src/data/scout_selection_pools.h` | Runtime remains present; generated pool data remains integration-branch evidence, not master policy. |
| Battle BGM Selector | `src/battle_bgm.c`, `include/battle_bgm.h`, `src/debug.c`, `test/battle_bgm.c` | Debug `Sound...` selectors are present; imported BGM assets remain integration/runtime artifacts. |
| TM Shop Migration | `include/config/item.h`, `data/maps/*/scripts.inc`, `include/constants/flags.h` | 16.0 script structure wins. Cozmo now migrates the retired `0xE5` Meteorite-return bit to `FLAG_RETURNED_METEORITE_TO_COZMO`. |
| Map Asset Relinker | `tools/map_asset_relinker*`, `.github/workflows`, release packaging docs | Source/tooling is present; generated desktop binaries stay out of git and should be distributed by Actions / Releases. |

Open items in `open_tree/carryover_items.md` are future 16.0+ decisions, not
evidence of a missing 15.3 runtime port. That includes CI matrix cleanup,
future upstream resync, randomizer / EX lane work, optional map / fly sample
data, Bag Expansion, future feature candidates, battle-BGM asset policy, and
experimental 1v2 doubles.

## Known Non-Blockers

| Item | Handling |
|---|---|
| GitHub Actions matrix | Do not block the handoff waiting for long Actions. Emerald local validation is green; FRLG / workflow policy remains a 16.0 CI cleanup item. |
| Map Asset Relinker Windows package | Keep generated `.exe` out of git. Use the desktop workflow artifact or release upload path for distribution. |
| 1v2 double trainer battle | Not default. The config hook exists, but normal double selection still requires 2 eligible Pokemon. |
| 15.3 docs | Do not copy the whole 15.3 tree back onto 16.0 docs. Treat it as evidence and only migrate relevant status notes. |
