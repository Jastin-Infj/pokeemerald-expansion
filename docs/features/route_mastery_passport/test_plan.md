# Route Mastery Passport Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Future Runtime Validation

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future mGBA Checks

- Route with no progress.
- After trainer clear.
- After item ball collected.
- After hidden item found.
- Map transitions.
- No duplicate completion reward if a later slice adds rewards.
