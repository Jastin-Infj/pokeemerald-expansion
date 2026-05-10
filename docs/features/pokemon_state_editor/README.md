# Pokemon State Editor

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-11 |
| Working branch | `feature/pokemon-state-editor-expansion` |
| Baseline | `master` `ab5abcad53`; `git describe` = `expansion/1.15.2-44-gab5abcad53` |
| Code status | MVP implemented; UI polish and extra status fields added |
| Provenance | User request, local code review, and Pokemon Champions UI reference review |

## Goal

Add a Pokemon state editor that is reached from the normal party Summary flow,
starting from the "Check summary" path rather than a debug-only menu.

The first implementation should cover the fields the user specifically called out:

- EVs: edit each stat smoothly from 0 to `MAX_PER_STAT_EVS` with a quick max path.
- IVs: edit each stat smoothly from 0 to `MAX_PER_STAT_IVS`.
- Nature: change the displayed/stat nature without forcing a raw personality rewrite.
- Ability: change between valid species ability slots.
- Friendship: edit 0..`MAX_FRIENDSHIP`.
- Gender: change male/female when the species gender ratio allows it, while keeping
  encrypted Pokemon substructs valid.
- Level: edit 1..current level cap, or 1..100 when no cap is active.
- Pokeball: edit the caught ball.
- Dynamax/Tera/Gigantamax: edit Dynamax level, Tera type when not forced by species,
  and Gigantamax factor.

Move editing is explicitly out of scope for this slice. It should remain a separate
feature so this editor does not compete with the move relearner work.

## UX Direction

The requested direction is closer to a Pokemon Champions-style training/status editor
than to the existing raw debug numeric editor. On GBA this should be interpreted as:

- a Summary-screen-launched editor, not a standalone debug submenu;
- multiple compact pages instead of one overloaded page;
- clear page labels and value rows;
- quick min/max controls for repetitive EV/IV editing;
- a dedicated Summary/interface-style menu panel instead of a plain white standard
  frame or a single dark rectangle;
- a right-side editor pane that preserves the Pokemon sprite area on the left;
- readable light text on dark body rows, with a clear selected-row highlight and
  separate header/footer bands;
- stable held-input rendering, with row-level redraws instead of full-panel redraws
  for every parameter change;
- coordinates controlled by defines so the user can tune layout without hunting
  through the implementation.

The MVP can be original UI. It does not need to clone Pokemon Champions visuals, but
it should avoid the current pain point where reaching 252 requires repeated +10 edits.

Reference pages checked for visual/product direction:

- <https://www.pokemonchampions.jp/>
- <https://www.pokemonchampions.jp/ja/battle/>
- <https://www.pokemonchampions.jp/ja/pokemon/>

## Scope

| Area | MVP decision |
|---|---|
| Entry point | Summary Screen, opened from the party menu path. |
| Launch button | Skills page `START EDIT` prompt in the top-right Summary prompt area. |
| Editor pages | Five editor pages: EVs, IVs, Core, Dynamax/Tera, Gender/Friendship. |
| Layout | Right-side Summary pane overlay; left-side Pokemon sprite area remains visible. |
| Coordinate tuning | Put editor window, text positions, palette, fill color, and level policy behind `#define`s. |
| Save data | No new save fields. Edit existing Pokemon fields only. |
| Party vs box | Party Summary first. Box Summary is a follow-up unless the same helper is proven safe. |
| Moves | Out of scope. |
| Origin/met data | Follow-up; needs valid location UX and summary memo validation. |

## Current Local Building Blocks

| Area | Existing symbol / file |
|---|---|
| Summary entry and input | `src/pokemon_summary_screen.c` `Task_HandleInput` |
| Summary mode/page state | `include/pokemon_summary_screen.h` |
| Summary config | `include/config/summary_screen.h` |
| Pokemon fields | `include/pokemon.h` `MON_DATA_*` |
| Stat recalculation | `src/pokemon.c` `CalculateMonStats` |
| Level caps | `include/config/caps.h`, `include/caps.h`, `GetCurrentLevelCap` |
| EV/IV caps | `include/constants/pokemon.h` `MAX_PER_STAT_EVS`, `MAX_TOTAL_EVS`, `MAX_PER_STAT_IVS` |
| Nature display/stat source | `MON_DATA_HIDDEN_NATURE`, `GetNature` |
| Ability slots | `MON_DATA_ABILITY_NUM`, `GetSpeciesAbility`, `NUM_ABILITY_SLOTS` |
| Gender derivation | `GetGenderFromSpeciesAndPersonality`, species `genderRatio` |
| Friendship cap | `MON_DATA_FRIENDSHIP`, `MAX_FRIENDSHIP` |
| Pokeballs | `MON_DATA_POKEBALL`, `gPokeBalls`, `POKEBALL_COUNT` |
| Battle gimmicks | `MON_DATA_DYNAMAX_LEVEL`, `MON_DATA_TERA_TYPE`, `MON_DATA_GIGANTAMAX_FACTOR`, `gTypesInfo` |
| Existing debug references | `src/debug.c`, `data/scripts/debug.inc` state-setting helpers |

## Assumptions

- The first editor is a Summary overlay, not a new Summary page in `PSS_PAGE_COUNT`.
  This keeps existing Summary tilemap paging stable.
- The implementation edits party Pokemon only at first. Box Pokemon editing is blocked
  on storage return-path review.
- Nature uses hidden nature. Gender uses a controlled personality low-byte rewrite
  through the personality helper and preserves shiny state plus visible/stat nature
  through the modifier fields.
- EV total remains capped at `MAX_TOTAL_EVS`; quick max on a stat clamps to the
  available remaining total.
- Ability rows skip `ABILITY_NONE` slots.
- Level writes matching EXP plus `MON_DATA_LEVEL`, then recalculates stats. If
  `P_SUMMARY_STATE_EDITOR_LEVEL_CAP` is true, max level comes from
  `GetCurrentLevelCap`.
- Pokeball cycles normal ball ids from `BALL_POKE` through `POKEBALL_COUNT - 1`.
- Tera type skips special-case types and locks when species forces a Tera type.
- Gigantamax factor is a raw on/off status flag; species legality policy is left to
  a follow-up.

## Open Questions

These do not block the MVP, but should be revisited after runtime validation:

- Should Box Summary enable the same editor after storage-specific validation?
- Should personality-derived visuals/forms such as Spinda spot layout or Unown letter
  get a warning or a species-specific block before enabling gender edits?
- Should origin/met location editing be its own page after a valid mapsec picker exists?
- Should Gigantamax factor be locked to species that can actually Gigantamax?
- Should mid-range EV editing add larger step sizes or direct numeric entry beyond
  held D-pad plus `L`/`R` min/max?
- Should the final UI get custom art work to move closer to Pokemon Champions?

## Next Review Notes

These notes are intentionally not implemented in this commit. Revisit them when the
next UI pass is requested:

- Entry/exit animation should feel like the Summary move-info panel sliding in from
  the right edge and returning to the right edge on cancel. The current behavior is
  acceptable for this checkpoint, but reads more like a delayed draw or fade than a
  true lateral slide. The next pass should also test a slightly faster duration.
- The right-pane layout is preferred, but the panel should be nudged to avoid
  covering the `POKEMON SKILLS` title area. A likely anchor is the green header band
  below `START EDIT` / near the purple `NEXT LV.` background, so the editor feels
  seated in the existing Summary composition.
- Investigate EV total text redraw artifacts. When an EV value is raised into
  multi-digit values such as `252` and then reduced again, a stray hyphen/underscore-
  like glyph can appear around the ones/tens area and the total line can briefly
  black out. Canceling and reopening the editor restores the display, so the next
  pass should focus on stale glyph clearing or partial-copy bounds for the EV total
  row.

## Child Docs

- [Dependencies](dependencies.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
