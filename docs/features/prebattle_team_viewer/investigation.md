# Pre-Battle / In-Battle Team Viewer Investigation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `7c19f56901`; `git describe` = `expansion/1.15.2-38-g7c19f56901` |
| Code status | Docs-only investigation |
| Provenance | Local project feature docs |

## Branch Baseline Check

Initial work started from `master` `7c19f56901` on `feature/prebattle-team-viewer`;
Phase 2 continued on `feature/prebattle-team-viewer-phase2`.
The previous selection branch exists at `feature/battle-selection-mvp`, but it is not a
safe implementation base.

Real diff from current `master` to `feature/battle-selection-mvp` includes source additions
for battle selection, but also deletes newer docs that now exist on `master`:

| Area | Result |
|---|---|
| battle selection source | `include/trainer_battle_selection.h`, `src/trainer_battle_selection.c`, `src/battle_setup.c`, `src/party_menu.c`, `include/config/battle.h` changes exist on the old branch. |
| current master docs | bag expansion and field move docs exist on `master` but not on the old selection branch. |
| decision | Do not merge the old branch wholesale. Reapply only the source slice needed by this feature. |

## Existing Files

| File | Symbols / data | Notes |
|---|---|---|
| `src/battle_setup.c` | `BattleSetup_StartTrainerBattle`, `CB2_EndTrainerBattle`, `gBattleTypeFlags`, `TRAINER_BATTLE_PARAM` | Correct hook point after trainer battle flags are known and before `DoTrainerBattle()`. Selection MVP already inserts its gate here. |
| `src/battle_main.c` | `CB2_InitBattleInternal`, `CreateNPCTrainerPartyFromTrainer`, `CreateNPCTrainerParty` | Normal trainer enemy party is generated during battle init, after the desired team viewer timing. |
| `src/trainer_pools.c` | `DoTrainerPartyPool`, `RandomizePoolIndices`, `PickMonFromPool` | Pool / random order can change which opponent mons appear. Preview must use the same result as battle. |
| `include/config/battle.h` | `B_POOL_SETTING_CONSISTENT_RNG`, `B_POOL_SETTING_USE_FIXED_SEED` | Defaults are `FALSE`, so pool preview cannot assume deterministic local RNG. |
| `src/party_menu.c` | `InitChooseHalfPartyForBattle`, `gSelectedOrderFromParty`, selection MVP entrypoints | Existing choose-half UI can remain the first selection implementation. |
| `src/battle_controller_player.c` | `HandleInputChooseAction`, `HandleInputChooseMove`, `OpenPartyMenuToChooseMon` | In-battle viewer button should hook action menu first. Move menu has existing L/START/SELECT behavior. |
| `src/battle_interface.c` | `CanThrowLastUsedBall`, `TryToAddMoveInfoWindow`, `SwapHpBarsWithHpText` | Existing shortcut UI uses R/L/START. In trainer battles, last-used-ball cannot throw, making action-menu `R_BUTTON` the least invasive first candidate. |
| `include/pokemon_icon.h`, `src/pokemon_icon.c` | `LoadMonIconPalettes`, `CreateMonIcon`, `FreeMonIconPalettes` | Team viewer can use existing Pokemon icon sprites, but should own palette / sprite lifetime on a dedicated screen. |
| `docs/features/battle_selection/` | selection MVP docs | Current dependency and restore contract. |
| `docs/features/battle_selection/opponent_party_and_randomizer.md` | opponent preview timing notes | Already identifies preview / battle mismatch as the main risk. |

## Official Reference Check

Checked on 2026-05-10:

| Source | What matters for this feature |
|---|---|
| `https://www.pokemonchampions.jp/ja/` | Official top page frames Pokémon Champions as a battle-focused game using type, abilities, and move choices. |
| `https://www.pokemonchampions.jp/ja/battle/` | Official battle page confirms single and double formats. |
| `https://www.pokemonchampions.jp/ja/pokemon/` | Official Pokémon page confirms Pokémon HOME visitors and notes move changes through training when a visiting Pokémon has unusable moves. |
| `https://www.pokemonchampions.jp/ja/training/` | Official training page confirms stats can be raised and abilities / moves can be changed. |
| `https://champions.pokemon.com/en-us/?pubDate=20250228` | Official page metadata exposes a Pokémon team menu screenshot and a training menu screenshot with stat points, stat alignment, moves, and Ability categories. |

The official text found during this pass did not explicitly describe the exact `Y` button
behavior on the selection screen. The user's screenshot shows a `Y` strength-display prompt,
so implementation should verify the official footage / runtime reference before finalizing
copy and button behavior.

Implementation note: the first GBA slice maps this strength-display action to
`SELECT_BUTTON` through `B_TEAM_VIEWER_DETAILS_BUTTON`, because the hardware has no `Y_BUTTON`.

## Legacy Console Reference Check

Older console references are secondary to Pokémon Champions, but they give useful precedents:

| Source | Design note |
|---|---|
| Nintendo UK Pokémon Colosseum page | Official page describes uploading Ruby / Sapphire Pokémon to GameCube, tournaments, four-player multi-battles, and two-on-two team battles. This supports treating team display as a formal battle setup surface. |
| Smogon Colosseum / XD mechanics guide | Notes Colosseum / XD center double battles and that Colosseum Battle Mode Mt. Battle uses 3v3 singles or 4v4 doubles chosen from 6. This matches the requested 3/4-from-6 selection model. |
| Bulbapedia Group Battle | Notes Colosseum battle rule setup was sequential, while XD put settings on one screen. For this feature, compact one-screen state is preferable. |
| Nintendo ZA Battle Revolution page | Official page describes copying trained Diamond / Pearl Pokémon, Rental Passes with six Pokémon, DS Battle Mode, hidden DS commands on TV, and simple Wii Remote / D-pad menu navigation. |
| Bulbapedia Battle Pass | Battle Passes contain trainer and party Pokémon information. This is useful as a "team card" mental model for the viewer cache and future visual polish. |

See [Legacy Console Battle UI References](legacy_console_references.md) for the source list and
takeaways.

## Existing Flow

Current normal trainer battle:

```text
trainerbattle script
  -> dotrainerbattle
  -> BattleSetup_StartTrainerBattle()
  -> gBattleTypeFlags decided
  -> DoTrainerBattle()
  -> CreateBattleStartTask()
  -> CB2_InitBattle()
  -> CB2_InitBattleInternal()
  -> CreateNPCTrainerParty()
  -> CreateNPCTrainerPartyFromTrainer()
  -> DoTrainerPartyPool()
  -> gEnemyParty finalized
  -> battle
```

Selection MVP branch flow:

```text
BattleSetup_StartTrainerBattle()
  -> TrainerBattleSelection_ShouldOffer()
  -> InitChooseHalfPartyForTrainerBattleSelection()
  -> party menu selection
  -> TrainerBattleSelection_StartBattleFromSelection()
  -> DoTrainerBattle()
```

Target team viewer flow:

```text
BattleSetup_StartTrainerBattle()
  -> PreBattleTeamViewer_ShouldOffer()
  -> PreBattleTeamViewer_PrepareOpponentCache()
  -> PreBattleTeamViewer_Begin()
  -> viewer confirm
  -> TrainerBattleSelection_Begin()
  -> selection confirm
  -> TrainerBattleSelection_StartBattleFromSelection()
  -> DoTrainerBattle()
  -> CB2_InitBattleInternal() uses cached opponent party
```

In-battle viewer flow:

```text
PlayerHandleChooseAction()
  -> HandleInputChooseAction()
  -> JOY_NEW(B_TEAM_VIEWER_BUTTON) in eligible trainer battle
  -> PreBattleTeamViewer_BeginInBattle()
  -> read-only viewer
  -> close with A/B/R
  -> restore battle action menu without emitting a battle command
```

## Opponent Party Timing

`gEnemyParty` is too late for this feature. It is normally populated inside
`CB2_InitBattleInternal()`, after battle resources and battle background setup begin.
The team viewer needs to run before that.

Calling `CreateNPCTrainerPartyFromTrainer()` directly from field / menu code is unsafe as-is:

- It calls `ZeroEnemyPartyMons()` when `firstTrainer == TRUE`, even if the caller passes a
  temporary party buffer.
- It writes `gBattleStruct->opponentMonCanDynamax` and
  `gBattleStruct->opponentMonCanTera`, but `gBattleStruct` is allocated by
  `AllocateBattleResources()` during `CB2_InitBattle()`.
- It calls `DoTrainerPartyPool()`, which may consume `Random32()` when
  `B_POOL_SETTING_CONSISTENT_RNG == FALSE`.

The implementation should not simply call the battle init helper early.

## In-Battle Button Timing

The request also needs team view during battle. The lowest-risk first hook is the player
action menu, not the move menu.

Observed button use:

| Button / config | Existing use | Impact |
|---|---|---|
| `L_BUTTON` / `B_MOVE_DESCRIPTION_BUTTON` | Move description window. Disabled when `L=A` would conflict. | Poor default for team viewer. |
| `R_BUTTON` / `B_LAST_USED_BALL_BUTTON` | Last used ball in wild battles. `CanThrowLastUsedBall()` returns false in trainer / Frontier battles. | Best default for trainer-battle action menu only. |
| `START_BUTTON` | Action menu HP bar/text swap; move menu gimmick toggle. | Conflict unless remapped. |
| `SELECT_BUTTON` | Debug battle menu and move rearrangement. | Conflict-prone. |

MVP recommendation:

- Add `B_IN_BATTLE_TEAM_VIEWER` and `B_TEAM_VIEWER_BUTTON`.
- Default `B_TEAM_VIEWER_BUTTON` to `R_BUTTON`.
- Only open from `HandleInputChooseAction()` while waiting for a player command.
- Only allow normal trainer battle initially.
- Do not open from move selection, target selection, bag, switch party menu, link battle, or recorded battle.
- Viewer close should restore the action menu and leave the player command unchosen.

## Strength View Detail Scope

The user requested a `Y`-style strength view that can show moves and stat allocation.
This should be modeled as a detail overlay / subpage, not as always-visible row content.

Recommended detail pages:

| Page | Player side | Opponent side MVP |
|---|---|---|
| Summary | species, type, gender, level, held item, ability, nature / stat alignment | species, type, gender, level |
| Moves | current 4 moves with type/category/PP/power/accuracy where available | hidden until official behavior confirms opponent move disclosure |
| Stats | current HP / stats plus ability-point-style investment bars | hidden until official behavior confirms opponent stat disclosure |

Rationale:

- Player-owned Pokémon details are known and align with official training categories.
- Opponent full moves / ability / allocation are competitively sensitive and should not be
  revealed unless the target reference clearly does so.
- The same detail overlay can be reused pre-battle and in-battle.

## Preview Data Options

| Option | Result | Risk |
|---|---|---|
| Show raw `trainer->party` data | Easy and non-mutating. | Wrong for pool / random order / override trainers. Not acceptable as default if UI says this is the actual team. |
| Generate preview party and generate again in battle | Easy to prototype. | Preview and battle can disagree; consumes RNG twice. |
| Generate preview party once, cache it, and make battle init consume the cache | Best match to user-facing team preview. | Requires source refactor and clear RNG policy. |
| Force `B_POOL_SETTING_CONSISTENT_RNG TRUE` | Makes preview deterministic. | Global config behavior change and does not solve all state ownership concerns. |

Recommendation: generate once, cache, and consume the cache in battle init.

## Source-Wide Impact Check

| Check | Result / notes |
|---|---|
| Constants / IDs | Likely add `B_PREBATTLE_TEAM_VIEWER` config. No species/item/move ID changes. |
| Primary data table | No direct data table edits. Reads `gTrainers[]` / trainer party data. |
| Runtime entry point | `BattleSetup_StartTrainerBattle()` is the intended hook. |
| Script command / special | No new script command for MVP. Normal trainerbattle flow should pick it up through C gate. |
| Callback / task | New viewer callback / task state needed. Pre-battle path must return to field before battle start like selection MVP. In-battle path must return to battle action input. |
| Save / runtime state | EWRAM cache only. Do not add SaveBlock fields. |
| UI / window / sprite / text | New dedicated screen, windows, icon sprites, palettes, type icons or text. |
| Battle / AI | Battle init must reuse cached opponent party to keep preview and battle identical. |
| Build tools / generated files | No generated data changes for MVP. |
| Tests | Need build, check, runtime boot, normal trainer preview, selection, battle start, restore, and preview/battle identity checks. |
| Upstream migration | Touches central battle setup and battle init files. Keep feature guarded and small. |

## Open Questions

- Should the viewer freeze opponent party at encounter start for all pool trainers, even when pool RNG is normally global-random at battle init?
- Is a lightweight preview entry enough, or does battle init need a full cached `struct Pokemon` party?
- Can type icons be reused cheaply, or should MVP use type text / color chips first?
- Should the viewer expose opponent levels? Reference UI does not require all details, but levels are useful for GBA route trainers.
- Should in-battle viewer show the original 6 player party, selected temporary battle party, or both? MVP should show selected battle party plus the cached opponent party.
- Confirm whether Champions shows opponent moves / stat allocation from the selection screen, or only own-side details.
