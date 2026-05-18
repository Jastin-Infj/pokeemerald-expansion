# Feature Docs Entry

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-18 |
| Baseline | `master` `abbbf47554`; `git describe` = `expansion/1.15.2-67-gabbbf47554` |
| Code status | Docs-only navigation |
| Provenance | Local project overlay |

この directory は local feature の planning / implementation handoff / validation
evidence を置く場所。新しい作業に入る時は、feature folder を直接読む前に
[Feature Registry](feature_registry.md) を先に読む。

## Read Order

1. [Feature Registry](feature_registry.md)
2. [Next Runtime Triage 2026-05-18](next_runtime_triage_2026_05_18.md)
3. [Runtime Integration Gate](../manuals/runtime_integration_gate.md)
4. [Validation Evidence Matrix](../manuals/validation_evidence_matrix.md)
5. [Open Investigation Queue](../manuals/open_investigation_queue.md)
6. 対象 feature folder の `README.md`

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

## Current Runtime Shelves

2026-05-17 cleanup 後、GitHub 上で open の runtime implementation shelf は
0 件。成功済み runtime PR は completed shelf として close し、branch は
preserved のまま残す。

| PR | Feature | Branch | Merge state from `gh pr list` |
|---|---|---|---|
| None | - | - | - |

Completed shelves closed on 2026-05-17:

| PR | Feature | Branch |
|---|---|---|
| #41 | No Random Encounters step-only | `feature/no-random-encounters-step-only-runtime-20260517` |
| #39 | Battle BGM Selector / Sound Archive | `feature/battle-bgm-selector-mvp-20260517` |
| #31 | TM Shop Migration | `feature/tm-shop-migration` |
| #28 | Unified Move Relearner | `feature/unified-move-relearner` |
| #26 | Summary Tera Type Icon | `feature/summary-tera-type-badge` |
| #23 | Pokemon State Editor | `feature/pokemon-state-editor-expansion` |
| #20 | Pre-Battle / In-Battle Team Viewer | `feature/prebattle-team-viewer` |

この snapshot は古くなる。実装採用前は必ず `gh pr list --state all` と
`gh pr view` で再確認する。
