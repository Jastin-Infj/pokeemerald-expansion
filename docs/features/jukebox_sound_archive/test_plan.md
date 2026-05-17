# Jukebox / Sound Archive Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Future Build Validation

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future mGBA Checks

- Open Jukebox from entry point.
- Play at least 3 different tracks.
- Press B to close.
- Confirm map BGM returns, or document failure.
- Reopen Jukebox.
- Hold / mash A and B.
- Confirm no lockup.
- Confirm no save corruption risk because no save data is used.
