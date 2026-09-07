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

## Direct CLI / Lua (MCP不要)

画面確認や手書き Lua の実行は MCP を使わず、プロジェクト内の wrapper から直接行える。
`mgba_live_cli.sh` は次の順で `mgba-live-cli` を解決する。

1. `MGBA_LIVE_CLI`
2. 観測済みの uv cache entry point
3. `PATH` 上の `mgba-live-cli`
4. `MGBA_LIVE_UVX` または `/home/jastin/.local/bin/uvx` の
   `uvx --from mgba-live-mcp` fallback

セッション状態は既定で現在の worktree の
`.cache/mgba-live-runtime/` に保存する。これにより MCP server の有無や
`~/.mgba-live-mcp/runtime/` の書込み権限に依存せず、worktree ごとにセッションを分離できる。
必要なら `MGBA_LIVE_RUNTIME_ROOT` で変更する。

```sh
BATTLE_ACTION_LOG_AUTOSAVE=0 \
  tools/mgba_live/start_mgba_live.sh team-box 60 /tmp/pokeemerald.gba
tools/mgba_live/run_lua.sh team-box /tmp/check_team_box.lua
tools/mgba_live/mgba_live_cli.sh screenshot --session team-box --out /tmp/team-box.png
tools/mgba_live/mgba_live_cli.sh stop --session team-box
tools/mgba_live/mgba_live_cli.sh status --all
```

`start_mgba_live.sh` は `QT_QPA_PLATFORM` が未指定で `WAYLAND_DISPLAY` が存在する場合、
WSLg で安定しやすい `QT_QPA_PLATFORM=wayland` を自動設定する。X11 を明示的に使う場合は
`QT_QPA_PLATFORM=xcb` を指定する。

最後の結果は `[]` にする。手書き Lua は `/tmp` に置き、再利用する helper だけを
`tools/mgba_live/` に置く。

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

## Raw CLI Fallback

プロジェクト wrapper が使えない場合だけ、package cache の raw CLI を直接使う。
通常の手動検証では上の `tools/mgba_live/mgba_live_cli.sh` を優先する。

```sh
rtk tools/mgba_live/mgba_live_cli.sh status --all
```

raw CLI を使う場合は `DISPLAY=:0` と script-capable `mgba-qt` wrapper を指定する。
package cache の content-addressed path は再インストールで変わるため、project script に
固定 path を追加しない。固定 path が無い場合は `MGBA_LIVE_UVX` または
`uvx --from mgba-live-mcp mgba-live-cli` の cache を確認し直す。

## Reusable Lua Tools

One-off validation Lua files should stay under `/tmp`, but reusable project tools may live under `tools/mgba_live/`.

Current reusable helpers:

| Tool | Purpose |
|---|---|
| `tools/mgba_live/mgba_live_cli.sh` | Project-local direct CLI entrypoint. Resolves the package runner and isolates session state per worktree. |
| `tools/mgba_live/run_lua.sh` | Runs a hand-written Lua file in a named live session without MCP. |
| `tools/mgba_live/battle_action_log_export.lua` | Reads `gBattleActionLog` and the signature-compatible `gBattleAiTraceLog` from the running ROM and writes host JSON. |
| `tools/mgba_live/battle_action_log_autosave.lua` | Startup Lua script loaded by the start wrappers to keep the latest non-empty action-log plus compatible AI-trace snapshot on the host. |
| `tools/mgba_live/start_mgba_live.sh` | WSL / Linux shortcut to start mGBA Live with explicit session, FPS, and ROM arguments. Defaults to 120 FPS. |
| `tools/mgba_live/start_mgba_live.bat` | Windows shortcut to start mGBA Live with explicit session, FPS, and ROM arguments. Defaults to 120 FPS. |
| `tools/mgba_live/export_battle_action_log.sh` | WSL / Linux shortcut for the exporter. |
| `tools/mgba_live/export_battle_action_log.bat` | Windows shortcut for the exporter when `mgba-live-cli` is on `PATH`. |

Typical WSL / Linux use:

```sh
tools/mgba_live/start_mgba_live.sh manual-ai-log 120
tools/mgba_live/export_battle_action_log.sh manual-ai-log /tmp/battle-action-log.json
```

The start wrapper enables action-log autosave by default and writes the latest non-empty snapshot to `/tmp/manual-ai-log-battle-action-log-autosave.json` unless `BATTLE_ACTION_LOG_OUT` is set. If only one session is live, the WSL / Linux exporter can omit `SESSION` and use the active mGBA Live session.

Typical Windows use:

```bat
tools\mgba_live\start_mgba_live.bat manual-ai-log 120
tools\mgba_live\export_battle_action_log.bat manual-ai-log %TEMP%\battle-action-log.json
```

The exporter resolves symbols from `pokeemerald.map`, so the ROM and map must come
from the same build. Both backing buffers are in EWRAM and are cleared by battle
initialization; export while the target battle is still active. If the session
closes before manual export and autosave was not running, the snapshot cannot be
recovered from `archived_sessions`.

Current export-buffer layouts are fixed by compile-time assertions:

| Buffer | Layout | Total |
|---|---|---:|
| `gBattleActionLog` | 128 entries × 28 bytes + 8-byte header | 3,592 bytes |
| `gBattleAiTraceLog` | 32 plans × 32 bytes + 256 candidates × 28 bytes + 96 boards × 96 bytes + 16-byte header | 17,424 bytes |

`gBattleActionLog` records confirmed commands for all live battlers and resolved
switch-ins. Its entries include named moves / items / gimmicks, targets, switch-in
party indexes, selected-gimmick markers, corrected-switch markers, AI plan / rank
links, reason tags, threat flags, and risk kinds. `gBattleAiTraceLog` schema version
5 uses header magic `0xA15C` and records plans, their retained candidates, and board
snapshots for the before, predicted-after, and actual-after phases. Each exported
plan also includes `board_comparison`, which pairs the predicted-after and
actual-after snapshots by plan ID and reports `matches`, a difference bitmask,
and readable difference names for weather, field, side, timers, active-battler
mask, and battler state.

When both the trace version and magic match, the exporter writes schema
`pokeemerald.battle_action_log.v5`. The top-level trace keys are
`ai_trace_header`, `ai_plans`, `ai_candidates`, and `ai_board_snapshots`; action
entries remain under `entries`. Confirm `source.ai_trace_signature_valid == true`
before using the trace as planner evidence. If `gBattleAiTraceLog` is absent or its
version / magic does not match, the exporter deliberately emits
`pokeemerald.battle_action_log.v4` with action-log data only. That fallback protects
against a stale `pokeemerald.map` or a different ROM layout; it is not evidence that
the runtime produced no AI plan.

Autosave calls the same exporter, so a compatible run also preserves the v5 trace
keys. It skips empty action logs and keeps the latest non-empty snapshot at the
configured `BATTLE_ACTION_LOG_OUT` path.

## Joint Planner Runtime Acceptance

Use `Party -> Joint Trace Double` from the field debug menu for a focused joint
planner check. The fixture is an ordinary active-2v2 trainer double with a reserve on
the AI side. The party source deliberately omits explicit `Party Size` lines so
trainer generation preserves the listed player two / AI three members:

- player-left is Sassy 0-Speed, bulky Intimidate Arcanine with only Tailwind.
- player-right is Sassy 0-Speed, bulky Power Herb Sturdy Skarmory with only Geomancy.
- AI-left is level-7 Adamant, zero-Attack-IV, physically bulky Scizorite Technician
  Scizor with only Bullet Punch.
- AI-right is Timid Focus Sash Prankster Whimsicott with only Tailwind.
- the AI reserve is Timid Power Herb Fairy Aura Xerneas with only Geomancy, and the
  route grants Mega Ring access.

The AI has Smart Trainer, Smart Mon Choices, Omniscient, and Read Player Move flags.
This board supplies a real prospective Mega profile, a switch candidate, and
nonzero board scores while keeping the selected depth-3 path inside the production
frame budgets. Raw roll / critical exactness is intentionally verified by the C
simulator suite rather than inferred from this runtime fixture.

Enter and confirm Tailwind for the player-left battler and Geomancy for the
player-right battler. Let the opponent choose, then allow the complete turn and
end-turn trace capture to finish before exporting while the battle remains active;
exporting immediately after command selection can legitimately omit `actual-after`.
A successful joint-path sample must satisfy all of the following:

- schema is `pokeemerald.battle_action_log.v5`, trace signature is valid, and the
  trace header reports version 5 plus magic `0xA15C` (`41308` in decimal JSON).
- `ai_plans` and `ai_candidates` are non-empty.
- at least one plan has `joint` and `deepest_complete_used` in `flag_names`, without
  `legacy_evaluator`, reports a completed depth, and has nonzero candidate score
  components.
- its chosen candidate has `joint` and `chosen` in `flag_names`; the retained or
  forced-trace review set also exposes a Mega action and a switch-to-Xerneas action.
- both opponent action-log entries link to that same `ai_plan_id` and candidate
  rank.
- the before, predicted-after, and actual-after boards are present, and prediction /
  actual action links agree.

The fixture proves shared-plan arbitration, prospective Mega and reserve-switch
visibility, command linkage, and v5 export. It does not export the raw stochastic
frontier, so exact 16-roll, critical, true-Speed-tie, and post-board-merge behavior
must be paired with the focused C simulator regressions. It also does not validate
every move, switch-in effect, field state, or gimmick; use a feature-specific battle
in addition when those mechanics are the target.

The former two-Shuckle, no-item, no-gimmick Tackle fixture and session
`smart-ai-joint-trace-final2-20260722` remain superseded wiring history only. They
proved a shared nonlegacy plan and v5 links, but every candidate score was zero and
the board had no switch, gimmick, or stochastic KO boundary.

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
