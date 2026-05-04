# How to use config flags and vars

This tutorial explains the pattern used by configs such as `B_FLAG_TERA_ORB_CHARGED`, `B_FLAG_DYNAMAX_BATTLE`, `B_VAR_NO_BAG_USE`, and `I_EXP_SHARE_FLAG`.

## Config Value Types

Expansion configs are not all the same kind of value.

| Type | Example | Meaning |
|---|---|---|
| Compile-time toggle | `#define B_SLEEP_CLAUSE FALSE` | Built into the ROM. Change the define and rebuild. |
| Generational config | `#define B_CHARGE GEN_LATEST` | Built into the ROM, but compared against `GEN_*` values. |
| Runtime flag slot | `#define B_FLAG_TERA_ORB_CHARGED 0` | `0` means disabled. Replace `0` with a real `FLAG_*` constant, then set/clear that flag at runtime. |
| Runtime var slot | `#define B_VAR_NO_BAG_USE 0` | `0` means disabled. Replace `0` with a real `VAR_*` constant, then store a numeric mode at runtime. |

Do not put `TRUE` into a runtime flag slot. A runtime flag config expects a flag ID, not a boolean. `TRUE` is just `1`, so it would point at flag number 1 instead of enabling the feature in a meaningful way.

## When to use a flag

Use a flag when the feature is binary: on or off.

Good flag examples:

- Tera Orb is charged or not charged.
- Dynamax is enabled for the current battle or not.
- No catching is active or not.
- Sleep Clause is enabled by script or not.

Typical setup:

1. Choose an unused flag in `include/constants/flags.h`.
2. Give it a descriptive name.
3. Assign that flag to the config in `include/config/*.h`.
4. Set or clear the flag from script or C.

Example:

```c
// include/constants/flags.h
#define FLAG_TERA_ORB_CHARGED  FLAG_UNUSED_0x264

// include/config/battle.h
#define B_FLAG_TERA_ORB_CHARGED FLAG_TERA_ORB_CHARGED
```

Script can then control it:

```asm
setflag FLAG_TERA_ORB_CHARGED
clearflag FLAG_TERA_ORB_CHARGED
goto_if_set FLAG_TERA_ORB_CHARGED, SomeScript_Label
```

C code usually checks whether the config has been assigned before reading it:

```c
if (B_FLAG_TERA_ORB_CHARGED != 0 && FlagGet(B_FLAG_TERA_ORB_CHARGED))
{
    // Charged behavior.
}
```

## When to use a var

Use a var when the feature has more than two modes.

Good var examples:

- Bag use mode: unrestricted, disabled against trainers, disabled in all battles.
- Difficulty mode.
- A temporary battle setup value.
- A selected map/event mode.

Typical setup:

1. Choose an unused var in `include/constants/vars.h`.
2. Give it a descriptive name.
3. Assign that var to the config in `include/config/*.h`.
4. Store a numeric mode with `setvar` or `VarSet`.

Example:

```c
// include/constants/vars.h
#define VAR_BATTLE_NO_BAG_MODE VAR_UNUSED_0x404E

// include/config/battle.h
#define B_VAR_NO_BAG_USE VAR_BATTLE_NO_BAG_MODE
```

Script:

```asm
setvar VAR_BATTLE_NO_BAG_MODE, NO_BAG_AGAINST_TRAINER
setvar VAR_BATTLE_NO_BAG_MODE, NO_BAG_RESTRICTION
```

C:

```c
if (B_VAR_NO_BAG_USE != 0 && VarGet(B_VAR_NO_BAG_USE) == NO_BAG_IN_BATTLE)
{
    // Disable bag use.
}
```

## Tera Orb charge example

Relevant files:

| File | Role |
|---|---|
| `include/config/battle.h` | `B_FLAG_TERA_ORB_CHARGED`, `B_FLAG_TERA_ORB_NO_COST` config slots. |
| `src/script_pokemon_util.c` | `HealPlayerParty` sets `B_FLAG_TERA_ORB_CHARGED` if assigned and the player has `ITEM_TERA_ORB`. |
| `src/battle_terastal.c` | `CanTerastallize` checks the charged flag. `ActivateTera` clears it after use unless no-cost is active. |

The current default is:

```c
#define B_FLAG_TERA_ORB_CHARGED 0
#define B_FLAG_TERA_ORB_NO_COST 0
```

That means the runtime charge system is not assigned to real save flags yet. To enable it, define real flags and assign them to these configs. Use a second flag for no-cost mode only if the game has an event that permanently or temporarily removes the charge cost.

## Dynamax battle flag example

Relevant files:

| File | Role |
|---|---|
| `include/config/battle.h` | `B_FLAG_DYNAMAX_BATTLE` config slot. |
| `src/battle_dynamax.c` | `CanDynamax` requires the player to have `ITEM_DYNAMAX_BAND` and the assigned flag to be set. |

The current default is:

```c
#define B_FLAG_DYNAMAX_BATTLE 0
```

With this default, normal player-side Dynamax is not enabled through a runtime flag. Assign a real flag if scripts should allow Dynamax only in selected battles or locations.

## Checklist

- Do not use `TRUE` or `FALSE` for `B_FLAG_*`, `I_*_FLAG`, or `B_VAR_*` slots unless the config comment explicitly says it is a boolean.
- Keep `0` when the feature should be compiled but inactive.
- Use `FLAG_*` for binary state and `VAR_*` for modes.
- Prefer descriptive constants over raw numbers in scripts.
- For temporary battle setup vars, clear them after the battle or before saving if the config comment says they should not persist.
- For FRLG world map behavior, use the FRLG-specific world map flags and `setworldmapflag` where required. `setflag` and `setworldmapflag` are not equivalent.

## Rule option examples

If a feature is already designed as a runtime flag / var, prefer assigning that slot instead of adding a new SaveBlock field.

| Desired option | Config slot | Use |
|---|---|---|
| Difficulty Easy / Normal / Hard | `B_VAR_DIFFICULTY` | Store `DIFFICULTY_EASY`, `DIFFICULTY_NORMAL`, or `DIFFICULTY_HARD` in the assigned var. |
| No catching | `B_FLAG_NO_CATCHING` | Set the assigned flag while the rule is active. |
| No running | `B_FLAG_NO_RUNNING` | Set the assigned flag while wild escape is disallowed. |
| Sleep Clause | `B_FLAG_SLEEP_CLAUSE` | Use a flag when the clause is optional; use `B_SLEEP_CLAUSE TRUE` only for always-on ROM behavior. |
| Dynamax allowed | `B_FLAG_DYNAMAX_BATTLE` | Player Dynamax also requires `ITEM_DYNAMAX_BAND`. |
| Tera Orb charged | `B_FLAG_TERA_ORB_CHARGED` | `HealPlayerParty` can recharge it once configured. |
| Tera no cost | `B_FLAG_TERA_ORB_NO_COST` | Prevents Tera Orb charge from being consumed. |
| Force shiny | `P_FLAG_FORCE_SHINY` | Forces wild / gift Pokemon shiny while the flag is set. |
| Force no shiny | `P_FLAG_FORCE_NO_SHINY` | Blocks wild / gift shinies while the flag is set. |
| Gen 6 Exp. Share | `I_EXP_SHARE_FLAG` | Required if `I_EXP_SHARE_ITEM` is Gen 6+ behavior. |

For rule families that do not already have a runtime slot, such as Nuzlocke area tracking, release-on-loss, catch rate multiplier, shiny reroll multiplier, Mega global disable, Z-Move global disable, and trade restrictions, add a dedicated rule helper first. Do not overload an unrelated flag just because it is available.
