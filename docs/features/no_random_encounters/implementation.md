# No Random Encounters Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Branch | `feature/no-random-encounters-step-only-runtime-20260517` |
| Base | `master` `788191a7cd`; `git describe` = `expansion/1.15.2-65-g788191a7cd` |
| Scope | Step-based random encounter suppression via existing overworld flag gate |

## Implementation Summary

The referenced implementation branch re-applies the previously validated
runtime slice on top of current `master` without merging the old feature branch.
The 2026-05-17 runtime branch keeps the source scope to the same three files
and records fresh local build plus mGBA evidence.

Changed files:

| File | Change |
|---|---|
| `include/constants/flags.h` | Renames `FLAG_UNUSED_0x8E5` to `FLAG_NO_ENCOUNTER` in the SYSTEM flag region. |
| `include/constants/flags_frlg.h` | Mirrors the macro name for FRLG compatibility, preserving the existing `0` value. |
| `include/config/overworld.h` | Assigns `OW_FLAG_NO_ENCOUNTER` to `FLAG_NO_ENCOUNTER`. |

No new encounter hook was added. The feature uses the existing
`CheckStandardWildEncounter` gate in `src/field_control_avatar.c` and the
existing Debug menu toggle in `src/debug.c`.

## Difference From Earlier Branches

Runtime behavior is unchanged from the earlier `feature/no-random-encounters`
and `feature/no-random-encounters-step-only` slices: all three branches only
connect the existing `OW_FLAG_NO_ENCOUNTER` gate to a real flag. There is no
encounter-rate optimization, RNG change, wild table rewrite, or new encounter
hook in this feature.

The 2026-05-17 branch differs operationally:

- it starts from current `master` instead of merging an older feature branch;
- it reapplies only the three runtime files listed above;
- it records fresh `all` / `debug` / `check` local builds;
- it adds an OFF / ON / OFF-restored mGBA Route 101 runtime pass;
- it updates docs and the validation matrix for the current PR handoff.

Adoption dependency notes:

- `OW_FLAG_NO_ENCOUNTER` must be assigned to `FLAG_NO_ENCOUNTER`, not `TRUE`
  or `1`.
- `FLAG_NO_ENCOUNTER` should remain an event flag alias, currently planned as
  `SYSTEM_FLAGS + 0x85`.
- `include/constants/flags_frlg.h` mirrors the macro name as `0` for FRLG
  compatibility; this does not enable the Emerald runtime flag there.
- `src/data/wild_encounters.json`, Fishing, Sweet Scent, Rock Smash, and
  scripted wild battle paths are out of this implementation slice.

## Behavior

When `FLAG_NO_ENCOUNTER` is clear, Route 101 grass still starts normal wild
battles. When the flag is set, standard step-based wild encounters are blocked
before `StandardWildEncounter` starts a battle.

The MVP intentionally does not suppress Fishing, Sweet Scent, Rock Smash, or
scripted `setwildbattle` / `dowildbattle` paths.

## Validation

Local validation on `feature/no-random-encounters-step-only-runtime-20260517`
on 2026-05-17:

| Check | Result |
|---|---|
| `rtk git diff --check` | Passed. |
| `rtk make -j16 -O all` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O debug` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O check` | Passed with existing RWX linker warning. |
| mGBA Live OFF check | Passed. `FLAG_NO_ENCOUNTER=false`, `repel=0`, Route 101 grass movement started a wild Wurmple battle after 163 macro frames. |
| mGBA Live ON check | Passed. `FLAG_NO_ENCOUNTER=true`, `repel=0`, Route 101 grass movement for 2400 macro frames stayed on `CB2_Overworld`. |
| mGBA Live OFF-restored check | Passed. Clearing `FLAG_NO_ENCOUNTER` restored a Route 101 wild battle after 394 macro frames. |
| mGBA Live cleanup | Passed after retry. CLI `stop` initially left `mgba-qt` defunct / `alive_after:true`; MCP `mgba_live_stop` cleared the stale session and final CLI `status --all` returned `[]`. |

Screenshots:

- `/tmp/no-random-encounters-20260517/off-wild-battle.png`
- `/tmp/no-random-encounters-20260517/on-after-2400frames.png`
- `/tmp/no-random-encounters-20260517/off-restored-wild-battle.png`

Runtime setup notes:

- ROM copy: `/tmp/no-random-encounters-20260517/no-random.gba`
- Save copy: `/tmp/no-random-encounters-20260517/no-random.sav`, copied from ignored local `pokeemerald.sav`
- Session: `codex-no-random-20260517`
- Route: Debug menu warp to `MAP_ROUTE101` (`map_group=0`, `map_num=16`)
- Symbol anchors: `gSaveBlock1Ptr=0x03005208`, `gMain=0x03006704`, `FLAG_NO_ENCOUNTER=0x8E5`, `SaveBlock1.flags=save1+0x1270`
- `gBattleTypeFlags` retained `0x4` after returning from the OFF battle; use callback / screen / flag state as the runtime oracle.

Docs-only review on 2026-05-17:

| Check | Result |
|---|---|
| `rtk git status --short --branch` | Clean `master` before the adoption branch was created. |
| `rtk git describe --tags --always --dirty` | `expansion/1.15.2-56-gc8b8e57183`. |
| `rtk gh pr list --state open ...` | Open runtime shelves were rechecked; no random encounters has no open PR yet. |
| Source scope review | Historical branch diff is still limited to `include/config/overworld.h`, `include/constants/flags.h`, and `include/constants/flags_frlg.h`. |
| Runtime build / mGBA | Superseded by the 2026-05-17 runtime branch validation above. |

## Remaining Risks

- Surf / cave step encounters were not separately walked in this fresh branch
  validation. The same existing `CheckStandardWildEncounter` gate covers them,
  but the focused runtime evidence is Route 101 land grass.
- Fishing, Sweet Scent, Rock Smash, static wild battles, DexNav, and option UI
  are out of MVP scope and should be handled as separate follow-up work.

## Merge Handoff

The referenced branch is a runtime implementation branch. Do not merge it into
`master` as a docs-only update. Keep the source diff limited to the three
runtime files above plus these docs, and use local build / mGBA evidence for
handoff instead of waiting on long Actions runs. If this branch is PR'd, target
the selected feature / integration base; merging runtime source into `master`
should remain an explicit policy decision, not an accidental docs merge.
