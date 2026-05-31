# Champions Challenge Test Plan

## Manual Tests

| Test | Setup | Expected |
|---|---|---|
| Start challenge | Normal party 6, bag has several items | Reception saves state, challenge party becomes 0, normal bag is unavailable or replaced by empty challenge bag. |
| Cancel before start | Decline at reception | Party / bag unchanged. |
| Create 5 mons | Add only 5 challenge Pokemon | Battle start is blocked. |
| Create 6 mons | Add 6 valid non-egg Pokemon | Battle start is allowed. |
| Egg rejected | Include egg in challenge party | Battle start / eligibility check rejects it. |
| Frontier ban optional | Set rule to `FRONTIER_BAN`, include banned species | Banned species rejected only in that rule. |
| Egg-only default | Set default rule, include legendary / Frontier banned non-egg | Allowed if not egg. |
| Lv.50 scaling | Use Lv.1, Lv.49, Lv.50 challenge mons | Battle display / stats follow effective Lv.50 rule, actual level remains unchanged after battle. |
| No EXP | Win a battle against EXP-yielding opponent | EXP, level, Exp Share recipients do not change. |
| Win aftercare | Win first battle | Streak increments; rule-selected HP / PP / item restoration occurs; next battle menu appears. |
| Clear carryover | Complete / clear a run with live run Pokemon | The live run party is deposited into Pokemon Storage before normal state restore, held items follow `CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE`, configured bag rewards are merged into the normal bag, and a full autosave persists PC storage. |
| Next start mode | Start a new run after a clear carryover | Default `CHAMPIONS_RUN_START_PARTY_EMPTY` starts with 0 Pokemon; `CHAMPIONS_RUN_START_PARTY_LAST_CLEAR` copies the last deposited clear-party Pokemon from PC box slots; `CHAMPIONS_RUN_START_PARTY_CURRENT` keeps the current normal party as the run party. |
| Loss aftercare | Lose a trainer or wild battle during an active run | Temporary checkpoint is not used as a retry point; normal party / bag is restored, active state clears, and the player returns to the run-start location. |
| Power cut recovery | Save after challenge start, reload | Game restores or resumes according to challenge status, never strands player with 0 normal party. |
| Temporary report resume | Use an active-run report / suspend point, close and reload | Continue resumes at the Champions checkpoint or returns through the documented recovery warp; run party / run bag match the checkpoint. |
| Pre-checkpoint power cut | Mutate run party / bag after the last checkpoint, then reload from the saved file | Game resumes from the previous checkpoint or forfeits according to the selected policy; partial volatile changes do not overwrite normal party / bag. |
| Bag restore | Use / gain challenge bag items, then lose | Normal bag is exactly pre-run state unless reward policy explicitly says otherwise. |
| PC disabled | Try to open PC during challenge | PC is unavailable or only challenge-approved mode opens. |
| PC checkpoint policy | If safe-room PC is enabled later, access PC and retire | Restore target is the last explicit safe-room checkpoint, not an implicit arbitrary box rollback. |
| No leakage | Leave challenge, start normal trainer battle | Normal battle uses normal party, normal bag, normal EXP behavior. |

## Automated Test Candidates

| Test | Target |
|---|---|
| Challenge state init/clear | New challenge state helper |
| Party snapshot/restore | Dedicated party helper around `gPlayerParty` |
| Bag snapshot/restore | Dedicated bag helper around `gSaveBlock1Ptr->bag` and `gLoadedSaveData.bag` |
| Run session state machine | Dedicated Champions run helper covering entry, preparing, battling, paused, won, lost, retired, recovering |
| Checkpoint save guard | Champions save helper never writes live challenge party into `SaveBlock1.playerParty[]` |
| PC access guard | `ChampionsRun_CanUseNormalPc` or equivalent blocks normal storage during MVP active runs |
| Eligibility egg-only | New `Challenge_IsMonEligible` helper |
| Eligibility frontier-ban | `isFrontierBanned` rule path |
| Required party count | `Challenge_CanStartBattle` |
| Disable EXP | EXP command / battle outcome hook |
| Loss cleanup | aftercare helper clears challenge party and restores normal state |
| Run restrictions | Champions helper blocks Bag, held-item changes, and normal EXP only while active |
| Partygen fixed party output | Existing fixed trainer output does not gain `Party Size` or pool fields |
| Partygen pool output | Pool trainer output uses `Party Size` only when pool behavior is intended |
| Partygen header preserve | Existing `Name` / `Class` / `Pic` / `Music` / `Items` / `Back Pic` stay unchanged unless explicitly overridden |
| Partygen field validation | Ball uses `BALL_*` / Pokeball names, and Tera is not emitted together with Dynamax / Gigantamax |

## Current Automated Coverage

Implemented on `feature/champions-run-session-runtime-20260524` and adopted
into `integration/runtime-dev-20260529`:

| Test | Evidence |
|---|---|
| Entry snapshot / clear | `test/champions_run_session.c` confirms `ChampionsRun_BeginEntry()` stores the normal state and clears live party / bag / money. |
| Restore normal state | `test/champions_run_session.c` confirms party species, bag item quantity, money, coins, and registered item restore. |
| Bag encryption key change | `test/champions_run_session.c` confirms decrypted normal bag snapshots restore correctly after `encryptionKey` changes. |
| Temporary report save routing | `test/champions_run_session.c` confirms active runs map `SAVE_NORMAL` requests to temporary-report behavior. |
| Temporary report map state | Source audit confirms `ChampionsRun_SaveCheckpoint()` calls `SaveMapView()` before `SAVE_LINK`, matching Battle Pyramid / Pike checkpoint saves and avoiding stale saved-map-view restore on reboot. |
| Retire saved-map-view cleanup | `test/champions_run_session.c` confirms `ChampionsRun_RetireAndSave()` clears stale `SaveBlock1.mapView` while restoring normal state. |
| Retire redraw route | Debug `Champs: Retire` calls `ChampionsRun_ReloadMapAfterRestore` after the success message so field camera / map buffers are rebuilt immediately. |
| PC access guard | `test/champions_run_session.c` confirms `ChampionsRun_CanUseNormalPc` blocks access while active. |
| Active-run restrictions | `test/champions_run_session.c` confirms Bag, held-item changes, and normal EXP suppression toggle only while active. |
| Loss restore path | `test/champions_run_session.c` confirms `ChampionsRun_EndByBattleOutcome(B_OUTCOME_LOST)` restores the normal party / bag / money and clears active state. |
| Loss restore Continue warp | `test/champions_run_session.c` confirms loss restore returns to the saved run-start location, writes that restored warp as the one-shot Continue target for the restore save, then clears the live RAM flag so later normal saves do not inherit a stale run-start warp. |
| Retire restore path | `test/champions_run_session.c` confirms `ChampionsRun_RetireAndSave()` restores the normal state, clears active state, and saves through the normal path. |
| Clear carryover path | `test/champions_run_session.c` confirms `ChampionsRun_CompleteClearAndSave()` deposits the live run party into storage, applies configured held-item carryover, restores normal state, and follows the configured next-start party mode. |
| SaveBlock budget | `test/save.c` expects `sizeof(struct SaveBlock1) == 15664`. |

Latest local evidence:

- `rtk make -j16 -O all` passes with the existing RWX linker warning.
- `rtk make -j16 -O debug` passes with the existing RWX linker warning.
- `rtk make -j16 -O check` exits 0 with existing `EXPECTED_FAIL` / crash-resume markers.
- `rtk make -j16 -O check TESTS=Champions` passes 8 tests.
- `rtk make -j16 -O check TESTS=SaveBlock` passes 3 tests after updating the SaveBlock1 expectation.
- mGBA Live CLI with `DISPLAY=:0` confirmed boot, `Champs: Start`,
  `Champs: Give Mon`, and `Champs: Retire`.
- mGBA Live CLI with `DISPLAY=:0` confirmed `Champs: Lose Test`: Steven battle
  started, Lv.1 Magikarp fainted, control returned to the run-start overworld
  location, Bag was visible again in the start menu, and the challenge party was
  no longer present.
- mGBA Live follow-up confirmed the reported loss-restore regression fix:
  after seeding a valid normal party, running `Scripts... > Champs: Lose Test`,
  and losing to Steven, the party menu showed restored `Buffie Lv100` instead
  of Bad Egg data. No blue-screen crash occurred. Screenshot:
  `/tmp/champions_loss_test_restore_after_fix.png`.
- 2026-05-31 focused loss-restore Continue warp check:
  `rtk make -j16 -O check TESTS=Champions` passes after extending the loss
  restore test to simulate a run-map location before loss and assert that the
  restored start location is written to `continueGameWarp` while the live
  one-shot flag is cleared after the restore save. This targets the manual
  issue where the in-session loss restore worked, but closing the game without
  another report could resume on an unexpected map, while avoiding stale warp
  inheritance on later normal saves.
- `codex review --uncommitted` was run twice. The first pass found stale
  in-memory one-shot Continue warp state after restore save; the source and
  tests were updated, and the second pass found no discrete issues.
- mGBA Live smoke after the final debug rebuild booted `pokeemerald.gba`,
  captured `/tmp/runtime-followup-20260531b-boot.png`, stopped cleanly, and
  `status --all` returned `[]`. The exact loss -> close without another report
  path was not replayed in mGBA this turn; use the focused unit regression plus
  previous manual loss-restore evidence as the current oracle.
- mGBA Live follow-up confirmed the default fresh-start mode after restore:
  `Scripts... > Champs: Start` opened the party menu with the no-Pokemon party
  message.
- mGBA Live follow-up confirmed `Scripts... > Champs: Clear` restores the
  normal party after depositing the run party. The exact next-start carryover
  mode is covered by the focused config-aware unit test because the branch
  default is now empty start rather than last-clear start.
- mGBA Live checkpoint reboot regression confirmed `Scripts... > Champs:
  Checkpoint`, mGBA stop/start, and Continue reload the same field without
  visible metatile / map-chip corruption after adding `SaveMapView()` before
  the Champions `SAVE_LINK` checkpoint. Screenshot:
  `/tmp/champs_checkpoint_continue_field.png`.
- mGBA Live retire redraw regression confirmed `Scripts... > Champs: Start`,
  `Scripts... > Champs: Give Mon`, and `Scripts... > Champs: Retire`; after
  closing the retire message, the field reloaded without a visible camera
  offset or map-chip corruption. Screenshot:
  `/tmp/champs_retire_redraw_after_fix.png`.
- mGBA Live cleanup caveat: the retire-redraw session stopped cleanly. An
  earlier `champs-start-retire-loss-fix` session returned `alive_after: true`
  after repeated stop attempts, and `pgrep` showed `[mgba-qt] <defunct>`.
  This is recorded as the known stale / zombie cleanup state rather than a
  feature failure.

Integration evidence on `integration/runtime-dev-20260529`:

- `rtk git diff --check`
- `rtk git diff --cached --check`
- `rtk make -j16 -O check TESTS=Champions`
- `rtk make -j16 -O check TESTS=SaveBlock`
- `rtk make -j16 -O check TESTS='All Ability Slots'`
- `rtk make -j16 -O check TESTS='Battle item restore'`
- `rtk make -j16 -O check TESTS=test/pokemon_vendor.c`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`
- mGBA Live `integration-champions-run-smoke` confirmed
  `Scripts...` contains the named Champions entries after preserving Vendor /
  Scout script slots, and confirmed `Champs: Start`, `Champs: Give Mon`, and
  `Champs: Retire` return to a stable field map. Screenshots:
  `/tmp/integration-champions-run-scripts-menu.png`,
  `/tmp/integration-champions-run-start.png`,
  `/tmp/integration-champions-run-give-mon.png`, and
  `/tmp/integration-champions-run-retire-map.png`.
- Cleanup was clean: `mgba_live_stop` reported `alive_after:false`, and
  `mgba-live-cli status --all` returned `[]`.

## Regression Tests

| Existing area | Expected |
|---|---|
| Battle Frontier lobby | Existing ineligible messages and entry rules unchanged. |
| Battle Pyramid | Pyramid bag and held item storage still work. |
| Frontier partial save | Existing `SaveGameFrontier` / Pyramid pause behavior still saves and resumes as before. |
| Cable Club / Union Room | `SavePlayerBag` / `LoadPlayerBag` behavior unchanged. |
| Trainer battle aftercare | Normal trainer battles still whiteout / return as before. |
| Bag menu | Normal bag contents and sort order survive challenge start/end. |
| Party menu | Normal choose-half validation unchanged outside Champions mode. |

## Test Data Needed

- A banned species with `isFrontierBanned = TRUE`.
- A non-banned ordinary species.
- An egg.
- A low-level Pokemon below 50.
- A held single-use item and a Berry for restore policy checks.
- A bag with at least one item in each pocket.

## Open Questions

- Automated battle tests can assert EXP unchanged, but Lv.50 effective stats may need a battle mon setup helper test.
- Active checkpoint power-off / Continue is only manually covered for map-view
  stability after checkpoint; add a full run-party / run-bag resume test once a
  real facility room path exists.
- If a later challenge bag becomes runtime-only instead of using the normal bag
  slot during active runs, reload behavior needs a save/load integration test
  rather than a simple unit test.
