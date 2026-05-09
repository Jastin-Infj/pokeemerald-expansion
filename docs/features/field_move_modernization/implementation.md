# Field Move Modernization Implementation Notes

最終更新: 2026-05-09
ブランチ: `feature/field-move-modernization-mvp`
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

## Deliberately Unchanged

この slice では以下はまだ変更していない。

- map object 配置。Cut tree / Rock Smash rock / Strength boulder は現行 map design のまま。
- party menu の field move action。現行どおり「Pokemon が技を持つ場合だけ」表示される。
- Fly の代替 UI。Poke Rider / region map shortcut は Phase 3 扱い。
- HM item 自体、TM/HM pocket、learn move UI。
- key item / power-up item による unlock source。現状は badge unlock のまま。
- `P_CAN_FORGET_HIDDEN_MOVE`、`CannotForgetMove`、Move Deleter、PC release、catch-swap HM checks。
- Rock Smash wild encounter と Rusturf Tunnel update。

## Impact Notes

`VAR_RESULT` は modernized HM でも `PARTY_SIZE` ではなく party slot を返す。これは既存 script / field effect の入力形を壊さないための互換措置であり、Pokemon show-mon banner を戻す場合も out-of-range にならない。

Pokemon show-mon banner を無効化する箇所は `field_effect.c` に閉じている。呼び出し側の task は従来どおり `FLDEFF_FIELD_MOVE_SHOW_MON` の終了を待つが、無効時は active list に入らないため即座に次 state へ進む。

Flash auto-use は manual Flash field effect (`FldEff_UseFlash`) を map load 中に起動せず、map 初期化中に `FLAG_SYS_USE_FLASH` だけを set する。既存の cave enter transition / field callback / script context と競合させないための安全寄せ。入場時に明滅演出まで入れる場合は、map load callback と palette fade の順序を別途検証する。

Dive の field input は既存コード上、Dive down が A button (`TrySetupDiveDownScript`) で、underwater Surface が B button (`TrySetupDiveEmergeScript`)。party menu 入口は `SetUpFieldMove_Dive()` が `TrySetDiveWarp()` を見て Dive / Surface のどちらにも入る。Surface の field trigger は B だが、yes/no prompt の決定は通常 UI どおり A なので、誤操作防止として Dive / Surface だけ確認を残す。

将来の unlock source は badge-only から、story flag、per-HM key item、または単一の field toolkit / power-up item へ移せるように検討を残す。推奨は単一 key item を UI / lore anchor として持たせ、実際の解禁は `FIELD_TOOLKIT_SURF` / `FIELD_TOOLKIT_DIVE` のような capability flags で管理する方式。per-HM key item は Key Items pocket を圧迫し、badge-only はジム進行以外の解禁設計が窮屈になる。現状の Key Items pocket は `BAG_KEYITEMS_COUNT 30` で、save block の `keyItems[BAG_KEYITEMS_COUNT]` に固定長で入っているため、per-HM key item を追加する前に bag capacity / save layout / debug item grant の整理が必要。bag 拡張は大型改修になるため、この feature では実装せず、別 feature として分離する。

## Validation

- `rtk make -j16 -O check`: PASS on 2026-05-09 before and after the prompt removal update.
- `rtk make -j16 -O all`: PASS on 2026-05-09 before and after the prompt removal update.
- mGBA Live boot / input check: PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba`, accepted Start input, and showed the continue menu. Screenshots: `/tmp/field_move_modernization_boot.png`, `/tmp/field_move_modernization_post_prompt_boot.png`.
- Manual user validation on 2026-05-09 confirmed the first runtime slice before the follow-up prompt removal / Flash auto-use adjustment.

## Remaining Runtime Checks

- Focused manual route after latest prompt update: Cut / Rock Smash / Strength / Surf / Waterfall は成功時 message なし、Dive / Surface は確認あり、Flash cave は入場時に自動で明るくなること。
