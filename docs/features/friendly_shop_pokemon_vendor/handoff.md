# Friendly Shop Pokemon Vendor Handoff

## Status

| Field | Value |
|---|---|
| Last updated | 2026-05-29 |
| Runtime branch | `integration/runtime-dev-20260529` |
| Source shelf | PR #57 / `feature/global-no-evolution-20260523` |
| Master policy | Runtime source is not eligible for `master`; use docs / Lua-only handoff content there. |

## Integration Notes

- Adopted after #47 Battle Item Restore, #48 Held Item Catalog, #54 Party /
  Status UI, and #51 Scout Selection.
- `data/scripts/debug.inc` was the only manual route conflict in this slice:
  `Script 1` opens the Pokemon Vendor, `Script 2` remains Scout Selection
  pick-6, and `Script 3` starts the vendor queued trainer-battle reward route.
- The original #57 standalone `Script 2` bond-award debug route is not present
  on the integration branch. Use `Script 3` or a temporary local script when a
  standalone `pokemonvendorawardbond` check is needed.

## Validation Evidence

| Check | Result | Notes |
|---|---|---|
| `rtk git diff --check` / `--cached --check` | Pass | No whitespace issues after merge resolution. |
| `rtk make -j16 -O check TESTS=test/pokemon_vendor.c` | Pass | Focused vendor tests pass on the integration stack. |
| `rtk make -j16 -O all` | Pass | Existing RWX linker warning only. |
| `rtk make -j16 -O debug` | Pass | Existing RWX linker warning only. |
| `rtk make -j16 -O check TESTS=test/random.c` | Pass | Random tests pass focused. |
| `rtk make -j16 -O check` | Not clean | Full hydra suite failed only `test/random.c` RandomUniform benchmark timing checks; no vendor failures were reported. |
| mGBA Live `integration-pokemon-vendor-smoke` | Pass | Opened `Scripts... -> Script 1`, confirmed vendor UI / prompt, bought Pikachu, and stopped cleanly. |

## Next Risks

- #60 All Ability Slots overlaps `include/pokemon.h`, `src/pokemon.c`,
  `src/party_menu.c`, `src/pokemon_summary_screen.c`, and `test/pokemon.c`.
- Status Editor / Unified Move Relearner should consume
  `PokemonVendor_IsEditEntitled()` from Summary-first entry points rather than
  adding more Party-menu actions.
- Gen 7 / Gen 8-style party swap delivery remains separate from this vendor
  slice; current behavior is party-or-PC fallback.
