# Party / Status UI Overhaul

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-21 |
| Baseline | `master` `b2d64f1577`; runtime branch `feature/party-status-ui-overhaul-20260521` |
| Code status | Runtime first slice implemented on feature branch |
| Provenance | User layout requirement, local source inspection, GitHub reference search |

## Goal

Future party / Pokemon status screens may move away from the current Emerald
layout and toward a BW / DS-style UI.

Confirmed layout intent for the party screen:

- do not keep the current `1 + 5` shape where slot 1 is large and the other
  five slots are stacked to the right;
- target six equal-ish party slots in a `2 columns x 3 rows` grid;
- read order should be `2 / 2 / 2`: row 1 slots 1-2, row 2 slots 3-4, row 3
  slots 5-6, unless battle-order mode explicitly requires a different mapping.

The first runtime slice implements the party screen grid in source without
importing external graphics. See `implementation.md` and `test_plan.md` for the
current branch status.

## Asset Requirement

This feature is not implementable as a code-only layout patch if the goal is a
polished UI. New or imported image assets are expected.

Expected asset groups:

| Asset group | Needed for | Notes |
|---|---|---|
| Party grid background | `2 x 3` party screen | The current `graphics/party_menu/bg.png` and tilemaps are designed around the `1 + 5` layout. |
| Equal slot frames | six same-priority party slots | Existing `slot_main` / `slot_wide` assets encode different left / right box shapes. A grid needs a new slot family plus empty / no-HP variants if those states are kept. |
| Confirm / Cancel placement assets | choose-half and action flows | Existing buttons may be reusable, but their position must be checked against the third row. |
| BW Summary page tiles / tilemaps | full status screen replacement | A full BW Summary import needs page tilemaps, button sprites, move selector, status / shiny / friendship / category icons, and possibly BW type icons. |
| Palette plan | all UI assets | GBA palette pressure must be checked; imported indexed PNGs still need local palette ownership and mGBA visual validation. |

Policy:

- Do not put PNG / BIN / palette assets into a docs-only branch or `master`
  docs merge.
- Import assets only on a runtime / implementation branch with the code that
  consumes them.
- Before importing external art, record source URL, author / handle, license or
  usage note, exact files copied, and required credit.
- If license is unclear, use the reference only for visual direction and create
  new local assets instead.

## Local Dependency Map

| Area | Files / symbols | Notes |
|---|---|---|
| Party layout constants | `include/constants/party_menu.h` `PARTY_LAYOUT_*` | Current layouts are `SINGLE`, `DOUBLE`, `MULTI`, `MULTI_SHOWCASE`. A grid layout likely needs a new `PARTY_LAYOUT_GRID_2X3` rather than overloading battle layouts. |
| Party layout data | `src/data/party_menu.h` | Owns window templates, sprite coordinates, slot tilemaps, info rects, and palette offsets. The current `SINGLE` layout is the `1 + 5` shape. |
| Party cursor movement | `src/party_menu.c` `UpdateCurrentPartySelection`, `UpdatePartySelectionSingleLayout`, `UpdatePartySelectionDoubleLayout` | Movement is not fully data-driven. A `2 x 3` grid needs its own traversal, especially for empty slots, Confirm, Cancel, and choose-half. |
| Party box drawing | `src/party_menu.c` `RenderPartyMenuBox`, `BlitBitmapToPartyWindow_LeftColumn`, `BlitBitmapToPartyWindow_RightColumn` | Existing code has "left main" and "right wide" slot families. Equal six-slot layout probably needs a third slot family. |
| Party icons | `src/pokemon_icon.c`, `src/party_menu.c` `CreatePartyMonIconSprite*` | Reusable, but all per-slot icon / item / status / Pokeball coordinates must be regenerated. |
| Summary screen | `src/pokemon_summary_screen.c`, `include/pokemon_summary_screen.h`, `include/config/summary_screen.h` | A BW-style summary screen can be an alternate implementation or a refactor of the current owner. It must preserve all existing entry modes and return callbacks. |
| Summary shared features | Summary Tera icon, Pokemon State Editor, Unified Move Relearner, Pre-Battle Team Viewer, Scout Selection Summary preview | These features already depend on Summary input, prompt, sprite, and return behavior. A full UI replacement must test them together. |

## SaveBlock Position

Party/status UI overhaul should be treated as visual / runtime UI work by
default. It should not consume SaveBlock capacity unless it adds a persisted
user option such as "Classic / BW party screen".

Policy:

- MVP uses compile-time config or a single runtime owner, not a new saved
  option.
- If a saved UI skin option is required later, allocate it through
  `SaveBlock2` option policy in `docs/flows/save_data_flow_v15.md`.
- Do not combine this UI work with Champions run-session SaveBlock changes.
  Champions needs dedicated party / bag snapshot state; party/status UI should
  not share that allocation.

## Implementation Shape

First runtime slice:

1. Add a new party layout constant for `2 x 3`. Done on
   `feature/party-status-ui-overhaul-20260521`.
2. Add a six-slot window template and sprite coordinate table. Done.
3. Add a grid cursor traversal helper instead of patching the current single /
   double movement logic. Done.
4. Keep the existing Summary screen in place. Done.
5. Validate field party menu, item-use party menu, choose-half, in-battle party
   menu, and return-from-Summary paths. In progress in `test_plan.md`.

This slice deliberately avoids importing PNG / BIN assets. It adopts the
Emerald Extra equal-column party slot tile-number pattern as a code reference
and builds a `14 x 5` framed slot from existing party menu background tiles.
New imported graphics remain future polish work, not a dependency of this
initial layout implementation.

Only after the party grid is stable should a BW-style Summary replacement be
considered.

## Open Questions

- Should `2 x 3` become the default field party layout only, or also replace
  choose-half and in-battle layouts?
- Should battle-order layouts keep the current left/right emphasis for active
  battlers, or also use the grid?
- Should BW Summary be a full alternate screen (`ShowPokemonSummaryScreen_BW`)
  or a refactor of the existing Summary owner?
- Do we want a persistent UI skin option, or is the new UI always-on?
- Do we import the RavePossum / Emerald Extra asset set with credits, or create
  a new local asset set tailored to the `2 x 3` layout?

## Validation Targets

Future runtime branch must prove:

- all six party slots draw in `2 / 2 / 2` order;
- empty slots do not break cursor traversal;
- A/B/START behavior still works for normal party, item target, choose-half,
  and Confirm / Cancel;
- Summary opens and returns to the same party slot;
- in-battle party order still maps to the correct `gPlayerParty` slot;
- mGBA Live screenshots cover the normal field party screen and at least one
  Summary return path.
