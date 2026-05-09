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

## Manual Tests

### Cut

- Cutter's House gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_CUT`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_CUT` is not set.
- Cut remains blocked without Field Kit, without `FLAG_RECEIVED_HM_CUT`, or without the Stone Badge while `OW_FIELD_MOVE_TOOLKIT_BADGES == TRUE`.
- Cut tree interaction before unlock.
- Cut tree interaction after unlock with no success prompt/message.
- Cut tree object removal and map reload behavior.
- Cut from party menu if party menu route remains enabled.
- follower field move user path.

### Rock Smash

- Rock Smash Dude gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_ROCK_SMASH`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_ROCK_SMASH` is not set.
- Rock Smash before unlock.
- Rock Smash after unlock with no success prompt/message.
- Rock object removal.
- Rusturf Tunnel state update.
- Rock Smash wild encounter trigger.
- Rock Smash from party menu if enabled.

### Strength

- Rusturf Tunnel reunion gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_STRENGTH`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_STRENGTH` is not set.
- Strength before unlock.
- Strength activation after unlock with no success prompt/message.
- Already-active boulder interaction produces no success message.
- `FLAG_SYS_USE_STRENGTH` set and clear after map transition / whiteout / fly / teleport / dig.
- boulder movement on Emerald and FRLG maps.

### Surf

- Wally's Dad gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_SURF`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_SURF` is not set.
- A-button water interaction.
- party menu Surf if enabled.
- no party mon with Surf under modern mode.
- successful Surf starts without yes/no prompt or success message.
- follower hide / return.
- surf blob graphics and player state.
- fast water message.

### Waterfall

- Wallace gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_WATERFALL`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_WATERFALL` is not set.
- waterfall when not surfing north.
- waterfall when surfing north.
- successful Waterfall starts without yes/no prompt or success message.
- follower hide / return.
- movement until top of waterfall.

### Dive

- Steven gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_DIVE`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_DIVE` is not set.
- A button dive down from a diveable tile.
- B button opens Surface prompt underwater.
- party menu Dive works for both Dive down and Surface when `TrySetDiveWarp()` allows it.
- Dive and Surface keep yes/no prompts to prevent accidental double-tap warps.
- `TrySetDiveWarp` failure cases.
- follower sprite after dive.

### Flash

- Granite Cave hiker gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_FLASH`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_FLASH` is not set.
- cave with `requires_flash: true`.
- unlock済みなら cave 入場時に自動で明るくなること。
- manual Flash field effect is not auto-started during map load; only `FLAG_SYS_USE_FLASH` is set.
- cave already flashed.
- whiteout / fly / teleport / dig clears `FLAG_SYS_USE_FLASH`.

### Fly

- Route 119 rival gives `ITEM_FIELD_KIT` if missing and sets `FLAG_RECEIVED_HM_FLY`.
- If Field Kit cannot be added, bag full message is shown and `FLAG_RECEIVED_HM_FLY` is not set.
- Fly remains gated by Field Kit + Fly capability + Fortree badge while `OW_FIELD_MOVE_TOOLKIT_BADGES == TRUE`.

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
