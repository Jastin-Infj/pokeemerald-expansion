# Feature Docs Entry

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-23 |
| Baseline | `master` `de310ef9eb`; `git describe` = `expansion/1.15.2-96-gde310ef9eb` |
| Code status | Docs-only navigation |
| Provenance | Local project overlay, 2026-05-18 comprehensive feature inventory, 2026-05-23 all-ability-slots investigation |

この directory は local feature の planning / implementation handoff / validation
evidence を置く場所。新しい作業に入る時は、feature folder を直接読む前に
[Feature Registry](feature_registry.md) を先に読む。

## Read Order

1. [Feature Registry](feature_registry.md)
2. [Comprehensive Feature Inventory 2026-05-18](comprehensive_feature_inventory_2026_05_18.md)
3. [Feature Branch Audit 2026-05-18](feature_branch_audit_2026_05_18.md)
4. [Next Runtime Triage 2026-05-18](next_runtime_triage_2026_05_18.md)
5. [Runtime Integration Gate](../manuals/runtime_integration_gate.md)
6. [Validation Evidence Matrix](../manuals/validation_evidence_matrix.md)
7. [Open Investigation Queue](../manuals/open_investigation_queue.md)
8. 対象 feature folder の `README.md`

feature folder 内は次の順で読む。

| File | Purpose |
|---|---|
| `README.md` | 現在の目的、scope、branch / PR 状態、current decision。 |
| `investigation.md` | 実ファイル / symbol / 既存挙動の調査。実装前の根拠。 |
| `mvp_plan.md` | first cut の実装単位、採用しないもの、順序。 |
| `risks.md` | blocker、accepted risk、future work。 |
| `implementation.md` | 実装済み branch の変更内容、検証結果、merge handoff。 |
| `test_plan.md` | local make、focused tests、mGBA Live、manual checks、known gaps。 |
| Other docs | `dependencies.md`、`candidate_data_flow.md`、`asset_references.md` など feature 固有の補助資料。 |

## Branch Policy

`master` は upstream intake + docs / handoff baseline。runtime source は直接入れない。
`src/`、`include/`、`data/`、`graphics/`、`sound/`、`tools/`、generated output、
ROM、save、screenshot は、通常の docs-only master 更新に含めない。

Open PR は implementation shelf であり、merge 許可ではない。採用する場合は
current `master` から fresh feature / integration branch を切り、必要な commit
または file だけを cherry-pick / reapply する。古い PR の merge state、CI、
docs handoff、validation evidence を確認せずに `gh pr merge` しない。

## Status Language

| Term | Meaning |
|---|---|
| Investigating | 実装前調査。runtime source はまだない。 |
| Planned | 方針はあるが implementation shelf はない、または採用順待ち。 |
| Implemented draft | runtime source は branch / PR にあるが、`master` にはない。 |
| Validated branch | local make / mGBA / focused evidence が branch にあるが、`master` にはない。 |
| Integration candidate | current `master` へ fresh branch で再適用できる候補。 |
| Shipped | `master` または明示された integration baseline に取り込まれ、仕様固定された状態。 |

`validated branch`、`implemented draft`、`shipped` は混同しない。

## New Feature Candidates

These are docs-only candidates for future local features. They are not runtime
implementation shelf PRs and have no source changes on `master`.

- [Future Feature Candidates](future_feature_candidates.md)
- [Comprehensive Feature Inventory 2026-05-18](comprehensive_feature_inventory_2026_05_18.md)
- [Feature Branch Audit 2026-05-18](feature_branch_audit_2026_05_18.md)
- [Next Runtime Triage 2026-05-18](next_runtime_triage_2026_05_18.md)
- [Jukebox / Sound Archive](jukebox_sound_archive/README.md)
- [Battle BGM Selector / Sound Archive](battle_bgm_selector/README.md)
- [Weather Lab Terminal](weather_lab_terminal/README.md)
- [Bounty Board / Request Board](bounty_board/README.md)
- [Field Notes / Lore Codex](field_notes_codex/README.md)
- [Route Mastery Passport](route_mastery_passport/README.md)
- [Trainer Titles / Achievement Badges](trainer_titles_achievement_badges/README.md)

## Policy Feature Candidates

These docs track larger gameplay policy candidates that are not source changes
on `master`. They often depend on existing implementation shelves but should be
adopted through fresh runtime branches.

- [Nonconsumable Held Items](nonconsumable_held_items/README.md)
- [Scout Selection](scout_selection/README.md)
- [All Ability Slots Runtime](all_ability_slots/README.md)

## Current Runtime Shelves

2026-05-30 時点では、主要 runtime shelf は
`integration/runtime-dev-20260529` / PR #68 に再適用済み。open PR は
evidence shelf として残っているものが多く、`master` への merge 許可ではない。

| PR | Feature | Branch | Current handling |
|---|---|---|---|
| #68 | Runtime Integration Staging | `integration/runtime-dev-20260529` | Current runtime integration lane. |
| #47 / #48 / #51 / #54 / #57 / #60 / #62 | Older runtime shelves | feature branches | Equivalent or newer code is staged in #68; keep as evidence until superseded / closed by user decision. |
| #65 | Map Asset Relinker | `feature/map-asset-relinker-20260525` | Tooling lane, not ROM runtime integration. |
| #39 and other closed runtime shelves | Battle BGM, TM/HM, Relearner, Team Viewer, State Editor, Summary Tera, Trainer Aftercare, Partygen | feature branches | Adopted into `integration/runtime-dev-20260529` or recorded as superseded / reference-only in the runtime audit. |

この snapshot は古くなる。実装採用前は必ず `gh pr list --state all` と
`gh pr view` と branch diff を再確認する。
