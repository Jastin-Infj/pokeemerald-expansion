# mGBA Live Runtime Validation

## Purpose

この文書は、mGBA Live CLI / MCP を使って runtime behavior を確認するときの運用メモである。

source change を push する前に、build だけで判断しない。field flow、battle flow、sound timing、menu operation、save/load、flag / var behavior のように ROM 実行でしか分からない変更は、mGBA で確認する。確認できない場合も、未確認理由と残リスクを `test_plan.md` か PR description に残す。

## Push Gate Policy

runtime に影響する branch では、push 前に次を確認する。

| Step | Required? | Notes |
|---|---|---|
| `make` | Required unless docs-only | ROM が作れることを確認する。 |
| `make debug` | Required for debug menu / debug symbol dependent checks | debug menu や memory symbol を使う branch では特に必要。 |
| focused mGBA check | Required when behavior is visible in-game | screenshot、input、memory、Lua のいずれかで確認する。 |
| skipped runtime check note | Required if mGBA check cannot run | missing display、missing save、missing test map、未実装 path などを記録する。 |
| session cleanup | Required | `mgba-live-cli status --all` が `[]` になるまで stop する。 |

「必ずすべてのテストを通す」ではなく、「push 前に妥当な runtime 確認を試み、できないことを明記する」を基準にする。

## Known Working Path

2026-05-06 時点で、この workspace では通常 Qt/xcb display を使う path が mGBA Live CLI の操作まで成功した。

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli start \
  --rom /tmp/mgba-live-fps/pokeemerald.gba \
  --session-id codex-mgba-runtime-check \
  --mgba-path /home/jastin/dev/pokeemerald-expansion/.cache/mgba-script-build-master/qt/mgba-qt \
  --fps-target 60 \
  --ready-timeout 15 \
  --config videoSync=1
```

Notes:

- `DISPLAY=:0` が使える環境では、この path が bridge ready になった。
- `QT_QPA_PLATFORM=offscreen` は process が残るが heartbeat が出ず、bridge command が timeout した。現時点では offscreen を成功扱いにしない。
- `xvfb-run` / `Xvfb` はこの環境では未検出。CI / headless 専用にするなら別途導入候補。

## FPS Control

`mgba-live-cli` 0.5.0 は `start` 時に mGBA へ `-C fpsTarget=<value>` を渡す。package code 上の default は `120.0`、`--fast` は `600`。

ただし `--fps-target 60` だけでは、この環境では通常速度に制限されなかった。2026-05-06 の測定結果:

| Start config | 5秒間の frame delta | Approx FPS | Result |
|---|---:|---:|---|
| `--fps-target 60` | `2028` | `405.6` | fast。音・sleep・timing 確認には不向き。 |
| `--fps-target 60 --config videoSync=1` | `304` | `60.8` | 通常速度確認に使える。 |

高速移動や長時間歩行の smoke test では uncapped / fast が便利だが、次を確認するときは `--config videoSync=1` を付ける。

- BGM / SE の違和感。
- sleep / wait / fade timing。
- menu cursor や animation の体感速度。
- frame count を test evidence として扱う検証。

## CLI Sanity Commands

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli screenshot --session codex-mgba-runtime-check --out /tmp/mgba-runtime-check/screen.png --timeout 8
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli run-lua --session codex-mgba-runtime-check --code 'return true' --timeout 8
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli stop --session codex-mgba-runtime-check
```

Screenshot が取れても、それだけで対象 behavior を確認済みにしない。画面状態、memory state、callback、flag/var、必要なら save state を合わせて見る。

## Lua Pitfalls

`run-lua` は強力だが、次で詰まりやすい。

| Pitfall | Handling |
|---|---|
| Long inline code が壊れる | 複雑な Lua は `/tmp` の file に出して `--file` で実行する。 |
| local bridge variable に触ろうとする | bridge の `frame` など local は inline Lua から直接読めない。command response の `frame` を使う。 |
| Battle flag の残り値を状態判定に使う | `gBattleTypeFlags` は field return 後も値が残ることがある。`gMain.callback2`、screen、player coords を併用する。 |
| Flag byte offset を間違える | `gSaveBlock1Ptr + flags offset + flag_id / 8` と `flag_id % 8` を分けて計算する。 |
| Repel / encounter immunity を見落とす | no encounter 系では repel var と immunity step を明示的に clear してから歩く。 |
| Input が押しっぱなしになる | `input-clear` か stop 前の cleanup を入れる。 |

## Debug Menu And Memory Tips

debug menu で到達できる path は実機操作に近い evidence になる。一方で、debug menu の ON/OFF 表示だけでは内部 flag の裏取りとして弱い。

推奨:

1. debug menu または script で目的の状態を作る。
2. Lua / memory read で flag、var、coords、map header を確認する。
3. screenshot で画面状態を残す。
4. input で対象 behavior を起こす。
5. callback / screen / memory のどれかで「起きた」「起きなかった」を判定する。

## Case Study: No Random Encounters

`feature/no-random-encounters` では、最初に build と static code check だけで push したのが不十分だった。runtime behavior は mGBA で Route 101 の草むらを歩いて確認した。

検証で使った要点:

- `make -j4` と `make debug -j4` は成功。
- OFF test: `FLAG_NO_ENCOUNTER=false` で Route 101 grass を歩き、wild Wurmple battle が発生することを screenshot で確認。
- ON test: `FLAG_NO_ENCOUNTER=true`、repel var 0、Route 101 grass を 2160 frames 歩き、field callback のまま encounter が発生しないことを確認。
- OFF battle 後に `gBattleTypeFlags` が field return 後も `4` のまま残ったため、ON 判定では `gMain.callback2` と screenshot を主に使った。
- screenshots は `/tmp/mgba-noencounter-flag-off-wild-battle.png` と `/tmp/mgba-noencounter-flag-on-after-2160frames.png` に出した。

この手の branch では、push 前の evidence として「OFF なら起きる」「ON なら起きない」の両方を見る。ON だけを見ると、map に encounter table がない、repel が残っている、草むらに乗れていない、という false positive を見逃す。

## Cleanup

調査の最後は必ず session を止める。

```bash
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli stop --session SESSION_ID
/home/jastin/.cache/uv/archive-v0/b4fssk3xyIDxQlGkquLhg/bin/mgba-live-cli status --all
```

Expected final status:

```json
[]
```
