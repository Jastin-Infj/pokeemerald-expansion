# mGBA Live MCP Manual

この manual は、この workspace で Codex / agent が mGBA Live MCP を使って
実画面・入力・runtime state を確認するための標準手順である。
詳細な調査ログ、Lua template、再構築手順は `docs/tools/` 側の文書を参照する。

## Standard Setup

この project では system package の `/usr/games/mgba-qt` を mGBA Live MCP
の標準 binary として使わない。Ubuntu package の `mgba-qt` は通常起動には使えるが、
確認済みの環境では `--script` option を受け付けない。

標準で使う binary は、workspace cache にある script 対応 build である。

| Item | Path / Value |
|---|---|
| Script-capable mGBA | `/home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt` |
| Version confirmed | `0.11-1-b19b557` |
| Source cache | `/home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-src-master` |
| Default wrapper | `/home/jastin/.local/bin/mgba-qt` |
| Required display fallback | `DISPLAY=:0` |

`/home/jastin/.local/bin/mgba-qt` は次の wrapper にする。

```sh
#!/bin/sh
export DISPLAY="${DISPLAY:-:0}"
exec /home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt "$@"
```

`mgba-live-mcp` は `PATH` 上の `mgba-qt` を探すため、wrapper が先に見つかる状態なら、
MCP tool の `mgba_live_start` に `mgba_path` を毎回渡さなくてよい。

## Quick Setup Check

作業開始時、または mGBA が動かない時は、先に wrapper と script support を確認する。

```sh
rtk which mgba-qt
rtk mgba-qt --version
rtk mgba-qt --script /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/lib/python3.12/site-packages/mgba_live_mcp/resources/mgba_live_bridge.lua --version
```

期待値:

- `which` は `/home/jastin/.local/bin/mgba-qt` を返す。
- version は `0.11-1-b19b557` 系を返す。
- `--script ... --version` が exit code 0 で終わる。

次の結果は失敗として扱う。

```text
/usr/games/mgba-qt: unrecognized option '--script'
```

この場合、system package の mGBA を拾っている。wrapper の配置、`PATH`、または
`mgba_path` の明示指定を見直す。

## MCP Smoke Check

runtime に影響する source / data / config 変更を push する前に、MCP が使えるなら
最初に一度だけ起動確認を試みる。

標準の確認内容:

1. `mgba_live_start` で ROM を起動する。
2. `mgba_live_get_view` で title screen または対象画面を確認する。
3. `mgba_live_input_set` / `mgba_live_input_clear` で最低 1 回 input を通す。
4. 対象 feature の画面または state を確認する。
5. `mgba_live_stop` で session を止める。
6. CLI の `status --all` が `[]` になることを確認する。

feature-specific な確認が難しい場合でも、boot / screenshot / input が通ったか、
どこで止まったかを feature docs に残す。失敗したものを成功扱いにしない。

## CLI Fallback

MCP tool 側で切り分けしづらい場合は CLI を使う。

```sh
rtk env DISPLAY=:0 /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all
```

`start` を CLI で使う時は、cache path が変わる可能性に注意する。
固定 path が存在しない場合は `uvx mgba-live-mcp` の cache を確認し直す。

## Validation Rules

- `make check` は mGBA headless test であり、実画面確認の代替ではない。
- party menu、summary、held item icon、field return、battle text、sound timing などは、
  必要に応じて mGBA Live または user manual check で確認する。
- screenshot だけで対象 behavior を確認済みにしない。input、memory、callback、party data、
  save / savestate、manual observation のどれかを組み合わせる。
- save / savestate が必要な時は、tracked docs や commit に含めず、path と前提だけを記録する。
- GitHub Actions の長時間 job は agent 作業で待ち続けない。20-30 分かかる場合は、
  local `make` と mGBA Live / manual check の evidence を残して push する。

## Recording Results

runtime check を行ったら、最低限次の docs を更新する。

| Result | Where to record |
|---|---|
| build / test command and result | feature `test_plan.md` |
| manual or real-device confirmation | feature `test_plan.md` and `implementation.md` |
| setup / wrapper / MCP operation change | this manual and `docs/tools/mgba_live_runtime_validation.md` |
| cache rebuild or dependency change | `docs/tools/mgba_live_mcp_rebuild_checklist.md` and `docs/tools/tool_dependency_inventory.json` |
| long GitHub Actions not re-waited | feature `test_plan.md`, PR text, or final handoff |

## Cleanup

調査の最後は必ず session を止める。

```sh
rtk env DISPLAY=:0 /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all
```

期待される最終状態:

```json
[]
```

残っている場合は `mgba_live_stop` または CLI `stop --session SESSION_ID` を実行し、
stale session や zombie が残るなら、その状態を feature docs に記録する。
