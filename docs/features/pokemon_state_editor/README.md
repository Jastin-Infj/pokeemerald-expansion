# Pokemon State Editor

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Working branch | `feature/pokemon-state-editor-expansion` / completed shelf #23 |
| Baseline | `master` `ab5abcad53`; `git describe` = `expansion/1.15.2-44-gab5abcad53` |
| Code status | MVP implemented; completed shelf #23 closed 2026-05-17 after CI success; not on `master` |
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
- matched header/footer band height and a slightly lower right-pane anchor so the
  Skills page title area remains visible;
- move-info-style right-edge slide-in and slide-out when opening or canceling the
  editor;
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
| Layout | Right-side Summary pane overlay; left-side Pokemon sprite and Skills title area remain visible. |
| Coordinate/color tuning | Put editor window, text positions, band height, slide speed, RGB palette colors, fill color, and level policy behind `#define`s. |
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

## Future Training Backlog

These are recommended follow-ups if the editor becomes the central place for tedious
training setup:

| Area | Recommended editor support |
|---|---|
| EV spreads | Add spread presets such as clear all, `252/252/4`, bulky split, and one-button `0 Atk` / `0 Spe`. This addresses mid-range editing better than holding D-pad. |
| EV allocation bars | Add slider/range-bar style EV controls for 0..252 per stat with a visible 510 total budget. This is the closest fit to the Pokemon Champions-style UX, but should be a later UI slice because it needs custom bar drawing, cursor behavior, and total-budget feedback. |
| Direct numeric entry | Add a compact value picker for EVs/IVs/level/friendship so values like `148` or `196` are not repetitive. |
| Hyper Training | Add a page or toggle group for `MON_DATA_HYPER_TRAINED_*`, separate from raw IVs, so battle-effective IV setup can preserve raw IV data. |
| Pokerus | Add optional strain/days controls for EV-training test setup when Pokerus is enabled. |
| PP training | Add PP Up / PP Max state controls once move editing lands, because it is tied to move slots rather than base Pokemon identity. Preferred UX is a Champions-like `PP TRAINING` page with `Move 1` through `Move 4` rows, each cycling `0`, `1`, `2`, `3`, and `MAX` where supported by the move data model. |
| Training templates | Add copy/paste or apply-template support for common competitive profiles after direct entry exists. |
| Passive Tera display | Show current Tera type/status on Summary near the existing type UI; keep this in a separate branch from editor value controls. |
| Editor feedback | Revisit sound and commit semantics. Opening/closing currently uses the selection SE and value changes use immediate apply, but the UX may need clearer value-change sounds and an explicit `DONE` / `APPLY` affordance so closing the editor does not feel like an accidental commit. |

## Next Review Notes

These notes remain open for a later UI/data polish pass:

- Investigate EV total text redraw artifacts. When an EV value is raised into
  multi-digit values such as `252` and then reduced again, a stray hyphen/underscore-
  like glyph can appear around the ones/tens area and the total line can briefly
  black out. Canceling and reopening the editor restores the display, so the next
  pass should focus on stale glyph clearing or partial-copy bounds for the EV total
  row.
- A separate future branch should expose the Pokemon's Terastal status/type on the
  normal Summary UI. Preferred placement is to the right of the existing type display,
  with either a small overlaid UI element or a new compact icon slot. This should be
  handled separately from the state editor controls so the editor remains focused on
  modifying values, while Summary can passively show the current Tera type.

## Child Docs

- [Dependencies](dependencies.md)
- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Implementation](implementation.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
