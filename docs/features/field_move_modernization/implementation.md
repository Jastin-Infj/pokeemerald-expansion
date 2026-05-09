# Field Move Modernization Implementation Notes

最終更新: 2026-05-09
ブランチ: `feature/field-move-toolkit-item` (stacked on `feature/field-move-modernization-mvp`)
ベースライン: `master` `4af2beb493`

## Implemented Runtime Slice

このブランチでは、HM field move の最初の runtime slice として以下を実装した。

- `OW_FIELD_MOVE_MODERNIZATION` を追加し、HM field move の script 判定を「badge unlock + field move 種別」で通すようにした。
- `ScrCmd_checkfieldmove` は modernized HM では `MonKnowsMove` を要求しない。既存 script 互換のため、`VAR_RESULT` には表示用 party slot を返す。
- Surf の A ボタン導線は `PartyHasMonWithSurf()` を必須にせず、badge unlock と surfable tile で `EventScript_UseSurf` へ入る。
- `OW_FIELD_MOVE_SHOW_MON_EFFECT` を追加し、Pokemon show-mon banner / 黒背景 cut-in を無効化した。
- `FLDEFF_FIELD_MOVE_SHOW_MON_INIT` は無効時に active list を即解除するため、Surf / Waterfall / Dive / Fly / Cut / Rock Smash / Strength の待機 state は止まらず次へ進む。
- Cut / Rock Smash / Strength / Surf / Waterfall の成功時 yes/no prompt と使用完了メッセージを削除した。失敗時メッセージは維持する。
- Dive / Surface は誤操作で即 map warp しやすいため、yes/no prompt と使用メッセージを残した。
- Flash は cave map load 時に `FIELD_MOVE_FLASH` が unlock 済みなら `FLAG_SYS_USE_FLASH` を自動 set し、`SetDefaultFlashLevel()` で明るい flash level に入るようにした。

## Field Kit Itemization Slice

`feature/field-move-toolkit-item` では、HM を直接 field progression item として配布する形から、単一 Key Item の `ITEM_FIELD_KIT` に寄せた。

- `ITEM_FIELD_KIT` を Key Items pocket に追加した。アイコンは後で生成画像へ差し替えやすいよう、現時点では `gItemIcon_QuestionMark` / `gItemIconPalette_QuestionMark` を placeholder として使う。
- `Common_EventScript_GiveFieldKit` を追加し、Field Kit 未所持なら配布、所持済みなら何もしない共通 script にした。
- Cut / Flash / Rock Smash / Strength / Surf / Fly / Dive / Waterfall の Emerald 入手イベントは、各 HM item の代わりに `Common_EventScript_GiveFieldKit` を呼ぶ。
- Field Kit 配布が失敗した場合は既存 `Common_EventScript_ShowBagIsFull` に入り、対応 `FLAG_RECEIVED_HM_*` を立てない。
- 既存の `FLAG_RECEIVED_HM_*` は Field Kit capability flags として再利用する。新規 save flag は増やしていない。
- `OW_FIELD_MOVE_TOOLKIT_REQUIRED` を追加し、modernized HM field move は Field Kit 所持 + 対応 capability flag を要求する。
- `OW_FIELD_MOVE_TOOLKIT_BADGES` を追加し、初期値では既存 badge gate も維持する。進行順を壊さず item 化するための安全寄せ。将来 capability-only にする場合はここを FALSE にする。
- Debug menu の `Scripts...` 先頭 4 項目を Field Kit 検証用にした。`Field Kit Full` は Field Kit 付与、全 HM capability flags、全 badge flags を一括付与する。`Field Kit Item` は item のみ、`Field Kit Flags` は capability flags のみ、`Field Kit Clear` は Field Kit item と capability / system flags を消す。
- FRLG は `FLAG_RECEIVED_HM_*` が placeholder 0 のため、この slice では toolkit requirement を適用せず既存 badge gate を維持する。

## Deliberately Unchanged

この slice では以下はまだ変更していない。

- map object 配置。Cut tree / Rock Smash rock / Strength boulder は現行 map design のまま。
- party menu の field move action。現行どおり「Pokemon が技を持つ場合だけ」表示される。
- Fly の代替 UI。Poke Rider / region map shortcut は Phase 3 扱い。
- HM item 自体の通常技化、TM/HM pocket、learn move UI。
- Key Item bag capacity 拡張。別 feature の大型改修として扱う。
- `P_CAN_FORGET_HIDDEN_MOVE`、`CannotForgetMove`、Move Deleter、PC release、catch-swap HM checks。
- Rock Smash wild encounter と Rusturf Tunnel update。

## Impact Notes

`VAR_RESULT` は modernized HM でも `PARTY_SIZE` ではなく party slot を返す。これは既存 script / field effect の入力形を壊さないための互換措置であり、Pokemon show-mon banner を戻す場合も out-of-range にならない。

Pokemon show-mon banner を無効化する箇所は `field_effect.c` に閉じている。呼び出し側の task は従来どおり `FLDEFF_FIELD_MOVE_SHOW_MON` の終了を待つが、無効時は active list に入らないため即座に次 state へ進む。

Flash auto-use は manual Flash field effect (`FldEff_UseFlash`) を map load 中に起動せず、map 初期化中に `FLAG_SYS_USE_FLASH` だけを set する。既存の cave enter transition / field callback / script context と競合させないための安全寄せ。入場時に明滅演出まで入れる場合は、map load callback と palette fade の順序を別途検証する。

Dive の field input は既存コード上、Dive down が A button (`TrySetupDiveDownScript`) で、underwater Surface が B button (`TrySetupDiveEmergeScript`)。party menu 入口は `SetUpFieldMove_Dive()` が `TrySetDiveWarp()` を見て Dive / Surface のどちらにも入る。Surface の field trigger は B だが、yes/no prompt の決定は通常 UI どおり A なので、誤操作防止として Dive / Surface だけ確認を残す。

Field Kit は UI / lore anchor として単一 item に留め、実際の解禁は capability flags で管理する。現時点の capability flags は既存 `FLAG_RECEIVED_HM_*` を再利用しているため save flag 増加はない。per-HM key item は Key Items pocket を圧迫し、badge-only はジム進行以外の解禁設計が窮屈になる。現状の Key Items pocket は `BAG_KEYITEMS_COUNT 30` で、save block の `keyItems[BAG_KEYITEMS_COUNT]` に固定長で入っているため、bag 拡張は大型改修として別 feature に分離する。

2026-05-09 の手動確認では、Field Kit itemization と debug shortcut は期待どおりに動作した。追加方針として、Field Kit の bag field use は Fly launcher として扱う。つまり `ITEM_FIELD_KIT` を「使う」操作は、Fly capability / badge gate を満たす場合に Fly の region map 導線へ入る。これで HM move を party menu から探す必要がある問題を Key Item に集約できる。Teleport / Dig など非 HM utility move は対象外。

## Validation

- `rtk make -j16 -O check`: PASS on 2026-05-09 before and after the prompt removal update.
- `rtk make -j16 -O all`: PASS on 2026-05-09 before and after the prompt removal update.
- mGBA Live boot / input check: PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba`, accepted Start input, and showed the continue menu. Screenshots: `/tmp/field_move_modernization_boot.png`, `/tmp/field_move_modernization_post_prompt_boot.png`.
- Manual user validation on 2026-05-09 confirmed the first runtime slice before the follow-up prompt removal / Flash auto-use adjustment.
- `feature/field-move-toolkit-item`: `rtk make -j16 -O check`: PASS on 2026-05-09 after Field Kit itemization.
- `feature/field-move-toolkit-item`: `rtk make -j16 -O all`: PASS on 2026-05-09 after Field Kit itemization.
- `feature/field-move-toolkit-item`: mGBA Live boot / input check: PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba`, accepted Start input, and showed the continue menu screenshot at `/tmp/field_move_toolkit_item_final_boot.png`; session stopped cleanly.
- Field Kit debug shortcut update: `rtk make -j16 -O debug`, `rtk make -j16 -O check`, and `rtk make -j16 -O all`: PASS on 2026-05-09. Final local ROM was rebuilt with `-O debug` for manual shortcut validation.
- Field Kit debug shortcut boot check: mGBA Live PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the debug ROM, accepted Start input, and showed the continue menu screenshot at `/tmp/field_kit_debug_script_boot.png`; session stopped cleanly.
- Manual user validation on 2026-05-09 confirmed Field Kit itemization and debug shortcut behavior as intended.

## Remaining Runtime Checks

- Next implementation candidate: set `ITEM_FIELD_KIT` bag field use to Fly launcher / region map entry, gated by Field Kit + Fly capability + badge policy.
- Keep Teleport / Dig out of this item-use path; they remain regular non-HM field utility moves.
