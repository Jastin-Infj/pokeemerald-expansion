# Trainer Titles / Achievement Badges

## Status

- Status: Planned
- Code status: No code changes on master
- Docs status: Initial design
- Feature type: Fully new local feature candidate

## Goal

Add lightweight player titles / achievement badges that can be granted through
events or NPC checks.

## First MVP

- Do not modify Trainer Card UI.
- Do not add SaveBlock fields.
- Use existing event flags for a few fixed achievements.
- NPC can display earned titles.
- NPC can grant title if simple condition is met.
- Current title selection is not MVP unless it can be represented safely.

Example titles:

- Route Cleaner
- Berry Keeper
- Rain Caller
- Sound Collector
- Frontier Rookie

## Non-goals

- No Trainer Card integration in first slice.
- No save layout change.
- No achievement database generation.
- No UI-heavy badge page.
- No online / profile system.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
