# Codex Review / Validation Log Runner

`tools/validation/run_review_checks.sh` records local validation output under
`logs/validation/<run-id>/`. Codex review is opt-in because it can consume a
large remote-review context on integration branches. The log directory is
ignored by Git, so large `make check` output and review output can be shared or
inspected without polluting feature diffs.

## 日本語運用メモ

このスクリプトは、`make` 系の検証結果と、必要な場合のみ `codex review`
の結果をファイルに逃がすための補助ツールである。目的は、チャット本文に
巨大ログを貼らずに、失敗箇所だけを後から確認できるようにすること。

`codex review` は不要ではない。むしろ大きい統合やセーブ / バトル / 復元系
の変更では有効な補助レビューである。ただし、差分をリモートレビューへ渡す
ため、差分が大きい統合ブランチではコンテキスト消費が大きい。したがって
Codex 側の作業では、ユーザーが明示的に許可した場合、またはタスク指示で
レビュー実行が必要な場合だけ `--with-codex-review` または
`--review-mode uncommitted` / `base` / `commit` を付けて `codex review` を
実行する。デフォルトの `tools/validation/run_review_checks.sh` は
`codex review` を実行しない。

一方、すでに生成済みの
`logs/validation/<run-id>/ACTIONABLE_FINDINGS.md` は常時参照してよい。
これは要約ファイルなので、フルログより先に読む。通常の確認順は以下。

1. `ACTIONABLE_FINDINGS.md` を読む。
2. 失敗ステップや `P0`-`P3` 指摘がある場合だけ、該当する `*.log` を読む。
3. フルの `make-check.log` や `codex-review-*.log` は、要約だけでは判断できない
   場合に限定して開く。

毎回の基本方針は「実行は許可制、閲覧は `ACTIONABLE_FINDINGS.md` 優先」。
これにより検証証跡を残しつつ、会話側のコンテキスト消費を抑える。

## 日本語: 基本実行手順

リポジトリ直下で以下を実行する。

```sh
cd /home/jastin/dev/pokeemerald-expansion
tools/validation/run_review_checks.sh
```

この基本実行では、以下が順番に走る。`codex review` は走らない。

```sh
rtk git status --short
rtk git diff --check
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check
```

出力先は以下の形式になる。

```text
logs/validation/YYYYMMDD-HHMMSS/
```

まず読むファイルは `ACTIONABLE_FINDINGS.md` のみでよい。

```sh
latest=$(ls -td logs/validation/* | head -1)
less "$latest/ACTIONABLE_FINDINGS.md"
```

`ACTIONABLE_FINDINGS.md` には、失敗したコマンド、既存の
`codex-review-*.log` がある場合は `P0`-`P3` 指摘や通信エラー、失敗ログの
末尾、`FAILED` / `ERROR` / `KNOWN_FAILING` / `EXPECTED_FAIL` などの
注意マーカーだけが集約される。

必要に応じて、同じディレクトリのサマリや個別ログを見る。

```sh
less "$latest/SUMMARY.md"
less "$latest/make-check.log"
```

ただし、フルログは重い。通常は `ACTIONABLE_FINDINGS.md` で判断し、
詳細確認が必要な場合だけ個別ログを開く。

## 日本語: よく使う軽量実行

`codex review` が重い、または通信環境で失敗しやすい場合は、
デフォルトのままローカル検証だけを実行する。

```sh
tools/validation/run_review_checks.sh
```

`make` 系はすでに終わっていて、レビュー結果だけを生成したい場合は、
明示的に以下を使う。

```sh
tools/validation/run_review_checks.sh --skip-make --review-mode uncommitted
```

ローカル検証も含めて、ユーザー側で手動で全部実行したい場合は以下。

```sh
tools/validation/run_review_checks.sh --with-codex-review
```

大きい統合ブランチでは、未コミット全体ではなくコミット単位でレビューする。

```sh
tools/validation/run_review_checks.sh --skip-make --review-mode commit --commit <sha>
```

`master` との差分全体を見たい場合は以下。ただし差分が大きい場合は
コンテキスト消費が大きくなる。

```sh
tools/validation/run_review_checks.sh --skip-make --review-mode base --base master
```

特定テストだけを回したい場合は `--tests` を使う。

```sh
tools/validation/run_review_checks.sh --review-mode none --tests 'All Ability Slots'
```

ユーザー側で手動実行した `codex review` のログだけを要約したい場合は、
対象ディレクトリに `codex-review-*.log` を置いてから以下を実行する。

```sh
tools/validation/run_review_checks.sh --summarize-only logs/validation/<run-id>
```

このモードは新しい `make` や `codex review` を実行せず、
`ACTIONABLE_FINDINGS.md` と `SUMMARY.md` だけを再生成する。

## Default Run

```sh
tools/validation/run_review_checks.sh
```

The default run executes local validation only:

- `rtk git status --short`
- `rtk git diff --check`
- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

Each step writes a separate `*.log` file. `SUMMARY.md` lists the generated logs
and any failed steps. The script keeps running after a failed step so the output
directory still contains as much evidence as possible.

Read `ACTIONABLE_FINDINGS.md` first. It is generated from the full logs and is
intended to reduce context usage. It contains:

- failed step names and exit codes;
- `codex review` priority findings (`P0`-`P3`) and review transport errors;
- the last 120 lines for failed logs only;
- suspicious markers such as `FAILED`, `ERROR`, `No tests found`,
  `EXPECTED_FAIL`, and `KNOWN_FAILING`.

The full logs are still kept beside it for detailed follow-up, but handoff can
usually start from `ACTIONABLE_FINDINGS.md`.

## Smaller Review Runs

For large integration branches, prefer commit or base-scoped review output:

```sh
tools/validation/run_review_checks.sh --skip-make --review-mode uncommitted
tools/validation/run_review_checks.sh --with-codex-review
tools/validation/run_review_checks.sh --skip-make --review-mode commit --commit <sha>
tools/validation/run_review_checks.sh --skip-make --review-mode base --base master
```

For local-only checks without remote Codex review, use the default run or pass
`--review-mode none` explicitly:

```sh
tools/validation/run_review_checks.sh --review-mode none
```

For focused tests:

```sh
tools/validation/run_review_checks.sh --review-mode none --tests 'All Ability Slots'
```

To regenerate only the actionable summary from an existing directory:

```sh
tools/validation/run_review_checks.sh --summarize-only logs/validation/<run-id>
```

## Isolated Codex Home

If the normal Codex home should not be used, add:

```sh
tools/validation/run_review_checks.sh --isolated-codex-home
```

This places `HOME`, `CODEX_HOME`, and `XDG_RUNTIME_DIR` inside the run output
directory only for the `codex review` step. Use this only when authentication
and local policy allow it; the normal mode inherits the user's existing Codex
configuration.

## Handoff

When another environment runs the review, place the produced
`logs/validation/<run-id>/ACTIONABLE_FINDINGS.md` in the workspace or quote its
findings in the feature handoff. Include full `codex-review-*.log` files only
when the actionable summary lacks enough context. The implementation agent
should fix actionable findings, rerun the focused local checks, and record any
environment-blocked review or mGBA Live step in the owning feature
`test_plan.md`.
