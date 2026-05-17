# Trainer Titles / Achievement Badges Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Future Runtime Validation

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future mGBA Checks

- Title not earned path.
- Grant path.
- Earned display path.
- Duplicate grant protection.
- Confirm no Trainer Card changes in MVP.
