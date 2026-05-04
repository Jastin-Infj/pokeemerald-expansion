# Field Message Peripherals v15

通常 field message (`msgbox` -> `Std_MsgboxDefault` -> `ShowFieldMessage` ->
`B_WIN_MSG` の field 版) は [Message Text Manual](../manuals/message_text_manual.md) と
[Map Script Flow v15](map_script_flow_v15.md) でカバー済みです。
この doc は、その「中央 path」から外れる field 周辺の text 経路を扱います。

## 一覧

| 種類 | 入口 | 表示先 | 中央 path との違い |
| --- | --- | --- | --- |
| `msgbox` の type 違い (`MSGBOX_NPC` / `SIGN` / `DEFAULT` / `YESNO` / `AUTOCLOSE` / `GETPOINTS` / `POKENAV`) | `asm/macros/event.inc` `msgbox` macro -> `callstd <type>` | 標準 dialogue frame | `lock` / `faceplayer` / `waitbuttonpress` / `waitfanfare` / `pokenavcall` の有無で分岐 |
| Namebox | `setspeaker` macro / inline `EXT_CTRL_CODE_SPEAKER` (`0x19`) | dialogue frame の左上に小窓を追加 | `gSpeakerName` を毎回更新する必要あり |
| Sign post | `Std_MsgboxSign` (`MSGBOX_SIGN`) | 看板用 frame (`gMsgIsSignPost = TRUE`) | `LoadSignPostWindowFrameGfx` で別 frame、 `FLAG_SAFE_FOLLOWER_MOVEMENT` を立てる |
| Pokenav call | `pokenavcall` (= `Std_MsgboxPokenav`) | Pokenav 専用 dialogue frame + namebox | `ShowPokenavFieldMessage` -> `StartMatchCallFromScript`、 `Task_HidePokenavMessageWhenDone` で hide 判定 |
| Braille | `braillemsgbox` macro (`braillemessage` + `waitbuttonpress` + `closebraillemessage`) | `sBrailleWindowId` (専用 window) | `FONT_BRAILLE` で印字、 通常 dialogue frame ではない |
| `messageinstant` | `ScrCmd_messageinstant` | 通常 dialogue frame | speed 0、 `waitbuttonpress` 無し。 link contest からの呼び出しのみ |
| `messageautoscroll` | `ScrCmd_messageautoscroll` -> `ShowFieldAutoScrollMessage` | 通常 dialogue frame | `gTextFlags.autoScroll = TRUE`、 `forceMidTextSpeed = TRUE` |
| TV ニュース | tv.c の `gTVStringVarPtrs[]` -> `gStringVar1..3` -> 通常 `msgbox` | 通常 dialogue frame | 文言生成側だけ独立。 表示は中央 path |
| Link error 画面 | `CB2_LinkError` (`SetMainCallback2`) -> `ErrorMsg_MoveCloserToPartner` / `ErrorMsg_CheckConnections` | 専用 BG + `WIN_LINK_ERROR_*` window | 通常 field UI から完全に脱離。 `gText_CommError*` を `AddTextPrinterParameterized3` で印字 |
| Minigame UI (Berry Picking, Berry Crush, Pokemon Jump 等) | 各 minigame の callback (`sGfx->windowIds[i]` 等) | 専用 window | 共通の field message helper は使わない。 `AddTextPrinter` / `AddTextPrinterParameterized3` を直叩き |
| Union Room chat keyboard | `src/union_room_chat.c` (`gText_UnionRoomChatKeyboard_*`) | 専用 BG + 専用 window | text engine は共通だが、 layout / sprite はすべて自前 |

## msgbox type の分岐

`asm/macros/event.inc` (line 2068-2084):

```
MSGBOX_NPC = 2
MSGBOX_SIGN = 3
MSGBOX_DEFAULT = 4
MSGBOX_YESNO = 5
MSGBOX_AUTOCLOSE = 6
MSGBOX_GETPOINTS = 9
MSGBOX_POKENAV = 10

.macro msgbox text, type=MSGBOX_DEFAULT
    loadword 0, \text
    callstd \type
.endm
```

各 type の本体は `data/scripts/std_msgbox.inc` (一部は `data/scripts/trainer_battle.inc` の `Std_MsgboxAutoclose`):

| Std script | 動作 |
| --- | --- |
| `Std_MsgboxNPC` | `lock` / `faceplayer` -> `message NULL` -> `waitmessage` -> `waitbuttonpress` -> `release` |
| `Std_MsgboxSign` | `setflag FLAG_SAFE_FOLLOWER_MOVEMENT` -> `lockall` -> `message NULL` -> `waitmessage` -> `waitbuttonpress` -> `releaseall` -> `clearflag FLAG_SAFE_FOLLOWER_MOVEMENT` |
| `Std_MsgboxDefault` | `message NULL` -> `waitmessage` -> `waitbuttonpress` |
| `Std_MsgboxYesNo` | `message NULL` -> `waitmessage` -> `yesnobox 20, 8` |
| `Std_MsgboxAutoclose` | `data/scripts/trainer_battle.inc` (line 132)。 内容は `message NULL` -> `waitmessage` -> `waitbuttonpress` -> `release` -> `return`。 `Std_MsgboxNPC` との違いは「`lock` / `faceplayer` を行わない」点。 主な使用例は trainer 戦の post-battle (FRLG trainer の `Route3_Text_*PostBattle` 等で `msgbox ..., MSGBOX_AUTOCLOSE`)、 戦闘終了直後の発話で player の向きを変えたくないとき。 |
| `Std_MsgboxGetPoints` | `message NULL` -> `playfanfare MUS_OBTAIN_B_POINTS` -> `waitfanfare` -> `waitmessage` |
| `Std_MsgboxPokenav` | 旧用。 現状 `pokenavcall` macro が `ScrCmd_pokenavcall` を直接呼ぶので未使用。 |

`message`, `waitmessage`, `closemessage` は `src/scrcmd.c` (`ScrCmd_message`, `ScrCmd_waitmessage`, `ScrCmd_closemessage`) で `ShowFieldMessage`、 `IsFieldMessageBoxHidden`、 `HideFieldMessageBox` に対応。

## Namebox

主なファイル:

| File | Role |
| --- | --- |
| `src/field_name_box.c` | `TrySpawnNamebox`, `TrySpawnAndShowNamebox`, `GetNameboxWindowId`, `DestroyNamebox`, `FillNamebox`, `DrawNamebox`, `ClearNamebox`, `SetSpeaker`, `gSpeakerName`, `sNameboxWindowId`。 sprite は使わず `AddWindow` した small window を `tilemapLeft = 2, tilemapTop = 13` に置く。 |
| `src/field_message_box.c` | `Task_DrawFieldMessage` の `case 1` で `GetNameboxWindowId() != WINDOW_NONE` のとき `DrawNamebox(...)` を呼ぶ。 つまり namebox は dialogue frame と同時に再描画される。 |
| `src/text.c` `case EXT_CTRL_CODE_SPEAKER` | text engine 内で `0x19` を見たら `gSpeakerNamesTable[name]` で `TrySpawnAndShowNamebox`。 |
| `asm/macros/event.inc` `setspeaker` | `callnative SetSpeaker`。 `SP_NAME_*` constant か ROM の string ポインタを取る。 |
| `data/speaker_names.h`, `include/constants/speaker_names.h` | `gSpeakerNamesTable[]` と enum。 |
| `match_call.c` line 1329 | match call では `TrySpawnAndShowNamebox(gSpeakerName, NAME_BOX_BASE_TILE_NUM)` を直接呼ぶ。 これで Pokenav call 時の話者表示が出る。 |

注意点:

- `gSpeakerName` を NULL のまま `Std_MsgboxNPC` で message を出すと namebox は出ない (`TrySpawnNamebox` が早期 return)。
- `OW_FLAG_SUPPRESS_NAME_BOX` が立っているか、 string の `AllocZeroed` が失敗した場合は強制的に `DestroyNamebox`。
- `DestroyNamebox` は `gSpeakerName = NULL` にする。 message 1 つに付き 1 回だけ表示する設計。
- `OW_NAME_BOX_USE_DYNAMIC_WIDTH` が TRUE なら `ConvertPixelWidthToTileWidth(GetStringWidth(...))` で名前長に応じて幅変更、 上限は `OW_NAME_BOX_DEFAULT_WIDTH`。
- `IsMatchCallTaskActive()` のときは `sNameBoxPokenavGfx` + palette 14 を使い、 文字色も `1 / 0` に変える。 通常時は `sNameBoxDefaultGfx` + `DLG_WINDOW_PALETTE_NUM`。

## Sign post

「sign 用の枠で出す」のと「`Std_MsgboxSign` で呼ぶ」は別の機構。 両方が組み合わさることもある。

- 枠の切り替えは `gMsgIsSignPost` flag。 declaration は `src/script.c` line 36、 set は `src/field_control_avatar.c` line 1305 の `SetMsgSignPostAndVarFacing` (`MetatileBehavior_IsPokemonCenterSign` / `IsPokeMartSign` / `IsSignpost` で signpost metatile を踏んだ場合)。 同関数で `gWalkAwayFromSignpostTimer = WALK_AWAY_SIGNPOST_FRAMES` と `gMsgBoxIsCancelable = TRUE` も同時に set。 reset は `src/field_control_avatar.c` line 169 (player が tile から離れたタイミングで毎フレーム decrement する場所も同 file line 1345 にある)。
- `Task_DrawFieldMessage` の `case 0` で `gMsgIsSignPost` が立っていれば `LoadSignPostWindowFrameGfx` (定義は `src/menu.c` line 211)、 そうでなければ `LoadMessageBoxAndBorderGfx`。 `WindowFunc_DrawSignFrame` / `WindowFunc_DrawDialogueFrame` の切り替えも `src/menu.c` (line 243 周辺) で `gMsgIsSignPost` を見て決める。
- `Std_MsgboxSign` (`callstd MSGBOX_SIGN`) は `lockall` / `releaseall` と `setflag FLAG_SAFE_FOLLOWER_MOVEMENT` / `clearflag` を伴う Std script。 これ自体は `gMsgIsSignPost` を直接いじらない。
- スクリプトから `msgbox X, MSGBOX_SIGN` を呼ぶと「sign 風 lock 動作だけ」になる。 「sign frame で出す」効果は metatile 経由でしか発生しない。 NPC イベントから sign 枠を出したい場合は metatile を sign に貼る必要がある。
- follower NPC の追従抑制は `FLAG_SAFE_FOLLOWER_MOVEMENT` で行う。 `Std_MsgboxSign` だけがこの flag を立てる。

## Pokenav call

`pokenavcall TEXT_PTR` macro -> `SCR_OP_POKENAVCALL` -> `ScrCmd_pokenavcall` -> `ShowPokenavFieldMessage(msg)` の流れ。

`src/field_message_box.c` `ShowPokenavFieldMessage`:

```
StringExpandPlaceholders(gStringVar4, str);
CreateTask(Task_HidePokenavMessageWhenDone, 0);
StartMatchCallFromScript(str);
sFieldMessageBoxMode = FIELD_MESSAGE_BOX_NORMAL;
```

つまり通常 `ShowFieldMessage` の `Task_DrawFieldMessage` ではなく、 match call 用の task に切り替わる。 hide 判定は `IsMatchCallTaskActive` で `Task_HidePokenavMessageWhenDone` が消す。

`src/match_call.c` の `StartMatchCallFromScript` (line 1174) -> `sMatchCallState.triggeredFromScript = TRUE` -> `StartMatchCall` -> `PlaySE(SE_POKENAV_CALL)` -> `CreateTask(ExecuteMatchCall, 1)`。 ここで `triggeredFromScript` を立てておくと `LockPlayerFieldControls` / `FreezeObjectEvents` / `PlayerFreeze` をスキップする (script 側で既に lock 済みなので)。 任意の C 経路 (例: 街でランダムに着信) から呼ぶときは `StartMatchCall` を直接呼ぶと lock を取り直す。

`src/match_call.c` 内で Pokenav frame (`sMatchCallWindow_Gfx` / `sMatchCallWindow_Pal`)、 Pokenav 用 namebox (palette 14 + `sNameBoxPokenavGfx`)、 trainer pic 等を組む。 `IsMatchCallTaskActive()` を見て `field_name_box.c` 側 (`GetNameboxGraphics`) も Pokenav graphics に切り替える。 `gSpeakerName` を `match_call.c` 内で `TrySpawnAndShowNamebox(gSpeakerName, NAME_BOX_BASE_TILE_NUM)` (line 1329) で更新する。

`Std_MsgboxPokenav` (`MSGBOX_POKENAV = 10`) は table 上 `pokenavcall NULL` -> `waitmessage` -> `return` の Std script だが、 実 code は `msgbox X, MSGBOX_POKENAV` の経路を経ずに `pokenavcall TEXT_PTR` macro (= `ScrCmd_pokenavcall` 直接) を使うのが一般的。

## Braille

`asm/macros/event.inc` `braillemsgbox`:

```
braillemessage \text
waitbuttonpress
closebraillemessage
```

`braillemessage` -> `ScrCmd_braillemessage` (`src/scrcmd.c` line 2041):

- `StringExpandPlaceholders(gStringVar4, ptr + 6)` (先頭 6 byte は RS 時代の位置情報、 Emerald では未使用)
- `width = GetStringWidth(FONT_BRAILLE, gStringVar4, -1) / 8` で window 幅自動計算 (上限 28)
- `height = 4 + 3 * (CHAR_NEWLINE 出現数)` で高さ自動計算 (上限 18)
- `xWindow`, `yWindow`, `xText`, `yText` を上記から決定し、 `CreateWindowTemplate(0, ...)`、 `AddWindow`
- `LoadUserWindowBorderGfx(sBrailleWindowId, 0x214, BG_PLTT_ID(14))` -> `DrawStdWindowFrame` -> `FillWindowPixelBuffer(... PIXEL_FILL(1))` -> `AddTextPrinterParameterized(sBrailleWindowId, FONT_BRAILLE, gStringVar4, xText, yText, TEXT_SKIP_DRAW, NULL)` -> `CopyWindowToVram(sBrailleWindowId, COPYWIN_FULL)`

`closebraillemessage` -> `ScrCmd_closebraillemessage` -> `CloseBrailleWindow()`。

通常 dialogue frame は引かないので `gMsgIsSignPost` も `gSpeakerName` も影響しない。 `FONT_BRAILLE` は固定。 文言は普通の string と同様 `gStringVar4` 経由で書ける。

## TV

`src/tv.c` は文字列を直接描画しない。 各 `DoTVShow*` 関数が `gTVStringVarPtrs[]` (== `gStringVar1`..`gStringVar4`) に値を埋め、 戻った先の field script 側で通常の `msgbox` / `message` を出す。

つまり TV 関連の差し替えは:

1. show 種別 -> string 設定: `src/tv.c` の `DoTVShow*` (例: `DoTVShowPokemonFanClubLetter`, `DoTVShowPokemonContestLiveUpdates` 等)
2. 表示文言: `data/maps/<TVMap>/scripts.inc` の `msgbox` で使う text。 placeholder (`{STR_VAR_1}` etc.) で受ける。

ニュースの題目 / カテゴリ表示は `gTVStringVarPtrs[varIdx]` に `gStdStrings[STDSTRING_*]` (例: `STDSTRING_NORMAL`, `STDSTRING_COOL`, `STDSTRING_BEAUTY`) を入れる。

## Link error

`src/link.c` の `CB2_LinkError` 経路:

- `SetMainCallback2(CB2_LinkError)` で field を完全に置き換える。
- `sLinkErrorBgTemplates`, `sLinkErrorWindowTemplates` を init し、 BG / window を専用に確保。
- `WIN_LINK_ERROR_TOP`, `WIN_LINK_ERROR_MID`, `WIN_LINK_ERROR_BOTTOM` の 3 window enum (`src/link.c` 内)。
- `ErrorMsg_MoveCloserToPartner` (RFU 切断) -> `gText_CommErrorEllipsis` + `gText_MoveCloserToLinkPartner`。
- `ErrorMsg_CheckConnections` (有線切断) -> `gText_CommErrorCheckConnections`。
- 結果待ちで `gText_ABtnTitleScreen` / `gText_ABtnRegistrationCounter` を追記。
- すべて `AddTextPrinterParameterized3(WIN_LINK_ERROR_*, FONT_SHORT_COPY_1, ...)` で書く。 標準 dialogue frame は使わない。

`gLinkErrorOccurred`, `gSuppressLinkErrorMessage`, `SetLinkErrorBuffer`, `TrySetLinkErrorBuffer` などで error 情報を保持。 トップ画面に戻る / 再接続を待つ動作はここで完結する。

## Minigame UI

通信 minigame は textbox 構成も完全に独自:

| File | 主な window 構成 |
| --- | --- |
| `src/dodrio_berry_picking.c` | `sGfx->windowIds[i]` を `AddTextPrinterParameterized` / `AddTextPrinterParameterized3` で利用。 `sRecordsTexts`, `sRankingTexts`, `gText_BerryPickingRecords`、 `sRecordYCoords` 等で position 制御。 |
| `src/pokemon_jump.c` | 同様に self-contained。 |
| `src/berry_crush.c` | `game->gfx.resultsWindowId` などを介した結果画面。 `sResultsTexts` を `StringExpandPlaceholders` -> `gStringVar4` -> `AddTextPrinterParameterized3`。 |
| `src/union_room_chat.c` | キーボード layout、入力履歴、 player list。 `gText_UnionRoomChatKeyboard_*` 等の table。 |

minigame text はすべて message engine が裏で同じ `AddTextPrinter` 系を使うが、 layout / palette / scroll は各 minigame ごとに別実装。 「field message を整える」修正で巻き込みづらい。

## Trade / Daycare 等の標準フロー (参考)

`asm/macros/event.inc` には `trade_pokemon`, `teach_move` などの higher-level macro があり、内部で `msgbox \wantTradeMsg, MSGBOX_YESNO` -> `... MSGBOX_DEFAULT` のように標準 type を組み合わせて動く。 文言を変えるときは macro arg に渡す string を差し替えればよい (中央 path)。

## 関連 docs

- [Message Text Manual](../manuals/message_text_manual.md) — battle / field の入口判定。
- [Map Script Flow v15](map_script_flow_v15.md) — field script 全体の流れ。
- [Battle Text Routes v15](battle_text_routes_v15.md) — battle 側で中央 path に乗らない経路。
- [Contest Message Flow v15](contest_message_flow_v15.md) — Contest 専用 textbox。
- [How to Namebox](../tutorials/how_to_namebox.md) — namebox を新規に足す手順 (config / table 拡張)。
