# mGBA Live Cache Preservation

## Purpose

この文書は、mGBA Live MCP / CLI を動かすために workspace と home cache に作られた local artifacts を、誤削除や PC 初期化から守るための運用メモである。

結論:

- `.cache/` と `/home/jastin/.cache/uv/...` は Git では戻せない。
- working mGBA source clone は clean で、未コミットの手編集 source はない。
- 動く状態を保つには、mGBA source checkout、CMake build output、`mgba-live-mcp` uv cache、復旧手順 docs の 4 点を守る。

## Current Cache Inventory

確認日: 2026-05-06.

| Path | Size | Keep | Role |
|---|---:|---|---|
| `.cache` | `323M` | local-only | workspace-local tool cache root。Git 追跡対象外。 |
| `.cache/mgba-script-src-master` | `99M` | Required | working mGBA source checkout。`--script` 対応 build の source。 |
| `.cache/mgba-script-build-master` | `53M` | Required | working Qt + Lua scripting build output。`qt/mgba-qt` を含む。 |
| `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg` | `36M` | Required or refreshable | `mgba-live-mcp 0.5.0` package cache。`mgba-live-cli` と bridge Lua を含む。 |
| `/home/jastin/.mgba-live-mcp/runtime` | `608K` | Optional evidence | session metadata、logs、archived bridge copies、screenshots。active session はなし。 |
| `.cache/mgba-script-src` | `87M` | Optional | older 0.10.5 source investigation checkout。final working path ではない。 |
| `.cache/mgba-script-build` | `49M` | Optional | older 0.10.5 build investigation output。final working path ではない。 |
| `.cache/clangd` | `38M` | Optional | clangd / LSP index cache。mGBA Live には不要。 |

## Git Tracking Status

`.cache/` は repository tracked file ではない。local の `.git/info/exclude` に次の exclude がある。

```text
.cache/
```

`git check-ignore -v .cache/mgba-script-build-master/qt/mgba-qt` は `.git/info/exclude:13:.cache/` を返す。

注意:

- `.git/info/exclude` は local file であり、clone 先には配られない。
- fresh clone で `.cache/` を再作成した場合、local exclude を入れないと untracked に見える可能性がある。
- `.cache/`、ROM、save、screenshots、runtime logs は commit しない。

## Source Modification Check

2026-05-06 の確認では、mGBA source tree に手編集差分はなかった。

| Path | Git state | HEAD |
|---|---|---|
| `.cache/mgba-script-src-master` | `master...origin/master`, diff empty | `b19b557a78930ede7ee7f5dcbc880f9ff2533ffe` |
| `.cache/mgba-script-src` | detached HEAD, diff empty | `26b7884bc25a5933960f3cdcd98bac1ae14d42e2` |

つまり、cache に残っている重要物は「source patch」ではなく「再現済み checkout と build output」。復旧不能リスクは、未コミット改造 source の消失ではなく、同じ commit / option / dependency で build を再現する手間である。

## Working Build Identity

Working binary:

```text
.cache/mgba-script-build-master/qt/mgba-qt
```

Observed:

| Item | Value |
|---|---|
| Version output | `0.11-1-b19b557 (b19b557a78930ede7ee7f5dcbc880f9ff2533ffe)` |
| Binary SHA256 | `a12307b0eb85412d2fb1fac653cf43d6ff612e5bafb8b3f9cfcbd734315e9acd` |
| Main linked local library | `.cache/mgba-script-build-master/libmgba.so.0.11` |
| Lua library | `/usr/lib/x86_64-linux-gnu/liblua5.4.so` |

Important CMake options from `.cache/mgba-script-build-master/CMakeCache.txt`:

| Option | Value |
|---|---|
| `CMAKE_BUILD_TYPE` | `Release` |
| `BUILD_QT` | `ON` |
| `FORCE_QT_VERSION` | `5` |
| `ENABLE_SCRIPTING` | `ON` |
| `USE_LUA` | `5.4` |
| `BUILD_SDL` | `OFF` |
| `BUILD_TEST` | `OFF` |
| `BUILD_SUITE` | `OFF` |
| `BUILD_CINEMA` | `OFF` |
| `BUILD_HEADLESS` | `OFF` |
| `BUILD_EXAMPLE` | `OFF` |
| `USE_LIBZIP` | `OFF` |
| `USE_MINIZIP` | `OFF` |
| `USE_DISCORD_RPC` | `OFF` |
| `SKIP_GIT` | `ON` |

Why this build exists:

- Ubuntu package `/usr/games/mgba-qt` (`0.10.2`) is useful as normal emulator, but did not support the observed `--script` bridge path.
- The checked `mGBA-0.10.5-ubuntu64-noble.tar.xz` package also did not provide the needed `--script` behavior.
- The final working path uses mGBA upstream master at `b19b557...`, Qt5, Lua 5.4, and scripting enabled.
- `USE_LIBZIP=OFF` / `USE_MINIZIP=OFF` avoided earlier package / CMake trouble around zip tooling while preserving the needed live scripting path.

## `mgba-live-mcp` Package Cache

Observed package:

| Item | Value |
|---|---|
| Package | `mgba-live-mcp` |
| Version | `0.5.0` |
| Requires-Python | `>=3.11` |
| Requires-Dist | `mcp>=1.0.0` |
| CLI | `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli` |
| Bridge Lua | `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/lib/python3.12/site-packages/mgba_live_mcp/resources/mgba_live_bridge.lua` |

The uv archive path is content/cache specific. After reinstalling with `uvx mgba-live-mcp`, the path may change. Do not hard-code the archive path in feature docs without also saying it is an observed local path.

## Feature Surface Preserved By The Cache

The package cache contains both CLI and MCP server code. The important observed capabilities are:

| Layer | Capability |
|---|---|
| `SessionManager.start` | start managed mGBA process with ROM, optional savestate, fps target, mGBA path, startup Lua, heartbeat paths, and bridge script staging. |
| `SessionManager.attach/status/stop` | attach to managed session, inspect session metadata / heartbeat, stop process and archive session metadata. |
| `SessionManager.run_lua` | run inline or file-based Lua in the live process. |
| `SessionManager.input_tap/input_set/input_clear` | press or clear GBA keys through bridge commands. |
| `SessionManager.screenshot/get_view` | persist screenshots or return screenshot data through MCP. |
| `SessionManager.read_memory/read_range` | read sparse or contiguous memory. |
| `SessionManager.dump_pointers/dump_oam/dump_entities` | inspect pointer tables, OAM, and structured entity bytes. |
| `mgba_live_bridge.lua` | frame callback loop, heartbeat, command file parsing, response JSON, screenshot, input, memory read, Lua file/inline execution. |

MCP tool names observed in `server.py` include:

```text
mgba_live_start
mgba_live_start_with_lua
mgba_live_start_with_lua_and_view
mgba_live_attach
mgba_live_status
mgba_live_get_view
mgba_live_stop
mgba_live_run_lua
mgba_live_run_lua_and_view
mgba_live_input_tap
mgba_live_input_tap_and_view
mgba_live_input_set
mgba_live_input_clear
mgba_live_export_screenshot
mgba_live_read_memory
mgba_live_read_range
mgba_live_dump_pointers
mgba_live_dump_oam
mgba_live_dump_entities
```

If these disappear after cache deletion or Codex restart, first confirm `uvx mgba-live-mcp` can expose the same package version and that Codex MCP config still points to `uvx mgba-live-mcp`.

## Runtime Cache

Runtime state lives under:

```text
/home/jastin/.mgba-live-mcp/runtime
```

Current status:

- Size: `608K`.
- Active sessions: `mgba-live-cli status --all` returned `[]`.
- `archived_sessions/` contains session metadata and logs from smoke tests, FPS checks, and no-random-encounter validation.
- session directories contain `session.json`, `heartbeat.json` when bridge ran, `response.json`, `stdout.log`, `stderr.log`, staged `mgba_live_bridge.lua`, and screenshots when captured.

Preservation:

- Runtime cache is useful evidence, but not required to start future sessions.
- If a session proves a bug or validation result, copy only the needed screenshot / log to `/tmp` or a named external artifact folder and reference it in docs.
- Do not commit runtime session directories.

## Backup Policy

Before deleting `.cache/`, moving machines, reinstalling Python/uv, or cleaning home cache, make an external artifact backup.

Suggested target:

```text
~/dev-artifacts/pokeemerald-expansion/mgba-live/
```

Suggested artifacts:

| Source | Archive name |
|---|---|
| `.cache/mgba-script-src-master` | `mgba-script-src-master-b19b557.tar.gz` |
| `.cache/mgba-script-build-master` | `mgba-script-build-master-b19b557-ubuntu-noble-qt5-lua54.tar.gz` |
| `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg` | `uv-mgba-live-mcp-0.5.0-python312.tar.gz` |
| `/home/jastin/.mgba-live-mcp/runtime` | `mgba-live-runtime-optional-YYYYMMDD.tar.gz` |

Use relative archive paths for workspace cache and be careful with absolute home paths.

```bash
mkdir -p ~/dev-artifacts/pokeemerald-expansion/mgba-live
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/mgba-script-src-master-b19b557.tar.gz .cache/mgba-script-src-master
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/mgba-script-build-master-b19b557-ubuntu-noble-qt5-lua54.tar.gz .cache/mgba-script-build-master
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/uv-mgba-live-mcp-0.5.0-python312.tar.gz -C /home/jastin/.cache/uv/archive-v0 b4fssk3xyIDxQlGkquLhg
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/mgba-live-runtime-optional-20260506.tar.gz -C /home/jastin/.mgba-live-mcp runtime
sha256sum ~/dev-artifacts/pokeemerald-expansion/mgba-live/*.tar.gz > ~/dev-artifacts/pokeemerald-expansion/mgba-live/SHA256SUMS
```

This docs task records what to preserve. It does not mean those tarballs already exist.

## Recovery Order

If cache is deleted:

1. Restore `.cache/mgba-script-src-master` and `.cache/mgba-script-build-master` from external artifacts if available.
2. Restore or refresh `mgba-live-mcp` package cache with `uvx mgba-live-mcp`.
3. Confirm the new `mgba-live-cli` path and bridge Lua path.
4. Confirm mGBA binary identity:

```bash
.cache/mgba-script-build-master/qt/mgba-qt --version
sha256sum .cache/mgba-script-build-master/qt/mgba-qt
```

5. Confirm scripting support:

```bash
.cache/mgba-script-build-master/qt/mgba-qt --script /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/lib/python3.12/site-packages/mgba_live_mcp/resources/mgba_live_bridge.lua --version
```

6. Start a smoke session using explicit `--mgba-path`.
7. Run `status`, `screenshot`, `run-lua`, `read-range`, then `stop`.
8. Update this document if the commit, package path, or version changed.

If no backup exists, rebuild from source using `docs/tools/mgba_live_mcp_rebuild_checklist.md`. Expect network access, package installation, CMake configure, and Ninja build time.

## Routine Operations

- Always pass `--mgba-path /home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt` unless Codex MCP config has been updated and verified.
- Use `DISPLAY=:0` / Qt xcb path for current local validation. `QT_QPA_PLATFORM=offscreen` was observed to leave process alive without heartbeat.
- Use `--config videoSync=1` when timing or sound matters.
- Use `mgba-live-cli status --all` before and after tests.
- Stop sessions explicitly; final expected status is `[]`.
- Treat save files, ROM copies, screenshots, and runtime logs as local artifacts, not project source.

## When To Refresh This Document

Update this document when any of these change:

- mGBA source commit.
- CMake options.
- `mgba-live-mcp` version or uv archive path.
- Python version used by uv cache.
- Qt / Lua / ffmpeg system package generation.
- Headless/offscreen strategy.
- External artifact storage location.
