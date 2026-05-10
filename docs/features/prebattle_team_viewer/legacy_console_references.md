# Legacy Console Battle UI References

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-10 |
| Baseline | `master` `7c19f56901`; `git describe` = `expansion/1.15.2-38-g7c19f56901` |
| Code status | Docs-only reference research |
| Provenance | Web research / local feature docs |

## Priority

Pokémon Champions remains the primary reference for this feature. Older console games are
secondary references for layout, battle flow, and privacy / information boundaries.

The useful older references are:

- Pokémon Colosseum
- Pokémon XD: Gale of Darkness
- Pokémon Battle Revolution

## Source Summary

| Source | Type | Useful facts |
|---|---|---|
| Nintendo UK: `https://www.nintendo.com/en-gb/Games/Nintendo-GameCube/Pokemon-Colosseum-268566.html` | official / publisher page | Colosseum lets players upload Pokémon from Ruby / Sapphire, battle on GameCube, enter tournaments, play four-player multi-battles, and join a friend in two-on-two battles. |
| Nintendo ZA: `https://www.nintendo.com/en-za/Games/Wii/Pokemon-Battle-Revolution-282629.html` | official / publisher page | Battle Revolution copies trained Pokémon from Diamond / Pearl, uses 10 battle colosseums, supports Rental Passes with six Pokémon, DS Battle Mode, hidden DS commands on TV, and Wii Remote / D-pad menu navigation. |
| Nintendo of America YouTube: `https://www.youtube.com/watch?v=Z10GIw_dH8w` | official video metadata | XD re-release description confirms Shadow Pokémon, Aura Reader, Snag Machine, and that the Nintendo Classics version cannot transfer Pokémon from GBA titles. |
| Smogon Colosseum / XD mechanics guide: `https://www.smogon.com/ingame/guides/colosseum_xd_mechanics_guide` | secondary mechanics guide | Colosseum / XD put double battles at the center. It also notes Colosseum Battle Mode Mt. Battle uses 3v3 singles or 4v4 doubles chosen from a pool of 6. |
| Bulbapedia Group Battle: `https://bulbapedia.bulbagarden.net/wiki/Group_Battle` | secondary mechanics / UI reference | Colosseum battle rules were sequential menus; XD redesigned settings into one screen. Single / double rules are explicit in link battle setup. |
| Bulbapedia Battle Pass: `https://bulbapedia.bulbagarden.net/wiki/Battle_Pass_(Battle_Revolution)` | secondary UI / data reference | A Battle Pass contains Trainer and party Pokémon information. Custom Passes use copied Gen IV Pokémon; Rental Passes have predetermined rental Pokémon; Friend Passes represent opponents. |

## Design Takeaways

| Reference | Takeaway for this feature |
|---|---|
| Colosseum tournament / multi-battle presentation | Team viewer can frame a battle as a formal match card: trainer identity plus six Pokémon. |
| Colosseum / XD double focus | Double battle viewer should not be a bolt-on. 4-from-6 selection needs equal UI weight with 3-from-6 singles. |
| Colosseum Battle Mode 3v3 / 4v4 from 6 | Directly supports the requested selection model: singles select 3, doubles select 4, from a visible 6. |
| XD one-screen rules setup | If config / rule information appears later, prefer one compact screen over multiple confirmation menus. |
| Battle Revolution Battle Pass | A reusable "team card" model is a good conceptual match for the cache/view-model: trainer metadata plus 6 party entries. |
| Battle Revolution Rental Pass | If future trainer party pools / generated teams need a public label, a "pass" / "roster card" style can distinguish generated preview from raw trainer data. |
| Battle Revolution DS hidden commands | In-battle viewer must not leak hidden command data. Read-only team view should avoid opponent moves / ability / held item unless explicitly public by the target reference. |
| Wii Remote / D-pad navigation | GBA implementation should keep both cursor movement and button shortcuts simple: D-pad moves highlight, A confirms / closes, B backs out, Y opens details where available. |

## Not Adopted As MVP Requirements

- Full 3D battle presentation, camera work, trainer customization, and Battle Pass artwork.
- DS-touchscreen style hidden command entry.
- Link battle / multi-battle support.
- Rental Pass trading or Friend Pass exchange.
- Colosseum / XD Shadow Pokémon mechanics.

## Open Questions

- Whether to visually label opponent preview as a "battle card" / "team card" in Phase 2.
- Whether generated trainer parties should carry a debug-only source label similar to a Battle Pass.
- Whether in-battle viewer should eventually show original full 6 plus selected 3/4, or only battle-available selected Pokémon.
