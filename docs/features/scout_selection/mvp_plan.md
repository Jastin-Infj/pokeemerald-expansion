# Scout Selection MVP Plan

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `e927b612b3`; `feature/scout-selection-runtime-20260520` |
| Code status | Runtime MVP implemented on feature branch |
| Provenance | Local source inspection and feature branch handoff docs |

## MVP

Create a reusable scout selection runtime:

1. Script sets pool id, sample count, and pick count.
2. C builds candidate Pokemon into an EWRAM buffer.
3. Dedicated CB2 screen shows Pokemon icons and compact labels.
4. Player can move cursor, open Summary, toggle selected candidates, and confirm.
5. Confirm gives the selected Pokemon to party / PC through the existing gift path.
6. Script receives result code and owns messages, flags, and NPC state.

## Proposed Runtime Contract

```asm
ScoutNpc_EventScript::
    lock
    faceplayer
    msgbox ScoutNpc_Text_Offer, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, NO, ScoutNpc_EventScript_End
    setvar VAR_0x8004, SCOUT_POOL_PARTYGEN_DEMO
    setvar VAR_0x8005, 12     @ candidate count
    setvar VAR_0x8006, 1      @ pick count
    special InitScoutSelection
    goto_if_eq VAR_RESULT, FALSE, ScoutNpc_EventScript_End
    special OpenScoutSelection
    waitstate
    goto_if_eq VAR_RESULT, SCOUT_RESULT_CANCEL, ScoutNpc_EventScript_End
    special GiveSelectedScoutMons
    goto_if_eq VAR_RESULT, MON_CANT_GIVE, ScoutNpc_EventScript_NoSpace
    setflag FLAG_SCOUT_STARTER_TEST_CLAIMED
ScoutNpc_EventScript_End::
    release
    end
```

`VAR_0x8004` / `VAR_0x8005` / `VAR_0x8006` are only the MVP proposal. If later
features already reserve these for another flow, define wrapper macros or a small
`scoutselection pool, sample, picks` macro in `asm/macros/event.inc`.

## Candidate Data Shape

Initial C-side spec:

```c
struct ScoutMonSpec
{
    u16 species;
    u8 level;
    enum Item item;
    enum PokeBall ball;
    u8 nature;
    enum Ability ability;
    u8 gender;
    enum Move moves[MAX_MON_MOVES];
    u8 ivs[NUM_STATS];
    u8 evs[NUM_STATS];
};
```

The implemented pool specs are generated from partygen set JSON by
`tools/scout_selection/make_scout_pools.py`. Defaults are allowed for missing
JSON fields:

| Field | Default |
|---|---|
| `item` | `ITEM_NONE` |
| `ball` | `BALL_POKE` |
| `nature` | `NATURE_RANDOM` or gift-compatible policy decided in implementation |
| `ability` | default by personality when `ABILITY_NONE` or no matching slot |
| `gender` | random |
| `moves` | `MOVE_DEFAULT` if no JSON moves; shorter explicit move lists are padded with `MOVE_NONE` |
| `ivs` | `31/31/31/31/31/31` in partygen order |
| `evs` | `0/0/0/0/0/0` in partygen order |

Use a pool table rather than many script vars for full candidate data. Script vars should
select the pool and counts; the table should own species / moves / items.
For generated pools, partygen stat order is HP / Atk / Def / SpA / SpD / Spe and
runtime MON_DATA order is HP / Atk / Def / Spe / SpA / SpD; the generator owns
that conversion.

## UI Shape

Recommended first UI:

| Area | MVP behavior |
|---|---|
| Layout | Single vertical list or 2-column compact list. Prefer vertical if scroll is needed. |
| Visible count | Up to 6 visible candidates. |
| Pool count | Implemented MVP supports 12 candidates. This covers the user-mentioned 11-candidate reference and requested 12-candidate scroll case. |
| Sprite | Pokemon icon via `CreateMonIconIsEgg`. |
| Summary | `SELECT` opens standard Summary in `SUMMARY_MODE_LOCK_MOVES`; return preserves cursor and selected order. |
| Selection | `A` toggles. `START` confirms only when selected count equals pick count. |
| Highlight | Cursor fill plus selected marker/order number. Follow Team Viewer selected marker style. |
| Cancel | `B` cancels before confirm unless caller marks the selection mandatory. |

## Implementation Steps

| Step | Files | Notes |
|---|---|---|
| 1 | `include/scout_selection.h`, `src/scout_selection.c` | New module, static EWRAM state, candidate buffer, public specials. |
| 2 | `data/specials.inc` | Register `InitScoutSelection`, `OpenScoutSelection`, `GiveSelectedScoutMons`. |
| 3 | `tools/champions_partygen/catalog/sets/*.json`, `tools/scout_selection/make_scout_pools.py`, `src/data/scout_selection_pools.h` | Generate one debug/test pool with 12 unique partygen-derived candidates; generated header is ignored. |
| 4 | `src/scout_selection.c` | Build candidate `struct Pokemon` array from specs. |
| 5 | `src/scout_selection.c` | Implement CB2 init, windows, icon sprites, cursor, scroll, selected marker. |
| 6 | `src/scout_selection.c` | Use standard `ShowPokemonSummaryScreen()` for MVP; direct skills-page entry remains future work. |
| 7 | `src/scout_selection.c` | Summary open/return path, icon/window cleanup, selection state persistence. |
| 8 | `src/scout_selection.c` | Give selected built mons through `GiveScriptedMonToPlayer` semantics. |
| 9 | `data/scripts/debug.inc` or a safe test map | Add debug-only scout route. Avoid story maps for first validation. |
| 10 | docs | Update this folder with implementation summary, validation evidence, and remaining risks. |

## Branching

Runtime implementation should start from current `master` on a dedicated branch, for example:

```text
feature/scout-selection-runtime-20260519
```

Do not merge runtime source into `master`. If docs updates are needed on `master`, make a
separate docs-only branch and cherry-pick / re-apply only Markdown changes.

## Non-Goals

- Do not replace the main Route 101 starter battle in the first runtime slice.
- Do not implement VP / tickets / 22-hour refresh.
- Do not add SaveBlock state.
- Do not copy Battle Factory rental party into `gSaveBlock2Ptr->frontier`.
- Do not use front battle sprites until icon MVP is validated.

## Future Work

- Starter replacement mode: sample 6 from configured starter pool, pick 1, then continue
  existing starter battle / lab flow.
- Champions Challenge integration: use scout selection as the 0-to-6 party builder.
- Trial scout / temporary ownership: mark received Pokemon for later cleanup.
- Daily / timed refresh: RTC / save-state design.
- Additional partygen-derived scout pools and generated validation beyond the
  current demo pool.
- Full front-sprite presentation for selected candidate only, if VRAM/OAM budget allows.

## Open Questions

- Should selected Pokemon be generated at screen open or only at confirm?
- If party and PC are full, should `GiveSelectedScoutMons` fail atomically for all picks?
- Should later pools expose exact IV / EV / Tera policy fields, or keep MVP fields only?
