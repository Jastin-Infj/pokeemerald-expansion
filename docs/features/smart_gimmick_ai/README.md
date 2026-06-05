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
- Mega Evolution / Ultra Burst may be delayed for setup turns or pre-Mega ability value, but spent for immediate ability, Speed, damage, or defensive payoff.
- Z-Moves remain behind viability and smart timing checks instead of firing only because a Z-Crystal exists.
- Smart switching can use reserve Pokemon as board-control tools: weather setters, terrain setters, Tailwind, and Trick Room can justify a pivot when they flip the field or speed state.

## Current Mega / Ultra Burst Payoffs

`AI_FLAG_SMART_MEGA` now treats Mega Evolution as a timing decision instead of a guaranteed first-turn action. It spends Mega / Ultra Burst when one of these is true:

- The target form's ability creates immediate board value, such as `Shadow Tag` trapping or weather control from `Drought`, `Drizzle`, `Sand Stream`, `Snow Warning`, or `Delta Stream`.
- The target form's Speed is estimated to flip the current matchup.
- The target form's defensive stats are meaningfully better and the current target can otherwise KO the AI.
- The target form's attacking stat meaningfully improves the selected damaging move.

The AI may still delay Mega on setup turns, and may preserve `Air Lock` / `Cloud Nine` while weather is active if the target form does not provide a stronger immediate payoff.

## Current Dynamax Payoffs

`AI_FLAG_SMART_DYNAMAX` now recognizes these additional Max Move reasons:

- Speed control: `Max Airstream` ally Speed boost and `Max Strike` opposing Speed drop.
- Weather control: `Max Flare`, `Max Geyser`, `Max Rockfall`, and `Max Hailstorm`.
- Terrain control: `Max Lightning`, `Max Overgrowth`, `Max Starfall`, and `Max Mindstorm`.
- Side-wide stat pressure: `Max Knuckle`, `Max Ooze`, `Max Quake`, `Max Steelspike`, `Max Wyrmwind`, `Max Flutterby`, `Max Phantasm`, and `Max Darkness`.

The weather and terrain checks reuse the existing AI field-status evaluators so this feature does not invent a separate weather / terrain opinion system.

## Current Z-Move Payoffs

`AI_FLAG_SMART_Z_MOVE` first applies the existing Z-Move viability checks. Under smart gimmick timing, damaging Z-Moves are then conserved unless one of these is true:

- The AI is on its last available Pokemon.
- The Z-Move converts the selected move into a KO.
- The Z-Move improves the damage race while the AI is under immediate KO pressure or trapped.
- The base move already has a KO line, but the Z-Move avoids a low-accuracy miss.

Status Z-Moves keep their existing tactical checks because their value is usually the Z-status effect rather than raw damage.

## Current Smart Switching Payoffs

`AI_FLAG_SMART_SWITCHING` now has two VGC-style switching layers in this feature branch:

- Bad-position switching preserves a Pokemon that has no useful pressure, is threatened by either opposing slot, and cannot be covered by its partner.
- Board-control switching can pivot into reserve weather, terrain, Tailwind, or Trick Room roles when those effects improve pressure or replace an unfavorable field state.

Singles remain more conservative; the AI still needs bad odds, bad matchup, weak current pressure, or a bad field state before it pivots for board control. Doubles allow more proactive pivots because the partner slot and the reserve role can create pressure together.

## VGC Reference Notes

These runtime heuristics are source-derived rather than copied from a single match:

- The 2019 Worlds recap records a Rayquaza player delaying Mega Evolution to keep `Air Lock` until it was no longer needed, which maps to preserving pre-Mega weather-nullifying abilities when there is no stronger immediate Mega payoff: https://pokemonblog.com/2019/08/20/official-recap-of-pokemon-vgc-at-the-2019-pokemon-world-championships/
- The same 2019 recap describes Mega Gengar / Perish Song as a trap archetype and a knockout breaking `Shadow Tag`, supporting immediate Mega payoff for trapping and Z payoff for breaking trap pressure: https://pokemonblog.com/2019/08/20/official-recap-of-pokemon-vgc-at-the-2019-pokemon-world-championships/
- Paul Ruiz's 2018 Worlds report emphasizes Mega Salamence Speed and KO benchmarks into Mega Gengar and other targets, supporting Mega Speed / damage pressure checks: https://victoryroad.pro/2018/09/14/soaring-higher-report-paul-ruiz-2018-world-champion/
- Public 2018 Worlds recaps describe Groundium Z helping escape a Perish Trap matchup, supporting Z-Move use before last-Pokemon turns when trap pressure changes the damage race: https://thegamehaus.com/esports/this-is-for-latin-america-2018-pokemon-world-championships-recap/2018/08/28/
- The 2017 Worlds finals recap notes a Dark-type switch-in stopping Prankster-boosted Z-Nature Power, so status Z-Moves remain under existing tactical legality / viability checks instead of being blindly conserved or blindly fired: https://www.nintendolife.com/news/2017/08/feature_everything_you_need_to_know_about_the_2017_pokemon_world_championships

## Debug Fixtures

Trainer IDs `855` through `863` are reserved smart-gimmick validation fixtures. Use the debug trainer-battle flow, set Trainer 1 to the ID, then start `Try Battle`.

See `docs/tutorials/ai_flags.md` for the ID table and expected first-turn behavior.

## Documents

- [Investigation](investigation.md)
- [Implementation](implementation.md)
- [Test Plan](test_plan.md)
