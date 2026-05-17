# Weather Lab Terminal

## Status

- Status: Planned
- Code status: No code changes on master
- Docs status: Initial design
- Feature type: Fully new local feature candidate

## Goal

Create a small terminal / NPC that can switch the current map weather for
testing, presentation, and atmosphere checks.

## First MVP

- Use existing weather types only.
- No new weather effects.
- No save data.
- No runtime rule option.
- Open from Weather Institute, debug map, or safe test map.
- Menu options: Clear, Rain, Thunderstorm, Sandstorm, Fog / Ash / safe existing weather, Reset / Cancel.
- Apply weather to current map if existing engine supports it.
- If restoring default weather is not straightforward, document this as a known risk.

## Non-goals

- Do not rewrite the weather system.
- Do not save custom weather.
- Do not change story weather events.
- Do not add new visual effects.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
