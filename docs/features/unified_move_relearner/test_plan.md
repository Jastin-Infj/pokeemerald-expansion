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
| Special inclusion | Use a Pokemon with runtime special candidates, such as Arceus. | Event / distribution-only moves appear with `Sp` source labels. |
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
| Special seed smoke | Use Arceus and page-scroll near the end of the unified list. | `Roar of Time`, `Spacial Rend`, `Shadow Force`, `Blast Burn`, `Hydro Cannon`, and `Earth Power` are reachable as `Sp` entries if not already known. |
| Rank / clear-flag unlock | Toggle a virtual TM unlock group or individual move. | Newly unlocked TM candidates appear without adding physical TM items. |
| Story reward copy | Trigger a story unlock event. | Text communicates move availability unlock, not receiving a TM item. |
| Cancel from move list | Select Cancel and confirm give-up. | Returns without charging or changing moves. |
| Cancel during overwrite | Pick a move on a full moveset, enter overwrite Summary, then cancel. | Returns cleanly without stale callbacks or wrong success flag. |
| Box Summary path | Open from a boxed Pokemon Summary if supported. | Box mon updates / returns without party-slot confusion. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-16 | Docs-only investigation | Not run | No source changes in this docs refresh. |
| 2026-05-16 | `rtk make -j16 -O debug` | Pass | Existing linker warning: `LOAD segment with RWX permissions`. |
| 2026-05-16 | `rtk make -j16 -O all` | Pass | Existing linker warning: `LOAD segment with RWX permissions`. |
| 2026-05-16 | `rtk make -j16 -O check` | Pass | First run hit transient `open tmpfd failed: File exists`; immediate rerun passed. Existing linker warning on test ROM link; suite output includes expected `EXPECTED_FAIL` / `KNOWN_FAILING` markers and exits 0. |
| 2026-05-16 | mGBA Live party entry | Pass | Continue-loaded Mew save; Start menu -> Pokemon -> Mew showed `RELEARN` and opened the unified list. |
| 2026-05-16 | mGBA Live long-list navigation | Pass | Mew list showed `Lv`, then page-scrolled through `TM` to `Tu`; screenshots in `/tmp/unified_move_relearner_mew_level_labels.png` and `/tmp/unified_move_relearner_mew_tutor_labels.png`. |
| 2026-05-16 | mGBA Live special seed | Pass | Continue-loaded save, created Arceus with debug `Give X -> Pokemon (Basic)`, opened party `RELEARN`, and confirmed `Roar of Time`, `Spacial Rend`, `Shadow Force`, `Blast Burn`, `Hydro Cannon`, and `Earth Power` as `Sp` entries. Screenshots: `/tmp/unified_move_relearner_arceus_special_roar_time.png`, `/tmp/unified_move_relearner_arceus_special_labels.png`. |
| 2026-05-16 | mGBA Live NPC/script cancel | Pass | Debug menu `Party -> Move Relearner`; cancel returned to YES/NO `Anything else`, not the old category multichoice. Screenshot: `/tmp/unified_move_relearner_script_yesno.png`. |
| 2026-05-16 | mGBA Live cleanup | Pass | `mgba_live_stop` succeeded and `mgba-live-cli status --all` returned `[]`. |

## Feature Complete Gate

- Build and focused tests are recorded.
- mGBA Live evidence covers at least one learn path and one cancel path.
- Candidate overflow behavior is validated with a high-candidate species.
- Docs record any skipped long GitHub Actions wait and remaining manual checks.

## Open Questions

- Which debug command should create the canonical Mew / broad-candidate test route?
- Should the canonical Mew route keep using debug menu `Give -> Pokemon (Basic)`, or should this branch add a dedicated Lua/debug shortcut?
- Should actual learn / overwrite confirmation be validated through party, summary, and NPC routes before merge?
- Should the next UX slice add source tabs once Gen 10-scale 600+ candidate capacity becomes the target?
- Should special source labels stay as one `Sp` badge, or split into per-entry
  `EV` / `XD` / `RG` / birthday labels once the candidate list is audited?
