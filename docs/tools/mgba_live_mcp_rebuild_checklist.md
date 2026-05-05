# mGBA Live MCP Rebuild Checklist

## Purpose

この文書は、`mgba-live-mcp` と mGBA Lua scripting 対応 build を、PC 初期化・cache 削除・別環境移行後に再現するための保全メモである。

今回の確認では、Ubuntu package の `mgba-qt` と `mGBA-0.10.5-ubuntu64-noble.tar.xz` に含まれる `mgba-qt` は `--script` option を受け付けなかった。そのため、`mgba-live-mcp` の bridge script 起動には mGBA upstream master の Qt build が必要だった。

Machine-readable inventory は `docs/tools/mcp_servers_inventory.json` と `docs/tools/tool_dependency_inventory.json` に分離している。token、API key、login URL は tracked docs に記録しない。

Cache の実体、source clone の clean 状態、backup policy、runtime cache の扱いは `docs/tools/mgba_live_cache_preservation.md` を参照する。

## Current Verified State

| Item | Value |
|---|---|
| Workspace | `/home/jastin/dev/pokeemerald-expansion` |
| Working branch at investigation time | `codex-docs-v15-investigation` |
| mGBA source used for live MCP | `.cache/mgba-script-src-master` |
| mGBA build dir | `.cache/mgba-script-build-master` |
| mGBA binary | `.cache/mgba-script-build-master/qt/mgba-qt` |
| mGBA commit | `b19b557a78930ede7ee7f5dcbc880f9ff2533ffe` |
| mGBA version output | `0.11-1-b19b557` |
| mGBA binary SHA256 observed on 2026-05-06 | `a12307b0eb85412d2fb1fac653cf43d6ff612e5bafb8b3f9cfcbd734315e9acd` |
| `mgba-live-mcp` version | `0.5.0` |
| `mgba-live-cli` path observed | `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli` |
| bridge Lua path observed | `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/lib/python3.12/site-packages/mgba_live_mcp/resources/mgba_live_bridge.lua` |
| Runtime validation display path observed | `DISPLAY=:0` with Qt/xcb |
| Runtime validation 60fps config observed | `--fps-target 60 --config videoSync=1` |
| Former local 0.10.5 archive | `mGBA-0.10.5-ubuntu64-noble.tar.xz` |
| Former local 0.10.5 archive SHA256 | `0bbf1e7ca511cd4b443239b97546f699df72211241a1db9177e331866031d8e9` |
| Former local 0.10.5 archive status | Removed from workspace on 2026-05-03 after user-approved cleanup. It was not used by the working live MCP setup. |

Approximate observed sizes:

| Path | Size | Keep? |
|---|---:|---|
| `.cache/mgba-script-src-master` | `99M` | Yes. Source tree for the working `--script` build. |
| `.cache/mgba-script-build-master` | `53M` | Yes. Working Qt + Lua scripting build. |
| `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg` | `36M` | Yes or refresh with `uvx`. Contains observed `mgba-live-mcp` package cache. |
| `.cache/mgba-script-src` | `87M` | Optional. 0.10.5 source investigation path; not the final working live build. |
| `.cache/mgba-script-build` | `49M` | Optional. 0.10.5 build investigation path; not the final working live build. |

## Download / Artifact History

| Artifact | How it appeared | Current status | Notes |
|---|---|---|---|
| Ubuntu packages | Installed through `apt` by user | Required | Exact observed versions are listed below. |
| `mgba-live-mcp` | Installed through `uvx mgba-live-mcp` | Required | Observed package version: `0.5.0`. Cache path may change after reinstall. |
| `mGBA-0.10.5-ubuntu64-noble.tar.xz` | User placed it at repo root | Removed from workspace | Contains `.deb` packages, but checked `mgba-qt` did not support `--script`. Checksum is recorded above if the artifact needs to be re-identified later. Original download URL was not confirmed in this repo. |
| `.cache/mgba-script-src` | Local 0.10.5 source investigation | Optional | Built successfully, but `--script` was still unavailable. |
| `.cache/mgba-script-src-master` | Local clone of upstream mGBA master | Required unless backed up elsewhere | Checked out at `b19b557a78930ede7ee7f5dcbc880f9ff2533ffe`. |
| `.cache/mgba-script-build-master` | Local CMake/Ninja build output | Required unless rebuilt | Produced the working `qt/mgba-qt` binary. |

Former contents of `mGBA-0.10.5-ubuntu64-noble.tar.xz` observed before deletion:

```text
mGBA-0.10.5-ubuntu64-noble/
mGBA-0.10.5-ubuntu64-noble/libmgba.deb
mGBA-0.10.5-ubuntu64-noble/mgba-qt.deb
mGBA-0.10.5-ubuntu64-noble/mgba-sdl.deb
```

## Preservation Policy

`.cache/` は再生成可能な場所として扱われやすい。誤削除されると、mGBA source clone、CMake configure、build、`uvx` package download をやり直す必要がある。

推奨:

1. `.cache/mgba-script-src-master` と `.cache/mgba-script-build-master` は重要な再現済み build として扱う。
2. 長期保管する場合は repo の Git 管理には入れず、別の backup directory、NAS、外付け disk、または private artifact storage に退避する。
3. `mGBA-0.10.5-ubuntu64-noble.tar.xz` は今回の live MCP には不十分だったため workspace から削除済み。必要になった場合は checksum を使って再取得物を照合する。
4. `mgba-live-mcp` の `uv` cache は absolute path が変わる可能性があるため、path 固定の運用にしすぎない。再構築時は `uvx mgba-live-mcp` または新しい archive path を確認する。
5. 大きな binary cache や build tree は通常 commit しない。必要なら Git LFS、release artifact、private storage などを使う。

### Suggested Backup Targets

| Source | Suggested backup name | Why |
|---|---|---|
| `.cache/mgba-script-src-master` | `mgba-script-src-master-b19b557.tar.gz` | `--script` 対応を確認した source tree。 |
| `.cache/mgba-script-build-master` | `mgba-script-build-master-b19b557-ubuntu-noble-qt5-lua54.tar.gz` | configure 済み・build 済み binary を含む。 |
| `mGBA-0.10.5-ubuntu64-noble.tar.xz` | optional, same filename plus checksum note | 削除済み。packaged stable 0.10.5 の比較が必要になった場合のみ再取得する。 |
| `/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg` | `uv-mgba-live-mcp-0.5.0-python312.tar.gz` | `mgba-live-cli` と Python package cache。 |

バックアップ先の例:

```bash
mkdir -p ~/dev-artifacts/pokeemerald-expansion/mgba-live
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/mgba-script-src-master-b19b557.tar.gz .cache/mgba-script-src-master
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/mgba-script-build-master-b19b557-ubuntu-noble-qt5-lua54.tar.gz .cache/mgba-script-build-master
tar -czf ~/dev-artifacts/pokeemerald-expansion/mgba-live/uv-mgba-live-mcp-0.5.0-python312.tar.gz /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg
sha256sum ~/dev-artifacts/pokeemerald-expansion/mgba-live/*.tar.gz > ~/dev-artifacts/pokeemerald-expansion/mgba-live/SHA256SUMS
```

注意: absolute path を含む tar を展開する場合は、展開先と所有権を確認してから実行する。

## Native Dependency Checklist

Ubuntu Noble で今回導入済み、または確認済みの package。

```bash
sudo apt install \
  git cmake ninja-build build-essential pkg-config \
  qtbase5-dev qtmultimedia5-dev qttools5-dev-tools libqt5opengl5-dev \
  libsdl2-dev \
  liblua5.4-dev \
  libavcodec-dev libavfilter-dev libavformat-dev libavutil-dev \
  libswscale-dev libswresample-dev \
  libzip-dev zlib1g-dev libpng-dev \
  libsqlite3-dev libelf-dev libedit-dev \
  libjson-c-dev libfreetype-dev \
  mgba-qt
```

Observed package versions:

| Package | Version |
|---|---|
| `build-essential` | `12.10ubuntu1` |
| `cmake` | `3.28.3-1build7` |
| `git` | `1:2.43.0-1ubuntu7.3` |
| `libavcodec-dev` | `7:6.1.1-3ubuntu5` |
| `libavfilter-dev` | `7:6.1.1-3ubuntu5` |
| `libavformat-dev` | `7:6.1.1-3ubuntu5` |
| `libavutil-dev` | `7:6.1.1-3ubuntu5` |
| `libedit-dev` | `3.1-20230828-1build1` |
| `libelf-dev` | `0.190-1.1ubuntu0.1` |
| `libfreetype-dev` | `2.13.2+dfsg-1ubuntu0.1` |
| `libjson-c-dev` | `0.17-1build1` |
| `liblua5.4-dev` | `5.4.6-3build2` |
| `libpng-dev` | `1.6.43-5ubuntu0.5` |
| `libqt5opengl5-dev` | `5.15.13+dfsg-1ubuntu1` |
| `libsdl2-dev` | `2.30.0+dfsg-1ubuntu3.1` |
| `libsqlite3-dev` | `3.45.1-1ubuntu2.5` |
| `libswresample-dev` | `7:6.1.1-3ubuntu5` |
| `libswscale-dev` | `7:6.1.1-3ubuntu5` |
| `libzip-dev` | `1.7.3-1.1ubuntu2` |
| `mgba-qt` | `0.10.2+dfsg-1.1build3` |
| `ninja-build` | `1.11.1-2` |
| `pkg-config` | `1.8.1-2build1` |
| `qtbase5-dev` | `5.15.13+dfsg-1ubuntu1` |
| `qtmultimedia5-dev` | `5.15.13-1` |
| `qttools5-dev-tools` | `5.15.13-1` |
| `zlib1g-dev` | `1:1.3.dfsg-3.1ubuntu2.1` |

Notes:

- `mgba-qt` package is useful as a normal emulator install, but the checked package did not support `--script`.
- `qttools5-dev-tools` produced a non-fatal `Qt5LinguistTools` warning during one configure path. It did not block the final master build.
- `libzip-dev` existed, but an earlier 0.10.5 source configure path failed because CMake imported a missing `/usr/bin/zipcmp`. The working master configure explicitly disabled external libzip/minizip.

## mGBA Source Build Checklist

The current working build uses upstream mGBA master, not the 0.10.5 archive.

```bash
git clone https://github.com/mgba-emu/mgba.git .cache/mgba-script-src-master
git -C .cache/mgba-script-src-master checkout b19b557a78930ede7ee7f5dcbc880f9ff2533ffe
```

Configure:

```bash
cmake -S .cache/mgba-script-src-master -B .cache/mgba-script-build-master -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-install-master \
  -DBUILD_QT=ON -DFORCE_QT_VERSION=5 \
  -DENABLE_SCRIPTING=ON -DUSE_LUA=5.4 \
  -DUSE_FREETYPE=ON -DUSE_JSON_C=ON \
  -DBUILD_SDL=OFF -DBUILD_TEST=OFF -DBUILD_SUITE=OFF \
  -DBUILD_CINEMA=OFF -DBUILD_HEADLESS=OFF -DBUILD_EXAMPLE=OFF \
  -DUSE_LIBZIP=OFF -DUSE_MINIZIP=OFF \
  -DUSE_DISCORD_RPC=OFF -DSKIP_GIT=ON
```

Build:

```bash
cmake --build .cache/mgba-script-build-master --target mgba-qt --parallel 20
```

Important CMake cache values observed:

| Option | Value |
|---|---|
| `BUILD_QT` | `ON` |
| `FORCE_QT_VERSION` | `5` |
| `ENABLE_SCRIPTING` | `ON` |
| `USE_LUA` | `5.4` |
| `USE_FREETYPE` | `ON` |
| `USE_JSON_C` | `ON` |
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

Validation:

```bash
.cache/mgba-script-build-master/qt/mgba-qt --version
.cache/mgba-script-build-master/qt/mgba-qt --script /home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/lib/python3.12/site-packages/mgba_live_mcp/resources/mgba_live_bridge.lua --version
ldd .cache/mgba-script-build-master/qt/mgba-qt | grep 'not found'
```

Expected:

- version includes `0.11-1-b19b557`.
- `--script ... --version` exits successfully.
- `ldd ... | grep 'not found'` prints nothing.

## mgba-live-mcp Checklist

Observed package:

| Item | Value |
|---|---|
| Package | `mgba-live-mcp` |
| Version | `0.5.0` |
| Python requirement | `>=3.11` |
| Runtime dependency observed | `mcp>=1.0.0` |

Codex MCP config observed earlier:

```toml
[mcp_servers.mgba]
command = "/home/jastin/.local/bin/uvx"
args = ["mgba-live-mcp"]
```

If the Codex session does not expose `mcp__mgba__...` tools after install, restart Codex and re-check tool discovery. The local CLI can still be used directly while MCP tools are unavailable.

CLI commands available in the checked version:

```text
start
attach
status
stop
run-lua
input-tap
input-set
input-clear
screenshot
read-memory
read-range
dump-pointers
dump-oam
dump-entities
```

## Live Smoke Test

Use a ROM copy outside the repo output path to avoid save/log pollution.

```bash
mkdir -p /tmp/mgba-live-smoke
cp pokeemerald.gba /tmp/mgba-live-smoke/pokeemerald.gba
```

Start:

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli start \
  --rom /tmp/mgba-live-smoke/pokeemerald.gba \
  --session-id codex-mgba-master-smoke \
  --mgba-path /home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt \
  --ready-timeout 15 \
  --fps-target 60 \
  --config videoSync=1
```

Observed issue:

- `start` once returned `Session created but bridge did not become ready before timeout`.
- Despite that, `status`, `screenshot`, `input-tap`, `dump-oam`, and `read-range` worked against the session.
- Treat this as an `Open Question` for the readiness check, not as proof that the bridge is unusable.
- `QT_QPA_PLATFORM=offscreen` on 2026-05-06 left the process alive but heartbeat stayed `null` and bridge commands timed out. Do not count offscreen as validated until this is fixed.
- `--fps-target 60` alone still ran at about 405fps in the checked Qt/xcb session. Add `--config videoSync=1` for timing-sensitive validation; a 5 second measurement produced 304 frames, about 60.8fps.

Useful verification commands:

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli screenshot --session codex-mgba-master-smoke --out /tmp/mgba-live-smoke/screen.png --timeout 8
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli input-tap --session codex-mgba-master-smoke --key A --frames 5 --timeout 8
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli dump-oam --session codex-mgba-master-smoke --count 5 --timeout 8
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli read-range --session codex-mgba-master-smoke --start 0x02000000 --length 16 --timeout 8
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli stop --session codex-mgba-master-smoke
```

Observed working behavior:

- screenshot at title menu succeeded.
- `input-tap --key A` advanced from `NEW GAME` to the opening conversation.
- `dump-oam` returned sprite entries.
- `read-range --start 0x02000000 --length 16` returned WRAM bytes.

For runtime feature validation policy and Lua pitfalls, see `docs/tools/mgba_live_runtime_validation.md`.

## Docker Option

Docker can make dependency replay easier, but it does not remove all GUI requirements. `mgba-qt` needs display support unless the workflow is changed to a headless/script-compatible build path.

Use Docker later if one of these becomes true:

- several PCs need the same build environment;
- local package versions drift and break mGBA build;
- CI smoke tests need reproducible tool installation;
- GUI display forwarding is acceptable or a headless-compatible mgba-live path is confirmed.

Native local build is currently simpler because the working path already depends on Qt GUI and a persistent live emulator session.

## Recovery Checklist

When `.cache/` is deleted or a new machine is used:

1. Install native packages from `Native Dependency Checklist`.
2. Restore backed-up `.cache/mgba-script-src-master` and `.cache/mgba-script-build-master` if available.
3. If no backup exists, clone mGBA master and checkout `b19b557a78930ede7ee7f5dcbc880f9ff2533ffe`.
4. Re-run the CMake configure command with `ENABLE_SCRIPTING=ON`, `BUILD_QT=ON`, `FORCE_QT_VERSION=5`, and `USE_LUA=5.4`.
5. Build `mgba-qt`.
6. Confirm `--script` works.
7. Reinstall or refresh `mgba-live-mcp` with `uvx`.
8. Confirm the bridge Lua path.
9. Start a smoke session with a copied ROM under `/tmp`.
10. Verify screenshot, A input, OAM dump, and WRAM read.
11. Stop the smoke session.
12. Record any changed versions or paths in this document.

## Open Questions

- Why did `mgba-live-cli start` report readiness timeout even though later commands worked?
- Should the project standardize a private artifact location for `.cache/mgba-script-build-master`?
- Should Codex config accept a configurable `mgba_path`, or should smoke commands always pass `--mgba-path` explicitly?
- Is a headless mGBA path possible for CI, or is Qt required for this `mgba-live-mcp` version?
- Should a Dockerfile be added later, or is this native checklist enough for the current solo/local workflow?
- Can `QT_QPA_PLATFORM=offscreen` be made to produce heartbeat / screenshots, or should the standard headless path use Xvfb instead?
