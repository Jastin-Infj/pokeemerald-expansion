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

## Save Data And Startup Preconditions

runtime validation は、ROM 起動後の状態に依存する。clean ROM では、通常は title screen、新規 game、主人公選択、博士の説明、truck から始まる。既存 save がある場合だけ、今回のようにすぐ field へ入れる。

検証前に次を明記する。

| Need | Handling |
|---|---|
| 既存 progress が必要 | user に save / savestate を求める。local save がある前提で進めない。 |
| 草むら、洞窟、水上、特定 NPC が必要 | map group / map num / coords / metatile を確認してから input を流す。 |
| debug menu が使える | warp、flag/var、give item、party setup に使う。ただし field control 前の intro text は bypass できないことがある。 |
| save がない | new-game setup を明示的な準備手順として扱う。検証時間に含め、PR / test plan に残す。 |

save file は検証 artifact。tracked docs や commit に含めない。ROM copy と save copy は `/tmp` に置き、必要なら path と checksum だけを記録する。

## Save Reuse And Clean-Start Decision

context を節約したいときは、毎回 clean start をやり直す前に save reuse が可能かを先に判定する。ただし save block layout が変わる branch では、古い `.sav` が読めても検証 evidence として弱くなる。

判断基準:

| Condition | Action |
|---|---|
| SaveBlock1 / SaveBlock2 / SaveBlock3、party struct、item storage、flag / var ID を変えた | clean start を優先する。既存 `.sav` が読めない、または初期化される前提で扱う。 |
| save layout は変えていないが runtime behavior だけを変えた | 既存 `.sav` を `/tmp` にコピーして reuse してよい。読み込めたこと、map / coords / party を短い report に残す。 |
| `.sav` が読み込めず title に `CONTINUE` が出ない | clean start path に切り替える。失敗した `.sav` は artifact として path だけ記録し、tracked docs へ入れない。 |
| user が特定地点 save / savestate を提供した | その前提を report の最初に書く。branch に同梱しない。 |
| 初期化挙動そのものを検証する | ROM basename を変えた temporary directory で sibling `.sav` がない状態を作る。 |

2 回目以降の report は、save block 形態が変わっていなければ短くてよい。最低限、対象 commit、ROM basename、`.sav` を使ったか、`CONTINUE` が出たか、開始 map / coords、party が valid か、Debug menu を使ったか、実画面で見た結果を残す。

clean start が必要になった場合は、`docs/tools/mgba_live_init_build_validation.md` の route を再利用する。Debug menu は field control 後なら使ってよいが、intro text、truck、最初の室内操作を飛ばせるとは限らない。

## FPS Control

`mgba-live-cli` 0.5.0 は `start` 時に mGBA へ `-C fpsTarget=<value>` を渡す。package code 上の default は `120.0`、`--fast` は `600`。GBA native は約 59.73fps だが、local validation では実用上 `60` を通常速度の近似として扱う。

ただし `--fps-target 60` だけでは、この環境では通常速度に制限されなかった。2026-05-06 の測定結果:

| Start config | 5秒間の frame delta | Approx FPS | Result |
|---|---:|---:|---|
| `--fps-target 60` | `2028` | `405.6` | fast。音・sleep・timing 確認には不向き。 |
| `--fps-target 60 --config videoSync=1` | `304` | `60.8` | 通常速度確認に使える。 |
| `--fps-target 180 --config videoSync=1` | `919` | `183.7` | 高速調査用。manual input 判定には注意。 |

速度の使い分け:

| Purpose | Suggested config |
|---|---|
| BGM / SE / fade / wait timing | native 相当の `--fps-target 60 --config videoSync=1` |
| 通常の目視 debug / menu 操作 | `--fps-target 60 --config videoSync=1` または `--fps-target 120 --config videoSync=1` |
| 長距離移動、encounter 発生待ち、smoke automation | `--fps-target 120 --config videoSync=1` or `--fps-target 180 --config videoSync=1` |
| 240fps 以上 | 原則避ける。画面と button 判定を追いにくい。 |

高速移動や長時間歩行の smoke test では 120 / 180 が便利だが、次を確認するときは 60 近辺に戻す。

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
| Lua で状態を作っただけで検証完了にする | 実画面、callback、memory、input result のどれかを合わせて確認する。Debug setup は setup であり、feature behavior の evidence とは分ける。 |

Lua の escaping、`--file` template、SaveBlock flag template、frame macro template は `docs/tools/mgba_live_lua_templates.md` を参照する。

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
