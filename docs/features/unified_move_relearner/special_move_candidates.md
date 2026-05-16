# Special Move Candidates

## Purpose

Unified Move Relearner supports special move candidates that are not normal
level-up, egg, TM/HM/TR, or tutor moves.

This covers official distribution and side-game cases such as event-exclusive
moves, Cherish Ball distribution Pokemon, PCNY / Wish Egg style legacy events,
and Pokemon XD purification moves. These candidates should live in an external
project-owned JSON source so upstream learnset regeneration does not erase the
local compatibility policy.

The global special-move dataset may be large because event distributions cover
many species. That should not imply a large visible list for every selected
Pokemon. Runtime lookup should filter by the selected species/form first, then
append only matching special candidates to that Pokemon's candidate list.

Current implementation status: the runtime source is
`tools/learnset_helpers/special_relearner_moves.json` and is compiled into the
generated unified relearner table. Runtime display uses the shared `Sp` label
for now; per-entry `EV` / `XD` / `RG` labels and unlock-group gating are still
future work.

## Source Policy

Use official-game-compatible public references for audit, but keep the project
data local.

| Source class | Use | Notes |
|---|---|---|
| PokemonWiki `配布・貴重なポケモン一覧` | Primary route map | Covers distributed Pokemon, distribution eggs, rare side-game Pokemon, and links Japanese / overseas generation pages. |
| PokemonWiki individual event / rare Pokemon pages | Primary move-level audit reference when available | Useful for pages that list initial moves directly, such as XD Lugia and WISHMKR Jirachi. |
| Bulbapedia event move pages | Primary web audit reference | Useful for `Celebrate`, `Happy Hour`, `Hold Back`, `Hold Hands`, `V-create`, and per-move event learnsets. |
| Bulbapedia Event Pokemon page | Primary category reference | Confirms event-exclusive moves and Cherish Ball distribution context. |
| Bulbapedia Purification page | Primary XD reference | Confirms Pokemon XD purified special moves such as Metal Sound Zapdos. |
| Bulbapedia event distribution list pages | Public audit reference | Useful for region-specific Wonder Card rows, especially Gen IV-VI. |
| Pokemon Wiki / Serebii / similar public event pages | Fallback audit reference | Use when Bulbapedia pages are incomplete or hard to search. |

Unofficial event databases or tools may be used as import material when they
are the most complete source for hard-to-find distribution details. The project
policy is to copy normalized compatibility data, not raw event payloads.

Allowed checked-in data:

- `species`, `move`, `form`, `generation`, `games`, `regions`,
  `languageRegions`, `distributionId`, and local source labels.
- Small project-owned notes needed to explain why a candidate exists.
- Import provenance such as "imported from checksum-backed event dataset" when
  a public page does not cover the row.

Avoid by default:

- Raw Wonder Card / Mystery Gift binary payloads.
- Private database exports in their original format.
- Copied event descriptions or long prose from unofficial tools.

If a future implementation needs raw payloads for validation, keep that as a
separate explicit decision. The Move Relearner only needs normalized
compatibility facts.

PokemonWiki pages currently advertise a CC BY-NC-SA 3.0 license in their
footer. Keep source URLs in `sourceRefs` for attribution, copy normalized facts
only, and avoid copying long prose into repository data.

## Region / Language Variants

Event move compatibility differs by Japanese, American, European, Korean, and
other regional distributions. The special source must not collapse those
differences during data capture.

Store distribution metadata separately from the final move candidate:

- `distributionId`: stable local id, for example `gen5_15th_anniversary_rayquaza_jp`.
- `distributionName`: human-readable event name.
- `generation` and `games`: original distribution generation / game family.
- `regions`: distribution regions such as `JP`, `NA`, `EU`, `KR`, `TW`, `WORLD`.
- `languageRegions`: language or game-region restrictions when known.
- `sourceRefs`: audit pages used to verify the entry.
- `auditStatus`: whether the entry is public-reference-backed, locally
  accepted, or still pending verification.
- `importProvenance`: optional note for rows imported from a checksum-backed or
  otherwise trusted event dataset when public pages are incomplete.

At runtime, the project can still collapse multiple regional distributions into
one displayed candidate when they grant the same `species + move + source badge`.
However, the source JSON should retain the original region/language data so a
later ruleset can choose "Japan-only events allowed" or "international events
only" without re-auditing the move.

The first seed was intentionally small. A 2026-05-16 broadening pass expanded
the runtime JSON from 25 candidate blocks / 50 moves to 164 blocks / 206 moves
using public move-page and XD references. This is still not exhaustive; a full
special-event pass should be scripted from a reviewed local intermediate list
and should leave an audit status on every row.

The first broad pass should start from Generation III onward. Earlier Mew
distributions are historically important, but they are only relevant to the
Move Relearner if they carry moves outside normal level / TM / tutor coverage.
Generation III and later Wonder Card / side-game transfer data is where the
special move compatibility surface becomes large enough to justify this source.

Audit workflow:

1. Start from PokemonWiki `配布・貴重なポケモン一覧`.
2. Follow the Japanese and overseas generation list pages.
3. Open individual distribution / rare Pokemon pages when the list row does not
   expose moves directly.
4. Normalize only Move Relearner-relevant facts into JSON: species, move,
   source, region, generation, games, distribution id/name, and audit status.
5. Cross-check ambiguous rows against Bulbapedia, Serebii, or a trusted
   checksum-backed event dataset before enabling by default.

## Proposed Data Shape

Historical seed data for this design is tracked in
[`special_move_candidates_seed.json`](special_move_candidates_seed.json).

Runtime data is tracked in
`tools/learnset_helpers/special_relearner_moves.json`. It keeps the same
normalized candidate facts and source-ref metadata, while the generator uses
only `species` and `moves` to build `src/data/pokemon/unified_relearner_learnsets.h`.

The runtime JSON shape is:

```json
{
  "schemaVersion": 1,
  "sourceKind": "special_relearner_moves",
  "candidates": [
    {
      "species": "SPECIES_ZAPDOS",
      "form": "default",
      "move": "MOVE_METAL_SOUND",
      "source": "xd_purification",
      "unlockGroup": "special_xd",
      "display": "XD",
      "regions": ["WORLD"],
      "languageRegions": ["all"],
      "distributionId": "xd_zapdos",
      "auditStatus": "public_reference_backed"
    }
  ]
}
```

Candidate identity remains `move + source`. If a move is already available via
TM or tutor, the special entry may still be useful for provenance, but the
unlock policy should decide whether to show duplicate special entries.

For implementation, prefer generating C data indexed by species/form from this
JSON instead of scanning a large flat table at runtime.

## Initial Candidate Seeds

This is not exhaustive. It is the original starting audit set kept here to show
the implementation shape. The current runtime JSON now also includes:

- the broader Bulbapedia `Celebrate`, `Happy Hour`, `Hold Back`, `Hold Hands`,
  and `V-create` event learnset rows;
- the Pokemon XD purification special-move table from Bulbapedia;
- Gen III Wish Egg / legacy rows for `Wish` and `Yawn`;
- event rows for `Sing`, `Teeter Dance`, and `Extreme Speed`;
- Cinema Genesect's `Extreme Speed`, `Blaze Kick`, and `Shift Gear`, cross-
  checked against PokemonWiki and Bulbapedia.

| Species | Move | Source kind | Notes |
|---|---|---|---|
| `SPECIES_ZAPDOS` | `MOVE_BATON_PASS` | `xd_purification` | XD purified Zapdos special move. |
| `SPECIES_ZAPDOS` | `MOVE_METAL_SOUND` | `xd_purification` | Key example from current feature discussion. |
| `SPECIES_ZAPDOS` | `MOVE_EXTRASENSORY` | `xd_purification` | XD purified Zapdos special move. |
| `SPECIES_ARTICUNO` | `MOVE_HAZE` | `xd_purification` | XD purified Articuno special move. |
| `SPECIES_ARTICUNO` | `MOVE_HEAL_BELL` | `xd_purification` | XD purified Articuno special move. |
| `SPECIES_ARTICUNO` | `MOVE_EXTRASENSORY` | `xd_purification` | XD purified Articuno special move. |
| `SPECIES_MOLTRES` | `MOVE_MORNING_SUN` | `xd_purification` | XD purified Moltres special move. |
| `SPECIES_MOLTRES` | `MOVE_WILL_O_WISP` | `xd_purification` | XD purified Moltres special move. |
| `SPECIES_MOLTRES` | `MOVE_EXTRASENSORY` | `xd_purification` | XD purified Moltres special move. |
| `SPECIES_KANGASKHAN` | `MOVE_SING` | `xd_purification` | XD purified Kangaskhan special move. |
| `SPECIES_KANGASKHAN` | `MOVE_YAWN` | `legacy_event` | PCNY / Wish Egg style event candidate; verify exact distribution before enabling by default. |
| `SPECIES_KANGASKHAN` | `MOVE_WISH` | `legacy_event` | Same event family as Yawn Kangaskhan; verify exact distribution before enabling by default. |
| `SPECIES_SNORLAX` | `MOVE_REFRESH` | `xd_purification` | XD purified Snorlax special move. |
| `SPECIES_SNORLAX` | `MOVE_FISSURE` | `xd_purification` | Also historical TM coverage exists; keep source policy explicit. |
| `SPECIES_LUGIA` | `MOVE_FEATHER_DANCE` | `xd_purification` | XD purified Lugia special move. |
| `SPECIES_LUGIA` | `MOVE_PSYCHO_BOOST` | `xd_purification` | XD purified Lugia special move. |
| `SPECIES_RAYQUAZA` | `MOVE_V_CREATE` | `event_exclusive` | Gen V 15th Anniversary Rayquaza; Japanese-region event example. |
| `SPECIES_DARKRAI` | `MOVE_ROAR_OF_TIME` | `event_exclusive` | Movie 10 / Alamos Darkrai distribution family. |
| `SPECIES_DARKRAI` | `MOVE_SPACIAL_REND` | `event_exclusive` | Movie 10 / Alamos Darkrai distribution family. |
| `SPECIES_ARCEUS` | `MOVE_ROAR_OF_TIME` | `event_exclusive` | Movie 12 / Michina Arceus distribution family. |
| `SPECIES_ARCEUS` | `MOVE_SPACIAL_REND` | `event_exclusive` | Movie 12 / Michina Arceus distribution family. |
| `SPECIES_ARCEUS` | `MOVE_SHADOW_FORCE` | `event_exclusive` | Movie 12 / Michina Arceus distribution family. |
| `SPECIES_ARCEUS` | `MOVE_BLAST_BURN` | `event_exclusive` | Dahara City Arceus. |
| `SPECIES_ARCEUS` | `MOVE_HYDRO_CANNON` | `event_exclusive` | Dahara City Arceus. |
| `SPECIES_ARCEUS` | `MOVE_EARTH_POWER` | `event_exclusive` | Dahara City Arceus. |
| `SPECIES_HEATRAN` | `MOVE_ERUPTION` | `ranger_transfer` | Pokemon Ranger: Guardian Signs transferable Heatran. |
| `SPECIES_JIRACHI` | `MOVE_DRACO_METEOR` | `event_exclusive` | Summer 2010 / Tanabata event family. |
| `SPECIES_RAIKOU` | `MOVE_AURA_SPHERE` | `event_exclusive` | Crown / Winter 2011 distribution family. |
| `SPECIES_RAIKOU` | `MOVE_WEATHER_BALL` | `event_exclusive` | Crown / Winter 2011 distribution family. |
| `SPECIES_ENTEI` | `MOVE_FLARE_BLITZ` | `event_exclusive` | Crown / Winter 2011 distribution family. |
| `SPECIES_SUICUNE` | `MOVE_SHEER_COLD` | `event_exclusive` | Crown / Winter 2011 distribution family. |
| `SPECIES_SUICUNE` | `MOVE_AIR_SLASH` | `event_exclusive` | Crown / Winter 2011 distribution family. |
| `SPECIES_CELEBI` | `MOVE_HOLD_BACK` | `event_exclusive` | Pokemon Bank Celebi special move candidate. |
| `SPECIES_VICTINI` | `MOVE_V_CREATE` | `event_exclusive` | Event-exclusive move family; exact species coverage needs full audit. |
| `SPECIES_PIKACHU` | `MOVE_HAPPY_HOUR` | `event_exclusive` | Event-exclusive move family; exact distribution variants need full audit. |
| `SPECIES_PIKACHU` | `MOVE_CELEBRATE` | `event_exclusive` | Birthday / special distribution family. |
| `SPECIES_PIKACHU` | `MOVE_HOLD_HANDS` | `event_exclusive` | Event-exclusive move family. |
| `SPECIES_EEVEE` | `MOVE_CELEBRATE` | `birthday_event` | Pokemon Center birthday sample. |
| `SPECIES_MILCERY` | `MOVE_CELEBRATE` | `birthday_event` | Japan / Singapore Pokemon Center birthday sample. |
| `SPECIES_PAWMI` | `MOVE_CELEBRATE` | `birthday_event` | Gen IX birthday sample. |
| `SPECIES_CHARCADET` | `MOVE_CELEBRATE` | `birthday_event` | Gen IX birthday sample. |
| `SPECIES_CHARIZARD` | `MOVE_HOLD_HANDS` | `event_exclusive` | Event distribution example. |
| `SPECIES_VIVILLON` | `MOVE_HOLD_HANDS` | `event_exclusive` | Event distribution example. |

## UX Contract

Special moves currently use a distinct `Sp` badge.
They should not be hidden inside the normal TM/tutor source because these moves
often exist only due to a specific distribution or side-game acquisition path.

The special source can contain many global entries. The UI should still only
show special entries that match the current Pokemon, so the existing 600+
candidate target remains driven by per-Pokemon stress cases rather than by the
total size of the external JSON.

The first implementation keeps special moves behind a separate config and keeps
unlock group metadata in JSON:

```text
P_UNIFIED_RELEARNER_SPECIAL_MOVES
special_xd
special_event
```

This lets story flags or debug flags expose special candidates without
requiring Mystery Gift emulation or event item injection.

## Open Questions

- Should event-exclusive novelty moves such as `Celebrate`, `Happy Hour`,
  `Hold Back`, and `Hold Hands` be enabled globally, or require a postgame /
  debug / special-event unlock group?
- Should XD purification moves be grouped separately from Wonder Card /
  distribution moves?
- Should region-locked distributions be enabled per region group, globally
  after audit, or only through debug / special-event unlocks?
- What audit statuses are acceptable for default gameplay: public-reference
  backed only, or also locally accepted checksum-backed entries?
- Should duplicate special entries be shown when the same move is already
  available through TM or tutor, or should source priority hide them?
- Should the first checked-in JSON include only audited official references, or
  also allow a local "custom compatibility" block for romhack-only decisions?
