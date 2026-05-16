# Unified Move Relearner Test Plan

## Build / Lint

| Test | Command | Expected |
|---|---|---|
| Diff check | `rtk git diff --check` | No whitespace errors. |
| Normal ROM | `rtk make -j16 -O all` | Build passes. |
| Debug ROM | `rtk make -j16 -O debug` | Build passes, because debug route / mGBA validation is expected. |
| Focused checks | `rtk make -j16 -O check` | Run when candidate helpers or generated data change. |
| Docs | `rtk mdbook build docs` | Docs build, with any existing warnings recorded. |

## Focused Tests

| Test | Steps | Expected |
|---|---|---|
| All level-up moves | Use a low-level Pokemon with a higher-level learnset move. | Higher-level move appears and can be learned. |
| Egg move inclusion | Use a Pokemon with egg moves and unlock egg source. | Egg moves appear in unified candidates. |
| Current TM/HM inclusion | Use a Pokemon compatible with a registered TM/HM. | Candidate follows selected TM policy. |
| Tutor inclusion | Use a Pokemon compatible with a generated tutor such as `MOVE_MEGA_PUNCH`. | Tutor move appears when tutor source is enabled. |
| Source duplicates | Pick a move legal through multiple sources. | Move appears once per source, with visible source labels. |
| Already-known filtering | Give the Pokemon one candidate move before opening the list. | Known move is hidden from every source entry. |
| Candidate cap | Use Mew / ALL_TEACHABLES with broad sources. | No memory corruption; overflow policy is visible and stable. |
| 250-300 virtual TM stress | Enable or mock a broad virtual TM candidate pool. | Mew's candidates fit the chosen UI/storage policy; pagination/tab/cap behavior is deterministic. |
| Long-list navigation | Open Mew with 250-300 virtual TM candidates. | User can reach beginning, middle, and end chunks without excessive single-list scrolling. |

## Manual Checks

| Check | Steps | Expected |
|---|---|---|
| Summary entry | Open Pokemon Summary moves page and press START. | Unified relearner opens and returns to the same Summary context on cancel / learn. |
| Party menu entry | Open field party menu and select Move Relearner action. | Relearner opens and returns to party menu coherently. |
| NPC script entry | Use a relearner NPC. | Cost / condition is checked by script, and item removal only happens after successful learning. |
| Source overlap | Use a move present in both virtual TM and tutor / tower pools. | Both source entries appear and teach the same move; labels make the source clear. |
| Rank / clear-flag unlock | Toggle a virtual TM unlock group or individual move. | Newly unlocked TM candidates appear without adding physical TM items. |
| Story reward copy | Trigger a story unlock event. | Text communicates move availability unlock, not receiving a TM item. |
| Cancel from move list | Select Cancel and confirm give-up. | Returns without charging or changing moves. |
| Cancel during overwrite | Pick a move on a full moveset, enter overwrite Summary, then cancel. | Returns cleanly without stale callbacks or wrong success flag. |
| Box Summary path | Open from a boxed Pokemon Summary if supported. | Box mon updates / returns without party-slot confusion. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-16 | Docs-only investigation | Not run | No source changes in this docs refresh. |

## Feature Complete Gate

- Build and focused tests are recorded.
- mGBA Live evidence covers at least one learn path and one cancel path.
- Candidate overflow behavior is validated with a high-candidate species.
- Docs record any skipped long GitHub Actions wait and remaining manual checks.

## Open Questions

- Which debug command should create the canonical Mew / broad-candidate test route?
- Should the stress route mock 250-300 virtual TM candidates before the final generated pool exists?
- Should the mGBA route use an existing NPC, Summary shortcut, party shortcut, or all three?
