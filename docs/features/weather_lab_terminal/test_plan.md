# Weather Lab Terminal Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Future Runtime Validation

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future mGBA Checks

- Open terminal.
- Switch Clear / Rain / Thunderstorm.
- Cancel / Reset.
- Enter and exit a building.
- Start battle and return.
- Confirm no story progression break.
