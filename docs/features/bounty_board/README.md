# Bounty Board / Request Board

## Status

- Status: Planned
- Code status: No code changes on master
- Docs status: Initial design
- Feature type: Fully new local feature candidate

## Goal

Add a small request board that offers simple fixed side requests, starting with
item delivery tasks.

## First MVP

- Script-driven only.
- No battle hooks.
- No catch hooks.
- No daily RTC.
- No random generation.
- No SaveBlock changes.
- Use existing event flags for completion.
- 3 fixed item delivery requests.
- If player has required item, remove item and give reward.
- Completed request cannot pay out twice.

Example MVP requests:

- Deliver Potion x1 -> Oran Berry x3
- Deliver Poke Ball x2 -> Great Ball x1
- Deliver Escape Rope x1 -> Nugget x1

Do not rely on exact item names or quantities until confirmed in the runtime branch.

## Non-goals

- No kill quests.
- No catch quests.
- No daily rotation.
- No rank system in first slice.
- No generated bounty text.
- No battle result hooks.

## Related Docs

- [Investigation](investigation.md)
- [MVP Plan](mvp_plan.md)
- [Risks](risks.md)
- [Test Plan](test_plan.md)
- [Future Runtime Handoff](future_runtime_handoff.md)
