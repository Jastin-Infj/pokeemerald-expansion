# Smart Gimmick AI Implementation

## Summary

The feature adds smart timing flags for battle gimmicks, expands Dynamax timing beyond immediate damage, adds Mega / Ultra Burst payoff checks, and adds a smart conservation layer for damaging Z-Moves.

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

For Mega / Ultra Burst, `ShouldUseSmartMega()` now looks at the target form before spending the gimmick. It allows immediate use for target-form ability payoffs such as trapping or weather control, estimated Speed flips, meaningful defensive improvement under KO pressure, and meaningful attacking-stat improvement for the selected damaging move. It still delays setup turns when there is no immediate pressure, and it can preserve `Air Lock` / `Cloud Nine` during active weather if the target form has no stronger immediate payoff.

For Z-Moves, `ShouldUseSmartZMove()` now wraps the existing Z-Move viability checks. Status Z-Moves keep their tactical checks. Damaging Z-Moves are conserved unless the AI is on its last Pokemon, the Z-Move converts the selected move into a KO, improves the damage race under immediate KO or trap pressure, or protects a low-accuracy KO line.

## Tests Added

- Conservation baseline: keeps Dynamax unused when another Dynamax user remains and the move has no immediate payoff.
- Last-Pokemon baseline: spends Dynamax when no reserve remains.
- Max Geyser payoff: spends Dynamax to set rain even with a reserve remaining.
- Max Airstream payoff: spends Dynamax in doubles for Speed-control tempo even with a reserve remaining.
- Smart Z conserve: keeps a damaging Z-Move unused when another Pokemon remains and the Z-Move has no immediate payoff.
- Smart Z last Pokemon: spends a damaging Z-Move when no reserve remains.
- Smart Z trap pressure: spends a damaging Z-Move when trapped and the Z-Move improves the damage race.
- Smart Mega Shadow Tag: spends Mega Evolution when the target form's ability creates immediate trapping pressure.
- Existing Tera, Mega, Z-Move, and combined environment tests remain in `ai_smart_gimmick.c`.

## Validation

2026-06-05 local validation:

- `rtk make -j16 -O check TESTS='AI_FLAG_GIMMICK_ENV'`: pass, 8 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_Z_MOVE'`: pass, 3 tests.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_MEGA'`: pass, 1 test.
- `rtk make -j16 -O check TESTS='AI_FLAG_SMART_TERA'`: pass, 4 tests.
- `rtk make -j16 -O check`: pass. Existing known-failing / expected-failing labels remained non-fatal.
- `rtk make -j16 -O all`: pass.
- `rtk mdbook build docs`: pass with existing warnings for missing root `CHANGELOG.md` include, `CREDITS.md` `</img>`, and large search index.
- mGBA Live: wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba` to the title screen and captured a screenshot. Stop returned `stopped:true`; CLI `status --all` returned `[]`.

## Known Gaps

- Air Lock / Cloud Nine preservation is source-derived, but not covered by a focused unit test yet because the AI test DSL does not have a direct pre-turn weather fixture in this file.
- Z-Move status tactics still rely on the existing Z-Move viability checks. More status Z-Move tactics can be modeled later.
- Tera doubles support uses the selected target and explicit candidate data. It does not fully simulate every partner threat or all possible double-target lines.
- G-Max unique secondary effects are not separately modeled in this slice. Regular Max Move effects are the current runtime focus.
