# Scout Selection

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-19 |
| Baseline | `master` `8bb44a15f4`; `git describe` = `expansion/1.15.2-77-g8bb44a15f4` |
| Code status | Docs-only feature kickoff; runtime branch not created yet |
| Provenance | Local source inspection, existing feature branch docs, public GitHub / official Pokemon Champions reference check |

Status: Planned
Code status: no source changes on this branch

## Goal

Pokemon Champions 風に、NPC / object event から候補 Pokemon を提示し、Summary
を確認してから 1 匹または N 匹を選んで受け取れる reusable runtime を作る。

MVP は「script で pool と pick count を指定し、最大 6 件表示、候補数が多い時は
縦スクロール、選択済み候補を明示、Summary から戻って選択状態を維持する」こと。

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

### Out of Scope

- `master` への runtime source merge。
- full Pokemon Champions economy (VP、22-hour timer、ticket、trial scout) の再現。
- Battle Factory rental state / Frontier save state の再利用。
- Battle front sprite 版 UI の初回実装。
- SaveBlock layout 追加。MVP は claim flag / existing vars / script state で扱う。
- Trainer Party Pool / partygen の直接統合。候補 pool 生成の後続 input としては使える。

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Scout Selection and Battlefield Status Design v15](../../overview/scout_selection_and_battlefield_status_v15.md)
- [Pre-Battle / In-Battle Team Viewer](../prebattle_team_viewer/README.md)
- [Trainer Battle Party Selection](../battle_selection/README.md)
- [Champions Challenge Facility](../champions_challenge/README.md)

## Open Questions

- MVP pool max は 11 固定でよいか、12 / 16 まで許容するか。
- Summary start page は Team Viewer と同じ `PSS_PAGE_SKILLS` でよいか。
- Candidate result は C 側 helper で直接 give するか、script vars に species / level
  などを返して `givemon` に任せるか。
- Starter replacement として使う時、Route 101 first battle flow も置き換えるか、
  lab / debug / facility NPC だけを先に実装するか。
