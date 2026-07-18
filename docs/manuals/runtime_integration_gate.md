# Runtime Integration Gate

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-07-18 |
| Baseline | fork `master` `b65afd70d7`; source version `1.16.1`; upstream release `expansion/1.16.2` at `ad0fd4d17f` |
| Code status | Docs-only integration manual |
| Provenance | Local project overlay |

この manual は open PR / closed shelf / feature branch を current `master` に採用する前の共通 gate。
PR と branch は implementation shelf であり、merge 許可ではない。

## Required Snapshot

採用作業の最初に必ず実行する。

```sh
rtk git status --short --branch
rtk git describe --tags --always --dirty
rtk git remote -v
rtk gh release view --repo rh-hideout/pokeemerald-expansion --json tagName,targetCommitish,publishedAt,url
rtk gh pr list --state open --json number,title,isDraft,headRefName,baseRefName,updatedAt,mergeStateStatus,statusCheckRollup
```

対象 PR は追加で確認する。

```sh
rtk gh pr view <number> --json number,title,state,isDraft,headRefName,baseRefName,updatedAt,mergeStateStatus,statusCheckRollup,files,commits
```

2026-05-17 cleanup 後、open runtime PR は 0 件。#41 / #39 / #31 / #28 /
#26 / #23 / #20 は successful implementation shelf として close 済みで、branch は
preserved。過去 docs の `CLEAN` / `DIRTY` 表記を信じず、採用直前の `gh` 出力と
local branch diff を source of truth にする。

## Gate Checklist

| Gate | Required action |
|---|---|
| Worktree | `master` clean から始める。dirty な場合はユーザー作業と自分の作業を分ける。 |
| Scope | docs-only か runtime integration かを先に決める。runtime source を docs-only branch に混ぜない。 |
| PR state | `mergeStateStatus`、draft 状態、base/head branch、updatedAt、checks を確認する。 |
| Diff scope | `gh pr view --json files` または local branch diff で `src/ include/ data/ graphics/ tools/` などの変更範囲を確認する。 |
| Docs sync | owning feature docs の `README.md`、`implementation.md`、`test_plan.md`、`risks.md` が branch の実装と一致しているか確認する。 |
| Base selection | upstream intake / standalone shelf は current `master`、current playable line の feature は pinned `integration/active-runtime-<version>` head を使う。base commit を記録する。 |
| Fresh branch | selected base から fresh `feature/*` または integration staging branch を切る。古い PR branch をそのまま merge しない。 |
| Upstream authority | release transition では upstream の variable/API/struct/save-data/generated contract を正とし、local implementation の意図を新 contract 上へ port する。 |
| Reapply strategy | commit 単位で cherry-pick するか、file 単位で再適用するか決める。古い docs や generated output を誤って戻さず、新 release tool で再生成する。 |
| Conflict handling | conflict 解消後、採用した差分が planned scope を超えていないか `<selected-base>..HEAD` で確認する。`master` PR の場合だけ base を `master` に固定する。 |
| Local build | runtime source / data / config 変更は `rtk make -j16 -O all`。debug route があるなら `rtk make -j16 -O debug`。 |
| Focused checks | battle / Pokemon / item / save / script logic は `rtk make -j16 -O check` または focused `TESTS=...` を使う。 |
| mGBA evidence | mGBA Live が使える時は boot + feature-specific screen/state を 1 つ確認する。できない場合は failure と manual gap を `test_plan.md` に残す。 |
| Handoff | PR body と feature `test_plan.md` に local make、mGBA/manual evidence、skipped long Actions、known gaps を残す。 |

## Upstream Generation Gate

新 upstream release は patch update でも世代移行として監査する。1.16.1 -> 1.16.2
だけでなく、1.16.x -> 1.17 以降も同じ gate を使う。

- exact release tag / commit と fork `master` commit を固定する。
- previous active integration を `snapshot/runtime-<old-version>/*` に凍結する。
- upstream intake と local feature re-apply を同じ PR に入れない。
- upstream で削除・rename・再設計された field / API / struct / data layout を、古い
  branch の file copy で復元しない。
- conflict ごとに producer / consumer / serialization / generated tool / tests を確認する。
- save layout、ID width、generated format、config default の変更は focused check だけで
  終えず、migration と old-save behavior を明示する。
- updated `master` から fresh active integration を作り、採用 feature manifest の順で
  separate PR を通す。

## Completion Vocabulary

PR の open / closed だけでは採用状態を判断しない。docs、PR body、branch name では次の
語を使う。

| State | Meaning |
|---|---|
| implementation complete | 単体実装と owning tests / runtime evidence が完了した。`master` merge を意味しない。 |
| integration pending | 実装 shelf は完成しているが、現行 active runtime へ未採用。 |
| integrated `<version>` | exact slice が対象 active runtime 向け PR で merge 済み、combined validation 済み。 |
| snapshot frozen | 複合 runtime を比較・復旧用に固定し、追加実装しない。 |
| docs handoff complete | Markdown / `AGENTS.md` / approved Lua evidence が別 PR で `master` へ反映済み。 |

2026-07-18 時点の current open runtime PR の読み方:

| PR | Recorded state | Current use |
|---|---|---|
| #69 16.0 runtime port | implementation complete / snapshot frozen | 16.0 replay comparison source。`master` merge 対象ではない。 |
| #71 Champions PartyGen | implementation complete / integration pending | next active runtime generation への採用候補。 |
| #72 Smart Gimmick AI | implementation complete / integrated in runtime-lab via #78 | reusable shelf。 |
| #74 Box NPC Party Pool | implementation complete / integrated in runtime-lab via #76 | reusable shelf。 |
| #75 Battle Team Boxes | implementation complete / integrated in runtime-lab via #77 | reusable shelf。 |

`integration/runtime-lab-20260716` は Box NPC / Battle Team / Smart の combined evidence
snapshot であり、upstream 1.16.2 active runtime を意味しない。1.16.2 向け playable base
は upstream intake 後に fresh branch として作る。

## Current Shelf Examples

| PR | Use as example for | Adoption caution |
|---|---|---|
| #41 No Random Encounters step-only | minimal flag / config runtime slice | `master` still keeps `OW_FLAG_NO_ENCOUNTER 0`; broad-wild mode is out of MVP scope. |
| #31 TM Shop Migration | data / script / config retirement slice | Emerald scope は確認済みだが FRLG-specific routes は follow-up。`I_REUSABLE_TMS` は branch-only change。 |
| #28 Unified Move Relearner | generated candidate data + Summary / party / NPC entry points | special labels、virtual TM unlock policy、actual overwrite-learning gap を確認する。Party grid #54 と同時採用する場合は Summary START を正規入口にし、party 直 `RELEARN` は optional/debug または vertical fallback に寄せる。 |
| #26 Summary Tera Type Icon | small Summary display UI + imported graphics | graphics / CREDITS / asset provenance を docs-only master に混ぜない。 |
| #23 Pokemon State Editor | Summary overlay UI and Pokemon field edits | box summary、redraw artifacts、legality locks、config defaults を確認する。 |
| #20 Pre-Battle / In-Battle Team Viewer | battle UI + Summary return + mGBA-heavy validation | trainer pool / randomized party cache mechanism is implemented on the source shelf; add optional focused regression before adoption. |

## Old PR Close Policy

- Superseded PR は、後継 branch / PR と docs evidence を残してから close する。
- Unique work を持つ branch は、PR を閉じても remote branch をすぐ消さない。
- Closed old PR を再開するより、current `master` から fresh branch を作る方が安全。
- PR を閉じた理由は Feature Registry または owning feature docs に残す。

## Not Allowed

- docs-only request で runtime implementation PR を merge しない。
- unknown merge state を推測で `CLEAN` と書かない。
- CI success だけで mGBA / manual evidence を省略しない。
- `validated branch` を `shipped` と書かない。
