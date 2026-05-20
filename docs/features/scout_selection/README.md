# Scout Selection

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-20 |
| Baseline | `master` `e927b612b3`; `feature/scout-selection-runtime-20260520` |
| Code status | Runtime MVP implemented on feature branch |
| Provenance | Local source inspection, existing feature branch docs, public GitHub / official Pokemon Champions reference check |

Status: Runtime MVP implemented on feature branch
Code status: source changes are intentionally not on `master`

## Goal

Pokemon Champions 風に、NPC / object event から候補 Pokemon を提示し、Summary
を確認してから 1 匹または N 匹を選んで受け取れる reusable runtime を作る。

MVP は「script で pool と pick count を指定し、最大 12 候補から 6 件表示、
候補数が多い時は縦スクロール、選択済み候補を明示、Summary から戻って
選択状態を維持する」こと。

## Implemented MVP

`feature/scout-selection-runtime-20260520` adds a reusable runtime:

| Area | Implementation |
|---|---|
| Script contract | `InitScoutSelection`, `OpenScoutSelection`, `GiveSelectedScoutMons` specials use `VAR_0x8004` pool id, `VAR_0x8005` candidate count, and `VAR_0x8006` pick count. |
| Candidate count | MVP supports up to 12 candidates. The debug pool now uses 12 unique species generated from partygen set JSON. |
| Visible layout | 2 columns x 3 rows, 6 visible candidates, vertical scroll by row. |
| Controls | D-pad cursor, `A` select/deselect, `SELECT` Summary, `START` confirm at exact pick count, `B` cancel. |
| Summary | Standard Summary opens for the highlighted candidate and returns to the scout screen with cursor, scroll, and selected order preserved. |
| Gift result | Confirmed candidates are given through `GiveScriptedMonToPlayer`; scripts receive `MON_GIVEN_TO_PARTY`, `MON_GIVEN_TO_PC`, or `MON_CANT_GIVE`. |
| Debug route | Debug menu `Scripts... > Scout Selection` opens the 12-candidate pool with pick count 1. `Scripts... > Script 2` opens the same pool with pick count 6 for multi-pick testing. |
| Validation | mGBA Live confirmed open, partygen-derived candidates, scroll, Summary return, confirm, and party gift on `feature/scout-selection-runtime-20260520`. |

## Current Decision

`src/starter_choose.c` を直接 6 / 11 枠へ拡張しない。既存 starter screen は
3 個の固定 Pokeball と full front sprite animation に強く寄っており、Summary
entry と N-of-M selection を足すと専用 UI としての制約が大きい。

実装時は新規 `scout_selection` module を作り、以下を流用する。

| Source | Reuse |
|---|---|
| `feature/prebattle-team-viewer-phase2` | Pokemon icon grid、選択 marker、Summary return、direct skills page entry |
| `src/battle_factory_screen.c` | 6 候補から選ぶ state model、Summary 用 temp mon buffer、candidate data generation の考え方 |
| `src/starter_choose.c` | `special ChooseStarter` + `waitstate` の field transition pattern と Pokeball visual reference |
| `givemon` / `createmon` path | 選択結果を party / PC へ入れる gift-mon semantics |

Battle Factory select screen は 6 候補 / Summary の先例だが、Frontier rental
state と 3 体固定前提が強いので直接流用しない。Team Viewer は UI / Summary
return の実装 shelf として最も近い。

## Scope

### In Scope

- NPC / Pokeball object event から `special` + `waitstate` で開ける scout UI。
- `.inc` から pool id、sample count、pick count を指定できる script contract。
- default pick count 1、N-of-M selection も可能な state model。
- Pokemon icon sprite 表示。battle front sprite は後続 phase に回す。
- `SELECT` または configured detail button から standard Summary を開き、戻る。
- 選択済み候補の背景 / marker / order number 表示。
- 最大 6 visible candidates。pool は 6 を超えても縦 scroll で扱う。
- 選択 Pokemon を gift path で party / PC へ渡し、`MON_GIVEN_TO_PARTY` /
  `MON_GIVEN_TO_PC` / `MON_CANT_GIVE` を script 側で扱えるようにする。
- 選択状態は static EWRAM に保持する。field return は heap を reset するため、
  `waitstate` 後の gift special まで heap pointer を残さない。

### Out of Scope

- `master` への runtime source merge。
- full Pokemon Champions economy (VP、22-hour timer、ticket、trial scout) の再現。
- Battle Factory rental state / Frontier save state の再利用。
- Battle front sprite 版 UI の初回実装。
- SaveBlock layout 追加。MVP は claim flag / existing vars / script state で扱う。
- Full Trainer Party Pool / partygen CLI integration. This slice only consumes
  curated partygen set JSON through a small Scout-specific build-time generator.

## Related Docs

- [Investigation](investigation.md)
- [Implementation](implementation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Scout Selection and Battlefield Status Design v15](../../overview/scout_selection_and_battlefield_status_v15.md)
- [Pre-Battle / In-Battle Team Viewer](../prebattle_team_viewer/README.md)
- [Trainer Battle Party Selection](../battle_selection/README.md)
- [Champions Challenge Facility](../champions_challenge/README.md)

## Open Questions

- General pool authoring is now generated C from `tools/champions_partygen/catalog/sets/*.json`
  for the demo pool. Map-specific `.inc` wrappers can still be added after the
  first runtime validation.
- Summary starts from the standard page for MVP. Direct skills-page entry remains future work.
- Pick count 2 / 3 is supported by state model but still needs focused mGBA evidence
  before a multi-pick facility uses it.
- Starter replacement として使う時、Route 101 first battle flow も置き換えるか、
  lab / debug / facility NPC だけを先に実装するか。
