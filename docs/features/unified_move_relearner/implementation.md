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
| Source labels | Compact row labels are shown as `Lv`, `Eg`, `TM`, `Tu`, and `Sp`; duplicate moves from different sources are preserved as separate rows. |
| Level moves | Unified mode includes all level-up moves up to `MAX_LEVEL`, not only the Pokemon's current level. |
| Generated historical pools | `tools/learnset_helpers/make_relearner_learnsets.py` generates ignored runtime data from `tools/learnset_helpers/porymoves_files/*.json` for historical egg / TM / tutor candidates. |
| Special pool | `tools/learnset_helpers/special_relearner_moves.json` adds project-owned runtime data for special event, distribution-only, XD purification, Ranger-transfer, and form-specific move candidates. |
| Supplemental form species | `make_relearner_learnsets.py` now emits species/form slots that are present in porymoves or special JSON but missing from `all_teaching_types.json`, as long as the `SPECIES_*` constant resolves to a unique numeric slot. |
| Summary entry | Summary move page START opens unified mode; old L/R source cycling is suppressed while unified mode is active. |
| Party entry | Field party action menu gets a direct `RELEARN` action when unified candidates exist. |
| NPC/script entry | Common, Fallarbor, and Two Island relearner scripts set unified state when enabled. Script-mode return is forced through `RELEARN_MODE_SCRIPT` after PC/party selection. |
| Long list UX | The existing list is retained, but D-pad left/right page-scroll is enabled for unified long lists. |

Candidate generation details are recorded in
[Candidate Data Flow](candidate_data_flow.md).

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
- Arceus is the focused special-move smoke species. The runtime special pool
  currently gives it Movie 12 moves (`Roar of Time`, `Spacial Rend`,
  `Shadow Force`) plus Dahara City moves (`Blast Burn`, `Hydro Cannon`,
  `Earth Power`).
- The special runtime JSON was expanded from 25 candidate blocks / 50 moves to
  174 blocks / 216 moves. The pass added the broader XD purification table,
  event-exclusive move-page rows, Wish Egg / legacy rows, Cinema Genesect
  special moves, Rotom appliance moves, and Cosplay Pikachu costume moves.
- Rotom form species now get generated form-specific TM/tutor pools from
  porymoves plus their appliance move as `Sp`: Heat/Overheat, Wash/Hydro Pump,
  Frost/Blizzard, Fan/Air Slash, and Mow/Leaf Storm.
- Cosplay Pikachu forms are emitted as special-only supplemental species because
  porymoves does not include separate costume form keys. Their costume moves are
  `Sp`: Rock Star/Meteor Mash, Belle/Icicle Crash, Pop Star/Draining Kiss,
  PhD/Electric Terrain, and Libre/Flying Press.
- Bellossom's current Gen 9 level-up learnset already includes `Moonblast`;
  the unified level source reads compiled current level data up to `MAX_LEVEL`,
  so Bellossom should expose that move unless it already knows it.
- The physical TM item list was not expanded. The TM rows in unified mode are a
  virtual relearner candidate pool.
- Special event / XD / distribution-only candidates are connected as one
  source labeled `Sp`. The JSON still retains `unlockGroup`, distribution, and
  source-ref metadata for later gating / audit work.
- Story/rank/clear-flag virtual TM unlock gating is still future work. Current
  unified TM/tutor source toggles are build-time config gates.

## Validation

| Check | Result | Notes |
|---|---|---|
| `rtk make -j16 -O debug` | Pass | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O all` | Pass | Existing linker warning: `LOAD segment with RWX permissions`. |
| `rtk make -j16 -O check` | Pass | Existing linker warning on test ROM link; suite output includes expected `EXPECTED_FAIL` / `KNOWN_FAILING` markers and exits 0. |
| Runtime special JSON audit | Pass | 31 source refs, 174 candidate blocks, 216 moves, and no unknown species or move constants. |
| mGBA Live boot / Continue | Pass | Debug ROM loaded the temporary Mew save via Continue. |
| mGBA party entry | Pass | Start menu -> Pokemon -> Mew showed direct `RELEARN` action and opened unified list. |
| mGBA long-list labels | Pass | Mew list showed `Lv` source rows, D-pad right page-scroll reached `TM`, then `Tu` rows. |
| mGBA special labels | Pass | Arceus party `RELEARN` route showed `Roar of Time`, `Spacial Rend`, `Shadow Force`, `Blast Burn`, `Hydro Cannon`, and `Earth Power` as `Sp` rows. |
| mGBA expanded special labels | Pass | Debug menu `Give X -> Pokemon (Basic)` created Bulbasaur; party `RELEARN` page-scrolled to `Celebrate` as a new `Sp` row. |
| mGBA Bellossom Moonblast check | Pass | Debug-created Lv1 Bellossom already knew `Moonblast`; party `RELEARN` hid it as an already-known move. |
| mGBA form-specific labels | Pass | Debug-created Wash Rotom showed `Hydro Pump` as `Sp`; debug-created Pikachu Libre showed `Flying Press` as `Sp`. |
| mGBA NPC/script cancel | Pass | Debug menu `Party -> Move Relearner`, cancel from list, confirmed give-up, then returned to a YES/NO `Anything else` prompt instead of the old category multichoice. |
| mGBA cleanup | Pass | `mgba_live_stop` succeeded and `mgba-live-cli status --all` returned `[]`. |

Screenshots:

- `/tmp/unified_move_relearner_mew_level_labels.png`
- `/tmp/unified_move_relearner_mew_tutor_labels.png`
- `/tmp/unified_move_relearner_arceus_special_roar_time.png`
- `/tmp/unified_move_relearner_arceus_special_labels.png`
- `/tmp/unified_move_relearner_special_expanded_bulbasaur_after_hold_right4.png`
- `/tmp/unified_move_relearner_bellossom_list_no_known_moonblast.png`
- `/tmp/unified_move_relearner_bellossom_moonblast_known_summary.png`
- `/tmp/unified_move_relearner_rotom_wash_hydro_pump_sp.png`
- `/tmp/unified_move_relearner_pikachu_libre_flying_press_sp.png`
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
- Special source currently uses one `Sp` display label. The runtime JSON keeps
  `display` / `unlockGroup` metadata, but per-entry labels and story/rank
  unlock gating are future work.
- Some special JSON entries are deliberate seed candidates with audit notes.
  They should receive a full distribution-data pass before treating the list as
  complete.
