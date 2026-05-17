# Future Runtime Handoff: Route Mastery Passport

FUTURE USE ONLY.

```text
runtime implementation task.

Goal:
Implement a Route Mastery Passport proof-of-concept for one route only. Do not
build full region support, do not change save layout, and use existing flags
only after a route-specific audit.

branch:
feature/route-mastery-passport-poc

Docs:
- docs/features/route_mastery_passport/*

MVP:
- One route only.
- Fixed route checklist.
- Existing flags only.
- No rewards in first slice unless explicitly approved.
- No Pokedex integration.

Validation:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- View empty route progress.
- Clear a trainer and confirm checklist update.
- Collect item ball / hidden item if included.
- Transition maps and reopen.
```
