# Runtime Integration Gate

## Document Metadata

| Field | Value |
|---|---|
| Last reviewed | 2026-05-17 |
| Baseline | `master` `9376760f68`; `git describe` = `expansion/1.15.2-54-g9376760f68` |
| Code status | Docs-only integration manual |
| Provenance | Local project overlay |

この manual は open PR / closed shelf / feature branch を current `master` に採用する前の共通 gate。
PR と branch は implementation shelf であり、merge 許可ではない。

## Required Snapshot

採用作業の最初に必ず実行する。

```sh
rtk git status --short --branch
rtk git describe --tags --always --dirty
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
| Fresh branch | current `master` から fresh `feature/*` または `integration/*` branch を切る。古い PR branch をそのまま merge しない。 |
| Reapply strategy | commit 単位で cherry-pick するか、file 単位で再適用するか決める。古い docs や generated output を誤って戻さない。 |
| Conflict handling | conflict 解消後、採用した差分が planned scope を超えていないか `git diff --name-only master..HEAD` で確認する。 |
| Local build | runtime source / data / config 変更は `rtk make -j16 -O all`。debug route があるなら `rtk make -j16 -O debug`。 |
| Focused checks | battle / Pokemon / item / save / script logic は `rtk make -j16 -O check` または focused `TESTS=...` を使う。 |
| mGBA evidence | mGBA Live が使える時は boot + feature-specific screen/state を 1 つ確認する。できない場合は failure と manual gap を `test_plan.md` に残す。 |
| Handoff | PR body と feature `test_plan.md` に local make、mGBA/manual evidence、skipped long Actions、known gaps を残す。 |

## Current Shelf Examples

| PR | Use as example for | Adoption caution |
|---|---|---|
| #41 No Random Encounters step-only | minimal flag / config runtime slice | `master` still keeps `OW_FLAG_NO_ENCOUNTER 0`; broad-wild mode is out of MVP scope. |
| #31 TM Shop Migration | data / script / config retirement slice | Emerald scope は確認済みだが FRLG-specific routes は follow-up。`I_REUSABLE_TMS` は branch-only change。 |
| #28 Unified Move Relearner | generated candidate data + Summary / party / NPC entry points | special labels、virtual TM unlock policy、actual overwrite-learning gap を確認する。 |
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
