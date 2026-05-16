# Unified Move Relearner Risks

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Candidate overflow | High | Mew or broad teachable species can write past `MAX_RELEARNER_MOVES`. | Add append helper with cap checks before unified aggregation. |
| Generation filtering gap | High | Current generated learnables lose source generation, so Gen 1 OK / Gen 2 OK config cannot be implemented cleanly at runtime. | Either defer to future generator work or generate per-source metadata. |
| TM policy mismatch | High | "All TM" can mean bag-owned, registered physical machines, historical machines, virtual candidates, or story-unlocked virtual candidates. | Add explicit config names and document defaults before implementation. |
| Virtual TM pool scaling | High | 250-300 relearner candidates pressure candidate storage and UI even when no TM items are added. | Keep relearner candidate data independent from physical item storage and design pagination/source tabs/caps around Mew. |
| Long-scroll UX | High | Existing list UI shows only a small visible slice, so scrolling through 250-300 candidates is slow even if storage is safe. | Add source tabs or 50-60 entry chunks before treating the feature as shippable. |
| Optional unlock save state | Medium | Always-available virtual TMs need no save data, but story/rank-gated virtual TMs need compact persistent state. | Use move/group bitsets only if progression gating is required; document save compatibility. |
| Unlock metadata drift | Medium | A virtual TM move can belong to TM history, generation group, rank group, and story flag at once. | Keep unlock metadata generated/table-driven and avoid scattering checks through UI code. |
| Reward copy mismatch | Low | If story text says the player received a TM item, the runtime model is misleading. | Write event copy as "TM-family moves are now available to relearn" or similar unlock language. |
| Cancel / return regression | High | Existing flow reuses `gSpecialVar_0x8004` for both selected slot and success flag in script flows. | Test cancel from every menu path and preserve success-only item removal. |
| Summary state cycling | Medium | Current L/R behavior assumes separate source states. | In unified mode, disable cycling or make it cycle source tabs intentionally. |
| Party menu UX drift | Medium | Existing party path creates a `MOVES` submenu. A direct action may conflict with field moves / summary action order. | Keep direct action guarded and test normal field party menu. |
| Source-duplicate storage | High | Existing candidate storage is move-only, so preserving TM + tower duplicates needs more metadata than `u16 move`. | Store candidate entries with move and source, and use candidate index as the menu id if needed. |
| Historical move availability | Medium | Some historical moves may not exist as enabled move constants or may be signature-only. | Generated source should skip unavailable moves or fail clearly. |
| Generated file churn | Medium | `teachable_learnsets.h` and `tutor_moves.h` are generated and can change broadly. | Do not hand-edit generated files. Isolate generator changes and inspect diffs. |

## Impact Notes

- This feature touches central UI and script entry points: Summary, party menu,
  event scripts, and the Move Relearner task.
- Existing docs already identify TM item expansion as a separate high-risk
  feature. Unified relearner should not imply 250-300 physical TMs.
- If Gen allow-list filtering is accepted for MVP, the feature becomes partly a
  build-tool / generated-data change, not just runtime UI.

## Accepted Risks

- MVP may use a generated virtual TM candidate pool and current generated tutor
  data while leaving physical TM item expansion out of scope.
- MVP should not hide source labels, because duplicate source rows can otherwise
  look like accidental repeated move names.
- The first implementation accepts a page-scrollable single list instead of a
  full source-tab UX. This is usable for the current Mew stress route but should
  be revisited before a 600+ Gen 10-scale candidate target.

## Open Questions

- Is a source-tab fallback acceptable as the default for high-candidate species?
- Should TM-heavy pools be chunked by count, grouped by source, or both?
- How should identical move names from different sources be visually labelled in
  the compact list row?
- Should rank / clear-flag unlock labels be visible in the row, or only affect
  availability?
- Should event scripts expose both "unified" and "category" NPC variants?
- Should historical data configs live in `include/config/summary_screen.h`,
  `include/config/pokemon.h`, or a new learnset/relearner config section?
