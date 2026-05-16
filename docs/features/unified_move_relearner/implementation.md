# Unified Move Relearner Implementation

## Status

Implemented on `feature/unified-move-relearner` as a guarded runtime feature.
The implementation keeps existing category-specific relearners available when
the unified config is disabled.

## What Changed

| Area | Change |
|---|---|
| Config | `P_UNIFIED_MOVE_RELEARNER` and per-source toggles were added in `include/config/summary_screen.h`; party menu relearner is enabled for this branch. |
| Candidate cap | `MAX_RELEARNER_MOVES` was raised to 640 and all unified appends use a cap guard. |
| Candidate model | Move Relearner candidates now carry `move + source`; menu ids are candidate indices rather than raw move ids. |
| Source labels | Compact row labels are shown as `Lv`, `Eg`, `TM`, and `Tu`; duplicate moves from different sources are preserved as separate rows. |
| Level moves | Unified mode includes all level-up moves up to `MAX_LEVEL`, not only the Pokemon's current level. |
| Generated historical pools | `tools/learnset_helpers/make_relearner_learnsets.py` generates ignored runtime data from `tools/learnset_helpers/porymoves_files/*.json` for historical egg / TM / tutor candidates. |
| Summary entry | Summary move page START opens unified mode; old L/R source cycling is suppressed while unified mode is active. |
| Party entry | Field party action menu gets a direct `RELEARN` action when unified candidates exist. |
| NPC/script entry | Common, Fallarbor, and Two Island relearner scripts set unified state when enabled. Script-mode return is forced through `RELEARN_MODE_SCRIPT` after PC/party selection. |
| Long list UX | The existing list is retained, but D-pad left/right page-scroll is enabled for unified long lists. |

The generated header is intentionally ignored:

```text
src/data/pokemon/unified_relearner_learnsets.h
```

This keeps broad generated learnset data out of hand-edited source diffs while
still making normal builds reproducible.

## Runtime Notes

- Mew was used as the broad-candidate stress species. The generated historical
  pool gives Mew hundreds of TM/tutor candidates, including a visible transition
  from level rows to TM rows and then tutor rows.
- The physical TM item list was not expanded. The TM rows in unified mode are a
  virtual relearner candidate pool.
- Special event / XD / distribution-only candidates remain documented seed data
  only. They are not connected to runtime candidate generation in this slice.
- Story/rank/clear-flag virtual TM unlock gating is still future work. Current
  unified TM/tutor source toggles are build-time config gates.

## Validation

| Check | Result | Notes |
|---|---|---|
| `rtk make -j16 -O debug` | Pass | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O all` | Pass | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O check` | Pass | Existing linker warning on test ROM link; suite output includes expected `EXPECTED_FAIL` / `KNOWN_FAILING` markers and exits 0. |
| mGBA Live boot / Continue | Pass | Debug ROM loaded the temporary Mew save via Continue. |
| mGBA party entry | Pass | Start menu -> Pokemon -> Mew showed direct `RELEARN` action and opened unified list. |
| mGBA long-list labels | Pass | Mew list showed `Lv` source rows, D-pad right page-scroll reached `TM`, then `Tu` rows. |
| mGBA NPC/script cancel | Pass | Debug menu `Party -> Move Relearner`, cancel from list, confirmed give-up, then returned to a YES/NO `Anything else` prompt instead of the old category multichoice. |
| mGBA cleanup | Pass | `mgba_live_stop` succeeded and `mgba-live-cli status --all` returned `[]`. |

Screenshots:

- `/tmp/unified_move_relearner_mew_level_labels.png`
- `/tmp/unified_move_relearner_mew_tutor_labels.png`
- `/tmp/unified_move_relearner_script_yesno.png`

GitHub Actions were not re-waited locally; this branch relies on the local
build/check and mGBA evidence above before push.

## Remaining Risks

- Actual move learning / overwrite selection should still get a focused manual
  pass before merge. This session validated list rendering, page navigation, and
  cancel/return behavior.
- The single list is now safe and page-scrollable, but it is still not as fast
  as a true source-tab or search UX for 600+ future candidates.
- Runtime generation currently uses broad historical porymoves data without
  per-generation allow-list filtering.
- Distribution-only special moves need a separate runtime data table before they
  can appear in the unified list.
