# Friendly Shop Pokemon Vendor Implementation

## Document Metadata

| Field | Value |
|---|---|
| Last updated | 2026-05-23 |
| Branch | `feature/global-no-evolution-20260523` |
| Scope | First runtime slice: global no-evolution / fixed species rule |

## Summary

The first implementation slice does not add the Pokemon vendor UI yet. It lands
the fixed-species runtime rule that the vendor depends on:

- `P_EVOLUTIONS_ENABLED` is disabled in `include/config/pokemon.h`.
- `AreRuntimeEvolutionsDisabled()` and `CanPokemonEvolveInThisRuntime()` expose
  the runtime policy from `include/pokemon.h` / `src/pokemon.c`.
- `GetEvolutionTargetSpecies()` returns `SPECIES_NONE` before resolving any
  evolution method while evolution is disabled.
- `IsMonPastEvolutionLevel()` returns `FALSE` while evolution is disabled, so
  fixed species do not receive unevolved EXP multiplier behavior.
- `test/pokemon.c` covers level, item-check, item-use, trade, and
  past-evolution-level behavior under the global no-evolution rule.

This intentionally blocks ordinary Pokemon evolution for the whole runtime, not
only vendor sealed-origin Pokemon. Mega Evolution, Dynamax, Tera, and other
battle form-change gimmicks are separate systems and are not changed by this
slice.

## Changed Files

| File | Change |
|---|---|
| `include/config/pokemon.h` | Adds `P_EVOLUTIONS_ENABLED FALSE`. |
| `include/pokemon.h` | Declares the no-evolution helper functions. |
| `src/pokemon.c` | Adds helper implementations and gates evolution target lookup / past-level checks. |
| `test/pokemon.c` | Adds focused no-evolution coverage. |

## Behavior

When `P_EVOLUTIONS_ENABLED` is `FALSE`:

- level-up evolution returns no target;
- Rare Candy / EXP Candy cannot trigger evolution;
- evolution stones and other evolution items return no target;
- trade / link evolution returns no target;
- script-trigger, overworld-special, battle-only, and battle-end evolution
  modes return no target through the same resolver;
- `IsMonPastEvolutionLevel()` is false, avoiding fixed-species EXP-side
  effects.

Item-use UX currently falls back to the existing `It won't have any effect.`
message. A custom fixed-species message can be added later if playtesting shows
the generic item message is unclear.

## Remaining Work

- Pokemon vendor product table and script-facing entry point.
- Normal Pokemon delivery finalizer that does not route through `AddBagItem()`.
- Sealed recruit creation / origin marker / lock state.
- Bond / seal EXP progress storage and unlock flow.
- Summary locked / ready / sealed-origin labels.
- Status Editor / relearner entitlement for released sealed-origin Pokemon.
- PC / daycare / trade blocking for locked recruits.

## Validation

| Date | Check | Result | Notes |
|---|---|---|---|
| 2026-05-23 | `rtk git diff --check` | Pass | No whitespace issues before build. |
| 2026-05-23 | `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only. |
| 2026-05-23 | `rtk make -j16 -O check` | Pass | Existing RWX linker warning only; suite exits 0. |
| 2026-05-23 | `rtk mdbook build docs` | Pass | Existing warnings: missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, large search index. |
| 2026-05-23 | mGBA Live boot | Pass | Direct MCP start failed because `DISPLAY` was unset. Retried through `mgba-live-cli` with `DISPLAY=:0`, captured the Game Freak boot screen, then stopped the session cleanly. |
