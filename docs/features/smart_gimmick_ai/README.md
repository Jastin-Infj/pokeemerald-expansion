# Smart Gimmick AI

Smart Gimmick AI makes trainer-owned gimmicks behave like strategic resources instead of automatic first-turn buttons.

## Status

| Field | Value |
| --- | --- |
| Runtime branch | `feature/smart-gimmick-ai-16-20260604` |
| Code status | Runtime implementation active on feature branch |
| Primary docs | `docs/tutorials/ai_flags.md`, this folder |
| Main flags | `AI_FLAG_SMART_GIMMICK`, `AI_FLAG_GIMMICK_ENV_TERA_ONLY`, `AI_FLAG_GIMMICK_ENV_DYNAMAX_ONLY`, `AI_FLAG_GIMMICK_ENV_DYNAMAX_TERA`, `AI_FLAG_GIMMICK_ENV_ALL`, `AI_FLAG_GIMMICK_ENV_INVERSE_BATTLE` |

## Runtime Intent

- Trainer data says a gimmick is available; smart AI decides whether this turn is worth spending it.
- Tera should be held until there is offensive payoff, defensive payoff, or a specific target interaction.
- Dynamax should be held unless the AI has last-Pokemon pressure, survival pressure, KO conversion, or a Max Move board payoff.
- Mega Evolution / Ultra Burst may be delayed for setup turns when the current form is not immediately threatened.
- Z-Moves remain behind the existing Z-Move viability checks instead of firing only because a Z-Crystal exists.

## Current Dynamax Payoffs

`AI_FLAG_SMART_DYNAMAX` now recognizes these additional Max Move reasons:

- Speed control: `Max Airstream` ally Speed boost and `Max Strike` opposing Speed drop.
- Weather control: `Max Flare`, `Max Geyser`, `Max Rockfall`, and `Max Hailstorm`.
- Terrain control: `Max Lightning`, `Max Overgrowth`, `Max Starfall`, and `Max Mindstorm`.
- Side-wide stat pressure: `Max Knuckle`, `Max Ooze`, `Max Quake`, `Max Steelspike`, `Max Wyrmwind`, `Max Flutterby`, `Max Phantasm`, and `Max Darkness`.

The weather and terrain checks reuse the existing AI field-status evaluators so this feature does not invent a separate weather / terrain opinion system.

## Debug Fixtures

Trainer IDs `855` through `863` are reserved smart-gimmick validation fixtures. Use the debug trainer-battle flow, set Trainer 1 to the ID, then start `Try Battle`.

See `docs/tutorials/ai_flags.md` for the ID table and expected first-turn behavior.

## Documents

- [Investigation](investigation.md)
- [Implementation](implementation.md)
- [Test Plan](test_plan.md)
