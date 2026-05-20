# Summary Screen Flow v15

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `9376760f68`; `git describe` = `expansion/1.15.2-54-g9376760f68` |
| Code status | Docs-only source investigation |
| Provenance | Local source read and feature docs |

Summary Screen は複数の implementation shelf が共有する UI owner。
Summary Tera Type Icon、Pokemon State Editor、Unified Move Relearner、
Pre-Battle Team Viewer の Summary return path が同じ input / prompt / sprite /
palette / redraw 面に乗る。

## Source Map

| Area | File / symbol |
|---|---|
| Main owner | `src/pokemon_summary_screen.c` |
| Public mode/page definitions | `include/pokemon_summary_screen.h` |
| Summary configs | `include/config/summary_screen.h`, `include/config/pokemon.h` |
| Input task | `Task_HandleInput` |
| Move relearner prompt | `ShouldShowMoveRelearner`, `ShowRelearnPrompt`, `TryUpdateRelearnType` |
| Utility prompt | `ShowUtilityPrompt`, `ShouldShowRename`, `ShouldShowIvEvPrompt` |
| Type icon sprites | `CreateMoveTypeIcons`, `SetTypeIcons`, `SetMonTypeIcons`, `SetTypeSpritePosAndPal` |
| Pokemon icon helpers | `src/pokemon_icon.c`, `include/pokemon_icon.h` |
| Existing type/palette data | `gTypesInfo`, `gContestCategoryInfo`, `graphics/summary_screen/` |

## Current Master Behavior

`Task_HandleInput` owns Summary input when the screen is idle.

| Input | Current owner |
|---|---|
| D-pad up/down | Change selected Pokemon. |
| D-pad left/right | Change Summary page. |
| `A` on Info | Rename or close, depending on `ShouldShowRename()`. |
| `A` on Skills | Cycle Stats / IVs / EVs when enabled. |
| `A` on Moves | Enter move selection / switch mode. |
| `B` | Close Summary. |
| `START` on Battle / Contest Moves | Launch Move Relearner when `ShouldShowMoveRelearner()` is true. |
| `L` / `R` on Moves | Cycle relearner category prompt when summary relearner is enabled and not in battle. |
| `SELECT` | Debug Pokemon sprite visualizer only when enabled and not in battle. |

Current `master` has `P_SHOW_TERA_TYPE = GEN_8`, so the built-in
`if (P_SHOW_TERA_TYPE >= GEN_9)` Summary Tera path is disabled by default.
The implementation shelf #26 replaces that concept with a small credited Tera
badge and placement defines.

## Shared UI Surfaces

| Surface | Owner risk |
|---|---|
| Prompt area | Move Relearner uses `START RELEARN` on move pages. State Editor uses `START EDIT` on Skills in its branch. Keep page-specific prompt ownership explicit. |
| Input buttons | `START`, `L`, `R`, and `SELECT` are already meaningful. New Summary features must be page-gated and mode-gated. |
| Windows | Summary uses prompt windows and page-specific text windows. Overlay panels must not leave stale tilemaps or window buffers. |
| Sprites | Type icon sprites share `SPRITE_ARR_ID_TYPE` slots. Tera badge / type icon changes must hide stale sprites on eggs and page changes. |
| Palettes | Type icons use `gTypesInfo[type].palette`; contest category icons use `gContestCategoryInfo`. Overlay palettes must avoid clobbering Summary palettes. |
| Redraw | `PrintPageSpecificText`, `SetTypeIcons`, and page tilemap swaps drive refresh. Partial redraws need explicit clear bounds to avoid stale glyphs. |
| Callback return | Summary closes through callbacks such as `CB2_InitLearnMove`; custom overlays must restore Summary input cleanly before returning. |

## Feature Interactions

| Feature | Summary touch point | Integration rule |
|---|---|---|
| Summary Tera Type Icon | `SetMonTypeIcons` and type icon sprite slots. | Draw after normal type icons, hide on eggs, keep placement configurable, and record asset credit. |
| Pokemon State Editor | Skills page prompt/input and right-side overlay. | Own `START` only on Skills page; refresh `currentMon` / extracted summary data after edits; keep box Summary gated until validated. |
| Unified Move Relearner | Move page `START`, `L/R` source prompt, return callback. | If unified mode removes source category cycling, update prompt copy and input gating together. |
| Pokemon Icon UI | Shared icon/sprite lifetime rules for custom viewers and future Summary overlays. | Use `CreateMonIcon*` helpers for Pokemon icons and destroy/free palettes on close; do not mix with Summary type icon slots. |
| Pre-Battle Team Viewer | Opens standard Summary from viewer and returns to viewer. | Summary mode must lock move reordering unless explicitly enabled, and return with viewer selection state intact. |
| Party / Status UI Overhaul | Possible BW-style Summary replacement or alternate screen. | Do not import a full alternate Summary until party grid layout is stable. Preserve all `ShowPokemonSummaryScreen` modes, return callbacks, prompt ownership, and existing feature overlays. |

## BW Summary Reference Note

`docs/features/party_status_ui_overhaul/external_references.md` records
RavePossum's Emerald Extra BW Summary implementation as an external reference.
It exposes a separate `ShowPokemonSummaryScreen_BW` style surface and includes
its own graphics, config flags, IV/EV modes, type icons, friendship / shiny /
status icons, move selector, and callback handling.

Adoption rule:

- Treat it as a reference first, not a drop-in replacement.
- A full replacement needs image assets and tilemaps. Do not treat the source
  file alone as enough to implement the screen.
- If copied, reconcile with current `src/pokemon_summary_screen.c` changes,
  Summary Tera icon, State Editor, Unified Move Relearner, Team Viewer, and
  Scout Selection Summary return path.
- Record credits before importing any code or graphics.

## Integration Checklist

- Search `Task_HandleInput` before assigning a new Summary button.
- Keep feature inputs page-gated (`Info`, `Skills`, `Battle Moves`, `Contest Moves`) and mode-gated (`BOX_CURSOR`, `SELECT_MOVE`, in-battle).
- For any new prompt, document its window owner and what clears it.
- For any new sprite, document sprite slot, palette owner, egg / close / page-change hide path.
- For any overlay, test open, held input, cursor movement, close, and immediate Summary exit.
- For any feature using Summary as a return path, test both party Summary and the specific feature entry path.

## Open Questions

- Should Summary Tera badge and Pokemon State Editor share a single passive Tera display contract?
- Should unified relearner remove the L/R category prompt entirely when unified mode is on?
- Should future Summary overlays reserve a common palette range instead of each feature picking a palette independently?
- Should box Summary remain disabled for state editing until storage return-path tests exist?
