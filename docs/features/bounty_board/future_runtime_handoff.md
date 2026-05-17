# Future Runtime Handoff: Bounty Board / Request Board

FUTURE USE ONLY.

Do not use this prompt during the docs-only task.

```text
runtime implementation task.

Goal:
Implement Bounty Board / Request Board MVP as a script-driven fixed request
board. This is a new feature, not an extension of battle, TM, Summary, or
Champions systems.

branch:
feature/bounty-board-mvp

Docs:
- docs/features/bounty_board/*

Investigation:
rg "checkitem|removeitem|giveitem|setflag|checkflag|multichoice|message|msgbox" data/maps data/scripts src include

MVP:
- No random generation.
- No daily RTC.
- No SaveBlock additions.
- Existing event flags only for completion.
- 3 fixed requests.
- Item delivery only.
- Existing item rewards.
- No accepted-state save. If the player can turn in, complete immediately.
- Completed requests cannot pay out twice.

Implementation candidate:
- Add board NPC or signpost to a safe debug/test map.
- If using a story map, avoid NPC placement and story flag conflicts.

Forbidden:
- No catch hook.
- No battle-end hook.
- No daily RTC.
- No SaveBlock changes.
- Do not mix with runtime rule options.

Validation:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- Open board.
- Insufficient item path is rejected.
- Sufficient item path completes.
- Required item decreases.
- Reward is added.
- Completion flag prevents duplicate reward.
```
