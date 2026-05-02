# Field Move Modernization Test Plan

## Manual Tests

### Cut

- Cut tree interaction before unlock.
- Cut tree interaction after unlock.
- Cut tree cancellation.
- Cut tree object removal and map reload behavior.
- Cut from party menu if party menu route remains enabled.
- follower field move user path.

### Rock Smash

- Rock Smash before unlock.
- Rock Smash after unlock.
- Rock object removal.
- Rusturf Tunnel state update.
- Rock Smash wild encounter trigger.
- Rock Smash from party menu if enabled.

### Strength

- Strength before unlock.
- Strength activation after unlock.
- `FLAG_SYS_USE_STRENGTH` set and clear after map transition / whiteout / fly / teleport / dig.
- boulder movement on Emerald and FRLG maps.

### Surf

- A-button water interaction.
- party menu Surf if enabled.
- no party mon with Surf under modern mode.
- follower hide / return.
- surf blob graphics and player state.
- fast water message.

### Waterfall

- waterfall when not surfing north.
- waterfall when surfing north.
- follower hide / return.
- movement until top of waterfall.

### Dive

- dive down from diveable tile.
- emerge underwater.
- `TrySetDiveWarp` failure cases.
- follower sprite after dive.

### Flash

- cave with `requires_flash: true`.
- cave already flashed.
- whiteout / fly / teleport / dig clears `FLAG_SYS_USE_FLASH`.

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
