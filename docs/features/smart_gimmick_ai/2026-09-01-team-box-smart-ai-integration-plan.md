# Team Box #84 と Smart AI の統合準備実行計画

設計書: `2026-09-01-team-box-smart-ai-integration-design.md`

## 目的

`integration/runtime-lab-current-1.16.1` を基点に、Team Box PR #84を先に適用し、その検証後にSmart AI board-searchとproject-local mGBA toolingを適用する。元のdirty worktreeは読み取り対象にとどめ、masterへのmergeと外部pushは行わない。

## 実行単位

### 0. 作業系統を固定する

1. 隔離worktreeが `integration/goal-team-box-smart-ai-20260901` であることを確認する。
2. 基点が `integration/runtime-lab-current-1.16.1` であることを確認する。
3. 元worktreeの未追跡ファイルや `all_learnables.json` の変更を持ち込まない。
4. 各 `make check` は共有temporary artifactの衝突を避けるため、並列実行せず順番に実行する。

### 1. Team Box #84を適用する

次の順序でPR #84のcommitを適用する。

```text
83e76f3e83 Fix Battle Team registration diagnostics
97d44ca4c8 Record Battle Team fix PR
fc3835f5db Implement in-storage Battle Team editor
```

source conflictが発生した場合は、動作を推測して書き換えず、cherry-pickを停止して差分を確認する。文書だけの衝突は、#84の記録と既存integration文書の両方を残す。

適用後に次を順番に実行する。

```sh
rtk git diff --check
rtk make -j16 -O debug
rtk make -j16 -O all
rtk make -j16 -O check TESTS='Battle Team'
rtk make -j16 -O check TESTS='Registered Battle Team'
rtk make -j16 -O check TESTS='SaveBlock3'
rtk make -j16 -O check
rtk mdbook build docs
```

### 2. Team Boxのruntimeを確認する

debug ROMを使い、project-local wrapperまたは文書に記録されたscript-capable mGBA binaryで次を試す。

1. ROM boot、screenshot、input、Lua bridge
2. `Party -> Manage Battle Teams` の導線
3. Team 1〜3の表示と登録・解除
4. first/random lead route、single/double route、incomplete-team rejection
5. save/restartと登録状態の復元
6. Berry消費後のsource item保持、daycare/trade PC制限、relearner可用性

実行できなかった項目は、コマンドと停止地点を `docs/features/battle_team_boxes/test_plan.md` に記録する。

### 3. Smart AIとmGBA toolingを適用する

Team Box検証後、次のcommitを順番に適用する。

```text
fe68cd2d9c feat(ai): add bounded doubles joint board search
104deec1d0 docs(ai): record joint planner handoff commit
9f084cd772 tooling: make direct mGBA Lua runner project-local
282517c16a tooling: select WSLg Wayland for direct mGBA
```

適用後に対象テスト名をソースと既存test planから確認し、board simulator、snapshot、joint runtime、planner、board evaluation、`AI_FLAG_READ_PLAYER_MOVE`、smart AIのfocused checkを順番に再実行する。既存テスト名がそのまま使える場合は次を最低限の入口にする。

```sh
rtk make -j16 -O check TESTS='AI_FLAG_READ_PLAYER_MOVE'
rtk make -j16 -O check TESTS='test/battle/ai/ai_board_sim.c'
rtk make -j16 -O check TESTS='test/battle/ai/ai_board_snapshot.c'
rtk make -j16 -O check TESTS='test/battle/ai/ai_joint_planner.c'
rtk make -j16 -O check TESTS='test/battle/ai/ai_smart_gimmick.c'
rtk make -j16 -O check TESTS='test/battle/ai/ai_doubles.c'
rtk make -j16 -O debug
rtk make -j16 -O all
rtk mdbook build docs
```

### 4. Smart AIのruntime evidenceを確認する

新しいdebug ROMでboot、screenshot、input、Lua bridgeを確認し、`Party -> Joint Trace Double` に到達できるかを試す。到達した場合は、v5 action-logをbattle中にexportし、predicted board、actual board、switch/gimmick/moveの主要fieldを照合する。到達できない場合は、fresh mGBA未確認としてhandoffする。

### 5. 文書と最終状態を更新する

成功したコマンド、警告、失敗した操作、未確認の受け入れ条件を次へ追記する。

- `docs/features/battle_team_boxes/test_plan.md`
- `docs/features/smart_gimmick_ai/test_plan.md`
- `docs/features/smart_gimmick_ai/implementation.md`
- `docs/manuals/mgba_live_mcp_manual.md`（手順やwrapperを変更した場合のみ）

最後に、commit系譜、`git diff --check`、branch、worktree、mGBA sessionの状態を確認する。PR #84の公開状態、master、remoteには書き込まない。

## 完了条件

- #84とSmart AIのcommitが隔離staging branchで追跡できる。
- Team BoxとSmart AIのfocused check、build、docs checkの結果が記録されている。
- mGBAの成功証拠と未確認項目が分離されている。
- 元のdirty worktreeが変更されていない。
- masterへのmerge、外部push、公開PR変更をしていない。

