# 16.0 Runtime Port Status

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-31 |
| Upstream baseline | `master` `4faec7cb08` / `expansion/1.16.0-104-g4faec7cb08` |
| Runtime branch | `integration/runtime-dev-16-20260531` |
| Runtime PR | PR #69 / `[codex] Port runtime integration to 16.0 baseline` |
| Previous runtime shelf | PR #68 / `integration/runtime-dev-20260529` |
| Policy | Upstream 16.0 source wins unless a local runtime feature must be replayed intentionally. |

## Summary

`master` was synced to the upstream 16.0 line, then the 1.15.3 runtime work was replayed onto a fresh integration branch. The 15.3 docs remain the evidence shelf; this file records what changed while moving that implementation set onto 16.0.

The port keeps the normal branch rule:

- `master` remains upstream intake plus docs / Lua-only overlay.
- Runtime source, data, graphics, generated output, and non-Lua tools stay on an integration / feature branch.
- Map Asset Relinker desktop binaries are release / Actions artifacts, not tracked source files.

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
| TM Shop Migration | Ported with review fix | New Mauville / Wattson script flow was adjusted for 16.0 replay: Wattson now remains in Mauville City until the post-generator completion conversation runs, then that conversation moves him back to the Gym. |
| Map Asset Relinker | Ported as tooling | Rust core, shell wrapper, GUI, Tauri build scripts, and desktop packaging workflow remain present. Windows `.exe` distribution should come from Actions artifact / release packaging. |

## Review And Validation

| Check | Result | Notes |
|---|---|---|
| `rtk codex review --base master` | Fixed | First pass reported held-item catalog ownership included consumable berries. Second pass reported two All Ability Slots regressions: defaulted ability-effect callers skipped extra slots, and switched-out battlers could still expose abilities. Third pass reported that the New Mauville generator state moved Wattson back to the Gym before his completion conversation. All were fixed with focused coverage or script-flow validation. |
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

## Known Non-Blockers

| Item | Handling |
|---|---|
| GitHub Actions matrix | Do not block the handoff waiting for long Actions. Emerald local validation is green; FRLG / workflow policy remains a 16.0 CI cleanup item. |
| Map Asset Relinker Windows package | Keep generated `.exe` out of git. Use the desktop workflow artifact or release upload path for distribution. |
| 1v2 double trainer battle | Not default. The config hook exists, but normal double selection still requires 2 eligible Pokemon. |
| 15.3 docs | Do not copy the whole 15.3 tree back onto 16.0 docs. Treat it as evidence and only migrate relevant status notes. |
