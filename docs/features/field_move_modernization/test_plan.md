# Field Move Modernization Test Plan

## Validation Log

2026-05-09 on `feature/field-move-modernization-mvp`:

- `rtk make -j16 -O check`: PASS.
- `rtk make -j16 -O all`: PASS.
- mGBA Live boot / input check: PASS. Direct script-build path failed with missing `DISPLAY`; wrapper `/home/jastin/.local/bin/mgba-qt` worked. Confirmed title screen, Start input, continue menu screenshot at `/tmp/field_move_modernization_boot.png`, then stopped session cleanly.
- Manual user validation: PASS for the first runtime slice before the follow-up prompt removal / Flash auto-use adjustment.
- Follow-up prompt removal / Flash auto-use update:
  - `rtk make -j16 -O check`: PASS.
  - `rtk make -j16 -O all`: PASS.
  - mGBA Live boot / input check: PASS. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba`, accepted Start input, and showed the continue menu screenshot at `/tmp/field_move_modernization_post_prompt_boot.png`; session stopped cleanly. `pgrep` showed only the mGBA Live MCP server processes, not the stopped ROM process.
- Focused no-HM route check: not yet performed; needs a prepared save/debug route with badges and no HM moves.
- Field Kit itemization update:
  - `rtk make -j16 -O check`: PASS.
  - `rtk make -j16 -O all`: PASS.
  - mGBA Live boot / input check: PASS. Wrapper `/home/jastin/.local/bin/mgba-qt` booted `pokeemerald.gba`, accepted Start input, and showed the continue menu screenshot at `/tmp/field_move_toolkit_item_final_boot.png`; session stopped cleanly. `pgrep` showed only the mGBA Live MCP server processes, not the stopped ROM process.
- Debug shortcut update:
  - Debug menu paths: `Scripts... > Field Kit Full`, `Field Kit Item`, `Field Kit Flags`, `Field Kit Clear`.
  - Expected `Field Kit Full` effect: gives `ITEM_FIELD_KIT`, sets all field-move capability flags, and sets all badge flags.
  - Expected focused effects: `Field Kit Item` gives only the item, `Field Kit Flags` sets only capability flags, and `Field Kit Clear` removes the item and clears capability / system flags.
  - `rtk make -j16 -O debug`: PASS.
  - `rtk make -j16 -O check`: PASS.
  - `rtk make -j16 -O all`: PASS.
  - mGBA Live boot / input check: PASS. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the debug ROM, accepted Start input, and showed the continue menu screenshot at `/tmp/field_kit_debug_script_boot.png`; session stopped cleanly. `pgrep` showed only the mGBA Live MCP server processes, not the stopped ROM process.
- Manual user validation: PASS. Field Kit itemization and debug shortcut behavior matched the intended behavior.
- Implemented follow-up direction:
  - `ITEM_FIELD_KIT` bag field use and SELECT registered key item use now open the Field Kit utility menu.
  - Fly use remains gated by Field Kit + Fly capability flag + badge policy while `OW_FIELD_MOVE_TOOLKIT_BADGES == TRUE`.
  - Teleport / Dig are Field Kit utility shortcuts and remain map-gated by their existing setup helpers.
- Field Kit menu update:
  - Expected menu order: Fly / Teleport / Dig when Fly is available; Teleport / Dig when Fly is not available.
  - `rtk make -j16 -O all`: PASS.
  - `rtk make -j16 -O check`: PASS.
  - `rtk make -j16 -O debug`: PASS.
  - mGBA Live boot / input check: PASS. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the debug ROM, loaded the save, reached the field, and SELECT without a registered item still showed the existing guidance message. Screenshot at `/tmp/field_kit_menu_final_boot.png`; session stopped cleanly. `pgrep` showed no `mgba-qt` ROM process after stop.
- Field Kit menu polish:
  - Expected Fly-unavailable menu frame height: 2 rows for Teleport / Dig, not a 3-row frame.
  - Expected Teleport / Dig behavior: no return-to-field fade flash between choosing the option and starting the field move effect.
  - Expected frame palette: standard black window frame in caves / dark maps after Field Kit menu opens.
  - `rtk make -j16 -O all`: PASS.
  - `rtk make -j16 -O check`: PASS.
  - `rtk make -j16 -O debug`: PASS.
  - mGBA Live boot / input check: PASS. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the debug ROM, loaded the save, reached the field, and SELECT without a registered item still showed the existing guidance message. Screenshot at `/tmp/field_kit_menu_polish_boot.png`; session stopped cleanly. `pgrep` showed no `mgba-qt` ROM process after stop.
  - Manual user validation: PASS for the 2-row Fly-unavailable frame and the standard black window frame palette. Remaining issue before this follow-up: Fly still briefly brightened night maps.
- Field Kit Fly night fade follow-up:
  - Expected Fly behavior: selecting Fly uses the current time-of-day palette as the black fade source, so night maps do not briefly brighten before the region map opens.
  - `rtk make -j16 -O all`: PASS.
  - `rtk make -j16 -O check`: PASS.
  - `rtk make -j16 -O debug`: PASS.
  - mGBA Live boot / input check: PASS. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the debug ROM, loaded the save, reached the field, and SELECT without a registered item still showed the existing guidance message. Screenshot at `/tmp/field_kit_fly_fade_boot.png`; session stopped cleanly. `pgrep` showed no `mgba-qt` ROM process after stop.
  - Manual user validation: PASS. Fly no longer briefly brightened the night field before the region map.
- Field Kit final manual validation:
  - Key Items use path: PASS.
  - SELECT registered key item path: PASS.
  - Fly / Teleport / Dig order and red Fly label: PASS.
  - Fly-unavailable Teleport / Dig 2-row frame: PASS.
  - Standard black window border palette in cave / dark-map contexts: PASS.
  - Teleport / Dig direct utility start without field brightness flash: PASS.
  - Fly night fade into region map: PASS.
  - Icon / palette follow-up: Field Kit now uses `field_styler.png` / `field_styler.pal`; manual bag visual confirmation is still useful after rebuild.
- Field Kit icon / palette wiring:
  - PNG format check: `field_styler.png` is 24x24, 4-bit indexed, 16 palette entries, non-interlaced, with transparent palette index 0.
  - Palette format check: `field_styler.pal` is JASC-PAL 0100 with 16 RGB rows matching the PNG palette.
  - Item data check: `ITEM_FIELD_KIT` uses `gItemIcon_FieldStyler` / `gItemIconPalette_FieldStyler`, not `gItemIcon_QuestionMark`.
  - `rtk make -j16 -O all`: PASS on 2026-05-09.
  - `rtk make -j16 -O debug`: PASS on 2026-05-09.
  - `rtk make -j16 -O check`: PASS on 2026-05-09.
  - mGBA Live boot / input check: PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the rebuilt ROM, accepted Start input, and showed the continue menu screenshot at `/tmp/field_kit_icon_wiring_boot.png`; session stopped cleanly. `pgrep` showed only the mGBA Live MCP server processes, not the stopped ROM process.
  - Manual bag visual confirmation after rebuild: pending user check.
- Capture Styler reference art revision:
  - Expected visual change: reduce the heavy black outer border, make the gray strap read more like a tilted ring / bracelet, and keep the top body closer to the official Capture Styler angle with a red shell, gold ring, and blue lens.
  - Cleanup pass: reduce roughness by removing isolated pixels, simplifying the ring into larger gray clusters, and keeping highlights as small blocks instead of scattered 1px noise.
  - Original-reference refit: recheck Nintendo's `img_capture.gif` and bias the icon back toward the source image's longer slanted red top shell, right-side red cover, high blue lens, and wider U-shaped strap.
  - Gap pass: carve a deliberate transparent opening between the upper body and the lower U-shaped strap so the bracelet ring has visible negative space like the source art.
  - Lower-ring pass: make the bracelet loop itself more true-circle-like and reduce the flat-bottom strap read while preserving the body/ring gap.
  - XLSX half-scale pass: read the user-provided 48x48 cell-art rough, use `A2:AV49` as the source crop, flood-fill only border-connected black as transparency, and downsample 2x2 source blocks into a 24x24 test icon.
  - XLSX stylized pass: use the 48x48 cell art as a rough shape / line reference, then redraw the final 24x24 icon with broader GBA item-icon clusters instead of preserving every half-scale source detail. The source rough is intentionally not committed.
  - PNG / palette format check: PASS. `field_styler.png` remains 24x24 4-bit indexed with 16 palette entries and transparent palette index 0; `field_styler.pal` still matches the PNG palette.
  - `rtk make -j16 -O all`: PASS on 2026-05-09.
  - `rtk make -j16 -O debug`: PASS on 2026-05-09.
  - mGBA Live boot / input check: PASS on 2026-05-09. Wrapper `/home/jastin/.local/bin/mgba-qt` booted the rebuilt ROM, accepted Start input, and showed the continue menu screenshot at `/tmp/field_styler_reference_art_boot.png`; session stopped cleanly. `pgrep` showed only the mGBA Live MCP server processes, not the stopped ROM process.
  - Ring-angle follow-up mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_ring_angle_boot.png`; session stopped cleanly.
  - Cleanup follow-up mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_cleanup_boot.png`; session stopped cleanly.
  - Original-reference refit mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_original_refit_boot.png`; session stopped cleanly.
  - Gap pass mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_gap_pass_boot.png`; session stopped cleanly.
  - Lower-ring pass mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_round_ring_boot.png`; session stopped cleanly.
  - XLSX half-scale pass mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_xlsx_half_boot.png`; session stopped cleanly.
  - XLSX stylized pass mGBA Live boot / input check: PASS on 2026-05-09. Screenshot at `/tmp/field_styler_xlsx_stylized_boot.png`; session stopped cleanly.
  - Manual bag visual confirmation after rebuild: pending user check.

## Manual Tests

### Debug Shortcut

- `Scripts... > Field Kit Full` shows the FIELD KIT completion message.
- `Scripts... > Field Kit Item`, `Field Kit Flags`, and `Field Kit Clear` show their completion messages.
- Key Items contains `ITEM_FIELD_KIT` with the Field Styler icon / palette, not the placeholder question mark icon.
- Cut / Rock Smash / Strength / Surf / Waterfall can be checked immediately from suitable maps.
- Dive / Surface still keep yes/no prompts after the shortcut.
- Flash still auto-lights a `requires_flash: true` cave after the shortcut.

### Field Kit Menu

- Register `ITEM_FIELD_KIT` to SELECT; pressing SELECT on the field opens the Field Kit menu.
- Use `ITEM_FIELD_KIT` from the Key Items pocket; it opens the same Field Kit menu.
- With `Field Kit Full`, menu order is Fly / Teleport / Dig and Fly is highlighted in red.
- With only `Field Kit Item`, Fly is hidden and the menu shows Teleport / Dig in a 2-row frame.
- Fly opens the region map when Field Kit + Fly capability + badge policy are satisfied.
- B on the Fly region map returns to the field, not the party menu.
- B on the Field Kit menu closes it and returns control to the player.
- Teleport and Dig obey their existing map restrictions; invalid maps show the cannot-use message and return control.
- Teleport and Dig begin directly from the Field Kit menu without the field briefly flashing brighter on night maps.
- Field Kit menu frame keeps the standard black palette in caves and dark maps.
- Fly from the Field Kit menu does not briefly brighten the field on night maps before opening the region map.
- Confirm follower leave restrictions still block Fly / Teleport where `CheckFollowerNPCFlag(FOLLOWER_NPC_FLAG_CAN_LEAVE_ROUTE)` fails.

### Cut

- Cutter's House gives `ITEM_FIELD_KIT` if missing and sets the Cut capability flag.
- If Field Kit cannot be added, bag full message is shown and the Cut capability flag is not set.
- Cut remains blocked without Field Kit, without Cut capability, or without the Stone Badge while `OW_FIELD_MOVE_TOOLKIT_BADGES == TRUE`.
- Cut tree interaction before unlock.
- Cut tree interaction after unlock with no success prompt/message.
- Cut tree object removal and map reload behavior.
- Cut from party menu if party menu route remains enabled.
- follower field move user path.

### Rock Smash

- Rock Smash Dude gives `ITEM_FIELD_KIT` if missing and sets the Rock Smash capability flag.
- If Field Kit cannot be added, bag full message is shown and the Rock Smash capability flag is not set.
- Rock Smash before unlock.
- Rock Smash after unlock with no success prompt/message.
- Rock object removal.
- Rusturf Tunnel state update.
- Rock Smash wild encounter trigger.
- Rock Smash from party menu if enabled.

### Strength

- Rusturf Tunnel reunion gives `ITEM_FIELD_KIT` if missing and sets the Strength capability flag.
- If Field Kit cannot be added, bag full message is shown and the Strength capability flag is not set.
- Strength before unlock.
- Strength activation after unlock with no success prompt/message.
- Already-active boulder interaction produces no success message.
- `FLAG_SYS_USE_STRENGTH` set and clear after map transition / whiteout / fly / teleport / dig.
- boulder movement on Emerald and FRLG maps.

### Surf

- Wally's Dad gives `ITEM_FIELD_KIT` if missing and sets the Surf capability flag.
- If Field Kit cannot be added, bag full message is shown and the Surf capability flag is not set.
- A-button water interaction.
- party menu Surf if enabled.
- no party mon with Surf under modern mode.
- successful Surf starts without yes/no prompt or success message.
- follower hide / return.
- surf blob graphics and player state.
- fast water message.

### Waterfall

- Wallace gives `ITEM_FIELD_KIT` if missing and sets the Waterfall capability flag.
- If Field Kit cannot be added, bag full message is shown and the Waterfall capability flag is not set.
- waterfall when not surfing north.
- waterfall when surfing north.
- successful Waterfall starts without yes/no prompt or success message.
- follower hide / return.
- movement until top of waterfall.

### Dive

- Steven gives `ITEM_FIELD_KIT` if missing and sets the Dive capability flag.
- If Field Kit cannot be added, bag full message is shown and the Dive capability flag is not set.
- A button dive down from a diveable tile.
- B button opens Surface prompt underwater.
- party menu Dive works for both Dive down and Surface when `TrySetDiveWarp()` allows it.
- Dive and Surface keep yes/no prompts to prevent accidental double-tap warps.
- `TrySetDiveWarp` failure cases.
- follower sprite after dive.

### Flash

- Granite Cave hiker gives `ITEM_FIELD_KIT` if missing and sets the Flash capability flag.
- If Field Kit cannot be added, bag full message is shown and the Flash capability flag is not set.
- cave with `requires_flash: true`.
- unlock済みなら cave 入場時に自動で明るくなること。
- manual Flash field effect is not auto-started during map load; only `FLAG_SYS_USE_FLASH` is set.
- cave already flashed.
- whiteout / fly / teleport / dig clears `FLAG_SYS_USE_FLASH`.

### Fly

- Route 119 rival gives `ITEM_FIELD_KIT` if missing and sets the Fly capability flag.
- If Field Kit cannot be added, bag full message is shown and the Fly capability flag is not set.
- Fly remains gated by Field Kit + Fly capability + Fortree badge while `OW_FIELD_MOVE_TOOLKIT_BADGES == TRUE`.
- Using `ITEM_FIELD_KIT` from the Key Items pocket or SELECT registered shortcut should expose Fly as the first Field Kit menu option when Fly is available.

### Move Forget / HM Restriction

- battle level-up learn move tries to overwrite HM.
- evolution learn move tries to overwrite HM.
- summary screen select move tries to forget HM.
- Lilycove Move Deleter Surf last-mon path.
- Fuchsia Move Deleter path.
- `P_CAN_FORGET_HIDDEN_MOVE` true/false behavior if config is changed later.

### Release / Catch Swap

- PC release last Surf mon.
- PC release last Dive mon.
- PC release Strength / Rock Smash mon in league maps.
- catch swap with `B_CATCH_SWAP_CHECK_HMS`.

## Regression Tests to Add Later

実装時に検討する automated tests:

- `CannotForgetMove` behavior table.
- `IsMoveHM` over `FOREACH_HM`.
- modern field move capability helper if added.
- field move unlock helper for badge/key item flags.

## Visual Checks

- `SetPlayerAvatarFieldMove` pose direction.
- `FLDEFF_FIELD_MOVE_SHOW_MON` indoor and outdoor streaks.
- Surf transition into surf blob.
- Waterfall and Dive show-mon timing.
- follower swap animation for Cut / Rock Smash.
