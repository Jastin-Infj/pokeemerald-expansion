# Champions Challenge Implementation

## Runtime Slice: Run Session Restore MVP

| Field | Value |
|---|---|
| Branch | `feature/champions-run-session-runtime-20260524` |
| Status | Source implementation on feature branch; not for direct `master` merge |
| Primary files | `src/champions_run_session.c`, `include/champions_run_session.h`, `include/global.h`, `src/save.c`, `data/scripts/debug.inc`, `data/scripts/pc.inc`, `data/specials.inc` |
| Last updated | 2026-05-25 |

## Implemented Contract

- Added `struct ChampionsRunSession` in `SaveBlock1`.
- Enabled `FREE_MYSTERY_GIFT` and `FREE_MYSTERY_EVENT_BUFFERS` to budget the snapshot.
- `ChampionsRun_BeginEntry()` snapshots the normal party, normal bag, mail, money,
  coins, registered item, and run-start warp, then prepares the live run state
  according to `CHAMPIONS_RUN_ENTRY_PARTY_MODE`.
- Active-run saves keep the live run party / bag in the normal `SaveBlock1`
  party and bag slots. The normal state lives in `SaveBlock1.championsRun`.
- `ChampionsRun_ShouldUseTemporarySave()` maps normal save requests during an
  active run to the `SAVE_LINK` path, so Pokemon Storage sectors are not
  rewritten during MVP checkpoints.
- `ChampionsRun_SaveCheckpoint()` calls `SaveMapView()` before the `SAVE_LINK`
  write. This matches Battle Pyramid / Pike checkpoint behavior and prevents a
  reboot from restoring stale `mapView` metatiles into the current map.
- `ChampionsRun_RestoreNormalState()` restores the normal party / bag / mail /
  money / coins / registered item and marks the run inactive.
- Battle loss / draw / forfeit is intercepted before normal whiteout. The run
  restores the normal snapshot, warps to the run-start location, clears active
  state, and writes a normal save.
- Retire / loss restore clears `SaveBlock1.mapView` before saving the restored
  normal state. This prevents stale checkpoint metatiles from being applied to
  the run-start map on the next Continue.
- The debug `Champs: Retire` route reloads the field map after the success
  message, matching the same practical redraw effect as changing maps with Fly.
- The loss / draw / forfeit path restores directly from
  `SaveBlock1.championsRun` instead of copying the full session snapshot onto
  the battle callback stack. This avoids stack pressure from the party + bag +
  mail snapshot and fixes the Bad Egg / blue-screen failure seen after loss
  restore.
- The Champions loss map-load handoff resets `gMain.state` before entering
  `CB2_LoadMap`, so returning from battle starts a fresh map-load state machine.
- Normal Pokemon Storage access through PC scripts is blocked while
  `ChampionsRun_IsActive()` is true.
- Active Champions runs suppress normal battle EXP, hide field / battle bag
  access, and remove party-menu held-item change actions.
- Clear-completion flow is separate from ordinary trainer-win handling. A
  script can call `ChampionsRun_CompleteClearAndSave()` when the facility
  clear flag / clear condition is reached.
- On clear, the live run party is copied into Pokemon Storage before the normal
  party snapshot is restored. The deposited box / slot pairs are stored as
  small carryover metadata in `SaveBlock1.championsRun`.
- The next `ChampionsRun_BeginEntry()` is configurable:
  `CHAMPIONS_RUN_START_PARTY_EMPTY` starts fresh with 0 Pokemon,
  `CHAMPIONS_RUN_START_PARTY_LAST_CLEAR` copies the last deposited clear party
  from Pokemon Storage, and `CHAMPIONS_RUN_START_PARTY_CURRENT` keeps the
  current normal party as the initial run party while still clearing the run
  bag / money state. The current default is fresh / empty.
- Clear-party held item carryover is configurable with
  `CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE`: strip and discard, keep on deposited
  Pokemon, or strip and merge into the restored normal bag when space allows.
- Run-bag reward carryover is pocket-configurable in `include/config/save.h`.
  The current defaults carry regular Items and TM/HM rewards, but do not carry
  Poke Balls, Berries, or Key Items.
- Clear autosave currently uses `TrySavingData(SAVE_NORMAL)` so Pokemon Storage
  sectors are persisted with the deposited reward Pokemon. No dedicated
  autosave icon implementation was found in the current local branches / refs;
  visual autosave feedback remains a UI follow-up.

## Save Layout Notes

`ChampionsRunSession.normalBag` stores item quantities decrypted. This is
intentional: `MoveSaveBlocks_ResetHeap()` can change the active save encryption
key on load, and restoring raw encrypted item quantities would corrupt the
normal bag. Runtime restore re-encrypts snapshot quantities with the current key.

Current `sizeof(struct SaveBlock1)` for this branch is `15664`, below the
four-sector SaveBlock1 capacity of `15872` bytes.

## Debug Route

The generic debug script slots are used as the first manual route:

| Debug menu label | Behavior |
|---|---|
| `Champs: Start` | Start a Champions run, write entry report, clear live party / bag / money. |
| `Champs: Give Mon` | Give a Lv.50 Dragonite to the run party and checkpoint it. |
| `Champs: Checkpoint` | Write an explicit Champions checkpoint. |
| `Champs: Retire` | Retire the active run, return to the run-start point, restore normal state, and save. |
| `Champs: Lose Test` | Start a run, give Lv.1 Magikarp, checkpoint, and start a Steven battle for loss/restore validation. |
| `Champs: Clear` | Complete the current run, deposit the live party into PC storage, restore normal state, and autosave. |

## Validation Evidence

- `rtk make -j16 -O all` passes with the existing RWX linker warning.
- `rtk make -j16 -O debug` passes with the existing RWX linker warning.
- `rtk make -j16 -O check` exits 0; suite still includes existing
  `EXPECTED_FAIL` / crash-resume markers.
- `rtk mdbook build docs` exits 0 with existing warnings for missing root
  `CHANGELOG.md`, `CREDITS.md` `</img>`, and large search index.
- Focused checks:
  - `rtk make -j16 -O check TESTS=Champions` passes 8 tests.
  - `rtk make -j16 -O check TESTS=SaveBlock` passes 3 tests.
- mGBA Live:
  - MCP start failed once because Qt had no `DISPLAY`.
  - CLI start with `DISPLAY=:0` succeeded using the script-capable mGBA build.
  - `Champs: Start` displayed "Champions run started"; `Champs: Give Mon`
    displayed "Debug Dragonite was added and checkpointed"; `Champs: Retire`
    displayed "Champions run retired. Returned to the start point."
  - `Champs: Lose Test` started a Steven battle with Lv.1 Magikarp; after
    Magikarp fainted, the game returned to the run-start overworld position
    instead of normal whiteout, the Bag entry was visible again, and the run
    party was gone.
- Retire / start / loss regression check:
  - mGBA Live MCP default wrapper booted `pokeemerald.gba`, loaded the existing
    save, and used `Party... > Set Party` to seed a valid normal party.
  - `Scripts... > Champs: Retire` restored the normal party and returned to the
    run-start point; opening the party menu showed the restored `Buffie Lv100`.
  - `Scripts... > Champs: Start` then started a fresh run with the default
    empty start mode; opening the party menu showed the no-Pokemon party
    message.
  - `Scripts... > Champs: Lose Test` then started the Steven battle, Lv.1
    Magikarp fainted, and the game returned to the run-start location.
  - Opening the party menu after loss showed the restored normal party entry
    `Buffie Lv100`; no Bad Egg screen or blue-screen crash occurred. Screenshot:
    `/tmp/champions_loss_test_restore_after_fix.png`.
- Clear carryover check:
  - mGBA Live MCP booted the debug ROM, loaded the existing save, and ran
    `Scripts... > Champs: Start`, `Champs: Give Mon`, and `Champs: Clear`.
  - Clear restored the normal party state after depositing the run party. The
    default next-start behavior is now covered by the focused config-aware test:
    empty by default, last-clear copy only when `CHAMPIONS_RUN_ENTRY_PARTY_MODE`
    is set to `CHAMPIONS_RUN_START_PARTY_LAST_CLEAR`.
- Checkpoint reboot map-view regression check:
  - mGBA Live CLI booted the debug ROM, loaded the existing save, ran
    `Scripts... > Champs: Checkpoint`, stopped mGBA, started a new mGBA
    session, and continued from the saved file.
  - The field reloaded on the same map without visible metatile / map-chip
    corruption after the checkpoint save. Screenshot:
    `/tmp/champs_checkpoint_continue_field.png`.
  - The fix is `SaveMapView()` before the Champions `SAVE_LINK` checkpoint,
    matching Battle Pyramid / Pike.
- Retire redraw regression check:
  - `ChampionsRun_RetireAndSave()` is covered by a focused test that fills
    `SaveBlock1.mapView` with stale metatiles before retiring and expects the
    restored normal state to clear it.
  - Debug `Champs: Retire` now calls `ChampionsRun_ReloadMapAfterRestore` after
    the success message so the currently loaded field camera / map buffer is
    rebuilt immediately instead of waiting for a later map transition.
  - mGBA Live CLI booted the debug ROM, loaded the existing save, ran
    `Scripts... > Champs: Start`, `Scripts... > Champs: Give Mon`, and
    `Scripts... > Champs: Retire`; after dismissing the retire message, the
    field reloaded without a visible camera offset or map-chip corruption.
    Screenshot: `/tmp/champs_retire_redraw_after_fix.png`.
  - Cleanup note: the retire-redraw session stopped cleanly. An earlier
    `champs-start-retire-loss-fix` session still showed the known stale /
    zombie cleanup condition after repeated stop attempts (`[mgba-qt]
    <defunct>`), but no current validation depends on that session.

## Remaining Runtime Work

- Hook real reception / Scout Selection / battle-selection scripts into the
  helper surface instead of using debug scripts.
- Add outcome-specific reward flow for win / loss / retire.
- Connect the real facility clear flag / result script to
  `ChampionsRun_CompleteClearAndSave()`.
- Add a dedicated autosave icon / message layer if a visual autosave UX is
  desired; current clear persistence is functional full-save behavior only.
- Decide whether `CHAMPIONS_RUN_HELD_ITEM_CARRY_TO_BAG` should surface a message
  when the restored normal bag has no room and a held item cannot be carried.
- Add real room / streak / seed fields once progression logic exists.
- Add mGBA Live validation covering power-off resume from an active checkpoint.
- Decide whether a later run-only storage or safe-room PC checkpoint replaces
  the current MVP PC block.

## Final Handoff

Implementation branch:

- Keep the runtime changes on `feature/champions-run-session-runtime-20260524`.
- Do not merge this branch directly into `master` while the branch contains
  source / include / data / test runtime changes.
- Use the branch as the implementation PR for the Champions run-session MVP.

Docs-only master branch:

- Cherry-pick or re-apply only Markdown documentation updates from this feature
  onto a fresh branch from current `master`.
- Expected master-facing files are under `docs/features/champions_challenge/`
  plus `docs/SUMMARY.md` if the new implementation doc needs to be linked.
- Confirm `rtk git diff --name-only master..HEAD` contains only Markdown docs
  before opening or merging the master PR.

Next implementation slice:

- Connect real facility scripts to `ChampionsRun_BeginEntryReport()`,
  `ChampionsRun_SaveCheckpoint()`, and `ChampionsRun_CompleteClearAndSave()`.
- Reuse Scout Selection / Pokemon Vendor only as roster producers; they should
  not own save / restore semantics.
- Add active-checkpoint power-off / Continue validation once a non-debug room
  path exists.
