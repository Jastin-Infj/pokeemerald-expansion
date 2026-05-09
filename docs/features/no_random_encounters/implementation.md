# No Random Encounters Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-09 |
| Branch | `feature/no-random-encounters-step-only` |
| Base | `master` `5591163a09` |
| Scope | Step-based random encounter suppression via existing overworld flag gate |

## Implementation Summary

The referenced implementation branch re-applies the previously validated
runtime slice on top of current `master` without merging the old feature branch.

Changed files:

| File | Change |
|---|---|
| `include/constants/flags.h` | Renames `FLAG_UNUSED_0x8E5` to `FLAG_NO_ENCOUNTER` in the SYSTEM flag region. |
| `include/constants/flags_frlg.h` | Mirrors the macro name for FRLG compatibility, preserving the existing `0` value. |
| `include/config/overworld.h` | Assigns `OW_FLAG_NO_ENCOUNTER` to `FLAG_NO_ENCOUNTER`. |

No new encounter hook was added. The feature uses the existing
`CheckStandardWildEncounter` gate in `src/field_control_avatar.c` and the
existing Debug menu toggle in `src/debug.c`.

## Behavior

When `FLAG_NO_ENCOUNTER` is clear, Route 101 grass still starts normal wild
battles. When the flag is set, standard step-based wild encounters are blocked
before `StandardWildEncounter` starts a battle.

The MVP intentionally does not suppress Fishing, Sweet Scent, Rock Smash, or
scripted `setwildbattle` / `dowildbattle` paths.

## Validation

Local validation on 2026-05-09:

| Check | Result |
|---|---|
| `rtk git diff --check` | Passed. |
| `rtk make -j16 -O debug` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O all` | Passed with existing RWX linker warning. |
| `rtk make -j16 -O check` | Passed with existing RWX linker warning. |
| mGBA Live OFF check | Passed. `FLAG_NO_ENCOUNTER=false`, `repel=0`, Route 101 grass movement started a wild Wurmple battle. |
| mGBA Live ON check | Passed. `FLAG_NO_ENCOUNTER=true`, `repel=0`, Route 101 grass movement for 2400 macro frames stayed on `CB2_Overworld`. |
| mGBA Live cleanup | Passed. Both sessions stopped and CLI `status --all` returned `[]`. |

Screenshots:

- `/tmp/mgba-noencounter-fresh-off-wild-battle-20260509.png`
- `/tmp/mgba-noencounter-fresh-on-after-2400frames-20260509.png`

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
