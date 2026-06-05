# Smart Gimmick AI Investigation

This investigation records the VGC references used to tune smart gimmick timing. The runtime goal is not to perfectly copy one match. It is to keep the AI from spending Mega, Z-Move, Dynamax, or Tera only because the mechanic is available.

## Source Set

| Source | Use |
| --- | --- |
| [Pokemon Worlds 2024 VGC Masters teams](https://www.pokemon.com/us/play-pokemon/worlds/2024/vgc-masters) | Official team reference for the 2024 finals environment. |
| [2024 Pokemon World Championships event results](https://www.pokemon.com/us/play-pokemon/worlds/2024/event-results) | Official event page linking VGC team lists, used to verify that field/weather pieces such as Pelipper and Rillaboom are real tournament team-slot pressure, not artificial test-only picks. |
| [Yuta Ishigaki vs Luca Ceribelli, Pokemon Worlds 2024 VGC Masters Finals](https://www.youtube.com/watch?v=jNnRJGnD8Hg) | Official VOD with usable auto-caption timestamps for Tera timing and defensive Tera discussion. |
| [Luca Ceribelli Worlds 2024 report, Victory Road](https://victoryroad.pro/2024/09/22/luca-ceribelli-worlds-report/) | Player report and team context for the 2024 winning team. |
| [Bulbagarden 2024 finals recap](https://www.bulbagarden.net/threads/luca-ceribelli-wins-the-2024-pokemon-world-championships-in-vgc-day-3-finals-results.302963/) | Turn-level public recap for the 2024 finals. |
| [VGC Guide: Protect in Battle](https://www.vgcguide.com/protect-in-battle) | Reference for defensive timing, double-battle pressure, and not committing every resource immediately. |
| [VGC Guide: Coming from Single Battles](https://www.vgcguide.com/coming-from-single-battles) | Reference for doubles positioning: switching happens frequently in VGC, double targets are central, and switching can reset volatile pressure or reuse entry abilities such as `Intimidate` and `Fake Out`. |
| [VGC Guide: Dynamax Candy and Max Soup](https://www.vgcguide.com/dynamax-candy-and-max-soup) | Reference for Max Move value and Gigantamax / Dynamax preparation context. |
| [Pokemon Ryuoh 2024 game division official page](https://www.pokemon.co.jp/ex/pokemonryuoh/2024/game/) | Official singles / Ryuoh reference. The Dragon Tera selection rule explicitly does not require always Terastallizing in battle, reinforcing the "permission is not immediate use" model for singles. |
| [Pokemon Ryuoh 2024 viewing information](https://www.pokemon.co.jp/ex/pokemonryuoh/2024/howtowatch/) | Official broadcast reference. The page documents YouTube / livestream coverage and Pokemon Battle Scope usage, confirming that Ryuoh battle-state interpretation was part of the official viewing product. |
| [Pokemon Worlds 2022 VGC Masters Grand Finals](https://www.youtube.com/watch?v=UldeXA6T4_A) | Official VOD for Gen 8 Dynamax-era resource timing review. |
| [Pokemon Worlds 2019 VGC Masters Finals](https://www.youtube.com/watch?v=hBBjcnADWDI) | Official VOD for Gen 7 Mega / Z-Move resource timing review. |
| [Official recap of Pokemon VGC at the 2019 Pokemon World Championships](https://pokemonblog.com/2019/08/20/official-recap-of-pokemon-vgc-at-the-2019-pokemon-world-championships/) | Source for Rayquaza delaying Mega Evolution to keep `Air Lock`, Mega Gengar / Perish Song trap pressure, and Fairium Z offensive pressure into Umbreon. |
| [Naoto Mizobuchi 2019 Worlds report, Victory Road](https://victoryroad.pro/2019/08/29/world-champion-naoto-mizobuchi-report/) | Player report for the 2019 Mega / Z-Move environment. |
| [Paul Ruiz 2018 Worlds report, Victory Road](https://victoryroad.pro/2018/09/14/soaring-higher-report-paul-ruiz-2018-world-champion/) | Earlier Gen 7 Worlds report for Z-Move / Mega context. |
| [2018 Pokemon World Championships recap, The Game Haus](https://thegamehaus.com/esports/this-is-for-latin-america-2018-pokemon-world-championships-recap/2018/08/28/) | Public recap for Paul Ruiz using Groundium Z and Mega Salamence pressure to escape Emilio Forbes's Perish Trap / Mega Gengar matchup. |
| [2017 Pokemon World Championships recap, Nintendo Life](https://www.nintendolife.com/news/2017/08/feature_everything_you_need_to_know_about_the_2017_pokemon_world_championships) | Public recap for Mandibuzz stopping Prankster-boosted Z-Nature Power, supporting tactical checks for status Z-Moves. |
| [Pokemon Wiki: Dynamax](https://wiki.pokemonwiki.com/wiki/%E3%83%80%E3%82%A4%E3%83%9E%E3%83%83%E3%82%AF%E3%82%B9) | Specification reference for Dynamax as a limited three-turn battle resource with HP / move changes and state-change immunity implications. |
| [Pokemon Wiki: Fake Out](https://wiki.pokemonwiki.com/wiki/%E3%81%AD%E3%81%93%E3%81%A0%E3%81%BE%E3%81%97) | Specification reference for first-action priority flinch pressure. |
| [Pokemon Wiki: Whirlwind](https://wiki.pokemonwiki.com/wiki/%E3%81%B5%E3%81%8D%E3%81%A8%E3%81%B0%E3%81%97) and [Roar](https://wiki.pokemonwiki.com/wiki/%E3%81%BB%E3%81%88%E3%82%8B) | Specification reference for forced-switch effects and their Dynamax failure cases. |
| [Pokemon Wiki: Status condition](https://wiki.pokemonwiki.com/wiki/%E7%8A%B6%E6%85%8B%E7%95%B0%E5%B8%B8) | Specification reference for status as damage, action denial, hidden stat penalties, prevention through Safeguard / Misty Terrain, and self-status item plans. |
| [Pokemon Wiki: Poison status](https://wiki.pokemonwiki.com/wiki/%E3%81%A9%E3%81%8F_%28%E7%8A%B6%E6%85%8B%E7%95%B0%E5%B8%B8%29) | Specification reference for residual damage, Toxic / Toxic Spikes pressure, status-benefit abilities, Magic Guard, Poison Heal, and status prevention. |
| [Pokemon Wiki: Freeze status](https://wiki.pokemonwiki.com/wiki/%E3%81%93%E3%81%8A%E3%82%8A_%28%E7%8A%B6%E6%85%8B%E7%95%B0%E5%B8%B8%29) | Specification reference for action denial, freeze / frostbite-style additional effects, thaw conditions, and weather / terrain prevention. |
| [Pokemon Wiki: Confusion](https://wiki.pokemonwiki.com/wiki/%E3%81%93%E3%82%93%E3%82%89%E3%82%93) | Specification reference for confusion as a status-change pressure layer that is sometimes grouped with status conditions but is not stored as regular non-volatile status. |
| [Pokemon Wiki: Aurora Veil](https://wiki.pokemonwiki.com/wiki/%E3%82%AA%E3%83%BC%E3%83%AD%E3%83%A9%E3%83%99%E3%83%BC%E3%83%AB) | Specification reference for snow / hail gating on Aurora Veil. |
| [Pokemon Wiki: Electric Seed](https://wiki.pokemonwiki.com/wiki/%E3%82%A8%E3%83%AC%E3%82%AD%E3%82%B7%E3%83%BC%E3%83%89) | Specification reference for field-triggered terrain seed stat activation. |
| [Pokemon Wiki: Skill Swap](https://wiki.pokemonwiki.com/wiki/%E3%82%B9%E3%82%AD%E3%83%AB%E3%82%B9%E3%83%AF%E3%83%83%E3%83%97) | Specification reference for ability exchange as a bridge to board-control ability lines. |
| [Pokemon Wiki: Sand Rush](https://wiki.pokemonwiki.com/wiki/%E3%81%99%E3%81%AA%E3%81%8B%E3%81%8D) | Specification reference for sandstorm speed synergy and sand damage immunity. |

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
- Dynamax can be a disruption-prevention resource, not only an offensive resource. If a known or predicted opposing move threatens Fake Out-style flinch or Roar / Whirlwind-style phazing, smart Dynamax may be correct because it lets the selected damaging move resolve.
- Doubles should not suppress switching only because two Pokemon are on the field. VGC positioning still uses frequent switches to preserve board state, reset volatile effects, and reuse entry abilities. Runtime double switching should therefore consider whether the active Pokemon has useful pressure, whether it is under pressure from either opposing slot, and whether its partner can cover the threat.
- Bench Pokemon can be active board resources, not only replacements after losing a matchup. This update maps that tournament pattern to entry weather / terrain setters (`Drizzle`, `Drought`, `Sand Stream`, `Snow Warning`, terrain surges), speed-control setters (`Tailwind`, `Trick Room`), terrain seed plans, status pressure / status prevention, and ability-bridge support such as `Skill Swap` when they can flip the board or increase pressure.
- The Rillaboom / Pelipper style examples are source-derived in principle rather than copied from a single turn: official Worlds team lists include those field/weather pieces, and VGC Guide identifies Tailwind, Trick Room, weather, terrain, switching, and repositioning as core doubles concepts.
- Weather and terrain Max Moves should use existing AI field-status opinions so they are only spent when the field state helps the AI side or disrupts an unfavorable field.
- Singles / Ryuoh-style logic should stay more conservative than doubles. Official Ryuoh 2024 rules require selecting a Dragon Tera Pokemon but do not require spending Tera in battle, so singles board-control switching should remain tied to bad odds, bad matchup, immediate field replacement, or lack of current pressure.
- Status conditions are not only negative cleanup states. They also create prevention lines (`Safeguard`, `Misty Terrain`), cure lines (`Heal Bell` / `Aromatherapy`), self-status item lines (`Flame Orb`, `Toxic Orb`), and ability payoff lines (`Guts`, `Quick Feet`, `Marvel Scale`, `Magic Guard`, `Poison Heal`, `Toxic Boost`, `Flare Boost`). Confusion / Yawn / Leech Seed / Toxic Spikes are tracked as adjacent status-pressure or board-pressure layers rather than raw `status1` states.
- Status pressure must include both direct status moves and damaging moves with secondary status or confusion effects. VGC-style board pressure often comes from "this move is still good damage, but can also burn, paralyze, freeze / frostbite, poison, or confuse" rather than from a pure status move only.
- Tera should remain calculation-led: offensive boost, defensive survival, and explicit candidate handling are more important than automatic first-turn activation.
- Mega Evolution should be delayed when the base form has continuing value, such as `Air Lock` / `Cloud Nine` during active weather, but spent when the target form creates immediate ability, Speed, damage, or defensive payoff.
- Z-Moves should not be last-Pokemon-only. 2018 trap examples support spending a damaging Z-Move earlier when it improves the damage race against trapping pressure.
- Status Z-Moves still need tactical legality checks. The 2017 Z-Nature Power example supports keeping those checks separate from simple resource-conservation heuristics.
