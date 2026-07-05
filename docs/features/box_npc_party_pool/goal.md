# Box NPC Party Pool Goal

## Status

Implemented as the standalone MVP on
`feature/box-npc-party-pool-20260705`. This document remains the dependency
survey and design record for the runtime implementation.

## Purpose

Create a debug-friendly opponent party source where Pokemon stored in Pokemon
Storage become the NPC trainer's battle candidates.

Primary user goal:

- Put up to 30 Pokemon in Box 1.
- Build a six-Pokemon candidate roster from those Box 1 Pokemon.
- Start a trainer battle where the opponent side is selected from that
  six-Pokemon candidate roster.
- Try different selection modes, especially:
  - choose six candidates from all valid Box 1 Pokemon.
  - use only Box 1 slots 1-6 as the six candidates.
  - battle with three Pokemon in singles and four Pokemon in doubles, selected
    from those six candidates.
  - later, choose from all storage boxes or other configured box ranges.
- Use this together with the current AI work to test whether the AI can pull
  strong, human-authored Pokemon into useful battle lines.

## Design Intent

This feature is the standalone replacement path for the heavy battle-debug setup
created during Smart Gimmick AI work.

The Smart Gimmick AI branch already proved that practical battle debugging needs
fast repeatable battles, aggressive AI presets, route-specific logs, and enough
control to reproduce awkward board states. Box NPC Party Pool should move that
workflow away from generated / hardcoded debug parties and toward user-authored
Pokemon in storage.

Intended workflow:

1. The user intentionally prepares Pokemon in Box 1.
2. The debug route reads Box 1 as a battle candidate pool.
3. The route selects a six-Pokemon candidate roster.
4. Singles choose three battle members from that roster.
5. Doubles choose four battle members from that roster.
6. The opponent AI runs with a strong / read-oriented debug AI preset where
   available.
7. Logs record enough information to replay or reason about the AI decision.

The existing Smart Gimmick AI debug battle route can remain as temporary
coverage, but the desired long-term manual AI harness is Box-authored opponents.
That lets the user design exact sets, held items, move pressure, speed control,
and awkward matchups without adding a new static trainer party for every test.

## Branch Strategy

This should not be folded into the Smart Gimmick AI feature branch as a normal
runtime change.

Decision:

- Planning docs: keep on a docs-only branch from current `master`.
- Clean implementation PR: create `feature/box-npc-party-pool-16-*` from current
  `master` when the feature is ready to implement.
- First runtime slice: implement Box NPC Party Pool as a standalone feature. Do
  not merge Pokemon State Editor, Unified Move Relearner, held item catalog, or
  Smart Gimmick AI into the first branch.
- Smart Gimmick AI is strongly desired for the actual manual testing target, but
  it remains a separate feature dependency. If the runtime Smart Gimmick AI
  source is still not on `master`, create a temporary integration branch from
  `feature/smart-gimmick-ai-16-20260604`, for example
  `integration/smart-ai-box-npc-party-pool-*`. That branch is for manual AI
  testing and should not be merged to `master` until its Smart Gimmick AI base is
  also ready.

Reason:

- The box-pool feature is a debug / trainer-party source feature.
- Smart Gimmick AI is an AI decision feature.
- They are useful together, but they have different ownership, validation, and
  merge risk.

Longer-term integration direction:

- Box NPC Party Pool should become one piece of a larger debug / training lab,
  alongside Pokemon State Editor, Unified Move Relearner, battle item restore,
  held item catalog assignment, and Smart Gimmick AI.
- Do not start by building one giant branch that owns every system at once.
  Adopt or refresh each shelf independently, then create a final integration
  branch when the individual slices are stable.
- A practical final integration branch name would be
  `integration/debug-battle-lab-*` or `integration/smart-ai-debug-lab-*`.

## Scope

### MVP In Scope

- Read Pokemon from Box 1, internal `boxId = 0`.
- Treat the box as read-only. Do not remove, move, consume, heal, or save over
  the original box Pokemon.
- Ignore empty slots, eggs, and bad eggs by using the existing storage sanity
  checks.
- Build a six-Pokemon candidate roster from Box 1, then select the actual battle
  members from that roster.
- Copy only the final battle members into `gParties[B_TRAINER_OPPONENT_A]`.
- Fill three opponent slots for single battles and four opponent slots for double
  battles.
- Clear unused opponent slots so `CalculateEnemyPartyCount()` reports the right
  party count.
- Add at least one debug-menu route that starts a trainer battle using the copied
  opponent party.
- Let the route use current player party by default so the user can test their
  team against the box-sourced NPC pool.
- Provide both single-battle and double-battle debug routes. The feature is for
  practical AI debugging, and singles / doubles stress different decision paths.
- Support multiple Box 1 selection modes:
  - `box1_slots_1_to_6`: use exactly Box 1 positions 0-5 as the six-candidate
    roster, rejecting the route if any required slot is empty, egg, or bad egg.
  - `box1_first_valid_6`: scan Box 1 and take the first six valid Pokemon as the
    candidate roster.
  - `box1_random_valid_6`: sample six valid Box 1 Pokemon without replacement as
    the candidate roster.
- Support battle-member selection from the six-candidate roster:
  - singles: use three Pokemon.
  - doubles: use four Pokemon.
  - include a deterministic first-N option for stable tests.
  - include a random-from-six option for the intended practical debug route.
- Reject invalid required slots for fixed-slot mode. Eggs and bad eggs are not
  legal battle candidates.
- Start the copied opponent party fully healed:
  - HP restored to max.
  - non-volatile status cleared.
  - PP restored.
- Do not consume or change held items on the source Box Pokemon. The battle copy
  may use items normally during battle, but the storage original must remain
  unchanged.
- For repeated debug battles, the player's current party should not permanently
  lose battle-consumed held items such as Berries. Box NPC MVP should either
  depend on the existing Battle Item Restore Policy shelf or include a narrow
  debug-battle aftercare path that restores original held items after battle.
- Add debug control for gimmick permission. MVP should include an "allow all
  selected opponent slots to use eligible gimmicks" option, while still allowing
  natural item / data-based checks for Mega, Z-Move, Tera, and Dynamax.
- Allow AI flags to be set independently for the Box NPC battle route. On
  `master`, this can use existing debug AI flag controls or selected-trainer AI
  metadata; on Smart Gimmick AI integration branches, add a full read-mode /
  smart-gimmick preset.
- Treat Box NPC battles as the future practical AI debug harness. The existing
  party-generated AI debug battle route should become optional or deprecated
  after Box NPC coverage replaces it.
- Put the route under the Party debug menu, not only under trainer selection.
- Record candidate source slots, final battle source slots, route format, AI
  preset, and mode in debug output or battle action logging.

### Later Scope

- Select from all boxes, not only Box 1.
- Select from a configured box range.
- Weighted selection from markings, held item, level, species role, or future
  catalog tags.
- Integrate Pokemon State Editor so the user can quickly edit EVs, IVs, level,
  nature, ability slot, Tera Type, Dynamax level, friendship, status-like debug
  fields, and other battle-relevant state before launching a Box NPC battle.
- Integrate Unified Move Relearner / move editing so the user can quickly set or
  relearn moves before launching a Box NPC battle.
- Integrate Battle Item Restore Policy so debug battles restore player held
  items, including Berries, after battle while preserving normal battle-time
  item mechanics.
- Integrate Nonconsumable Held Items / catalog assignment if the desired debug
  loop needs one owned held item to be assignable to multiple Pokemon without Bag
  quantity friction.
- Preview the selected opponent party before battle.
- Reuse Pre-Battle / In-Battle Team Viewer cache ideas if preview and battle
  result must match exactly.
- Run Box 1 pool battles under Smart Gimmick AI read-mode / gauntlet routes once
  that runtime source is available on the integration branch.

### Out of Scope For MVP

- Do not mutate Pokemon Storage.
- Do not deposit opponent battle changes back into Box 1.
- Do not create a general trainer-party editor UI.
- Do not rewrite `trainerproc` or static `trainers.party` generation.
- Do not promise opponent preview in the first slice.
- Do not support link, recorded, Frontier, E-Reader, Trainer Hill, Secret Base,
  or multi-opponent special cases in the first slice.

## Dependency Survey

| Area | Files / symbols | Notes |
| --- | --- | --- |
| Pokemon Storage layout | `include/pokemon_storage_system.h`, `struct PokemonStorage`, `TOTAL_BOXES_COUNT`, `IN_BOX_COUNT`, `gPokemonStoragePtr` | Box 1 is `boxId = 0`. Each box has 30 slots. |
| Box validation | `CheckBoxMonSanityAt(boxId, boxPosition)` | Existing helper accepts species-bearing non-egg, non-bad-egg BoxMons. Prefer this over checking only `MON_DATA_SPECIES`. |
| Box count / access | `CountMonsInBox`, `GetBoxMonDataAt`, `GetBoxedMonPtr`, `BoxMonAtToMon` | `CountMonsInBox` includes eggs because it checks species only, so MVP should scan slots with `CheckBoxMonSanityAt`. |
| Box to party copy | `BoxMonToMon`, `BoxMonAtToMon` | Copies BoxPokemon into a live `struct Pokemon`, recalculates stats, and derives HP from stored HP loss. MVP must decide whether to preserve or normalize HP / status. |
| Opponent party storage | `gParties[B_TRAINER_OPPONENT_A]`, `gPartiesCount`, `CalculateEnemyPartyCount`, `ZeroEnemyPartyMons`, `ZeroPartyMons` | Runtime party array is six slots, but Box NPC singles should end with count 3 and doubles should end with count 4. Empty trailing slots must be zeroed. |
| Normal trainer party generation | `src/battle_main.c`, `CreateNPCTrainerPartyFromTrainer`, `CreateNPCTrainerParty`, `DoTrainerPartyPool` | Static trainer party pools already solve "N candidates -> pick party", but only from ROM `TrainerMon` data. BoxMon pool is a live-save source and should not be forced through `TrainerMon` first. It is still a useful reference for candidate-to-battle-member selection. |
| Trainer Party Pools | `src/trainer_pools.c`, `DoTrainerPartyPool`, `RandomizePoolIndices`, `PickMonFromPool` | Useful reference for sampling without replacement, role tags, and fallback behavior. The first Box 1 mode can be simpler. |
| Debug trainer battle route | `src/debug.c`, `DebugAction_Trainers_TryBattle`, `DebugAction_Party_BattleSingle` | Existing debug actions can prebuild `gParties[B_TRAINER_OPPONENT_A]`, set `gBattleTypeFlags`, set `gIsDebugBattle`, calculate enemy party count, then start `BattleSetup_StartTrainerBattle_Debug()`. |
| Battle init behavior | `src/battle_main.c`, `CB2_InitBattleInternal` | When `DEBUG_OVERWORLD_MENU && gIsDebugBattle`, normal `CreateNPCTrainerParty()` is skipped. This makes a prebuilt Box 1 enemy party feasible. |
| Battle resource lifetime | `src/battle_util2.c`, `AllocateBattleResources`; `src/battle_controllers.c`, `SetUpBattleVarsAndBirchZigzagoon` | `gBattleStruct` is allocated during battle init. Gimmick slot bits cannot safely be treated as already set just because BoxMon was copied before battle init. |
| AI flags | `src/battle_ai_main.c`, `BattleAI_SetupFlags`; `src/debug.c`, `gDebugAIFlags` | Debug battle AI can be driven by `gDebugAIFlags`. Box NPC should become the preferred manual AI harness. Smart Gimmick AI read-mode flags are available only on branches that contain that runtime source. |
| Gimmick eligibility | `gBattleStruct->opponentMonCanTera`, `gBattleStruct->opponentMonCanDynamax`, `ShouldTrainerBattlerUseGimmick`, `CanTerastallize`, `CanDynamax` | Static `TrainerMon` generation sets these bits. BoxMon direct-copy must define how opponent Tera / Dynamax permission is set after battle resources exist. |
| Battle item restore | `docs/features/battle_item_restore_policy/`, `docs/features/nonconsumable_held_items/`, `src/battle_util.c` `TryRestoreHeldItems()` | Existing docs record a berry-inclusive battle-end restore shelf on `feature/battle-item-restore-policy`. `master` restores non-berry held items under Gen 9 policy but excludes Berries. Box NPC debug loops should not permanently consume player Berries. |
| Held item catalog assignment | `docs/features/nonconsumable_held_items/`, `feature/held-item-catalog-current-master-20260519` | Separate from battle-end restore. Catalog assignment solves Bag quantity / duplicate assignment friction; it should not be folded into the first Box NPC party-copy helper. |
| Pokemon State Editor | `docs/features/pokemon_state_editor/`, `feature/pokemon-state-editor-expansion` | Existing shelf for Summary-launched state editing. Useful for preparing Box 1 and player party test sets before Box NPC battles. |
| Unified Move Relearner | `docs/features/unified_move_relearner/`, `feature/unified-move-relearner` | Existing shelf for expanded move relearning / candidate generation. Useful for fast move setup before Box NPC battles. |
| Randomness | `include/random.h`, `RandomUniform`, `RandomElement`, `RandomTag` | Add a specific RNG tag for Box NPC party selection so test logs and battle determinism are readable. Avoid accidental extra RNG consumption if preview is added later. |
| Tests | `test/battle/trainer_control.c`, `test/save.c`, storage helpers | Existing trainer-party tests cover `CreateNPCTrainerPartyFromTrainer`. New tests should directly cover Box 1 scanning, selection without replacement, empty / egg / bad-egg filtering, and enemy party count. |
| Existing design docs | `docs/features/battle_selection/opponent_party_and_randomizer.md`, `docs/features/battle_selection/risks.md`, `docs/features/smart_gimmick_ai/` | Battle selection docs already warn about preview / battle mismatch and RNG double-consumption. Smart Gimmick AI docs are the intended manual AI-testing consumer. |

## Rechecked Dependency Findings

This pass used the corrected contract: Box 1 creates a six-Pokemon candidate
roster, singles copy three battle members, and doubles copy four battle members.

Findings:

- Standalone MVP can be implemented from `master`. It does not need Smart
  Gimmick AI source to build the Box-authored opponent party.
- `Makefile` gathers C sources with `$(wildcard src/*.c src/*/*.c
  src/*/*/*.c)`, so a new `src/box_npc_party_pool.c` module will be picked up
  without a source-list edit.
- `CheckBoxMonSanityAt()` is the correct Box candidate filter. `CountMonsInBox()`
  only checks species, so it can include eggs.
- `BoxMonAtToMon()` is the correct copy primitive, but it preserves storage HP
  loss and status through `BoxMonToMon()`. The copied battle party must be
  normalized after copy.
- `CalculatePartyCount()` stops at the first `SPECIES_NONE`. Therefore singles
  must fill enemy slots 0-2 and zero 3-5; doubles must fill enemy slots 0-3 and
  zero 4-5.
- `DebugAction_Party_BattleSingle()` proves the debug route can prebuild
  `gParties[B_TRAINER_OPPONENT_A]`, set `gBattleTypeFlags`, set
  `gDebugAIFlags`, set `gIsDebugBattle`, call `CalculateEnemyPartyCount()`, then
  call `BattleSetup_StartTrainerBattle_Debug()`.
- `CB2_InitBattleInternal()` skips normal `CreateNPCTrainerParty()` when
  `DEBUG_OVERWORLD_MENU && gIsDebugBattle`, so Box NPC can safely supply the
  prebuilt enemy party without rewriting trainer party generation.
- `SetUpBattleVarsAndBirchZigzagoon()` runs before the normal trainer party
  generation point and before `AssignUsableGimmicks()`. This is the right battle
  init window for applying Box NPC pending opponent gimmick permission bits.
- `AssignUsableGimmicks()` runs at first-turn setup and every new turn. Box NPC
  Tera / Dynamax permission must be present before this runs, otherwise opponent
  slots may never expose those gimmicks to the AI.
- On `master`, Mega / Ultra Burst / Z-Move permission is mostly natural,
  item-based behavior for opponent battlers. Tera / Dynamax additionally require
  `gBattleStruct->opponentMonCanTera` / `opponentMonCanDynamax` bits for NPCs.
- Smart Gimmick AI branch adds `AI_FLAG_READ_PLAYER_MOVE`,
  `AI_FLAG_SMART_GIMMICK`, aggressive gimmick flags, generated gauntlet debug
  routes, and `gBattleActionLog`. These are integration dependencies, not
  standalone MVP dependencies.
- Smart Gimmick AI branch's manual gauntlets already use 3v3 singles and 4v4
  doubles. Box NPC should match that battle size so the generated gauntlet route
  can later be replaced by Box-authored opponents cleanly.

## Integration Plan With Existing Shelves

Recommended adoption order:

1. Box NPC Party Pool MVP from current `master`: read Box 1, build a six-Pokemon
   candidate roster, select 3v3 singles or 4v4 doubles battle members, launch
   debug battles, and log both candidate slots and final battle slots.
2. Battle Item Restore Policy from its existing shelf or a fresh reapply:
   restore player held items, including Berries, after debug battles while
   preserving normal battle-time consumption state for `Recycle`, `Unburden`,
   `Harvest`, `Pickup`, and related mechanics.
3. Pokemon State Editor adoption: fast setup for IVs / EVs / nature / ability
   slot / Tera / Dynamax / other battle-relevant state.
4. Unified Move Relearner adoption: fast move setup and relearning.
5. Nonconsumable Held Item catalog assignment if team-building friction remains:
   one catalog token can assign held-effect items without Bag quantity drift.
6. Smart Gimmick AI integration: use the Box NPC pool as the practical manual AI
   test harness with full read-mode / smart-gimmick presets.

This keeps each feature reviewable while still aiming at a unified debug lab.
The integration branch can be built after the individual slices have their own
validation evidence.

## Branch Documentation And Dependency Discipline

Because this feature is meant to combine several already-large shelves, every
branch must update its own owning docs before integration. The integration branch
should not be the first place where behavior, validation, or risk is explained.

Per-branch requirements:

| Branch / shelf | Owning docs to update | Dependency notes required |
| --- | --- | --- |
| Box NPC Party Pool | `docs/features/box_npc_party_pool/goal.md`, later `implementation.md` and `test_plan.md` | Box read-only policy, selection modes, singles / doubles route evidence, selected-slot logging, storage-original preservation. |
| Battle Item Restore Policy | `docs/features/battle_item_restore_policy/`, plus cross-link from Box NPC docs | Whether Berries restore, which battle types restore, and why battle-time item loss remains intact. |
| Nonconsumable Held Items | `docs/features/nonconsumable_held_items/` | Catalog assignment scope, Bag / Party / Storage ownership, duplicate item rules, and whether it is debug-only or global. |
| Pokemon State Editor | `docs/features/pokemon_state_editor/` | Which state fields are editable, whether Box mons are editable, Summary / Party entry points, and redraw / save safety evidence. |
| Unified Move Relearner | `docs/features/unified_move_relearner/` | Which move sources are exposed, move overwrite behavior, Box / Party entry scope, and candidate generation limits. |
| Smart Gimmick AI | `docs/features/smart_gimmick_ai/` | Whether Box NPC battles are used as manual AI evidence, which AI flags / read-mode preset is used, and what logs prove the AI path. |
| Debug Battle Lab integration | New integration handoff doc, probably `docs/features/debug_battle_lab/` if the umbrella becomes real | Exact branch list, commit IDs, conflict decisions, final route map, and combined validation matrix. |

Integration rules:

- Start integration only after each component branch has its own focused
  validation evidence.
- Do not merge a component branch into an integration branch without reading its
  owning docs and test plan.
- When a component behavior is changed during integration, update that
  component's owning docs, not only the integration notes.
- Before any `master` PR, run `rtk git diff --name-only master..HEAD` and verify
  the branch scope matches the PR purpose.
- Keep docs-only master handoffs separate from runtime implementation branches.
- Record accepted risks explicitly. Examples: old mGBA stale session cleanup,
  existing mdBook warnings, broad test filters with known failing labels, or
  temporarily missing Smart Gimmick AI integration.

Dependency check before coding:

1. Confirm branch base: clean `master` for independent adoption; Smart Gimmick AI
   branch only for temporary integration testing.
2. Confirm source ownership: party / storage code, battle item restore code,
   Summary UI code, move relearner code, and AI code should not be mixed without
   an integration plan.
3. Confirm runtime state ownership: `gPokemonStoragePtr`, `gParties`,
   `gPartiesCount`, `gBattleStruct`, `gDebugAIFlags`, Bag quantities, and battle
   item snapshots all have different lifetimes.
4. Confirm validation path: source changes need focused `check`, normal/debug
   ROM builds where relevant, mdBook, and one mGBA Live smoke.
5. Confirm manual evidence: for this feature family, record selected Box slots,
   battle format, AI preset, gimmick permission mode, and item-restore policy in
   logs or docs.

## Proposed MVP Runtime Shape

Use a dedicated helper instead of overloading static trainer data:

```c
enum BoxNpcPartyPoolMode
{
    BOX_NPC_POOL_BOX1_SLOTS_1_TO_6,
    BOX_NPC_POOL_BOX1_FIRST_VALID_6,
    BOX_NPC_POOL_BOX1_RANDOM_VALID_6,
};

enum BoxNpcBattleFormat
{
    BOX_NPC_BATTLE_SINGLE_3,
    BOX_NPC_BATTLE_DOUBLE_4,
};

enum BoxNpcBattleMemberMode
{
    BOX_NPC_BATTLE_MEMBERS_FIRST_N,
    BOX_NPC_BATTLE_MEMBERS_RANDOM_N,
};

u8 BuildOpponentPartyFromBoxNpcPool(
    u8 boxId,
    enum BoxNpcPartyPoolMode poolMode,
    enum BoxNpcBattleFormat battleFormat,
    enum BoxNpcBattleMemberMode memberMode);
```

Suggested behavior:

1. `ZeroEnemyPartyMons()`.
2. Scan Box 1 slot `0..IN_BOX_COUNT - 1`.
3. Keep only `CheckBoxMonSanityAt(boxId, slot)` candidates.
4. Choose a six-Pokemon candidate roster according to `poolMode`.
5. Choose final battle members from that six-Pokemon roster:
   - three members for `BOX_NPC_BATTLE_SINGLE_3`.
   - four members for `BOX_NPC_BATTLE_DOUBLE_4`.
   - deterministic first-N or random-N according to `memberMode`.
6. For each final battle member slot, call
   `BoxMonAtToMon(boxId, slot, &gParties[B_TRAINER_OPPONENT_A][outSlot])`.
7. Normalize battle-start state:
   - restore HP to max.
   - clear non-volatile status.
   - restore PP.
8. Recalculate stats after normalization.
9. Set `gPartiesCount[B_TRAINER_OPPONENT_A]` through `CalculateEnemyPartyCount()`.
10. Store candidate source slots, final battle source slots, format, member mode,
    AI preset, and pool mode for logs / debug display.
11. Start the debug trainer battle with `gIsDebugBattle = TRUE`.
12. After battle resources exist, apply the route's opponent gimmick permission
    policy to `gBattleStruct->opponentMonCanTera` and
    `gBattleStruct->opponentMonCanDynamax`.

## Standalone Implementation Method

Preferred file layout:

| File | Change |
| --- | --- |
| `include/box_npc_party_pool.h` | Public config / result structs and helper declarations. |
| `src/box_npc_party_pool.c` | Box scanning, six-roster selection, final 3/4 member selection, copy / normalization, pending gimmick policy, last-result debug state. |
| `src/debug.c` | Add Party menu entries and call the Box NPC helper. Keep menu glue here; do not put storage selection logic here. |
| `src/battle_main.c` | Add a narrow battle-init hook after `SetUpBattleVarsAndBirchZigzagoon()` for pending Box NPC gimmick policy. |
| `include/random.h` | Add specific RNG tags for Box NPC roster selection and final battle-member selection. |

Proposed public shape:

```c
#define BOX_NPC_POOL_BOX_ID 0
#define BOX_NPC_CANDIDATE_ROSTER_SIZE 6
#define BOX_NPC_SINGLE_BATTLE_SIZE 3
#define BOX_NPC_DOUBLE_BATTLE_SIZE 4

enum BoxNpcPartyPoolMode
{
    BOX_NPC_POOL_BOX1_SLOTS_1_TO_6,
    BOX_NPC_POOL_BOX1_FIRST_VALID_6,
    BOX_NPC_POOL_BOX1_RANDOM_VALID_6,
};

enum BoxNpcBattleFormat
{
    BOX_NPC_BATTLE_SINGLE_3,
    BOX_NPC_BATTLE_DOUBLE_4,
};

enum BoxNpcBattleMemberMode
{
    BOX_NPC_BATTLE_MEMBERS_FIRST_N,
    BOX_NPC_BATTLE_MEMBERS_RANDOM_N,
};

enum BoxNpcOpponentGimmickPolicy
{
    BOX_NPC_GIMMICK_NATURAL,
    BOX_NPC_GIMMICK_ALLOW_TERA_DYNAMAX_ALL_FINAL_MEMBERS,
};

struct BoxNpcPartyPoolConfig
{
    enum BoxNpcPartyPoolMode poolMode;
    enum BoxNpcBattleFormat battleFormat;
    enum BoxNpcBattleMemberMode memberMode;
    enum BoxNpcOpponentGimmickPolicy gimmickPolicy;
    u64 aiFlags;
};

struct BoxNpcPartyPoolResult
{
    u8 boxId;
    u8 candidateSlots[BOX_NPC_CANDIDATE_ROSTER_SIZE];
    u8 finalSlots[PARTY_SIZE];
    u8 candidateCount;
    u8 battleCount;
    enum BoxNpcPartyPoolMode poolMode;
    enum BoxNpcBattleFormat battleFormat;
    enum BoxNpcBattleMemberMode memberMode;
    enum BoxNpcOpponentGimmickPolicy gimmickPolicy;
};

bool32 BoxNpcPartyPool_TryBuildOpponentParty(
    const struct BoxNpcPartyPoolConfig *config,
    struct BoxNpcPartyPoolResult *result);

void BoxNpcPartyPool_ApplyPendingBattleInitPolicy(void);
void BoxNpcPartyPool_ClearPendingBattleInitPolicy(void);
const struct BoxNpcPartyPoolResult *BoxNpcPartyPool_GetLastResult(void);
```

Implementation details:

- `BoxNpcPartyPool_TryBuildOpponentParty()` owns all Box scanning and party-copy
  logic. It should not start battle by itself.
- `debug.c` owns the route:
  1. validate player party requirements for the selected route.
  2. call `ZeroEnemyPartyMons()`.
  3. call `BoxNpcPartyPool_TryBuildOpponentParty()`.
  4. set `gBattleTypeFlags = BATTLE_TYPE_TRAINER` plus
     `BATTLE_TYPE_DOUBLE` for doubles.
  5. set `gDebugAIFlags = config.aiFlags`.
  6. set `gIsDebugBattle = TRUE`.
  7. set `gBattleEnvironment = BattleSetup_GetEnvironmentId()`.
  8. call `CalculateEnemyPartyCount()`.
  9. call `BattleSetup_StartTrainerBattle_Debug()`.
- Do not call `CreateNPCTrainerPartyFromTrainer()` for the opponent side in Box
  NPC routes.
- Do not call `ZeroPlayerPartyMons()` in the default Box NPC route. The desired
  workflow is user-authored player-side sets plus user-authored Box 1 opponent
  candidates.
- If strict player-side 3v3 / 4v4 is required, implement it as a separate route
  decision:
  - require the current player party count to already be 3 or 4; or
  - snapshot the player party with `SavePlayerParty()`, copy first/random 3 or 4
    selected player mons into slots 0..N-1, zero the rest, and restore with
    `LoadPlayerParty()` after battle.
- Player-side temporary truncation must be treated as a higher-risk addition
  because battle-end and whiteout code can read saved player party data. Keep it
  separate from the opponent Box copy helper.
- Normalize each copied opponent member using the same pattern as `HealMon()` in
  `src/battle_pike.c`: set HP to max, restore PP with `CalculatePPWithBonus()`,
  clear `MON_DATA_STATUS`, then `CalculateMonStats()`.
- Add two RNG tags, not one:
  - one for selecting the six-candidate roster from Box 1.
  - one for selecting the final 3 or 4 battle members from the six-candidate
    roster.
- Log both arrays. A log that records only the final party cannot explain why a
  random route did or did not choose a specific Box 1 Pokemon.
- For Tera / Dynamax, store a pending Box NPC policy while building the party.
  Apply it from battle init after `gBattleStruct` exists and before first-turn
  `AssignUsableGimmicks()`.
- For Mega / Ultra Burst / Z-Move in standalone MVP, prefer natural item-based
  behavior. A true "all permit" bypass for those gimmicks should be delayed until
  Smart Gimmick AI integration unless the route explicitly needs it.
- Use `DebugPrintf()` for a minimal master-side route log when available:
  route format, pool mode, member mode, candidate slots, final slots, battle
  count, AI flags, and gimmick policy. The richer JSON battle action log belongs
  to the Smart Gimmick AI integration branch.

Minimal battle-init hook:

```c
SetUpBattleVarsAndBirchZigzagoon();
BoxNpcPartyPool_ApplyPendingBattleInitPolicy();
```

The hook must be a no-op unless a Box NPC route has set pending policy. It should
not affect normal trainer battles, generated debug battles, link battles,
recorded battles, Frontier, Trainer Hill, Secret Base, or special partner
battles.

## Proposed Debug Entries

MVP menu candidates:

- `Party -> Box NPC Battle -> Single 3v3 -> Box1 Slots 1-6`
- `Party -> Box NPC Battle -> Single 3v3 -> Box1 First Valid 6`
- `Party -> Box NPC Battle -> Single 3v3 -> Box1 Random 6`
- `Party -> Box NPC Battle -> Double 4v4 -> Box1 Slots 1-6`
- `Party -> Box NPC Battle -> Double 4v4 -> Box1 First Valid 6`
- `Party -> Box NPC Battle -> Double 4v4 -> Box1 Random 6`

Alternative if menu space is tight:

- Add options under existing `Debug -> Trainers` next to `Try Battle`.
- Use the selected Trainer 1 only as metadata for trainer class / name / AI flags,
  while the actual opponent party comes from Box 1.
- Keep the user-facing route clear about both stages:
  - Box source mode: slots 1-6, first valid 6, or random valid 6.
  - battle-member mode: first-N or random-N from the six-candidate roster.

Default for manual AI work:

- Player side: current player party.
- Opponent side: Box 1 six-candidate roster, then three singles members or four
  doubles members copied into the actual enemy party.
- Format: explicit singles and doubles routes. Do not hide doubles behind a
  vague toggle; the selected route should make the battle format clear.
- AI flags: individually configurable. On `master`, include a strong debug AI
  preset that covers the existing intelligent flags available there. On Smart
  Gimmick AI integration branches, map this route to the full read-mode /
  smart-gimmick preset because Box-authored opponents are meant to replace the
  generated party AI debug battle route.
- Gimmicks: include a debug all-permit option. Natural permission should also
  work when the copied Pokemon / player inventory state already satisfies the
  usual Mega / Z / Tera / Dynamax checks.

## Selection Semantics

Define these explicitly before implementation:

| Mode | Behavior | Use case |
| --- | --- | --- |
| `box1_first_valid_6` | Scan Box 1 left-to-right, top-to-bottom, taking the first six valid non-egg Pokemon. Empty slots are skipped. | Stable quick smoke. |
| `box1_slots_1_to_6` | Only inspect Box 1 positions 0-5. If any required slot is empty, egg, or bad egg, reject the route and show a clear debug message. | Exact manual curation. |
| `box1_random_valid_6` | Scan all 30 Box 1 slots, then sample six valid candidates without replacement. If fewer than six valid candidates exist, reject with a clear debug message. | Main "30 candidates -> candidate roster" behavior. |
| `all_boxes_random_6` | Later: scan every storage box, sample six. | Larger experiment pool. |

After the six-candidate roster is selected, choose the actual battle party from
those six:

| Battle format | Battle count | Member modes |
| --- | --- | --- |
| Singles | 3 | first three candidates for stable smoke; random three from six for practical debug. |
| Doubles | 4 | first four candidates for stable smoke; random four from six for practical debug. |

For random pool selection and random battle-member selection, prefer a
Fisher-Yates shuffle over the candidate slot array or a repeated `RandomUniform`
with used-slot rejection. The latter is fine for 30 slots, but shuffle is easier
to test for "no duplicates".

Do not copy all six roster members into the enemy party for MVP. The intended
debug battle sizes are 3v3 singles and 4v4 doubles.

## State And Save Policy

| Data | Owner | Lifetime | Policy |
| --- | --- | --- | --- |
| Box 1 Pokemon | `gPokemonStoragePtr->boxes[0]` | Save-backed | Read-only. Never mutate for this feature. |
| Six-candidate roster slots | New temporary runtime helper or debug buffer | One battle start | Records the six Box source slots selected by pool mode. Needed for logs, random reproducibility, and future preview. Do not put in save for MVP. |
| Final battle source slots | New temporary runtime helper or debug buffer | One battle start | Records the three singles or four doubles slots selected from the six-candidate roster. Needed to prove the actual enemy party matches the logged selection. |
| Opponent battle party | `gParties[B_TRAINER_OPPONENT_A]` | Current battle | Copied from the final battle source slots. Singles count should be 3; doubles count should be 4. Can change during battle normally. |
| Opponent AI flags | `gDebugAIFlags` or selected trainer AI flags | Current battle | Individually configurable; route should make the selected AI policy visible in docs / debug output. |
| Gimmick permission bits | `gBattleStruct` | Current battle | Must be initialized after battle resources exist, or via a battle-init hook. Include a debug all-permit option. |
| Held items | Copied battle party only | Current battle | Storage originals must not lose or consume items. |
| HP / status / PP | Copied battle party only | Current battle | Normalize to full HP, no non-volatile status, and full PP before battle. |

## Key Risks

| Risk | Severity | Mitigation |
| --- | --- | --- |
| Box Pokemon accidentally mutated | High | Use copy-only helpers and never write back with `SetBoxMonAt` / `SetBoxMonDataAt`. |
| Eggs or bad eggs enter battle | High | Filter with `CheckBoxMonSanityAt`, not `CountMonsInBox`. |
| Enemy party count is wrong | High | Zero all opponent slots before copy, copy only 3 singles or 4 doubles battle members, and call `CalculateEnemyPartyCount()` after copy. |
| Candidate roster and final battle party are confused | High | Store and log the six-candidate roster separately from the final 3/4 battle-member selection. Tests must check both arrays. |
| Gimmick-capable Box mons cannot Tera / Dynamax as opponents | High for AI testing | Add explicit Box NPC gimmick policy and initialize `opponentMonCanTera` / `opponentMonCanDynamax` after `gBattleStruct` allocation. |
| Gimmick policy hook affects normal battles | High | Make `BoxNpcPartyPool_ApplyPendingBattleInitPolicy()` a no-op unless the Box NPC route set pending state. Test a normal debug battle still uses existing behavior. |
| Singles and doubles drift apart | High | Add both route variants from the start and test both formats. Doubles must not be treated as an afterthought because AI switch, Protect, targeting, and partner logic differ. |
| Player side is not actually 3v3 / 4v4 | High if strict battle size is required | Decide whether Box NPC routes only limit the opponent party or also temporarily limit the player party. If player truncation is implemented, snapshot and restore carefully with `SavePlayerParty()` / `LoadPlayerParty()`. |
| Player Berries / consumables are lost during repeated debug battles | High | Integrate Battle Item Restore Policy or add a debug-route aftercare hook before accepting the manual testing loop as complete. Do not disable battle-time consumption because mechanics and AI rely on temporary item loss. |
| Held item catalog scope swallows Box NPC MVP | Medium | Keep battle-end restore separate from catalog assignment. Catalog mode can be adopted later for setup convenience. |
| Existing State Editor / Move Relearner shelves conflict with Box NPC UI | Medium | Keep Box NPC entry under Party, but integrate editor / relearner through Summary or a debug lab hub in a later branch. |
| Debug battle skips normal trainer party generation | Medium | This is useful for MVP, but the route must prebuild `gParties[B_TRAINER_OPPONENT_A]` before battle start. |
| Preview and battle use different random picks | Medium later | Do not add preview in MVP. Later cache selected source slots and reuse them for battle. |
| Randomness is hard to reproduce | Medium | Add a dedicated `RandomTag`; log seed, candidate roster slots, and final battle slots in debug or action-log output. |
| Box 1 has fewer than required Pokemon | Medium | Reject the route with a clear debug message for modes that require six Pokemon. |
| Smart Gimmick AI dependency is not on `master` | Medium | Keep clean feature independent; use a temporary integration branch for immediate AI testing. |
| Existing generated Party AI debug battle route becomes obsolete | Medium | Keep it during standalone rollout, but document Box NPC as the preferred long-term manual AI harness once equivalent AI preset coverage exists. |
| Copied HP / status is surprising | Medium | Normalize to healthy battle state for debug battles and document that storage originals are unchanged. |
| Held items are copied from Box mons | Low / Medium | This is desired for realistic set testing, but tests should confirm the original held item remains in storage. |

## Initial Implementation Plan

1. Add `include/box_npc_party_pool.h` and `src/box_npc_party_pool.c`.
2. Implement Box 1 candidate scanning with `CheckBoxMonSanityAt`.
3. Implement `box1_slots_1_to_6`, `box1_first_valid_6`, and
   `box1_random_valid_6`.
4. Implement six-candidate roster selection.
5. Implement final battle-member selection: first/random 3 for singles and
   first/random 4 for doubles.
6. Copy only final battle members into `gParties[B_TRAINER_OPPONENT_A]`.
7. Normalize HP / status / PP on the copied battle party.
8. Preserve source Box held items by never writing battle results back to
   storage.
9. Add single 3v3 and double 4v4 debug menu entries under Party.
10. Add AI flag configuration / preset handling for the route.
11. Add route-level gimmick permission policy, including all-permit debug mode.
12. Add candidate-roster and final-battle-slot logging.
13. Add the pending battle-init hook for Tera / Dynamax opponent permission,
    guarded so non-Box routes are unchanged.
14. Decide whether player side is current-party unrestricted, current party must
    already be 3/4, or temporary first/random 3/4 with restore.
15. Decide whether the first runtime branch includes a narrow debug-battle held
    item restore hook or depends on the Battle Item Restore Policy branch.
16. Add unit tests for candidate scanning, six-roster selection, and 3/4
    battle-member selection.
17. Add focused battle tests for enemy party count, copied moves/items/species,
    and storage-original held item preservation.
18. Add focused battle-end item restore coverage if item restore is included in
    this branch.
19. Add mGBA Live smoke for menu route and battle start.
20. On Smart Gimmick AI integration branch, rerun manual AI sessions using the
    same Box 1 pool route.

## Validation Plan

Docs-only planning:

- `rtk mdbook build docs`
- `rtk git diff --check`

Runtime implementation:

- `rtk make -j16 -O check TESTS='Box NPC Party Pool'`
- `rtk make -j16 -O check TESTS='test/battle/trainer_control.c'`
- `rtk make -j16 -O check TESTS='Debug battles do not give exp or EVs'`
- Focused checks that normal trainer/debug battles are unchanged when no Box NPC
  pending state exists.
- If item restore is included: focused battle item restore checks for Berries
  and non-berry held items.
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- mGBA Live focused route:
  - boot debug ROM.
  - open debug menu.
  - start a Box 1 NPC single 3v3 battle.
  - confirm three opponent party members come from the logged Box 1 final battle
    slots.
  - start a Box 1 NPC double 4v4 battle.
  - confirm four opponent party members come from the logged Box 1 final battle
    slots and both opposing leads are valid.
  - confirm the six-candidate roster and final battle slots are logged.
  - consume a player held Berry in a debug battle and confirm it is restored
    after battle when restore policy is enabled.
  - stop session cleanly or record cleanup caveat.

## Open Questions

- Exact debug UI shape for AI flag editing:
  - reuse existing trainer debug AI flags.
  - add a compact Box NPC AI preset selector.
  - or both.
- Exact gimmick all-permit implementation:
  - route-level option.
  - debug global.
  - or hidden preset tied to Box NPC battle modes.
- Whether Mega / Z-Move should remain purely item / inventory based or also get
  an all-permit debug bypass.
- Whether first Box NPC implementation should directly include debug-battle item
  restore, or require the Battle Item Restore Policy shelf to be adopted first.
- Whether held item catalog assignment belongs in the final debug lab integration
  branch, or remains a separate team-building convenience feature.
- Whether Pokemon State Editor and Unified Move Relearner should be integrated
  through a shared "Debug Battle Lab" hub or kept as Summary-first tools that the
  user opens before starting Box NPC battles.
- Whether the route should optionally use a generated debug player party in
  addition to the current player party.
- Whether strict 3v3 / 4v4 means player side must also be temporarily reduced,
  or whether the first standalone route only constrains the opponent side.
- If player side is reduced, whether the route uses first-N, random-N, or an
  explicit player selection UI before battle.
- Whether the default practical route should use random battle members from the
  six-candidate roster, with first-N kept only for stable tests.
- Should this route eventually integrate with Team Viewer preview, or remain a
  fast battle-start tool?
- Should selected source slots be included in the existing battle action log on
  the Smart Gimmick AI branch?
