# Battle Team Boxes Goal And Dependencies

## Status

Implemented and retained on `shelf/runtime-1.16.1/battle-team-boxes` at
`ba98fed382` (formerly `feature/battle-team-boxes-20260716`) as a stacked
extension of the Box NPC shelf now named
`shelf/runtime-1.16.1/box-npc-party-pool` at `71e9d602f0`.

On July 16, 2026, its three commits were reapplied onto the dedicated
runtime-lab target after Box NPC PR #76 merged. The staging branch is now
`archive/staging/runtime-1.16.1/battle-team-20260716`; Smart Gimmick AI is not in
this PR.

The implementation branch is intentionally separate from Smart Gimmick AI.
Box-authored opponent teams are useful to that AI, but team storage and AI move
selection have different ownership and validation risk.

## Goal

Provide a practical way to author exact NPC opponent candidates in Pokemon
Storage:

1. Keep the original Pokemon in any Box and slot.
2. Register six Box Pokemon into one of three persistent Battle Teams.
3. Allow the same source Pokemon to appear in different teams.
4. Start a debug single battle with the first or a random three of the six.
5. Start a debug double battle with the first or a random four of the six.
6. Copy and fully heal the opponent party without mutating the Box originals.
7. Preserve route metadata and selected source slots in debug output.

This replaces the need to keep every practical AI test party in generated or
hardcoded trainer data. It is also the intended Box-authored input for a later
Smart Gimmick AI integration branch.

## Reference Behavior

The design follows main-series Battle Team behavior, scaled to the requested
three teams:

- The official Sword and Shield documentation creates Battle Teams from Pokemon
  Boxes, allows up to six teams, and registers a selected Box Pokemon by moving
  it into a team slot: [Sword and Shield Battle Teams](https://www.pokemon.co.jp/ex/sword_shield/howtoplay/191108_rental.html).
- The official Sun and Moon documentation states that each team contains one to
  six Pokemon and that the same Pokemon can be registered in multiple teams:
  [Sun and Moon Battle Teams](https://www.pokemon.co.jp/ex/sun_moon/fight/161004_01.html).
- Scarlet and Violet still expose team registration state through icons on Box
  Pokemon in Battle Team mode:
  [Scarlet and Violet 1.2.0 notes](https://www.pokemon.co.jp/info/2023/02/230217_gm01.html).
- PKHeX's Gen 8 save implementation was used as a non-authoritative storage
  cross-check. It represents Battle Team members as Box and slot indexes rather
  than copied Pokemon records:
  [TeamIndexes8.cs](https://github.com/kwsch/PKHeX/blob/master/PKHeX.Core/Saves/Substructures/Gen8/SWSH/TeamIndexes8.cs).

## Chosen Contract

### Team storage

- Exactly three Battle Teams.
- Exactly six ordered slots per team.
- A slot stores `{boxId, boxPosition}`, not a `struct BoxPokemon` copy.
- A source may appear in multiple teams.
- Registering the same source twice in one team moves its registration to the
  newly selected team slot.
- Empty slots, Eggs, Bad Eggs, and out-of-range references are invalid.
- If an external system removes a source, the stale registration is cleared the
  next time it is queried.

### Storage editing

- Registered Box sources remain live. Move, item, marking, form, and other data
  reads use the current Box Pokemon.
- Pokemon Storage blocks operations that change a registered source's Box
  location: move, shift, withdraw, release, and multi-move pickup.
- PC-selection types that transfer a source out of storage, such as daycare and
  trade selection, also block registered sources. Non-destructive selection
  types remain available.
- Summary, markings, and held-item editing remain available.
- The source remains locked until it is removed from every Battle Team that
  references it.

### Debug battles

- A registered Battle Team must have all six valid members before battle.
- Singles copy three members; doubles copy four.
- First-N mode is deterministic and preserves team order.
- Random-N mode samples from the six without replacement.
- Enemy copies start fully healed.
- Source HP loss, status, PP, and held items are not changed.
- Existing legacy Box 1 pool routes remain available under a separate submenu.
- The current strong `master` debug AI flag preset remains configurable through
  `BoxNpcPartyPoolConfig`.

## Dependency Survey

| Dependency | Owned symbols | Contract |
| --- | --- | --- |
| Save extension | `struct SaveBlock3`, save chunks in `src/save.c` | Registry is appended to SaveBlock3. Size changes from 4 to 84 bytes and remains below the 1,624-byte limit. |
| Pokemon Storage | `gPokemonStoragePtr`, `GetBoxMonDataAt`, `SetBoxMonAt`, `CheckBoxMonSanityAt` | References are valid only for an existing non-Egg, non-Bad-Egg Box Pokemon. |
| Storage UI | `src/pokemon_storage_system.c` | Adds the team manager, Box-only selector, and registered-source movement locks. |
| PC selection | `src/chooseboxmon.c`, `SELECT_PC_MON_*` | Marks destructive selection types and keeps the Lotad/Seedot size check on a distinct non-destructive type. |
| Box NPC source | `BoxNpcPartyPoolConfig`, `BoxNpcPartyPoolResult` | Candidate sources must carry both Box ID and Box position. |
| Battle copy | `BoxMonAtToMon`, `HealPokemon`, `ZeroEnemyPartyMons` | Opponent party copies are temporary and contiguous from slot zero. |
| Debug battle route | `src/debug.c`, `BattleSetup_StartTrainerBattle_Debug` | Adds Team 1-3 single/double first/random routes without removing legacy routes. |
| Gimmick policy | `BoxNpcPartyPool_ApplyPendingBattleInitPolicy` | Existing pending Tera/Dynamax policy applies equally to registered-team copies. |
| Randomness | `RNG_BOX_NPC_PARTY_POOL_BATTLE_MEMBER` | Existing final-member sampling tag is reused, avoiding a second preview roll. |
| Save tests | `test/save.c` | Intentional SaveBlock3 growth must update the compatibility baseline to 84. |
| Feature tests | `test/battle_team.c` | Covers references, uniqueness, stale cleanup, Box preservation, and NPC copy behavior. |

## Save Compatibility

The registry begins with a magic value and version. Existing saves contain no
valid registry header, so the first registry access initializes all 18 slots to
the empty sentinel. Pokemon Storage itself is not resized, avoiding changes to
its nine-sector checksum layout.

Changing the feature schema later requires a versioned migration. Reordering or
silently reinterpreting the saved fields is not acceptable.

## Branch And Integration Result

- Closed PR #75 preserves the standalone Battle Team delta against the Box NPC
  shelf. Neither standalone runtime PR was merged into `master`.
- Box NPC and Battle Team were reapplied separately through PRs #76 and #77.
- Smart Gimmick AI remained separate and entered through PR #78.
- The combined result is frozen at
  `snapshot/runtime-1.16.1/battle-lab-20260716`; the standalone source remains
  `shelf/runtime-1.16.1/battle-team-boxes`.
- Pokemon State Editor, Unified Move Relearner, and battle item restore remain
  independent shelves. They may be combined later in a debug battle lab, but
  they are not prerequisites for this team registry.

## Out Of Scope

- Rental-team sharing or import codes.
- Team names and icons.
- More than three teams.
- Partial registered-team battles with fewer than six candidates.
- Automatic role-based lead selection.
- Depositing battle changes back into Box sources.
- Smart Gimmick AI decision changes.
