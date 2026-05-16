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

- `ITEM_FIELD_KIT` を Key Items pocket に追加した。アイコンは `gItemIcon_FieldStyler` / `gItemIconPalette_FieldStyler` を使い、専用の 24x24 4bpp indexed PNG と JASC palette で表示する。
- `Common_EventScript_GiveFieldKit` を追加し、Field Kit 未所持なら配布、所持済みなら何もしない共通 script にした。
- Cut / Flash / Rock Smash / Strength / Surf / Fly / Dive / Waterfall の Emerald 入手イベントは、各 HM item の代わりに `Common_EventScript_GiveFieldKit` を呼ぶ。
- Field Kit 配布が失敗した場合は既存 `Common_EventScript_ShowBagIsFull` に入り、対応 capability flag を立てない。
- 古い HM receive flags は TM Shop Migration で `FLAG_UNUSED_0x...` に戻したため、Field Kit capability flags は field move feature 側で別名 / 別枠として用意する。
- `OW_FIELD_MOVE_TOOLKIT_REQUIRED` を追加し、modernized HM field move は Field Kit 所持 + 対応 capability flag を要求する。
- `OW_FIELD_MOVE_TOOLKIT_BADGES` を追加し、初期値では既存 badge gate も維持する。進行順を壊さず item 化するための安全寄せ。将来 capability-only にする場合はここを FALSE にする。
- Debug menu の `Scripts...` 先頭 4 項目を Field Kit 検証用にした。`Field Kit Full` は Field Kit 付与、全 HM capability flags、全 badge flags を一括付与する。`Field Kit Item` は item のみ、`Field Kit Flags` は capability flags のみ、`Field Kit Clear` は Field Kit item と capability / system flags を消す。
- `ITEM_FIELD_KIT` の field use を追加した。Key Items pocket の「使う」と registered key item の SELECT 起動は同じ Field Kit menu に入り、選択肢は Fly / Teleport / Dig の順で出す。
- Fly は Field Kit + Fly capability flag + badge gate を満たす場合だけ menu に表示する。Fly 選択時は region map を直接開き、B cancel では party menu ではなく field に戻る。
- Teleport / Dig は HM capability ではなく Field Kit utility menu の常設 shortcut として扱う。既存 `SetUpFieldMove_Teleport()` / `SetUpFieldMove_Dig()` の map / follower 条件が失敗した場合は既存の cannot-use message を表示する。
- Field Kit menu は option count に合わせて window height を縮める。Fly 未解禁時は Teleport / Dig の 2-row frame にする。
- Field Kit からの Teleport / Dig は party menu と同じ return-to-field fade を挟まず、menu close 後に `gPostMenuFieldCallback` を直接開始する。night palette などで field が一瞬明るく見える戻りフェードを避けるための Field Kit 専用 path。
- Field Kit menu 表示前に `LoadMessageBoxAndBorderGfx()` を呼び、cave / overworld palette 状態に依存せず標準 window frame palette を使う。
- Field Kit からの Fly map 起動は `BeginNormalPaletteFade()` ではなく `FadeScreen(FADE_TO_BLACK, 0)` を使う。`FadeScreen` は fade-out 前に現在の faded palette を unfaded buffer へ写すため、night / time-of-day palette を一瞬解除せずに黒フェードできる。
- FRLG は separate legacy HM route が残るため、この slice では toolkit requirement を適用せず既存 badge gate を維持する。

## Deliberately Unchanged

この slice では以下はまだ変更していない。

- map object 配置。Cut tree / Rock Smash rock / Strength boulder は現行 map design のまま。
- party menu の field move action。現行どおり「Pokemon が技を持つ場合だけ」表示される。
- Poke Rider / ride UI。Fly の現行代替入口は Field Kit menu として実装済み。
- HM item 自体の通常技化、TM/HM pocket、learn move UI。
- Key Item bag capacity 拡張。[Bag Expansion](../bag_expansion/README.md) の大型改修として扱う。
- `P_CAN_FORGET_HIDDEN_MOVE`、`CannotForgetMove`、Move Deleter、PC release、catch-swap HM checks。
- Rock Smash wild encounter と Rusturf Tunnel update。

## Impact Notes

`VAR_RESULT` は modernized HM でも `PARTY_SIZE` ではなく party slot を返す。これは既存 script / field effect の入力形を壊さないための互換措置であり、Pokemon show-mon banner を戻す場合も out-of-range にならない。

Pokemon show-mon banner を無効化する箇所は `field_effect.c` に閉じている。呼び出し側の task は従来どおり `FLDEFF_FIELD_MOVE_SHOW_MON` の終了を待つが、無効時は active list に入らないため即座に次 state へ進む。

Flash auto-use は manual Flash field effect (`FldEff_UseFlash`) を map load 中に起動せず、map 初期化中に `FLAG_SYS_USE_FLASH` だけを set する。既存の cave enter transition / field callback / script context と競合させないための安全寄せ。入場時に明滅演出まで入れる場合は、map load callback と palette fade の順序を別途検証する。

Dive の field input は既存コード上、Dive down が A button (`TrySetupDiveDownScript`) で、underwater Surface が B button (`TrySetupDiveEmergeScript`)。party menu 入口は `SetUpFieldMove_Dive()` が `TrySetDiveWarp()` を見て Dive / Surface のどちらにも入る。Surface の field trigger は B だが、yes/no prompt の決定は通常 UI どおり A なので、誤操作防止として Dive / Surface だけ確認を残す。

Field Kit は UI / lore anchor として単一 item に留め、実際の解禁は capability flags で管理する。古い HM receive flags は TM Shop Migration で unused に戻したため、capability flags は field move feature 側で別名 / 別枠として持つ。per-HM key item は Key Items pocket を圧迫し、badge-only はジム進行以外の解禁設計が窮屈になる。現状の Key Items pocket は `BAG_KEYITEMS_COUNT 30` で、save block の `keyItems[BAG_KEYITEMS_COUNT]` に固定長で入っているため、[Bag Expansion](../bag_expansion/README.md) は大型改修として別 feature に分離する。

2026-05-09 の手動確認では、Field Kit itemization と debug shortcut は期待どおりに動作した。その後の Field Kit menu slice で、`ITEM_FIELD_KIT` を「使う」操作と SELECT registered key item 起動を Field Kit utility menu に集約した。Fly は capability / badge gate を満たす場合だけ先頭に出し、未解禁時は Teleport / Dig のみを表示する。Teleport / Dig は非 HM utility として Field Kit から呼べるが、解禁 flag は持たず既存 map 条件に従う。

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
- Field Kit menu update: `rtk make -j16 -O all`, `rtk make -j16 -O check`, and `rtk make -j16 -O debug`: PASS on 2026-05-09.
- Field Kit menu boot check: mGBA Live PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the debug ROM, loaded the save, reached the field, and confirmed SELECT without a registered item still shows the existing registered-item guidance. Screenshot: `/tmp/field_kit_menu_final_boot.png`; session stopped cleanly and no `mgba-qt` process remained.
- Field Kit menu polish: Fly-unavailable menu height now uses a 2-row frame, Teleport / Dig start without the return-to-field fade flash, and the menu reloads the standard window border palette before drawing. `rtk make -j16 -O all`, `rtk make -j16 -O check`, and `rtk make -j16 -O debug`: PASS on 2026-05-09. mGBA Live boot / input check also PASS with screenshot `/tmp/field_kit_menu_polish_boot.png`; session stopped cleanly and no `mgba-qt` process remained. User manual validation confirmed the 2-row frame and standard window border palette fix; the remaining Fly night-brightening issue is addressed by the follow-up `FadeScreen(FADE_TO_BLACK, 0)` change.
- Field Kit Fly night fade follow-up: `rtk make -j16 -O all`, `rtk make -j16 -O check`, and `rtk make -j16 -O debug`: PASS on 2026-05-09 after replacing the Field Kit Fly fade with `FadeScreen(FADE_TO_BLACK, 0)`. mGBA Live boot / input check also PASS with screenshot `/tmp/field_kit_fly_fade_boot.png`; session stopped cleanly and no `mgba-qt` process remained. User manual validation confirmed the Fly night fade fix.
- Field Kit icon / palette asset follow-up: `graphics/items/icons/field_styler.png` is a 24x24 4bpp indexed PNG with 16 palette entries and transparent palette index 0. `graphics/items/icon_palettes/field_styler.pal` is a matching JASC 16-color palette. `ITEM_FIELD_KIT` now points to `gItemIcon_FieldStyler` / `gItemIconPalette_FieldStyler`. `rtk make -j16 -O all`, `rtk make -j16 -O debug`, and `rtk make -j16 -O check`: PASS on 2026-05-09. mGBA Live boot / input check also PASS with screenshot `/tmp/field_kit_icon_wiring_boot.png`; session stopped cleanly and no `mgba-qt` ROM process remained.
- Capture Styler reference art follow-up: `field_styler.png` / `field_styler.pal` were revised against Nintendo's Capture Styler reference silhouette, reducing the heavy black outer outline and moving the 24x24 read toward a red body, gold ring, blue lens, and gray wrist strap. A later art pass pushed the silhouette further toward a Mega Ring / bracelet-like read: the gray strap now forms a lower ring, while the red body / gold lens bezel keep a simplified official-style angle. The latest source-guided stylized pass used a user-provided 48x48 cell-art rough as a shape and line reference, but redraws the final 24x24 asset with broader GBA item-icon color clusters instead of copying the half-scale pixel detail exactly. The source rough is intentionally not committed. The asset remains 24x24 4bpp indexed with 16 palette entries and transparent palette index 0. `rtk make -j16 -O all` and `rtk make -j16 -O debug`: PASS on 2026-05-09. mGBA Live boot / input check also PASS with screenshots `/tmp/field_styler_reference_art_boot.png`, `/tmp/field_styler_ring_angle_boot.png`, `/tmp/field_styler_cleanup_boot.png`, `/tmp/field_styler_original_refit_boot.png`, `/tmp/field_styler_gap_pass_boot.png`, `/tmp/field_styler_round_ring_boot.png`, `/tmp/field_styler_xlsx_half_boot.png`, and `/tmp/field_styler_xlsx_stylized_boot.png`; sessions stopped cleanly and no `mgba-qt` ROM process remained.

## Remaining Runtime Checks

- Field Kit menu implementation has been manually confirmed on 2026-05-09, including the Key Items / SELECT menu path, Fly / Teleport / Dig ordering, red Fly label, Fly-unavailable 2-row frame, standard cave / dark-map window border palette, Teleport / Dig no return-to-field brightness flash, and Fly no night-brightening before the region map.
- Visual polish follow-up is now limited to art iteration / naming preference if the current rough Field Styler art is replaced. Use [Field Kit Icon And Palette Manual](../../manuals/field_kit_icon_palette_manual.md) for future icon swaps.
