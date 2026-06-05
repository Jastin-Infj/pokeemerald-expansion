# Smart Gimmick AI Investigation

This investigation records the VGC references used to tune smart gimmick timing. The runtime goal is not to perfectly copy one match. It is to keep the AI from spending Mega, Z-Move, Dynamax, or Tera only because the mechanic is available.

## Source Set

| Source | Use |
| --- | --- |
| [Pokemon Worlds 2024 VGC Masters teams](https://www.pokemon.com/us/play-pokemon/worlds/2024/vgc-masters) | Official team reference for the 2024 finals environment. |
| [Yuta Ishigaki vs Luca Ceribelli, Pokemon Worlds 2024 VGC Masters Finals](https://www.youtube.com/watch?v=jNnRJGnD8Hg) | Official VOD with usable auto-caption timestamps for Tera timing and defensive Tera discussion. |
| [Luca Ceribelli Worlds 2024 report, Victory Road](https://victoryroad.pro/2024/09/22/luca-ceribelli-worlds-report/) | Player report and team context for the 2024 winning team. |
| [Bulbagarden 2024 finals recap](https://www.bulbagarden.net/threads/luca-ceribelli-wins-the-2024-pokemon-world-championships-in-vgc-day-3-finals-results.302963/) | Turn-level public recap for the 2024 finals. |
| [VGC Guide: Protect in Battle](https://www.vgcguide.com/protect-in-battle) | Reference for defensive timing, double-battle pressure, and not committing every resource immediately. |
| [VGC Guide: Dynamax Candy and Max Soup](https://www.vgcguide.com/dynamax-candy-and-max-soup) | Reference for Max Move value and Gigantamax / Dynamax preparation context. |
| [Pokemon Worlds 2022 VGC Masters Grand Finals](https://www.youtube.com/watch?v=UldeXA6T4_A) | Official VOD for Gen 8 Dynamax-era resource timing review. |
| [Pokemon Worlds 2019 VGC Masters Finals](https://www.youtube.com/watch?v=hBBjcnADWDI) | Official VOD for Gen 7 Mega / Z-Move resource timing review. |
| [Official recap of Pokemon VGC at the 2019 Pokemon World Championships](https://pokemonblog.com/2019/08/20/official-recap-of-pokemon-vgc-at-the-2019-pokemon-world-championships/) | Source for Rayquaza delaying Mega Evolution to keep `Air Lock`, Mega Gengar / Perish Song trap pressure, and Fairium Z offensive pressure into Umbreon. |
| [Naoto Mizobuchi 2019 Worlds report, Victory Road](https://victoryroad.pro/2019/08/29/world-champion-naoto-mizobuchi-report/) | Player report for the 2019 Mega / Z-Move environment. |
| [Paul Ruiz 2018 Worlds report, Victory Road](https://victoryroad.pro/2018/09/14/soaring-higher-report-paul-ruiz-2018-world-champion/) | Earlier Gen 7 Worlds report for Z-Move / Mega context. |
| [2018 Pokemon World Championships recap, The Game Haus](https://thegamehaus.com/esports/this-is-for-latin-america-2018-pokemon-world-championships-recap/2018/08/28/) | Public recap for Paul Ruiz using Groundium Z and Mega Salamence pressure to escape Emilio Forbes's Perish Trap / Mega Gengar matchup. |
| [2017 Pokemon World Championships recap, Nintendo Life](https://www.nintendolife.com/news/2017/08/feature_everything_you_need_to_know_about_the_2017_pokemon_world_championships) | Public recap for Mandibuzz stopping Prankster-boosted Z-Nature Power, supporting tactical checks for status Z-Moves. |

## Timestamp Notes

The 2024 official VOD had English auto-captions available through `yt-dlp`; the 2022 and 2019 official VODs did not expose the requested English caption track during this pass.

| VOD timestamp | Runtime takeaway |
| --- | --- |
| 2024 finals, `19:13` | Ogerpon Fire pressure is discussed before the eventual Tera turn, showing that the Tera threat can shape play before it is spent. |
| 2024 finals, `22:47` | Ogerpon Tera happens later in the game, not automatically on turn 1. This supports treating Tera as conditional. |
| 2024 finals, `23:01` | Embody Aspect turns Tera into an immediate stat / pressure swing, supporting offensive payoff checks. |
| 2024 finals, `24:41` | A Pokemon that has already spent Tera cannot later use a different defensive Tera. This supports conserving Tera until the chosen defensive or offensive line is valuable. |
| 2024 finals, `26:31` | Defensive Grass Tera on Pelipper is discussed as a survival line, supporting defensive Tera checks. |

## Conclusions

- Gimmick availability is not the same as correct timing. The AI needs a reason to spend the resource.
- Doubles makes Speed control and side-wide effects more valuable than singles. `Max Airstream`, `Max Strike`, and side-wide stat Max Moves should be considered board payoffs, not only damage conversions.
- Weather and terrain Max Moves should use existing AI field-status opinions so they are only spent when the field state helps the AI side or disrupts an unfavorable field.
- Tera should remain calculation-led: offensive boost, defensive survival, and explicit candidate handling are more important than automatic first-turn activation.
- Mega Evolution should be delayed when the base form has continuing value, such as `Air Lock` / `Cloud Nine` during active weather, but spent when the target form creates immediate ability, Speed, damage, or defensive payoff.
- Z-Moves should not be last-Pokemon-only. 2018 trap examples support spending a damaging Z-Move earlier when it improves the damage race against trapping pressure.
- Status Z-Moves still need tactical legality checks. The 2017 Z-Nature Power example supports keeping those checks separate from simple resource-conservation heuristics.
