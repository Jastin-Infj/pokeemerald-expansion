# Route Mastery Passport

## Status

- Status: Planned
- Code status: No code changes on master
- Docs status: Initial design
- Feature type: Fully new local feature candidate
- Complexity: Medium

## Goal

Create a small route completion / exploration record that can show mastery
progress for selected maps.

## First MVP

- Support only 1 or 2 maps.
- Do not attempt full Hoenn coverage.
- No SaveBlock changes.
- Use existing flags where possible.
- Display simple checklist:
  - visited
  - trainers cleared
  - visible item balls collected
  - hidden items found
- Species seen / caught is not MVP.

## Non-goals

- No full region completion system.
- No Pokedex integration.
- No live minimap.
- No new save layout.
- No per-tile exploration tracking.
- No automatic support for all maps.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
