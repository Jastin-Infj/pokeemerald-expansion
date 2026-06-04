# Smart Gimmick AI Implementation

## Summary

The feature adds smart timing flags for battle gimmicks and expands Dynamax timing beyond immediate damage.

Runtime files:

- `include/constants/battle_ai.h`
- `src/battle_ai_util.c`
- `test/battle/ai/ai_smart_gimmick.c`

Documentation:

- `docs/tutorials/ai_flags.md`
- `docs/features/smart_gimmick_ai/`

## Runtime Behavior

`DecideGimmickBeforeMoveSelection()` still handles the early smart-gimmick gate. `ReconsiderSmartGimmick()` then checks the chosen move and cancels the gimmick when the selected turn is not worth spending.

For Dynamax, `ShouldUseSmartDynamax()` now accepts these reasons:

- Last available Pokemon.
- The current target can otherwise KO the AI Pokemon.
- Dynamax converts the selected move into a KO that the regular move would miss.
- The selected Max Move creates strategic board value:
  - Speed control through `Max Airstream` or `Max Strike`.
  - Weather control through Fire, Water, Rock, or Ice Max Moves.
  - Terrain control through Electric, Grass, Fairy, or Psychic Max Moves.
  - Side-wide stat boosts or drops from Fighting, Poison, Ground, Steel, Dragon, Bug, Ghost, and Dark Max Moves.

Weather and terrain decisions call `ShouldSetWeather()`, `ShouldSetFieldStatus()`, and `ShouldClearFieldStatus()` so smart Dynamax follows the same field-state opinions used by normal AI move scoring.

## Tests Added

- Conservation baseline: keeps Dynamax unused when another Dynamax user remains and the move has no immediate payoff.
- Last-Pokemon baseline: spends Dynamax when no reserve remains.
- Max Geyser payoff: spends Dynamax to set rain even with a reserve remaining.
- Max Airstream payoff: spends Dynamax in doubles for Speed-control tempo even with a reserve remaining.
- Existing Tera, Mega, Z-Move, and combined environment tests remain in `ai_smart_gimmick.c`.

## Validation

2026-06-05 local validation:

- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 8 tests.
- `rtk make -j16 -O check`: pass. Existing known-failing / expected-failing labels remained non-fatal.
- `rtk make -j16 -O all`: pass.
- `rtk mdbook build docs`: pass with existing warnings for missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, and large search index.
- mGBA Live: direct script-build binary failed without `DISPLAY`; wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen. Stop returned `stopped:true`; CLI `status --all` returned `[]`.

## Known Gaps

- Mega Evolution still only delays selected setup turns when there is no immediate KO pressure. Mega ability weather / terrain re-control needs a separate heuristic.
- Z-Move timing still relies on the existing Z-Move viability checks. More status Z-Move tactics can be modeled later.
- Tera doubles support uses the selected target and explicit candidate data. It does not fully simulate every partner threat or all possible double-target lines.
- G-Max unique secondary effects are not separately modeled in this slice. Regular Max Move effects are the current runtime focus.
