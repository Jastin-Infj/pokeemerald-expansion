# Pokemon State Editor Dependencies

## Runtime Entry

| Dependency | Notes |
|---|---|
| `src/pokemon_summary_screen.c` | Owns Summary input, page state, windows, and return callbacks. The editor should live here for the first slice. |
| `include/config/summary_screen.h` | Best place for `P_SUMMARY_SCREEN_STATE_EDITOR` and user-tunable editor X/Y/size defines. |
| `include/pokemon_summary_screen.h` | No public API is expected for MVP unless another file needs to open the editor. |
| `src/party_menu.c` | Existing party action already opens Summary. No party menu action should be added for MVP. |

## Pokemon Data

| Field | Access | Policy |
|---|---|---|
| EVs | `MON_DATA_HP_EV` through `MON_DATA_SPDEF_EV` | Clamp each stat to `MAX_PER_STAT_EVS` and total to `MAX_TOTAL_EVS`. Recalculate stats after edits. |
| IVs | `MON_DATA_HP_IV` through `MON_DATA_SPDEF_IV` | Clamp 0..`MAX_PER_STAT_IVS`. Recalculate stats after edits. |
| Nature | `MON_DATA_HIDDEN_NATURE` | Use hidden nature so the Summary/stat nature changes without raw personality churn. |
| Ability | `MON_DATA_ABILITY_NUM` | Show only slots whose `GetSpeciesAbility(species, slot) != ABILITY_NONE`. |
| Level | `MON_DATA_EXP`, `MON_DATA_LEVEL`, `gExperienceTables`, `GetCurrentLevelCap` | Set EXP to the exact target level and clamp to the configured level-cap policy. Recalculate stats. |
| Pokeball | `MON_DATA_POKEBALL`, `gPokeBalls`, `POKEBALL_COUNT` | Cycle displayable ball ids and show the matching item name. |
| Dynamax level | `MON_DATA_DYNAMAX_LEVEL`, `MAX_DYNAMAX_LEVEL` | Clamp 0..max and recalculate stats because HP can depend on Dynamax level. |
| Tera type | `MON_DATA_TERA_TYPE`, `gTypesInfo`, species `forceTeraType` | Cycle editable non-special types. Lock species that force a Tera type. |
| Gigantamax factor | `MON_DATA_GIGANTAMAX_FACTOR` | Toggle raw on/off flag. Species-specific legality remains a follow-up policy decision. |
| Gender | `SetMonPersonality`, `MON_DATA_PERSONALITY` | Change low byte to match `GetGenderFromSpeciesAndPersonality`, then preserve shiny/hidden nature modifiers. Reject fixed/genderless species. |
| Friendship | `MON_DATA_FRIENDSHIP` | Clamp 0..`MAX_FRIENDSHIP`. |

## Rendering / Input

| Dependency | Notes |
|---|---|
| `AddWindow`, `PrintTextOnWindow`, `FillWindowPixelRect`, dedicated BG palette | Enough for a compact overlay with header/footer bands, row striping, selected-row highlight, and readable text without changing Summary page tilemaps or using the white standard frame. |
| `JOY_REPEAT` | Needed for smooth held D-pad increment/decrement. |
| `A_BUTTON` / `B_BUTTON` | Page advance and close. |
| `L_BUTTON` / `R_BUTTON` | Quick min/max for numeric rows and first/last for cyclic rows. |

## External References

The prior local docs suspected an existing Uroxido/state editor implementation, but no
local branch/file with that editor has been identified in this repository. A web
reference exists for a Pokemon Emerald Legacy Enhanced stat editor, but this branch
should not import code from it blindly. The implementation should be local and adapted
to this repo's Summary Screen.

Known local reference:

- `src/debug.c` has existing debug-only Set Hidden Nature, Set Ability, Set Friendship,
  and Give Pokemon EV/IV selection flows.

Known external reference:

- <https://github.com/Exclsior/Pokemon_Emerald_Legacy_Enhanced>
