# Field Notes / Lore Codex Test Plan

## Docs-only Validation

- `rtk mdbook build docs`

## Future Runtime Validation

- `rtk make -j16 -O all`
- `rtk make -j16 -O debug`
- `rtk make -j16 -O check`

## Future mGBA Checks

- Open viewer.
- Select each entry.
- Close and reopen.
- Verify long text display, if used.
- Mash input.
- Confirm no save changes are required.
