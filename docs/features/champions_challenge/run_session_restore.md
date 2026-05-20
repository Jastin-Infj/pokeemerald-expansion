# Champions Run Session Restore

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `4125f7c4d5`; docs-only investigation branch `docs/champions-run-session-restore-20260520` |
| Code status | Docs-only investigation; runtime not implemented |
| Provenance | Local project feature docs |

## Goal

Pokemon Champions style run facility needs a Battle Pyramid / Battle Frontier
style save contract, but with roguelike lifecycle rules:

- player can enter with 0 Pokemon in the live challenge party;
- normal party, normal bag, and optionally PC state are restored on retire /
  loss / end;
- a temporary report or autosave can resume the active run after power-off;
- run counters / streaks / seed / offered pools are more flexible than the
  vanilla Frontier challenge counter;
- normal saved data must not be overwritten by the challenge party or empty bag.

This doc is the focused dependency and feasibility investigation for that
session / checkpoint / restore layer.

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

Required future state:

```c
struct ChampionsRunSession
{
    bool8 active;
    u8 status;          // none / preparing / battling / paused / won / lost
    u8 checkpointKind;  // entry / room / battle / safe-room / retire
    u8 runPartyCount;
    u8 requiredPartyCount;
    u32 runSeed;
    u32 runIndex;
    u32 streak;
    struct Pokemon normalParty[PARTY_SIZE];
    u8 normalPartyCount;
    struct Bag normalBag;
    struct Pokemon runParty[PARTY_SIZE];
    struct Bag runBag;
};
```

The exact location is still a save-layout decision. `docs/flows/save_data_flow_v15.md`
already notes that party + bag snapshot is roughly 1.4 KB plus metadata, which
does not fit in the current small SaveBlock1 spare area without freeing or
migrating space.

### SaveBlock Gate

This feature must not move from docs to runtime until the save layout is
explicitly budgeted.

Minimum gate before source implementation:

| Gate | Required check |
|---|---|
| Size measurement | Measure `sizeof(struct Pokemon)`, `sizeof(struct Bag)`, and the proposed `ChampionsRunSession` with the current config, not an old estimate. |
| Owner block | Prefer a dedicated SaveBlock1 field for party + bag snapshot. Do not put the large snapshot in SaveBlock3 while DexNav search levels can consume most of it. |
| FREE toggle policy | If using `FREE_MYSTERY_GIFT` / `FREE_MYSTERY_EVENT_BUFFERS`, record the migration and confirm this fork does not need Mystery Gift / Mystery Event. |
| PC storage | Do not reserve space for a full PC snapshot in normal save. Use PC disabled / run-only stash / explicit safe-room checkpoint instead. |
| UI separation | Party / Status UI overhaul, including a future `2 x 3` party grid or BW Summary screen, must not share this SaveBlock allocation unless it adds a saved UI option. |
| Migration | Add a focused save migration / compatibility test before merging any source branch with new fields. |

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

Therefore the recommended helper is not a direct `SaveGameFrontier()` reuse. It
should be a new `SaveGameChampionsRun()` or equivalent that:

1. keeps normal save-facing party / bag coherent;
2. persists Champions run state in a dedicated struct;
3. writes either a Frontier-style partial save or a full save based on policy;
4. never lets live challenge party / empty bag overwrite the normal save slots.

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
| Win | Restore normal party / bag, pay rewards, clear run state, then save normal state. |

### Power-Off Semantics

The cleanest player-facing rule:

- If the game was saved at a temporary report or autosave checkpoint, power-off
  resumes from that checkpoint.
- If power is lost before the next checkpoint, the run resumes from the previous
  checkpoint, not from every volatile in-room action.
- Retire/loss/win always finalizes by restoring the normal snapshot and clearing
  active run state.

This gives the "can suspend and come back" behavior without needing to persist
every frame of the room.

## Implementation Shape For A Future Runtime Branch

Future source slice should be separate from master and should not reuse
Frontier globals directly.

Likely modules / touch points:

| Area | Future files / hooks | Notes |
|---|---|---|
| Core session | `src/champions_run_session.c`, `include/champions_run_session.h` | Owns state machine, entry snapshot, checkpoint save, restore, and clear helpers. |
| Save layout | `include/global.h`, save migration docs / tests | Needs a dedicated state field or a verified SaveBlock3 allocation. |
| Save helper | `src/save.c`, `src/load_save.c`, `src/start_menu.c` | Add Champions-aware checkpoint save that prevents challenge party / bag from overwriting normal save slots. |
| Party lifecycle | `src/pokemon.c`, `src/script_pokemon_util.c`, Scout Selection integration | Clear live party to 0, fill run party, restore normal party on exit. |
| Bag lifecycle | `src/item.c`, `src/bag.c`, `src/load_save.c` | Normal bag snapshot + run bag. Avoid direct scattered `memcpy`. |
| PC policy | `src/pokemon_storage_system.c` | MVP should block normal PC entry while active, or route to run-only stash. |
| Battle aftercare | `src/battle_setup.c`, `docs/features/trainer_battle_aftercare/` | Loss/retire/win outcome must be owned by Champions rule, not normal whiteout. |
| Item restore | `docs/features/battle_item_restore_policy/`, `docs/features/nonconsumable_held_items/` | Decide whether battle-consumed held items restore before run party deletion. |
| Progression | partygen / Scout Selection / battle selection | Stores run seed, offered candidates, selected Pokemon, next opponent, and streak. |

Suggested future helper surface:

```c
bool32 ChampionsRun_IsActive(void);
bool32 ChampionsRun_CanUseNormalPc(void);
bool32 ChampionsRun_BeginEntryReport(void);
bool32 ChampionsRun_SaveCheckpoint(u8 checkpointKind);
void ChampionsRun_ClearLivePartyAndBag(void);
void ChampionsRun_RestoreNormalState(u8 outcome);
void ChampionsRun_HandleBootRecovery(void);
```

## MVP Recommendation

First runtime slice should choose the conservative contract:

1. Entry requires Yes / report.
2. Save normal party + normal bag in dedicated Champions state.
3. Do a full normal save before mutating live party / bag.
4. Clear live party to 0 and use an empty run bag.
5. Disable normal PC while `ChampionsRun_IsActive()`.
6. Save run checkpoints with a Champions-specific helper.
7. On retire/loss/win, restore normal party / bag, clear run party / run bag,
   clear active state, then save.

This satisfies the user-visible goal without solving arbitrary PC rollback in
the first pass.

## Open Questions

- Should power-off resume from the last checkpoint by default, or should a
  stricter rule forfeit the run if power is cut outside a report point?
- Should safe-room PC access be allowed as "commit a new checkpoint" behavior,
  or should normal PC stay disabled for all active runs?
- Do recruited run Pokemon ever become rewards, or are all run Pokemon deleted
  unless explicitly converted by a reward screen?
- Should autosave happen after every battle, every room transition, or only at
  safe rooms?
- Where should the dedicated state live: freed SaveBlock1 space, SaveBlock3, or
  a new save extension with migration?

## Validation Targets

Future runtime branch must prove:

- entry with 0 live party after report;
- normal party / bag restore after retire;
- normal party / bag restore after loss;
- power-off after temporary report resumes the run;
- power-off before next checkpoint resumes the previous checkpoint or forfeits
  according to the chosen policy;
- no challenge Pokemon are written into `SaveBlock1.playerParty[]`;
- normal bag is not overwritten by empty run bag;
- normal PC cannot be mutated during MVP run, or mutations follow the selected
  safe-room checkpoint policy;
- mGBA Live boot/resume evidence covers at least one active checkpoint and one
  final restore.
