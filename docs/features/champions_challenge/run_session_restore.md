# Champions Run Session Restore

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-29 |
| Baseline | `master` `4e48ff993f`; implementation branch `feature/champions-run-session-runtime-20260524` |
| Code status | MVP runtime adopted into `integration/runtime-dev-20260529`; not on `master` |
| Provenance | Local project feature docs |

## Goal

Pokemon Champions style run facility needs a Battle Pyramid / Battle Frontier
style save contract, but with roguelike lifecycle rules:

- player can enter with 0 Pokemon in the live challenge party;
- normal party, normal bag, and optionally PC state are restored on retire /
  loss / end;
- a temporary report or autosave can resume the active run after power-off;
- battle loss must not resume from the latest checkpoint; it finalizes the run
  and restores the run-start state;
- run counters / streaks / seed / offered pools are more flexible than the
  vanilla Frontier challenge counter;
- normal saved data must not be overwritten by the challenge party or empty bag.

This doc is the focused dependency and feasibility investigation for that
session / checkpoint / restore layer. The first source slice now implements the
MVP contract described below; handoff details live in
`docs/features/champions_challenge/implementation.md`.

## Existing Emerald Behavior

| Area | Files / symbols | Behavior to reuse or avoid |
|---|---|---|
| Full save | `src/save.c` `TrySavingData(SAVE_NORMAL)` | Writes SaveBlock2, SaveBlock1, and Pokemon Storage sectors. This persists party, bag, map state, and PC boxes. |
| Frontier save | `src/save.c` `TrySavingData(SAVE_LINK)` | Writes SaveBlock2 and SaveBlock1 only. It intentionally skips Pokemon Storage sectors. |
| Frontier wrapper | `src/frontier_util.c` `SaveGameFrontier` | Copies live challenge party to heap, loads saved normal party, sets continue warp, calls `TrySavingData(SAVE_LINK)`, clears continue warp flag, then restores live challenge party. |
| Battle Pyramid pause | `src/battle_pyramid.c` `SavePyramidChallenge`, `PausePyramidChallenge` | Saves `challengeStatus`, marks `challengePaused`, saves map view, and uses the Frontier save path. Start-menu save first restores Pyramid party data back into saved party, then normal save path runs. |
| Battle Pyramid resume | `data/maps/BattleFrontier_BattlePyramidLobby/scripts.inc`, `BattlePyramidFloor/scripts.inc`, `BattlePyramidTop/scripts.inc` | Lobby / floor scripts route `CHALLENGE_STATUS_PAUSED` and `CHALLENGE_STATUS_SAVING` into resume or forced lobby-return flows. |
| Start menu | `src/start_menu.c` `BuildBattlePyramidStartMenu`, `SaveDoSaveCallback` | Pyramid start menu adds Rest and Retire. Normal save calls `PausePyramidChallenge()` before writing. After save it soft-resets in Pyramid. |
| Party save helper | `src/load_save.c` `SavePlayerParty`, `LoadPlayerParty` | Copies live `gPlayerParty` into `SaveBlock1.playerParty[]` and back. This is not a separate backup buffer. |
| Bag save helper | `src/load_save.c` `SavePlayerBag`, `LoadPlayerBag` | Copies `gLoadedSaveData.bag` / mail to or from `SaveBlock1.bag` / mail. It is useful as a pattern, not a complete snapshot solution. |
| PC storage | `include/pokemon_storage_system.h`, `src/save.c` sectors 5-13 | Pokemon Storage is a separate large save block. A second full PC snapshot does not fit in the existing normal save area. |

## Key Findings

### `SavePlayerParty` Is Not Enough

`SavePlayerParty()` writes the current live party into the normal saved party
slot. If Champions clears the party to 0 and then a normal save runs, the
normal party is destroyed in the save file.

Battle Pyramid can use this pattern because it restores / rewrites the selected
Frontier party in a narrow facility flow. Champions runs are longer and need
autosave / suspend behavior, so the normal party snapshot must live in a
dedicated Champions state field, not in `SaveBlock1.playerParty[]`.

Implemented MVP state:

```c
struct ChampionsRunSession
{
    u32 signature;
    u32 version;
    u32 runSeed;
    u32 runIndex;
    u32 streak;
    u32 normalMoney;
    u16 normalCoins;
    u16 normalRegisteredItem;
    u8 active;
    u8 status;
    u8 checkpointKind;
    u8 outcome;
    u8 normalPartyCount;
    u8 requiredPartyCount;
    u8 lastClearPartyCount;
    u8 lastClearBoxIds[PARTY_SIZE];
    u8 lastClearBoxPositions[PARTY_SIZE];
    u8 padding;
    struct WarpData startLocation;
    struct Pokemon normalParty[PARTY_SIZE];
    struct Bag normalBag;
    struct Mail normalMail[MAIL_COUNT];
};
```

The MVP stores only the normal snapshot. During an active run, the existing
`SaveBlock1.playerParty[]` and `SaveBlock1.bag` slots represent the live run
party / bag. This keeps temporary reports resumable while still allowing
retire/loss/win to restore the normal snapshot.

The `startLocation` field is intentionally small. It stores the map and
coordinates where the run was accepted. It is not a full map-object snapshot;
active checkpoints keep their live map state through the normal SaveBlock1 map
fields, while loss uses `startLocation` to return to the entry point.

### SaveBlock Gate

This feature must not move from docs to runtime until the save layout is
explicitly budgeted.

Source implementation gate result:

| Gate | Required check |
|---|---|
| Size measurement | `sizeof(struct SaveBlock1)` is `15664` on this branch, below the four-sector `15872` byte budget. |
| Owner block | Dedicated `SaveBlock1.championsRun` field. SaveBlock3 is not used. |
| FREE toggle policy | `FREE_MYSTERY_GIFT` and `FREE_MYSTERY_EVENT_BUFFERS` are enabled on the implementation branch. |
| PC storage | Full PC snapshot is not reserved. Normal Pokemon Storage is blocked during active runs. |
| UI separation | Party / Status UI overhaul remains a separate feature. |
| Migration | `test/save.c` expected SaveBlock1 size updated; focused runtime tests added in `test/champions_run_session.c`. |

### `SAVE_LINK` Is Useful But Not Sufficient

`SAVE_LINK` is the important Frontier trick: it skips PC storage. That is useful
for a run mode because mid-run checkpoint saves can avoid committing normal PC
box changes.

However, Champions needs additional run data that Frontier does not have:

- run party and generated/scouted roster;
- challenge bag / item pool;
- current stage / room / battle number;
- run seed and generator state;
- resume warp / room state;
- pending reward or retire/loss outcome.

Therefore the helper is not a direct `SaveGameFrontier()` reuse. The current
source slice adds `src/champions_run_session.c`, which:

1. stores the normal party / bag / mail / money snapshot in a dedicated struct;
2. keeps the active run party / bag as the live saved party / bag;
3. prepares the next run from a config mode: empty party, last clear-party
   carryover from PC slots, or current normal party;
4. maps active-run normal save requests to the Frontier-style partial save path;
5. restores the normal snapshot before final normal save on retire/loss/win;
6. intercepts battle defeat before whiteout, warps to `startLocation`, and
   writes a normal save with active state cleared.

### PC Box Policy Is The Hardest Requirement

Restoring PC boxes to the state at challenge entry is expensive because
`struct PokemonStorage` spans save sectors 5-13. There is no spare second copy
of the whole PC storage in the normal save layout.

Practical policy options:

| Policy | Behavior | Feasibility |
|---|---|---|
| MVP: no normal PC access during active run | Entry save captures normal PC. Mid-run saves use partial save and do not touch PC sectors. Retire/loss restores normal party / bag and PC remains as it was at entry. | Best first implementation. |
| Run-only PC / stash | Challenge has a small dedicated run storage for recruited Pokemon, not the normal PC boxes. Retire/loss deletes it or converts selected rewards. | Good later phase if 6 slots are not enough. |
| Safe-room PC checkpoint | Accessing PC in a safe room commits a new checkpoint. Retire restores to the last safe-room checkpoint, not necessarily original entry. | Possible, but semantics must be explicit. |
| Full PC rollback journal | Track every normal PC mutation and undo it on retire/loss. | High complexity; not recommended for MVP. |
| Second full PC snapshot | Store a full copy of `struct PokemonStorage`. | Not feasible without expanded save storage / external sectors. |

Recommendation: MVP blocks normal PC access during active Champions runs. If the
desired "Pit-like" behavior needs PC access, implement a small run-only stash
or safe-room checkpoint later. Do not promise arbitrary normal PC rollback in
the first runtime slice.

## Recommended Runtime Contract

### Session States

| State | Meaning |
|---|---|
| `NONE` | Not in a Champions run. |
| `ENTRY_SAVED` | Normal party / bag snapshot is stored and initial report succeeded. |
| `PREPARING` | Live party can be 0-6 challenge Pokemon. Scout / edit / recruit UI is active. |
| `BATTLING` | Battle or room progression is active. |
| `PAUSED` | Temporary report exists; boot should resume the run. |
| `WON` | Run ended successfully; restore normal state and pay rewards. |
| `LOST` | Run failed; delete run party / run bag, restore normal state. |
| `RETIRED` | Player manually quit; delete run party / run bag, restore normal state. |
| `RECOVERING` | Power-cut repair state; restore normal state or last checkpoint before clearing active. |

### Save Points

| Save point | Required behavior |
|---|---|
| Entry acceptance | Snapshot normal party / bag, write an entry report, then clear live party and normal bag view for the run. |
| After scout / roster change | Autosave run party and run bag, but keep normal party / bag save-facing data intact. |
| After battle result | Autosave run status, streak, rewards, and next room seed before handing control back to field. |
| Rest / suspend | Mark `PAUSED`, write checkpoint, and boot back into the run at the resume warp. |
| Retire / loss | Restore normal party / bag, clear run party / bag unless a reward policy says otherwise, then save normal state. |
| Win / clear | Deposit the live run party into Pokemon Storage when configured, process held items according to `CHAMPIONS_RUN_CLEAR_HELD_ITEM_MODE`, merge configured run-bag reward pockets into the normal bag snapshot, restore normal party / bag, clear run state, then write a full normal save so Pokemon Storage persists. |

### Power-Off Semantics

The cleanest player-facing rule:

- If the game was saved at a temporary report or autosave checkpoint, power-off
  resumes from that checkpoint.
- If power is lost before the next checkpoint, the run resumes from the previous
  checkpoint, not from every volatile in-room action.
- Battle loss / draw / forfeit does not use the checkpoint as a retry point.
  It finalizes immediately by restoring the normal snapshot, warping to the
  run-start location, and clearing active run state.
- Retire follows the snapshot-restore/finalize rule with no carryover. Win /
  clear uses the clear-specific carryover path: run Pokemon can be copied to PC
  storage first, configured run-bag pockets can be merged into the normal bag,
  and the full save persists the PC reward state.
- The next run defaults to a fresh 0-Pokemon start. A config switch can instead
  seed the run from the last clear-party PC slots or from the current normal
  party.

This gives the "can suspend and come back" behavior without needing to persist
every frame of the room.

## Implementation Shape For The Runtime Branch

The first source slice is separate from master and does not reuse Frontier
globals directly.

Likely modules / touch points:

| Area | Files / hooks | Notes |
|---|---|---|
| Core session | `src/champions_run_session.c`, `include/champions_run_session.h` | Owns state machine, entry snapshot, checkpoint save, restore, and clear helpers. |
| Save layout | `include/global.h`, `test/save.c` | Dedicated `SaveBlock1.championsRun` field; SaveBlock1 size expectation is updated to `15664`. |
| Save helper | `src/save.c`, `src/fieldmap.c` | Active-run normal saves are routed to `SAVE_LINK`; explicit Champions checkpoints call `SaveMapView()` before the partial save, and existing party / bag save slots represent the run checkpoint. |
| Party lifecycle | `src/pokemon.c`, `src/script_pokemon_util.c`, Scout Selection integration | Clear live party to 0, fill run party, restore normal party on exit. |
| Bag lifecycle | `src/item.c`, `src/bag.c`, `src/load_save.c` | Normal bag snapshot + run bag. Avoid direct scattered `memcpy`. |
| PC policy | `src/pokemon_storage_system.c` | MVP should block normal PC entry while active, or route to run-only stash. |
| Battle aftercare | `src/battle_setup.c`, `docs/features/trainer_battle_aftercare/` | Loss/retire/win outcome must be owned by Champions rule, not normal whiteout. |
| EXP / Bag / held-item restrictions | `src/battle_script_commands.c`, `src/battle_util.c`, `src/start_menu.c`, `src/party_menu.c` | Active Champions runs suppress normal EXP, block field/battle bag use, and remove party held-item change actions. |
| Item restore | `docs/features/battle_item_restore_policy/`, `docs/features/nonconsumable_held_items/` | Decide whether battle-consumed held items restore before run party deletion. |
| Progression | partygen / Scout Selection / battle selection | Stores run seed, offered candidates, selected Pokemon, next opponent, and streak. |

Suggested future helper surface:

```c
bool32 ChampionsRun_IsActive(void);
bool32 ChampionsRun_CanUseNormalPc(void);
bool32 ChampionsRun_BeginEntryReport(void);
bool32 ChampionsRun_SaveCheckpoint(u8 checkpointKind);
bool32 ChampionsRun_EndByBattleOutcome(u8 battleOutcome);
u8 ChampionsRun_RetireAndSave(void);
bool32 ChampionsRun_ShouldBlockBagUse(void);
bool32 ChampionsRun_ShouldBlockHeldItemChanges(void);
bool32 ChampionsRun_ShouldSuppressExp(void);
void ChampionsRun_ClearLivePartyAndBag(void);
void ChampionsRun_RestoreNormalState(u8 outcome);
void ChampionsRun_HandleBootRecovery(void);
```

## Implemented MVP Contract

The runtime branch currently uses the conservative contract:

1. Entry requires Yes / report.
2. Save normal party + normal bag in dedicated Champions state.
3. Clear live party to 0 and use an empty run bag / money state.
4. Write an entry checkpoint through `SAVE_LINK` so PC storage is not rewritten.
5. Disable normal PC while `ChampionsRun_IsActive()`.
6. Disable normal EXP, field/battle bag use, and held-item change actions while
   active.
7. Save run checkpoints with a Champions-specific helper.
8. On battle loss, restore normal party / bag, warp to the run-start location,
   clear active state, then save.
9. On retire, restore normal party / bag, warp to the run-start location, clear
   active state, then save.
10. On win / clear, apply configured Pokemon / item carryover, restore normal
   party / bag, clear active state, then save.

This satisfies the user-visible goal without solving arbitrary PC rollback in
the first pass.

## Dependency Handoff

The feature branch has these integration dependencies:

| Dependency | Current handling |
|---|---|
| Save layout | `SaveBlock1.championsRun` is gated by `SAVE_CHAMPIONS_RUN_SESSION`; `FREE_MYSTERY_EVENT_BUFFERS` and `FREE_MYSTERY_GIFT` must stay enabled while the snapshot is stored there. |
| Partial save / map view | Active reports use `SAVE_LINK`; Champions checkpoints call `SaveMapView()` first so Continue does not reload stale map metatiles. |
| Pokemon Storage | Normal PC access is blocked while active; clear rewards use full save after depositing run party into storage. Full PC rollback is intentionally not implemented. |
| Battle aftercare | `battle_setup.c` intercepts loss / draw / forfeit before whiteout and hands control to the Champions restore callback. |
| EXP and Bag rules | `battle_script_commands.c`, `battle_util.c`, `start_menu.c`, and `party_menu.c` ask Champions helpers instead of duplicating challenge-specific conditions. |
| Scout Selection / Pokemon Vendor | Not wired directly yet. They should add / edit run-party Pokemon only after `ChampionsRun_IsActive()` and checkpoint through `ChampionsRun_SaveCheckpoint()` when the roster changes. |
| Partygen / battle selection | Remains a separate feature. It should consume the active run state and progression fields later rather than owning save/restore itself. |
| Autosave UX | Clear autosave is functional through `TrySavingData(SAVE_NORMAL)`, but no visual autosave icon was found or connected. |
| Party / Summary UI overhaul | Separate UI feature. It should not add saved state to the Champions snapshot unless a persistent option is required. |

Master handoff should remain docs-only. The source branch is evidence for the
runtime slice, but `master` should receive only this dependency record and
handoff notes until an implementation integration branch is explicitly selected.

## Open Questions

- Should power-off resume from the last checkpoint by default, or should a
  stricter rule forfeit the run if power is cut outside a report point?
- Should safe-room PC access be allowed as "commit a new checkpoint" behavior,
  or should normal PC stay disabled for all active runs?
- Do recruited run Pokemon ever become rewards, or are all run Pokemon deleted
  unless explicitly converted by a reward screen?
- Should autosave happen after every battle, every room transition, or only at
  safe rooms?
- Which real facility scripts should call the debug-proven begin / checkpoint /
  restore helpers first?

## Validation Targets

Future runtime branch must prove:

- entry with 0 live party after report;
- normal party / bag restore after retire;
- normal party / bag restore after loss;
- power-off after temporary report resumes the run;
- power-off before next checkpoint resumes the previous checkpoint or forfeits
  according to the chosen policy;
- active-run `SaveBlock1.playerParty[]` and `SaveBlock1.bag` reload as the run
  checkpoint;
- normal party / bag snapshot restores correctly from `SaveBlock1.championsRun`;
- normal PC cannot be mutated during MVP run, or mutations follow the selected
  safe-room checkpoint policy;
- mGBA Live boot/resume evidence covers at least one active checkpoint and one
  final restore.
