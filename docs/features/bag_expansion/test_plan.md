# Bag Expansion Test Plan

## Build / Lint

| Test | Command | Expected |
|---|---|---|
| Normal build | `rtk make -j16 -O all` | Build passes. |
| Focused check | `rtk make -j16 -O check TESTS=save` | Save compatibility test either passes unchanged or fails only with intentional expected-size update. |
| Full check | `rtk make -j16 -O check` | Item / save related checks pass. |
| Debug build | `rtk make -j16 -O debug` | Required if debug fill or Field Kit debug routes are used for validation. |

## Focused Tests

| Test | Steps | Expected |
|---|---|---|
| SaveBlock size | Compare `sizeof(struct SaveBlock1)` against `test/save.c`. | Any size change is intentional and documented. |
| Affected pocket fill | Add enough distinct items to fill the expanded pocket. | `CheckBagHasSpace` returns false only when the new capacity is reached. |
| Sort affected pocket | Fill and sort the expanded pocket by each available sort mode. | No crash, lost item, duplicate item, or cursor corruption. |
| Save/load bag | Fill affected pocket, save, reload. | Item IDs, quantities, cursor positions, and registered item state remain valid. |

## Manual Checks

| Check | Steps | Expected |
|---|---|---|
| Bag UI scroll | Open the expanded pocket and scroll from first to last item. | Smooth scrolling; Cancel row appears correctly. |
| Debug fill | Use debug PC/Bag fill for affected pockets. | Fill completes; bag remains usable. |
| Field Kit | Register and use `ITEM_FIELD_KIT` from Key Items. | Field Kit menu still opens and returns cleanly. |
| Bag full script | Trigger an item gift when the target pocket is full. | Existing bag full message appears and no flag is set early. |
| Wally tutorial | Run Wally tutorial bag route. | Temporary item / Poke Ball pockets restore correctly. |
| Battle Pyramid | Enter / exit Pyramid-style inventory paths if touched indirectly. | Pyramid bag behavior unchanged. |

## Results

| Date | Command / check | Result | Notes |
|---|---|---|---|
| 2026-05-09 | Docs-only kickoff | Not run | No source / data / config changes. Runtime validation will be required once pocket constants change. |

## Feature Complete Gate

- Target pocket counts and save compatibility policy are documented.
- `test/save.c` either remains unchanged or has an intentional expected-size update.
- `rtk make -j16 -O all`, `rtk make -j16 -O check`, and required focused save tests pass.
- mGBA Live or manual runtime confirms bag open, fill, scroll, save/load, and affected downstream feature path.
- `docs/features/feature_registry.md`, affected feature docs, and `docs/upgrades/upstream_diff_checklist.md` link the final policy.

## Open Questions

- Which `.sav` files should be used for migration regression if save compatibility is required?
- Does the final target need mGBA validation for both normal and debug ROMs?
