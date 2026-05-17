# Bounty Board Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Future Runtime Validation

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future mGBA Checks

- Open board.
- Insufficient item path.
- Sufficient item path.
- Required item is removed.
- Reward is added.
- Completed flag is set.
- Duplicate reward cannot be claimed.
- Bag full behavior is documented.
