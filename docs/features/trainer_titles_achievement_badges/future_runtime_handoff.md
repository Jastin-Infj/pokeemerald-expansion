# Future Runtime Handoff: Trainer Titles / Achievement Badges

FUTURE USE ONLY.

```text
runtime implementation task.

Goal:
Implement title clerk MVP with event flags only. Do not modify Trainer Card and
do not add SaveBlock fields.

branch:
feature/trainer-titles-achievement-badges-mvp

Docs:
- docs/features/trainer_titles_achievement_badges/*

MVP:
- 3 fixed titles.
- Event flag ownership only.
- NPC grants titles from simple conditions.
- NPC displays earned titles.
- No equipped active title.
- No Trainer Card UI.
- No save layout change.

Validation:
rtk mdbook build docs
rtk make -j16 -O all
rtk make -j16 -O debug
rtk make -j16 -O check

mGBA:
- Talk to title clerk before earning.
- Meet / simulate one condition.
- Grant title.
- Re-talk and confirm duplicate protection.
- Confirm Trainer Card behavior is unchanged.
```
