# Scout Selection Risks

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `e927b612b3`; `feature/scout-selection-runtime-20260520` |
| Code status | Runtime MVP implemented on feature branch |
| Provenance | Local source inspection and previous UI branch evidence |

## Risks

| Risk | Severity | Impact | Mitigation |
|---|---|---|---|
| Summary return corrupts custom UI state | High | Cursor / selected slots can reset or BGs can render incorrectly after Summary. | Reuse Team Viewer cleanup/reinit pattern; keep candidate/selection state in EWRAM; validate Summary -> back -> Summary again. |
| Field return resets heap before script gift | High | Confirmed selections can point at freed / overwritten heap data before `GiveSelectedScoutMons` runs. | Keep the generated candidate array and selected order in static EWRAM until the gift special clears it. |
| Direct Summary skills-page layout regression | Medium | Starting Summary on `PSS_PAGE_SKILLS` can corrupt page scroll / info page. | Reapply `ShowPokemonSummaryScreenAtPage()` and `SetInitialSkillsPageTilemaps()` behavior from Team Viewer only if using skills page as first page. |
| Candidate preview differs from received Pokemon | High | Player chooses based on Summary but receives a different nature/moves/item. | Generate `struct Pokemon` candidates before display and give the exact selected structs, or document if Summary is species-only. |
| Partial give for N picks | High | First Pokemon may be given, later one fails due full party/PC, leaving script state inconsistent. | Preflight space for pick count or make `GiveSelectedScoutMons` transactional with clear failure rules. |
| Icon palette / sprite lifetime leak | Medium | Returning from Summary or closing UI can leak sprites/palettes or render invisible icons. | Use `LoadMonIconPalettes`, `CreateMonIconIsEgg`, `FreeAndDestroyMonIconSprite`, `FreeMonIconPalettes`; validate repeated open/close. |
| Scroll + selection marker mismatch | Medium | Selected marker may appear on wrong candidate after scroll. | Store selected candidate indices independent of visible row; redraw visible rows from state. |
| Generated Scout pool drifts from partygen JSON | Medium | Candidate list can change unexpectedly when catalog set order or schema changes. | Treat `tools/champions_partygen/catalog/sets/*.json` as the source of truth; generator fails on unsupported schema, invalid stat ranges, too few unique species, and more than four moves. Record source JSON order in implementation docs. |
| Duplicate species in partygen catalog | Low | The same Pokemon can appear multiple times in Scout choices if set-level de-duplication is missed. | Generator de-duplicates by exact `species` symbol. First set wins; later duplicate species are skipped. |
| Ability label does not match species slot | Medium | Summary / received Pokemon may fall back to personality ability rather than requested partygen ability. | Runtime maps ability IDs through `GetSpeciesAbility()`. Catalog validation should continue catching unknown constants; add species-slot validation if partygen catalog adoption expands. |
| Button conflicts | Low | `SELECT` may conflict with existing detail/help expectations. | Keep MVP screen-specific controls only; document button map in test plan. |
| Script var collision | Medium | `VAR_0x8004`-style inputs are shared scratch vars. | Use vars only during immediate script call, or wrap with macros / dedicated helper contract. |
| Starter story flow conflicts | High | Replacing Route 101 starter flow touches first battle, lab handoff, rival state, and `VAR_STARTER_MON`. | Do not replace starter story in MVP; add a debug/test NPC first. |
| SaveBlock pressure from persistent scout state | Medium | Daily refresh / timed scout can require new saved state. | Keep MVP stateless except script flags; defer timed economy to a separate design. |
| Front battle sprite budget | Medium | Full sprites can exceed VRAM/OAM/window budget on a scrollable screen. | Use Pokemon icons first; optionally show one front sprite for current cursor in later phase. |

## Interactions With Other Features

| Feature | Interaction | Current decision |
|---|---|---|
| Pre-Battle / In-Battle Team Viewer | Provides the best Summary return and selected marker reference. | Reuse design, not branch merge. |
| Trainer Battle Party Selection | Similar N-of-M selection and `gSelectedOrderFromParty` idea. | Conceptual reference only; scout receives/gives Pokemon rather than compressing an existing party for battle. |
| Champions Challenge Facility | Scout can become the party acquisition screen for a 0 Pokemon start. | Keep scout standalone first; facility integration later. |
| Trainer Partygen catalog | Current demo source for candidate pools. | Use only curated set JSON plus a Scout-specific generator in this slice; do not require the full partygen CLI or generated trainer output for normal ROM build. |
| Held Item Catalog / Battle Item Restore | Scout candidates may hold items. | Giving a Pokemon with held item should follow normal Pokemon item ownership semantics; do not mix with Bag token logic in MVP. |
| Pokemon State Editor / Unified Relearner | Summary can expose move/edit affordances depending branch config. | Use `SUMMARY_MODE_LOCK_MOVES` first; do not enable editor/relearner flows from scout without explicit policy. |

## Accepted MVP Risks

- The first runtime slice can use icons instead of front battle sprites.
- The first runtime slice can support one debug/test pool before general map adoption.
- The first runtime slice can be branch-only and not available on `master`.
- Summary opens the standard first page in this slice; direct skills-page entry remains later work.
- Pick count 2 / 3 support is implemented by the state model but still needs a focused
  mGBA evidence pass before it is used in a player-facing multi-pick facility.

## Open Questions

- Whether generated Scout should validate ability/species legality at build time
  instead of relying on runtime fallback.
- Whether cancel should be disabled for mandatory starter use.
- Whether scout pools should be deterministic across save/reload before claim.
